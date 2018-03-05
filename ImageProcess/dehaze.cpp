#include "dehaze.h"


Dehaze::Dehaze(Mat &mat, int ksize, double weight)
{
    if(mat.channels()!=3)
    {
        throw("Error");
    }

    m_SrcMat = mat.clone(); //mat.copyTo(this->m_SrcMat);
    m_Ksize = ksize;
    m_Wight = weight;
    m_Rows = m_SrcMat.rows;
    m_Cols = m_SrcMat.cols;
    m_SelectShowIndex = 0;
    m_TranGenMethod = 0;
    m_DarkMinOrder = 0;
    m_TranMinOrder = 0;
    m_GuideFilterRadius = m_Ksize*4;
    m_GuideFilterEps = 0.001;
    m_DarkChannelMat = Mat::zeros(m_Rows, m_Cols, CV_8UC1);//暗通道图像的初始化
    m_TransmissionMat = Mat::zeros(m_Rows, m_Cols, CV_64FC1);//传导图像的初始化
    m_OriginScene = Mat::zeros(m_Rows, m_Cols, CV_8UC3);//

 
}




//类的接口 公开调用
Mat Dehaze::getDarkChannelImage()
{
	return m_DarkChannelMat;
}

Mat Dehaze::getTransmissionMap()
{
    Mat DarkChannelMat = Mat::zeros(m_Rows, m_Cols, CV_8UC1);
    m_TransmissionMat.convertTo(DarkChannelMat, CV_8UC1, 255);
    return DarkChannelMat;
}

Mat Dehaze::getOriginScene()
{
	return m_OriginScene;
}


bool Dehaze::doDehazing()
{
	//计算暗通道图像
    CalDarkChannel();
	//估算大气亮度
    CalAtmosphericLight();
	//估计传导图像
    CalTransmission();
	//估计原始场景图
    CalOriginScene();

	return true;
}

void Dehaze::CalDarkChannel()
{
    if(m_DarkMinOrder==0){
        //正常顺序
        DarkChannelImage();
    }else{
        //反顺序
        DrakChannelImageExchangeMin();
    }
}

void Dehaze::CalAtmosphericLight()
{
    AtmosphericLight();
}

void Dehaze::CalTransmission()
{
    if(m_TranMinOrder==0){
        EstimatingTransmission();
    }else{
        EstimatingTransmissionExchangeMin();
    }
}

void Dehaze::CalOriginScene()
{
    OriginScene();
}


/***计算暗通道图像***/
void Dehaze::DarkChannelImage()
{
	//正常Min顺序 先对个三个通道单独进行最小值滤波 然后求每个像素在三个通道下的最小值 

    //最小值滤波
    Mat minFilterMat;//CV_8UC3
    MinFilter(m_SrcMat, minFilterMat);

    //计算每个像素的三个通道的最小值 最终得到暗通道图像(CV_8UC1)
    minimumPixelInChannles(minFilterMat, m_DarkChannelMat);

}

/***计算暗通道图像 更改最小化顺序***/
void Dehaze::DrakChannelImageExchangeMin() 
{
    //先求单个像素三个通道的最小值 形成单通道图像 然后进行最小值滤波

    //计算每个像素在三个通道下的最小值
    Mat minimumMat = Mat::zeros(m_Rows, m_Cols, CV_8UC1);
    minimumPixelInChannles(m_SrcMat, minimumMat);

    //最小值滤波
    MinFilter(minimumMat, m_DarkChannelMat);
}

