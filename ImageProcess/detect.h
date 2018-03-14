#ifndef DETECT_H
#define DETECT_H



#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "lssd.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include<sstream>

using namespace std;
using namespace cv;


class Detect
{
public:
    Detect(vector<Mat> &mats);
    void doDetecting();



private:

    vector<Mat> m_InputMats;
    vector<LSSD> m_Lssds;

    int m_k;//输入图像个数
    int m_w;
    int m_h;
    int m_Margin;

    vector<vector<Point>> m_MaxMatchPointsVec;//best match points
    vector<vector<double>> m_MaxMatchValuesVec;//best match values


	vector<vector<vector<double>>> m_AllMatchesValuesVec;//所有match
	vector<vector<vector<Point>>> m_AllMatchesPointsVec;//所有match



    void InitLSSD();


    double CalcSimBetweenTwoDescriptor(const LSSDescriptor &d1, const LSSDescriptor &d2);
    //double CalcSignifiance(const vector<double> &matchValues);
    vector<Point> &FindCommonRegion();
    void SelectInterest();
    void DrawCentralPoint(vector<Point> &points);
	//void SelectBasedOnRect(Rect region, vector<vector<double>>& allMatchesValuesVec, vector<vector<Point>>& allMatchesPointsVec);
    void SelectBasedOnRect(Rect region);
	void FindAllMatchesInAllImages(Rect sketchRegion, vector<vector<Point>>& allmatchPoints, vector<vector<double>>& allmatchValues);
	//void FindBestMatchInAllImages(Rect sketchRegion, vector<Point> &matchPoints, vector<double> &matchValues);
	void CalcAllMatches(Rect sketchRegion, int indexK, vector<double>& matchValue, vector<Point>& matchPoint);
	//void CalcBestMatch(Rect sketchRegion, int indexK, Point matchPoint, double matchValue);
    double CalcMatchBetweenTwoRegion(Rect rect1, Rect rect2, int indexK);
	double CalcSignifiance(int indexK);
};

#endif // DETECT_H
