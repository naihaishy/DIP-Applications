#include "tracker.h"


Tracker::Tracker()
{
	pause = false;
	selectObject = false;
	trackObject = 0;
	 
}






void Tracker::CamShiftTracker(string videoFile = ""){

	VideoCapture capture;
	
	if (videoFile.empty()) {
		capture.open(0);
	}
	else {
		capture.open(videoFile);
	}

	double rate = capture.get(CV_CAP_PROP_FPS);
	int delay = videoFile.empty()? 0: 1000 / rate;
    

    if (!capture.isOpened())
    {
        cout << "read video failure" << endl;
        return ;
    }

    //绘制ROI
 
	namedWindow("CamShift", 0);
    setMouseCallback("CamShift", mouseHandler, this);

	createTrackbar("Vmin", "CamShift", &vmin, 256, 0);
	createTrackbar("Vmax", "CamShift", &vmax, 256, 0);
	createTrackbar("Smin", "CamShift", &smin, 256, 0);

    Mat hMat;
	Mat hsv, hue, mask, hist, backproj;

	int hsize = 16;
	float hranges[] = { 0,180 };
	const float* phranges = hranges;
	Rect trackWindow;

	for (;;) {

		if (!pause) {
			capture.read(frame);
			if (frame.empty())
				break;
			frame.copyTo(srcMat);
		}


		if (!pause)
		{
			cvtColor(srcMat, hsv, COLOR_BGR2HSV);

			if (trackObject)//有需要跟踪的物体
			{

				inRange(hsv, Scalar(0, smin, MIN(vmin, vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
				int ch[] = { 0, 0 };
				hue.create(hsv.size(), hsv.depth());
				mixChannels(&hsv, 1, &hue, 1, ch, 1);


				Mat roi(hue, selection), maskroi(mask, selection);

				//计算直方图
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				//显示直方图
				ShowHist(hist);

				normalize(hist, hist, 0, 255, NORM_MINMAX);

				if (trackObject < 0) //如果需要跟踪的物体还没有进行属性提取，则进行选取框类的图像属性提取
				{
					
					trackWindow = selection;
					trackObject = 1;


				}
				//计算back projection
				calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
				backproj &= mask; 
				RotatedRect trackBox = CamShift(backproj, trackWindow, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1) );
			 

				if (trackWindow.area() <= 1)
				{
					int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
					trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,trackWindow.x + r, trackWindow.y + r) &Rect(0, 0, cols, rows);
				}

				//显示跟踪结果
				ellipse(srcMat, trackBox, Scalar(0, 0, 255), 3, LINE_AA);
		 

			}
		}
		else if (trackObject < 0) {
			pause = false;
		}
			
	

		//当处于选择物体时画出选择框
		if (selectObject && selection.width > 0 && selection.height > 0)
		{
			Mat roi = srcMat(selection);
			bitwise_not(roi, roi);
		}

		putText(srcMat, "ESC to exit and P to pause", Point(0, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 1);
		putText(srcMat, "Use mouse to select ROI", Point(0, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 1);
		imshow("CamShift", srcMat);

		char c = (char)waitKey(10);
		if (c == 27)
			break;
		switch (c)
		{
			case 'p':
				pause = !pause;
				break;
			case 'r':
				frame.copyTo(srcMat);
				imshow("CamShift", srcMat);
				break;
		}

	}


	destroyAllWindows();



}

void Tracker::mouseHandler(int event, int x, int y, int, void * userdata)
{
	Tracker *tracker = reinterpret_cast<Tracker*>(userdata);
	tracker->onMouse(event, x, y);
}

 
void Tracker::onMouse(int event, int x, int y)
{	 

	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);
		selection &= Rect(0, 0, frame.cols, frame.rows);
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		//鼠标按下 
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		selectObject = true;
		break;
	case CV_EVENT_LBUTTONUP:
		//鼠标松开
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			trackObject = -1;//如果有选择区域 则打开跟踪
		 
		break;
	}
	 

}


//显示直方图
void Tracker::ShowHist(const Mat &hist)
{
	Mat histimg = Mat::zeros(300, 500, CV_8UC3);
	histimg = Scalar::all(0);

	int hsize = 16;
	float hranges[] = { 0,180 };
	const float* phranges = hranges;

	int binW = histimg.cols / hsize;
	Mat buf(1, hsize, CV_8UC3);
	for (int i = 0; i < hsize; i++)
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / hsize), 255, 255);


	cvtColor(buf, buf, COLOR_HSV2BGR);

	for (int i = 0; i < hsize; i++)
	{
		int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows / 255);
		rectangle(histimg, Point(i*binW, histimg.rows),
			Point((i + 1)*binW, histimg.rows - val),
			Scalar(buf.at<Vec3b>(i)), -1, 8);
	}

	imshow("Hist", histimg);

}


 
