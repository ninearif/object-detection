#ifndef OBJECT_DETECT_SUPPORT_H
#define OBJECT_DETECT_SUPPORT_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#if __linux__
#define KEY_UP      1113938
#define KEY_DOWN    1113940
#define KEY_RIGHT   1113939
#define KEY_LEFT    1113937
#define KEY_ENTER   1048586
#else
#define KEY_UP      2490368
#define KEY_DOWN    2621440
#define KEY_RIGHT   2555904
#define KEY_LEFT    2424832
#define KEY_ENTER   13
#endif

namespace fs = boost::filesystem;

std::vector<fs::path> filterImages(std::string inputDir);

#endif //OBJECT_DETECT_SUPPORT_H
