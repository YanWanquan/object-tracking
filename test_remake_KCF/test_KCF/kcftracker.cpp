/*

Tracker based on Kernelized Correlation Filter (KCF) [1] and Circulant Structure with Kernels (CSK) [2].
CSK is implemented by using raw gray level features, since it is a single-channel filter.
KCF is implemented by using HOG features (the default), since it extends CSK to multiple channels.

[1] J. F. Henriques, R. Caseiro, P. Martins, J. Batista,
"High-Speed Tracking with Kernelized Correlation Filters", TPAMI 2015.

[2] J. F. Henriques, R. Caseiro, P. Martins, J. Batista,
"Exploiting the Circulant Structure of Tracking-by-detection with Kernels", ECCV 2012.

Authors: Joao Faro, Christian Bailer, Joao F. Henriques
Contacts: joaopfaro@gmail.com, Christian.Bailer@dfki.de, henriques@isr.uc.pt
Institute of Systems and Robotics - University of Coimbra / Department Augmented Vision DFKI


Constructor parameters, all boolean:
    hog: use HOG features (default), otherwise use raw pixels
    fixed_window: fix window size (default), otherwise use ROI size (slower but more accurate)
    multiscale: use multi-scale tracking (default; cannot be used with fixed_window = true)

Default values are set for all properties of the tracker depending on the above choices.
Their values can be customized further before calling init():
    interp_factor: linear interpolation factor for adaptation
    sigma: gaussian kernel bandwidth
    lambda: regularization
    cell_size: HOG cell size
    padding: area surrounding the target, relative to its size
    output_sigma_factor: bandwidth of gaussian target
    template_size: template size in pixels, 0 to use ROI size
    scale_step: scale step for multi-scale estimation, 1 to disable it
    scale_weight: to downweight detection scores of other scales for added stability

For speed, the value (template_size/cell_size) should be a power of 2 or a product of small prime numbers.

Inputs to init():
   image is the initial frame.
   roi is a cv::Rect with the target positions in the initial frame

Inputs to update():
   image is the current frame.

Outputs of update():
   cv::Rect with target positions for the current frame


By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
 */


#include "kcftracker.hpp"
#include "ffttools.hpp"
#include "recttools.hpp"


#include "hog.h"




// Constructor
KCFTracker::KCFTracker(bool hog, bool fixed_window, bool multiscale, bool lab)
{
    // Parameters equal in all cases
    lambda = 0.0001;
    padding = 2.5; 
    //output_sigma_factor = 0.1;
    output_sigma_factor = 0.125;
	interp_factor = 0.012;
	sigma = 0.6;
}

// Initialize tracker 
void KCFTracker::init(const cv::Rect &roi, cv::Mat image)
{
    _roi = roi;
    assert(roi.width >= 0 && roi.height >= 0);
    _tmpl = getFeatures(image, 1);
    _prob = createGaussianPeak(size_patch[0], size_patch[1]);
    _alphaf = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
    train(_tmpl, 1.0); // train with initial frame
 }
// Update position based on the new frame
cv::Rect KCFTracker::update(cv::Mat image)
{
    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;

    float cx = _roi.x + _roi.width / 2.0f;
    float cy = _roi.y + _roi.height / 2.0f;


    float peak_value;
    cv::Point2f res = detect(_tmpl, getFeatures(image, 0, 1.0f), peak_value);

    //if (scale_step != 1) {
        // Test at a smaller _scale
    float new_peak_value;
	scale_step = 1.06;
	scale_weight = 0.95;
    cv::Point2f new_res = detect(_tmpl, getFeatures(image, 0, 1.0f / scale_step), new_peak_value);
	printf("1. peak value is %f\n", new_peak_value);
    if (scale_weight * new_peak_value > peak_value) 
	{
        res = new_res;
        peak_value = new_peak_value;
        _scale /= scale_step;
        _roi.width /= scale_step;
        _roi.height /= scale_step;
    }

        // Test at a bigger _scale
    new_res = detect(_tmpl, getFeatures(image, 0, scale_step), new_peak_value);
	printf("2. peak value is %f\n", new_peak_value);
   if (scale_weight * new_peak_value > peak_value)
   {
       res = new_res;
       peak_value = new_peak_value;
       _scale *= scale_step;
       _roi.width *= scale_step;
       _roi.height *= scale_step;
   }
    //}

    // Adjust by cell size and _scale
    _roi.x = cx - _roi.width / 2.0f + ((float) res.x * _scale);
    _roi.y = cy - _roi.height / 2.0f + ((float) res.y * _scale);

    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

    assert(_roi.width >= 0 && _roi.height >= 0);
    cv::Mat x = getFeatures(image, 0);
    train(x, interp_factor);

    return _roi;
}

