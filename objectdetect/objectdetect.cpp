#include <iostream>
#include <chrono>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "common/support.h"
#include <X11/Xlib.h>

#define INTERVAL_THRESHOLD 10
#define MIN_CONTINUE_REQUIRED 15

using namespace cv;
using namespace std;
namespace fs = boost::filesystem;

const string MAIN_WINDOW_NAME = "Square Detection";


//#include "common/SaveFrameTask.h"
#include "common/angle.h"
#include "common/SaveFrameTaskHighResolution.h"

// returns sequence of squares detected on the frame.
// the sequence is stored in the specified memory storage
vector<vector<Point>> findSquares(const Mat &workingFrame, vector<RotatedRect> &resultRects);

bool isAlreadyExistSquare(RotatedRect rotatedRect, vector<RotatedRect> &rotatedRectList);

// the function draws all the squares in the frame
void drawSquares(Mat &frame, const vector<vector<Point> > &squares);

//void saveFrame(Mat frame, string outputDir, const String fileName);

bool isContinueSquareEnough(const int squareCount, int &groupCounter, int &emptyCounter,
                            int &continueFoundCount);

void doSaveTasks(BlockingQueue<SaveFrameTaskHighResolution> *taskQueue);

string getFileName(const int runningNumber, const String typeImage);

int main(int argc, char **argv) {
    //get screen resolution for setting up window size
    Display* disp = XOpenDisplay(NULL);
    Screen*  scrn = DefaultScreenOfDisplay(disp);
    int height = scrn->height;
    int width  = scrn->width;

    //variable of isContinueSquareEnough
    int groupCounter = 0;
    int emptyCounter = 0;
    int continueFoundCount = 0;

    if (argc != 2) {
        cerr << "usage: paperscanner <output_dir>" << endl;
        return -1;
    }

    string outputDir = argv[1];
    if (!fs::is_directory(fs::path(outputDir)) && !fs::create_directory(fs::path(outputDir))) {
        cerr << "cannot create output_dir" << endl;
        return -2;
    }

    String outputPath = "/home/pi/paper-scaner/paperscanner/in/";

    namedWindow(MAIN_WINDOW_NAME, 1);

    VideoCapture capture(0);
    if (!capture.open(0)) {
        cerr << "Error couldn't open camera" << endl;
        return 1;
    }

    capture.set(CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(CAP_PROP_FPS, 15);

    Mat originalFrame;

    //variable for Show Notification
    double fontScale = 1;
    int thickness = 2;
    int runningNumber = 0;
    Point textOrg(50, 100);
    string messageNotification = "SAVED";

    bool showTextSave = false;
    int countFrameShowTextSave = 0;
    int frameShowTextSave = 12;
    //end variable for Show Notification

    if (!capture.isOpened())
        throw "Error when reading steam";
    capture >> originalFrame;
    if (!capture.read(originalFrame)) {
        cerr << "ERROR: while parsing video originalFrame" << endl;
    }

    BlockingQueue<SaveFrameTaskHighResolution> taskQueue;
    thread taskThread(doSaveTasks, &taskQueue);
    vector<RotatedRect> resultRects;
    String fileNameImageHighRes;
    while (!originalFrame.empty()) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        Mat workingFrame;
        resize(originalFrame, workingFrame, Size(0, 0), RESIZE_FACTOR, RESIZE_FACTOR);
        vector<vector<Point>> squares = findSquares(workingFrame, resultRects);
        drawSquares(workingFrame, squares);

        if (isContinueSquareEnough((const int) squares.size(), groupCounter, emptyCounter, continueFoundCount)) {
            showTextSave = true;
            runningNumber++;
            fileNameImageHighRes = getFileName(runningNumber, ".jpg");
            String outputPathImageHighRes = outputPath + fileNameImageHighRes;
            SaveFrameTaskHighResolution task(outputDir, outputPathImageHighRes, false);
            taskQueue.put(task);
        }

        if (showTextSave) {
            countFrameShowTextSave++;
            if (countFrameShowTextSave <= frameShowTextSave) {
                putText(workingFrame, messageNotification, textOrg, FONT_HERSHEY_DUPLEX, fontScale, CV_RGB(255, 0, 0),
                        thickness,
                        8);
            } else {
                countFrameShowTextSave = 0;
                showTextSave = false;
                cout << "End text SAVE finish" << endl;
            }
        }

        imshow(MAIN_WINDOW_NAME, workingFrame);

        int keyPressed = waitKey(1);
        if ((char) keyPressed == 27) {
            break;
        }

        capture >> originalFrame;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        long processTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        cout << (float) processTime / 1000 << endl;
    }

    SaveFrameTaskHighResolution poisonTask("", "", true);
    taskQueue.put(poisonTask);
    taskThread.join();
    return 0;
}

string getFileName(const int runningNumber, const String typeImage) {
    String fileName;
    time_t rawTime;
    struct tm *currentDateTime;
    char buffer[16];

    time(&rawTime);
    currentDateTime = localtime(&rawTime);
    strftime(buffer, 16, "%Y%m%d-%H%M%S", currentDateTime);

    fileName = String(buffer);
    fileName = fileName + "-" + to_string(runningNumber) + typeImage;

    return fileName;
}

