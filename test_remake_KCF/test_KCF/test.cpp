#if 0

//#include<sys/time.h>
#include "interface.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void move(int y, unsigned char* pdata)
{
 memset(pdata, 0, 600*600);
 for (int i = 10; i < 40;i++)
 {
   pdata[y * 600 + i] = 128;
   pdata[(y+1) * 600 + i] = 128;
   pdata[(y+2) * 600 + i] = 128;
 }
}


int main()
{
//test();

#if 1
// prepare frame data with YUV420 planar,
// normally, input with Y channel only 
unsigned char* pdata = (unsigned char*)malloc(600*600);
move(10, pdata);

// initial tracker
void* handle = initial_tracker();
printf(">>>>initial finished<<<<\n");
// set initial rectangle position
RoiRect rect_t;
rect_t.x = 10;
rect_t.y = 10;
rect_t.w = 40;
rect_t.h = 40;
int flag = 1;

// normally, feature enhance, open it 
bool isEnhance = true;
//struct  timeval start;
//struct  timeval end;
//unsigned  long diff;
// do cycle here
for (int i = 0;i < 100;i++)
{
  if (i > 1) flag = 0; 
  //gettimeofday(&start,NULL);
printf(">>>>process begin<<<<\n");
  do_tracker(pdata, 600, 600, flag, isEnhance, rect_t,  handle);
printf(">>>>process finished<<<<\n");
  //gettimeofday(&end,NULL);
  //diff = 1000 * (end.tv_sec-start.tv_sec)+ (end.tv_usec-start.tv_usec)/1000;

  printf("-----(%f,%f,%f,%f)-----\n",rect_t.x,rect_t.y,rect_t.w,rect_t.h);
  //printf("-----time cost (%ld)-----\n",diff);
  move(10+i, pdata);
}

// finally, free the trakcer, attention!!
free_tracker(handle);
free(pdata);
#endif
return 0;
}
#endif



// testOnly.cpp : 定义控制台应用程序的入口点。
//
#if 1

#include <stdio.h>
#include <iostream>

//#include "matrix.h"
//#include "FFT2D.h"
//#include "fhog.hpp"
#include "kcftracker.hpp"



#include "opencv2/opencv.hpp"
#include "opencv2/video.hpp"
using namespace cv;
using namespace std;
#include <chrono>


#define DEBUG  0

float invSqrt(float x)
{
	float hf = 0.5f*x;
	int i = *(int*)&x;
	i = 0x5f375a86 - (i >> 1);
	x = *(float*)&i;
	x = x*(1.5 - hf*x*x);
	x = x*(1.5 - hf*x*x);
	x = x*(1.5 - hf*x*x);
	return x;
}

//#include <windows.h>  

