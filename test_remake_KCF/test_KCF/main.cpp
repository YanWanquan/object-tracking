#if  0

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include "kcftracker.hpp"
using namespace cv;
using namespace std;






int main(int argc, char *argv[])
{

	//createHanningMats();





#if 1
	VideoCapture capture;
	Mat frame;
#if _DEBUG
	frame = capture.open("G:/����/baseline/�ڵ�/�����о�������˶�_����ͷƽ��.avi");
	//frame = capture.open(0);
#else
	cout << "video name" << endl;
	string tmp_name;
	getline(cin, tmp_name);
	if (tmp_name.empty())
	{
		cout << "error name" << endl;
		return -1;
	}
	else
	{
		tmp_name = tmp_name + ".mp4";
	}
	frame = capture.open(tmp_name);
#endif

	if (!capture.isOpened())
	{
		printf("can not open ...\n");
		return -1;
	}
	Mat firstFrame;
	//��ȡ��Ƶ�ĵ�һ֡,����ѡĿ��
	capture.read(firstFrame);
	int count = 1;
	//while (count < 50) {
	//	capture.read(firstFrame);
	//	count += 1;
	//}

	Rect bbox;
	//bbox.x = 820;
	//bbox.y = 242;
	//bbox.width = 69;
	//bbox.height = 61;
	if (!firstFrame.empty())
	{
		namedWindow("output", WINDOW_AUTOSIZE);
		imshow("output", firstFrame);
		if(bbox.empty())
		bbox = selectROI("output",firstFrame, true, false);
		waitKey();
	}
	
	capture.read(frame);
	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = false;
	bool LAB = false;
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

	cv::Mat grayframe;
	cv::cvtColor(firstFrame, grayframe, CV_RGB2GRAY);
	tracker.init(bbox, grayframe);
	//namedWindow("output", WINDOW_AUTOSIZE);
	destroyAllWindows();
	while (capture.read(frame))
	{
		
		cv::Mat igrayframe;
		cv::cvtColor(frame, igrayframe, CV_RGB2GRAY);

		float start = getTickCount();
		bbox = tracker.update(igrayframe);
		float end = getTickCount();
		cout << "times::" << ((end - start) * 1000) / getTickFrequency() << "ms" << endl;

		rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
		imshow("output", frame);
		if (waitKey(1) == 27)
		{
			capture.release();
			destroyWindow("output");
		}
	}
	if (waitKey(1) == 27)
	{
		capture.release();
		destroyWindow("output");
	}
	capture.release();
	destroyWindow("output");
	system("pause");
#endif
	return 0;
}
#endif


#if 0


#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  


//#include "template_tracker.h"
#include "matchtemplate.h"
//��ͼ��img -> ptr[][] -> ͼ��img2, ͼ��->��ά����->ͼ��
void TransImg(cv::Mat img, ImgData& p)
{
	int row = img.rows;
	int col = img.cols;
	CreateMemory(p, row, col);
	for (int i = 0; i < p.row; i++)
	{
		for (int j = 0; j < p.col; j++)
		{
			p.BufData[i* p.col + j] = img.at<uchar>(i, j);//img�ľ������ݴ�����ά����ptr[][]
		}
	}
}

int main()
{
	//opencv��ȡ��Ƶ
	cv::VideoCapture capture;
	capture.open("D:\\workspace\\vs2015_workspace\\test_KCF\\test_KCF\\1111.mp4");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	//��ȡroi
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect;// = selectROI("ԭͼ", frame);
	Template_Rect.x = 267;
	Template_Rect.y = 287;
	Template_Rect.width = 52;
	Template_Rect.height = 50;
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();

	//cv::imshow("CC", img_Template);
	//cv::waitKey(0);

	std::cout << "roi pixel num:" << img_Template.rows << "*" << img_Template.cols << "=" << img_Template.rows*img_Template.cols << std::endl;

	//����ģ��ImgData  x,y,row,col,bufdata
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate); //opencv mat����ת��ΪImgData

										 //һά����ʹ�÷���  ģ��ͼ����Ҫ x y����
										 /*ImgData imgTemplate;
										 imgTemplate = SetMemory(img_Template_1D, row, col, x, y);*/

	ImgData result;
	while (capture.read(frame))
	{
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		//��Ƶ֡ͼ��
		ImgData imgSrc;
		//һά����ʹ�÷���     ֡ͼ�� ����Ҫx y����
		//imgSrc = SetMemory(img_Template_1D, row, col, x, y);


		TransImg(frame, imgSrc);//opencv mat����ת��ΪImgData
		double t = (double)cvGetTickCount();

		//ƥ��  
		//result�����ҵ���x y���꣨�ҵ���Ӧ�������ʼ�����Ͻ����꣩ �������   
		result = ImageMatch(imgSrc, imgTemplate);

		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (result.x == 0 && result.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}

		//����ƥ��λ��  ���Ϊģ��Ĵ�С
		cv::Rect box(cv::Point(result.x, result.y), cv::Point(result.x + imgTemplate.col, result.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
		cv::imshow("ԭͼ", frame);
		if (cv::waitKey(1) == 27)
		{
			capture.release();
			cv::destroyAllWindows();
			break;
		}

		//���´���
		updateCorr(result, imgTemplate, 20.0);
	}
	capture.release();
	cv::destroyAllWindows();
}
#endif