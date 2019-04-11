#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "kcftracker.h"

using namespace cv;
using namespace std;

RNG g_rng(12345);
bool isDrawRect = false;
Point LClicked = Point(-1, -1);
Point mouseLocation = Point(-1, -1);
bool isReady = false;

//void on_mouse(int event, int x, int y, int flags, void* ustc)
//{
//	Mat& image = *(cv::Mat*) ustc;//这样就可以传递Mat信息了，很机智
//	switch (event)
//	{
//	case CV_EVENT_LBUTTONDOWN://按下左键
//	{
//		isDrawRect = true;
//		LClicked = Point(x, y);
//		cout << "CV_EVENT_LBUTTONDOWN" << endl;
//	}   break;
//	case CV_EVENT_MOUSEMOVE://移动鼠标
//	{
//		mouseLocation = Point(x, y);
//		cout << "CV_EVENT_MOUSEMOVE" << endl;
//		if (isDrawRect)
//		{
//		}
//	}break;
//	case EVENT_LBUTTONUP:
//	{
//		cout << "EVENT_LBUTTONUP" << endl;
//
//		isDrawRect = false;
//		//调用函数进行绘制
//		cv::rectangle(image, LClicked, mouseLocation, cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)));//随机颜色
//		isReady = true;
//	}break;
//
//	default:
//		break;
//	}
//}
//
//
//int main()
//{
//
//	//system("color 9F");
//	//setMouseCallback("img", on_mouse, 0);//调用回调函数
//
//	bool HOG = true;
//	bool FIXEDWINDOW = false;
//	bool MULTISCALE = true;
//	bool SILENT = false;
//	bool LAB = true;//true
//
//					// Create KCFTracker object
//	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
//
//	// Frame readed
//	Mat frame;
//
//	// Tracker results
//	Rect result;
//
//	// Frame counter
//	int nFrames = 0;
//
//	float xMin = 150;
//	float yMin = 164;
//	float width = 100;
//	float height = 150;
//
//	/*VideoCapture cam(0);*/
//	std::string video = "1111.mp4";
//	VideoCapture cam(video);
//	/*
//	string outFlie = "E:/3.avi";
//	VideoWriter write;
//	//获得帧的宽高
//	int w = static_cast<int>(1920);//cam.get(CV_CAP_PROP_FRAME_WIDTH));
//	int h = static_cast<int>(1080);//cam.get(CV_CAP_PROP_FRAME_HEIGHT));
//	Size S(w, h);
//	//获得帧率
//	double r = cam.get(CV_CAP_PROP_FPS);
//	//打开视频文件，准备写入
//	write.open(outFlie, -1, r, S, true);
//	*/
//
//	//double rate = cam.get(CV_CAP_PROP_FPS);
//	//获取视频帧的尺寸
//	//     int Width = cam.get(CV_CAP_PROP_FRAME_WIDTH);
//	//     int Height = cam.get(CV_CAP_PROP_FRAME_HEIGHT);
//	//根据打开视频的参数初始化输出视频格式
//	//     cv::VideoWriter w_cap("E:/3.avi", CV_FOURCC('M', 'J', 'P', 'G'), rate, cv::Size(Width, Height));
//
//
//	while (1)
//	{
//		Mat temp1;
//		Mat temp2;
//
//		if (!cam.isOpened())
//		{
//			exit(0);
//		}
//
//		cam >> frame;
//
//
//
//		if (isReady)
//		{
//			nFrames = 0;
//
//			xMin = LClicked.x;
//			yMin = LClicked.y;
//			width = mouseLocation.x - LClicked.x;
//			height = mouseLocation.y - LClicked.y;
//
//		}
//
//		isReady = false;
//
//		int j = 0;
//		// First frame, give the groundtruth to the tracker
//		if (nFrames == 0) {
//			tracker.init(Rect(xMin, yMin, width, height), frame);
//			rectangle(frame, Point(xMin, yMin), Point(xMin + width, yMin + height), Scalar(0, 255, 255), 2, 8);
//			//tracker.init( Rect(LClicked.x, LClicked.y, mouseLocation.x, mouseLocation.y), frame );
//		}
//		// Udate
//		else
//		{
//			result = tracker.update(frame);
//			rectangle(frame, Point(result.x, result.y), Point(result.x + result.width, result.y + result.height), Scalar(0, 255, 255), 2, 8);
//
//			int centerX = result.x + result.width / 2.0;
//			int centerY = result.y + result.height / 2.0;
//
//			int frame_width = frame.cols / 2;
//			int frame_height = frame.rows / 2;
//
//			int errorX = frame_width - centerX;
//			int errorY = frame_height - centerY;
//
//
//			line(frame, Point(centerX, centerY), Point(frame_width, frame_height), cv::Scalar(0, 0, 255));
//
//		}
//
//		nFrames++;
//
//		if (!SILENT)
//		{
//			namedWindow("Image");
//			imshow("Image", frame);
//			setMouseCallback("Image", on_mouse, &frame);  //注册鼠标相应回调函数
//
//			waitKey(1);
//		}
//	}
//
//	return 0;
//}



int main1(int argc, char *argv[])
{
	// declares all required variables
	//! [vars]
	Rect2d roi;
	Rect result;
	Mat frame;
	//! [vars]

	// create a tracker object
	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = false;
	bool LAB = true;//true
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	//! [create]

	// set input video
	//! [setvideo]
	//std::string video = "testHuman9.mp4";
	cout << "video name" << endl;
	string video;
	getline(cin, video);
	if (video.empty())
	{
		cout << "error name" << endl;
		video = "0.mp4";
	}
	else
	{
		video = video + ".mp4";
	}

	VideoCapture cap(video);
	//! [setvideo]

	// get bounding box
	//! [getframe]
	cap >> frame;
	//! [getframe]
	//! [selectroi]选择目标roi以GUI的形式
	medianBlur(frame, frame, 3);
	//Mat roi_tmp;
	//frame.copyTo(roi_tmp);
	roi = selectROI("tracker", frame);
	//! [selectroi]

	//quit if ROI was not selected
	if (roi.width == 0 || roi.height == 0)
	{
		cap.release();
		destroyAllWindows();
		return 0;
	}

	// initialize the tracker
	//! [init]
	tracker.init(Rect(roi), frame);
	//! [init]

	// perform the tracking process
	printf("Start the tracking process\n");
	for (;; ) {
		// get frame from the video
		cap >> frame;
		medianBlur(frame, frame, 3);
		// stop the program if no more images
		if (frame.rows == 0 || frame.cols == 0)
			break;

		//time
		double time_start = getTickCount();

		// update the tracking result
		//! [update]
		result = tracker.update(frame);
		// draw the tracked object
		rectangle(frame, Point(result.x, result.y), Point(result.x + result.width, result.y + result.height), Scalar(0, 255, 255), 2, 8);

		double time_end = getTickCount();
		cout << "time of one process" << (time_end - time_start) * 1000 / (getTickFrequency()) << "ms" << endl;
		//rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);

		// show image with the tracked object
		imshow("tracker", frame);
		//! [visualization]
		//quit on ESC button
		if (waitKey(10) == 27)
			break;
	}
	cap.release();
	destroyAllWindows();
	system("pause");
	return 0;
}
