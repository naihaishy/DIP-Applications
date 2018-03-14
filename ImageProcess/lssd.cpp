#include "lssd.h"

LSSD::LSSD(const Mat &mat)
{
	this->m_SrcMat = mat;

    //转换为灰度图
    //cvtColor(m_SrcMat, m_SrcMat, CV_BGR2GRAY);
    //m_SrcMat.convertTo(m_SrcMat, CV_BGR2GRAY);

    m_Rows = m_SrcMat.rows;//宽度
    m_Cols = m_SrcMat.cols;//高度

    m_PatchSize = 5; //patch大小 5x5
    m_PatchRadius = (m_PatchSize-1)/2; //patch半径 2

    m_RegionRadius = 20;//Region Image半径 也就是描述子的半径[矩形] 40
    m_RegionSize = m_RegionRadius * 2 + 1;

    m_Margin = m_RegionRadius + m_PatchRadius;

    m_Angles = 20;
    m_Radials = 4;

	CalcSelfSimilarityDescriptor();
    
}

void LSSD::doTesting()
{
    
	//CalcSelfSimilarityDescriptor();
	vector<double> bins;
    CalcDescriptorofSepcifiedPixel(60, 140, bins);
}

void LSSD::doing() {
	CalcSelfSimilarityDescriptor();
}

int LSSD::getMargin()
{
    return 42;
}


//计算全幅图像的局部自相似描述子
void LSSD::CalcSelfSimilarityDescriptor()
{
    //descriptors
    vector<LSSDescriptor> descriptors1, descriptors2;

    //计算图像中每个像素的描述子
    CalcDescriptorofSepcifiedMat(descriptors1);

    //去除无用的descriptors 1.salient  2.homogerous

    //RemoveNonInformativeDescriptor(descriptors1, descriptors2);

    //描述子归一化
    NormaliseDescriptor(descriptors1, m_Descriptors);

}

//计算一幅图像的描述子
void LSSD::CalcDescriptorofSepcifiedMat(vector<LSSDescriptor> &descriptors)
{
    //计算图像中每个像素的描述子
    for (int i = m_Margin; i<m_Rows - m_Margin -1; ++i)
    {
        for (int j = m_Margin; j<m_Cols - m_Margin -1; ++j)
        {
            vector<double> bins;
            CalcDescriptorofSepcifiedPixel(i, j, bins);
            LSSDescriptor descriptor(i, j, bins);
            descriptors.push_back(descriptor);
        }
    }
}


//计算图像中指定像素的描述子
void LSSD::CalcDescriptorofSepcifiedPixel(int row, int col, vector<double> &bins)
{
    //点q
    //计算SSD
    Mat ssdMat = Mat::zeros(m_RegionSize, m_RegionSize, CV_64F);

    CalcSSDofSepcifiedPixel(row, col, ssdMat);

    //转换到correlation surface
    Mat corrMat = Mat::zeros(m_RegionSize, m_RegionSize, CV_64F);
    TranSSDIntoCorrelationSurface(row, col, ssdMat, corrMat);

    //转换到对数极坐标下
    Mat logPolarMat, inverseMat;
    TranIntoLogPolar(corrMat, logPolarMat, inverseMat);

    //分解为80个bins 20angles 4 radial intervals
    PartBins(logPolarMat, m_Angles, m_Radials, bins);

    //bins构成一个descriptor
}


