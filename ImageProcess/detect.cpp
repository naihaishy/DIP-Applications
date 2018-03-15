#include "detect.h"
 
 
 

Detect::Detect(vector<Mat> &mats)
{
    this->m_InputMats = mats;
    this->m_w = 100;
    this->m_h = 100;

    //扩展margin
    m_Margin = LSSD::getMargin();

}

void Detect::doDetecting()
{
    /* 说明:
     * 1.用矩形框遍历给定的第一张图像  n个
     * 2.对于每个矩形框图像区域, 基于LSSD在其他的(k-1个)输入的每张图像中找到最匹配的区域
     * 3.确定相同的区域
     */
 

	InitLSSD();

    SelectInterest();

    vector<Point> &commonRegion = FindCommonRegion();


    //根据这些点将common object 绘制出来
    DrawCentralPoint(commonRegion);


}



//初始化 计算每张输入图像的LSSD
void Detect::InitLSSD()
{
    for(int k=0; k<m_InputMats.size(); k++)
    {
        LSSD lssd(m_InputMats[k]);//第k张图像全部下像素的LSSD

        m_Lssds.push_back(lssd);
    }
 
}

//基于LSSD的目标匹配
void Detect::SelectInterest()
{
    /* 说明:
     * 1.用矩形框遍历给定的第一张图像 n个
     * 2.对于每个矩形框图像区域, 基于LSSD在其他的(k-1个)输入的每张图像中找到最匹配的区域
     */

    int rectSize = 99;
    Mat firstImg = m_InputMats[0]; //99x99

    int count = 0;
	//只在有描述子的图像区域进行运算 [m_Margin, width - m_Margin-1]
    //for(int i= m_Margin; i<firstImg.rows - m_Margin - 1 -rectSize; i+=rectSize)//row
    //{
    //    for(int j= m_Margin; j<firstImg.cols - m_Margin - 1 -rectSize; j+=rectSize)//col
    //    {
    //        count++;
    //        //sketch
    //        Rect region(j, i, rectSize, rectSize);//x->col y->row
    //        SelectBasedOnRect(region);
    //    }
    //}

	//test
	for (int i = 60; i<160; i += 10)//row
	{
		for (int j =100; j<200; j += 10)//col
		{
			count++;
			//sketch
			Rect region(j, i, rectSize, rectSize);//x->col y->row
			SelectBasedOnRect(region);
		}
	}



}


//确定图像相同区域
vector<Point>& Detect::FindCommonRegion()
{
    /* 说明:
     * 从n个区域对中确定最有意义的目标区域对
     */

    int size = m_MaxMatchValuesVec.size();// = n
    vector<double> signs;
    for(int i=0; i<size; i++)
    {
        //计算Signifiance
        double signifiance = CalcSignifiance(i);
        signs.push_back(signifiance);
    }

    //最有意义的目标区域对
    int index = std::distance(signs.begin(), std::max_element(signs.begin(), signs.end()));

	//返回最有意义一组ck的点信息
    return m_MaxMatchPointsVec[index];
}


//基于sketch的Rect detecting common
void Detect::SelectBasedOnRect(Rect region)
{
	//存储该region的所有match信息
	vector<vector<Point>> allMatchesPointsVec;
	vector<vector<double>> allMatchesValuesVec;
	
	//计算k幅图像中与此region 的all matches
	FindAllMatchesInAllImages(region, allMatchesPointsVec, allMatchesValuesVec);
	//存储该region的match信息
	m_AllMatchesPointsVec.push_back(allMatchesPointsVec);
	m_AllMatchesValuesVec.push_back(allMatchesValuesVec);


	//计算k幅图像中与此region最match的部分
	vector<Point> bestmatchPoints;
	vector<double> bestmatchValues;
	// k = allMatchesPointsVec.size() k幅图(k-1也行)
	for (int i = 0;i < allMatchesPointsVec.size();++i)
	{
		//得到第i幅图像的 all match
		vector<Point> &matchPoints = allMatchesPointsVec[i];
		vector<double> &matchValues = allMatchesValuesVec[i];

		//计算该幅图像的best match

		//bestmatch 值
		double bestmatchValue = *std::max_element(matchValues.begin(), matchValues.end());
		//bestMatch的位置ck
		int bestMatchindex = std::distance(matchValues.begin(), std::max_element(matchValues.begin(), matchValues.end()));//最大值在vector中的位置

		Point bestmatchPoint = matchPoints[bestMatchindex];

		bestmatchPoints.push_back(bestmatchPoint);
		bestmatchValues.push_back(bestmatchValue);
	}


	//存储以region为sktech的1对相匹配的矩形框图像区域
	m_MaxMatchPointsVec.push_back(bestmatchPoints);
	m_MaxMatchValuesVec.push_back(bestmatchValues);
}



//基于S计算所有match
void Detect::FindAllMatchesInAllImages(Rect sketchRegion, vector<vector<Point>> &allmatchPoints, vector<vector<double>> &allmatchValues) {
	
	/* 说明:
	* 在所有输入图像中计算存储所有matches 为了下面计算重要性
	*/

	for (int i = 1; i<m_InputMats.size(); i++)
	{
		//all matches
		vector<double> allmatchValue;//存储第K张输入图像的每个矩形框区域图像与sketch的match值
		vector<Point> allmatchPoint; //存储第K张输入图像的每个矩形框区域图像与sketch的match点
		CalcAllMatches(sketchRegion, i, allmatchValue, allmatchPoint);

		//allmatchPoints 存储K张图像的match信息
		allmatchPoints.push_back(allmatchPoint);
		allmatchValues.push_back(allmatchValue);
	}
}


