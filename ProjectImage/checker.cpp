#include "checker.h"

#define STATUS_UNKNOW -1
#define STAND_UP 0
#define SIT_DOWN 1
#define SIT 2
#define STAND 3

#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()




//使用人脸检测初始化ROI
void faceInitRegion(Mat image, vector<Rect> &faces)
{
	//resize(Image, Image, Size(Image.cols / 2, Image.rows / 2), 0, 0, INTER_LINEAR);
	Mat grayImage;
	cvtColor(image, grayImage, CV_BGR2GRAY);

	CascadeClassifier faces_cascade;
	faces_cascade.load("haarcascade_frontalface_alt.xml");
	faces_cascade.detectMultiScale(grayImage, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE | CV_HAAR_DO_CANNY_PRUNING, Size(15, 15));

	Mat hhh = image.clone();
	for (int i = 0;i < faces.size();i++)
		rectangle(hhh, faces[i], Scalar(0, 255, 0), 2, 8, 0);

	imwrite("face.jpg", hhh);

	return;
}


// initialize the kcf tracker 
void initTrackers(Mat &frame, vector<Rect> &rois, vector<KCFTracker> &trackers, vector<Scalar> &colors)
{
	RNG rng(time(0));//随机数发生器
	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;
	bool LAB = false;

	for (size_t i = 0; i < rois.size(); i++)
	{
		//KCFTracker初始化
		KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
		tracker.init(rois[i], frame);
		trackers.push_back(tracker);

		//随机分配颜色
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		colors.push_back(color);
		//画图
		rectangle(frame, rois[i], color, 2, 1);
	}
}

//多个kcf tracker并行计算 独立地
class trackerUpdateComputer :public ParallelLoopBody
{
public:

	trackerUpdateComputer(
		Mat &_image,
		std::vector<KCFTracker> &_trackerList,
		std::vector<Rect> &_objects,
		std::vector<Scalar> &_colors,
		std::vector<int> &_status,
		std::vector<int> &_oriStatus,
		std::vector<Point> &_oriPosition,
		std::vector<Point> &_position,
		std::vector<Point> &_exPosition) :
		trackerList(_trackerList),
		objects(_objects),
		image(_image),
		colors(_colors),
		status(_status),
		oriStatus(_oriStatus),
		oriPosition(_oriPosition),
		position(_position),
		exPosition(_exPosition) { }

	void operator()(const cv::Range& range) const
	{
		const int begin = range.start;
		const int end = range.end;

		for (int i = begin; i<end; i++)
		{
			objects[i] = trackerList[i].update(image);//若返回false表明目标物体不在画面中

			if (!objects[i].empty()) //检测到指定目标
			{
				rectangle(image, objects[i], colors[i], 2, 1);
				//更新position
				position[i] = Point(objects[i].x + objects[i].width / 2, objects[i].y + objects[i].height / 2);
				//最高点 或者 最低点
				if (position[i].y > oriPosition[i].y && position[i].y > exPosition[i].y)
				{
					exPosition[i] = position[i];
				}
				if (position[i].y < oriPosition[i].y && position[i].y < exPosition[i].y)
				{
					exPosition[i] = position[i];
				}
			}
			else
			{
				//没有检测到指定目标 目标离开了
			}
		}
	}

private:
	Mat & image;
	std::vector<KCFTracker> &trackerList;
	std::vector<Rect> &objects;
	std::vector<Scalar> &colors;
	std::vector<int> &status;
	std::vector<int> &oriStatus;
	std::vector<Point> &oriPosition;
	std::vector<Point> &position;
	std::vector<Point> &exPosition;
};

//串行计算
void updateTracker(Mat &image, vector<KCFTracker> &trackerList, vector<Rect> &objects, vector<Scalar> &colors, vector<Point> &position)
{
	for (int i = 0; i<trackerList.size(); i++)
	{
		objects[i] = trackerList[i].update(image);//若返回false表明目标物体不在画面中

		if (!objects[i].empty()) //检测到指定目标
		{
			rectangle(image, objects[i], colors[i], 2, 1);
			//更新position
			position[i] = Point(objects[i].x + objects[i].width / 2, objects[i].y + objects[i].height / 2);
		}
		else
		{
			//没有检测到指定目标 目标离开了
		}
	}
}


void confInitStatus(vector<Point> &oriPosition, vector<Point> &position, vector<int> &oriStatus)
{
	for (size_t i = 0; i < position.size(); i++)
	{
        if (oriStatus[i] == STATUS_UNKNOW && abs(position[i].y - oriPosition[i].y) >80)
		{
			if (position[i].y > oriPosition[i].y)
			{
				oriStatus[i] = STAND;
			}
			else {
				oriStatus[i] = SIT;
			}
		}
	}
}


