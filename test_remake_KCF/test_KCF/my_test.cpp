#include <stdio.h>
#include <iostream>

#include "kcftracker.hpp"
#include "KalmanTracker.h"

#include "opencv2/opencv.hpp"
#include "opencv2/video.hpp"
using namespace cv;
using namespace std;
#include <chrono>

int main()
{

	cv::VideoCapture capture;
	cv::Mat frame;
	//capture.open("G:/track/baseline/遮挡/清晰中距物体的运动_摄像头平移.avi");
	//capture.open("G:/track/baseline/小目标/小尺寸物体平移遮挡_相机无晃动2_Trim.mp4");
	capture.open("D:/360MoveData/Users/cjy/Desktop/video/1.avi");
	//capture.open("G:/track/baseline/car/IMG_1557_Trim.mp4"); //IMG_1550  IMG_1557_Trim.mp4
	
	//capture.open("G:/track/baseline/遮挡/中距物体移动_相机旋转_部分遮挡.avi");
	//capture.read(frame);

	if (!capture.isOpened())
	{
		printf("can not open ...\n");
		return -1;
	}
	cv::Mat firstFrame;
	////获取视频的第一帧,并框选目标
	capture.read(firstFrame);
	int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	std::cout << "cap width:" << width << "cap height" << height << endl;
	if (firstFrame.empty()) return -1;
	int count = 1;
	while (cv::waitKey(0) != 27) {
		capture.read(firstFrame);
		imshow("output_lu", firstFrame);
		count += 1;
	}
	//IME::TemplateTracker tracker;
	//cv::Rect roi;

	if (firstFrame.empty()) return -1;
	cv::Rect bbox = cv::selectROI("output_lu", firstFrame, true, false);
	//cv::Rect bbox(718, 585, 32, 30);

	if (bbox.empty()) return -1;
	printf("first bbox width:%d height%d\n", bbox.width, bbox.height);
	printf("first bbox center: %d %d\n", (bbox.x + bbox.width / 2), (bbox.y + bbox.height / 2));

	KalmanTracker KF(bbox,4,2);

	cv::Mat grayframe;
	cv::cvtColor(firstFrame, grayframe, CV_BGR2GRAY);

	//uchar* pdata = grayframe.ptr<uchar>(0);
	int w = firstFrame.cols;
	int h = firstFrame.rows;
	KCFTracker tracker;
	tracker.init(bbox, grayframe);

	FILE* fp;
	fp = fopen("bbox.txt","w+");
	while (capture.read(frame))
	{
		cv::cvtColor(frame, grayframe, CV_BGR2GRAY);

		auto start = chrono::system_clock::now();
		bbox = tracker.update(grayframe, KF.peak_value, bbox, KF.count_change_scale, KF.peak_value_thro);
		printf("x:%d  y:%d \n", bbox.x, bbox.y);
		fprintf(fp, "x:%d  y:%d \n", bbox.x,bbox.y);
		auto end = chrono::system_clock::now();
		auto dur = chrono::duration_cast<chrono::microseconds>(end - start);
		std::cout << "all times::" << double(dur.count())*chrono::microseconds::period::num / chrono::microseconds::period::den << "s" << std::endl;
		//printf("rect size is (%d, %d)\n", bbox.width, bbox.height);

		KF.Update(bbox);

		//5.plot real and prediction
		std::printf("预测值statePre: %d %d\n预估值statePost: %d %d\n真实值measurement: %d %d\n",
			KF.predict_pt.x, KF.predict_pt.y,
			KF.correction_pt.x, KF.correction_pt.y,
			KF.measurement_pt.x, KF.measurement_pt.y);
		Point tmp_line = Point((KF.correction_pt.x - KF.bbox_pre.x) * 10, (KF.correction_pt.y - KF.bbox_pre.y) * 10);
		line(frame, KF.bbox_pre, KF.correction_pt + tmp_line, Scalar(255, 255, 255), 1);
		circle(frame, KF.correction_pt + tmp_line, 2, Scalar(0, 0, 0), 4);

		//circle(frame, KF.measurement_pt, 2, Scalar(0, 0, 255), 4);
		rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
		imshow("output_lu", frame);
		if (cv::waitKey(0) == 27)
		{
			capture.release();
			cv::destroyWindow("output_lu_lu");
		}
	}

	fclose(fp);
	std::system("pause");
	return 0;
}



int main2()
{

	cv::VideoCapture capture;
	cv::Mat frame;
	capture.open("G:/track/baseline/car/IMG_1551.MOV");

	//capture.read(frame);


	if (!capture.isOpened())
	{
		printf("can not open ...\n");
		return -1;
	}
	cv::Mat firstFrame;
	////获取视频的第一帧,并框选目标
	capture.read(firstFrame);
	int count = 1;
	while (count < 50) {
		capture.read(firstFrame);
		count += 1;
	}
	cv::Rect bbox;
	KCFTracker tracker;
	//IME::TemplateTracker tracker;
	//cv::Rect roi;

	if (!firstFrame.empty())
	{
		bbox = cv::selectROI("output_lu",firstFrame, true, false);
		cv::waitKey();
	}
	//roi.x = bbox.x;
	//roi.y = bbox.y;
	//roi.width = bbox.width;
	//roi.height = bbox.height;



	cv::Mat grayframe;
	cv::cvtColor(firstFrame, grayframe, CV_BGR2GRAY);

	//uchar* pdata = grayframe.ptr<uchar>(0);
	int w = firstFrame.cols;
	int h = firstFrame.rows;
	tracker.init(bbox, grayframe);

	while (capture.read(frame))
	{

		cv::cvtColor(frame, grayframe, CV_BGR2GRAY);
		uchar* pdata = grayframe.ptr<uchar>(0);

		//float start = cv::getTickCount();
		auto start = chrono::system_clock::now();
		bbox = tracker.update(grayframe);
		//bbox.x = r.x;
		//bbox.y = r.y;
		//bbox.width = r.width;
		//bbox.height = r.height;

		auto end = chrono::system_clock::now();
		//float end = cv::getTickCount();
		auto dur = chrono::duration_cast<chrono::microseconds>(end - start);
		std::cout << "all times::" << double(dur.count())*chrono::microseconds::period::num / chrono::microseconds::period::den << "s" << std::endl;

		printf("rect size is (%d, %d)\n", bbox.width, bbox.height);

		rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
		imshow("output_lu", frame);
		if (cv::waitKey(0) == 27)
		{
			capture.release();
			cv::destroyWindow("output_lu");
		}
	}

	return 0;
}




