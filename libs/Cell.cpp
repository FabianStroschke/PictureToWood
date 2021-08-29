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

cell::cell(picture *source, const Shape *shape, int x, int y, int stepX, int stepY) : stepHeight(stepY), stepWidth(stepX), y(y), x(x), shape(shape), source(source) {
    this->width = shape->size.width;
    this->height = shape->size.height;
    rot = 0;
}


/**
 * Displays the Image data the cell is linked to
 */
void cell::show() {
    namedWindow("Cell", cv::WINDOW_AUTOSIZE);
    auto tmp = this->source->images[0].img(cv::Rect(x,y,width,height));
    cv::Mat crop;
    tmp.copyTo(crop, shape->mask);
    imshow("Cell", crop);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}

/**
 * Changes the coordinates of the cell on the source image.
 * @param x_ new x coordinate of the cell
 * @param y_ new y coordinate of the cell
 */
void cell::moveTo(int x_,int y_) {
    this->x = x_;
    this->y = y_;
}

/**
 * Checks if the cell references continuous data and if so copies it and marks it as uses in the corresponding Image.
 * @param cutWidth Defines how many pixel outside of the copied data should be marked as used.
 * @returns true - if the data was copied successful.
 * @returnn false - if the data referenced by the cell isn't continuous.
 */
bool cell::claimCell(int cutWidth) {
    if( not this->isContinuous()) return false;

    //copy data to cell memory
    cv::Rect rec(x,y,width,height);
    auto tmp = source->images[rot].img(rec);
    tmp.copyTo(data, shape->mask);

    //calculate mask rotation
    auto &src = source->images[0].mask;
    double angle = -(rot*360.0/source->images.size());

    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f((source->images[rot].mask.cols-1)/2.0, (source->images[rot].mask.rows-1)/2.0), angle, 1.0);
    rotMat.at<double>(0,2) += src.cols/2.0 - source->images[rot].mask.cols/2.0;
    rotMat.at<double>(1,2) += src.rows/2.0 - source->images[rot].mask.rows/2.0;

    std::vector<cv::Point> notTransformedPoints;
    std::vector<cv::Point> transformedPoints;
    for (auto &p: this->shape->points) {
        notTransformedPoints.emplace_back(p.x+this->x, p.y+this->y);
    }
    cv::transform(notTransformedPoints,transformedPoints,rotMat);

    //apply mask and update mask set
    cv::polylines(src,transformedPoints,true,0,cutWidth+2);
    cv::fillConvexPoly(src,transformedPoints,0);

    source->updateMasks();
    return true;
}

/**
 * Check if data referenced by the cell contains pixel that are already used.
 * @returns true - Data is complete.
 * @returnn false - Data contains missing pixel.
 */
bool cell::isContinuous() const {
    for(int i = 0; i<this->height; i++) {
        uchar *ptr = this->source->images[this->rot].mask.ptr(this->y + i, this->x);
        const uchar *ptrShape = this->shape->mask.ptr(i, 0);
        for (int j = 0; j < this->width; j++, ptr++, ptrShape++) {
            if (*ptr == 0 && *ptrShape != 0) return false;
        }
    }
    return true;
}