/*大气亮度估计*/
void Dehaze::AtmosphericLight()
{
	// 计算暗通道图灰度最大的前0.1%的像素所在位置
    vector<Point> mostPoints;
    FindMostLightPostion(m_DarkChannelMat, mostPoints, 0.001);

    //将原始带雾图像在这些位置中最大的灰度值作为大气亮度
    vector<uchar> mostPixelR, mostPixelG, mostPixelB;
    for (int i = 0; i<mostPoints.size(); ++i)
    {
        int row = mostPoints[i].x;
        int col = mostPoints[i].y;
        Vec3b *pCols = m_SrcMat.ptr<Vec3b>(row);
        mostPixelB.push_back(pCols[col][0]);//B
        mostPixelG.push_back(pCols[col][1]);//G
        mostPixelR.push_back(pCols[col][2]);//R
    }

	//最大值
	m_AtmosphericLight[0] = (int)*max_element(mostPixelB.begin(), mostPixelB.end());//B
	m_AtmosphericLight[1] = (int)*max_element(mostPixelG.begin(), mostPixelG.end());//G
	m_AtmosphericLight[2] = (int)*max_element(mostPixelR.begin(), mostPixelR.end());//R
  
}


//估计传导图像 正常顺序
void Dehaze::EstimatingTransmission()
{
    //图像归一化 I/A
    Mat normalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC3);
    NormalizeSrcMat(m_SrcMat, normalMat, m_AtmosphericLight);

    // 计算每个像素在三个通道下的最小值
    Mat minimumNormalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC1);//CV_64FC1 元素为double类型
    minimumPixelInChannles(normalMat, minimumNormalMat);

    //计算归一化后Mat的最小值滤波
    Mat minNormalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC1);//CV_64FC3 元素为double类型
    MinFilter(minimumNormalMat, minNormalMat);

    // 计算最终传导图像
    m_TransmissionMat = 1 - minNormalMat * m_Wight;

    //精制传导图像
    if(m_TranGenMethod==1){
        RefineTransmission(m_TransmissionMat, m_TransmissionMat);
    }


}

//估计传导图像 调换min顺序
void Dehaze::EstimatingTransmissionExchangeMin()
{
    //图像归一化 I/A
    Mat normalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC3);
    NormalizeSrcMat(m_SrcMat, normalMat, m_AtmosphericLight);

    //计算归一化后Mat的最小值滤波
    Mat minNormalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC3);//CV_64FC3 元素为double类型
    MinFilter(normalMat, minNormalMat);

    // 计算每个像素在三个通道下的最小值
    Mat minimumNormalMat = Mat::zeros(m_Rows, m_Cols, CV_64FC1);//CV_64FC1 元素为double类型
    minimumPixelInChannles(minNormalMat, minimumNormalMat);

    // 计算最终传导图像
    m_TransmissionMat = 1 - minimumNormalMat * m_Wight;


    //精制传导图像
    if(m_TranGenMethod==1){
        RefineTransmission(m_TransmissionMat, m_TransmissionMat);
    }

}


//估计原始场景图
void Dehaze::OriginScene()
{
	double ttt0 = 0.1;
	//多通道分解
	vector<Mat> srcMatChannels;
	split(m_SrcMat, srcMatChannels);//BGR

	vector<Mat> channels;

	//值全是t0的矩阵
    Mat toMat(m_Rows, m_Cols, CV_64FC1, cv::Scalar(ttt0));
	//分母
	Mat denominator = cv::max(toMat, m_TransmissionMat); // max(tx,t0) 单通道 CV_64FC1
	//计算分母矩阵每个元素的倒数
	Mat denominatorRe = Mat(m_Rows, m_Cols, CV_64FC1);
	cv::divide(Mat::ones(m_Rows, m_Cols, CV_64FC1), denominator, denominatorRe);


	for(int cc=0; cc<3; cc++)
	{
		//单个通道的处理
		//构建大气亮度的单通道矩阵 
		Mat temp1(m_Rows, m_Cols, CV_64FC1, cv::Scalar((double)m_AtmosphericLight[cc]));// A
		//矩阵类型转换
		Mat srcM; //I
		srcMatChannels[cc].convertTo(srcM, CV_64FC1);//CV_8UC1->CV_64FC1
		Mat numerator = srcM - temp1;//分子 I-A

		Mat temp2 = Mat(m_Rows, m_Cols, CV_64FC1);
		temp2 = numerator.mul(denominatorRe) + temp1;//CV_64FC1 mul 两个矩阵的对应元素相乘 
		channels.push_back(temp2);
	}

	//合并通道
	Mat temp3 = Mat(m_Rows, m_Cols, CV_64FC3);
	merge(channels, temp3);
	temp3.convertTo(m_OriginScene, CV_8UC3);
}


