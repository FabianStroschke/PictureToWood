//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"

cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list);

std::vector<std::vector<cell>> findMatchingPatches(const std::vector<std::vector<cell>>& target, Picture &source, const std::function<long(const cell &, const cell &)> &comp){
    std::vector<std::vector<cell>> match(target.size(),std::vector<cell>(target.front().size()));
    boost::asio::thread_pool pool(boost::thread::hardware_concurrency());
    for(int x = 0; x < target.size(); x++){
        for(int y = 0; y < target[x].size(); y++){
            const cell& t = target[x][y];
            auto func = [x , y, &t, &source, &match, &comp](){
                cv::Rect rec(0,0,t.width,t.height);
                cell best(source.img,rec);
                long min = comp(t, best);
                for(int y_i = 0; y_i < source.img.rows - t.height; y_i+=7){
                    for(int x_i = 0; x_i < source.img.cols - t.width; x_i+=7){
                        rec.x = x_i;
                        rec.y = y_i;
                        cell cur(source.img,rec);
                        long diff = comp(t,cur);
                        if(diff < min){
                            min = diff;
                            best = cur;
                            //match[x][y] = best;
                        }
                    }
                }
                match[x][y] = best;
            };
            boost::asio::post(pool, func);
}
    }
    while(match.back().back().width == 0){
        cv::waitKey(20);
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

cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list) {
    if(!patch_list.front().front().img.empty()){
        int x = (int) patch_list.size();
        int y = (int) patch_list.front().size();
        int width = patch_list.front().front().img.cols;
        int height = patch_list.front().front().img.rows;

        cv::Mat matDst(cv::Size(width * x, height * y), patch_list.front().front().img.type(), cv::Scalar::all(0));
        for(int j = 0; j<y; j++){
            for(int i = 0; i<x; i++){
                cv::Mat matRoi = matDst(cv::Rect(width*i,height*j,width,height));
                if(!patch_list[i][j].img.empty()){
                    //cv::waitKey(100);
                    patch_list[i][j].img.copyTo(matRoi);
                }
            }
        }
        return matDst;
    }
    return cv::Mat();
}

cv::Mat assembleOutput(std::vector<std::vector<cell>> &patch_list, Picture &target){
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