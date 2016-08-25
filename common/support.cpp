#include "common/support.h"

#include <vector>
#include <boost/range.hpp>

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;

vector<fs::path> filterImages(string inputDir) {
    vector<fs::path> files;
    for (auto &it : boost::make_iterator_range(fs::directory_iterator(inputDir), {})) {
        fs::path path = it.path();
        fs::path ext = path.filename().extension();
        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
            files.push_back(path);
        } else {
            continue;
        }
    }

    return files;
}