/**最小值滤波 可处理多通道和单通道
@param inputMat 三通道或单通道图像
@param outputMat 三通道或单通道图像
*/
void Dehaze::MinFilter(Mat &inputMat, Mat &outputMat)
{
    Mat se = getStructuringElement(MORPH_RECT, Size(m_Ksize, m_Ksize));//se
    erode(inputMat, outputMat, se, Point(-1, -1), 1, BORDER_REPLICATE); //borderType  BORDER_REFLECT 边界扩充 aaaaaa|abcdefgh|hhhhhhh
    //三个通道独立进行最小值滤波

}


/**计算每个像素在三个通道下的最小值
@param inputMat 三通道图像
@param outputMat 单通道图像
*/
void Dehaze::minimumPixelInChannles(Mat &inputMat, Mat &outputMat)
{
    if(inputMat.channels()!=3){
        throw "input mat is not a 3 channles mat" ;
    }
    Mat channels[3], temp;
    split(inputMat, channels); //分解为BGR三个通道
    temp = cv::min(channels[0], channels[1]);//find minimum between B and G channels
    outputMat = cv::min(temp, channels[2]);  //find minimum between temp and R channels
}

/**归一化原图 I/A
@param inputMat 三通道图像
@param outputMat 三通道图像
*/
void Dehaze::NormalizeSrcMat(Mat &inputMat, Mat &outputMat, int *light)
{
    if(inputMat.channels()!=3){
        throw "input mat is not a 3 channles mat" ;
    }
    //Mat类型转换
    Mat inputMat2;
	inputMat.convertTo(inputMat2, CV_64FC3);//CV_8UC3 ->CV_64FC3
	//多通道分解
	vector<Mat> srcMatChannels;
    split(inputMat2, srcMatChannels);//BGR
    for(int cc=0; cc< 3; cc++)
    {
		double scale = 1.0 /(double(light[cc]));// 除以大气亮度
        srcMatChannels[cc] = srcMatChannels[cc] * scale;
    }
    merge(srcMatChannels, outputMat);//合并通道
}

/**计算图像灰度最大的前n个像素所在位置
@param inputMat 暗通道图像
@param outputMat 三通道图像
@param rate 比例 n = size*rate
*/
void Dehaze::FindMostLightPostion(Mat &inputMat, vector<Point> &output, double rate)
{
    if(inputMat.channels()!=1){
        throw "input mat is not dark channle mat" ;
    }


    //遍历暗通道图像的所有像素 CV_8UC1
    vector<DarkChannelMost> temp;

    for( int i = 0; i < m_Rows; i++)//行
    {
        uchar * pCols = inputMat.ptr<uchar>(i);
        for( int j = 0; j < m_Cols; j++)//列
        {
            temp.push_back(DarkChannelMost(pCols[j], i, j));
        }
    }

    //降序排序
    sort(temp.begin(), temp.end(), greater<DarkChannelMost>()); //根据pixel进行降序排序
    //选取前n个灰度值
    for(int i=0; i<temp.size()*rate; ++i)
    {
        output.push_back(Point(temp[i].row, temp[i].col));
    }
}

//精制传导图像
void Dehaze::RefineTransmission(Mat &inputMat, Mat &outputMat)
{
    Mat guide, temp;
    //使用原始图像
    m_SrcMat.convertTo(temp, CV_32FC1);//CV_8UC3->CV_64FC1
    normalize(temp, guide, 1.0, 0.0, NORM_MINMAX);//0-255 0-1

    Mat dst = Mat::zeros(m_Rows, m_Cols, CV_32FC1);
    Mat src;
    inputMat.convertTo(src, CV_32FC1);

    cv::ximgproc::guidedFilter(guide, src, dst, m_GuideFilterRadius, m_GuideFilterEps);
    dst.convertTo(outputMat, CV_64FC1);
}




