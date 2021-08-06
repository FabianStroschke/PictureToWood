//
// Created by fabian on 17.05.21.
//

#ifndef PICTURETOWOOD_PICTURE_HPP
#define PICTURETOWOOD_PICTURE_HPP

#include <iostream>
#include "opencv2/highgui.hpp"
#include <boost/filesystem.hpp>
#include <opencv2/imgproc.hpp>


struct image_set{
    cv::Mat img;
    cv::Mat img_gray;
    cv::Mat img_filter;
    cv::Mat mask;
};

class picture {
    public:
        std::string name;
        std::vector<struct image_set> images;
        unsigned int origDPI;
        struct image_set origImage;

        picture(const std::string &path, unsigned int dpi, int filter_type, double filter_ratio);
        void show() const;
        void addRotations(int n);
        void updateMasks();
        void scaleTo(unsigned int dpi);
        void transformHistTo(cv::Mat targetHist, int channel);

        unsigned int currentDPI;
        unsigned int filterType;
        double filter_ratio;
        void updateImageSet();
};

cv::Mat cumulativeHist(std::vector<picture> &picList, int channel);


#endif //PICTURETOWOOD_PICTURE_HPP
