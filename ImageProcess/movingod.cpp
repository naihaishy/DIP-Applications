#include "movingod.h"
//Moving objecct detection



MovingOD::MovingOD()
{

}

//
void MovingOD::FrameDiffDetection(string videoPath)
{
    VideoCapture capture(videoPath);

    if (!capture.isOpened()) {
        QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("read video failure\n"));
        return;
    }

    double rate = capture.get(CV_CAP_PROP_FPS);//获取视频帧率

    int delay = 1000 / rate;


    Mat framepro, frame, dframe, grayImage;

    bool flag = false;

   

    while (capture.read(frame)){
        if (false == flag){
            framepro = frame.clone();//将第一帧图像拷贝给framePro
            flag = true;
        }
        else{
            absdiff(frame, framepro, dframe);//帧间差分计算两幅图像各个通道的相对应元素的差的绝对值。
            framepro = frame.clone();//将当前帧拷贝给framepro
            threshold(dframe, dframe, 60, 255, CV_THRESH_BINARY);//阈值分割
            cvtColor(dframe, grayImage, CV_BGR2GRAY);

            imshow("foreground", grayImage);

            imshow("origin", frame);

            if (waitKey(delay) >= 0)
            {
                break;
            }
        }
    }
	destroyAllWindows();
    frame.release();
}


void MovingOD::GMMDetection(string videoPath)
{

    VideoCapture capture(videoPath);
    if (!capture.isOpened()) {
        QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("read video failure\n"));
        return;
    }

    /******************************************************/
    // YOUR CODE HERE :
    //声明一个类型为BackgroundSubtractorMOG2，名称为mog的 变量，调整其中的参数并查看效果
    //history 100  varThreshold  detectShadow
    Ptr<cv::BackgroundSubtractorMOG2> mog = createBackgroundSubtractorMOG2(100, 25, false);


    /******************************************************/

    Mat foreground;
    Mat background;

    Mat frame;
    while (capture.read(frame))
    {
        //Mat ff;
        //resize(frame, ff, cvSize(frame.cols / 2, frame.rows / 2));

        /******************************************************/
        // YOUR CODE HERE :
        //使用BackgroundSubtractorMOG2类的()运算符更新背景，找到前景
        mog->apply(frame, foreground);
 
        /******************************************************/

        // 腐蚀
        erode(foreground, foreground, Mat());
        // 膨胀
        dilate(foreground, foreground, Mat());

        mog->getBackgroundImage(background);   // 返回当前背景图像

        imshow("video", frame);
        imshow("foreground", foreground);
        imshow("background", background);


        if (waitKey(25) >= 0)
        {
			frame.release();
            break;
        }

    }

	destroyAllWindows();
    foreground.release();
    background.release();
}


void MovingOD::SGMDetection(string videoPath)
{
    VideoCapture capture(videoPath);
    if (!capture.isOpened()) {
        QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("read video failure\n"));
        return;
    }


    double alpha = 0.5;//背景建模alpha值
    double std = 20;//初始化标准差
    double var = std*std;//初始化方差
    double lamda = 2.5*1.2;//背景更新参数


    Mat foreground;
    Mat background;

    Mat frame, frameU, frameStd, frameVar;
    capture.read(frame);

    Mat frameTemp = Mat::zeros(frame.rows, frame.cols, CV_64FC3);
    //取前10帧计算均值和方差
    int count = 0;
    vector<Mat> frame10;
    while (capture.read(frame))
    {
        if(count == 10){
            frameU = frameTemp /10; //均值矩阵
            Mat accum = Mat::zeros(frame.rows, frame.cols, CV_64FC3);
            std::for_each(frame10.begin(), frame10.end(), [&](Mat &mat){
                accum += (mat - frameU).mul(mat - frameU);
            });

            cv::sqrt(accum/9, frameStd);//标准差均值
            frameVar = accum/10;//方差矩阵

            break;
        }else{
            Mat temp;
            frame.convertTo(temp, CV_64FC3);
            frameTemp +=temp;
            frame10.push_back(temp);
            count ++;
        }
    }

    background = frameU.clone();
 
    while (capture.read(frame))
    {
        Mat a = frame - background;
        Mat b = Mat::zeros(frame.rows, frame.cols, CV_64FC3);
        b = a.mul(a);
    }



    foreground.release();
    background.release();

}

