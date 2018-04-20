#include "common.h"

Common::Common(QObject *parent) : QObject(parent)
{

}

/***在Label上显示图像***/
void Common::displayMatOnLabel(const Mat &srcMat, QLabel *label)
{
    Mat tempMat;
    QImage img;
    imwrite("sss.png", srcMat);
    if(srcMat.channels()==3)
    {
        cvtColor(srcMat, tempMat, CV_BGR2RGB);
        img = QImage((const uchar*)(tempMat.data), tempMat.cols, tempMat.rows, tempMat.cols*tempMat.channels(), QImage::Format_RGB888);
    }
    else
    {
        img = QImage((const uchar*)(srcMat.data), srcMat.cols, srcMat.rows, srcMat.cols*srcMat.channels(), QImage::Format_Indexed8);
    }
    label->setPixmap(QPixmap::fromImage(img));
    label->resize(label->pixmap()->size());
    label->show();
}

