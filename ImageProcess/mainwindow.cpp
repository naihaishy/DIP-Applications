#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dehaze.h"
#include "detect.h"
#include "test.h"
#include "movingod.h"
#include "tracker.h"
#include "feature.h"
#include "faceview.h"
#include "stereomatch.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/***初始化菜单***/
void MainWindow::InitMenu()
{
    ui->menuDehaze->setEnabled(false);
    ui->menuDetect->setEnabled(false);

    // 正常状态信息
    statusLabel = new QLabel();
    statusLabel->setMinimumSize(150,20);
    statusLabel->setFrameShape(QFrame::WinPanel);
    statusLabel->setFrameShadow(QFrame::Sunken);
    statusLabel->setText(tr("Hello"));
    ui->statusBar->addWidget(statusLabel);

}

/***加载图像***/
bool MainWindow::loadImage(const QString fileName)
{
    //读取图像数据
    m_SrcImage = cv::imread(fileName.toLocal8Bit().toStdString());
    // 判断非空
    if(m_SrcImage.empty()){
        QMessageBox::warning(this, tr("Image Process"), tr("Image data is empty"));
        return false;
    }
    //Qt显示图像
    display(m_SrcImage);
    // 更新菜单状态
    ui->menuDehaze->setEnabled(true);
    // 设置当前文件
    CurrentFile = QFileInfo(fileName).canonicalFilePath();
    // 设置窗口标题
    setWindowTitle(CurrentFile);
    return true;
}

/***加载图像***/
bool MainWindow::loadImage(const QStringList fileNames)
{
    //读取图像数据
    std::for_each (fileNames.begin(), fileNames.end(), [&](const QString& item){
        Mat mat = cv::imread(item.toLocal8Bit().toStdString());
        if(mat.empty()){
            QMessageBox::warning(this, tr("Image Process"), tr("Image data is empty"));
            return false;
        }
        m_SrcImageVec.push_back(mat);
    });
    //Qt显示图像
    display(m_SrcImageVec, fileNames);
    // 更新菜单状态
    ui->menuDetect->setEnabled(true);

    // 设置当前文件
    CurrentFile = tr("Multi Files");
    // 设置窗口标题
    setWindowTitle(CurrentFile);
    return true;
}


/***显示图像***/
void MainWindow::display(Mat &mat)
{
    Mat rgb;//opencv图像数据类型
    QImage img;
    if(mat.channels()==3)
    {
        cvtColor(mat, rgb, CV_BGR2RGB);
        img = QImage((const uchar*)(rgb.data), rgb.cols, rgb.rows, rgb.cols*rgb.channels(), QImage::Format_RGB888);
    }
    else
    {
        img = QImage((const uchar*)(mat.data), mat.cols, mat.rows, mat.cols*mat.channels(), QImage::Format_Indexed8);
    }
    ui->imageLabel->setPixmap(QPixmap::fromImage(img));
    ui->imageLabel->resize(ui->imageLabel->pixmap()->size());
    ui->imageLabel->show();
}

/***显示图像***/
void MainWindow::display(vector<Mat> &matVec, QStringList fileNames)
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(QObject::tr("Multi Images"));
    dlg->resize(QSize(800,600));
    dlg->setFont(QFont("Microsoft Yahei"));

    QGridLayout *mainLayout = new QGridLayout(dlg);
    QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i< matVec.size(); ++i) {
        Mat rgb;
        QImage img;
        Mat mat = matVec[i];
        if(mat.channels()==3)
        {
            cvtColor(mat, rgb, CV_BGR2RGB);
            img = QImage((const uchar*)(rgb.data), rgb.cols, rgb.rows, rgb.cols*rgb.channels(), QImage::Format_RGB888);
        }
        else
        {
            img = QImage((const uchar*)(mat.data), mat.cols, mat.rows, mat.cols*mat.channels(), QImage::Format_Indexed8);
        }

        QLabel *label = new QLabel();

        label->setPixmap(QPixmap::fromImage(img));
        label->resize(label->pixmap()->size());
        layout->addWidget(label);
    }
    layout->setMargin(3);
    mainLayout->addLayout(layout, 0,0);

    dlg->show();
}




