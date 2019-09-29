#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
#include "matchtemplate.h"
#include "speeddetection.h"
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

int main1()
{
	//opencv��ȡ��Ƶ
	cv::VideoCapture capture;
	capture.open("G:/����/��Ƶ/1.avi");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	cv::Mat firstFrame;
	////��ȡ��Ƶ�ĵ�һ֡,����ѡĿ��
	int count = 1;
	while (count < 50) {
		capture.read(firstFrame);
		count += 1;
	}
	//��ȡroi
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("ԭͼ", frame);
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();
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

int main2()
{
	//opencv��ȡ��Ƶ
	cv::VideoCapture capture;
	capture.open("G:/����/��Ƶ/11111.mp4");
	//capture.open("G:/����/��Ƶ/1.avi");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	cv::Mat firstFrame;
	////��ȡ��Ƶ�ĵ�һ֡,����ѡĿ��
	int count = 1;
	while (count < 2) {
		capture.read(firstFrame);
		count += 1;
	}
	//��ȡroi
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("ԭͼ", frame);
	if (Template_Rect.empty())
		return -1;
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();
	std::cout << "roi pixel num:" << img_Template.rows << "*" << img_Template.cols << "=" << img_Template.rows*img_Template.cols << std::endl;
	std::cout << "Template_Rect x " << Template_Rect.x << " Template_Rect y" << Template_Rect.y << std::endl;
	cv::imwrite("template.jpg", img_Template);
	
	//���ԣ���ȡͼ���й̶���������򣬵���ģ��
	//cv::Mat frame;
	//capture >> frame;
	//cv::Rect Template_Rect(356, 99,16, 14);//ͼ�����������У�ĳ���������ģ��̫��  ���⣺û�лҶȻ�   ���
	//cv::cvtColor(frame, frame, CV_BGR2GRAY);
	//cv::Mat img_Template = frame(Template_Rect).clone();
	//rectangle(firstFrame, Template_Rect, cv::Scalar(255, 255, 255), 1);
	//cv::imshow("123", firstFrame);
	//cv::waitKey(0);
	
	
	//����ģ��ImgData  x,y,row,col,bufdata
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate); //opencv mat����ת��ΪImgData
	
	//һά����ʹ�÷���  ģ��ͼ����Ҫ x y����
	/*ImgData imgTemplate;
	imgTemplate = SetMemory(img_Template_1D, row, col, x, y);*/

	//�������������
	ImgData result;
	
	//����
	std::vector<ImgData> MultiFrames;  //��֡����
	std::vector<ImgData> Martixs;	//��֡������
	ImgData boxWindow;	//�������������
	CreatImgData(boxWindow);
	int frame_count = 1;//֡��
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
		result = ImageMatch(imgSrc, imgTemplate,boxWindow);
		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (result.x == 0 && result.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}
		
		//���´���
		updateCorr(result, imgTemplate, boxWindow);

		//��������
		UpdateBboxMartix(imgTemplate, MultiFrames, Martixs, boxWindow);
		//printf("boxWindow x: %d y: %d ", boxWindow.x, boxWindow.y);

		//����������ƥ�������
		cv::Rect box(cv::Point(imgTemplate.x, imgTemplate.y), cv::Point(imgTemplate.x + imgTemplate.col, imgTemplate.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(255, 255, 255), 1);
		cv::putText(frame, std::to_string(frame_count), cv::Point(10,10), 1,1, cv::Scalar(255, 255, 0));
		cv::imshow("ԭͼ", frame);
		if (cv::waitKey(1) == 'c')
		{
			cv::waitKey(0); 
		}
		if (cv::waitKey(1) == 27)
		{
			capture.release();
			cv::destroyAllWindows();
			break;
		}
		frame_count++;
	}
	capture.release();
	cv::destroyAllWindows();
	system("pause");
}