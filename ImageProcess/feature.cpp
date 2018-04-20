#include "feature.h"
#include "common.h"




/******************************* SIFT *****************************/
namespace NaieFeature {
SIFT::SIFT(const vector<Mat> &matVec)
{
    queryImg = matVec[0];
    trainImg = matVec[1];
}

SIFT::SIFT(const Mat &_queryImg, const Mat &_trainImg)
{
    queryImg = _queryImg;
    trainImg = _trainImg;
}

void SIFT::doing()
{
    detect();
    showControPanel();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

void SIFT::detect()
{
    // Detect keypoints in both images.
    Ptr<xfeatures2d::SiftFeatureDetector> detector = xfeatures2d::SiftFeatureDetector::create(m_nfeatures, 3, 0.04, m_thresh, m_sigma);
    vector<KeyPoint> queryKeypoints, trainKeypoints;

    /******************************************************/
    //使用SiftFeatureDetector类的detect函数对queryImg和trainImg进行关键点检测，结果存于queryKeypoints和trainKeypoints中
    detector->detect(queryImg, queryKeypoints);
    detector->detect(trainImg, trainKeypoints);

    /******************************************************/


    //m_ResultProcessInfoLabel->setText(tr("Found %1 and %2 keypoints.\n").arg(QVariant(queryKeypoints.size()).toString()).arg(QVariant(trainKeypoints.size()).toString()) );

    //SiftDescriptorExtractor extractor;
    Ptr<xfeatures2d::SiftDescriptorExtractor> extractor = xfeatures2d::SiftDescriptorExtractor::create(m_nfeatures, 3, 0.04, m_thresh, m_sigma);
    Mat queryDescriptors, trainDescriptors;
    /******************************************************/
    //使用SiftDescriptorExtractor类的compute函数对queryImg和trainImg进行SIFT特征提取，结果存于queryDescriptors和trainDescriptors中
    extractor->compute(queryImg, queryKeypoints, queryDescriptors);
    extractor->compute(trainImg, trainKeypoints, trainDescriptors);

    /******************************************************/

    //匹配
    BFMatcher matcher(NORM_L2);
    vector<DMatch> matches;
    /******************************************************/
    // YOUR CODE HERE :
    //使用BFMatcher类的match函数对queryDescriptors和trainDescriptors中的特征向量进行匹配，结果存于matches中
    matcher.match(queryDescriptors, trainDescriptors, matches);


    /******************************************************/
    //m_ResultProcessInfoLabel->setText(tr("Found %d matches.\n").arg(matches.size()));


    // Draw the results. Displays the images side by side, with colored circles at
    // each keypoint, and lines connecting the matching keypoints between the two images.

    drawMatches(queryImg, queryKeypoints, trainImg, trainKeypoints, matches, dstMat);


}

void SIFT::showControPanel()
{
    //创建dialog对象
    QDialog *dlg = new QDialog();
    dlg->setWindowTitle(QObject::tr("SIFT Control Panel"));
    dlg->resize(QSize(500,300));
    dlg->setFont(QFont("Microsoft Yahei"));

    //dialog的主布局
    QGridLayout *mainLayout = new QGridLayout(dlg);


    // 下面绘制各个子布局和组件

    // 控制nfeatures参数
    QVBoxLayout *layout1 = new QVBoxLayout();
    QHBoxLayout *layout1_1 = new QHBoxLayout();
    QLabel *nfeaturesLabel = new QLabel(QObject::tr("Set Number of Best Features:"));
    QSlider *nfeaturesSlider = new QSlider();
    QSpinBox *nfeaturesSpinBox = new QSpinBox();

    nfeaturesSpinBox->setMinimum(0);
    nfeaturesSpinBox->setMaximum(999);
    nfeaturesSpinBox->setSingleStep(10);
    nfeaturesSpinBox->setValue(50);

    nfeaturesSlider->setOrientation(Qt::Horizontal);  // 水平方向
    nfeaturesSlider->setMinimum(0);
    nfeaturesSlider->setMaximum(999);
    nfeaturesSlider->setSingleStep(1);
    nfeaturesSlider->setValue(50);

    layout1_1->addWidget(nfeaturesLabel);
    layout1_1->addWidget(nfeaturesSpinBox);

    layout1->addLayout(layout1_1);
    layout1->addWidget(nfeaturesSlider);


    //控制edgeThreshold参数
    QVBoxLayout *layout2 = new QVBoxLayout();
    QHBoxLayout *layout2_2 = new QHBoxLayout();
    QLabel *edgeThresholdLabel = new QLabel(QObject::tr("Set Edge Threshold:"));
    QSlider *edgeThresholdSlider = new QSlider();
    QSpinBox *edgeThresholdSpinBox = new QSpinBox();// 微调框

    edgeThresholdSpinBox->setMinimum(1);
    edgeThresholdSpinBox->setMaximum(255);
    edgeThresholdSpinBox->setSingleStep(1);
    edgeThresholdSpinBox->setValue(10);


    edgeThresholdSlider->setOrientation(Qt::Horizontal);  // 水平方向
    edgeThresholdSlider->setMinimum(1);
    edgeThresholdSlider->setMaximum(255);
    edgeThresholdSlider->setSingleStep(5);
    edgeThresholdSlider->setValue(10);

    layout2_2->addWidget(edgeThresholdLabel);
    layout2_2->addWidget(edgeThresholdSpinBox);
    layout2->addLayout(layout2_2);
    layout2->addWidget(edgeThresholdSlider);

    //控制sigma参数
    QVBoxLayout *layout3 = new QVBoxLayout();
    QHBoxLayout *layout3_1 = new QHBoxLayout();
    QLabel *sigmaLabel = new QLabel(QObject::tr("Set Sigma of Gaussian:"));
    QSlider *sigmaSlider = new QSlider();
    QSpinBox *sigmaSpinBox = new QSpinBox();// 微调框

    sigmaSpinBox->setMinimum(1);
    sigmaSpinBox->setMaximum(99);
    sigmaSpinBox->setSingleStep(1);
    sigmaSpinBox->setValue(16);
    sigmaSpinBox->setSuffix(QString(" /10"));

    sigmaSlider->setOrientation(Qt::Horizontal);  // 水平方向
    sigmaSlider->setMinimum(1);
    sigmaSlider->setMaximum(99);
    sigmaSlider->setSingleStep(3);
    sigmaSlider->setValue(16);

    layout3_1->addWidget(sigmaLabel);
    layout3_1->addWidget(sigmaSpinBox);
    layout3->addLayout(layout3_1);
    layout3->addWidget(sigmaSlider);


    // 显示效果图像
    QVBoxLayout *layout4 = new QVBoxLayout();
    m_ResultLabel = new QLabel();

    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图

    m_ResultProcessInfoLabel = new QLabel();

    layout4->addWidget(m_ResultLabel);
    layout4->addWidget(m_ResultProcessInfoLabel);


    // 连接信号槽（相互改变）
    QObject::connect(nfeaturesSpinBox, SIGNAL(valueChanged(int)), nfeaturesSlider, SLOT(setValue(int)));
    QObject::connect(nfeaturesSlider, SIGNAL(valueChanged(int)), nfeaturesSpinBox, SLOT(setValue(int)));
    QObject::connect(edgeThresholdSpinBox, SIGNAL(valueChanged(int)), edgeThresholdSlider, SLOT(setValue(int)));
    QObject::connect(edgeThresholdSlider, SIGNAL(valueChanged(int)), edgeThresholdSpinBox, SLOT(setValue(int)));
    QObject::connect(sigmaSpinBox, SIGNAL(valueChanged(int)), sigmaSlider, SLOT(setValue(int)));
    QObject::connect(sigmaSlider, SIGNAL(valueChanged(int)), sigmaSpinBox, SLOT(setValue(int)));

    //动态修改参数 效果图像
    QObject::connect(nfeaturesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(FeatureNumDynamicChangeResultMat(int)));
    QObject::connect(nfeaturesSlider, SIGNAL(valueChanged(int)), this, SLOT(FeatureNumDynamicChangeResultMat(int)));
    QObject::connect(edgeThresholdSpinBox, SIGNAL(valueChanged(int)), this, SLOT(EdgeThresholdDynamicChangeResultMat(int)));
    QObject::connect(edgeThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(EdgeThresholdDynamicChangeResultMat(int)));
    QObject::connect(sigmaSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SigmaDynamicChangeResultMat(int)));
    QObject::connect(sigmaSlider, SIGNAL(valueChanged(int)), this, SLOT(SigmaDynamicChangeResultMat(int)));


    mainLayout->addLayout(layout1, 0, 0);
    mainLayout->addLayout(layout2, 3, 0);
    mainLayout->addLayout(layout3, 5, 0);
    mainLayout->addLayout(layout4, 7, 0);
    dlg->exec();

}

void SIFT::FeatureNumDynamicChangeResultMat(int num)
{
    m_nfeatures = num;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

void SIFT::EdgeThresholdDynamicChangeResultMat(int threshold)
{
    m_thresh = threshold;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

void SIFT::SigmaDynamicChangeResultMat(int sigma)
{
    m_sigma = sigma/10;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图

}



}





/******************************* Harris Corner *****************************/
namespace NaieFeature {

HarrisCorner::HarrisCorner(const Mat &mat)
{
    srcMat = mat;

    cvtColor(srcMat, srcGrayMat, CV_BGR2GRAY);

}

void HarrisCorner::doing()
{

    detect();
    showControPanel();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}


//构造控制面板 参数调控和图像显示
void HarrisCorner::showControPanel()
{
    //创建dialog对象
    QDialog *dlg = new QDialog();
    dlg->setWindowTitle(QObject::tr("Harris Corner Control Panel"));
    dlg->resize(QSize(500,300));
    dlg->setFont(QFont("Microsoft Yahei"));

    //dialog的主布局
    QGridLayout *mainLayout = new QGridLayout(dlg);


    // 下面绘制各个子布局和组件

    // 控制blockSize参数
    QVBoxLayout *layout1 = new QVBoxLayout();
    QHBoxLayout *layout1_1 = new QHBoxLayout();
    QLabel *blockSizeLabel = new QLabel(QObject::tr("Set blockSize:"));
    QSlider *blockSizeSlider = new QSlider();
    QSpinBox *blockSizeSpinBox = new QSpinBox();// 微调框

    blockSizeSpinBox->setMinimum(2);
    blockSizeSpinBox->setMaximum(99);
    blockSizeSpinBox->setSingleStep(1);
    blockSizeSpinBox->setValue(2);

    blockSizeSlider->setOrientation(Qt::Horizontal);  // 水平方向
    blockSizeSlider->setMinimum(2);
    blockSizeSlider->setMaximum(99);
    blockSizeSlider->setSingleStep(1);
    blockSizeSlider->setValue(2);

    layout1_1->addWidget(blockSizeLabel);
    layout1_1->addWidget(blockSizeSpinBox);

    layout1->addLayout(layout1_1);
    layout1->addWidget(blockSizeSlider);


    //控制k参数
    QVBoxLayout *layout2 = new QVBoxLayout();
    QHBoxLayout *layout2_2 = new QHBoxLayout();
    QLabel *kLabel = new QLabel(QObject::tr("Set K Value:"));
    QSlider *kSlider = new QSlider();
    QSpinBox *kSpinBox = new QSpinBox();// 微调框

    kSpinBox->setMinimum(1);
    kSpinBox->setMaximum(100);
    kSpinBox->setSingleStep(1);
    kSpinBox->setValue(4); // 0.04
    kSpinBox->setSuffix(QString(" /100"));

    kSlider->setOrientation(Qt::Horizontal);  // 水平方向
    kSlider->setMinimum(1);
    kSlider->setMaximum(100);
    kSlider->setSingleStep(1);
    kSlider->setValue(4);

    layout2_2->addWidget(kLabel);
    layout2_2->addWidget(kSpinBox);
    layout2->addLayout(layout2_2);
    layout2->addWidget(kSlider);

    //控制threshold参数
    QVBoxLayout *layout3 = new QVBoxLayout();
    QHBoxLayout *layout3_1 = new QHBoxLayout();
    QLabel *tLabel = new QLabel(QObject::tr("Set Threshold:"));
    QSlider *tSlider = new QSlider();
    QSpinBox *tSpinBox = new QSpinBox();// 微调框

    tSpinBox->setMinimum(80);
    tSpinBox->setMaximum(255);
    tSpinBox->setSingleStep(1);
    tSpinBox->setValue(80);

    tSlider->setOrientation(Qt::Horizontal);  // 水平方向
    tSlider->setMinimum(80);
    tSlider->setMaximum(255);
    tSlider->setSingleStep(5);
    tSlider->setValue(80);

    layout3_1->addWidget(tLabel);
    layout3_1->addWidget(tSpinBox);
    layout3->addLayout(layout3_1);
    layout3->addWidget(tSlider);


    // 显示效果图像
    QVBoxLayout *layout4 = new QVBoxLayout();
    m_ResultLabel = new QLabel();

    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图

    layout4->addWidget(m_ResultLabel);



    // 连接信号槽（相互改变）
    QObject::connect(blockSizeSpinBox, SIGNAL(valueChanged(int)), blockSizeSlider, SLOT(setValue(int)));
    QObject::connect(blockSizeSlider, SIGNAL(valueChanged(int)), blockSizeSpinBox, SLOT(setValue(int)));
    QObject::connect(kSpinBox, SIGNAL(valueChanged(int)), kSlider, SLOT(setValue(int)));
    QObject::connect(kSlider, SIGNAL(valueChanged(int)), kSpinBox, SLOT(setValue(int)));
    QObject::connect(tSpinBox, SIGNAL(valueChanged(int)), tSlider, SLOT(setValue(int)));
    QObject::connect(tSlider, SIGNAL(valueChanged(int)), tSpinBox, SLOT(setValue(int)));

    //动态修改参数 效果图像
    QObject::connect(blockSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(BlockSizeDynamicChangeResultMat(int)));
    QObject::connect(blockSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(BlockSizeDynamicChangeResultMat(int)));
    QObject::connect(kSpinBox, SIGNAL(valueChanged(int)), this, SLOT(KDynamicChangeResultMat(int)));
    QObject::connect(kSlider, SIGNAL(valueChanged(int)), this, SLOT(KDynamicChangeResultMat(int)));
    QObject::connect(tSpinBox, SIGNAL(valueChanged(int)), this, SLOT(ThresholdDynamicChangeResultMat(int)));
    QObject::connect(tSlider, SIGNAL(valueChanged(int)), this, SLOT(ThresholdDynamicChangeResultMat(int)));


    mainLayout->addLayout(layout1, 0, 0);
    mainLayout->addLayout(layout2, 3, 0);
    mainLayout->addLayout(layout3, 5, 0);
    mainLayout->addLayout(layout4, 7, 0);
    dlg->exec();


}

void HarrisCorner::detect()
{
    dstMat = srcMat.clone();

    Mat dst, dst_norm;
    dst = Mat::zeros(srcMat.size(), CV_32FC1);

    /// Detecting corners
    cornerHarris(srcGrayMat, dst, m_blockSize, m_apertureSize, m_k, BORDER_DEFAULT);

    /// Normalizing
    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());

    /// Drawing a circle around corners
    for (int j = 0; j < dst_norm.rows; j++) {
        for (int i = 0; i < dst_norm.cols; i++) {
            if ((int)dst_norm.at<float>(j, i) > m_thresh) {
                circle(dstMat, Point(i, j), 5, Scalar(0, 0, 255), 2, 8, 0);
            }
        }
    }
}

/***SLOT:动态更改参数BlockSize 修改效果图像***/
void HarrisCorner::BlockSizeDynamicChangeResultMat(int blocksize)
{
    m_blockSize = blocksize;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

/***SLOT:动态更改参数K 修改效果图像***/
void HarrisCorner::KDynamicChangeResultMat(int k)
{
    m_k = k/100;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

/***SLOT:动态更改参数Threshold 修改效果图像***/
void HarrisCorner::ThresholdDynamicChangeResultMat(int threshold)
{
    m_thresh = threshold;
    detect();
    Common::displayMatOnLabel(dstMat, m_ResultLabel);//显示效果图
}

}

