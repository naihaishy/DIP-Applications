#ifndef DEHAZE_H
#define DEHAZE_H


#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QSpinBox>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ximgproc.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

//定义结构体
struct DarkChannelMost
{
    uchar pixel;//像素值
    int row;//所在行
    int col;//所在列
    //构造函数
    DarkChannelMost(uchar p, int i, int j) {
        pixel = p;
        row = i;
        col = j;
    }
    //重载运算符
    bool operator >(const DarkChannelMost &drak)const   //降序排序
    {
        return pixel > drak.pixel;
    }
    bool operator <(const DarkChannelMost &drak)const   //升序排序
    {
        return pixel < drak.pixel;
    }

};


class Dehaze : public QObject
{
    Q_OBJECT

public:
 
    Dehaze(Mat &mat, int ksize=15, double wight=0.95);

	Mat getDarkChannelImage();
	Mat getTransmissionMap();
	Mat getOriginScene();
	bool doDehazing();//执行去雾操作
    void ShowControlPanel();

protected:
    void CalDarkChannel();
    void CalAtmosphericLight();
    void CalTransmission();
    void CalOriginScene();

private slots:
    void WDynamicChangeResultMat(int w);
    void KsizeDynamicChangeResultMat(int ksize);
    void ImageShowChanged(int index);
    void TransmissionGenerateMethodChanged(int index);
    void DrakMinOrderChanged(int index);
    void TranMinOrderChanged(int index);
    void GuideFilterEpsChanged(QString string);
    void GuideFilterRadiusChanged(QString string);
private:

	Mat m_SrcMat;//原始带雾图像 保存不变
	Mat m_DarkChannelMat;//暗通道图像
	int m_AtmosphericLight[3];//大气亮度
	Mat m_TransmissionMat;//传导图像
	Mat m_OriginScene;
	int m_Ksize;//窗口大小
    int m_Rows;
    int m_Cols;
    double m_Wight;//权重
    int m_SelectShowIndex;
    int m_TranGenMethod;//传导图像生成方法
    int m_DarkMinOrder;//暗通道最小滤波的顺序
    int m_TranMinOrder;//传导图像最小滤波的顺序
    int m_GuideFilterRadius;
    double m_GuideFilterEps;

    QLabel *m_ResultLabel;


	void DarkChannelImage();
	void DrakChannelImageExchangeMin();
	void AtmosphericLight();
	void EstimatingTransmission();
    void EstimatingTransmissionExchangeMin();
	void OriginScene();

    void display(const Mat &srcMat, QLabel *label);
    void MinFilter(Mat &inputMat, Mat &outputMat);
    void minimumPixelInChannles(Mat &inputMat, Mat &outputMat);
    void NormalizeSrcMat(Mat &inputMat, Mat &outputMat, int *light);
    void FindMostLightPostion(Mat &inputMat, vector<Point> &output, double rate);
    void ShowImageBySelectIndex(int index);
    void RefineTransmission(Mat &inputMat, Mat &outputMat);
};

#endif // DEHAZE_H
