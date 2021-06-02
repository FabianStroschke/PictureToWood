//
// Created by fabian on 23.05.21.
//

#ifndef PICTURETOWOOD_CELL_HPP
#define PICTURETOWOOD_CELL_HPP


#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>

class cell {
    public:
        cv::Mat img;
        cv::Mat img_gray;
        cv::Mat img_filter;
        int width;
        int height;

        cell();
        cell(cv::Mat img);
        cell(const cv::Mat& img, const cv::Rect& rec);

        void show();
};


#endif //PICTURETOWOOD_CELL_HPP
