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

//限制滑动的窗口大小
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

//仅更新坐标 不更新模板的像素值
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
	ImgData dstSize;//最终求出的坐标值
	dstSize.x = 0;
	dstSize.y = 0;
	ImgData pSrcWindow;
	int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow);
	if (ret < 0)
		return dstSize;

	//计算dSigmaT
	double dSigmaT = 0;
	for (int n = 0; n < pTemplate.row; n++)
	{
		for (int m = 0; m < pTemplate.col; m++)
		{
			// 指向模板图像的指针,全部求平方和
			//std::cout << n*pTemplate.col + m << std::endl; //test
			double s = (double)(pTemplate.BufData[n*pTemplate.col + m] * pTemplate.BufData[n*pTemplate.col + m]);
			dSigmaT += s;
		}
	}

	//找到图像中最大相似性的出现位置
	double dSigmaST = 0;
	double dSigmaS = 0;
	//相似性测度
	double R = 0.0;
	//最大相似性测度
	double  dbMaxR = 0.0;
	//最大相似性出现位置
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
			//计算相似性
			R = dSigmaST / (sqrt(dSigmaS)*sqrt(dSigmaT));
			//与最大相似性比较
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