#ifndef STEREOMATCH_H
#define STEREOMATCH_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>


#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;


enum { STEREO_BM = 0, STEREO_SGBM = 1, STEREO_HH = 2, STEREO_3WAY = 3, STEREO_HH4 = 4};

namespace Ui {
class StereoMatch;
}

class StereoMatch : public QDialog
{
    Q_OBJECT

public:
    explicit StereoMatch(QWidget *parent = 0);
    ~StereoMatch();

    void doing();
protected:

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::StereoMatch *ui;
    Mat leftImg;
    Mat rightImg;
    Mat dstMat;
    int SadWindowSize;
    int DisparityNum;
    int AlgorithmMode;//0-BM 1-SGBM 2-HH 3-3WAY 4-HH4

    bool isActived;

    Ptr<StereoBM> bm;
    Ptr<StereoSGBM> sgbm;

    void InitStatus();
    void updateTime();
    void InitStereoModel();
    void SGBMSetParamaters(Ptr<StereoSGBM> &stereoSgbm, int &SADWindowSize, int mode, int &numberOfDisparities);
    void BMSetParamaters(Ptr<StereoBM> &stereoBm, int &SADWindowSize, int &numberOfDisparities);

};


namespace Naie {

//自定义的StereoBM
class StereoBM: public StereoMatcher{

public:
    int getNumDisparities() const { return this->numDisparities; }
    void setNumDisparities(int numDisparities) { this->numDisparities = numDisparities; }
    int getBlockSize() const { return this->SADWindowSize; }
    void setBlockSize(int blockSize) { this->SADWindowSize = blockSize; }
private:
    int SADWindowSize;
    int numDisparities;
};
}

#endif // STEREOMATCH_H
