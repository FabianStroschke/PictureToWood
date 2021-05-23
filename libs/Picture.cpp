//
// Created by fabian on 17.05.21.
//

#include "Picture.hpp"
using namespace cv;

/**
 * Load the Image specified by path into the object
 * @param path absolute path or relative path from the working directory
 * @return creates a Picture Object containing the colored and grayscale version of the image
 */
Picture::Picture(char *path) {
    this->loadImg(path);
}

void Picture::show() const {
    if(not (img_normal.empty() || img_gray.empty())){
        namedWindow("Image", WINDOW_AUTOSIZE);
        namedWindow("Image Gray", WINDOW_AUTOSIZE);
        moveWindow("Image Gray",100,100);
        imshow( "Image", this->img_normal);
        imshow( "Image Gray", this->img_gray);
        waitKey ( 10000);//TODO replace with better solution for waiting
    } else{
        std::cout << "No image to show.";
    }
}
void Picture::loadImg(char *path) {
    try {
        String file_path = samples::findFile(path);
        this->img_normal = imread(file_path, IMREAD_COLOR);
        this->img_gray = imread(file_path, IMREAD_GRAYSCALE);
        this->name = file_path.substr(file_path.find_last_of('/')+1, file_path.find_first_of('.') - file_path.find_last_of('/')-1);
    }catch (const std::exception& e) {
        std::cout << "Could not read image because of:\n" << e.what();
    }
}

void Picture::cutIntoRect(int width, int height, alignment align) {
    Rect cell;
    cell.width = width;
    cell.height = height;
    int offsetX = 0;
    int offsetY = 0;
    switch(align){
        case TOP_LEFT:
            offsetX = 0;
            offsetY = 0;
            break;
        case TOP:
            offsetX = (img_gray.cols % width)/2;
            offsetY = 0;
            break;
        case TOP_RIGHT:
            offsetX = (img_gray.cols % width);
            offsetY = 0;
            break;
        case LEFT:
            offsetX = 0;
            offsetY = (img_gray.rows % height)/2;
            break;
        case CENTER:
            offsetX = (img_gray.cols % width)/2;
            offsetY = (img_gray.rows % height)/2;
            break;
        case RIGHT:
            offsetX = (img_gray.cols % width);
            offsetY = (img_gray.rows % height)/2;
            break;
        case BOTTOM_LEFT:
            offsetX = 0;
            offsetY = (img_gray.rows % height);
            break;
        case BOTTOM:
            offsetX = (img_gray.cols % width)/2;
            offsetY = (img_gray.rows % height);
            break;
        case BOTTOM_RIGHT:
            offsetX = (img_gray.cols % width);
            offsetY = (img_gray.rows % height);
            break;
    }

    std::cout << this->croppingLoss(width,height)*100 << "% of the picture will be lost due to cropping!\n";
    std::cout << "width = " << offsetX << "| height = " << offsetY;
    //this->croppingAdjust(width,height);
    //go from top left to bottom right
    for(int y = 0+offsetY; y + height <= img_gray.rows; y += height){
        for(int x = 0+offsetX; x + width <= img_gray.cols; x += width){
            cell.x = x;
            cell.y = y;

            this->patches_gray.emplace_back(img_gray(cell));
            this->patches_normal.emplace_back(img_normal(cell));
            //this->patches_gray.back().show();
        }
    }

}

void Picture::cutIntoSquares(int width, alignment align) {
    this->cutIntoRect(width, width,align);
}

double Picture::croppingLoss(int width, int height) const {
    int newWidth;
    int newHeight;

    newHeight = img_gray.rows - img_gray.rows % height;
    newWidth = img_gray.cols - img_gray.cols % width;

    return 1.0 - (double(newWidth*newHeight) / double(img_gray.cols*img_gray.rows));
}

void Picture::croppingAdjust(int &width, int &height, bool keepRatio) {
    if(keepRatio) {
        double ratio = (double) width / (double) height;
        double loss = this->croppingLoss(width, height);
        double minLoss = loss;
        int originalWidth = width;
        int newWidth;
        int newHeight;
        for (int x = -20; x <= 20; x++) {
            newWidth = originalWidth + x;
            newHeight = newWidth / ratio;
            loss = this->croppingLoss(newWidth, newHeight);
            if (loss < minLoss) {
                width = newWidth;
                height = newHeight;
                minLoss = loss;
            }
        }
        if (width != originalWidth) {
            std::cout << "Better size for current Ratio in a 20 Pixel range found for: width = " << width
                      << "| height = " << height << " with a loss of " << minLoss * 100 << "%";
        }
    }else{
     //TODO
    }
}


void Picture::cutIntoGrid(int cols, alignment align) {
    int width = img_gray.cols/cols;
    int height = width;
    this->croppingAdjust(width,height);
    this->cutIntoSquares(width,align);
}

void Picture::cutIntoGrid(int cols, int rows, alignment align) {
    int width = img_gray.cols/cols;
    int height = img_gray.rows/rows;
    this->croppingAdjust(width,height);
    this->cutIntoRect(width,height,align);
}

void Picture::save_patches(const std::string& path) {
    std::string output_path = boost::filesystem::current_path().string()+ "/" + path;
    if(not boost::filesystem::exists(output_path))
        boost::filesystem::create_directory(output_path);
    output_path += "/" + this->name;

    if(boost::filesystem::exists(output_path)){
        boost::filesystem::remove_all(output_path);
        boost::filesystem::create_directory(output_path);
    }else{
        boost::filesystem::create_directory(output_path);
    }
    for(int i = 0; i < patches_gray.size(); i++){
        imwrite(output_path + "/" + "n" + std::to_string(i) +".tiff", patches_normal[i].data);
        imwrite(output_path + "/" + "g" + std::to_string(i) +".tiff", patches_gray[i].data);
    }
}

