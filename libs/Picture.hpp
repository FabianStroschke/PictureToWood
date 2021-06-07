//
// Created by fabian on 17.05.21.
//

#ifndef PICTURETOWOOD_PICTURE_HPP
#define PICTURETOWOOD_PICTURE_HPP

#include <iostream>
#include "opencv2/highgui.hpp"
#include <boost/filesystem.hpp>

class picture {
    public:
        std::string name;
        cv::Mat img;

        explicit picture(char *path);
        void show() const;
        void loadImg(char *path);
};


#endif //PICTURETOWOOD_PICTURE_HPP