//计算单个像素的SSD
void LSSD::CalcSSDofSepcifiedPixel(int row, int col, Mat &ssdMat)
{
    //image patch 5x5 截取
	//opencv Rect(x, y, width, height) x->col y->row
    Rect patchROI(col - m_PatchRadius, row - m_PatchRadius, m_PatchSize, m_PatchSize);
    Mat patch, patchLab;
    patch = m_SrcMat(patchROI).clone();//ROI修改将导致原图相应位置像素被修改 这里做了个拷贝
    patch.convertTo(patch, CV_32F, 1.0 / 255);//0-1 F
    cvtColor(patch, patchLab, CV_BGR2Lab);//BGR转换到CIELAB


    //在region内 计算patch 与region中每个像素的patch' 进行计算平方差和
	int count=0;
    //遍历region区域的每个像素作为patch'的中心点
    for(int k = row - m_RegionRadius, kk=0; k<row + m_RegionRadius+1, kk<m_RegionSize; k+=1, ++kk)
	{
        double *pSSDCols = ssdMat.ptr<double>(kk); //猜测1：SSDq(x,y)是点q的ssd矩阵的位置
        for(int l = col - m_RegionRadius, ll=0; l<col + m_RegionRadius+1, ll<m_RegionSize; l+=1, ++ll)
        {
            //region内每个patch 5x5
            Rect patchROI2(l - m_PatchRadius, k- m_PatchRadius, m_PatchSize, m_PatchSize);
            Mat patch2, patchLab2;
            patch2 = m_SrcMat(patchROI2).clone();
            patch2.convertTo(patch2, CV_32F, 1.0 / 255);
            cvtColor(patch2, patchLab2, CV_BGR2Lab);

            //计算两幅相同大小patch的ssd 在CEI Lab色彩空间下计算
            double temp = CalSSDBetweenTwoMats(patchLab, patchLab2);
            pSSDCols[ll] = temp;
			count++;
        }
    }

	int c = count;

}

//计算两个矩阵的SSD type相同
int LSSD::CalSSDBetweenTwoMats(const Mat &mat1, const Mat &mat2)
{
    if(mat1.size() != mat2.size()){
        throw "this two mats should have same shape";
    }
    Mat diff = mat1 - mat2;
    int diffSq = diff.dot(diff);
    return diffSq;
}


//某个像素点的SSD转换为Correlation Surface type均为float或者double
void LSSD::TranSSDIntoCorrelationSurface(int row, int col, Mat &ssdMat, Mat &corrMat)
{
    if(ssdMat.depth()<5){
        ssdMat.convertTo(ssdMat, CV_64F);
    }

    if(corrMat.depth()<5){
        corrMat.convertTo(corrMat, CV_64F);
    }

    //var(noise)
    int varNoise = 25000;
    //计算var(auto)
    //int varAuto = CalcVarAutoofOnePixel(row, col, m_SrcMat, 1);

    //Correlation Surface Mat
    //int temp = max(varNoise, varAuto);//分母 分母越大效果越好 e+4
	int temp = 25000;
    Mat temp2 = ssdMat * (-1.0/temp);
	cv::exp(temp2, corrMat);

	/*Mat temp2, temp3;
	ssdMat.convertTo(temp2, CV_64F, 1.0 / 25000, 1);
	corrMat = 1.0 / temp2;*/

    
}


//计算某个像素点的var(auto)
int LSSD::CalcVarAutoofOnePixel(int row, int col, const Mat&inputMat, int patchRadius)
{
    //暂时先取100
    //return 1000;

    //以q为中心的半径为patchRadius的区域
    Rect patchROI(col - patchRadius, row - patchRadius,  2*patchRadius+1, 2*patchRadius+1);

    Mat patch = inputMat(patchROI).clone();//3x3

    vector<Mat> temp;
    for(int i=0;i<4;++i)
    {
        int x = i/2;
        int y = i%2;
        Mat temp1 = patch(Rect(x, y, 2, 2)).clone();
        temp.push_back(temp1);
    }

    vector<double> variacne;//方差

    for(int i=0; i<temp.size(); ++i)
    {
        for(int j=i+1; j<temp.size(); ++i)
        {
            Mat diff = temp[i] - temp[j];
            Mat meanV, stddev;
            meanStdDev(diff, meanV, stddev);
            Scalar temp = cv::mean(stddev);//取标准差的三个通道下的均值
            double tempVal = temp.val[0];
            variacne.push_back(tempVal*tempVal);
        }
    }

    double maxVariance = *std::max_element(variacne.begin(), variacne.end());

    return int(maxVariance + 0.5);//一般很大 (至少几百)  精度无需float
}

