#include "stereomatch.h"
#include "ui_stereomatch.h"
#include "common.h"

StereoMatch::StereoMatch(QWidget *parent) : QDialog(parent), ui(new Ui::StereoMatch)
{
    ui->setupUi(this);
    InitStatus();
    InitStereoModel();

}

StereoMatch::~StereoMatch()
{
    delete ui;
	leftImg.release();
	rightImg.release();
}

//初始化组件信息
void StereoMatch::InitStatus()
{
    //参数设置
    connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        SadWindowSize = item;
        if(isActived)
            doing();
    });
    connect(ui->spinBox_2, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        DisparityNum = item;
        if(isActived)
            doing();
    });


    //算法选择
    AlgorithmMode = ui->comboBox->currentIndex();
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int item)
    {
        AlgorithmMode = item;
        if(isActived)
            doing();
    });

}


void StereoMatch::InitStereoModel()
{
    bm = StereoBM::create();
    sgbm = StereoSGBM::create();
    isActived = false;
    SadWindowSize = 5;
    DisparityNum  = 32;
    AlgorithmMode = STEREO_BM;
}



void StereoMatch::doing()
{

    if (DisparityNum < 1 || DisparityNum % 16 != 0)
    {
        QMessageBox::critical(this, tr("Parameter Error"), tr("The max disparity must be a positive integer divisible by 16\n"));
        return ;
    }

    if (SadWindowSize < 1 || SadWindowSize % 2 != 1)
    {
        QMessageBox::critical(this, tr("Parameter Error"), tr("The block size must be a positive odd number\n"));
        return;
    }


    if (leftImg.empty())
    {
        QMessageBox::critical(this, tr("Load Error"), tr("could not load the first input image file\n"));
        return;
    }

    if (rightImg.empty())
    {
        QMessageBox::critical(this, tr("Load Error"), tr("could not load the second input image file\n"));
        return;
    }

    Size img_size = leftImg.size();

    DisparityNum = DisparityNum > 0 ? DisparityNum : ((img_size.width / 8) + 15) & -16;


    if (AlgorithmMode == STEREO_BM)
        BMSetParamaters(bm, SadWindowSize, DisparityNum);
    else
        SGBMSetParamaters(sgbm, SadWindowSize, AlgorithmMode, DisparityNum);


    Mat disp;
    int64 t = getTickCount();//统计时间

    //算法核心
    if (AlgorithmMode == STEREO_BM)
        bm->compute(leftImg, rightImg, disp);
    else if (AlgorithmMode == STEREO_SGBM || AlgorithmMode == STEREO_HH || AlgorithmMode == STEREO_3WAY || AlgorithmMode == STEREO_HH4)
        sgbm->compute(leftImg, rightImg, disp);


    disp.convertTo(dstMat, CV_8U, 255 / (DisparityNum * 16.));



	normalize(dstMat, dstMat, 0, 255, NORM_MINMAX);

    t = getTickCount() - t;//统计时间

    //显示结果图像
    Common::displayMatOnLabel(dstMat, ui->resultLabel);//显示效果图
}

void StereoMatch::BMSetParamaters(Ptr<StereoBM> &stereoBm, int &SADWindowSize, int &numberOfDisparities)
{
    Rect roi1, roi2;
    if(SADWindowSize>0 && SADWindowSize<5)
        SADWindowSize = 5;
    stereoBm->setROI1(roi1);
    stereoBm->setROI2(roi2);
    stereoBm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
    stereoBm->setPreFilterCap(31);
    stereoBm->setMinDisparity(1);
    stereoBm->setNumDisparities(numberOfDisparities);
    stereoBm->setTextureThreshold(10);
    stereoBm->setUniquenessRatio(15);
    stereoBm->setSpeckleWindowSize(100);
    stereoBm->setSpeckleRange(32);
    stereoBm->setDisp12MaxDiff(1);
}

void StereoMatch::SGBMSetParamaters(Ptr<StereoSGBM> &stereoSgbm,int &SADWindowSize, int mode, int &numberOfDisparities )
{
    //设置mode
    if (mode == STEREO_BM)
        stereoSgbm->setMode(StereoSGBM::MODE_HH);
    else if (mode == STEREO_SGBM)
        stereoSgbm->setMode(StereoSGBM::MODE_SGBM);
    else if (mode == STEREO_3WAY)
        stereoSgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);
    else if(mode == STEREO_HH4)
        stereoSgbm->setMode(StereoSGBM::MODE_HH4);

    int cn = leftImg.channels();

    stereoSgbm->setPreFilterCap(63);
    stereoSgbm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 3);
    stereoSgbm->setP1(8 * cn * stereoSgbm->getBlockSize() * stereoSgbm->getBlockSize());
    stereoSgbm->setP2(32 * cn * stereoSgbm->getBlockSize() * stereoSgbm->getBlockSize());
    stereoSgbm->setMinDisparity(0);
    stereoSgbm->setNumDisparities(numberOfDisparities);
    stereoSgbm->setUniquenessRatio(10);
    stereoSgbm->setSpeckleWindowSize(100);
    stereoSgbm->setSpeckleRange(32);
    stereoSgbm->setDisp12MaxDiff(1);


}



//start
void StereoMatch::on_pushButton_3_clicked()
{
    doing();

    if(isActived==true)
        isActived = false;
    else
        isActived = true;

    if(ui->pushButton_3->text()==QString("Start"))
    {
        ui->pushButton_3->setText(tr("Stop"));
    }
    else if(ui->pushButton_3->text()==QString("Stop"))
    {
        ui->pushButton_3->setText(tr("Start"));
    }

}

//left image
void StereoMatch::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Left Image"), tr(""), tr("Image (*.png *.bmp *.jpg)") );

    if(!fileName.isEmpty())
    {
        int color_mode = AlgorithmMode == 0 ? 0 : -1;//BM读取灰度图 SGB读取彩色图
        leftImg = imread(fileName.toStdString(), color_mode);
        ui->lineEdit->setText(fileName);
    }
}

//right image
void StereoMatch::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Right Image"), tr(""), tr("Image (*.png *.bmp *.jpg)") );

    if(!fileName.isEmpty())
    {
        int color_mode = AlgorithmMode == 0 ? 0 : -1;//BM读取灰度图 SGB读取彩色图
        rightImg = imread(fileName.toStdString(), color_mode);
        ui->lineEdit_2->setText(fileName);
    }
}

void StereoMatch::updateTime()
{

}


