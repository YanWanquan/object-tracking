#pragma once
#pragma once
#include "vector"
#include "matchtemplate.h"

/*
����֡�����м�������λ������ٶȷ�����Ϣ
������
N֡��⵽���������λ�õ�ƫ�ơ����롢���򣬼�������
ʵ�֣�
���ݣ��������꣬
������������Ϣ������λ�ô�ȡ  MartixStore
����N֡��Ϣ������ȡ����  CalculationMartix  CalculationBbox
���ݵ�ǰ֡��������  UpdateBboxMartix
*/

ImgData AddMartix(ImgData box1, ImgData box2);

int DivideMartix(ImgData& box1, double num);


int MartixStore(std::vector<ImgData>& Bboxs, ImgData box, int FrameNum = 5);

ImgData CalculationMartix(ImgData box1, ImgData box2);

int CalculationBbox(std::vector<ImgData> Matrixs, ImgData& boxWindow, bool average = false);

//TODO��implement
int UpdateBboxMartix(ImgData Frame, std::vector<ImgData>& MultiFrames, std::vector<ImgData>& Martixs, ImgData& boxWindow, int FrameNum = 5);