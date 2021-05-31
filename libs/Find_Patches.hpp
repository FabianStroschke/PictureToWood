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


std::vector<cell *> findMatchingPatches(const std::vector<cell>& target, std::vector<cell>& source, const std::function<long(const cell &, const cell &)> &comp);

long compareGray(const cell& a, const cell& b);

cv::Mat assambleOutput(std::vector<cell *> &patch_list, Picture &target);

#endif //PICTURETOWOOD_FIND_PATCHES_HPP