vector<vector<Point>> findSquares(const Mat &workingFrame, vector<RotatedRect> &resultRects) {
    vector<vector<Point>> squares;
    resultRects.clear();
    vector<vector<Point>> contours;
    Mat workingMat;

    Mat MedianBlurMat = workingFrame.clone();
    medianBlur(workingFrame, MedianBlurMat, 11);

    Mat mixedChannelMat(MedianBlurMat.size(), CV_8U);
    // find squares in every color plane of the workingFrame
    for (int c = 0; c < 3; c++) {
        int fromTo[] = {c, 0};
        mixChannels(&MedianBlurMat, 1, &mixedChannelMat, 1, fromTo, 1);
        // Canny helps to catch squares with gradient shading
        Canny(mixedChannelMat, workingMat, 10, 20, 3);
        // Dilate helps to remove potential holes between edge segments
        //dilate(workingMat, workingMat, Mat());
    }
    // Find contours and store them in a list
    findContours(workingMat, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    vector<Point> approxResult;
    double length;
    double area;
    for (size_t i = 0; i < contours.size(); i++) {
        area = fabs(contourArea(contours[i]));
        if (area < MIN_CONTOUR_AREA) {
            continue;
        }
        // approximate contour with accuracy proportional
        // to the contour perimeter
        length = arcLength(Mat(contours[i]), true);
        approxPolyDP(Mat(contours[i]), approxResult, length * 0.02, true);

        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if (approxResult.size() != 4 || fabs(contourArea(Mat(approxResult))) < MIN_CONTOUR_AREA ||
            !isContourConvex(Mat(approxResult))) {
            continue;
        }

        double maxCosine = 0;

        for (int j = 2; j < 5; j++) {
            double cosine = fabs(angle(approxResult[j % 4], approxResult[j - 2], approxResult[j - 1]));
            maxCosine = MAX(maxCosine, cosine);
        }

        if (maxCosine > 0.1) {
            continue;
        }

        //Draw RotatedRect around contour area.
        RotatedRect rotatedRect = minAreaRect(Mat(contours[i]));
        //check that already same as another saved rotatedRect or not.
        if (!isAlreadyExistSquare(rotatedRect, resultRects)) {
            //get points of 4 corners of RotateRect.
            Point2f rectPoints[4];
            rotatedRect.points(rectPoints);
            for (int c = 0; c < 4; c++) {
                approxResult[c] = rectPoints[c];
            }
            //if not, save this rect to the selected list.
            resultRects.push_back(rotatedRect);
            squares.push_back(approxResult);
        }
    }

    return squares;
}

void drawSquares(Mat &frame, const vector<vector<Point> > &squares) {
    for (size_t i = 0; i < squares.size(); i++) {
        for (int j = 0; j < squares[i].size(); j++) {
            circle(frame, squares[i][j], 6, Scalar(255, 255, 0), CV_FILLED, 8, 0);
            line(frame, squares[i][j], squares[i][(j + 1) % squares[i].size()], Scalar(110 + (10 * j), 220, 0), 2, 8);
        }
    }
}

bool isContinueSquareEnough(const int squareCount, int &groupCounter, int &emptyCounter, int &continueFoundCount) {
    bool continueSquares = false;
    if (squareCount > 0) { //if can detect
        if (emptyCounter > INTERVAL_THRESHOLD) {
            continueFoundCount = 1; //clear frame counter in group
        } else {
            continueFoundCount++;
            if (continueFoundCount == MIN_CONTINUE_REQUIRED) { //if detect enough frame in current group,so grouping it.
                groupCounter++;
                //cout << "group : " << group_counter << endl;
                continueSquares = true;
            }
        }
        emptyCounter = 0;
    } else {
        continueFoundCount = 0;
        emptyCounter++;
    }
    return continueSquares;
}

bool isAlreadyExistSquare(const RotatedRect rotatedRect, vector<RotatedRect> &rotatedRectList) {
    vector<Point2f> intersectionRegion;
    int intersectType;
    for (int i = 0; i < rotatedRectList.size(); i++) {  //last element is
        intersectType = rotatedRectangleIntersection(rotatedRect, rotatedRectList[i], intersectionRegion);
        if (intersectType != INTERSECT_NONE) {
            //cout << "--THIS SQUARE IS ALREADY ADD"<< endl;
            return true;
        }
    }
    //cout << "--NOT INTERSECT" << endl;
    return false;
}

void doSaveTasks(BlockingQueue<SaveFrameTaskHighResolution> *taskQueue) {
    SaveFrameTaskHighResolution task;
    while (true) {
        bool hasTask = taskQueue->poll(task, std::chrono::milliseconds(100));
        if (!hasTask) {
            continue;
        }
        if (task.isPoison()) {
            cout << "poisoned die." << endl;
            break;
        }
        task.operate();
    }
}