//计算某sketch与第k个输入图像的all match信息
void Detect::CalcAllMatches(Rect sketchRegion, int indexK, vector<double> &matchValue, vector<Point> &matchPoint) {
	/* 说明:
	* match = sum(sim(S,I`k));
	* 1.以矩形框形式遍历第k张输入图像
	* 2.计算每个矩阵区域像素和mat的描述子集合的match
	* 3.选择最大的match作为第k个图像与mat匹配的结果
	*/

	//以矩形框形式遍历第k张输入图像
	Mat inputMat = m_InputMats[indexK];

	int rectWidth = sketchRegion.width; //sketch的宽度cols
	int rectHeight = sketchRegion.height;//sketch的高度rows
	//只在有描述子的区域运算
	for (int i = m_Margin; i<inputMat.rows - m_Margin - 1 - rectHeight; i += rectHeight)//行
	{
		for (int j = m_Margin; j<inputMat.cols - m_Margin - 1 - rectWidth; j += rectWidth)//列
		{
			Rect region(j, i, rectWidth, rectHeight);//x->col y->row

			double match = CalcMatchBetweenTwoRegion(sketchRegion, region, indexK);

			matchValue.push_back(match);
			Point centerPoint(j + (rectWidth - 1) / 2, i + (rectHeight - 1) / 2);
			matchPoint.push_back(centerPoint);//以ck为中心
		}
	}
}

//计算sim(d1,d2)=- |d1-d2|p
double Detect::CalcSimBetweenTwoDescriptor(const LSSDescriptor &d1, const LSSDescriptor &d2)
{
    return LSSD::SimilarityBetweenTwoDescriptor(d1, d2);
}

//计算两个相同形状Region 描述子的match
double Detect::CalcMatchBetweenTwoRegion(Rect rect1, Rect rect2, int indexK)
{
    /* 说明:
     * match = sum(sim(dki, dsi));
     * 1.第k张图像的全图描述子集合
     * 2.选择rect2内的描述子集合
     * 3.计算match
     */

    //全图描述子集合
    LSSD klssd = m_Lssds[indexK];//第k张图像的全图描述子集合
    LSSD sketchLssd = m_Lssds[0];//第1张

    //Rect内的描述子集合
    vector<LSSDescriptor> descriptors1, descriptors2;
    std::for_each(sketchLssd.m_Descriptors.begin(), sketchLssd.m_Descriptors.end(), [&](LSSDescriptor &desc){
        Point p(desc.col, desc.row);
        if(rect1.contains(p)){
            descriptors1.push_back(desc);
        }
    });

    std::for_each(klssd.m_Descriptors.begin(), klssd.m_Descriptors.end(), [&](LSSDescriptor &desc){
        Point p(desc.col, desc.row);
        if(rect2.contains(p)){
            descriptors2.push_back(desc);
        }
    });

    double match = 0;

    for(int i=0; i<descriptors1.size(); ++i)
    {
        //每个像素的descriptor
        LSSDescriptor &dki = descriptors1[i];
        LSSDescriptor &dsi = descriptors2[i];
        //计算sim(d1,d2)
        double sim = CalcSimBetweenTwoDescriptor(dki, dsi);
        match += sim;
    }

    return match;
}

//计算第i个region区域对的重要性
double Detect::CalcSignifiance(int indexK)
{
	/* 说明:
	* 每个目标区域对有k幅图像的match信息
	* vector<vector<Point>> vector<vector<double>>
	*/
	//allmatchPoints 大小为k-1
	vector<vector<Point>> &allmatchPoints = m_AllMatchesPointsVec[indexK];
	vector<vector<double>> &allmatchValues = m_AllMatchesValuesVec[indexK];
	
	double sum = 0.0;
	int count = 0;//all images match个数
	std::for_each(allmatchValues.begin(), allmatchValues.end(), [&](const vector<double> &matchValues) {
		//该幅图像all match和
		std::for_each(matchValues.begin(), matchValues.end(), [&](const double d) {
			sum += d;
			count++;
		});
	});

    double avgMatch =  sum / count; //均值

    double accum  = 0.0;
	std::for_each(allmatchValues.begin(), allmatchValues.end(), [&](const vector <double> matchValues) {
		//该幅图像all match和
		std::for_each(matchValues.begin(), matchValues.end(), [&](const double d) {
			accum += (d - avgMatch)*(d - avgMatch);
		});
	});

    double stdMatch = sqrt(accum/(count -1)); //方差


    double signifiance = 0.0;
	vector<double> &maxMatchValues = m_MaxMatchValuesVec[indexK];

    std::for_each (maxMatchValues.begin(), maxMatchValues.end(), [&](const double item){
        signifiance += (item - avgMatch)/stdMatch;
    });

    return signifiance;

}


void Detect::DrawCentralPoint(vector<Point> &points)
{
	
	int rectSize = 99;
	int rectRadius = 44;


    for(int k=0; k<m_InputMats.size(); ++k)
    {
        //每幅图像
        Point center = points[k];//中心
		//矩形框
		Rect rect(center.x - rectRadius, center.y- rectRadius, rectSize, rectSize);

		//该矩形框内像素设为常数
		Mat &srcImage = m_InputMats[k];
		//红色 
		Mat red(rectSize, rectSize, CV_8UC3, Scalar(0, 0, 255));//x->col y->row

		addWeighted(srcImage(rect), 1.0, red, 0.5, 0.0, srcImage(rect));

		 
		//multi images
		stringstream ss;

		string name = "result_";
		string type = ".jpg";
		ss << name << (k + 1) << type;
		string filename = ss.str();
		imwrite(filename, srcImage);
    }
 


}





