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
	//�ļ�ָ��
	FILE* fp;
	//�Զ����Ʒ�ʽ��ͼ��
	if ((fp = fopen(InImgName, "rb")) == NULL)
	{
		std::cout << "Open image failed!" << std::endl;
		std::exit(0);
	}
	//��ȡͼ�������ܳ���
	fseek(fp, 0, SEEK_END);
	this->BufferLength = ftell(fp);
	rewind(fp);
	//����ͼ�����ݳ��ȷ����ڴ�buffer
	this->ImgDataBuffer = (char*)malloc(this->BufferLength * sizeof(char));
	//��ͼ�����ݶ���buffer
	fread(this->ImgDataBuffer, this->BufferLength, 1, fp);
	fclose(fp);
	*length = this->BufferLength;
	return;
}

/*
//����Ҫ������ļ���
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
	//�ļ�ָ��
	FILE* fp;
	//�Զ�����д�뷽ʽ
	if ((fp = fopen(OutImgName, "wb")) == NULL)
	{
		std::cout << "Open File failed!" << std::endl;
		std::exit(0);
	}
	//��buffer��д���ݵ�fpָ����ļ���
	fwrite(ImgBuffer, length, 1, fp);
	std::cout << "Done!" << std::endl;
	//�ر��ļ�ָ�룬�ͷ�buffer�ڴ�
	fclose(fp);
}

