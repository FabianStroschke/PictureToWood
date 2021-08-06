//
// Created by fabian on 02.08.21.
//

#include "Pattern.hpp"

Pattern::Pattern() {
    this->gridStepX = 0;
    this->gridStepY = 0;
}


Pattern::Pattern(char *path) {
    std::ifstream i(path);
    nlohmann::json j;
    i >> j;

    this->gridStepX = j["gridStepX"];
    this->gridStepY = j["gridStepY"];

    for (auto &shape: j["Shapes"]) {
        std::vector<cv::Point> points;
        for (auto &p: shape) {
            points.emplace_back(p["x"],p["y"]);
        }
        this->shapeList.emplace_back(points);

    }

    for (auto &row:j["Layout"]) {
        this->layout.emplace_back();
        for (auto &col: row) {
            this->layout.back().emplace_back(col);
        }
    }

}

void Pattern::show(int repeat) {
    int max_width = 0;
    for (auto &row:this->layout) {
        if(row.size() > max_width) max_width = row.size();
    }
    cv::Mat pattern(layout.size()*gridStepY*repeat, max_width*gridStepX*repeat ,CV_8UC3,cv::Scalar(0,0,0));

    for (int x = 0; x < max_width*repeat; ++x) {
        for (int y = 0; y < layout.size()*repeat; ++y) {
            auto shape = this->getShapeAt(x, y);
            if(not shape.size.empty()){
                cv::Mat matRoi = pattern(cv::Rect(x*gridStepX, y*gridStepY, shape.size.width,shape.size.height));
                cv::fillConvexPoly(matRoi,shape.points,cv::Scalar(std::rand()%205+50,std::rand()%205+50,std::rand()%205+50));
            }
        }
    }

    namedWindow("Pattern", cv::WINDOW_AUTOSIZE);
    imshow("Pattern", pattern);
    cv::waitKey ( 10000);//TODO replace with better solution for waiting
}

cv::Size Pattern::getGridDimension(const cv::Size& ImageDimensions) {
    int width = 0;
    int height = 0;
    for(auto &s:shapeList){
        if(width < s.size.width) width = s.size.width;
        if(height < s.size.height) height = s.size.height;
    }
    return {(ImageDimensions.width-width)/gridStepX +1,(ImageDimensions.height-height)/gridStepY+1};
}

Shape &Pattern::getShapeAt(int x, int y) {
    y = y % layout.size();
    x = x % layout[y].size();
    return shapeList[layout[y][x]];
}

cv::Point Pattern::getPointAt(int x, int y) const {
    return {x*gridStepX, y*gridStepY};
}

void Pattern::scalePattern(double x, double y) {
    gridStepX *= x;
    gridStepY *= y;

    for (auto &shape:shapeList) {
        shape.scale(x,y);
    }
}

cv::Size Pattern::getPatternSize(const cv::Size &ImageDimensions) {
    return {(ImageDimensions.width/gridStepX)*gridStepX,(ImageDimensions.height/gridStepY)*gridStepY};
}