/***保存效果图像***/
bool MainWindow::saveImage()
{

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), tr("result.png"), tr("Images (*.png *.bmp *.jpg)"));
    if(fileName.isEmpty())
    {
        return false;
    }
    else
    {
        // 效果图像是否存在
        if(m_EffectImage.empty())
        {
            QMessageBox::warning(this, tr("Image Process"), tr("Effetc image data is empty\nSave failed!"));
            return false;
        }

        // 创建file对象
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(this, tr("Image Process"), tr("Cant't write file %1：/n %2").arg(fileName).arg(file.errorString()));
            return false;
        }
        // 写入到效果图像
        imwrite(fileName.toStdString(), m_EffectImage);

        file.close();
        return true;
    }
}

/***图像有效性提醒***/
void MainWindow::imageValidInfo()
{
    if(m_SrcImage.empty())
    {
        QMessageBox::warning(this, tr("Image Process"), tr("Image data is empty\nPlease load image"));
        return;
    }
}

/***SLOT:打开图像***/
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), tr(""), tr("Image (*.png *.bmp *.jpg)") );

    if(!fileName.isEmpty())
    {
        loadImage(fileName);//加载图像
    }
}

/***SLOT:打开多个文件***/
void MainWindow::on_actionOpen_Multi_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Image"), tr(""), tr("Image (*.png *.bmp *.jpg)") );

    if(!fileNames.isEmpty())
    {
        loadImage(fileNames);//加载图像
    }
}

void MainWindow::on_actionSave_triggered()
{
    saveImage();
}


void MainWindow::on_actionExit_triggered()
{
    qApp->quit();// qApp是指向应用程序的全局指针
}

void MainWindow::on_actionAbout_triggered()
{
    //创建dialog对象
    QDialog *aboutDlg = new QDialog(this);
    aboutDlg->setWindowTitle(tr("About Image Process"));
    aboutDlg->resize(QSize(500,300));
    //dialog的主布局
    QGridLayout *mainLayout = new QGridLayout(aboutDlg);


    // 软件介绍文字
    QHBoxLayout *layout = new QHBoxLayout(aboutDlg);
    QTextEdit *text = new QTextEdit(aboutDlg);
    text->setReadOnly(true);
    text->setFont(QFont("Microsoft Yahei"));
    text->setText(tr("Image Process Base on Qt and OpenCV"));

    //about qt
    QPushButton *btn = new QPushButton(tr("About Qt"), aboutDlg);
    connect(btn, SIGNAL(clicked(bool)), qApp, SLOT(aboutQt()));

    layout->addWidget(text);
    layout->addWidget(btn);

    mainLayout->addLayout(layout, 0, 0, 2, 5);

    aboutDlg->exec();

}

void MainWindow::on_actionCompile_Guide_triggered()
{
    //创建dialog对象
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("About Image Process"));
    dlg->resize(QSize(500,300));
    //dialog的主布局
    QGridLayout *mainLayout = new QGridLayout(dlg);


    // 软件介绍文字
    QHBoxLayout *layout = new QHBoxLayout();
    QTextBrowser *broswer = new QTextBrowser();

    broswer->setFont(QFont("Microsoft Yahei"));
    broswer->setOpenExternalLinks(true);
    //从compile.txt读取软件编译指南内容

    QFile file(QString(":/help/compile.txt")); // 新建QFile对象
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
       QMessageBox::warning(this, tr("Image Process"), tr("Can't read compile.txt"));
       return ;
    }
    broswer->setHtml(file.readAll());

    layout->addWidget(broswer);


    mainLayout->addLayout(layout, 0, 0, 2, 5);

    dlg->exec();

}



/******作业1: 基于暗通道先验的图像去雾******/

/***SLOT:暗通道图像***/
void MainWindow::on_actionDark_Channel_Image_triggered()
{
    Dehaze dehaze(m_SrcImage);
	dehaze.doDehazing();
	Mat mat = dehaze.getDarkChannelImage();
    imshow("Drak Channel", mat);
}

/***SLOT:传导图像***/
void MainWindow::on_actionTransmission_Image_triggered()
{
    Dehaze dehaze(m_SrcImage);
	dehaze.doDehazing();
	Mat mat = dehaze.getTransmissionMap();
    imshow("Transmission", mat);
}

/***SLOT:原始场景图像***/
void MainWindow::on_actionOrigial_Scene_triggered()
{
    Dehaze dehaze(m_SrcImage);
	dehaze.doDehazing();
    Mat mat = dehaze.getOriginScene();
    imshow("Origin Scene", mat);
}
/***SLOT:图像去雾操作总参数控制面板***/
void MainWindow::on_actionDeHaze_Control_Panel_triggered()
{
    Dehaze dehaze(m_SrcImage);
    dehaze.doDehazing();
    dehaze.ShowControlPanel();
}



