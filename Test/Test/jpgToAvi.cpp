#include <iostream>
#include <string>
#include <direct.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//批量读取图像
//利用命名规则读取
bool readBatImage(string file_name,string type_name ,int image_number)
{
	Mat image;
	string image_name;
	int n = 1;
	while (n<image_number)
	{
		image_name = "img";
		stringstream ss;
		string str;
		ss << n;
		ss >> str;
		image_name = file_name + image_name + str + "." + type_name;
		cout << "processing: " << image_name << endl;
		image = imread(image_name);
		if (image.empty())
		{
			cout << "error: " << image_name << endl;

		}
	}
	return true;

}

//批量读取图像
//利用txt文件读取
bool readBatImage(string file_name, string txt_name)
{
	Mat image;
	string image_name;
	txt_name = file_name + "\\" + txt_name;
	ifstream inf(txt_name);
	while (getline(inf,image_name))
	{
		cout << "processing: " << image_name << endl;
		image_name = file_name + "\\" + image_name;
		image = imread(image_name);
		if (image.empty())
		{
			cout << "error: " << image_name << endl;
		}
		


	}
	return true;

}

//批量读取图像
//读取指定文件夹下的所有文件   #include <direct.h>
//bool readBatImage(string file_name)
//{
//	static char dot[] = ".", dotdot[] = "..";
//	const char *name;
//	DIR *dirp;
//	struct dirent *dp;
//
//	if (argc == 2)
//		name = argv[1];
//	else
//		name = dot;
//	printf(" the request dir name is %s\n", name);
//
//	//open the request dir.   
//	//return DIR pointer if open succeed.   
//	//return NULL if opendir failed.   
//	dirp = opendir(name);
//	if (dirp == NULL) {
//		fprintf(stderr, "%s: opendir(): %s: %s\n",
//			argv[0], name, strerror(errno));
//		exit(errno);
//	}
//	else {
//		printf("opendir %s succeed!\n", name);
//	}
//
//
//	//readdir(dirent)   
//	//return dirent pointer if readdir succeed.   
//	//return NULL if readdir failed.   
//	while ((dp = readdir(dirp)) != NULL) {
//		//判断文件类型是DT_DIR, 也就是目录   
//		if (dp->d_type == DT_DIR)
//			//如果文件名不是"."　或者"..",就打印出来   
//			if (strcmp(dp->d_name, dot)
//				&& strcmp(dp->d_name, dotdot))
//				printf("%s/\n", dp->d_name);
//	}
//
//	//closedir(dirent)   
//	closedir(dirp);
//
//}

//读取一个目录下的jpg 转换成视频
/*
CV_FOURCC(‘P’, ‘I’, ‘M’, ‘1’) = MPEG-1 codec
CV_FOURCC(‘M’, ‘J’, ‘P’, ‘G’) = motion-jpeg codec
CV_FOURCC(‘M’, ‘P’, ‘4’, ‘2’) = MPEG-4.2 codec
CV_FOURCC(‘D’, ‘I’, ‘V’, ‘3’) = MPEG-4.3 codec
CV_FOURCC(‘D’, ‘I’, ‘V’, ‘X’) = MPEG-4 codec
CV_FOURCC(‘U’, ‘2’, ‘6’, ‘3’) = H263 codec
CV_FOURCC(‘I’, ‘2’, ‘6’, ‘3’) = H263I codec
CV_FOURCC(‘F’, ‘L’, ‘V’, ‘1’) = FLV1 codec
*/
bool writerBatImage()
{
	// 从一个文件夹下读取多张jpg图片
	String pattern = "C:\\Users\\cjy\\Desktop\\LY\\视频\\OTB50\\Walking\\img\\*.jpg";
	vector<String> fn;
	glob(pattern, fn, false);

	// 构造一个VideoWriter
	Mat tmp_image = imread(fn[0]);
	int cols = tmp_image.cols;
	int rows = tmp_image.rows;
	VideoWriter video("testHuman9.mp4", CV_FOURCC('D', 'I', 'V', 'X'), 15.0, Size(cols, rows));
	
	size_t count = fn.size();
	for (size_t i = 0; i < count; i++)
	{
		Mat image = imread(fn[i]);
		// 这个大小与VideoWriter构造函数中的大小一致。
		resize(image, image, Size(cols, rows));
		// 流操作符，把图片传入视频
		video << image;
	}
	cout << "处理完毕！" << endl;
	// 处理完之后会在得到一个名为test.avi的视频文件。
	return true;
}


void main1()
{
	//string file_name = "C:\\Users\\cjy\\Desktop\\LY\\视频\\OTB50\\Basketball\\img";
	//string txt_name = "readimage.txt";
	//readBatImage(file_name,txt_name);

	writerBatImage();

	return;
}
