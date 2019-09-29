#pragma once
#include "vector"
#include "speeddetection.h"

int MartixStore(std::vector<ImgData>& Bboxs, ImgData box, int FrameNum)
{
	if (box.x == 0 & box.y == 0)
	{
		return -1;
	}
	if (Bboxs.size() < FrameNum)
	{
		Bboxs.push_back(box);
	}
	else
	{
		Bboxs.erase(Bboxs.begin());
		Bboxs.push_back(box);
	}
	return 1;
}

ImgData CalculationMartix(ImgData box1, ImgData box2)
{
	ImgData result;
	CreatImgData(result);
	if ((box1.x == 0 & box1.y == 0) || (box2.x == 0 & box2.y == 0))
		return result;
	result.x = box2.x - box1.x;
	result.y = box2.y - box1.y;
	return result;
}

//向量相加
ImgData AddMartix(ImgData box1, ImgData box2)
{
	ImgData result;
	result.x = box2.x + box1.x;
	result.y = box2.y + box1.y;
	return result;
}

//向量除以数值
int DivideMartix(ImgData& box, double num)
{
	if (num == 0)
		return 0;
	box.x = box.x / num;
	box.y = box.y / num;
	return 1;
}


//考虑前几帧移动向量直接相加 还是考虑取平均
//average代表是否平均，默认false
int CalculationBbox(std::vector<ImgData> Matrixs, ImgData& boxWindow, bool average)
{
	if (Matrixs.size() < 2)
		return -1;
	ImgData MatrixTotal;
	CreatImgData(MatrixTotal);
	for (int i = 0; i < Matrixs.size(); i++)
	{
		MatrixTotal = AddMartix(Matrixs[i], MatrixTotal);
	}
	if (average == true)
		DivideMartix(MatrixTotal, Matrixs.size());
	CopyImgData(MatrixTotal, boxWindow);
	return 1;
}

//TODO：implement
int UpdateBboxMartix(ImgData Frame,std::vector<ImgData>& MultiFrames, std::vector<ImgData>& Martixs, ImgData& boxWindow,int FrameNum)
{
	if ((Frame.x == 0 && Frame.y == 0) || (Frame.col == 0 && Frame.row == 0))
		return -1;
	//将多帧数据进行存储
	MartixStore(MultiFrames, Frame, FrameNum);
	if (MultiFrames.size() < 2) return -1;

	//计算多帧数据的移动向量
	/*for (int i = 1; i < MultiFrames.size(); i++)
	{
		ImgData Martix = CalculationMartix(MultiFrames[i], MultiFrames[i-1]);
		MartixStore(MultiFrames, Frame, FrameNum);
	}*/

	//仅考虑新增加的，速度更快
	//ImgData Martix;
	//CreatImgData(Martix);
	//CopyImgData(CalculationMartix(MultiFrames[MultiFrames.size() - 2], MultiFrames[MultiFrames.size() - 1]), Martix);

	ImgData Martix = CalculationMartix(MultiFrames[MultiFrames.size() - 2], MultiFrames[MultiFrames.size() - 1]);

	MartixStore(Martixs, Martix, FrameNum);

	//方向向量限制
	if (Martixs.size() > 1)
	{
		CalculationBbox(Martixs, boxWindow, true);
	}

	return 1;
}