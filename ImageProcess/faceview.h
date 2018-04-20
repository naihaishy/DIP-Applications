#ifndef FACEVIEW_H
#define FACEVIEW_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>


#include "face.h"

namespace Ui {
class FaceView;
}

class FaceView : public QDialog
{
    Q_OBJECT

public:
    explicit FaceView(QWidget *parent = 0);
    ~FaceView();

private slots:



    void on_startButton_clicked();

    void onAutoScroll();
private:
    Ui::FaceView *ui;

    Naie::Face *face;

    int LBP_Algorithm;//LBP算法 OPenCV 或者Custom
    int LBP_Para_Radius;
    int LBP_Para_Neighbors;
    int LBP_Para_GridX;
    int LBP_Para_GridY;


    void InitStatus();
};

#endif // FACEVIEW_H
