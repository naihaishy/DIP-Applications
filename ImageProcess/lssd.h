#ifndef LSSD_H
#define LSSD_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <vector>
#include <algorithm>


using namespace std;
using namespace cv;


struct LSSDescriptor
{
    int row;
    int col;
    vector<double> value;
    LSSDescriptor(int i, int j, vector<double> v):row(i),col(j), value(v) {}
};


class LSSD
{
public:
    LSSD(const Mat &mat);

    vector<LSSDescriptor> m_Descriptors;//ensemble of descriptors of whole image

    static double SimilarityBetweenTwoDescriptor(const LSSDescriptor &lssd1, const LSSDescriptor &lssd2);
    static int getMargin();
    void doTesting();

	void doing();


private:
    Mat m_SrcMat;//
    int m_Rows;
    int m_Cols;

    int m_PatchSize;//Patch大小 5x5
    int m_PatchRadius;//patch radius

    int m_RegionSize;//40x2+1
    int m_RegionRadius;//Image Region 大小  radius 40
    int m_Margin;
    int m_Angles;//20 angles
    int m_Radials;//4 radial intervals




    void CalcSelfSimilarityDescriptor();
    void CalcSSDofSepcifiedPixel(int row, int col, Mat & ssdMat);
	int CalSSDBetweenTwoMats(const Mat &mat1, const Mat &mat2);
    int CalcVarAutoofOnePixel(int row, int col, const Mat &srcMat, int patchSize = 1);
    void TranIntoLogPolar(const Mat & inputMat, Mat & outputMat, Mat & inverseMat);
    void TranSSDIntoCorrelationSurface(int row, int col, Mat &ssdMat, Mat &corrMat);
    void CalcDescriptorofSepcifiedPixel(int row, int col, vector<double> &desc);
    void PartBins(const Mat &inputMat, int angles, int radial, vector<double> &bins);
    void RemoveNonInformativeDescriptor(vector<LSSDescriptor> &inputDesc, vector<LSSDescriptor> &outputDesc);
    void NormaliseDescriptor(vector<LSSDescriptor> &inputDesc, vector<LSSDescriptor> &outputDesc);
    void CalcDescriptorofSepcifiedMat(vector<LSSDescriptor> &descriptors);
};

#endif // LSSD_H
