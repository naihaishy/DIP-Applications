#ifndef CHECKER_H
#define CHECKER_H

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\tracking\tracker.hpp>
#include <opencv2\objdetect.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>

#include "Libs/kcf/kcftracker.hpp"
#include "opencv2/videoio.hpp"

using namespace cv;
using namespace std;



class Checker
{
public:
    Checker();
    Checker(string _videopath);
    int doing();

private:
    string videopath;
};

#endif // CHECKER_H
