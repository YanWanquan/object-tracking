#pragma once
class ImgData
{
public:
	ImgData();
	~ImgData();

public:
	void readImage(char *InImgName, int * length);
	void writerImage(char *OutImgName, char * ImgBuffer, int length);


	void SetPara(int height, int width, char* ImgDataBuffer, int length = 0);
	
	int GetHeight();
	int GetWidth();
	char* GetImgDataBuffer();

private:
	//ͼ������
	int height;
	int width;

	//��ȡ���ݵ�BUFF
	int BufferLength;
	char* ImgDataBuffer;
};

