#ifndef COLORHISTOGRAM_H
#define COLORHISTOGRAM_H

#include <opencv.hpp>

using namespace cv;

class ColorHistogram
{
public:
    ColorHistogram();
    Mat getHueHistogram(Mat src);
    float getAverage();

private:
    MatND computeOneChannelHist(const Mat &image);

private:
    int histSize[1];
    float hranges[2];
    const float* ranges[1];
    int channel[1];
    float average;
};

#endif // COLORHISTOGRAM_H
