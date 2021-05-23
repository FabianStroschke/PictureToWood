//
// Created by fabian on 23.05.21.
//

#include "Cell.hpp"

cell::cell(cv::Mat img) {
    this->data = img;
    this->width = img.cols;
    this->height = img.rows;
}

cell::cell(const cv::Mat& img, const cv::Rect& rec) {
    this->data = img(rec);
    this->width = data.cols;
    this->height = data.rows;
}

void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    imshow("Cell", this->data);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}
