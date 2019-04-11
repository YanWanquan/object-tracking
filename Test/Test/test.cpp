#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracker.hpp>
using namespace cv;
using namespace std;

void draw_rectangle(int event, int x, int y, int flags, void*);
Mat firstFrame;
Point previousPoint, currentPoint;
Rect2d bbox;
int main(int argc, char *argv[])
{
	VideoCapture capture;
	Mat frame;
#if _DEBUG
	frame = capture.open("testHuman9.mp4");
#else
	cout << "video name" <<endl;
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
	//获取视频的第一帧,并框选目标
	capture.read(firstFrame);
	if (!firstFrame.empty())
	{
		namedWindow("output", WINDOW_AUTOSIZE);
		imshow("output", firstFrame);
		setMouseCallback("output", draw_rectangle, 0);
		waitKey();
	}
	//使用跟踪
	cout << "tracker type:" << endl;
	cout << "     TrackerMIL == 1" << endl;
	cout << "     TrackerTLD == 2" << endl;
	cout << "     TrackerKCF == 3" << endl;
	cout << "     TrackerMedianFlow == 4" << endl;
	cout << "     TrackerBoosting == 5" << endl;
	cout << "     TrackerGOTURN == 6" << endl;
	cout << "     TrackerMOSSE == 7" << endl;
	cout << "entry number of tracker type::" << endl;
	int type_num = 0;
	cin >> type_num;
	Ptr<Tracker> tracker;
	switch (type_num)
	{
	case 1:
		tracker = TrackerMIL::create();
		break;
	case 2:
		tracker = TrackerTLD::create();
		break;
	case 3:
		tracker = TrackerKCF::create();
		break;
	case 4:
		tracker = TrackerMedianFlow::create();
		break;
	case 5:
		tracker = TrackerBoosting::create();
		break;
	case 6:
		tracker = TrackerGOTURN::create();
		break;
	case 7:
		tracker = TrackerMOSSE::create();
		break;
	default:
		break;
	}

	capture.read(frame);
	tracker->init(frame, bbox);
	//namedWindow("output", WINDOW_AUTOSIZE);
	while (capture.read(frame))
	{
		float start = getTickCount();
		tracker->update(frame, bbox);
		float end = getTickCount();
		cout << "times::" << ((end - start) * 1000)/ getTickFrequency() <<"ms"<< endl;
		rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
		imshow("output", frame);
		if (waitKey(15) == 27)
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
	return 0;
}

//框选目标
void draw_rectangle(int event, int x, int y, int flags, void*)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		previousPoint = Point(x, y);
	}
	else if (event == EVENT_MOUSEMOVE && (flags&EVENT_FLAG_LBUTTON))
	{
		Mat tmp;
		firstFrame.copyTo(tmp);
		currentPoint = Point(x, y);
		rectangle(tmp, previousPoint, currentPoint, Scalar(0, 255, 0, 0), 1, 8, 0);
		imshow("output", tmp);
	}
	else if (event == EVENT_LBUTTONUP)
	{
		bbox.x = previousPoint.x;
		bbox.y = previousPoint.y;
		bbox.width = abs(previousPoint.x - currentPoint.x);
		bbox.height = abs(previousPoint.y - currentPoint.y);
	}
	else if (event == EVENT_RBUTTONUP)
	{
		destroyWindow("output");
	}
}