cv::Rect KCFTracker::update(cv::Mat image, float& peak_value, cv::Rect_<float> bbox,int count_change_scale, float peak_value_thro)
{
	float change_scale = 0.0f;
	if (count_change_scale > 5) change_scale = 1.0;
	if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
	if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
	if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
	if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;

	float cx = _roi.x + _roi.width / 2.0f;
	float cy = _roi.y + _roi.height / 2.0f;


	float tmp_peak_value;
	cv::Point2f res;
	if (change_scale != 0)
		res = detect(_tmpl, getFeatures(image, 0, change_scale), tmp_peak_value);
	else
		res = detect(_tmpl, getFeatures(image, 0, 1.0f), tmp_peak_value);

	//if (scale_step != 1) {
	// Test at a smaller _scale
	if (change_scale != 0)
	{
		scale_step = change_scale * 1.06;
		scale_weight = change_scale * 0.95;
	}
	else
	{
		scale_step = 1.06;
		scale_weight = 0.95;
	}
	float new_peak_value;
	cv::Point2f new_res = detect(_tmpl, getFeatures(image, 0, 1.0f / scale_step), new_peak_value);
	printf("1. peak value is %f\n", new_peak_value);
	if (scale_weight * new_peak_value > tmp_peak_value)
	{
		res = new_res;
		tmp_peak_value = new_peak_value;
		_scale /= scale_step;
		_roi.width /= scale_step;
		_roi.height /= scale_step;
	}

	// Test at a bigger _scale
	new_res = detect(_tmpl, getFeatures(image, 0, scale_step), new_peak_value);
	printf("2. peak value is %f\n", new_peak_value);
	if (scale_weight * new_peak_value > tmp_peak_value)
	{
		res = new_res;
		tmp_peak_value = new_peak_value;
		_scale *= scale_step;
		_roi.width *= scale_step;
		_roi.height *= scale_step;
	}
	//}
	peak_value = tmp_peak_value;
	if (peak_value < peak_value_thro)
	{
		_roi = bbox;
		if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
		if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
		if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
		if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;
		assert(_roi.width >= 0 && _roi.height >= 0);
	}
	else
	{
		// Adjust by cell size and _scale
		_roi.x = cx - _roi.width / 2.0f + ((float)res.x * _scale);
		_roi.y = cy - _roi.height / 2.0f + ((float)res.y * _scale);

		if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
		if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
		if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
		if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

		assert(_roi.width >= 0 && _roi.height >= 0);
		cv::Mat x = getFeatures(image, 0);
		train(x, interp_factor);
	}
	//_roi.x = cx - _roi.width / 2.0f + ((float)res.x * _scale);
	//_roi.y = cy - _roi.height / 2.0f + ((float)res.y * _scale);

	//if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
	//if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
	//if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
	//if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

	//assert(_roi.width >= 0 && _roi.height >= 0);
	//cv::Mat x = getFeatures(image, 0);
	//train(x, interp_factor);

	return _roi;
}


