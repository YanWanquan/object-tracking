#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cstring>

using namespace std;
using namespace cv;

int main11() {
	// declares all required variables
	//! [vars]
	Rect2d roi;
	Mat frame;
	//! [vars]

	// create a tracker object
	//Ptr<Tracker> tracker = TrackerKCF::create();
	Ptr<Tracker> tracker = TrackerCSRT::create();
	//! [create]

	// set input video
	//! [setvideo]
	std::string video = "testHuman9.mp4";
	VideoCapture cap(video);
	//! [setvideo]

	// get bounding box
	//! [getframe]
	cap >> frame;
	//! [getframe]
	//! [selectroi]选择目标roi以GUI的形式
	medianBlur(frame, frame,3);
	roi = selectROI("tracker", frame);
	//! [selectroi]

	//quit if ROI was not selected
	if (roi.width == 0 || roi.height == 0)
		return 0;

	// initialize the tracker
	//! [init]
	tracker->init(frame, roi);
	//! [init]

	// perform the tracking process
	printf("Start the tracking process\n");
	for (;; ) {
		// get frame from the video
		cap >> frame;
		medianBlur(frame, frame,3);
		// stop the program if no more images
		if (frame.rows == 0 || frame.cols == 0)
			break;
		
		//time
		double time_start = getTickCount();

		// update the tracking result
		//! [update]
		tracker->update(frame, roi);
		//! [update]

		double time_end = getTickCount();
		cout << "time of one process" << (time_end - time_start) * 1000 / (getTickFrequency()) << "ms" << endl;
		//! [visualization]
		// draw the tracked object
		rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);

		// show image with the tracked object
		imshow("tracker", frame);
		//! [visualization]
		//quit on ESC button
		if (waitKey(10) == 27)
			break;
	}
	cap.release();
	destroyAllWindows();

	return 0;
}
