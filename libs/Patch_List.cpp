//
// Created by fabian on 07.06.21.
//

#include "Patch_List.hpp"

patch_list::patch_list(picture &p, int x, int y, style style, alignment align) : pic(p){
    int w = 0;
    int h = 0;
    switch(style){
        case RECTANGLE:
            w = x;
            h = y;
            break;
        case GRID:
            w = pic.img.cols/x;
            h = pic.img.rows/y;
            break;
    }
    this->shape = cv::Mat(cv::Size(w, h), CV_8U, 1);
    cutIntoShape(align);
}

patch_list::patch_list(picture &p, cv::Mat &shape, alignment align) : pic(p){
    this->shape = shape.clone();
    cutIntoShape(align);
}


/**
 * Cut the picture into patches defined by shape.
 * @param align Controls where the rectangle containing the patches will start. If the image needs to be cropped, this will control where the cropping takes place. E.g TOP_LEFT meaning the Bottom and right boarder will be discarded.
 */
void patch_list::cutIntoShape(alignment align) {
    int offsetX = 0;
    int offsetY = 0;
    int width = shape.cols;
    int height = shape.rows;
    auto &img = pic.img;

    //calculate offset of patches
    switch(align){
        case TOP_LEFT:
            offsetX = 0;
            offsetY = 0;
            break;
        case TOP:
            offsetX = (img.cols % width)/2;
            offsetY = 0;
            break;
        case TOP_RIGHT:
            offsetX = (img.cols % width);
            offsetY = 0;
            break;
        case LEFT:
            offsetX = 0;
            offsetY = (img.rows % height)/2;
            break;
        case CENTER:
            offsetX = (img.cols % width)/2;
            offsetY = (img.rows % height)/2;
            break;
        case RIGHT:
            offsetX = (img.cols % width);
            offsetY = (img.rows % height)/2;
            break;
        case BOTTOM_LEFT:
            offsetX = 0;
            offsetY = (img.rows % height);
            break;
        case BOTTOM:
            offsetX = (img.cols % width)/2;
            offsetY = (img.rows % height);
            break;
        case BOTTOM_RIGHT:
            offsetX = (img.cols % width);
            offsetY = (img.rows % height);
            break;
    }

    std::cout << this->croppingLoss(width,height)*100 << "% of the picture will be lost due to cropping!\n";
    std::cout << "cols:"<< img.cols / width << "\n";
    std::cout << "rows:"<< img.rows / height << "\n";
    //this->croppingAdjust(width,height);
    this->patches = std::vector<std::vector<cell>>(img.cols / width);

    //go from top left to bottom right
    for(int x = 0; offsetX + x*width +width<= img.cols; x++){
        this->patches[x].reserve(img.rows / height);
        for(int y = 0; offsetY + y*height + height <= img.rows; y++){

            //create patch and put it into list
            this->patches[x].emplace_back(pic, shape,offsetX + x * width, offsetY + y * height);
            //this->patches.back().show();
        }
    }

}


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
    output_path += "/" + this->pic.name;
    if(boost::filesystem::exists(output_path)){
        boost::filesystem::remove_all(output_path);
        boost::filesystem::create_directory(output_path);
    }else{
        boost::filesystem::create_directory(output_path);
    }

    //writes the images
    for(int x = 0; x < patches.size(); x++) {
        for (int y = 0; y < patches[x].size(); y++) {
            auto &p = patches[x][y];
            auto rec = cv::Rect(p.x,p.y,p.width,p.height);
            imwrite(output_path + "/" + "n" + std::to_string(x) + "," + std::to_string(y) + ".tiff", this->pic.img(rec));
            //imwrite(output_path + "/" + "g" + std::to_string(x) + "," + std::to_string(y) + ".tiff", patches[x][y].img_gray);
        }
    }
}

/**
 * Calculates the percentage of image area that will be lost, if the image is cut into patches of size width x height.
 * @param width Width of the patches in px.
 * @param height Height of the patches in px.
 * @return Percentage of image area that would be lost, using these dimensions in range from 0-1.
 */
double patch_list::croppingLoss(int width, int height) const{
    int newWidth;
    int newHeight;

    newHeight = pic.img.rows - pic.img.rows % height;
    newWidth = pic.img.cols - pic.img.cols % width;

    return 1.0 - (double(newWidth*newHeight) / double(pic.img.cols*pic.img.rows));
}

/**
 * Adjusts the parameters to minimize the loss due to cropping.
 * @param width Width of the patches in px. Will be changed if a better value is found.
 * @param height Height of the patches in px. Will be changed if a better value is found.
 * @param keepRatio Specifics if the ratio between width and height should be kept the same or not.
 * @return Writes return values into width and height.
 */
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