//从直角坐标系转换到对数极坐标系下
void LSSD::TranIntoLogPolar(const Mat &inputMat, Mat &outputMat, Mat &inverseMat)
{
    Point2f center( (float)inputMat.cols / 2, (float)inputMat.rows / 2 );
    double M = 56;
    logPolar(inputMat, outputMat, center, M, INTER_LINEAR + WARP_FILL_OUTLIERS);

    logPolar(outputMat, inverseMat, center, M, INTER_LINEAR + WARP_INVERSE_MAP);

}

//分解bins
void LSSD::PartBins(const Mat &inputMat, int angles, int radial, vector<double> &bins){

    /* 说明:
     * 对数极坐标得到的矩阵 行对应极径rho 列对应角度theta
     * 1.极径方向分为4个区间
     * 2.角度方向分为20个区间
     */

    int width = inputMat.cols;
    int height  = inputMat.rows;

    int radialSteps = (int) ceil(width /radial);
    int angleSteps =  (int) ceil(height /angles);

    int count = 0;
	Rect wholeMat(0, 0, width, height);//整幅图像的Rect
    for(int i=0; i<width-radialSteps; i+=radialSteps)//列
    {
        for(int j=0; j<height-angleSteps; j+=angleSteps)//行
        {
            Rect region(i, j, radialSteps, angleSteps);
            region = region & wholeMat;//交集处理防止边界溢出
            Mat temp = inputMat(region);
            double minVal, maxVal;
            Point minLoc, maxLoc;
            minMaxLoc(temp, &minVal, &maxVal, &minLoc, &maxLoc );//计算该mat内的最值
            bins.push_back(maxVal);//最大值作为该Bin的代替
            count++;
        }
    }


}

//去除无用的描述子
void LSSD::RemoveNonInformativeDescriptor(vector<LSSDescriptor> &inputDesc, vector<LSSDescriptor> &outputDesc){

    /* 思路:
     * 每个像素对应的descriptor包含80个bins, salient的描述子的bins都很小，homo的都很大
     * 因此算法流程如下:
     * 1.遍历所有descriptor
     * 2.对于当前descriptor，计算所有80个bins的min max
     * 3.若min<t1,则认为是salient; 若max>t2,则认为是homo
     * 4.将有效的descriptor进行存储
     */

    double t1 = 0.1;
    double t2 = 0.8;



    for(int i=0; i<inputDesc.size(); ++i)
    {
        LSSDescriptor currDescriptor = inputDesc[i];//当前descriptor 有80个bins

        double maxBin = *max_element(currDescriptor.value.begin(), currDescriptor.value.end());
        double minBin = *min_element(currDescriptor.value.begin(), currDescriptor.value.end());

        if(maxBin>t1 && minBin<t2){
            outputDesc.push_back(currDescriptor);//排除了salient和homo
        }
    }
}

//归一化描述子
void LSSD::NormaliseDescriptor(vector<LSSDescriptor> &inputDesc, vector<LSSDescriptor> &outputDesc){
    for(int i=0; i<inputDesc.size(); ++i)
    {
        LSSDescriptor currDescriptor = inputDesc[i];//当前descriptor 有80个bins

        double maxBin = *max_element(currDescriptor.value.begin(), currDescriptor.value.end());
        double minBin = *min_element(currDescriptor.value.begin(), currDescriptor.value.end());

        for(int k=0; k<m_Angles*m_Radials; k++){
            currDescriptor.value[k] = (currDescriptor.value[k] - minBin)/(maxBin-minBin);
        }
        outputDesc.push_back(currDescriptor);
    }
}

//计算两个描述子的相似性
double LSSD::SimilarityBetweenTwoDescriptor(const LSSDescriptor &lssd1, const LSSDescriptor &lssd2)
{
    //CV_ASSERT(lssd1.value.size() == lssd2.value.size());
    //1. sim(d1,d2) = -||d1-d2||p

    double sum = 0.0;
    for(int i=0; i<lssd1.value.size(); ++i)
    {
        double t = lssd1.value[i] - lssd2.value[i];
        sum += abs(t);
    }
    return (-1.0)*sum;
}


