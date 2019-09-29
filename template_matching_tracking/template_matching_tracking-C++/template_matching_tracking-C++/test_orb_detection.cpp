#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc.hpp>  
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include<opencv2/opencv.hpp>
using namespace cv;

int orb_extra(Mat& src, Mat& dst) {
	// -- Step 1: Detect the keypoints using STAR Detector 
	std::vector<KeyPoint> keypoints;
	int nkeypoint = 50;//特征点个数
	Ptr<ORB> orb = ORB::create(nkeypoint);
	orb->detect(src, keypoints);
	printf("orb keypoints: %d \n", keypoints.size());
	Mat res1;
	drawKeypoints(src, keypoints, res1, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	imshow("KeyPoints of orb", res1);
	// -- Stpe 2: Calculate descriptors (feature vectors) 
	Mat descriptors;
	orb->compute(src, keypoints, descriptors);
	return 0;
}

int hog_extra(Mat& src, Mat& dst) {
	// -- Step 1: Detect the keypoints using STAR Detector 
	Mat tmp, tmp_gray;
	resize(src, tmp, Size(64, 128));
	cvtColor(tmp, tmp_gray, CV_BGR2GRAY);
	HOGDescriptor detector(Size(64, 128), Size(16, 16), Size(8, 8), Size(8, 8), 9);
	std::vector<float> descriptors;
	//std::vector<Point> locations;
	detector.compute(tmp_gray, descriptors, Size(0, 0), Size(0, 0)/*, locations*/);
	//printf("hog descriptors: %d \n hog keypoints: %d \n", descriptors.size(),locations.size());  //3780D
	printf("hog descriptors: %d \n", descriptors.size());  //3780D
	Mat res1;
	src.copyTo(res1);
	/*for (int i = 0; i < locations.size(); i++)
	{
		circle(res1, locations[i], 1, Scalar(255, 255, 0));
	}*/
	imshow("hog detection", res1);
	return 0;
}

int orb_tst()
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
	orb_extra(image, dst);
	int end_orb = getTickCount();
	printf("orb times: %f ms\n", (end_orb - start) / getTickFrequency() * 1000);

	hog_extra(image, dst);
	int end_hog = getTickCount();
	printf("orb times: %f ms\n", (end_hog - end_orb) / getTickFrequency() * 1000);


	waitKey(0);
	destroyAllWindows();
}