#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
#include "matchtemplate.h"
#include "speeddetection.h"
//读图像img -> ptr[][] -> 图像img2, 图像->二维数组->图像
void TransImg(cv::Mat img, ImgData& p)
{
	int row = img.rows;
	int col = img.cols;
	CreateMemory(p, row, col);
	for (int i = 0; i < p.row; i++)
	{
		for (int j = 0; j < p.col; j++)
		{
			p.BufData[i* p.col + j] = img.at<uchar>(i, j);//img的矩阵数据传给二维数组ptr[][]
		}
	}
}

int main1()
{
	//opencv读取视频
	cv::VideoCapture capture;
	capture.open("G:/跟踪/视频/1.avi");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	cv::Mat firstFrame;
	////获取视频的第一帧,并框选目标
	int count = 1;
	while (count < 50) {
		capture.read(firstFrame);
		count += 1;
	}
	//提取roi
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("原图", frame);
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();
	std::cout << "roi pixel num:" << img_Template.rows << "*" << img_Template.cols << "=" << img_Template.rows*img_Template.cols << std::endl;

	//定义模板ImgData  x,y,row,col,bufdata
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate); //opencv mat数据转化为ImgData
	
	//一维数据使用方法  模板图像需要 x y坐标
	/*ImgData imgTemplate;
	imgTemplate = SetMemory(img_Template_1D, row, col, x, y);*/
	
	ImgData result;
	while (capture.read(frame))
	{
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		//视频帧图像
		ImgData imgSrc;
		//一维数据使用方法     帧图像 不需要x y坐标
		//imgSrc = SetMemory(img_Template_1D, row, col, x, y);


		TransImg(frame, imgSrc);//opencv mat数据转化为ImgData
		double t = (double)cvGetTickCount();

		//匹配  
		//result含有找到的x y坐标（找到对应区域的起始点左上角坐标） 不含宽高   
		result = ImageMatch(imgSrc, imgTemplate);

		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (result.x == 0 && result.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}

		//画出匹配位置  宽高为模板的大小
		cv::Rect box(cv::Point(result.x, result.y), cv::Point(result.x + imgTemplate.col, result.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
		cv::imshow("原图", frame);
		if (cv::waitKey(1) == 27)
		{
			capture.release();
			cv::destroyAllWindows();
			break;
		}

		//更新窗口
		updateCorr(result, imgTemplate, 20.0);
	}
	capture.release();
	cv::destroyAllWindows();
}

int main2()
{
	//opencv读取视频
	cv::VideoCapture capture;
	capture.open("G:/跟踪/视频/11111.mp4");
	//capture.open("G:/跟踪/视频/1.avi");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	cv::Mat firstFrame;
	////获取视频的第一帧,并框选目标
	int count = 1;
	while (count < 2) {
		capture.read(firstFrame);
		count += 1;
	}
	//提取roi
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("原图", frame);
	if (Template_Rect.empty())
		return -1;
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();
	std::cout << "roi pixel num:" << img_Template.rows << "*" << img_Template.cols << "=" << img_Template.rows*img_Template.cols << std::endl;
	std::cout << "Template_Rect x " << Template_Rect.x << " Template_Rect y" << Template_Rect.y << std::endl;
	cv::imwrite("template.jpg", img_Template);
	
	//测试，读取图像中固定坐标的区域，当作模板
	//cv::Mat frame;
	//capture >> frame;
	//cv::Rect Template_Rect(356, 99,16, 14);//图像搜索窗口中，某个区域跟该模板太像  问题：没有灰度化   解决
	//cv::cvtColor(frame, frame, CV_BGR2GRAY);
	//cv::Mat img_Template = frame(Template_Rect).clone();
	//rectangle(firstFrame, Template_Rect, cv::Scalar(255, 255, 255), 1);
	//cv::imshow("123", firstFrame);
	//cv::waitKey(0);
	
	
	//定义模板ImgData  x,y,row,col,bufdata
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate); //opencv mat数据转化为ImgData
	
	//一维数据使用方法  模板图像需要 x y坐标
	/*ImgData imgTemplate;
	imgTemplate = SetMemory(img_Template_1D, row, col, x, y);*/

	//跟踪物体的坐标
	ImgData result;
	
	//向量
	std::vector<ImgData> MultiFrames;  //多帧数据
	std::vector<ImgData> Martixs;	//多帧的向量
	ImgData boxWindow;	//求出的限制向量
	CreatImgData(boxWindow);
	int frame_count = 1;//帧数
	while (capture.read(frame))
	{
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		//视频帧图像
		ImgData imgSrc;
		//一维数据使用方法     帧图像 不需要x y坐标
		//imgSrc = SetMemory(img_Template_1D, row, col, x, y);


		TransImg(frame, imgSrc);//opencv mat数据转化为ImgData
		double t = (double)cvGetTickCount();

		//匹配  
		//result含有找到的x y坐标（找到对应区域的起始点左上角坐标） 不含宽高   
		result = ImageMatch(imgSrc, imgTemplate,boxWindow);
		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (result.x == 0 && result.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}
		
		//更新窗口
		updateCorr(result, imgTemplate, boxWindow);

		//计算向量
		UpdateBboxMartix(imgTemplate, MultiFrames, Martixs, boxWindow);
		//printf("boxWindow x: %d y: %d ", boxWindow.x, boxWindow.y);

		//画出仅做完匹配的坐标
		cv::Rect box(cv::Point(imgTemplate.x, imgTemplate.y), cv::Point(imgTemplate.x + imgTemplate.col, imgTemplate.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(255, 255, 255), 1);
		cv::putText(frame, std::to_string(frame_count), cv::Point(10,10), 1,1, cv::Scalar(255, 255, 0));
		cv::imshow("原图", frame);
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