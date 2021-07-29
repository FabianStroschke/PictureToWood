//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"

cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list);

boost::mutex claimMutex;
boost::mutex finishMutex;

std::vector<std::vector<cell>> findMatchingPatches(patch_list &target, std::vector<picture> &source, const int stepX, const int stepY, const std::function<long(const cell &, const cell &)> &comp){
    auto &patches = target.patches;
    std::vector<std::vector<cell>> match(patches.size(),std::vector<cell>(patches.front().size()));
    boost::asio::thread_pool pool(boost::thread::hardware_concurrency());
    std::vector<cv::Point3d> score;
    score.reserve((patches.size() * patches[0].size()));
    std::vector<cv::Point3i> prio_map;
    prio_map.reserve((patches.size() * patches[0].size()));
    int offsetX = patches.size()/2;
    int offsetY = patches[0].size()/2;

    cv::Mat salMat;
    cv::saliency::StaticSaliencyFineGrained s;
    s.computeSaliency(target.patches[0][0].source->images[0].img, salMat);

    double maxDist = 0;
    double maxSal = 0;

    for(int x = 0; x < patches.size(); x++){
        for(int y = 0; y < patches[x].size(); y++) {
            double dist = abs(x-offsetX) + abs(y-offsetY);

            if(dist > maxDist){
                maxDist = dist;
            }

            //compute detail score
            cell &c = target.patches[x][y];
            cv::Rect rec(c.x,c.y,c.width,c.height);
            cv::Mat data = salMat(rec);
            if(c.shape){
                data.copyTo(data,*c.shape);
            }

            double sal =  *cv::sum(data).val;
            if(sal > maxSal){
                maxSal = sal;
            }
            score.emplace_back(dist, sal,0);
        }
    }

    for (auto &e:score) {
        e.x = 1-(e.x / maxDist);
        e.y = e.y / maxSal;
        e.z = (e.x+e.y)*score.size();
    }

    for(int x = 0; x < patches.size(); x++){
        for(int y = 0; y < patches[x].size(); y++) {
            prio_map.emplace_back(x,y,score[x * (patches[x].size()) +y].z);
        }
    }

    auto compPrio = [](const cv::Point3i& p1, const cv::Point3i& p2){
        return p1.z > p2.z;
    };
    std::sort(prio_map.begin(), prio_map.end(), compPrio);

    unsigned long openProcesses = prio_map.size();

    for(auto &p: prio_map){
        int x = p.x;
        int y = p.y;
        const cell& t = patches[x][y];

            auto func = [x , y, stepX, stepY, &t, &source, &match, &comp, &openProcesses](){
                bool cellClaimed = false;
                int count = 0;
                while(not cellClaimed && count <10 ) { //limit for tries to find a patch
                    count++;
                    cell best(&source[0], t.shape, 0, 0);
                    cell cur(&source[0], t.shape, 0, 0);
                    long min = -1;

                for (auto &s: source) {
                    cur.source = &s;
                    for (int r = 0; r < s.images.size(); r++) {
                        cur.rot = r;
                        auto img = s.images[r].img;
                        for (int y_i = 0; y_i < img.rows - t.height; y_i += stepY) {
                            for (int x_i = 0; x_i < img.cols - t.width; x_i += stepX) {
                                cur.moveTo(x_i, y_i);
                                long diff = comp(t, cur);
                                if ((diff < min && diff > 0) || min < 0) {
                                    min = diff;
                                    best = cur;
                                    //match[x][y] = best;
                                }
                            }
                        }
                    }
                }
                match[x][y] = best;
                claimMutex.lock();
                cellClaimed = match[x][y].claimCell();
                claimMutex.unlock();

            }
            finishMutex.lock();
            openProcesses--;
            finishMutex.unlock();
        };
        boost::asio::post(pool, func);
    }


    while(openProcesses>0){
        cv::waitKey(20);
        auto out = stitchPicture(match);
        if(not out.empty()){
            namedWindow("Final Image", cv::WINDOW_AUTOSIZE);
            imshow( "Final Image", out);

            cv::Mat maskedImg;
            cv::bitwise_and(source.back().images[0].img_gray,source.back().images[0].mask,maskedImg);

            namedWindow("Cut", cv::WINDOW_AUTOSIZE);
            imshow( "Cut", maskedImg);
        }
    }
    pool.join();
    return match;
}

long compareFilter(const cell& a, const cell& b){
    long sum = 0;
    for(int y = 0; y<a.height; y++){
        uchar *aPtrFilter = a.source->images[a.rot].img_filter.ptr(a.y+y, a.x);
        uchar *bPtrFilter = b.source->images[b.rot].img_filter.ptr(b.y+y, b.x);
        uchar *aPtrMask = a.source->images[a.rot].mask.ptr(a.y+y, a.x);
        uchar *bPtrMask = b.source->images[b.rot].mask.ptr(b.y+y, b.x);
        for(int x = 0; x<a.width; x++, aPtrMask++, bPtrMask++, aPtrFilter++, bPtrFilter++){
            if(*aPtrMask == 0 || *bPtrMask ==0) return -1;
            auto diff = (*aPtrFilter - *bPtrFilter);
            sum += diff*diff;
        }
    }
    return sum;
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
            auto diff1 = (*aPtr - *bPtr);
            sum += (diff1*diff1);
        }
    }
    return sum;
}


cv::Mat stitchPicture(std::vector<std::vector<cell>> &patch_list) {
    cell *c = nullptr;
    for (auto &e:patch_list) {
        for (auto &p:e) {
            if(p.source != nullptr){
                c = &p;
                break;
            }
        }
    }
    if(c == nullptr){
        return cv::Mat();
    }

    int x = (int) patch_list.size();
    int y = (int) patch_list.front().size();
    int width = c->width;
    int height = c->height;

    cv::Mat matDst(cv::Size(width * x, height * y), c->source->images[0].img.type(), cv::Scalar::all(0));
    for(int j = 0; j<y; j++){
        for(int i = 0; i<x; i++){
            if(patch_list[i][j].source != nullptr) {
                cv::Mat matRoi = matDst(cv::Rect(width * i, height * j, width, height));
                patch_list[i][j].data.copyTo(matRoi);
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