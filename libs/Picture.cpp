//
// Created by fabian on 17.05.21.
//

#include "Picture.hpp"
using namespace cv;

/**
 * Load the Image specified by path into the picture object.
 * @param path Absolute path or relative path from the working directory.
 * @return Creates a picture Object containing the colored and grayscale version of the image.
 */
picture::picture(char *path) {
    this->loadImg(path);
}

/**
 * Show the colored and grayscale version of the image.
 */
void picture::show() const {
    if(not img.empty()){
        namedWindow("Image", WINDOW_AUTOSIZE);
        imshow( "Image", this->img);
        waitKey ( 10000);//TODO replace with better solution for waiting
    } else{
        std::cout << "No image to show.";
    }
}

/**
 * Load the Image specified by path into the picture object, replacing the current images and replacing the name with the new file name.
 * @param path Absolute path or relative path from the working directory.
 */
void picture::loadImg(char *path) {
    try {
        String file_path = samples::findFile(path);
        this->img = imread(file_path, IMREAD_COLOR);
        this->name = file_path.substr(file_path.find_last_of('/')+1, file_path.find_first_of('.') - file_path.find_last_of('/')-1);
    }catch (const std::exception& e) {
        std::cout << "Could not read image because of:\n" << e.what();
    }
}
