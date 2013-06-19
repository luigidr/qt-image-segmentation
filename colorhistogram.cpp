#include "colorhistogram.h"

ColorHistogram::ColorHistogram()
{
    // number of histogram lines
    histSize[0]=180;
    // HSV values range
    hranges[0]= 0.0;
    hranges[1]= 179.0;
    ranges[0] = hranges;
    channel[0]=0;
    average = 0.f;
}

float ColorHistogram::getAverage()
{
    return average;
}

MatND ColorHistogram::computeOneChannelHist(const Mat &image)
{
    MatND hist;

    calcHist(&image,
                 1, // only one image
                 channel, // used channels
                 Mat(), // no mask
                 hist, // final histogram
                 1, // one histogram in 3 dimensions
                 histSize, // number of the values present in the histogram
                 ranges // range present in the channel
        );

    return hist;

}

Mat ColorHistogram::getHueHistogram(Mat src) {

    //src: blur + HSV
    Mat temp;

    blur(src, temp, Size(3,3));
    cvtColor(temp, temp, CV_BGR2HSV);

    // I need only the Hue channel
    vector<Mat> hsv_planes;
    split(temp, hsv_planes);

    MatND hist_hue = computeOneChannelHist(hsv_planes[0]);

    // max and min value of bins
    double maxVal= 0;
    double minVal=0;
    minMaxLoc(hist_hue,&minVal,&maxVal,0,0);

    // image for drawing the histogram
    int img_h = 150;
    int img_w = histSize[0];
    Mat histImg_hue(img_h,img_w,CV_8UC3,Scalar(0,0,30));

    // max height (90% of image height)
    int hpt = static_cast<int>(0.9*img_h);

    average = 0.f;

    // draw a single line for each bin
    for (int h=0; h<img_w; h++) {
        average += (hist_hue.at<float>(h) * h);
         // normalized height (with respect to the histogram image)
        int intensity_hue = static_cast<int>(hist_hue.at<float>(h)*hpt/maxVal);
        // effectively draw the line
        line(histImg_hue,Point(h,img_h),Point(h,img_h-intensity_hue),Scalar(h,255,255));
    }

    average = average / temp.size().height / temp.size().width;

    line(histImg_hue,Point(average,img_h),Point(average,0),Scalar(0,0,255));

    cvtColor(histImg_hue, histImg_hue, CV_HSV2BGR);

    return histImg_hue;

}
