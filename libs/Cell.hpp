//
// Created by fabian on 23.05.21.
//

#ifndef PICTURETOWOOD_CELL_HPP
#define PICTURETOWOOD_CELL_HPP


#include "opencv2/highgui.hpp"
#include "Picture.hpp"
#include "Shape.hpp"
#include <opencv2/imgproc.hpp>

class cell {
    public:
        picture *source;
        const Shape *shape;
        cv::Mat data;
        int x;
        int y;
        int width;
        int height;
        int stepWidth;
        int stepHeight;
        int rot;

        cell();
        cell(picture *source, const Shape *shape, int x = 0, int y = 0, int stepX = -1, int stepY = -1);
        void show();
        void moveTo(int x_,int y_);
        bool claimCell(int cutWidth);
        bool isContinuous() const;

};


#endif //PICTURETOWOOD_CELL_HPP
