#pragma once

#include <opencv2\core.hpp>
#include <opencv2\face\facerec.hpp>
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
using namespace std;


namespace Naie {
class Face
{
public:
	Face();
	~Face();
	void doing();
	void setAlgorithm(int _al);
	double getRecognitionRate() const;
	vector<string> getLog() const;
    void setPara(int r, int n, int gx,int gy);



private:
	void FaceOpenCV();
	void FaceCustom();

	void buildTraingTestSet(vector<Mat>& images, vector<int>& labels);
	double calcRecognitionRate(vector<int> &predictedLabels, vector<int> &realLabels);

	void calcLBP(Mat &mat, Mat &lbp);


	vector<Mat> traingImages;
	vector<int> traingLabels;
	vector<Mat> testImages;
	vector<int> testLabels;

	vector<string> predictLogs;

	int algorithm;

	double recognitionRate;

    int LBP_Para_Radius;
    int LBP_Para_Neighbors;
    int LBP_Para_GridX;
    int LBP_Para_GridY;
};

}



