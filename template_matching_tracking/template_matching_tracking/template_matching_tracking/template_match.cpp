#include "stdafx.h"
#include <iostream>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
//图片数据
struct ImgData
{
	uchar **BufData;
	//行高
	int row;
	//列宽
	int col;
	//位于原始图片的坐标
	int x;
	int y;
};
void CreateMemory(ImgData& Data, int row = 0, int col= 0)
{
	if (row != 0) 
	{
		Data.row = row;
		Data.col = col;
	}
	Data.BufData = (uchar **)malloc(row * sizeof(uchar *));//二维数组ptr[][]
	for (int i = 0; i < row; i++)
	{
		Data.BufData[i] = (uchar *)malloc(col * sizeof(uchar));
	}
}

//TODO：implement
void CopyDataPara(ImgData src, ImgData& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.row = src.row;
	dst.col = src.col;
}

//读图像img -> ptr[][] -> 图像img2, 图像->二维数组->图像
void TransImg(cv::Mat img,ImgData& p)
{
	int row = img.rows;
	int col = img.cols;
	CreateMemory(p, row, col);
	for (int i = 0; i < p.row; i++)
	{
		for (int j = 0; j < p.col; j++)
		{
			p.BufData[i][j] = img.at<uchar>(i, j);//img的矩阵数据传给二维数组ptr[][]
		}
	}
}

//限制滑动的窗口大小
int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow)
{
	if (pTemplate.x == 0 && pTemplate.y == 0)
		return -1;
	pSrcWindow.row = pTemplate.row * 3;
	pSrcWindow.col = pTemplate.col * 3;
	pSrcWindow.x = pTemplate.x + pTemplate.col * 0.5 - pSrcWindow.col * 0.5;
	pSrcWindow.y = pTemplate.y + pTemplate.row * 0.5 - pSrcWindow.row * 0.5;

	if (pSrcWindow.x < 0)
		pSrcWindow.x = 0;
	if (pSrcWindow.x > pSrc.col)
		pSrcWindow.x = pSrc.col;

	if (pSrcWindow.y < 0)
		pSrcWindow.y = 0;
	if (pSrcWindow.y > pSrc.row)
		pSrcWindow.y = pSrc.row;
	return 0;

}

ImgData ImageMatch(ImgData pSrc, ImgData pTemplate)
{
	ImgData dstSize;//最终求出的坐标值
	dstSize.x = 0;
	dstSize.y = 0;
	ImgData pSrcWindow;
	int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow);
	if (ret < 0)
		return dstSize;
	//计算dSigmaT
	double dSigmaT = 0;
	for (int n = 0; n < pTemplate.row; n++)
	{
		for (int m = 0; m < pTemplate.col; m++)
		{
			// 指向模板图像倒数第j行，第i个象素的指针			
			dSigmaT += (double)(pTemplate.BufData[n][m] * pTemplate.BufData[n][m]);
		}
	}

	//找到图像中最大相似性的出现位置
	double dSigmaST = 0;
	double dSigmaS = 0;
	//相似性测度
	double R = 0.0;
	//最大相似性测度
	double  dbMaxR= 0.0;
	//最大相似性出现位置
	int nMaxWidth = 0;
	int nMaxHeight = 0;
	for (int i = pSrcWindow.y; i < pSrcWindow.y + pSrcWindow.row - pTemplate.row + 1; i++)
	{
		for (int j = pSrcWindow.x; j < pSrcWindow.x + pSrcWindow.col + pTemplate.col + 1; j++)
		{
			dSigmaST = 0;
			dSigmaS = 0;

			for (int n = 0; n < pTemplate.row; n++)
			{
				for (int m = 0; m < pTemplate.col; m++)
				{
					double unchPixel = (unsigned char)pSrc.BufData[i+n][j+m];
					double unchTemplatePixel = (unsigned char)pTemplate.BufData[n][m];

					dSigmaS += (double)unchPixel*unchPixel;
					dSigmaST += (double)unchPixel*unchTemplatePixel;
				}
			}
			//计算相似性
			R = dSigmaST / (sqrt(dSigmaS)*sqrt(dSigmaT));
			//与最大相似性比较
			if (R >  dbMaxR)
			{
				dbMaxR = R;
				nMaxHeight = i;
				nMaxWidth = j;
			}
		}
	}
	dstSize.x = nMaxWidth;
	dstSize.y = nMaxHeight;
	return dstSize;
}

