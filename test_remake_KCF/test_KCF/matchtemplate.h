#pragma once

typedef unsigned char uchar;
//图片数据
struct ImgData
{
	uchar *BufData;
	//行高
	int row;
	//列宽
	int col;
	//位于原始图片的坐标
	int x;
	int y;
};


void CreateMemory(ImgData& Data, int row = 0, int col = 0);

ImgData SetMemory(void* bufData, int row, int col, int x = 0, int y = 0);//传进来的数据，原始图像不用xy坐标，模板图像需要x y坐标

int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow);

ImgData ImageMatch(ImgData pSrc, ImgData pTemplate);

void updateCorr(ImgData& dst, ImgData& imgTemplate, double Thro=20.0);