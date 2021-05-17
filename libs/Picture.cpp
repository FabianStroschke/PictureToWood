//
// Created by fabian on 17.05.21.
//

#include "Picture.hpp"
Picture::Picture(char *path) {
    this->loadImg(path);
}

void Picture::show() const {
    if(not (img_normal.empty() || img_gray.empty())){
        namedWindow("Image", cv::WINDOW_AUTOSIZE);
        namedWindow("Image Gray", cv::WINDOW_AUTOSIZE);
        imshow( "Image", this->img_normal);
        imshow( "Image Gray", this->img_gray);
        cv::waitKey ( 10000);//TODO replace with better solution for waiting
    } else{
        std::cout << "No image to show.";
    }
}
void Picture::loadImg(char *path) {
    try {
        this->img_normal = imread(cv::samples::findFile(path), cv::IMREAD_COLOR);
        this->img_gray = imread(cv::samples::findFile(path), cv::IMREAD_GRAYSCALE);
    }catch (const std::exception& e) {
        std::cout << "Could not read image because of:\n" << e.what();
    }
}