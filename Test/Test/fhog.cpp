#include "fhog.h"

#ifdef HAVE_TBB
#include <tbb/tbb.h>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/**********************************************************************************
// Getting feature map for the selected subimage
//
// API
// int getFeatureMaps(const IplImage * image, const int k, featureMap **map);
// INPUT
// image             - selected subimage
// k                 - size of cells
// OUTPUT
// map               - feature map
// RESULT
// Error status
函数功能：计算image的hog特征，结果在map结构中的map大小为sizeXsizeYNUM_SECTOR3（Getting feature map for the selected subimage）
函数参数:选中的子图，cell的大小，返回的特征图
RESULT：Error status
总体过程是：
1.计算每个像素的水平梯度dx和垂直梯度dy
2.计算每个像素的通道间最大梯度大小r及其最邻近梯度方向的索引值
3.计算每个block(2+4+2)(2+4+2)的梯度直方图（分为9和18bin）存于map中
	每个block的特征是以一个cell为中心，根据像素的位置以及像素点的梯度强度进行加权获得的
**********************************************************************************/
int getFeatureMaps(const IplImage* image, const int k, CvLSVMFeatureMapCaskade **map)
{
    int sizeX, sizeY;
    int p, px, stringSize;
    int height, width, numChannels;
    int i, j, kk, c, ii, jj, d;
    float  * datadx, * datady;

    int   ch;
    float magnitude, x, y, tx, ty;

    IplImage * dx, * dy;
    int *nearest;
    float *w, a_x, b_x;

    // 横向和纵向的3长度{-1，0，1}矩阵
    float kernel[3] = {-1.f, 0.f, 1.f};
    CvMat kernel_dx = cvMat(1, 3, CV_32F, kernel);          // 1*3的矩阵
    CvMat kernel_dy = cvMat(3, 1, CV_32F, kernel);          // 3*1的矩阵

    float * r;      //记录每个像素点的每个通道的最大梯度
    int   * alfa;   //记录每个像素的梯度方向的索引值，分别为9份时的索引值和18份时的索引值。

    float boundary_x[NUM_SECTOR + 1];                       // boundary_x[10]
    float boundary_y[NUM_SECTOR + 1];
    float max, dotProd;
    int   maxi;

    height = image->height;
    width  = image->width ;

    numChannels = image->nChannels;

    // 采样图像大小的Ipl图像
    dx    = cvCreateImage(cvSize(image->width, image->height),
                          IPL_DEPTH_32F, 3);
    dy    = cvCreateImage(cvSize(image->width, image->height),
                          IPL_DEPTH_32F, 3);

    // 向下取整的（边界大小/4），k = cell_size
    sizeX = width  / k;		// 将图像分割成多个元胞(cell),x方向上cell的个数
    sizeY = height / k;		// y方向上cell的个数
    px    = 3 * NUM_SECTOR;     // px=3*9=27 三通道？NUM_SECTOR=9 Hog特征中的9个角度范围
    p     = px;
    stringSize = sizeX * p;     // stringSize = 27*sizeX
    allocFeatureMapObject(map, sizeX, sizeY, p);  // 为map初始化内存sizeX*sizeY*p=sizeY*stringSize


    // image：输入图像.
    // dx：输出图像.
    // kernel_dx：卷积核, 单通道浮点矩阵. 如果想要应用不同的核于不同的通道，先用 cvSplit 函数分解图像到单个色彩通道上，然后单独处理。
    // cvPoint(-1, 0)：核的锚点表示一个被滤波的点在核内的位置。 锚点应该处于核内部。缺省值 (-1,-1) 表示锚点在核中心。
    // 函数 cvFilter2D 对图像进行线性滤波，支持 In-place 操作。当核运算部分超出输入图像时，函数从最近邻的图像内部象素差值得到边界外面的象素值。
    cvFilter2D(image, dx, &kernel_dx, cvPoint(-1, 0));      // 起点在(x-1,y)，按x方向滤波
    cvFilter2D(image, dy, &kernel_dy, cvPoint(0, -1));      // 起点在(x,y-1)，按y方向滤波

    // 初始化cos和sin函数
    float arg_vector;
	// 计算梯度角的边界，并存储在boundary__y中
    for(i = 0; i <= NUM_SECTOR; i++)
    {
        arg_vector    = ( (float) i ) * ( (float)(PI) / (float)(NUM_SECTOR) );  // 每个角的角度
        boundary_x[i] = cosf(arg_vector);  // 每个角度对应的余弦值
        boundary_y[i] = sinf(arg_vector);  // 每个角度对应的正弦值
    }/*for(i = 0; i <= NUM_SECTOR; i++) */

    r    = (float *)malloc( sizeof(float) * (width * height));   //3通道中对应的最大梯度幅值
    alfa = (int   *)malloc( sizeof(int  ) * (width * height * 2)); //幅值对应的方向角度最靠近那个bins的索引值

    for(j = 1; j < height - 1; j++)
    {
		// 记录每一行的首地址
		datadx = (float*)(dx->imageData + dx->widthStep * j);
        datady = (float*)(dy->imageData + dy->widthStep * j);

        // 遍历该行每一个元素 // 遍历一行中的非边界像素
        for(i = 1; i < width - 1; i++)
        {
            // 第一颜色通道
            c = 0;
            x = (datadx[i * numChannels + c]);
            y = (datady[i * numChannels + c]);

            r[j * width + i] =sqrtf(x * x + y * y);   // 计算0通道的梯度大小

            // 使用梯度向量大小最大的通道替代储存值
            for(ch = 1; ch < numChannels; ch++)		// 计算其他两个通道
            {
                tx = (datadx[i * numChannels + ch]);
                ty = (datady[i * numChannels + ch]);
                magnitude = sqrtf(tx * tx + ty * ty);	// 计算幅值
				// 找出每个像素点的梯度的最大值（有三个颜色空间对应的梯度），并记录通道数以及水平梯度以及垂直梯度
                if(magnitude > r[j * width + i])	
                {
                    r[j * width + i] = magnitude; // r表示最大幅值
                    c = ch;	// c表示这个幅值来自的通道序号
                    x = tx;	// x表示这个幅值对应的坐标处的x梯度
                    y = ty;	// y表示这个幅值对应的坐标处的y梯度
                }
            }/*for(ch = 1; ch < numChannels; ch++)*/

            // 使用sqrt（cos*x*cos*x+sin*y*sin*y）最大的替换掉
            max  = boundary_x[0] * x + boundary_y[0] * y;   // max = 1*x+0*y;
            maxi = 0;
			// 假设像素点的梯度方向为a,梯度方向为t,梯度大小为r,则dotProd=r*cosa*cost+r*sina*sint=r*cos(a-t)
            for (kk = 0; kk < NUM_SECTOR; kk++)	// 遍历9个HOG划分的角度范围
            {
                dotProd = boundary_x[kk] * x + boundary_y[kk] * y;	// 计算两个向量的点乘 点乘代表两个向量的角度 即相关性
				// 若dotProd最大，则说明t最接近a
				if (dotProd > max)	
                {
                    max  = dotProd;
                    maxi = kk;
                }
				// 若-dotProd最大，则说明t最接近a+pi
                else
                {
                    if (-dotProd > max)
                    {
                        max  = -dotProd;					// 取相反数
                        maxi = kk + NUM_SECTOR;             // 周期的，所以+一个周期NUM_SECTOR
                    }
                }
            }
            // 看起来有点像储存cos和sin的周期值
            alfa[j * width * 2 + i * 2    ] = maxi % NUM_SECTOR;
            alfa[j * width * 2 + i * 2 + 1] = maxi;
        }/*for(i = 0; i < width; i++)*/
    }/*for(j = 0; j < height; j++)*/
#if 0
	printf("magnitude and angle of Hog\n");
	for (int i = 0; i < (width * height); i++)
	{
		printf("r[%d]: %f \n",i, *(r+i));
		
	}
	for (int i = 0; i < (width * height*2); i++)
	{
		printf("alfa[%d]: %d \n", i,*(alfa+i));

	}
#endif
    nearest = (int  *)malloc(sizeof(int  ) *  k);
    w       = (float*)malloc(sizeof(float) * (k * 2));  //TODO:应该是梯度方向归属于那个bins 采用双线性差值的方式
	// 给nearest初始化，为了方便以后利用相邻的cell的特征计算block（8*8，每个block以一个cell为中心，以半个cell为边界厚度）的属性
    // nearest=[-1,-1,1,1];
    for(i = 0; i < k / 2; i++)
    {
        nearest[i] = -1;
    }/*for(i = 0; i < k / 2; i++)*/
    for(i = k / 2; i < k; i++)
    {
        nearest[i] = 1;
    }/*for(i = k / 2; i < k; i++)*/
	 //给w初始化？不明白w的作用，可能是cell（4*4）中每个像素贡献给直方图的权值（1/8+3/8+5/8+7/8+7/8+5/8+3/8+1/8）*（1/8+3/8+5/8+7/8+7/8+5/8+3/8+1/8）=4*4
    // 这算的都是啥？怎么没在算法上看见这一段？？？
    //        1/a          1/b
    // w[1]=_______  w[2]=_______
    //      1/a+1/b       1/a+1/b
    for(j = 0; j < k / 2; j++)
    {
        b_x = k / 2 + j + 0.5f;
        a_x = k / 2 - j - 0.5f;
        w[j * 2    ] = 1.0f/a_x * ((a_x * b_x) / ( a_x + b_x));
        w[j * 2 + 1] = 1.0f/b_x * ((a_x * b_x) / ( a_x + b_x));
    }/*for(j = 0; j < k / 2; j++)*/
    for(j = k / 2; j < k; j++)
    {
        a_x = j - k / 2 + 0.5f;
        b_x =-j + k / 2 - 0.5f + k;
        w[j * 2    ] = 1.0f/a_x * ((a_x * b_x) / ( a_x + b_x));
        w[j * 2 + 1] = 1.0f/b_x * ((a_x * b_x) / ( a_x + b_x));
    }/*for(j = k / 2; j < k; j++)*/
#if 0
	for (int i = 0; i < (k * 2); i++)
	{
		printf("w[%d]: %f \n", i, *(w + i));
	}
#endif

    // 计算梯度的公式好像和算法不太一样，应该是经过了某种离奇的推倒
	/*
		imagesize = w*h = 40*104
		cellsize = k = 4 
		sizeY = h / cellsize = 104 /4 = 26
		sizeX = w / cellsize = 40 /4 = 10
		则i代表了图像划分为cell后的行数
		  j代表了图像划分为cell后的列数
		则ii代表了每个cell的行数
		  jj代表了每个cell的列数
		首先if判断是去掉image的边界像素，即i=0 or j=0 or i=height or j=width
		d则为每个cell里面4*4像素在图像中的坐标索引   
			需注意：cell为4*4 但是为了去掉边界，故第一行 最后一行 第一列 最后一列在cell中是不参与计算的
			故：第一行的cell为3*3（第一行 第一列不参与） （往后是第一行不参与）3*4 3*4 。。。3*3（第一行和最后一列） 
				第二行的cell为4*3（第一列不参与） （往后是正常）4*4 4*4 。。。 4*3（最后一列不参与）
				以此类推。。。
				最后一行的cell为3*3（最后一行 第一列不参与） （往后是最后一行不参与）3*4 3*4 。。。 3*3（第一行和最后一列）
		然后将每个cell对应的梯度方向 幅值对应相加得到mapfeature   w代表权重
		然后再将-1 -1 1 1组成的nearest 即2*2cell组成的block相加得到最后的mapfeature
	*/
    for(i = 0; i < sizeY; i++)
    {
      for(j = 0; j < sizeX; j++)
      {
        for(ii = 0; ii < k; ii++)
        {
          for(jj = 0; jj < k; jj++)
          {
			  //第i行的第j个cell的第ii行第jj个像素
            if ((i * k + ii > 0) &&
                (i * k + ii < height - 1) &&
                (j * k + jj > 0) &&
                (j * k + jj < width  - 1)) //要跳过厚度为1的边界像素，因为边界的梯度值不准确，但这样会导致含有边界的cell统计不完整
            {
              d = (k * i + ii) * width + (j * k + jj); 
              (*map)->map[ i * stringSize + j * (*map)->numFeatures + alfa[d * 2    ]] +=
                  r[d] * w[ii * 2] * w[jj * 2];//第i行第j个cell的第alfa[d * 2]个梯度方向（0-8）
              (*map)->map[ i * stringSize + j * (*map)->numFeatures + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2] * w[jj * 2];//第i行第j个cell的第alfa[d * 2+1]个梯度方向（9-26）
              if ((i + nearest[ii] >= 0) &&
                  (i + nearest[ii] <= sizeY - 1))
              {
                (*map)->map[(i + nearest[ii]) * stringSize + j * (*map)->numFeatures + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 ];
                (*map)->map[(i + nearest[ii]) * stringSize + j * (*map)->numFeatures + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 ];
              }
              if ((j + nearest[jj] >= 0) &&
                  (j + nearest[jj] <= sizeX - 1))
              {
                (*map)->map[i * stringSize + (j + nearest[jj]) * (*map)->numFeatures + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2] * w[jj * 2 + 1];
                (*map)->map[i * stringSize + (j + nearest[jj]) * (*map)->numFeatures + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2] * w[jj * 2 + 1];
              }
              if ((i + nearest[ii] >= 0) &&
                  (i + nearest[ii] <= sizeY - 1) &&
                  (j + nearest[jj] >= 0) &&
                  (j + nearest[jj] <= sizeX - 1))
              {
                (*map)->map[(i + nearest[ii]) * stringSize + (j + nearest[jj]) * (*map)->numFeatures + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 + 1];
                (*map)->map[(i + nearest[ii]) * stringSize + (j + nearest[jj]) * (*map)->numFeatures + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 + 1];
              }
            }
          }/*for(jj = 0; jj < k; jj++)*/
        }/*for(ii = 0; ii < k; ii++)*/
      }/*for(j = 1; j < sizeX - 1; j++)*/
    }/*for(i = 1; i < sizeY - 1; i++)*/

    // 释放变量
    cvReleaseImage(&dx);
    cvReleaseImage(&dy);


    free(w);
    free(nearest);

    free(r);
    free(alfa);

    return LATENT_SVM_OK;
}

