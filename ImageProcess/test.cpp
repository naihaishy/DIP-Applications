#include "test.h"





Test::Test(Mat &mat)
{
    mat.copyTo(this->m_SrcMat);
}


void Test::Show()
{

    Point2f center( (float)m_SrcMat.cols / 2, (float)m_SrcMat.rows / 2 );
    double M = 70;
    Mat temp;
    logPolar(m_SrcMat, temp, center, M, INTER_LINEAR + WARP_FILL_OUTLIERS);


    imshow("aaa", temp);

}


void Test::Detectss()
{
 
	LSSD lssd(m_SrcMat);
    lssd.doTesting();

}
