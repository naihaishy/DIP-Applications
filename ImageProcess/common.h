#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QApplication>
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

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);

    static void displayMatOnLabel(const Mat &srcMat, QLabel *label);
signals:

public slots:
};

#endif // COMMON_H
