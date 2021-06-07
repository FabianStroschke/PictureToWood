//
// Created by fabian on 07.06.21.
//

#ifndef PICTURETOWOOD_PATCH_LIST_HPP
#define PICTURETOWOOD_PATCH_LIST_HPP


#include "Cell.hpp"

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

enum style{
    RECTANGLE,
    GRID

};

class patch_list {
public:
    std::vector<std::vector<cell>> patches;
    picture &pic;
    cv::Mat shape;

    patch_list(picture &p, int x, int y, style style=RECTANGLE, alignment align=CENTER);
    patch_list(picture &p, cv::Mat &shape, alignment align=CENTER);
    void save_patches(const std::string& path);

private:
    void cutIntoShape(alignment align);
    double croppingLoss(int width, int height) const;
    void croppingAdjust(int &width, int &height, bool keepRatio = true);
};


#endif //PICTURETOWOOD_PATCH_LIST_HPP
