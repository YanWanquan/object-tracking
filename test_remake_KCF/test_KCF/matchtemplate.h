#pragma once

typedef unsigned char uchar;
//ͼƬ����
struct ImgData
{
	uchar *BufData;
	//�и�
	int row;
	//�п�
	int col;
	//λ��ԭʼͼƬ������
	int x;
	int y;
};


void CreateMemory(ImgData& Data, int row = 0, int col = 0);

ImgData SetMemory(void* bufData, int row, int col, int x = 0, int y = 0);//�����������ݣ�ԭʼͼ����xy���꣬ģ��ͼ����Ҫx y����

int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow);

ImgData ImageMatch(ImgData pSrc, ImgData pTemplate);

void updateCorr(ImgData& dst, ImgData& imgTemplate, double Thro=20.0);