//
// Created by fabian on 07.06.21.
//

#include "Patch_List.hpp"

#include <utility>

/** LEGACY CODE
patch_list::patch_list(picture &p, int x, int y, style style, alignment align){
    int w = 0;
    int h = 0;
    switch(style){
        case RECTANGLE:
            w = x;
            h = y;
            break;
        case GRID:
            w = p.images[0].img.cols/x;
            h = p.images[0].img.rows/y;
            break;
    }
    this->shape = cv::Mat(cv::Size(w, h), CV_8U, 255);
    cutIntoShape(p, align);
}

patch_list::patch_list(picture &p, cv::Mat &shape, int stepX, int stepY, alignment align){
    this->_stepX = stepX;
    this->_stepY = stepY;
    this->shape = shape.clone();
    cutIntoShape(p, align);
}

patch_list::patch_list(picture &p, cv::Mat &shape, alignment align){
    this->shape = shape.clone();
    cutIntoShape(p, align);
}
**/

/**
 * Cut the picture into patches defined by shape.
 * @param align Controls where the rectangle containing the patches will start. If the image needs to be cropped, this will control where the cropping takes place. E.g TOP_LEFT meaning the Bottom and right boarder will be discarded.
 */
 /** LEGACY CODE
void patch_list::cutIntoShape(picture &p, alignment align) {
    int offsetX = 0;
    int offsetY = 0;
    int stepWidth = shape.cols;
    int stepHeight = shape.rows;
    auto &img = p.images[0].img;

    //calculate offset of patches
    switch(align){
        case TOP_LEFT:
            offsetX = 0;
            offsetY = 0;
            break;
        case TOP:
            offsetX = (img.cols % stepWidth) / 2;
            offsetY = 0;
            break;
        case TOP_RIGHT:
            offsetX = (img.cols % stepWidth);
            offsetY = 0;
            break;
        case LEFT:
            offsetX = 0;
            offsetY = (img.rows % stepHeight) / 2;
            break;
        case CENTER:
            offsetX = (img.cols % stepWidth) / 2;
            offsetY = (img.rows % stepHeight) / 2;
            break;
        case RIGHT:
            offsetX = (img.cols % stepWidth);
            offsetY = (img.rows % stepHeight) / 2;
            break;
        case BOTTOM_LEFT:
            offsetX = 0;
            offsetY = (img.rows % stepHeight);
            break;
        case BOTTOM:
            offsetX = (img.cols % stepWidth) / 2;
            offsetY = (img.rows % stepHeight);
            break;
        case BOTTOM_RIGHT:
            offsetX = (img.cols % stepWidth);
            offsetY = (img.rows % stepHeight);
            break;
    }


    this->patches = std::vector<std::vector<patch>>((img.cols-shape.cols) / stepWidth + 1);

    //go from top left to bottom right
    for(int x = 0; offsetX + x * stepWidth + shape.cols <= img.cols; x++){
        this->patches[x].reserve((img.rows-shape.rows) / stepHeight + 1);
        for(int y = 0; offsetY + y * stepHeight + shape.rows <= img.rows; y++){

            //create patch and put it into list
            this->patches[x].push_back({cell(&p, &shape,offsetX + x * stepWidth, offsetY + y * stepHeight, stepWidth, stepHeight), cell()});

            //this->patches.back().show();
        }
    }

}
**/

/**
* Saves the patches into the specified directory.
* @param path Directory name or relative directory path.
*/
void patch_list::save_patches(const std::string& path) {
    //checks if the output directory exist or creates
    std::string output_path = boost::filesystem::current_path().string()+ "/" + path;
    if(not boost::filesystem::exists(output_path))
        boost::filesystem::create_directory(output_path);

    //creates a sub directory for the image or clears an existing one
    output_path += "/Patches";
    if(boost::filesystem::exists(output_path)){
        boost::filesystem::remove_all(output_path);
        boost::filesystem::create_directory(output_path);
    }else{
        boost::filesystem::create_directory(output_path);
    }

    //writes the images
    for(int x = 0; x < patches.size(); x++) {
        for (int y = 0; y < patches[x].size(); y++) {
            auto &p = patches[x][y].target;
            auto rec = cv::Rect(p.x,p.y,p.width,p.height);
            imwrite(output_path + "/" + "n" + std::to_string(x) + "," + std::to_string(y) + ".tiff", p.source->images[0].img(rec));
            //imwrite(output_path + "/" + "g" + std::to_string(x) + "," + std::to_string(y) + ".tiff", patches[x][y].img_gray);
        }
    }
}

