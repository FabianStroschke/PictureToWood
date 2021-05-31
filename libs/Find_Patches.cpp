//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"


std::vector<cell *> findMatchingPatches(const std::vector<cell>& target, std::vector<cell>& source, const std::function<long(const cell &, const cell &)> &comp){
    std::vector<cell *> match(target.size());
    boost::asio::thread_pool pool(6);

    for(int i = 0; i < target.size(); i++){
        const cell& t = target[i];
        auto func = [i, &t, &source, &match, &comp](){
            cell *best = &(source.front());
            long min = comp(t, *best);
            for(cell& s : source){
                long diff = comp(t,s);
                if(diff < min){
                    min = diff;
                    best = &s;
                }
            }
            match.at(i) = best;
        };
        boost::asio::post(pool, func);
    }
    pool.join();
    return match;
}

long compareGray(const cell& a, const cell& b){
    int numPixel = a.img_gray.dataend - a.img_gray.datastart;
    long sum = 0;
    for(int i = 0; i < numPixel; i++){
        sum += std::abs((a.img_gray.data[i] - b.img_gray.data[i]));
    }
    return sum;
}
cv::Mat assambleOutput(std::vector<cell *> &patch_list, Picture &target){
    int x = target.img.cols / patch_list.front()->width;
    int y = (int) patch_list.size() / x;
    int width = patch_list.front()->img.cols;
    int height = patch_list.front()->img.rows;

    cv::Mat matDst(cv::Size(width*x,height*y),patch_list.front()->img.type(),cv::Scalar::all(0));
    for(int j = 0; j< y; j++){
        for(int i = 0; i<x; i++){
            cv::Mat matRoi = matDst(cv::Rect(width*i,height*j,width,height));
            patch_list[i+x*j]->img.copyTo(matRoi);
        }
    }


    //checks if the output directory exist or creates
    std::string output_path = boost::filesystem::current_path().string()+ "/Output";
    if(not boost::filesystem::exists(output_path))
        boost::filesystem::create_directory(output_path);

    //creates a sub directory for the image or clears an existing one
    output_path += "/Final_Picture";
    if(boost::filesystem::exists(output_path)){
        boost::filesystem::remove_all(output_path);
        boost::filesystem::create_directory(output_path);
    }else{
        boost::filesystem::create_directory(output_path);
    }

    //writes the images
    imwrite(output_path + "/result.tiff", matDst);

    //show image
    imshow("result",matDst);
    return matDst;
}