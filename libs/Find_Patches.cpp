//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"
#include "Patch_List.hpp"

cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list);

std::vector<std::vector<cell>> findMatchingPatches(patch_list &target, picture &source, const std::function<long(const cell &, const cell &)> &comp){
    auto &patches = target.patches;
    std::vector<std::vector<cell>> match(patches.size(),std::vector<cell>(patches.front().size()));
    boost::asio::thread_pool pool(boost::thread::hardware_concurrency());
    for(int x = 0; x < patches.size(); x++){
        for(int y = 0; y < patches[x].size(); y++){
            const cell& t = patches[x][y];

            auto func = [x , y, &t, &source, &match, &comp](){
                cell best(&source, t.shape, 0, 0);
                cell cur(&source, t.shape, 0, 0);

                long min = comp(t, best);
                for(int r = 0; r < source.images.size(); r++) {
                    cur.rot = r;
                    auto img = source.images[r].img;
                    for (int y_i = 0; y_i < img.rows - t.height; y_i += 7) {
                        for (int x_i = 0; x_i < img.cols - t.width; x_i += 7) {
                            cur.moveTo(x_i, y_i);
                            long diff = comp(t, cur);
                            if (diff < min && diff >0) {
                                min = diff;
                                best = cur;
                                //match[x][y] = best;
                            }
                        }
                    }
                }
                match[x][y] = best;
                //std::cout << cur.x << "\n";
            };
            boost::asio::post(pool, func);
}
    }

    while(match.back().back().source == nullptr){
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
    long sum = 0;
    for(int y = 0; y<a.height; y++){
        uchar *aPtr = a.source->images[a.rot].img_gray.ptr(a.y+y, a.x);
        uchar *bPtr = b.source->images[b.rot].img_gray.ptr(b.y+y, b.x);
        uchar *aPtrMask = a.source->images[a.rot].mask.ptr(a.y+y, a.x);
        uchar *bPtrMask = b.source->images[b.rot].mask.ptr(b.y+y, b.x);
        for(int x = 0; x<a.width; x++,aPtr++, bPtr++, aPtrMask++, bPtrMask++){
            if(*aPtrMask == 0 || *bPtrMask ==0) return -1;
            auto diff = (*aPtr - *bPtr);
            sum += diff*diff;
        }
    }
    return sum;
}


cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list) {
    if(patch_list.front().front().source == nullptr){
        return cv::Mat();
    }

    int x = (int) patch_list.size();
    int y = (int) patch_list.front().size();
    int width = patch_list.front().front().width;
    int height = patch_list.front().front().height;

    cv::Mat matDst(cv::Size(width * x, height * y), patch_list.front().front().source->images[0].img.type(), cv::Scalar::all(0));
    for(int j = 0; j<y; j++){
        for(int i = 0; i<x; i++){
            if(patch_list[i][j].source != nullptr) {
                cv::Mat matRoi = matDst(cv::Rect(width * i, height * j, width, height));
                patch_list[i][j].source->images[patch_list[i][j].rot].img(cv::Rect(patch_list[i][j].x, patch_list[i][j].y, patch_list[i][j].width,
                                                      patch_list[i][j].height)).copyTo(matRoi);
            }
        }
    }

    return matDst;
}

cv::Mat assembleOutput(std::vector<std::vector<cell>> &patch_list, picture &target){
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