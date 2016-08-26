//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
//C++
#include <iostream>

using namespace cv;
using namespace std;

// Global variables
Mat frame; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard
void help();

void processVideo(char *videoFilename);

void processImages(char *firstFrameFilename);

void help() {
    cout
            << "--------------------------------------------------------------------------" << endl
            << "This program shows how to use background subtraction methods provided by " << endl
            << " OpenCV. You can process both videos (-vid) and images (-img)." << endl
            << endl
            << "Usage:" << endl
            << "./bs {-vid <video filename>|-img <image filename>}" << endl
            << "for example: ./bs -vid video.avi" << endl
            << "or: ./bs -img /data/images/1.png" << endl
            << "--------------------------------------------------------------------------" << endl
            << endl;
}

int main(int argc, char *argv[]) {
    //print help information
    help();
    //check for the input parameter correctness
    if (argc != 3) {
        cerr << "Incorret input list" << endl;
        cerr << "exiting..." << endl;
        return EXIT_FAILURE;
    }
    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask MOG 2");
    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach
    if (strcmp(argv[1], "-vid") == 0) {
        //input data coming from a video
        processVideo(argv[2]);
    } else if (strcmp(argv[1], "-img") == 0) {
        //input data coming from a sequence of images
        processImages(argv[2]);
    } else {
        //error in reading input parameters
        cerr << "Please, check the input parameters." << endl;
        cerr << "Exiting..." << endl;
        return EXIT_FAILURE;
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}

void processVideo(char *videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    int morph_size = 4;
    Mat element = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );

    if (!capture.isOpened()) {
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    //read input data. ESC or 'q' for quitting
    while ((char) keyboard != 'q' && (char) keyboard != 27) {
        //read the current frame
        if (!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the background model
        pMOG2->apply(frame, fgMaskMOG2,0);
        Mat finalMat = fgMaskMOG2.clone();
        for (int i=1;i<10;i++)
        {
            morphologyEx(fgMaskMOG2,finalMat, MORPH_OPEN, element);
        }
        imshow("Final",finalMat);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100, 20),
                  cv::Scalar(255, 255, 255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);

        //get the input from the keyboard
        keyboard = waitKey(30);

        //waitKey(0);
    }
    //delete capture object
    capture.release();
}

void processImages(char *fistFrameFilename) {
    //read the first file of the sequence
    frame = imread(fistFrameFilename);
    if (frame.empty()) {
        //error in opening the first image
        cerr << "Unable to open first image frame: " << fistFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
    //current image filename
    string fn(fistFrameFilename);
    //read input data. ESC or 'q' for quitting
    while ((char) keyboard != 'q' && (char) keyboard != 27) {
        //update the background model
        pMOG2->apply(frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("/");
        if (index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0, index + 1);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index + 1, index2 - index - 1);
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        rectangle(frame, cv::Point(10, 2), cv::Point(100, 20),
                  cv::Scalar(255, 255, 255), -1);
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey(30);
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber + 1);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        frame = imread(nextFrameFilename);
        if (frame.empty()) {
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
            exit(EXIT_FAILURE);
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
    }
}