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
//��ʼ���ṹ��
void CreatImgData(ImgData& src);
//��copy TODO implement
void DeepCopyImgData(ImgData src, ImgData& dst);
//ǳcopy
void CopyImgData(ImgData src, ImgData& dst);

void CreateMemory(ImgData& Data, int row, int col);

ImgData SetMemory(void* bufData, int row, int col, int x, int y);

//���ƻ����Ĵ��ڴ�С
//����ʹ��������ֵ������չģ�� xx��չx row   yy��չy col
int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow, ImgData Martix = {NULL,0,0,0,0});

//���������� ������ģ�������ֵ
void updateCorr(ImgData dst, ImgData& imgTemplate, double Thro);
void updateCorr(ImgData dst, ImgData& imgTemplate, ImgData Thro);

//�޷�����������
ImgData ImageMatch(ImgData pSrc, ImgData pTemplate);

//������������
ImgData ImageMatch(ImgData pSrc, ImgData pTemplate, ImgData MartixThro);
