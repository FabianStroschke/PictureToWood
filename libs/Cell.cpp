//
// Created by fabian on 23.05.21.
//

#include "Cell.hpp"

cell::cell(Picture &source, const cv::Mat &shape, int x, int y) : shape(shape), source(source) {
    this->x = x;
    this->y = y;
    this->width = shape.cols;
    this->height = shape.rows;
}


void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    namedWindow("Cell_gray", cv::WINDOW_AUTOSIZE);
    auto crop = this->source.img(cv::Rect(x,y,width,height));
    imshow("Cell", crop);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}

