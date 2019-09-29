#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include<opencv2/opencv.hpp>
using namespace cv;

int fast_detection(Mat& src, Mat& dst) {
	// Detect FAST features, 20 is a good threshold
	int fastThreshold = 20;
	Ptr<FastFeatureDetector> fd = FastFeatureDetector::create(fastThreshold, true);
	std::vector<KeyPoint> keypoints;
	fd->detect(src, keypoints);
	keypoints.clear();
	return 0;
}

int sift_detection(Mat& src, Mat& dst) {
	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(50);
	std::vector<KeyPoint> keypoints;
	sift->detect(src, keypoints);
	keypoints.clear();
	return 0;
}

int surf_detection(Mat& src, Mat& dst) {
	// Detect FAST features, 20 is a good threshold
	Ptr<Feature2D> surf = xfeatures2d::SURF::create(50);
	std::vector<KeyPoint> keypoints;
	surf->detect(src, keypoints);
	keypoints.clear();
	return 0;
}

int orb_detection(Mat& src, Mat& dst) {
	// -- Step 1: Detect the keypoints using STAR Detector 
	std::vector<KeyPoint> keypoints;
	int nkeypoint = 50;//���������
	Ptr<ORB> orb = ORB::create(nkeypoint);
	orb->detect(src, keypoints);
	/*Mat res1;
	drawKeypoints(src, keypoints, res1, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	imshow("KeyPoints of orb", res1);*/
	// -- Stpe 2: Calculate descriptors (feature vectors) 
	Mat descriptors;
	orb->compute(src, keypoints, descriptors);
	return 0;
}

