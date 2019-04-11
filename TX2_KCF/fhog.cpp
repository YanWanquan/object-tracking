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

/*
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
*/
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

    float * r;
    int   * alfa;

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
    sizeX = width  / k;
    sizeY = height / k;
    px    = 3 * NUM_SECTOR;     // px=3*9=27
    p     = px;
    stringSize = sizeX * p;     // stringSize = 27*sizeX
    allocFeatureMapObject(map, sizeX, sizeY, p);


    // image：输入图像.
    // dx：输出图像.
    // kernel_dx：卷积核, 单通道浮点矩阵. 如果想要应用不同的核于不同的通道，先用 cvSplit 函数分解图像到单个色彩通道上，然后单独处理。
    // cvPoint(-1, 0)：核的锚点表示一个被滤波的点在核内的位置。 锚点应该处于核内部。缺省值 (-1,-1) 表示锚点在核中心。
    // 函数 cvFilter2D 对图像进行线性滤波，支持 In-place 操作。当核运算部分超出输入图像时，函数从最近邻的图像内部象素差值得到边界外面的象素值。
    cvFilter2D(image, dx, &kernel_dx, cvPoint(-1, 0));      // 起点在(x-1,y)，按x方向滤波
    cvFilter2D(image, dy, &kernel_dy, cvPoint(0, -1));      // 起点在(x,y-1)，按y方向滤波

    // 初始化cos和sin函数
    float arg_vector;
    for(i = 0; i <= NUM_SECTOR; i++)
    {
        arg_vector    = ( (float) i ) * ( (float)(PI) / (float)(NUM_SECTOR) );
        boundary_x[i] = cosf(arg_vector);
        boundary_y[i] = sinf(arg_vector);
    }/*for(i = 0; i <= NUM_SECTOR; i++) */

    r    = (float *)malloc( sizeof(float) * (width * height));
    alfa = (int   *)malloc( sizeof(int  ) * (width * height * 2));

    for(j = 1; j < height - 1; j++)
    {
        // 每一行起点
        datadx = (float*)(dx->imageData + dx->widthStep * j);
        datady = (float*)(dy->imageData + dy->widthStep * j);

        // 遍历该行每一个元素
        for(i = 1; i < width - 1; i++)
        {
            // 第一颜色通道
            c = 0;
            x = (datadx[i * numChannels + c]);
            y = (datady[i * numChannels + c]);

            r[j * width + i] =sqrtf(x * x + y * y);

            // 使用向量大小最大的通道替代储存值
            for(ch = 1; ch < numChannels; ch++)
            {
                tx = (datadx[i * numChannels + ch]);
                ty = (datady[i * numChannels + ch]);
                magnitude = sqrtf(tx * tx + ty * ty);
                if(magnitude > r[j * width + i])
                {
                    r[j * width + i] = magnitude;
                    c = ch;
                    x = tx;
                    y = ty;
                }
            }/*for(ch = 1; ch < numChannels; ch++)*/

            // 使用sqrt（cos*x*cos*x+sin*y*sin*y）最大的替换掉
            max  = boundary_x[0] * x + boundary_y[0] * y;   // max = 1*x+0*y;
            maxi = 0;
            for (kk = 0; kk < NUM_SECTOR; kk++)
            {
                dotProd = boundary_x[kk] * x + boundary_y[kk] * y;
                if (dotProd > max)
                {
                    max  = dotProd;
                    maxi = kk;
                }
                else
                {
                    if (-dotProd > max)
                    {
                        max  = -dotProd;
                        maxi = kk + NUM_SECTOR;             // 周期的，所以+一个周期NUM_SECTOR
                    }
                }
            }
            // 看起来有点像储存cos和sin的周期值
            alfa[j * width * 2 + i * 2    ] = maxi % NUM_SECTOR;
            alfa[j * width * 2 + i * 2 + 1] = maxi;
        }/*for(i = 0; i < width; i++)*/
    }/*for(j = 0; j < height; j++)*/

    nearest = (int  *)malloc(sizeof(int  ) *  k);
    w       = (float*)malloc(sizeof(float) * (k * 2));

    // nearest=[-1,-1,1,1];
    for(i = 0; i < k / 2; i++)
    {
        nearest[i] = -1;
    }/*for(i = 0; i < k / 2; i++)*/
    for(i = k / 2; i < k; i++)
    {
        nearest[i] = 1;
    }/*for(i = k / 2; i < k; i++)*/

    // 这算的都是啥？我怎么没在算法上看见这一段？？？
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

    // 计算梯度的公式好像和算法不太一样，应该是经过了某种离奇的推倒
    for(i = 0; i < sizeY; i++)
    {
      for(j = 0; j < sizeX; j++)
      {
        for(ii = 0; ii < k; ii++)
        {
          for(jj = 0; jj < k; jj++)
          {
            if ((i * k + ii > 0) &&
                (i * k + ii < height - 1) &&
                (j * k + jj > 0) &&
                (j * k + jj < width  - 1))
            {
              d = (k * i + ii) * width + (j * k + jj);
              (*map)->map[ i * stringSize + j * (*map)->numFeatures + alfa[d * 2    ]] +=
                  r[d] * w[ii * 2] * w[jj * 2];
              (*map)->map[ i * stringSize + j * (*map)->numFeatures + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2] * w[jj * 2];
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
// Feature map Normalization and Truncation
//
// API
// int normalizeAndTruncate(featureMap *map, const float alfa);
// INPUT
// map               - feature map
// alfa              - truncation threshold
// OUTPUT
// map               - truncated and normalized feature map
// RESULT
// Error status
*/
//
int normalizeAndTruncate(CvLSVMFeatureMapCaskade *map, const float alfa)
{
    int i,j, ii;
    int sizeX, sizeY, p, pos, pp, xp, pos1, pos2;
    float * partOfNorm; // norm of C(i, j)
    float * newData;
    float   valOfNorm;

    sizeX     = map->sizeX;
    sizeY     = map->sizeY;
    partOfNorm = (float *)malloc (sizeof(float) * (sizeX * sizeY));

    p  = NUM_SECTOR;
    xp = NUM_SECTOR * 3;
    pp = NUM_SECTOR * 12;

    for(i = 0; i < sizeX * sizeY; i++)
    {
        valOfNorm = 0.0f;
        pos = i * map->numFeatures;
        for(j = 0; j < p; j++)
        {
            valOfNorm += map->map[pos + j] * map->map[pos + j];
        }/*for(j = 0; j < p; j++)*/
        partOfNorm[i] = valOfNorm;
    }/*for(i = 0; i < sizeX * sizeY; i++)*/

    sizeX -= 2;
    sizeY -= 2;

    newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp));
//normalization
    for(i = 1; i <= sizeY; i++)
    {
        for(j = 1; j <= sizeX; j++)
        {
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;
            pos1 = (i  ) * (sizeX + 2) * xp + (j  ) * xp;
            pos2 = (i-1) * (sizeX    ) * pp + (j-1) * pp;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii        ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 4] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p    ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 6] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 2] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 8] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 3 ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 10] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
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
// Feature map reduction
// In each cell we reduce dimension of the feature vector
// according to original paper special procedure
//
// API
// int PCAFeatureMaps(featureMap *map)
// INPUT
// map               - feature map
// OUTPUT
// map               - feature map
// RESULT
// Error status
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
            pos1 = ((i)*sizeX + j)*p;
            pos2 = ((i)*sizeX + j)*pp;
            k = 0;
            for(jj = 0; jj < xp * 2; jj++)
            {
                val = 0;
                for(ii = 0; ii < yp; ii++)
                {
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];
                }/*for(ii = 0; ii < yp; ii++)*/
                newData[pos2 + k] = val * ny;
                k++;
            }/*for(jj = 0; jj < xp * 2; jj++)*/
            for(jj = 0; jj < xp; jj++)
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
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];
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
