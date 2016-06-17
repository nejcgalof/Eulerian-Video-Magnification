#ifndef SPATIALFILTER_H
#define SPATIALFILTER_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

//laplacova piramida
bool buildLaplacianPyramid(const cv::Mat &img, const int levels, std::vector<cv::Mat> &pyramid);

//rekonstrukcija slike iz laplacove piramide
void reconImgFromLaplacianPyramid(const std::vector<cv::Mat> &pyramid, const int levels, cv::Mat &dst);

#endif // SPATIALFILTER_H