// Detect object in the current frame.
cv::Point2f KCFTracker::detect(cv::Mat z, cv::Mat x, float &peak_value)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, z);
    cv::Mat res = (real(fftd(complexMultiplication(_alphaf, fftd(k)), true)));

    //minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
    cv::Point2i pi;
    double pv;
    cv::minMaxLoc(res, NULL, &pv, NULL, &pi);
    peak_value = (float) pv;

    //subpixel peak estimation, coordinates will be non-integer
    cv::Point2f p((float)pi.x, (float)pi.y);

    if (pi.x > 0 && pi.x < res.cols-1) {
        p.x += subPixelPeak(res.at<float>(pi.y, pi.x-1), peak_value, res.at<float>(pi.y, pi.x+1));
    }

    if (pi.y > 0 && pi.y < res.rows-1) {
        p.y += subPixelPeak(res.at<float>(pi.y-1, pi.x), peak_value, res.at<float>(pi.y+1, pi.x));
    }

    p.x -= (res.cols) / 2;
    p.y -= (res.rows) / 2;

    return p;
}

// train tracker with a single image
void KCFTracker::train(cv::Mat x, float train_interp_factor)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, x);
    cv::Mat alphaf = complexDivision(_prob, (fftd(k) + lambda));
    
    _tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x;
    _alphaf = (1 - train_interp_factor) * _alphaf + (train_interp_factor) * alphaf;


    /*cv::Mat kf = fftd(gaussianCorrelation(x, x));
    cv::Mat num = complexMultiplication(kf, _prob);
    cv::Mat den = complexMultiplication(kf, kf + lambda);
    
    _tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x;
    _num = (1 - train_interp_factor) * _num + (train_interp_factor) * num;
    _den = (1 - train_interp_factor) * _den + (train_interp_factor) * den;

    _alphaf = complexDivision(_num, _den);*/

}

// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
cv::Mat KCFTracker::gaussianCorrelation(cv::Mat x1, cv::Mat x2)
{
    using namespace FFTTools;
    cv::Mat c = cv::Mat( cv::Size(size_patch[1], size_patch[0]), CV_32F, cv::Scalar(0) );
    // HOG features
    //if (_hogfeatures) {
        cv::Mat caux;
        cv::Mat x1aux;
        cv::Mat x2aux;
		int t_t = x1.type();
        for (int i = 0; i < size_patch[2]; i++) {
            x1aux = x1.row(i);   // Procedure do deal with cv::Mat multichannel bug
			int t = x1aux.type();
            x1aux = x1aux.reshape(1, size_patch[0]);
            x2aux = x2.row(i).reshape(1, size_patch[0]);
			cv::Mat f1 = fftd(x1aux);
			cv::Mat f2 = fftd(x2aux);
            cv::mulSpectrums(f1, f2, caux, 0, true); 
            caux = fftd(caux, true);
            //caux.convertTo(caux,CV_32F);
			cv::Mat r0 = real(caux);
            c = c + r0;
        }
		rearrange(c);
   // }
    // Gray features
    //else {
    //    cv::mulSpectrums(fftd(x1), fftd(x2), c, 0, true);
    //    c = fftd(c, true);
    //    rearrange(c);
    //    c = real(c);
    //}
    cv::Mat d; 
	float v1 = cv::sum(x1.mul(x1))[0];
	float v2 = cv::sum(x2.mul(x2))[0];
    cv::max(( (v1 + v2)- 2. * c) / (size_patch[0]*size_patch[1]*size_patch[2]) , 0, d);

    cv::Mat k;
    cv::exp((-d / (sigma * sigma)), k);
    return k;
}

// Create Gaussian Peak. Function called only in the first frame.
cv::Mat KCFTracker::createGaussianPeak(int sizey, int sizex)
{
    cv::Mat_<float> res(sizey, sizex);

    int syh = (sizey) / 2;
    int sxh = (sizex) / 2;

    float output_sigma = std::sqrt((float) sizex * sizey) / padding * output_sigma_factor;
	//float output_sigma = 4.0;
    float mult = -0.5 / (output_sigma * output_sigma);

    for (int i = 0; i < sizey; i++)
        for (int j = 0; j < sizex; j++)
        {
            int ih = i - syh;
            int jh = j - sxh;
            res(i, j) = std::exp(mult * (float) (ih * ih + jh * jh));
        }
    return FFTTools::fftd(res);
}

