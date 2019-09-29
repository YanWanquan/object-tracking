#pragma once
#pragma once
#include "vector"
#include "matchtemplate.h"

/*
利用帧序列中检测物体的位置添加速度方向信息
方案：
N帧检测到物体框中心位置的偏移、距离、方向，即向量！
实现：
数据：方框，坐标，
函数：方框信息、中心位置存取  MartixStore
根据N帧信息进行求取向量  CalculationMartix  CalculationBbox
根据当前帧更新向量  UpdateBboxMartix
*/

ImgData AddMartix(ImgData box1, ImgData box2);

int DivideMartix(ImgData& box1, double num);


int MartixStore(std::vector<ImgData>& Bboxs, ImgData box, int FrameNum = 5);

ImgData CalculationMartix(ImgData box1, ImgData box2);

int CalculationBbox(std::vector<ImgData> Matrixs, ImgData& boxWindow, bool average = false);

//TODO：implement
int UpdateBboxMartix(ImgData Frame, std::vector<ImgData>& MultiFrames, std::vector<ImgData>& Martixs, ImgData& boxWindow, int FrameNum = 5);