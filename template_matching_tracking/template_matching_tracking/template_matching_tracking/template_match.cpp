#include "stdafx.h"
#include <iostream>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
//ͼƬ����
struct ImgData
{
	uchar **BufData;
	//�и�
	int row;
	//�п�
	int col;
	//λ��ԭʼͼƬ������
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
	Data.BufData = (uchar **)malloc(row * sizeof(uchar *));//��ά����ptr[][]
	for (int i = 0; i < row; i++)
	{
		Data.BufData[i] = (uchar *)malloc(col * sizeof(uchar));
	}
}

//TODO��implement
void CopyDataPara(ImgData src, ImgData& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.row = src.row;
	dst.col = src.col;
}

//��ͼ��img -> ptr[][] -> ͼ��img2, ͼ��->��ά����->ͼ��
void TransImg(cv::Mat img,ImgData& p)
{
	int row = img.rows;
	int col = img.cols;
	CreateMemory(p, row, col);
	for (int i = 0; i < p.row; i++)
	{
		for (int j = 0; j < p.col; j++)
		{
			p.BufData[i][j] = img.at<uchar>(i, j);//img�ľ������ݴ�����ά����ptr[][]
		}
	}
}

//���ƻ����Ĵ��ڴ�С
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
	ImgData dstSize;//�������������ֵ
	dstSize.x = 0;
	dstSize.y = 0;
	ImgData pSrcWindow;
	int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow);
	if (ret < 0)
		return dstSize;
	//����dSigmaT
	double dSigmaT = 0;
	for (int n = 0; n < pTemplate.row; n++)
	{
		for (int m = 0; m < pTemplate.col; m++)
		{
			// ָ��ģ��ͼ������j�У���i�����ص�ָ��			
			dSigmaT += (double)(pTemplate.BufData[n][m] * pTemplate.BufData[n][m]);
		}
	}

	//�ҵ�ͼ������������Եĳ���λ��
	double dSigmaST = 0;
	double dSigmaS = 0;
	//�����Բ��
	double R = 0.0;
	//��������Բ��
	double  dbMaxR= 0.0;
	//��������Գ���λ��
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
			//����������
			R = dSigmaST / (sqrt(dSigmaS)*sqrt(dSigmaT));
			//����������ԱȽ�
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

//����ͼƬ����
void imageTest()
{
	//����ͼ����
	/*char InSourceImgName[10];
	std::cout << "Enter source Image name:";
	std::cin >> InSourceImgName;*/
	char InSourceImgName[10] = "1.bmp";//"pp.bmp"
	cv::Mat img_Src = cv::imread(InSourceImgName, 0);
	cv::namedWindow("ԭͼ");
	cv::imshow("ԭͼ", img_Src);
	ImgData imgSrc;
	TransImg(img_Src,imgSrc);

	//test
	//Mat img2 = Mat(imgSrc.row, imgSrc.col, CV_8UC1);
	//uchar *ptmp = NULL;
	//for (int i = 0; i < imgSrc.row; i++)
	//{
	//	ptmp = img2.ptr<uchar>(i);//ָ��ָ��img2�ĵ�i��
	//	for (int j = 0; j < imgSrc.col; j++)
	//	{
	//		ptmp[j] = imgSrc.BufData[i][j];//��ά�������ݴ���img2�ĵ�i�е�j��
	//	}
	//}
	//namedWindow("��ͼ");
	//imshow("��ͼ", img2);

	//����ͼ����
	//char InTemplateImgName[10];
	//std::cout << "Enter source Image name:";
	//std::cin >> InTemplateImgName;
	//char InTemplateImgName[10] = "pp11.bmp";
	//Mat img_Template = imread(InTemplateImgName, 0);
	//namedWindow("ģ��");
	//imshow("ģ��", img_Template);
	//ImgData imgTemplate;
	//TransImg(img_Template, imgTemplate);

	cv::Rect Template_Rect = selectROI("ԭͼ", img_Src);
	cv::Mat img_Template = img_Src(Template_Rect).clone();
	cv::namedWindow("ģ��");
	cv::imshow("ģ��", img_Template);
	
	ImgData imgTemplate;
	imgTemplate.x = Template_Rect.x;
	imgTemplate.y = Template_Rect.y;
	TransImg(img_Template, imgTemplate);

	//ƥ��
	ImgData dst;
	dst = ImageMatch(imgSrc, imgTemplate);
	if (dst.x == 0 && dst.y == 0)
	{
		std::cout << "error cor" << std::endl;
	}
	//����ƥ��λ��
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
	capture.open("C:/Users/cjy/Desktop/LY/code/DaSiamRPN/code/����/11111.mp4");
	bool fromfile = true;
	//Init camera
	if (!capture.isOpened())
	{
		std::cout << "capture device failed to open!" << std::endl;
		return -1;
	}
	
	cv::Mat frame;
	capture >> frame;
	cv::Rect Template_Rect = selectROI("ԭͼ", frame);
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::Mat img_Template = frame(Template_Rect).clone();

	//����ģ��ImgData
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
		//ƥ��
		double t = (double)cvGetTickCount();
		dst = ImageMatch(imgSrc, imgTemplate);
		t = (double)cvGetTickCount() - t;
		std::cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << std::endl;
		if (dst.x == 0 && dst.y == 0)
		{
			std::cout << "error cor" << std::endl;
		}

		//����ƥ��λ��
		cv::Rect box(cv::Point(dst.x, dst.y), cv::Point(dst.x + imgTemplate.col, dst.y + imgTemplate.row));
		rectangle(frame, box, cv::Scalar(0, 0, 255), 3);
		cv::imshow("ԭͼ", frame);
		if (cv::waitKey(1) == 27)
		{
			capture.release();
			cv::destroyAllWindows();
			break;
		}

		//���´���
		double Thro = 20; //��ֹ����������̫Զ
		if (((dst.x - imgTemplate.x) < Thro) || ((dst.y - imgTemplate.y) < Thro))
		{
			imgTemplate.x = dst.x;
			imgTemplate.y = dst.y;
		}
	}
	capture.release();
	cv::destroyAllWindows();
}