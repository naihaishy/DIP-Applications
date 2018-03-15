//  基于混合高斯模型的运动目标检测

#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>

int main(int argc, char** argv)
{
	std::string videoFile = "768x576.avi";

	cv::VideoCapture capture;
	capture.open(videoFile);

	if (!capture.isOpened())
	{
		std::cout << "read video failure" << std::endl;
		return -1;
	}

	/******************************************************/
	// YOUR CODE HERE :
	//声明一个类型为BackgroundSubtractorMOG2，名称为mog的 变量，调整其中的参数并查看效果
	//history 100  varThreshold  detectShadows
	cv::Ptr<cv::BackgroundSubtractorMOG2> mog = cv::createBackgroundSubtractorMOG2(100, 25, false);


	/******************************************************/

	cv::Mat foreground;
	cv::Mat background;

	cv::Mat frame;
	long frameNo = 0;
	while (capture.read(frame))
	{
		++frameNo;

		std::cout << frameNo << std::endl;
		cv::Mat ff;
		resize(frame, ff, cvSize(frame.cols / 2, frame.rows / 2));

		/******************************************************/
		// YOUR CODE HERE :
		//使用BackgroundSubtractorMOG2类的()运算符更新背景，找到前景
		mog->apply(ff, foreground);


		/******************************************************/


		// 腐蚀
		cv::erode(foreground, foreground, cv::Mat());

		// 膨胀
		cv::dilate(foreground, foreground, cv::Mat());

		mog->getBackgroundImage(background);   // 返回当前背景图像

		cv::imshow("video", ff);
		cv::imshow("foreground", foreground);
		cv::imshow("background", background);


		if (cv::waitKey(25) >= 0)
		{
			break;
		}
	}



	return 0;
}