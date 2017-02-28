#include "common/support.h"

#include <vector>
#include <boost/range.hpp>

using namespace std;


string getRandString(const int len) {
  string randString;
  static const char charIdx[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < len; ++i) {
    randString.push_back(charIdx[rand() % (sizeof(charIdx) - 1)]);
  }
  return randString;
}

string getOccurredTime(void) {
  time_t rawTime;
  struct tm *currentDateTime;
  char buffer[80];

  time(&rawTime);
  currentDateTime = localtime(&rawTime);

  strftime(buffer, 80, "%Y-%m-%d %X", currentDateTime);

  string occurredTime = string(buffer);

  return occurredTime;
}

string createSavePath(string saveDir) {
  string fileName;
  string timeStamp;
  time_t rawTime;
  struct tm *currentDateTime;
  char buffer[16];

  string randName = getRandString(6);

  time(&rawTime);
  currentDateTime = localtime(&rawTime);
  strftime(buffer, 16, "%d%m%Y-%H%M%S", currentDateTime);

  timeStamp = string(buffer);

  fileName = saveDir[saveDir.length() - 1] == '/' ? saveDir : saveDir + "/";
  fileName = fileName + timeStamp + "-" + randName + ".jpg";

  return fileName;
}
