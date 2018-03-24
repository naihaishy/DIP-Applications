#ifndef TRACKER_H
#define TRACKER_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2\video\tracking.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>


using namespace std;
using namespace cv;




class Tracker
{
public:
    Tracker();
	 

	void CamShiftTracker(string videoFile);
	


 
	
private:
     
	 
	

    Rect selection;
    bool selectObject;
    Point origin;
    Mat frame, srcMat;
 
    bool pause;
	int trackObject;
 
	int vmin = 10, vmax = 256, smin = 30;
	
	

	static void mouseHandler(int event, int x, int y, int, void * userdata);
	void onMouse(int event, int x, int y);
	void ShowHist(const Mat & dstHist);

};

#endif // TRACKER_H
