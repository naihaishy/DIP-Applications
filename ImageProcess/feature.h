#ifndef FEATURE_H
#define FEATURE_H

#include <QDialog>
#include <QPainter>
#include <QLabel>

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace std;
using namespace cv;



/******************************* SIFT *****************************/
namespace NaieFeature {
class SIFT: public QObject
{
    Q_OBJECT
public:
    SIFT(const vector<Mat> &matVec);
    SIFT(const Mat &_queryImg, const Mat &_trainImg);
    void doing();
protected:
    void detect();
    void showControPanel();

private slots:
    void EdgeThresholdDynamicChangeResultMat(int threshold);
    void FeatureNumDynamicChangeResultMat(int num);
    void SigmaDynamicChangeResultMat(int sigma);
private:

    Mat queryImg;
    Mat trainImg;
    Mat dstMat;

    int m_thresh = 10;
    int m_nfeatures = 50;
    double m_sigma = 1.6;

    QLabel *m_ResultLabel;
    QLabel *m_ResultProcessInfoLabel;
};
}



/******************************* Harris Corner *****************************/
namespace NaieFeature {
class HarrisCorner: public QObject
{
    Q_OBJECT

public:
    HarrisCorner(const Mat &mat);
    void doing();
protected:
    void detect();
    void showControPanel();
private slots:

    void KDynamicChangeResultMat(int k);
    void BlockSizeDynamicChangeResultMat(int blocksize);
    void ThresholdDynamicChangeResultMat(int threshold);

private:

    Mat srcMat;
    Mat srcGrayMat;
    Mat dstMat;

    int m_thresh = 80;
    int m_blockSize = 2;
    int m_apertureSize = 3;
    double m_k = 0.04;

    QLabel *m_ResultLabel;

};

}



#endif // FEATURE_H
