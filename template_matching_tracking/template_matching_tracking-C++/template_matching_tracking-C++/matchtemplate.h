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
//初始化结构体
void CreatImgData(ImgData& src);
//深copy TODO implement
void DeepCopyImgData(ImgData src, ImgData& dst);
//浅copy
void CopyImgData(ImgData src, ImgData& dst);

void CreateMemory(ImgData& Data, int row, int col);

ImgData SetMemory(void* bufData, int row, int col, int x, int y);

//限制滑动的窗口大小
//可以使用输入数值进行扩展模板 xx扩展x row   yy扩展y col
int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow, ImgData Martix = {NULL,0,0,0,0});

//仅更新坐标 不更新模板的像素值
void updateCorr(ImgData dst, ImgData& imgTemplate, double Thro);
void updateCorr(ImgData dst, ImgData& imgTemplate, ImgData Thro);

//无方向向量限制
ImgData ImageMatch(ImgData pSrc, ImgData pTemplate);

//方向向量限制
ImgData ImageMatch(ImgData pSrc, ImgData pTemplate, ImgData MartixThro);
