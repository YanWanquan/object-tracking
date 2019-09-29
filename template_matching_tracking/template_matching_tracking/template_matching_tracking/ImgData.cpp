#include "stdafx.h"
#include "ImgData.h"
#include<iostream>


ImgData::ImgData()
{
	this->height = 0;
	this->width = 0;
	this->BufferLength = 0;
	this->ImgDataBuffer = NULL;
}


ImgData::~ImgData()
{
	
}

void ImgData::SetPara(int height, int width, char* ImgDataBuffer, int length)
{
	this->height = height;
	this->width = width;
	if (this->ImgDataBuffer != NULL)
	{
		ImgDataBuffer = NULL;
		this->BufferLength = 0;
	}
	this->ImgDataBuffer = ImgDataBuffer;
	this->BufferLength = length;
}

int ImgData::GetHeight()
{
	return this->height;
}

int ImgData::GetWidth()
{
	return this->width;
}

char* ImgData::GetImgDataBuffer()
{
	return this->ImgDataBuffer;
}

void ImgData::readImage(char *InImgName, int * length)
{
	//文件指针
	FILE* fp;
	//以二进制方式打开图像
	if ((fp = fopen(InImgName, "rb")) == NULL)
	{
		std::cout << "Open image failed!" << std::endl;
		std::exit(0);
	}
	//获取图像数据总长度
	fseek(fp, 0, SEEK_END);
	this->BufferLength = ftell(fp);
	rewind(fp);
	//根据图像数据长度分配内存buffer
	this->ImgDataBuffer = (char*)malloc(this->BufferLength * sizeof(char));
	//将图像数据读入buffer
	fread(this->ImgDataBuffer, this->BufferLength, 1, fp);
	fclose(fp);
	*length = this->BufferLength;
	return;
}

/*
//输入要保存的文件名
char OutImgName[10];
cout << "Enter the name you wanna to save:";
cin >> OutImgName;
if (OutImgName != NULL && ImgBuffer!=NULL)
{
writerImage(OutImgName, ImgBuffer, length);
free(ImgBuffer);
}
*/
void ImgData::writerImage(char *OutImgName, char * ImgBuffer, int length)
{
	//文件指针
	FILE* fp;
	//以二进制写入方式
	if ((fp = fopen(OutImgName, "wb")) == NULL)
	{
		std::cout << "Open File failed!" << std::endl;
		std::exit(0);
	}
	//从buffer中写数据到fp指向的文件中
	fwrite(ImgBuffer, length, 1, fp);
	std::cout << "Done!" << std::endl;
	//关闭文件指针，释放buffer内存
	fclose(fp);
}