void Dehaze::ShowControlPanel()
{
    //创建dialog对象
    QDialog *dlg = new QDialog();
    dlg->setWindowTitle(QObject::tr("Dehaze control panel"));
    dlg->resize(QSize(500,300));
    dlg->setFont(QFont("Microsoft Yahei"));

    //dialog的主布局
    QGridLayout *mainLayout = new QGridLayout(dlg);


    // 下面绘制各个子布局和组件
    // 控制ksize参数
    int minkSize = 3;// 最小值
    int maxkSize = 99;// 最大值
    int singleStep = 1;// 步长
    int defaultSize = 15;// 默认值
    QVBoxLayout *layout1 = new QVBoxLayout();
    QHBoxLayout *layout1_1 = new QHBoxLayout();
    QLabel *kSizeLabel = new QLabel(QObject::tr("Set Window Size"));
    QSlider *kSizeSlider = new QSlider();
    QSpinBox *kSizeSpinBox = new QSpinBox();// 微调框

    kSizeSpinBox->setMinimum(minkSize);
    kSizeSpinBox->setMaximum(maxkSize);
    kSizeSpinBox->setSingleStep(singleStep);
    kSizeSpinBox->setValue(defaultSize);

    kSizeSlider->setOrientation(Qt::Horizontal);  // 水平方向
    kSizeSlider->setMinimum(minkSize);
    kSizeSlider->setMaximum(maxkSize);
    kSizeSlider->setSingleStep(singleStep*2);
    kSizeSlider->setValue(defaultSize);

    layout1_1->addWidget(kSizeLabel);
    layout1_1->addWidget(kSizeSpinBox);

    layout1->addLayout(layout1_1);
    layout1->addWidget(kSizeSlider);




    //控制w参数 n%
    QVBoxLayout *layout2 = new QVBoxLayout();
    QHBoxLayout *layout2_2 = new QHBoxLayout();
    QLabel *wLabel = new QLabel(QObject::tr("Set Weight Value"));
    QSlider *wSlider = new QSlider();
    QSpinBox *wSpinBox = new QSpinBox();// 微调框

    wSpinBox->setMinimum(1);
    wSpinBox->setMaximum(100);
    wSpinBox->setSingleStep(1);
    wSpinBox->setValue(95); // 0.95

    wSlider->setOrientation(Qt::Horizontal);  // 水平方向
    wSlider->setMinimum(1);
    wSlider->setMaximum(100);
    wSlider->setSingleStep(5);
    wSlider->setValue(95);

    layout2_2->addWidget(wLabel);
    layout2_2->addWidget(wSpinBox);
    layout2->addLayout(layout2_2);
    layout2->addWidget(wSlider);

    //选择要显示的图像
    QVBoxLayout *layout3 = new QVBoxLayout();
    QHBoxLayout *layout3_1 = new QHBoxLayout();
    QLabel *selectLabel = new QLabel(QObject::tr("Select Image"));
    QComboBox *matSelectBox = new QComboBox();
    matSelectBox->addItem(QObject::tr("Drak Channel Image"));
    matSelectBox->addItem(QObject::tr("Tranmission Image"));
    matSelectBox->addItem(QObject::tr("Origin Scene"));

    layout3_1->addWidget(selectLabel);
    layout3_1->addWidget(matSelectBox);

    layout3->addLayout(layout3_1);



    //传导图像生成方法选择
    QVBoxLayout *layout4 = new QVBoxLayout();
    QHBoxLayout *layout4_1 = new QHBoxLayout();
    QLabel *selectLabel4 = new QLabel(QObject::tr("Select Transmission Generate Method"));
    QComboBox *matSelectBox4 = new QComboBox();
    matSelectBox4->addItem(QObject::tr("Normal"));
    matSelectBox4->addItem(QObject::tr("Guide Filter"));

    layout4_1->addWidget(selectLabel4);
    layout4_1->addWidget(matSelectBox4);

    layout4->addLayout(layout4_1);

    //传导图像的参数设置
    QHBoxLayout *layout6 = new QHBoxLayout();
    QLabel *label6_1 = new QLabel(QObject::tr("Set Guide Filter Radius"));
    QLineEdit *edit6_1 = new QLineEdit(QVariant(m_Ksize*4).toString());// m_Ksize*4
    edit6_1->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));   //只能输入数字
    QLabel *label6_2 = new QLabel(QObject::tr("Set Guide Filter Eps/10000"));
    QLineEdit *edit6_2 = new QLineEdit(QVariant(10).toString());
    edit6_2->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));   //只能输入数字


    layout6->addWidget(label6_2);
    layout6->addWidget(edit6_2);
    layout6->addWidget(label6_1);
    layout6->addWidget(edit6_1);






    //选择最小化滤波的顺序 1.暗通道图像的滤波顺序  2.估计传导图像的滤波顺序
    QVBoxLayout *layout5 = new QVBoxLayout();

    QHBoxLayout *layout5_1 = new QHBoxLayout();
    QLabel *selectLabel5_1 = new QLabel(QObject::tr("Select Drak Channeel Minimum Order"));
    QComboBox *matSelectBox5_1 = new QComboBox();
    matSelectBox5_1->addItem(QObject::tr("Normal"));
    matSelectBox5_1->addItem(QObject::tr("Reverse"));
    matSelectBox5_1->setItemData(0, QObject::tr("first minimum in 3 channels independently,and get the min value of 3 channels"), Qt::ToolTipRole);
    matSelectBox5_1->setItemData(1, QObject::tr("first get the min value of 3 channels, and minumum in 3 channels independently"), Qt::ToolTipRole);
    layout5_1->addWidget(selectLabel5_1);
    layout5_1->addWidget(matSelectBox5_1);

    QHBoxLayout *layout5_2 = new QHBoxLayout();
    QLabel *selectLabel5_2 = new QLabel(QObject::tr("Select Transmission Minimum Order"));
    QComboBox *matSelectBox5_2 = new QComboBox();
    matSelectBox5_2->addItem(QObject::tr("Normal"));
    matSelectBox5_2->addItem(QObject::tr("Reverse"));
    matSelectBox5_2->setItemData(0, QObject::tr("first minimum in 3 channels independently,and get the min value of 3 channels"), Qt::ToolTipRole);
    matSelectBox5_2->setItemData(1, QObject::tr("first get the min value of 3 channels, and minumum in 3 channels independently"), Qt::ToolTipRole);
    layout5_2->addWidget(selectLabel5_2);
    layout5_2->addWidget(matSelectBox5_2);



    layout5->addLayout(layout5_1);
    layout5->addLayout(layout5_2);



    // 显示效果图像
    QVBoxLayout *layout9 = new QVBoxLayout();
    m_ResultLabel = new QLabel();

    layout9->addWidget(m_ResultLabel);




    // 连接信号槽（相互改变）
    QObject::connect(kSizeSpinBox, SIGNAL(valueChanged(int)), kSizeSlider, SLOT(setValue(int)));
    QObject::connect(kSizeSlider, SIGNAL(valueChanged(int)), kSizeSpinBox, SLOT(setValue(int)));
    QObject::connect(wSpinBox, SIGNAL(valueChanged(int)), wSlider, SLOT(setValue(int)));
    QObject::connect(wSlider, SIGNAL(valueChanged(int)), wSpinBox, SLOT(setValue(int)));
    //动态修改参数 效果图像
    QObject::connect(kSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(KsizeDynamicChangeResultMat(int)));
    QObject::connect(kSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(KsizeDynamicChangeResultMat(int)));
    QObject::connect(wSpinBox, SIGNAL(valueChanged(int)), this, SLOT(WDynamicChangeResultMat(int)));
    QObject::connect(wSlider, SIGNAL(valueChanged(int)), this, SLOT(WDynamicChangeResultMat(int)));

    //动态选择图像
    QObject::connect(matSelectBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ImageShowChanged(int)));

    //动态选择传导图像生成方法
    QObject::connect(matSelectBox4, SIGNAL(currentIndexChanged(int)), this, SLOT(TransmissionGenerateMethodChanged(int)));


    //最小化滤波的顺序
    QObject::connect(matSelectBox5_1, SIGNAL(currentIndexChanged(int)), this, SLOT(DrakMinOrderChanged(int)));
    QObject::connect(matSelectBox5_2, SIGNAL(currentIndexChanged(int)), this, SLOT(TranMinOrderChanged(int)));

    //Guide Filter参数改变
    QObject::connect(edit6_1, SIGNAL(textChanged(QString)), this, SLOT(GuideFilterRadiusChanged(QString)));
    QObject::connect(edit6_2, SIGNAL(textChanged(QString)), this, SLOT(GuideFilterEpsChanged(QString)));





    display(m_DarkChannelMat, m_ResultLabel);

    mainLayout->addLayout(layout1, 0, 0);
    mainLayout->addLayout(layout2, 3, 0);
    mainLayout->addLayout(layout3, 5, 0);
    mainLayout->addLayout(layout4, 7, 0);
    mainLayout->addLayout(layout5, 9, 0);
    mainLayout->addLayout(layout6, 12, 0);
    mainLayout->addLayout(layout9, 15, 0);
    dlg->exec();
}

