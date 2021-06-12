//
// Created by fabian on 25.05.21.
//

#ifndef PICTURETOWOOD_FIND_PATCHES_HPP
#define PICTURETOWOOD_FIND_PATCHES_HPP

#include <vector>
#include <functional>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/thread.hpp>
#include <boost/asio/post.hpp>
#include "opencv2/highgui.hpp"
#include "Cell.hpp"
#include "Picture.hpp"
#include "Patch_List.hpp"

std::vector<std::vector<cell>> findMatchingPatches(patch_list &target, picture &source, const std::function<long(const cell &, const cell &)> &comp);

long compareGray(const cell& a, const cell& b);

cv::Mat assembleOutput(std::vector<std::vector<cell>> &patch_list, picture &target);

#endif //PICTURETOWOOD_FIND_PATCHES_HPP
