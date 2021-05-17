//
// Created by fabian on 17.05.21.
//

#ifndef PICTURETOWOOD_PICTURE_HPP
#define PICTURETOWOOD_PICTURE_HPP

#include <iostream>
#include "opencv2/highgui.hpp"

class Picture {
    public:
        cv::Mat img_normal;
        cv::Mat img_gray;
        explicit Picture(char *path);
        void show() const;
        void loadImg(char *path);
};


#endif //PICTURETOWOOD_PICTURE_HPP