void MovingOD::SGMTest(string videoPath)
{
    VideoCapture capture(videoPath);
    if (!capture.isOpened()) {
        QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("read video failure\n"));
        return;
    }

    Mat frame, frame_u, frame_d, frame_var, frame_std;

    //第一帧
    capture.read(frame);
	//frame.convertTo(frame, CV_64FC3);

    double alpha = 0.5;//背景建模alpha值
    double std_int = 20;//初始化标准差
    double var_int = std_int*std_int;//初始化方差
    double lamda = 2.5*1.2;//背景更新参数

    //初始化frame_u,frame_d,frame_var,frame_std
    frame_u = frame.clone(); //均值 第一帧的拷贝
    frame_d = Mat::zeros(frame.rows, frame.cols, CV_64FC3);//前景
    frame_var = Mat(frame.rows, frame.cols, CV_64FC3, Scalar(var_int, var_int,var_int));//方差
    frame_std = Mat(frame.rows, frame.cols, CV_64FC3, Scalar(std_int, std_int, std_int));//标准差

    while (capture.read(frame))
    {
		//frame.convertTo(frame, CV_64FC3);
        //单高斯背景更新
        for(int y=0; y<frame.rows; ++y)
        {
			Vec3b *pixelCols = frame.ptr<Vec3b>(y);
			Vec3b *pixel_uCols = frame_u.ptr<Vec3b>(y);
			Vec3b *pixel_dCols = frame_d.ptr<Vec3b>(y);
			Vec3b *pixel_stdCols = frame_std.ptr<Vec3b>(y);
			Vec3b *pixel_varCols = frame_var.ptr<Vec3b>(y);
            for(int x=0; x<frame.cols; ++x)
            {
                Vec3b pixel = pixelCols[x];
                Vec3b pixel_u = pixel_uCols[x];
                Vec3b pixel_d = pixel_dCols[x];
                Vec3b pixel_std = pixel_stdCols[x];
                Vec3b pixel_var = pixel_varCols[x];

                //|I-u|<lamda*std时认为是背景，进行更新
                if(fabs(pixel[0]-pixel_u[0])<lamda*pixel_std[0] &&
                        fabs(pixel[1]-pixel_u[1])<lamda*pixel_std[1] &&
                        fabs(pixel[2]-pixel_u[2])<lamda*pixel_std[2] )
                {
                    //更新期望u=(1-alpha)*u+alpha*I
                    pixel_u[0] =(1-alpha)*pixel_u[0] + alpha*pixel[0];
                    pixel_u[1] =(1-alpha)*pixel_u[1] + alpha*pixel[1];
                    pixel_u[2] =(1-alpha)*pixel_u[2] + alpha*pixel[2];

                    //更新方差var=(1-alpha)*var+alpha*(I-u)^2
                    pixel_var[0] = (1-alpha)*pixel_var[0] + alpha*(pixel[0]-pixel_u[0])*(pixel[0]-pixel_u[0]);
                    pixel_var[1] = (1-alpha)*pixel_var[1] + alpha*(pixel[1]-pixel_u[1])*(pixel[1]-pixel_u[1]);
                    pixel_var[2] = (1-alpha)*pixel_var[2] + alpha*(pixel[2]-pixel_u[2])*(pixel[2]-pixel_u[2]);

                    //更新标准差
                    pixel_std[0] = sqrt(pixel_var[0]);
                    pixel_std[1] = sqrt(pixel_var[1]);
                    pixel_std[2] = sqrt(pixel_var[2]);

                    //写入矩阵
					//pixel_uCols[x] = pixel_u;
					pixel_stdCols[x] = pixel_std;
					pixel_varCols[x] = pixel_var;

      
                }
                else
                {
                    pixel_d[0] = pixel[0]-pixel_u[0];
                    pixel_d[1] = pixel[1]-pixel_u[1];
                    pixel_d[2] = pixel[2]-pixel_u[2];
					pixel_dCols[x] = pixel_d;
                   
                }
            }

			
        }
		//更新背景
		frame_u = 0.6 * frame_u + (1 - 0.6)*frame;
		frame_d = frame - frame_u;
        //显示结果
		Mat grayImage;
		cvtColor(frame_d, grayImage, CV_BGR2GRAY);
        cv::threshold(grayImage, grayImage, 50, 255, CV_THRESH_BINARY);//阈值分割

		cv::imshow("origin", frame);
		cv::imshow("background",frame_u);
		cv::imshow("foreground", grayImage);

        if (waitKey(25) >= 0)
        {
            break;
        }
    }


	destroyAllWindows();

}
