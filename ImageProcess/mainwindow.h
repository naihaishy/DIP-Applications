#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QCloseEvent>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include <QTextCursor>
#include <QDebug>
#include <QTextEdit>
#include <QPainter>
#include <QTextBrowser>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionExit_triggered();

    void on_actionDark_Channel_Image_triggered();

    void on_actionTransmission_Image_triggered();

    void on_actionOrigial_Scene_triggered();


    void on_actionDeHaze_Control_Panel_triggered();

    void on_actionAbout_triggered();

    void on_actionCompile_Guide_triggered();

private:
    Ui::MainWindow *ui;


    QString CurrentFile;

    // 状态栏
    QLabel *statusLabel;

    //图像数据
    Mat m_SrcImage;
    Mat m_EffectImage;

    bool loadImage(const QString fileName);
    bool saveImage();
    void display(Mat mat);

    void imageValidInfo();
    void InitMenu();
};

#endif // MAINWINDOW_H