// Obtain sub-window from image, with replication-padding and extract features
cv::Mat KCFTracker::getFeatures(const cv::Mat & image, bool inithann, float scale_adjust)
{
    cv::Rect extracted_roi;

    float cx = _roi.x + _roi.width / 2;
    float cy = _roi.y + _roi.height / 2;

	_tmpl_sz.width = 32;// 136;
	_tmpl_sz.height = 32;// 136;
	int s_w = _roi.width*3;
	int s_h = _roi.height*3;
	int ss = s_w > s_h ? s_w : s_h;
	extracted_roi.width  = scale_adjust*ss;
	extracted_roi.height = scale_adjust*ss;

	_scale = extracted_roi.width/ _tmpl_sz.width;

    // center roi with new size
    extracted_roi.x = cx - extracted_roi.width / 2;
    extracted_roi.y = cy - extracted_roi.height / 2;

    cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);
	cv::imshow("z", z);

    cv::resize(z, z, _tmpl_sz);
    
		//cv::Mat z_ipl = z;
	size_patch[0] = 32;
	size_patch[1] = 32;
	size_patch[2] = 16;
	cv::Mat FeaturesMapU8(16, 32 * 32, CV_8U);
	unsigned char* pdata = (unsigned char*)FeaturesMapU8.ptr<unsigned char>(0);
	histograms_of_gradient_directions_sse(z.data, pdata, &pdata[32 * 32], &pdata[32 * 32 * 2], &pdata[1024 * 3], &pdata[1024 * 4],
		&pdata[1024 * 5], &pdata[1024 * 6], &pdata[1024 * 7], &pdata[1024 * 8],
		&pdata[1024 * 9], &pdata[1024 * 10], &pdata[1024 * 11], &pdata[1024 * 12],
		&pdata[1024 * 13], &pdata[1024 * 14], &pdata[1024 * 15], 32, 32, 5);

	cv::Mat FeaturesMap;
	FeaturesMapU8.convertTo(FeaturesMap, CV_32F, 1.0/128);
    if (inithann) 
	{
        createHanningMats();
    }
    FeaturesMap = hann.mul(FeaturesMap);
    return FeaturesMap;
}
    
// Initialize Hanning window. Function called only in the first frame.
void KCFTracker::createHanningMats()
{   
    cv::Mat hann1t = cv::Mat(cv::Size(size_patch[1],1), CV_32F, cv::Scalar(0));
    cv::Mat hann2t = cv::Mat(cv::Size(1,size_patch[0]), CV_32F, cv::Scalar(0)); 

    for (int i = 0; i < hann1t.cols; i++)
        hann1t.at<float > (0, i) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann1t.cols - 1)));
    for (int i = 0; i < hann2t.rows; i++)
        hann2t.at<float > (i, 0) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann2t.rows - 1)));

    cv::Mat hann2d = hann2t * hann1t;
    // HOG features
    //if (_hogfeatures) {
        cv::Mat hann1d = hann2d.reshape(1,1); // Procedure do deal with cv::Mat multichannel bug
        
        hann = cv::Mat(cv::Size(size_patch[0]*size_patch[1], size_patch[2]), CV_32F, cv::Scalar(0));
        for (int i = 0; i < size_patch[2]; i++) {
            for (int j = 0; j<size_patch[0]*size_patch[1]; j++) {
                hann.at<float>(i,j) = hann1d.at<float>(0,j);
            }
        }
    //}
    // Gray features
    //else {
    //    hann = hann2d;
   // }
}

// Calculate sub-pixel peak for one dimension
float KCFTracker::subPixelPeak(float left, float center, float right)
{   
    float divisor = 2 * center - right - left;

    if (divisor == 0)
        return 0;
    
    return 0.5 * (right - left) / divisor;
}