/*
Feature map Normalization and Truncation

API
int normalizeAndTruncate(featureMap *map, const float alfa);
INPUT
map               - feature map
alfa              - truncation threshold
OUTPUT
map               - truncated and normalized feature map
RESULT
Error status
函数功能：特征图标准化与截断（Feature map Normalization and Truncation）
函数参数：特征图，截断阈值
函数输出：标准化与截断之后的特征图
RESULT：Error status
计算步骤：
1.分别计算每个block（除去边界）的9分特性的9个特性的平方和
2.分别计算每个block在各个方向上的9分特性的2范数
3.用各个属性（共27个）除以各个方向上的2范数，得到归一化的274个属性
*/
int normalizeAndTruncate(CvLSVMFeatureMapCaskade *map, const float alfa)
{
    int i,j, ii;
    int sizeX, sizeY, p, pos, pp, xp, pos1, pos2;
    float * partOfNorm; // norm of C(i, j)
    float * newData;
    float   valOfNorm;  //计算每个cell的前九个特征的2范数

    sizeX     = map->sizeX;
    sizeY     = map->sizeY;
    partOfNorm = (float *)malloc (sizeof(float) * (sizeX * sizeY));  //每个cell前9个特征的平方和

    p  = NUM_SECTOR;  //每个cell的bin的数目   9个bins
    xp = NUM_SECTOR * 3; //一个cell的bins特征 3个通道*9个bins
    pp = NUM_SECTOR * 12; //一个block的特征 即3*9*4,2*2个cell组成一个block

	//每个cell前9个特征的平方和valOfNorm 存放进partOfNorm
    for(i = 0; i < sizeX * sizeY; i++)
    {
        valOfNorm = 0.0f;
        pos = i * map->numFeatures;  //第i个cell的第一个特征点索引号
        for(j = 0; j < p; j++)
        {
            valOfNorm += map->map[pos + j] * map->map[pos + j];  //计算第i个cell的前9个特征的平方和
        }/*for(j = 0; j < p; j++)*/
        partOfNorm[i] = valOfNorm;
    }/*for(i = 0; i < sizeX * sizeY; i++)*/

    sizeX -= 2;  //去掉第一列和最后一列的cell
    sizeY -= 2;  //去掉一第行和最后一行的cell
    newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp)); //新的mapfeature 去掉了靠近边界cell的区域
	//normalization
	//一个cell位于不同的block 故会出现上下左右的区分计算
	//
    for(i = 1; i <= sizeY; i++)
    {
        for(j = 1; j <= sizeX; j++)
        {
			//从第11个cell开始
			//第一次是11 12 21 22cell组成的block
			//去掉了边界的cell
			//右下
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;//计算该block四个cell的9个特征的2范数
			//第一个block的第一个cell中27个特征的首地址 即11 cell的27个特征的首地址
			//第i行第j列的cell的属性的第一个值的索引值
            pos1 = (i  ) * (sizeX + 2) * xp + (j  ) * xp;
			//去掉边界后第一个block的中108个特征的首地址
			//除掉边界后的第i-1行第j-1列的block的newdata的首地址
            pos2 = (i-1) * (sizeX    ) * pp + (j-1) * pp;
			//去掉边界后cell特征前9个除以valOfNorm
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii        ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
#if 0
			for (int i = 0; i < p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i, newData[pos2 + i]);		
			}
#endif
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 4] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
#if 0
			for (int i = 0; i < 2 * p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 4, newData[pos2 + i + p * 4]);
			}