patch_list::patch_list(picture &p, Pattern &ptrn, alignment align) {
    pattern = &ptrn;
    auto &img = p.images[0].img;
    auto dims = pattern->getGridDimension(cv::Size(img.cols,img.rows));
    auto patternSize = pattern->getPatternSize(cv::Size(img.cols,img.rows));

    //calculate offset of patches
    switch(align){
        case TOP_LEFT:
            offset.x = 0;
            offset.y = 0;
            break;
        case TOP:
            offset.x = (img.cols - patternSize.width) / 2;
            offset.y = 0;
            break;
        case TOP_RIGHT:
            offset.x = (img.cols - patternSize.width);
            offset.y = 0;
            break;
        case LEFT:
            offset.x = 0;
            offset.y = (img.rows - patternSize.height) / 2;
            break;
        case CENTER:
            offset.x = (img.cols - patternSize.width) / 2;
            offset.y = (img.rows - patternSize.height) / 2;
            break;
        case RIGHT:
            offset.x = (img.cols - patternSize.width);
            offset.y = (img.rows - patternSize.height) / 2;
            break;
        case BOTTOM_LEFT:
            offset.x = 0;
            offset.y = (img.rows - patternSize.height);
            break;
        case BOTTOM:
            offset.x = (img.cols - patternSize.width) / 2;
            offset.y = (img.rows - patternSize.height);
            break;
        case BOTTOM_RIGHT:
            offset.x = (img.cols - patternSize.width);
            offset.y = (img.rows - patternSize.height);
            break;
    }

    this->patches = std::vector<std::vector<patch>>(dims.width);

    //go from top left to bottom right
    for(int x = 0; x < dims.width; x++){
        this->patches[x].reserve(dims.height);
        for(int y = 0; y < dims.height; y++){

            //create patch and put it into list
            auto pos = pattern->getPointAt(x,y);
            Shape &shapeRef = pattern->getShapeAt(x,y);
            if(not shapeRef.size.empty()
                && shapeRef.size.width+pos.x <=patternSize.width
                && shapeRef.size.height+pos.y <=patternSize.height) {
                this->patches[x].push_back({cell(&p, &(shapeRef),offset.x + pos.x, offset.y + pos.y), cell()});

            }
        }
    }

    //get size of picture cover by patches
    this->size = patternSize;
}

/**
 * Calculates the percentage of image area that will be lost, if the image is cut into patches of size width x height.
 * @param width Width of the patches in px.
 * @param height Height of the patches in px.
 * @return Percentage of image area that would be lost, using these dimensions in range from 0-1.
 */
 /*
double patch_list::croppingLoss(int width, int height) const{
    int newWidth;
    int newHeight;

    newHeight = p.img.rows - p.img.rows % height;
    newWidth = p.img.cols - p.img.cols % width;

    return 1.0 - (double(newWidth*newHeight) / double(p.img.cols*p.img.rows));
}
*/

/**
 * Adjusts the parameters to minimize the loss due to cropping.
 * @param width Width of the patches in px. Will be changed if a better value is found.
 * @param height Height of the patches in px. Will be changed if a better value is found.
 * @param keepRatio Specifics if the ratio between width and height should be kept the same or not.
 * @return Writes return values into width and height.
 */
/*
void patch_list::croppingAdjust(int &width, int &height, bool keepRatio) {
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
*/

