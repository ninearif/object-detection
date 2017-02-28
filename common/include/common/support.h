#ifndef OBJECT_DETECT_SUPPORT_H
#define OBJECT_DETECT_SUPPORT_H

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#if __linux__
#define KEY_UP      82
#define KEY_DOWN    84
#define KEY_RIGHT   83
#define KEY_LEFT    81
#define KEY_ENTER   13
#define KEY_ESC     27
#define KEY_N       110
#define KEY_R       114
#define KEY_S       115
#else
#define KEY_UP      2490368
#define KEY_DOWN    2621440
#define KEY_RIGHT   2555904
#define KEY_LEFT    2424832
#define KEY_ENTER   13
#endif

std::string getRandString(const int len);

std::string getOccurredTime(void);

std::string createSavePath(std::string saveDir);
#endif //OBJECT_DETECT_SUPPORT_H
