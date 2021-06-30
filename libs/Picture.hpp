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
        cv::Mat origImage;

        explicit picture(const std::string &path, unsigned int dpi);
        void show() const;
        void loadImg(const std::string &path, int filter_type, unsigned int dpi);
        void addRotations(int n);
        void updateMasks();
        void scaleTo(unsigned int dpi);

    private:
        unsigned int currentDPI;
        unsigned int filterType;

    void updateImageSet();
};


#endif //PICTURETOWOOD_PICTURE_HPP
