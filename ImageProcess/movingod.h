#ifndef MOVINGOD_H
#define MOVINGOD_H


#include <QObject>
#include <QMessageBox>


#include <opencv2/core/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include<sstream>

using namespace std;
using namespace cv;


class MovingOD
{
public:
    MovingOD();
    static void GMMDetection(string videoPath);
    static void FrameDiffDetection(string videoPath);
    static void SGMDetection(string videoPath);
    static void SGMTest(string videoPath);
};

#endif // MOVINGOD_H
