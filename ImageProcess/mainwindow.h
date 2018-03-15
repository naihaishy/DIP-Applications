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
#include <QListWidget>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QtMultimedia>
#include <QtMultimediaWidgets>


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
using namespace cv;
using namespace std;

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
    void on_actionDetecting_triggered();
    void on_actionSketching_triggered();
    void on_actionDetect_Control_Panel_triggered();
    void on_actionOpen_Multi_triggered();


    void on_actionFrame_Diff_triggered();

    void on_actionSGM_triggered();

    void on_actionGMM_triggered();

    void on_actionOpen_Video_triggered();

private:
    Ui::MainWindow *ui;


    QString CurrentFile;

    // 状态栏
    QLabel *statusLabel;

    //图像数据
    Mat m_SrcImage;
    Mat m_EffectImage;
    vector<Mat> m_SrcImageVec;

    QString m_SrcVideoPath;//路径

    bool loadImage(const QString fileName);
    bool loadImage(const QStringList fileNames);
    bool saveImage();
    void display(Mat &mat);
    void display(vector<Mat> &matVec, QStringList fileNames);

    void imageValidInfo();
    void InitMenu();


    void playVideo(QString videoPath);
};

#endif // MAINWINDOW_H
