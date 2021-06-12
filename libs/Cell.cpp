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
    rot = 0;
}

cell::cell(picture *source, const cv::Mat *shape, int x, int y) : y(y), x(x), shape(shape), source(source) {
    this->width = shape->cols;
    this->height = shape->rows;
    rot = 0;
}


void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    auto crop = this->source->images[0].img(cv::Rect(x,y,width,height));
    imshow("Cell", crop);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}

void cell::moveTo(int x_,int y_) {
    this->x = x_;
    this->y = y_;
}

bool cell::claimCell() {
    if( not this->isContinuous()) return false;

    cv::Rect rec(x,y,width,height);
    data = source->images[rot].img(rec);
    cv::Mat cut(source->images[rot].mask.rows,source->images[rot].mask.cols, CV_8U,cv::Scalar(0));
    cv::Mat matDst = cut(rec);
    shape->copyTo(matDst);

    auto &src = source->images[0].mask;
    cv::Mat rotatedMask;
    double angle = -(rot*360.0/source->images.size());

    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f((cut.cols-1)/2.0, (cut.rows-1)/2.0), angle, 1.0);
    rotMat.at<double>(0,2) += src.cols/2.0 - cut.cols/2.0;
    rotMat.at<double>(1,2) += src.rows/2.0 - cut.rows/2.0;

    cv::warpAffine(cut, rotatedMask, rotMat, src.size());
    src -= rotatedMask*255;
    source->updateMasks();
    return true;
}

bool cell::isContinuous() const {
    for(int i = 0; i<this->height; i++) {
        uchar *ptr = this->source->images[this->rot].mask.ptr(this->y + i, this->x);
        for (int j = 0; j < this->width; j++, ptr++) {
            if (*ptr == 0) return false;
        }
    }
    return true;
}


