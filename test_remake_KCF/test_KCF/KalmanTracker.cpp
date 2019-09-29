#include "KalmanTracker.h"

//1.kalman init
KalmanTracker::KalmanTracker(cv::Rect bbox,int stateNum, int measureNum, int controlParams, int type)
{
	//4��1����(x, y, ��x, ��y)   2��1����(x, y)
	KF.init(stateNum, measureNum, controlParams);
	KF.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);	//ת�ƾ���A
	setIdentity(KF.measurementMatrix);								//��������H
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));			//ϵͳ�����������Q
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));		//���������������R
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));				//����������Э�������P
	
	this->measurement = cv::Mat::zeros(measureNum, 1, CV_32F);		//��ʼ����ֵx'(0)����Ϊ����Ҫ�������ֵ�����Ա����ȶ���
	KF.statePost.at<float>(0) = (float)(bbox.x + bbox.width / 2);			
	KF.statePost.at<float>(1) = (float)(bbox.y + bbox.height / 2);

	peak_value = 0.0f;							//KCF�ķ�ֵ
	peak_value_thro = 0.3f;						//KCF��ֵ����ֵ
	thro_change = false;						//�Ƿ��KCF��ֵ���и���
	count_change_scale = 0;						//��ζ���ֵ���и���
	change_flag = 0;
}

//1.kalman init
void KalmanTracker::init(cv::Rect bbox, int stateNum, int measureNum, int controlParams, int type)
{
	KF.init(stateNum, measureNum, controlParams, type);
	KF.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);  
	setIdentity(KF.measurementMatrix);                                        
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));                   
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));               
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));                         
	
	this->measurement = cv::Mat::zeros(measureNum, 1, CV_32F);                         
	KF.statePost.at<float>(0) = (float)(bbox.x + bbox.width / 2);
	KF.statePost.at<float>(1) = (float)(bbox.y + bbox.height / 2);

	peak_value = 0.0f;						
	peak_value_thro = 0.3f;					
	thro_change = false;					
	count_change_scale = 0;					
	change_flag = 0;
}

//2.kalman prediction 
void KalmanTracker::Prediction()
{
	//Ԥ���statepre���֮ǰ��statepost statepost����
	this->prediction = KF.predict();
	this->predict_pt = cv::Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));   //Ԥ��ֵ(x',y')
}

//3.kalman Update
void KalmanTracker::Update(cv::Rect& bbox)
{
	bbox_pre = cv::Point((int)KF.statePost.at<float>(0), (int)KF.statePost.at<float>(1));
	this->Prediction();
	//this->obtainDiff(cv::Point((bbox.x + bbox.width / 2), (bbox.y + bbox.height / 2)));
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
		bool bbox_change = false;
		//bbox_change = judgeDiff();
		if (!bbox_change)
		{
			measurement.at<float>(0) = (float)(bbox.x + bbox.width / 2);
			measurement.at<float>(1) = (float)(bbox.y + bbox.height / 2);
		}
		else
		{
			bbox.x = predict_pt.x - bbox.width / 2;
			bbox.y = predict_pt.y - bbox.height / 2;
			measurement.at<float>(0) = (float)(predict_pt.x);
			measurement.at<float>(1) = (float)(predict_pt.y);
		}
		change_flag = change_flag + 1;
		if (change_flag > 2)
		{
			count_change_scale = 0;
			peak_value_thro = 0.3f;
		}
	}
	if (count_change_scale > 30) peak_value_thro = 0.15f;
	measurement_pt = cv::Point((int)measurement.at<float>(0), (int)measurement.at<float>(1));
	this->Correction();
}


void KalmanTracker::Correction()
{
	correction = KF.correct(measurement);
	correction_pt = cv::Point(correction.at<float>(0), correction.at<float>(1));   //����ֵ(x',y')
}

void KalmanTracker::obtainDiff(cv::Point bbox)
{
	if (count_diff.size() > 50)
	{
		count_diff.erase(count_diff.begin());
	}
	if ((bbox.x - bbox_pre.x) > 0)
		count_diff.push_back(1);
	else if ((bbox.x - bbox_pre.x) == 0)
		count_diff.push_back(0);
	else
		count_diff.push_back(-1);
}

bool KalmanTracker::judgeDiff()
{
	bool bbox_change = true;							//�Ƿ����bbox�Բ���������и���
	if (count_diff.size() > 10)
	{
		int positive_cout = 0;
		int nagative_cout = 0;
		int zero_cout = 0;
		for (int i = count_diff.size() - 10; i < count_diff.size(); i++)
		{
			printf("%d ", count_diff[i]);
			if (count_diff[i] > 0) positive_cout++;
			else if (count_diff[i] < 0) nagative_cout++;
			else zero_cout++;
		}
		printf("\n");
		if ( ((positive_cout >= nagative_cout) && ((count_diff[count_diff.size() - 1]) >= 0)) ||
			 ((nagative_cout > positive_cout) && ((count_diff[count_diff.size() - 1]) <= 0)) )
			bbox_change = false;
	}
	return bbox_change;
}

/*
	<=0 ǰһ�����ٿ����ĵ�
	>0 ��ǰ���ٿ����ĵ�
*/
void KalmanTracker::SetBoxPoint(cv::Point point, int index)
{
	if (index)
		this->bbox_pt = point;
	else
		this->bbox_pre = point;
}

/*
	<=0 Ԥ��ĵ�
	>0 ������ĵ�
*/
cv::Point KalmanTracker::GetPoint(int index)
{
	cv::Point point;
	if (index)
		point = this->correction_pt;
	else
		point = this->predict_pt;
	return point;
}


KalmanTracker::~KalmanTracker()
{
}
