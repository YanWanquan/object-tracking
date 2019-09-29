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
	//图像数据
	int height;
	int width;

	//读取数据的BUFF
	int BufferLength;
	char* ImgDataBuffer;
};

