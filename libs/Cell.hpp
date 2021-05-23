//
// Created by fabian on 23.05.21.
//

#ifndef PICTURETOWOOD_CELL_HPP
#define PICTURETOWOOD_CELL_HPP


#include "opencv2/highgui.hpp"

class cell {
    public:
        cv::Mat data;
        int width;
        int height;

        cell(cv::Mat img);
        cell(const cv::Mat& img, const cv::Rect& rec);

        void show();
};


#endif //PICTURETOWOOD_CELL_HPP
