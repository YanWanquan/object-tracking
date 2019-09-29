#pragma once
#include "opencv2/opencv.hpp"
#include "opencv2/video.hpp"

class KalmanTracker
{
public:
	KalmanTracker(cv::Rect bbox, int stateNum, int measureNum, int controlParams = 0, int type = CV_32F);
	void init(cv::Rect bbox, int stateNum, int measureNum, int controlParams = 0, int type = CV_32F);
	~KalmanTracker();

	//预测
	virtual void Prediction();
	//更新
	virtual void Update(cv::Rect& bbox);
	//修正
	virtual void Correction();

	void obtainDiff(cv::Point bbox);
	bool judgeDiff();


	void SetBoxPoint(cv::Point point,int index = 0);
	cv::Point GetPoint(int index = 0);

	float peak_value;							//KCF的峰值
	float peak_value_thro;						//KCF峰值的阈值
	bool thro_change;							//是否对KCF阈值进行更改
	int count_change_scale, change_flag;		//如何对阈值进行更新
	std::vector<int> count_diff;				//如何对测量矩阵进行更改

	cv::Point bbox_pre;								//前一个跟踪框中心点
	cv::Point bbox_pt;								//当前跟踪框中心点

	cv::Point predict_pt;								//预测的中心点
	cv::Point correction_pt;							//修正的中心点
	cv::Point measurement_pt;							//测量的中心点
private:
	cv::KalmanFilter KF;								//卡尔曼滤波器

	cv::Mat prediction;									//预测
	cv::Mat measurement;								//修正
	cv::Mat correction;									//修正的结果
	
	//cv::Mat transitionMatrix;							//状态转移矩阵F
	//cv::Mat measurementMatrix;                          //测量矩阵H
	//cv::Mat processNoiseCov;                            //系统噪声方差矩阵Q
	//cv::Mat measurementNoiseCov;                        //测量噪声方差矩阵R
	//cv::Mat errorCovPost;                               //后验错误估计协方差矩阵P
	
};

