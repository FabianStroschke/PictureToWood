//
// Created by fabian on 23.05.21.
//

#include "Cell.hpp"

cell::cell() {
    this->width = 0;
    this->height = 0;
}

cell::cell(cv::Mat img) {
    this->img = img;
    cvtColor(img, this->img_gray, cv::COLOR_BGR2GRAY);
    this->width = img.cols;
    this->height = img.rows;
}

cell::cell(const cv::Mat& img, const cv::Rect& rec) {
    this->img = img(rec);
    cvtColor(this->img, this->img_gray, cv::COLOR_BGR2GRAY);
    this->width = this->img.cols;
    this->height = this->img.rows;
}

void cell::rot90(){
    cv::rotate( this->img, this->img, cv::ROTATE_90_CLOCKWISE);
    cv::rotate( this->img_gray, this->img_gray, cv::ROTATE_90_CLOCKWISE);

}


void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    namedWindow("Cell_gray", cv::WINDOW_AUTOSIZE);
    imshow("Cell", this->img);
    imshow("Cell_gray", this->img_gray);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}