//单张图片测试
void imageTest()
{
	//输入图像名
	/*char InSourceImgName[10];
	std::cout << "Enter source Image name:";
	std::cin >> InSourceImgName;*/
	char InSourceImgName[10] = "1.bmp";//"pp.bmp"
	cv::Mat img_Src = cv::imread(InSourceImgName, 0);
	cv::namedWindow("原图");
	cv::imshow("原图", img_Src);
	ImgData imgSrc;
	TransImg(img_Src,imgSrc);

	//test
	//Mat img2 = Mat(imgSrc.row, imgSrc.col, CV_8UC1);
	//uchar *ptmp = NULL;
	//for (int i = 0; i < imgSrc.row; i++)
	//{
	//	ptmp = img2.ptr<uchar>(i);//指针指向img2的第i行
	//	for (int j = 0; j < imgSrc.col; j++)
	//	{
	//		ptmp[j] = imgSrc.BufData[i][j];//二维数组数据传给img2的第i行第j列
	//	}
	//}
	//namedWindow("新图");
	//imshow("新图", img2);

	//输入图像名
	//char InTemplateImgName[10];
	//std::cout << "Enter source Image name:";
	//std::cin >> InTemplateImgName;
	//char InTemplateImgName[10] = "pp11.bmp";
	//Mat img_Template = imread(InTemplateImgName, 0);
	//namedWindow("模板");
	//imshow("模板", img_Template);
	//ImgData imgTemplate;
	//TransImg(img_Template, imgTemplate);

	cv::Rect Template_Rect = selectROI("原图", img_Src);
	cv::Mat img_Template = img_Src(Template_Rect).clone();
	cv::namedWindow("模板");
	cv::imshow("模板", img_Template);
	
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate);

	//匹配
	ImgData dst;
	dst = ImageMatch(imgSrc, imgTemplate);
	if (dst.x == 0 && dst.y == 0)
	{
		std::cout << "error cor" << std::endl;
	}
	//画出匹配位置
	cv::Rect box(cv::Point(dst.x, dst.y), cv::Point(dst.x + imgTemplate.col, dst.y + imgTemplate.row));
	rectangle(img_Src, box, cv::Scalar(0, 0, 255), 3);
	imshow("result", img_Src);

	cv::waitKey(100000);
	cv::destroyAllWindows();
}

int main()
{
	//imageTest();
	
	cv::VideoCapture capture;
	capture.open("C:/Users/cjy/Desktop/LY/code/DaSiamRPN/code/跟踪/11111.mp4");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("原图", frame);
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();

	//定义模板ImgData
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate);
	ImgData dst;
	while (capture.read(frame))
	{
		cv::cvtColor(frame,frame, CV_BGR2GRAY);
		ImgData imgSrc;
		TransImg(frame, imgSrc);
		//匹配
		double t = (double)cvGetTickCount();
		dst = ImageMatch(imgSrc, imgTemplate);
		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (dst.x == 0 && dst.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}

		//画出匹配位置
		cv::Rect box(cv::Point(dst.x, dst.y), cv::Point(dst.x + imgTemplate.col, dst.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
		cv::imshow("原图", frame);
		if (cv::waitKey(1) == 27)
		{
			capture.release();
			cv::destroyAllWindows();
			break;
		}

		//更新窗口
		double Thro = 20; //防止滑动窗口跑太远
		if (((dst.x - imgTemplate.x) < Thro) || ((dst.y - imgTemplate.y) < Thro))
		{
			imgTemplate.x = dst.x;
			imgTemplate.y = dst.y;
		}
	}
	capture.release();
	cv::destroyAllWindows();
}