int hog_detection(Mat& src, Mat& dst) {
	// -- Step 1: Detect the keypoints using STAR Detector 
	Mat tmp,tmp_gray;
	resize(src, tmp, Size(64, 128));
	cvtColor(tmp, tmp_gray,CV_BGR2GRAY);
	HOGDescriptor detector(Size(64, 128), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	std::vector<float> descriptors;
	std::vector<Point> locations;
	detector.compute(tmp_gray, descriptors, Size(0, 0), Size(0, 0), locations);
	//printf("hog descriptors: %d \n", descriptors_1.size());  //3780D
	return 0;
}


cv::Mat lbp_detection(cv::Mat& srcImage)
{
	const int nRows = srcImage.rows;
	const int nCols = srcImage.cols;
	cv::Mat resultMat(srcImage.size(), srcImage.type());
	// ����ͼ������LBP����
	for (int y = 1; y < nRows - 1; y++)
	{
		for (int x = 1; x < nCols - 1; x++)
		{
			// ��������
			uchar neighbor[8] = { 0 };
			neighbor[0] = srcImage.at<uchar>(y - 1, x - 1);
			neighbor[1] = srcImage.at<uchar>(y - 1, x);
			neighbor[2] = srcImage.at<uchar>(y - 1, x + 1);
			neighbor[3] = srcImage.at<uchar>(y, x + 1);
			neighbor[4] = srcImage.at<uchar>(y + 1, x + 1);
			neighbor[5] = srcImage.at<uchar>(y + 1, x);
			neighbor[6] = srcImage.at<uchar>(y + 1, x - 1);
			neighbor[7] = srcImage.at<uchar>(y, x - 1);
			// ��ǰͼ��Ĵ������� 
			uchar center = srcImage.at<uchar>(y, x);
			uchar temp = 0;
			// ����LBP��ֵ 
			for (int k = 0; k < 8; k++)
			{
				// �������ĵ�����
				temp += (neighbor[k] >= center)* (1 << k);
			}
			resultMat.at<uchar>(y, x) = temp;
		}
	}
	return resultMat;
}

int harr_detection(Mat& src, Mat& dst) {
	Ptr<Feature2D> harr = xfeatures2d::HarrisLaplaceFeatureDetector::create(50);
	std::vector<KeyPoint> keypoints;
	harr->detect(src, keypoints);
	keypoints.clear();

	return 0;
}

int harr_detection(Mat& src) {
	Mat imageGrey;
	cvtColor(src, imageGrey, CV_RGB2GRAY);

	Mat imageHarris(src.rows, src.cols, CV_8UC1);
	int sobelApertureSize = 3 ;
	int harrisApertureSize = 2;
	double kValue = 0.01;
	int threshold =64 ;
	bool showProcessed;
	cornerHarris(imageGrey, imageHarris, harrisApertureSize, sobelApertureSize, kValue);

	return 0;
}


//ֱ��ͼ
int RGB_hist_detection(Mat& srcimage)
{
	int channels = 0;
	int histsize[] = { 256 };
	float midranges[] = { 0,255 };
	const float *ranges[] = { midranges };
	MatND  dsthist;    //Ҫ�����ֱ��ͼ
					   //�ص��עcalcHist��������Ϊ����ֱ��ͼ�ĺ���
	calcHist(&srcimage, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);

	Mat b_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	double g_dhistmaxvalue;
	minMaxLoc(dsthist, 0, &g_dhistmaxvalue, 0, 0);
	for (int i = 0; i < 256; i++) {
		//�����dsthist.at<float>(i)����ÿ��bins��Ӧ������ĸ߶�
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(b_drawImage, Point(i, b_drawImage.rows - 1), Point(i, b_drawImage.rows - 1 - value), Scalar(255, 0, 0));
	}
	//imshow("Bͨ��ֱ��ͼ", b_drawImage);

	channels = 1;
	calcHist(&srcimage, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
	Mat g_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	for (int i = 0; i < 256; i++) {
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(g_drawImage, Point(i, g_drawImage.rows - 1), Point(i, g_drawImage.rows - 1 - value), Scalar(0, 255, 0));
	}
	//imshow("Gͨ��ֱ��ͼ", g_drawImage);

	channels = 2;
	calcHist(&srcimage, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
	Mat r_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	for (int i = 0; i < 256; i++) {
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(r_drawImage, Point(i, r_drawImage.rows - 1), Point(i, r_drawImage.rows - 1 - value), Scalar(0, 0, 255));
	}
	//imshow("Rͨ��ֱ��ͼ", r_drawImage);

	//add(b_drawImage, g_drawImage, r_drawImage);   //������ֱ��ͼ����һ��
	r_drawImage = b_drawImage + g_drawImage + r_drawImage;
	putText(r_drawImage, "red is R, green is G, bule is B", Point(0,20), 1, 1, Scalar(255, 255, 0));
	imshow("RGBֱ��ͼ", r_drawImage);
	//waitKey(0);
	return 0;
}


int HSV_hist_detection(Mat& srcimage)
{
	Mat srcimageHSV;
	//ͼ��ת��HSV��ɫ�ռ�ͼ��
	cvtColor(srcimage, srcimageHSV, COLOR_BGR2HSV);
	//imshow("HSV�ռ�ͼ��", srcimageHSV);
	int channels = 0;
	int histsize[] = { 256 };
	float midranges[] = { 0,255 };
	const float *ranges[] = { midranges };
	MatND  dsthist;
	calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
	Mat b_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);

	double g_dhistmaxvalue;
	minMaxLoc(dsthist, 0, &g_dhistmaxvalue, 0, 0);
	for (int i = 0; i < 256; i++) {
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(b_drawImage, Point(i, b_drawImage.rows - 1), Point(i, b_drawImage.rows - 1 - value), Scalar(255, 0, 0));
	}
	//imshow("Hͨ��ֱ��ͼ", b_drawImage);

	channels = 1;
	calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
	Mat g_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	for (int i = 0; i < 256; i++) {
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(g_drawImage, Point(i, g_drawImage.rows - 1), Point(i, g_drawImage.rows - 1 - value), Scalar(0, 255, 0));
	}
	//imshow("Sͨ��ֱ��ͼ", g_drawImage);

	channels = 2;
	calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
	Mat r_drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	for (int i = 0; i < 256; i++) {
		int value = cvRound(256 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
		line(r_drawImage, Point(i, r_drawImage.rows - 1), Point(i, r_drawImage.rows - 1 - value), Scalar(0, 0, 255));
	}
	//imshow("Vͨ��ֱ��ͼ", r_drawImage);
	//add(b_drawImage, g_drawImage, r_drawImage);
	r_drawImage = b_drawImage + g_drawImage + r_drawImage;
	putText(r_drawImage, "bule is H, green is S, red is V", Point(0, 20), 1, 1, Scalar(255, 255, 0));
	imshow("HSVֱ��ͼ", r_drawImage);
	//waitKey(0);
	return 0;
}


int test_detection()
{
	Mat img = imread("D:/360MoveData/Users/cjy/Desktop/1.jpg");
	if (img.empty())
	{
		std::cout << "failed to open!" << std::endl;
		return -1;
	}
	std::cout << "pixel w" << img.cols << "   h" << img.rows << std::endl;
	Rect bbox = selectROI(img, true, false);
	std::cout << "pixel w" << bbox.width << "   h" << bbox.height << std::endl;
	Mat image = img(bbox);
	//imshow("roi", image);

	Mat dst;
	int start = getTickCount();

	fast_detection(image, dst);
	int end_fast = getTickCount();
	printf("fast times: %f ms\n", (end_fast - start) / getTickFrequency() * 1000);

	sift_detection(image, dst);
	int end_sift = getTickCount();
	printf("sift times: %f ms\n", (end_sift - end_fast) / getTickFrequency() * 1000);

	surf_detection(image, dst);
	int end_surf = getTickCount();
	printf("surf times: %f ms\n", (end_surf - end_sift) / getTickFrequency() * 1000);

	orb_detection(image, dst);
	int end_orb = getTickCount();
	printf("orb times: %f ms\n", (end_orb - end_surf) / getTickFrequency()*1000);

	hog_detection(image, dst);
	int end_hog = getTickCount();
	printf("hog times: %f ms\n", (end_hog - end_orb) / getTickFrequency()*1000);

	lbp_detection(image);
	int end_lbp = getTickCount();
	printf("lbp times: %f ms\n", (end_lbp - end_hog) / getTickFrequency() * 1000);

	harr_detection(image,dst);
	int end_harr = getTickCount();
	printf("harr times: %f ms\n", (end_harr - end_lbp) / getTickFrequency() * 1000);

	harr_detection(image);
	int end_harr2 = getTickCount();
	printf("harr times: %f ms\n", (end_harr2 - end_harr) / getTickFrequency() * 1000);


	RGB_hist_detection(image);
	HSV_hist_detection(image);


	waitKey(0);
	destroyAllWindows();
}