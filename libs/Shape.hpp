//
// Created by fabian on 02.08.21.
//

#ifndef PICTURETOWOOD_SHAPE_HPP
#define PICTURETOWOOD_SHAPE_HPP

#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>


class Shape {
public:
    cv::Size size = cv::Size(0,0);
    std::vector<cv::Point> points;
    cv::Mat mask;

    explicit Shape(std::vector<cv::Point> pointList);
    void scale(double x, double y);
    void generateMask();
};


#endif //PICTURETOWOOD_SHAPE_HPP
