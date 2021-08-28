//
// Created by fabian on 02.08.21.
//

#include "Shape.hpp"

/**
 * Creates the shape based on the polygon provided as a point list.
 * @param pointList List of points that form a polygon.
 */
Shape::Shape(std::vector<cv::Point> pointList) {
    this->points = std::move(pointList);
    this->size = cv::Size(0,0);
    if(this->points.empty()) return;
    //get offset
    int offsetX = points[0].x;
    int offsetY = points[0].y;
    for (auto &p: points) {
        if (offsetX > p.x) offsetX = p.x;
        if (offsetY > p.y) offsetY = p.y;
    }

    //apply offset
    for (auto &p: points) {
        p.x -= offsetX;
        p.y -= offsetY;

        if(size.width < p.x) size.width = p.x;
        if(size.height < p.y) size.height = p.y;
    }
    this->generateMask();
}


/**
 * Scales the shape and corresponding mask.
 * @param x Scaling along x axis.
 * @param y Scaling along y axis.
 */
void Shape::scale(double x, double y) {
    if(points.empty())return;
    for (auto &p: points) {
        p.x *= x;
        p.y *= y;
    }
    this->size.width *=x;
    this->size.height *=y;
    this->generateMask();
}


/**
 * Generates the mask of the shape object
 */
void Shape::generateMask() {
    if(points.empty())return;
    this->mask = cv::Mat(this->size, CV_8U, cv::Scalar(0));
    cv::fillConvexPoly(mask,points,255);
}
