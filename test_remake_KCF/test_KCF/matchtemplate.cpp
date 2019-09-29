#include "iostream"
#include "stdlib.h" 
#include "math.h"
#include "matchtemplate.h"
void CreateMemory(ImgData& Data, int row, int col)
{
	if (row != 0)
	{
		Data.row = row;
		Data.col = col;
	}
	Data.BufData = (uchar *)malloc(Data.row* Data.col * sizeof(uchar *));
}

ImgData SetMemory(void* bufData,int row, int col,int x,int y)
{
	ImgData Data;
	Data.x = x;
	Data.y = y;
	if (bufData == NULL)
		return Data;
	CreateMemory(Data, row, col);
	Data.BufData = (uchar *)bufData;
	return Data;
}

//���ƻ����Ĵ��ڴ�С
int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow)
{
	if (pTemplate.x == 0 && pTemplate.y == 0)
		return -1;
	pSrcWindow.row = pTemplate.row * 3;
	pSrcWindow.col = pTemplate.col * 3;
	pSrcWindow.x = pTemplate.x + pTemplate.col * 0.5 - pSrcWindow.col * 0.5;
	pSrcWindow.y = pTemplate.y + pTemplate.row * 0.5 - pSrcWindow.row * 0.5;

	if (pSrcWindow.x < 0)
		pSrcWindow.x = 0;
	if (pSrcWindow.x > pSrc.col)
		pSrcWindow.x = pSrc.col;

	if (pSrcWindow.y < 0)
		pSrcWindow.y = 0;
	if (pSrcWindow.y > pSrc.row)
		pSrcWindow.y = pSrc.row;
	return 0;
}

//���������� ������ģ�������ֵ
void updateCorr(ImgData& dst, ImgData& imgTemplate, double Thro)
{
	if (((dst.x - imgTemplate.x) < Thro) || ((dst.y - imgTemplate.y) < Thro))
	{
		imgTemplate.x = dst.x;
		imgTemplate.y = dst.y;
	}

}

ImgData ImageMatch(ImgData pSrc, ImgData pTemplate)
{
	ImgData dstSize;//�������������ֵ
	dstSize.x = 0;
	dstSize.y = 0;
	ImgData pSrcWindow;
	int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow);
	if (ret < 0)
		return dstSize;

	//����dSigmaT
	double dSigmaT = 0;
	for (int n = 0; n < pTemplate.row; n++)
	{
		for (int m = 0; m < pTemplate.col; m++)
		{
			// ָ��ģ��ͼ���ָ��,ȫ����ƽ����
			//std::cout << n*pTemplate.col + m << std::endl; //test
			double s = (double)(pTemplate.BufData[n*pTemplate.col + m] * pTemplate.BufData[n*pTemplate.col + m]);
			dSigmaT += s;
		}
	}

	//�ҵ�ͼ������������Եĳ���λ��
	double dSigmaST = 0;
	double dSigmaS = 0;
	//�����Բ��
	double R = 0.0;
	//��������Բ��
	double  dbMaxR = 0.0;
	//��������Գ���λ��
	int nMaxWidth = 0;
	int nMaxHeight = 0;
	for (int i = pSrcWindow.y; i < pSrcWindow.y + pSrcWindow.row - pTemplate.row + 1; i++)
	{
		for (int j = pSrcWindow.x; j < pSrcWindow.x + pSrcWindow.col + pTemplate.col + 1; j++)
		{
			dSigmaST = 0;
			dSigmaS = 0;

			for (int n = 0; n < pTemplate.row; n++)
			{
				for (int m = 0; m < pTemplate.col; m++)
				{
					int tmp_pos = (pSrc.col*(i + n) + j + m);

					double unchPixel = (unsigned char)pSrc.BufData[tmp_pos];
					double unchTemplatePixel = (unsigned char)pTemplate.BufData[n*pTemplate.col + m];

					dSigmaS += (double)unchPixel*unchPixel;
					dSigmaST += (double)unchPixel*unchTemplatePixel;
				}
			}
			//����������
			R = dSigmaST / (sqrt(dSigmaS)*sqrt(dSigmaT));
			//����������ԱȽ�
			if (R >  dbMaxR)
			{
				dbMaxR = R;
				nMaxHeight = i;
				nMaxWidth = j;
			}
		}
	}
	if (dbMaxR < 0.85)
	{
		dstSize.x = pTemplate.x;
		dstSize.y = pTemplate.y;
	}
	else
	{
		dstSize.x = nMaxWidth;
		dstSize.y = nMaxHeight;
	}
	return dstSize;
}