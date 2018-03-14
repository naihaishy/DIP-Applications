#ifndef TEST_H
#define TEST_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "detect.h"
#include "lssd.h"


using namespace std;
using namespace cv;

class Test
{
public:
    Test(Mat &mat);

    void Show();
    void Detectss();

private:
    Mat m_SrcMat;


};

#endif // TEST_H
