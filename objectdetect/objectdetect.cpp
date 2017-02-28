//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
//C++
#include <iostream>
#include <common/support.h>

using namespace cv;
using namespace std;

// Global variables
Mat currFrame; //current currFrame
Mat bgModelFrame; //Background model currFrame

void help();

template<class T>
void processVideo(T vidSource);

// Initialize settings
class Config{
 protected:
 public:
  int threshold, maxVal;
  boost::property_tree::ptree pt;
  bool readConf(string configPath){
    try {
      boost::property_tree::ini_parser::read_ini(configPath, pt);
    } catch (exception) {
      cerr << "ERROR while opening conf file" << endl;
      return 1;
    }
    return 0;
  }
};

Config conf;

int main(int argc, char *argv[]) {
  //check for the input parameter correctness
  if (argc != 4) {
    cerr << "Incorrect input list" << endl;
    // Print help information
    help();
    return EXIT_FAILURE;
  }

  conf.readConf(argv[1]);

  // Load settings from conf file.
  conf.threshold = conf.pt.get<int>("BS_SETTING.THRESHOLD");
  conf.maxVal = conf.pt.get<int>("BS_SETTING.MAXVALUE");

  //create GUI windows
  namedWindow("Current Frame");
  if (strcmp(argv[2], "-vid") == 0) {
    string videoPath = argv[3];
    processVideo(videoPath);
  } else if (strcmp(argv[2], "-cam") == 0) {
    int camId = atoi(argv[3]);
    processVideo(camId);
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

void help() {
  cout
      << "--------------------------------------------------------------------------" << endl
      << "Usage:" << endl
      << "./objectdetect <config_path> {-vid <video_filename>|-cam <camera_id>}" << endl
      << "for example: " << endl
      << "./objectdetect /conf.ini -vid video.avi" << endl
      << "./objectdetect /conf.ini -cam 0" << endl
      << "--------------------------------------------------------------------------" << endl
      << endl;
}

template<class T>
void processVideo(T vidSource) {
  //create the capture object
  VideoCapture capture(vidSource);

  if (!capture.isOpened()) {
    //error in opening the video input
    cerr << "Unable to open" << endl;
    exit(EXIT_FAILURE);
  }

  if (!capture.read(currFrame)){
    //error in opening the video input
    cerr << "Error while reading frame: " << endl;
    exit(EXIT_FAILURE);
  }
  //read input data. ESC or 'q' for quitting
  Mat diffFrame,grayMat, threshFrame,thf;
  bgModelFrame = currFrame.clone();
  int keyPressed = 0;

  while (!currFrame.empty()) {
    imshow("Current Frame", currFrame);

    absdiff(bgModelFrame,currFrame,diffFrame); // Absolute differences between the 2 images
    threshold(diffFrame,threshFrame,conf.threshold,conf.maxVal,CV_THRESH_BINARY); // set threshold to ignore small differences you can also use inrange function
    medianBlur(threshFrame,threshFrame,5);
    imshow("BG",diffFrame);
    imshow("TH",threshFrame);

    //get input from keyboard
    keyPressed = waitKey(30);
    if ((char) keyPressed == KEY_ESC) {
      break;
    } else if ((char) keyPressed == KEY_N){
      bgModelFrame = currFrame.clone();
      imshow ("Background Model",bgModelFrame);
    }

    capture >> currFrame; // capture next currFrame
  }
  //delete capture object
  capture.release();
}