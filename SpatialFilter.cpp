#include "SpatialFilter.h"

//prikaz funkcij piramid: http://opencv-python-tutroals.readthedocs.io/en/latest/py_tutorials/py_imgproc/py_pyramids/py_pyramids.html
//naredimo lapacovo piramido od slike
//imamo sliko, levelov piramide in piramido
bool buildLaplacianPyramid(const cv::Mat &img, const int levels,std::vector<cv::Mat> &pyramid)
{
    //levelov more bit saj 1
    if (levels < 1){
        perror("Levelov mora biti saj 1");
        return false;
    }
    pyramid.clear();//pobrisemo
    cv::Mat currentImg = img;
    for (int l=0; l<levels; l++){//za vsako plast
        cv::Mat down,up;
        //naredimo po gaussu naslednjo plast
        pyrDown(currentImg, down);//current umage, destination image (downsampling)
        //da dobimo naslednjo po gaussu, size da pase pri odstevanju
        pyrUp(down, up, currentImg.size());//current image, destination image, velikost (upsampling and blur)
        cv::Mat lap = currentImg - up;//laplace: L0=I1-I0 (I so po gaussu)
        pyramid.push_back(lap); //damo v vector
        currentImg = down; //pripravljeno za naslednjo plast
    }
    pyramid.push_back(currentImg); //L3=I3
    return true;
}

//rekonstrukcija gibanja iz piramide
void reconImgFromLaplacianPyramid(const std::vector<cv::Mat> &pyramid,const int levels,cv::Mat &dst)
{
    cv::Mat currentImg = pyramid[levels];
    for (int l=levels-1; l>=0; l--){
        cv::Mat up;
        cv::pyrUp(currentImg, up, pyramid[l].size()); //širimo
        currentImg = up + pyramid[l]; //dodajamo
    }
    dst = currentImg.clone();
}

