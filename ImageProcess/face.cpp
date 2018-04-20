#include "face.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>



namespace Naie {

Face::Face()
{
	LBP_Para_Radius = 1;
	LBP_Para_Neighbors = 8;
	LBP_Para_GridX = 8;
	LBP_Para_GridY = 8;
}


Face::~Face()
{
}

void Face::doing()
{
	if (algorithm == 0) {
		//opencv
		FaceOpenCV();
	}
	else {
		//custom
		//FaceCustom();
		FaceOpenCV();
	}
}

void Face::setAlgorithm(int _al)
{
	algorithm = _al;
}

double Face::getRecognitionRate() const
{
	return recognitionRate;
}

vector<string> Face::getLog() const
{
    return predictLogs;
}

void Face::setPara(int r, int n, int gx, int gy)
{
	LBP_Para_Radius=r;
	LBP_Para_Neighbors=n;
	LBP_Para_GridX=gx;
	LBP_Para_GridY=gy;
}


static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
	std::ifstream file(filename.c_str(), ifstream::in);
	if (!file) {
		string error_message = "No valid input file was given, please check the given filename.";
		CV_Error(CV_StsBadArg, error_message);
	}
	string line, path, classlabel;
	while (getline(file, line)) {
		stringstream liness(line);
		getline(liness, path, separator);
		getline(liness, classlabel);
		if (!path.empty() && !classlabel.empty()) {
			images.push_back(imread(path, 0));
			labels.push_back(atoi(classlabel.c_str()));
		}
	}
}

void Face::FaceOpenCV()
{
	string fn_csv = string("facerec_at_t.txt");
	// These vectors hold the images and corresponding labels.
	vector<Mat> images;
	vector<int> labels;
	// Read in the data. This can fail if no valid
	// input filename is given.
	try {
		read_csv(fn_csv, images, labels);
	}
	catch (cv::Exception& e) {
		cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
		// nothing more we can do
		exit(1);
	}

	std::cout << images.size() << " images loaded\n";

	// Quit if there are not enough images for this demo.
	if (images.size() <= 1) {
		string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
		CV_Error(CV_StsError, error_message);
	}
 
	//build sets
	buildTraingTestSet(images, labels);
 
	//traning
	Ptr<face::LBPHFaceRecognizer> model = face::LBPHFaceRecognizer::create(LBP_Para_Radius, LBP_Para_Neighbors, LBP_Para_GridX, LBP_Para_GridY, DBL_MAX);
	model->train(traingImages, traingLabels);

	//predict
	vector<int> predictedLabels(testImages.size());
	vector<double> predictedConfidences(testImages.size());
    for (int i = 0; i < testImages.size(); i++)
	{
		model->predict(testImages[i], predictedLabels[i], predictedConfidences[i]);
		string result_message = format("Predicted class = %d / Actual class = %d.", predictedLabels[i], testLabels[i]);
		predictLogs.push_back(result_message);
	}
 
	//calculate recognitionRate
	recognitionRate = calcRecognitionRate(predictedLabels, testLabels);
}

void Face::FaceCustom()
{
}


void Face::buildTraingTestSet(vector<Mat>& images, vector<int>& labels)
{

	// 
    for (int i = 0; i < images.size(); i+=10)
	{
		int cat = (i / 10)*10;
        for (int j = 0; j < 5; ++j)
		{
			int select = cat + j;//  
			testImages.push_back(images[select]);
			testLabels.push_back(labels[select]);
		}

        for (int k = 5; k < 10; ++k)
		{
			int select = cat + k;//  
			traingImages.push_back(images[select]);
			traingLabels.push_back(labels[select]);
		}
 

	}

  
}

double Face::calcRecognitionRate(vector<int> &predictedLabels, vector<int> &realLabels)
{
	CV_Assert(predictedLabels.size()== realLabels.size());

	int size = realLabels.size();
	int count = 0;
    for (int i = 0; i < predictedLabels.size(); i++)
	{
		if (predictedLabels[i] == realLabels[i])
		{
			count++;
		}
	}

	return ((double)count)/size;
}

//calcluate lbp 
void Face::calcLBP(Mat & mat, Mat & lbp)
{
	for (int i = 0;i < mat.rows;i++) 
	{
		uchar * pCols = mat.ptr<uchar>(i);
		for (int j = 0;j < mat.cols;j++) 
		{
			uchar cc = pCols[j];//current pixel
			//surroudings
			int diff[8];
			if (mat.at<uchar>(i - 1, j - 1) < cc)
				diff[0] = 0;
			else
				diff[0] = 1;

			if (mat.at<uchar>(i - 1, j ) < cc)
				diff[1] = 0;
			else
				diff[1] = 1;

			if (mat.at<uchar>(i - 1, j + 1) < cc)
				diff[2] = 0;
			else
				diff[2] = 1;

			if (mat.at<uchar>(i , j + 1) < cc)
				diff[3] = 0;
			else
				diff[3] = 1;

			if (mat.at<uchar>(i + 1, j + 1) < cc)
				diff[4] = 0;
			else
				diff[4] = 1;

			if (mat.at<uchar>(i + 1, j ) < cc)
				diff[5] = 0;
			else
				diff[5] = 1;

			if (mat.at<uchar>(i + 1, j - 1) < cc)
				diff[6] = 0;
			else
				diff[6] = 1;

			if (mat.at<uchar>(i , j - 1) < cc)
				diff[7] = 0;
			else
				diff[7] = 1;

			//ת��Ϊ10����
			lbp.at<uchar>(i, j) = diff[0] * 1 + diff[1] * 2 + diff[2] * 4 + diff[3] * 8 + diff[4] * 16 + diff[5] * 32 + diff[6] * 64 + diff[7] * 128;
		}
	}
}
 




}