/***在Label上显示图像***/
void Dehaze::display(Mat &srcMat, QLabel *label)
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

/***SLOT:动态更改参数W 修改效果图像***/
void Dehaze::WDynamicChangeResultMat(int w)
{
    m_Wight = w/100.0; // double n%  w=1-100
    ShowImageBySelectIndex(m_SelectShowIndex);
}

/***SLOT:动态更改参数Ksize 修改效果图像***/
void Dehaze::KsizeDynamicChangeResultMat(int ksize)
{
    m_Ksize = ksize;
    doDehazing();
    ShowImageBySelectIndex(m_SelectShowIndex);
}

/***SLOT:动态选择显示的图像***/
void Dehaze::ImageShowChanged(int index)
{
    m_SelectShowIndex = index;
    ShowImageBySelectIndex(index);
}

/***根据选择框显示图像***/
void Dehaze::ShowImageBySelectIndex(int index)
{
    Mat Temp = Mat::zeros(m_Rows, m_Cols, CV_8UC1);
    m_TransmissionMat.convertTo(Temp, CV_8UC1, 255);

    switch (index) {
        case 0:
            display(m_DarkChannelMat, m_ResultLabel);
            break;
        case 1:
            display(Temp, m_ResultLabel);
            break;
        case 2:
            display(m_OriginScene, m_ResultLabel);
            break;
        default:
            display(m_OriginScene, m_ResultLabel);
            break;
    }
}

