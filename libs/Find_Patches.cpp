//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"

cv::Mat stitchPicture(std::vector<std::vector<cell *>> &patch_list);

std::vector<std::vector<cell *>> findMatchingPatches(const std::vector<std::vector<cell>>& target, std::vector<std::vector<cell>>& source, const std::function<long(const cell &, const cell &)> &comp){
    std::vector<std::vector<cell *>> match(target.size(),std::vector<cell *>(target.front().size(), nullptr));
    boost::asio::thread_pool pool(boost::thread::hardware_concurrency());
    for(int x = 0; x < target.size(); x++){
        for(int y = 0; y < target[x].size(); y++){
            const cell& t = target[x][y];
            auto func = [x , y, &t, &source, &match, &comp](){
                cell *best = &(source.front().front());
                long min = comp(t, *best);
                for(std::vector<cell>& col : source){
                    for(cell& s : col){
                        long diff = comp(t,s);
                        if(diff < min){
                            min = diff;
                            best = &s;
                        }
                    }
                }
                match[x][y] = best;
            };
            boost::asio::post(pool, func);
}
    }
    while(match.back().back() == nullptr){
        cv::waitKey(200);
        auto out = stitchPicture(match);
        if(not out.empty()){
            namedWindow("Final Image", cv::WINDOW_AUTOSIZE);
            imshow( "Final Image", out);
        }
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

cv::Mat stitchPicture(std::vector<std::vector<cell *>> &patch_list) {
    if(patch_list.front().front() != nullptr){
        int x = (int) patch_list.size();
        int y = (int) patch_list.front().size();
        int width = patch_list.front().front()->img.cols;
        int height = patch_list.front().front()->img.rows;

        cv::Mat matDst(cv::Size(width * x, height * y), patch_list.front().front()->img.type(), cv::Scalar::all(0));
        for(int j = 0; j<y; j++){
            for(int i = 0; i<x; i++){
                cv::Mat matRoi = matDst(cv::Rect(width*i,height*j,width,height));
                if(patch_list[i][j] != nullptr){
                    patch_list[i][j]->img.copyTo(matRoi);
                }
            }
        }
        return matDst;
    }
}

cv::Mat assembleOutput(std::vector<std::vector<cell *>> &patch_list, Picture &target){
    cv::Mat matDst = stitchPicture(patch_list);

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