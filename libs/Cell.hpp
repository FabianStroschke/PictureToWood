//
// Created by fabian on 23.05.21.
//

#ifndef PICTURETOWOOD_CELL_HPP
#define PICTURETOWOOD_CELL_HPP


#include "opencv2/highgui.hpp"
#include "Picture.hpp"
#include <opencv2/imgproc.hpp>

class cell {
    public:
        const cv::Mat &shape;
        Picture &source;
        int x;
        int y;
        int width;
        int height;

        cell(Picture &source, const cv::Mat &shape, int x = 0, int y = 0);
        void show();
};


#endif //PICTURETOWOOD_CELL_HPP