/***SLOT:传导图像生成方法改变***/
void Dehaze::TransmissionGenerateMethodChanged(int index)
{
    m_TranGenMethod = index;//0 normal 1 guidefilter
    doDehazing();
    ShowImageBySelectIndex(m_SelectShowIndex);
}

/***SLOT:Dark最小滤波顺序改变***/
void Dehaze::DrakMinOrderChanged(int index)
{
    m_DarkMinOrder = index;// 0 normal 1 reverse
    doDehazing();
    ShowImageBySelectIndex(m_SelectShowIndex);
}

/***SLOT:Tran最小滤波顺序改变***/
void Dehaze::TranMinOrderChanged(int index)
{
    m_TranMinOrder = index;// 0 normal 1 reverse
    doDehazing();
    ShowImageBySelectIndex(m_SelectShowIndex);
}

/***SLOT:Guide Filter参数改变***/
void Dehaze::GuideFilterRadiusChanged(QString string)
{
    m_GuideFilterRadius = string.toInt();
    if(m_TranGenMethod==1){
        //使用了GuideFilter
        doDehazing();
        ShowImageBySelectIndex(m_SelectShowIndex);
    }
}
/***SLOT:Guide Filter参数改变***/
void Dehaze::GuideFilterEpsChanged(QString string)
{
    m_GuideFilterEps = string.toInt() / 10000.0;
    if(m_TranGenMethod==1){
        //使用了GuideFilter
        doDehazing();
        ShowImageBySelectIndex(m_SelectShowIndex);
    }
}