#endif

			 //右上
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p    ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
#if 0
			for (int i = 0; i < p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p, newData[pos2 + i + p]);
			}
#endif
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 6] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
#if 0
			for (int i = 0; i < 2 * p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 6, newData[pos2 + i + p * 6]);
			}
#endif

			 //左下
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 2] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
#if 0
			for (int i = 0; i < p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 2, newData[pos2 + i + p * 2]);
			}
#endif
			for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 8] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
#if 0
			for (int i = 0; i < 2 * p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 8, newData[pos2 + i + p * 8]);
			}
#endif


			 //左上
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 3 ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
#if 0
			for (int i = 0; i < p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 3, newData[pos2 + i + p * 3]);
			}
#endif
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 10] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
#if 0
			for (int i = 0; i < 2 * p; i++)
			{
				printf("newData[%d]:%f \n", pos2 + i + p * 10, newData[pos2 + i + p * 10]);
			}
#endif
        }/*for(j = 1; j <= sizeX; j++)*/
    }/*for(i = 1; i <= sizeY; i++)*/
//truncation
    for(i = 0; i < sizeX * sizeY * pp; i++)
    {
        if(newData [i] > alfa) newData [i] = alfa;
    }/*for(i = 0; i < sizeX * sizeY * pp; i++)*/
//swop data

    map->numFeatures  = pp;
    map->sizeX = sizeX;
    map->sizeY = sizeY;

    free (map->map);
    free (partOfNorm);

    map->map = newData;

    return LATENT_SVM_OK;
}
/*
Feature map reduction
In each cell we reduce dimension of the feature vector
according to original paper special procedure

API
int PCAFeatureMaps(featureMap *map)
INPUT
map               - feature map
OUTPUT
map               - feature map
RESULT
Error status
函数功能：特征图降维（Feature map reduction）
In each cell we reduce dimension of the feature vector according to original paper special procedure
函数参数：特征图
函数输出：特征图
RESULT：Error status

步骤：
1.计算每个18分属性在4个方向上的和；
2.计算每个9分属性在4个方向上的和
3.计算4个方向上18分属性的和
*/
int PCAFeatureMaps(CvLSVMFeatureMapCaskade *map)
{
    int i,j, ii, jj, k;
    int sizeX, sizeY, p,  pp, xp, yp, pos1, pos2;
    float * newData;
    float val;
    float nx, ny;

    // 初始化Hog所需要的参数
    sizeX = map->sizeX;
    sizeY = map->sizeY;
    p     = map->numFeatures;           // 3*9
    pp    = NUM_SECTOR * 3 + 4;         // 9*3+4
    yp    = 4;
    xp    = NUM_SECTOR;

    nx    = 1.0f / sqrtf((float)(xp * 2));
    ny    = 1.0f / sqrtf((float)(yp    ));

    // 新建一个map->map的指针
    newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp));

    for(i = 0; i < sizeY; i++)
    {
        for(j = 0; j < sizeX; j++)
        {
            pos1 = ((i)*sizeX + j)*p; //去掉边界后的第i行第j列的block的的第一个属性值的索引值
            pos2 = ((i)*sizeX + j)*pp; //newData关于第i行第j列的block的的第一个属性值的索引值
            k = 0;
            for(jj = 0; jj < xp * 2; jj++)//18分属性
            {
                val = 0;
                for(ii = 0; ii < yp; ii++)
                {
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];//计算每个block的18分属性在四个方向的和
                }/*for(ii = 0; ii < yp; ii++)*/
                newData[pos2 + k] = val * ny;
                k++;
            }/*for(jj = 0; jj < xp * 2; jj++)*/
            for(jj = 0; jj < xp; jj++)//9分属性
            {
                val = 0;
                for(ii = 0; ii < yp; ii++)
                {
                    val += map->map[pos1 + ii * xp + jj];
                }/*for(ii = 0; ii < yp; ii++)*/
                newData[pos2 + k] = val * ny;
                k++;
            }/*for(jj = 0; jj < xp; jj++)*/
            for(ii = 0; ii < yp; ii++)
            {
                val = 0;
                for(jj = 0; jj < 2 * xp; jj++)
                {
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];//计算每个block的18分属性在一个方向上的和，
                }/*for(jj = 0; jj < xp; jj++)*/
                newData[pos2 + k] = val * nx;
                k++;
            } /*for(ii = 0; ii < yp; ii++)*/
        }/*for(j = 0; j < sizeX; j++)*/
    }/*for(i = 0; i < sizeY; i++)*/
