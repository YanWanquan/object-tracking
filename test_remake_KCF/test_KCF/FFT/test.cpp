#include "FFT2D.h"
#include <stdlib.h>

//#include "Filter2D.h"

//#include "opencv/cv.h"
//#include <opencv2/imgproc.hpp>



int main()
{
   float data[16*16];
   unsigned char indata[16*16];
   for (int j = 0; j < 16; j++)
   {
     for (int i = 0; i < 16; i++)
     {
       unsigned char t = rand()%255;
       float tt = 1.0*t;
       data[i + j*16] = tt;
       indata[i + j*16] = t;
       printf("%f ",tt);
     }
     printf("\n");
   }
#if 0
   cv::Mat inMat(16, 16, CV_32FC1, data);
   cv::Mat kernel_dy = (cv::Mat_<float>(3, 1) << -1.f, 0.f, 1.f);
   cv::Mat dy;
   cv::filter2D(inMat, dy, inMat.depth(), kernel_dy);
   float ker[3] = {-1.f, 0.f, 1.f};
   float out[16*16];
   filter2D_col(indata, 16, 16, ker, 3, out);
   float* ptr = dy.ptr<float>(0);
   for (int j = 0; j < 16; j++)
   {
	  for (int i = 0; i < 16; i++)
	  {
	      printf("(%f vs %f) ",ptr[i + j*16], out[i + j*16]);
	  }
	  printf("\n");
   }
#endif
#if 1
   FFT fft_;
   // do initial
   fft_.doInitial(16, 16);

  cp* dst = (cp*)malloc(16*16*sizeof(cp));
  // do fft
  const cp* res = fft_.fft2d(indata);
  for (int j = 0; j < 16; j++)
  {
	  for (int i = 0; i < 16; i++)
	  {
	      cp t = res[i + j*16];
	      printf("(%f+%fi) ",t.real(),t.imag());
	  }
	  printf("\n");
   }
#endif


#if 0
   cv::Mat image(16,16, CV_32FC1, (float*)data);
   cv::Mat in_image;
   cv::Mat planes[] = {cv::Mat_<float> (image), cv::Mat_<float>::zeros(image.size())};
   cv::merge(planes, 2, in_image);
   cv::dft(in_image, in_image, 0 );
   cv::split(in_image, planes);
   cv::dft(in_image, in_image, cv::DFT_INVERSE | cv::DFT_SCALE);
   cv::split(in_image, planes);
float* ptr = planes[0].ptr<float>(0);
   for (int j = 0; j < 16; j++)
   {
	  for (int i = 0; i < 16; i++)
	  {
	      printf("(%f) ",ptr[i + j*16]);
	  }
	  printf("\n");
   }
#endif

#if 1
  // do inverse fft
  const cp* inv_res = fft_.ifft2d(res);
  for (int j = 0; j < 16; j++)
  {
	  for (int i = 0; i < 16; i++)
	  {
	      printf("%f ",inv_res[j*16+i].real());
	  }
	  printf("\n");
   }
 free(dst);
#endif
}