void updateStatus(Mat &frame,
	vector<Point> &oriPosition, vector<Point> &prevPosition, vector<Point> &position,
	vector<Point> &exPosition, vector<int> &oriStatus, vector<int> &status)
{
	for (size_t i = 0; i < position.size(); i++)
	{
		if (abs(prevPosition[i].y - position[i].y) > 30)//y状态发生改变 
		{
			if (position[i].y - prevPosition[i].y > 30)
			{
				status[i] = SIT_DOWN;
			}
			if (prevPosition[i].y - position[i].y > 30)
			{
				status[i] = STAND_UP;
			}
		}
		else //y状态未发生改变 纠正
		{
			if (abs(position[i].y - oriPosition[i].y) < 50)//初始位置
			{
				status[i] = oriStatus[i];
			}
			else if (abs(position[i].y - exPosition[i].y) < 50)//最高点 或者最低点位置 与初始状态相反
			{
				if (oriStatus[i] == SIT)
					status[i] = STAND;
				else
					status[i] = SIT;
			}
		}
	}
}



void showInfo(Mat &image, vector<Point> &position, vector<int> &oriStatus, vector<int> &status)
{
	for (size_t i = 0; i < position.size(); i++)
	{
		if (oriStatus[i] != STATUS_UNKNOW)
		{
			switch (status[i])
			{
			case STAND_UP:
				putText(image, "Stand Up", position[i], FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);
				break;
			case SIT_DOWN:
				putText(image, "Sit Down", position[i], FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
				break;
			case STAND:
				putText(image, "Stand", position[i], FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 0), 2);
				break;
			case SIT:
				putText(image, "Sit", position[i], FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 255), 2);
				break;
			}
		}
	}

}



 
Checker::Checker()
{

}

Checker::Checker(string _videopath)
{
    videopath = _videopath;
}






//检测状态
int Checker::doing()
{
	// set input video
	VideoCapture cap(videopath);

	if (!cap.isOpened())
		cerr << "open video failed" << std::endl;

	double rate = cap.get(CV_CAP_PROP_FPS);
	int delay = 1000 / rate;

	Mat frame;

	// get bounding box
	cap >> frame;
	vector<Rect> ROIs;
	//selectROIs("tracker", frame, ROIs); //人工标记
	faceInitRegion(frame, ROIs);//人脸检测 自动标记


	//quit when the tracked object(s) is not provided
	if (ROIs.size()<1)
		return -1;


	// initialize the kcf tracker 
	vector<KCFTracker> trackers;
	vector<Scalar> colors;
	initTrackers(frame, ROIs, trackers, colors);


	// initialize the status and position 
	vector<int> status(ROIs.size(), STATUS_UNKNOW);//每个人的状态
	vector<Point> position;//矩形框中心点位置
	for (size_t i = 0; i < ROIs.size(); i++)
	{
		Point center = Point(ROIs[i].x + ROIs[i].width / 2, ROIs[i].y + ROIs[i].height / 2);
		position.push_back(center);
	}
	vector<int> oriStatus = status;//每个人的初始状态
	vector<Point> oriPosition = position;//矩形框中心点初始位置
	vector<Point> exPosition = position;//最高点 或者最低点

	int interval = 0;//隔 10帧 更新一次状态
	vector<Point> prevPosition = position;

	// do the tracking
	printf("Start the tracking process, press ESC to quit.\n");
	for (;; ) {
		double timer = (double)getTickCount();

		// get frame from the video
		cap >> frame;

		// stop the program if no more images
		if (frame.rows == 0 || frame.cols == 0)
			break;

		//update the tracking result 
		parallel_for_(Range(0, static_cast<int>(trackers.size())),
			trackerUpdateComputer(frame, trackers, ROIs, colors, status, oriStatus, oriPosition, position, exPosition));

		//roi非重叠处理


		//updateTracker(frame, trackers, ROIs, colors, position);
		//confirm init status 确定初始状态 stand 还是 sit
		confInitStatus(oriPosition, position, oriStatus);

		//update status of every person
		interval++;
		if (interval == 10)
		{
			interval = 0;
			updateStatus(frame, oriPosition, prevPosition, position, exPosition, oriStatus, status);
			prevPosition = position;

		}

		showInfo(frame, position, oriStatus, status);


		//calculate the fps 
		float fps = getTickFrequency() / ((double)getTickCount() - timer);

		//show the fps 
		putText(frame, "FPS : " + SSTR(int(fps)), Point(0, 30), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

		// show image with the tracked object
		imshow("tracker", frame);


		char c = (char)waitKey(delay);
		if (c == 27)
			break;


	}

	destroyAllWindows();
	return 0;
}
