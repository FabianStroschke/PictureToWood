//
// Created by fabian on 02.08.21.
//

#ifndef PICTURETOWOOD_PATTERN_HPP
#define PICTURETOWOOD_PATTERN_HPP

#include <vector>
#include <fstream>
#include "Shape.hpp"
#include "json.hpp"

class Pattern {
public:
    int gridStepX;
    int gridStepY;
    std::vector<Shape> shapeList;
    std::vector<std::vector<int>> layout;

    Pattern();
    explicit Pattern(const std::string &path);

    void show(int repeat = 1);
    cv::Size getGridDimension(const cv::Size& ImageDimensions);
    cv::Size getPatternSize(const cv::Size& ImageDimensions);
    Shape &getShapeAt(int x, int y);
    cv::Point getPointAt(int x, int y) const;
    void scalePattern(double x, double y);
    void convertToCm(double DPI);

private:
    double width_in_cm = 0;
    bool scale_to_cm = false;
};


#endif //PICTURETOWOOD_PATTERN_HPP
