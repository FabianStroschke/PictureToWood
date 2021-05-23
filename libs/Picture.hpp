//
// Created by fabian on 17.05.21.
//

#ifndef PICTURETOWOOD_PICTURE_HPP
#define PICTURETOWOOD_PICTURE_HPP

#include <iostream>
#include "opencv2/highgui.hpp"

enum alignment{
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    LEFT,
    CENTER,
    RIGHT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT
};

class Picture {
    public:
        cv::Mat img_normal;
        cv::Mat img_gray;
        explicit Picture(char *path);
        void show() const;
        void loadImg(char *path);
        void cutIntoRect(int width, int height, alignment align=CENTER);
        void cutIntoSquares(int width, alignment align=CENTER);
        void cutIntoGrid(int cols, alignment align=CENTER);
        void cutIntoGrid(int cols, int rows, alignment align=CENTER);
    private:
        double croppingLoss(int width, int height) const;
        void croppingAdjust(int &width, int &height, bool keepRatio = true);
};


#endif //PICTURETOWOOD_PICTURE_HPP