//#include "opencv/cv.h"
//using namespace cv;
int main1()
{

	cv::VideoCapture capture;
	cv::Mat frame;
	//capture.open("G:/track/baseline/遮挡/清晰中距物体的运动_摄像头平移.avi");
	capture.open("G:/track/baseline/小目标/小尺寸物体平移遮挡_相机无晃动2_Trim.mp4");
	//capture.open("G:/track/baseline/1.mp4");
	//capture.read(frame);

	if (!capture.isOpened())
	{
		printf("can not open ...\n");
		return -1;
	}
	cv::Mat firstFrame;
	////获取视频的第一帧,并框选目标
	capture.read(firstFrame);
	if (firstFrame.empty()) return -1;
	int count = 1;
	while (count < 50) {
		capture.read(firstFrame);
		count += 1;
	}
	//IME::TemplateTracker tracker;
	//cv::Rect roi;

	if (firstFrame.empty()) return -1;
	//cv::Rect bbox = cv::selectROI("output",firstFrame, true, false);
	cv::Rect bbox(718, 585, 32, 30);

	if (bbox.empty()) return -1;
	printf("first bbox: %d %d\n", (bbox.x + bbox.width/2), (bbox.y + bbox.height/2));
	//roi.x = bbox.x;
	//roi.y = bbox.y;
	//roi.width = bbox.width;
	//roi.height = bbox.height;

	/****************************************
	kalman 参数设置
	****************************************/
	//1.kalman filter setup
	const int stateNum = 4;                                      //状态值4×1向量(x,y,△x,△y)
	const int measureNum = 2;                                    //测量值2×1向量(x,y)	
	KalmanFilter KF(stateNum, measureNum, 0);
	KF.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0,   0, 1, 0, 1,   0, 0, 1, 0,   0, 0, 0, 1);  //转移矩阵A
	setIdentity(KF.measurementMatrix);                                             //测量矩阵H
	setIdentity(KF.processNoiseCov, Scalar::all(1e-5));                            //系统噪声方差矩阵Q
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));                        //测量噪声方差矩阵R
	setIdentity(KF.errorCovPost, Scalar::all(1));                                  //后验错误估计协方差矩阵P
	KF.statePost.at<float>(0) = bbox.x + bbox.width / 2;
	KF.statePost.at<float>(1) = bbox.y + bbox.height / 2;
	Mat measurement = Mat::zeros(measureNum, 1, CV_32F);                           //初始测量值x'(0)，因为后面要更新这个值，所以必须先定义
	float peak_value = 0.0f;
	float peak_value_thro = 0.3f;
	bool thro_change = false;
	FILE *fp = NULL;
	fp = fopen("peak_value.txt", "w+");
	int count_peak_value = 0, count_frames = 0, count_change_scale=0, change_flag = 0;
	Point predict_pt;
	Point correction_pt;
	Point bbox_pre;


	cv::Mat grayframe;
	cv::cvtColor(firstFrame, grayframe, CV_BGR2GRAY);

	//uchar* pdata = grayframe.ptr<uchar>(0);
	int w = firstFrame.cols;
	int h = firstFrame.rows;
	KCFTracker tracker;
	tracker.init(bbox, grayframe);

	while (capture.read(frame))
	{
		cv::cvtColor(frame, grayframe, CV_BGR2GRAY);
		uchar* pdata = grayframe.ptr<uchar>(0);
		
		//bbox_pre = Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);
		bbox_pre = Point((int)KF.statePost.at<float>(0), (int)KF.statePost.at<float>(1));;

		auto start = chrono::system_clock::now();
		bbox = tracker.update(grayframe, peak_value,bbox, count_change_scale, peak_value_thro);
		auto end = chrono::system_clock::now();
		auto dur = chrono::duration_cast<chrono::microseconds>(end - start);
		std::cout << "all times::" << double(dur.count())*chrono::microseconds::period::num / chrono::microseconds::period::den << "s" << std::endl;
		//printf("rect size is (%d, %d)\n", bbox.width, bbox.height);

		fprintf(fp, "%f", peak_value);
		fprintf(fp, "  %d\n", count_frames);
		if (peak_value < 0.15f) count_peak_value = count_peak_value + 1;
		//2.kalman prediction 
		//预测后statepre变成之前的statepost statepost不变
		Mat prediction = KF.predict();
		predict_pt = Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));   //预测值(x',y')
		//更新修正
		if (peak_value < peak_value_thro) {
			//cv::waitKey(0);
			bbox.x = predict_pt.x - bbox.width / 2;
			bbox.y = predict_pt.y - bbox.height / 2;
			measurement.at<float>(0) = (float)(predict_pt.x);
			measurement.at<float>(1) = (float)(predict_pt.y);
			change_flag = 0;
			count_change_scale = count_change_scale + 1;
		}
		else
		{
			measurement.at<float>(0) = (float)(bbox.x + bbox.width / 2);
			measurement.at<float>(1) = (float)(bbox.y + bbox.height / 2);
			change_flag = change_flag + 1;
			if (change_flag > 2)
			{
				count_change_scale = 0;
				peak_value_thro = 0.3f;
			}
		}
		if (count_change_scale > capture.get(CV_CAP_PROP_FPS)) peak_value_thro = 0.15f;
		//4.update  
		//修正后statepost会变
		Mat correction = KF.correct(measurement);
		correction_pt = Point((int)correction.at<float>(0), (int)correction.at<float>(1));   //预测值(x',y')
		//5.plot real and prediction
		std::printf("预测值statePre: %d %d\n预估值statePost: %d %d\n真实值measurement: %f %f\n", 
			predict_pt.x, predict_pt.y, 
			correction_pt.x, correction_pt.y, 
			measurement.at<float>(0), measurement.at<float>(1));
		//Point tmp_line = Point((correction_pt.x - bbox_pre.x) * 10, (correction_pt.y - bbox_pre.y) * 10);
		//line(frame, bbox_pre, correction_pt + tmp_line, Scalar(255,255,255),1);
		//circle(frame, Point(bbox_pre.x, bbox_pre.y), 2, Scalar(255, 255, 255),2);
		//circle(frame, correction_pt + tmp_line, 2, Scalar(0, 0, 0), 4);
		//circle(frame, Point(int(measurement.at<float>(0)), int(measurement.at<float>(1))), 2, Scalar(0, 0, 255), 4);
		rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
		imshow("output", frame);
		count_frames = count_frames + 1;
		if (cv::waitKey(10) == 27)
		{
			capture.release();
			cv::destroyWindow("output");
		}
	}
	fprintf(fp, "\ncount_peak_value: %d\n", count_peak_value);
	fprintf(fp, "\ncount_frames: %d\n", count_frames);
	fprintf(fp, "\n%f\n", float(count_frames - count_peak_value) / float(count_frames));
	fclose(fp);
	system("pause");
	return 0;
}

#endif