//swop data

    // 将计算结果，指针复制到结果输出的map上
    map->numFeatures = pp;

    free (map->map);

    map->map = newData;

    return LATENT_SVM_OK;       // return 0
}


//modified from "lsvmc_routine.cpp"
// 根据输入，转换成指针**obj，其中(*obj)->map为sizeX * sizeY  * numFeatures大小
int allocFeatureMapObject(CvLSVMFeatureMapCaskade **obj, const int sizeX,
                          const int sizeY, const int numFeatures)
{
    int i;
    (*obj) = (CvLSVMFeatureMapCaskade *)malloc(sizeof(CvLSVMFeatureMapCaskade));
    (*obj)->sizeX       = sizeX;
    (*obj)->sizeY       = sizeY;
    (*obj)->numFeatures = numFeatures;          // 27
    (*obj)->map = (float *) malloc(sizeof (float) *
                                  (sizeX * sizeY  * numFeatures));
    for(i = 0; i < sizeX * sizeY * numFeatures; i++)
    {
        (*obj)->map[i] = 0.0f;
    }
    return LATENT_SVM_OK;
}


// 释放自己定义的CvLSVMFeatureMapCaskade数据
int freeFeatureMapObject (CvLSVMFeatureMapCaskade **obj)
{
    if(*obj == NULL) return LATENT_SVM_MEM_NULL;
    free((*obj)->map);
    free(*obj);
    (*obj) = NULL;
    return LATENT_SVM_OK;
}
