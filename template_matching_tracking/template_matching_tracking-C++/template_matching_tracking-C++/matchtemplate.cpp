#pragma once
#include "iostream"
#include "stdlib.h" 
#include "math.h"
#include "matchtemplate.h"

void CreatImgData(ImgData& src)
{
	src.BufData = NULL;
	src.col = 0;
	src.row = 0;
	src.x = 0;
	src.y = 0;
}
//深copy TODO implement
void DeepCopyImgData(ImgData src, ImgData& dst)
{
	if (src.BufData != NULL)
	{

	}
}
//浅copy
void CopyImgData(ImgData src, ImgData& dst)
{
	//不需要buf数据
	//if (src.BufData != NULL)
	//{
	//	dst = src;
	//}
	dst.row = src.row;
	dst.col = src.col;
	dst.x = src.x;
	dst.y = src.y;
}



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

//限制滑动的窗口大小  按照模板的大小 扩展一定倍数
int ChangeSearchWindow(ImgData pSrc, ImgData pTemplate, ImgData& pSrcWindow, ImgData Martix)
{
	if (pTemplate.x == 0 && pTemplate.y == 0)
		return -1;
	if (Martix.x == 0 && Martix.y == 0)
	{
		//pSrcWindow.row = pTemplate.row * 3;
		//pSrcWindow.col = pTemplate.col * 3;
		//pSrcWindow.x = pTemplate.x + pTemplate.col * 0.5 - pSrcWindow.col * 0.5;
		//pSrcWindow.y = pTemplate.y + pTemplate.row * 0.5 - pSrcWindow.row * 0.5;
		pSrcWindow.row = pTemplate.row * 2;
		pSrcWindow.col = pTemplate.col * 2;
		pSrcWindow.x = pTemplate.x - pTemplate.col * 0.5;
		pSrcWindow.y = pTemplate.y - pTemplate.row * 0.5;

	}
	else
	{
		//int xx = pTemplate.col % 10 * 2;
		//int yy = pTemplate.row % 10 * 2;
		//pSrcWindow.row = pTemplate.row + yy * 2;
		//pSrcWindow.col = pTemplate.col + xx * 2;
		//pSrcWindow.x = pTemplate.x + Martix.x - xx;
		//pSrcWindow.y = pTemplate.y + Martix.y - yy;
		pSrcWindow.row = pTemplate.row * 1.5;
		pSrcWindow.col = pTemplate.col * 1.5;
		pSrcWindow.x = pTemplate.x - pTemplate.col * 0.25;
		pSrcWindow.y = pTemplate.y - pTemplate.row * 0.25;
		pSrcWindow.x = pTemplate.x + Martix.x;
		pSrcWindow.y = pTemplate.y + Martix.y;
	}

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

//仅更新坐标 不更新模板的像素值，利用固定值
void updateCorr(ImgData dst, ImgData& imgTemplate, double Thro)
{
	if (((dst.x - imgTemplate.x) < Thro) || ((dst.y - imgTemplate.y) < Thro))
	{
		imgTemplate.x = dst.x;
		imgTemplate.y = dst.y;
	}

}
//仅更新坐标 不更新模板的像素值，利用ImgData
void updateCorr(ImgData dst, ImgData& imgTemplate, ImgData Thro)
{
	int xxThro = 20, yyThro = 20;
	if (Thro.x != 0 || Thro.y != 0)
	{
		xxThro = 5 * Thro.x;
		yyThro = 5 * Thro.y;
	}

	if ((std::abs(dst.x - imgTemplate.x) < xxThro) || (std::abs(dst.y - imgTemplate.y) < yyThro))
	{
		imgTemplate.x = dst.x;
		imgTemplate.y = dst.y;
	}

}


//无方向向量限制
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
			dSigmaT += (double)(pTemplate.BufData[n*pTemplate.col +m] * pTemplate.BufData[n*pTemplate.col + m]);
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

//方向向量限制
ImgData ImageMatch(ImgData pSrc, ImgData pTemplate, ImgData MartixThro)
{
	ImgData dstSize;//最终求出的坐标值
	CreatImgData(dstSize);
	ImgData pSrcWindow;
	if (MartixThro.x == 0 && MartixThro.y == 0)
	{
		int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow);
		if (ret < 0)	return dstSize;
	}
	else
	{
		int ret = ChangeSearchWindow(pSrc, pTemplate, pSrcWindow,MartixThro);
		if (ret < 0)	return dstSize;
	}

	//模板匹配
	//计算dSigmaT
	double dSigmaT = 0;
	for (int n = 0; n < pTemplate.row; n++)
	{
		for (int m = 0; m < pTemplate.col; m++)
		{
			// 指向模板图像的指针,全部求平方和
			//std::cout << n*pTemplate.col + m << std::endl; //test
			//dSigmaT += (double)(pTemplate.BufData[n*pTemplate.col + m] * pTemplate.BufData[n*pTemplate.col + m]);
			//模板像素跟着变化，传入仅用坐标
			dSigmaT += (double)(pTemplate.BufData[n*pTemplate.col + m] * pTemplate.BufData[n*pTemplate.col + m]);
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
	for (int i = pSrcWindow.y; i < pSrcWindow.y + pSrcWindow.row - pTemplate.row + 1; i++)//搜索窗口的y坐标，不需要最后一行窗口的滑动
	{
		for (int j = pSrcWindow.x; j < pSrcWindow.x + pSrcWindow.col + pTemplate.col + 1; j++)//搜索窗口的x坐标，不需要最后一列窗口的滑动
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
	/*if (dbMaxR < 0.85)
	{
		dstSize.x = pTemplate.x;
		dstSize.y = pTemplate.y;
	}*/
	if (dbMaxR < 0.95)
	{
		dstSize.x = pTemplate.x + MartixThro.x;
		dstSize.y = pTemplate.y + MartixThro.y;
	}
	else
	{
		dstSize.x = nMaxWidth;
		dstSize.y = nMaxHeight;
		/*if (pTemplate.x == nMaxWidth && pTemplate.y == nMaxHeight)
		{
			dstSize.x = dstSize.x + MartixThro.x;
			dstSize.y = dstSize.y + MartixThro.y;
		}*/
	}
	return dstSize;
}