/******作业2: detectint and sketching the common******/
void MainWindow::on_actionDetecting_triggered()
{
    statusLabel->setText(tr("Process is running!"));

    QMessageBox::information(this, tr("Process takes so long"), tr("Please wait patiently due to the Intensive Computation\n"));

    Detect detect(m_SrcImageVec);
    detect.doDetecting();

    statusLabel->setText(tr("Hello"));

}

void MainWindow::on_actionSketching_triggered()
{

}

void MainWindow::on_actionDetect_Control_Panel_triggered()
{

}



/******作业3: moving object detection******/
void MainWindow::on_actionOpen_Video_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load video"), tr(""), tr("Video (*.mp4 *.avi *.wav)") );

    if(!fileName.isEmpty())
    {
        m_SrcVideoPath = fileName;
        QMessageBox box(this);
        box.setWindowTitle(tr("Video Load Success"));
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("Play Video \r") + CurrentFile + "?");
        QPushButton *yesBtn = box.addButton(tr("Yes"), QMessageBox::YesRole);
        QPushButton *noBtn = box.addButton(tr("No"), QMessageBox::NoRole);
        box.exec();
        QPushButton* clickedButton =(QPushButton*)box.clickedButton();
        if ( clickedButton== yesBtn)
            playVideo(m_SrcVideoPath); // 播放视频
    }
}

//播放视频
void MainWindow::playVideo(QString videoPath)
{
    QMediaPlayer *videoPlayer = new QMediaPlayer;

    videoPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(videoPath)));

    QVideoWidget *videoWidget = new QVideoWidget;

    videoPlayer->setVideoOutput(videoWidget);

    videoWidget->show();
    videoPlayer->play();

}
//帧间差
void MainWindow::on_actionFrame_Diff_triggered()
{
    MovingOD::FrameDiffDetection(m_SrcVideoPath.toStdString());
}

//单高斯模型
void MainWindow::on_actionSGM_triggered()
{
    MovingOD::SGMTest(m_SrcVideoPath.toStdString());

}

//混合高斯模型
void MainWindow::on_actionGMM_triggered()
{
    MovingOD::GMMDetection(m_SrcVideoPath.toStdString());
}



void MainWindow::on_actionCamShift_triggered()
{
    Tracker *tracker = new Tracker();
    QMessageBox box(this);
    box.setWindowTitle(tr("CamShift"));
    box.setIcon(QMessageBox::Warning);
    box.setText(tr("Yes to Select Video\r No to Open Camera"));
    QPushButton *yesBtn = box.addButton(tr("Yes"), QMessageBox::YesRole);
    QPushButton *noBtn = box.addButton(tr("No"), QMessageBox::NoRole);
    box.exec();
    QPushButton* clickedButton =(QPushButton*)box.clickedButton();
    if ( clickedButton== yesBtn)
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load video"), tr(""), tr("Video (*.mp4 *.avi *.wav)") );
        if(!fileName.isEmpty())
        {
            tracker->CamShiftTracker(fileName.toStdString());
        }
    }

    if ( clickedButton== noBtn)
    {
        tracker->CamShiftTracker("");
    }





}

void MainWindow::on_actionSIFT_triggered()
{
    if(m_SrcImageVec.size()!=2)
    {
        QMessageBox::warning(this, tr("Image Process"), tr("Only Two Images are needed\nPlease Reload two images"));
        return ;
    }
    NaieFeature::SIFT *sift = new NaieFeature::SIFT(m_SrcImageVec);
    sift->doing();

}

void MainWindow::on_actionHarris_Corner_triggered()
{
    if(m_SrcImage.empty())
    {
        QMessageBox::warning(this, tr("Image Process"), tr("Image data is empty\nPlease load image"));
        return;
    }
    //using namespace NaieFeature;
    NaieFeature::HarrisCorner *harrisCorner = new NaieFeature::HarrisCorner(m_SrcImage);
    harrisCorner->doing();
}

void MainWindow::on_actionLBP_triggered()
{
    FaceView *faceView = new FaceView(this);
    faceView->show();
}

void MainWindow::on_actionCustomLBP_triggered()
{

}

void MainWindow::on_actionMatching_triggered()
{
    StereoMatch *match = new StereoMatch(this);
    match->show();
}
