#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  

using namespace cv;
using namespace std;

void main()
{
	Mat img1 = imread("D:/LY/code/erasePrint/data/tmp/1_1.jpg");
	Mat img2 = imread("D:/LY/code/erasePrint/data/tmp/1_2.jpg");
	Mat img3 = imread("D:/LY/code/erasePrint/data/tmp/2_1.jpg");
	Mat img4 = imread("D:/LY/code/erasePrint/data/tmp/3_1.jpg");
	Mat img5 = imread("D:/LY/code/erasePrint/data/tmp/3_2.jpg");
	Mat img6 = imread("D:/LY/code/erasePrint/data/tmp/3_3.jpg");
	Mat img7 = imread("D:/LY/code/erasePrint/data/tmp/4_1.jpg");
	//Mat dst;
	//resize(img, dst, Size(608,608), 0, 0, INTER_LINEAR);
	Mat src_gray,gaussian_gray;
	cvtColor(img2, src_gray, COLOR_BGR2GRAY);
	medianBlur(src_gray, gaussian_gray,5);
	/*
	CV_EXPORTS_W void HoughCircles( InputArray image, OutputArray circles,
									int method, double dp, double minDist,
										double param1 = 100, double param2 = 100,
										int minRadius = 0, int maxRadius = 0 );
	dp 越小越好 找到的圆月圆 1.2以下
	minDist 两个圆之间的距离  30以下
	canny 阈值 param1 param2	param2 100	param2 30
	minRadius 圆最小的半径
	maxRadius 圆最大的半径
	*/
	vector<Vec3f> circles;
	vector<Vec3f> circles2;
	vector<Vec3f> circles3;
	HoughCircles(gaussian_gray, circles, HOUGH_GRADIENT, 1.1, 20, 100, 30, 10, 20);
	HoughCircles(gaussian_gray, circles2, HOUGH_GRADIENT,1.1, 20, 100, 30, 10, 20);
	HoughCircles(gaussian_gray, circles3, HOUGH_GRADIENT,1.1, 20, 100, 30, 10, 20);


	Mat tmp1;
	src_gray.copyTo(tmp1);
	for (size_t i = 0; i < circles.size(); i++)
	{
	    Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	    int radius = cvRound(circles[i][2]);
		//绘制圆心
		circle(tmp1, center, 3, Scalar(255, 255, 255), -1, 8, 0);
		//绘制圆的轮廓
		circle(tmp1, center, radius, Scalar(0, 0, 0),1, 8, 0);
	}
	 imshow("效果图", tmp1);

	 Mat tmp2;
	 src_gray.copyTo(tmp2);
	 for (size_t i = 0; i < circles2.size(); i++)
	 {
		 Point center(cvRound(circles2[i][0]), cvRound(circles2[i][1]));
		 int radius = cvRound(circles2[i][2]);
		 //绘制圆心
		 circle(tmp2, center, 3, Scalar(255, 255, 255), -1, 8, 0);
		 //绘制圆的轮廓
		 circle(tmp2, center, radius, Scalar(0, 0, 0),1, 8, 0);
	 }
	 imshow("效果图", tmp2);

	 Mat tmp3;
	 src_gray.copyTo(tmp3);
	 for (size_t i = 0; i < circles3.size(); i++)
	 {
		 Point center(cvRound(circles3[i][0]), cvRound(circles3[i][1]));
		 int radius = cvRound(circles3[i][2]);
		 //绘制圆心
		 circle(tmp3, center, 3, Scalar(255, 255, 255), -1, 8, 0);
		 //绘制圆的轮廓
		 circle(tmp3, center, radius, Scalar(0, 0, 0), 1, 8, 0);
	 }
	 imshow("效果图", tmp3);

	waitKey(0);

	return;
}