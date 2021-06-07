//
// Created by fabian on 23.05.21.
//

#include "Cell.hpp"

cell::cell() {
    source = nullptr;
    shape = nullptr;
    x = -1;
    y = -1;
    width = -1;
    height = -1;
}

cell::cell(picture *source, const cv::Mat *shape, int x, int y) : y(y), x(x), shape(shape), source(source) {
    this->width = shape->cols;
    this->height = shape->rows;
}


void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    auto crop = this->source->img(cv::Rect(x,y,width,height));
    imshow("Cell", crop);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}

void cell::moveTo(int x_,int y_) {
    this->x = x_;
    this->y = y_;
}


