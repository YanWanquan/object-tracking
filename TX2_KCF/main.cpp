#include <QCoreApplication>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "kcftracker.h"
#include <dirent.h>
#include  <QImage>
#include "SerialPort.h"

using namespace cv;
using namespace std;

RNG g_rng(12345);//毛大大的博客里看到的生成随机数，用于生成随机颜色
bool isDrawRect = false;//不可避免地还是要定义几个全局变量，伤心
Point LClicked=Point(-1,-1);
Point mouseLocation= Point(-1, -1);
bool isReady = false;

void on_mouse(int event, int x, int y, int flags, void* ustc)
{
    Mat& image = *(cv::Mat*) ustc;//这样就可以传递Mat信息了，很机智
    char temp[16];

    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN://按下左键
        {
            sprintf(temp, "(%d,%d)", x, y);
            putText(image, temp, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
            isDrawRect = true;
            LClicked= Point(x, y);

            cout << "CV_EVENT_LBUTTONDOWN" << endl;

        }   break;
        case CV_EVENT_MOUSEMOVE://移动鼠标
        {
            mouseLocation = Point(x, y);
            cout << "CV_EVENT_MOUSEMOVE" << endl;
            if (isDrawRect)
            { }
        }break;
        case EVENT_LBUTTONUP:
        {
            cout << "EVENT_LBUTTONUP" << endl;

            isDrawRect = false;
            sprintf(temp, "(%d,%d)", x, y);
            putText(image, temp, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
            //调用函数进行绘制
            cv::rectangle(image, LClicked, mouseLocation, cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)));//随机颜色
            isReady = true;

        }break;

    default:
        break;
    }
}


int main()
{

    //system("color 9F");//毛大大程序里的，改变console颜色
    //setMouseCallback("img", on_mouse, 0);//调用回调函数

    bool HOG = true;
    bool FIXEDWINDOW = false;
    bool MULTISCALE = true;
    bool SILENT = false;
    bool LAB = true;//true

    // Create KCFTracker object
    KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

    // Frame readed
    Mat frame;

    // Tracker results
    Rect result;

    // Frame counter
    int nFrames = 0;

    float xMin = 150;
    float yMin = 164;
    float width = 100;
    float height = 150;

    VideoCapture cam(0);
    /*
    string outFlie = "E:/3.avi";
    VideoWriter write;
    //获得帧的宽高
    int w = static_cast<int>(1920);//cam.get(CV_CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(1080);//cam.get(CV_CAP_PROP_FRAME_HEIGHT));
    Size S(w, h);
    //获得帧率
    double r = cam.get(CV_CAP_PROP_FPS);
    //打开视频文件，准备写入
    write.open(outFlie, -1, r, S, true);
*/

    //double rate = cam.get(CV_CAP_PROP_FPS);
        //获取视频帧的尺寸
   //     int Width = cam.get(CV_CAP_PROP_FRAME_WIDTH);
   //     int Height = cam.get(CV_CAP_PROP_FRAME_HEIGHT);
        //根据打开视频的参数初始化输出视频格式
   //     cv::VideoWriter w_cap("E:/3.avi", CV_FOURCC('M', 'J', 'P', 'G'), rate, cv::Size(Width, Height));

    //SerialPort serialPort;

    while ( 1 )
    {
        Mat temp1;
        Mat temp2;

        if ( !cam.isOpened() )
        {
            exit(0);
        }

        cam >> frame;



        if( isReady )
        {
            nFrames = 0;

            xMin = LClicked.x;
            yMin = LClicked.y;
            width = mouseLocation.x - LClicked.x;
            height = mouseLocation.y - LClicked.y;

        }

        isReady = false;

        int j = 0;
        // First frame, give the groundtruth to the tracker
        if (nFrames == 0) {
            tracker.init( Rect(xMin, yMin, width, height), frame );
            rectangle( frame, Point( xMin, yMin ), Point( xMin+width, yMin+height), Scalar( 0, 255, 255 ), 2, 8 );
            //tracker.init( Rect(LClicked.x, LClicked.y, mouseLocation.x, mouseLocation.y), frame );
        }
        // Udate
        else
        {
            result = tracker.update(frame);
            rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 2, 8 );

            int centerX = result.x + result.width/2.0;
            int centerY = result.y + result.height/2.0;

            int frame_width = frame.cols / 2;
            int frame_height = frame.rows / 2;

            int errorX = frame_width - centerX;
            int errorY = frame_height - centerY;


            line(frame, Point(centerX, centerY), Point(frame_width, frame_height), cv::Scalar(0, 0, 255));

            char cX[10];
            char cY[10];

            itoa(errorX, cX, 10);
            itoa(errorY, cY, 10);

            putText(frame, "(" + string(cX) + "," + string(cY) + ")", Point(frame_width, frame_height), FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 0, 0));

            //serialPort.writeData(frame_width-centerX, frame_width-centerY);

            //w_cap.write(frame);

            //j++;

            //if( j == 50 )
            //{
           //     w_cap.release();

          //      break;
          //  }

        }

        nFrames++;

        if (!SILENT)
        {
            namedWindow("Image");
            imshow("Image", frame);
            setMouseCallback("Image", on_mouse, &frame);  //注册鼠标相应回调函数

            waitKey(1);
        }
    }

    return 0;
}
