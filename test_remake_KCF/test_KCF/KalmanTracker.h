#pragma once
#include "opencv2/opencv.hpp"
#include "opencv2/video.hpp"

class KalmanTracker
{
public:
	KalmanTracker(cv::Rect bbox, int stateNum, int measureNum, int controlParams = 0, int type = CV_32F);
	void init(cv::Rect bbox, int stateNum, int measureNum, int controlParams = 0, int type = CV_32F);
	~KalmanTracker();

	//Ԥ��
	virtual void Prediction();
	//����
	virtual void Update(cv::Rect& bbox);
	//����
	virtual void Correction();

	void obtainDiff(cv::Point bbox);
	bool judgeDiff();


	void SetBoxPoint(cv::Point point,int index = 0);
	cv::Point GetPoint(int index = 0);

	float peak_value;							//KCF�ķ�ֵ
	float peak_value_thro;						//KCF��ֵ����ֵ
	bool thro_change;							//�Ƿ��KCF��ֵ���и���
	int count_change_scale, change_flag;		//��ζ���ֵ���и���
	std::vector<int> count_diff;				//��ζԲ���������и���

	cv::Point bbox_pre;								//ǰһ�����ٿ����ĵ�
	cv::Point bbox_pt;								//��ǰ���ٿ����ĵ�

	cv::Point predict_pt;								//Ԥ������ĵ�
	cv::Point correction_pt;							//���������ĵ�
	cv::Point measurement_pt;							//���������ĵ�
private:
	cv::KalmanFilter KF;								//�������˲���

	cv::Mat prediction;									//Ԥ��
	cv::Mat measurement;								//����
	cv::Mat correction;									//�����Ľ��
	
	//cv::Mat transitionMatrix;							//״̬ת�ƾ���F
	//cv::Mat measurementMatrix;                          //��������H
	//cv::Mat processNoiseCov;                            //ϵͳ�����������Q
	//cv::Mat measurementNoiseCov;                        //���������������R
	//cv::Mat errorCovPost;                               //����������Э�������P
	
};

