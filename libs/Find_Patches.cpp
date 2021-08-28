//
// Created by fabian on 25.05.21.
//

#include "Find_Patches.hpp"

cv::Mat stitchPicture(patch_list &patches);

std::vector<cv::Point>
getTransformedPoints(int outputDPI, bool flipV, bool flipH, std::map<picture *, cv::Mat> &m, cell &c);

boost::mutex claimMutex;
boost::mutex finishMutex;

void
findMatchingPatches(patch_list &target, std::vector<picture> &source, const int stepX, const int stepY, double cutWidth,
                    const std::function<long(const cell &, const cell &)> &comp) {
    auto &patches = target.patches;
    boost::asio::thread_pool pool(boost::thread::hardware_concurrency());
    std::vector<cv::Point3d> score;
    score.reserve((patches.size() * patches[0].size()));
    std::vector<cv::Point3i> prio_map;
    prio_map.reserve((patches.size() * patches[0].size()));
    int offsetX = target.size.width/2;
    int offsetY = target.size.height/2;

    cv::Mat salMat;
    cv::saliency::StaticSaliencyFineGrained s;
    s.computeSaliency(target.patches[0][0].target.source->images[0].img, salMat);

    double maxDist = 0;
    double maxSal = 0;

    for(int x = 0; x < patches.size(); x++){
        for(int y = 0; y < patches[x].size(); y++) {
            auto &p = patches[x][y].target;
            double dist = abs(p.x+p.width/2-offsetX) + abs(p.y+p.height/2-offsetY);
            if(dist > maxDist){
                maxDist = dist;
            }

            //compute detail score
            cell &c = target.patches[x][y].target;
            cv::Rect rec(c.x,c.y,c.width,c.height);
            cv::Mat data = salMat(rec);
            if(c.shape){
                data.copyTo(data,c.shape->mask);
            }

            double sal =  *cv::sum(data).val;
            sal = sal / std::count(c.shape->mask.begin<uchar>(), c.shape->mask.end<uchar>(), 255);
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
    std::cout << "\n";

    int next = 0;
    for(int x = 0; x < patches.size(); x++){
        for(int y = 0; y < patches[x].size(); y++) {
            prio_map.emplace_back(x,y,score[next].z);
            next++;
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
        patch& curPatch = patches[x][y];

            auto func = [cutWidth, stepX, stepY, &curPatch, &source,  &comp, &openProcesses](){
                bool cellClaimed = false;
                int count = 0;
                auto &t = curPatch.target;
                while(not cellClaimed && count <20 ) { //limit for tries to find a patch
                    count++;
                    curPatch.source = cell(&source[0], t.shape, 0, 0, t.stepWidth, t.stepHeight);
                    cell cur(&source[0], t.shape, 0, 0, t.stepWidth, t.stepHeight);
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
                                    curPatch.source = cur;
                                    //match[x][y] = best;
                                }
                            }
                        }
                    }
                }

                claimMutex.lock();
                cellClaimed = curPatch.source.claimCell(std::ceil(curPatch.source.source->currentDPI/25.4*cutWidth));
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
        auto out = stitchPicture(target);
        if(not out.empty()){
            namedWindow("Final Image", cv::WINDOW_AUTOSIZE);
            imshow( "Final Image", out);
/*
            cv::Mat maskedImg;
            cv::bitwise_and(source.back().images[0].img_gray,source.back().images[0].mask,maskedImg);

            namedWindow("Cut", cv::WINDOW_AUTOSIZE);
            imshow( "Cut", source[1].images[0].mask);
*/
        }
    }
    pool.join();
}

long compareFilter(const cell& a, const cell& b){
    long sum = 0;
    for(int y = 0; y<a.height; y++){
        uchar *aPtrFilter = a.source->images[a.rot].img_filter.ptr(a.y+y, a.x);
        uchar *bPtrFilter = b.source->images[b.rot].img_filter.ptr(b.y+y, b.x);
        uchar *aPtrMask = a.source->images[a.rot].mask.ptr(a.y+y, a.x);
        uchar *bPtrMask = b.source->images[b.rot].mask.ptr(b.y+y, b.x);
        const uchar *aPtrShape = a.shape->mask.ptr(y, 0); //shapes should be the same, maybe check if the shapes are equal before

        for(int x = 0; x<a.width; x++, aPtrMask++, bPtrMask++, aPtrFilter++, bPtrFilter++, aPtrShape++){
            if(*aPtrShape != 0){
                if(*aPtrMask == 0 || *bPtrMask ==0) return -1;
                auto diff = (*aPtrFilter - *bPtrFilter);
                sum += diff*diff;
            }
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


cv::Mat stitchPicture(patch_list &patches) {
    /*   cell *c = nullptr;
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
       int sizeX = c->width+(x-1)*c->stepWidth;
       int sizeY = c->height+(y-1)*c->stepHeight;

   */

    cv::Mat matDst(patches.size, CV_8UC3, cv::Scalar::all(50));
    for(auto &col:patches.patches){
        for(auto &p:col){
            if(not p.source.data.empty()) {
                cv::Mat matRoi = matDst(cv::Rect(p.target.x - patches.offset.x, p.target.y - patches.offset.y, p.target.width, p.target.height));
                p.source.data.copyTo(matRoi, p.source.shape->mask);
            }
        }
    }

    return matDst;
}

std::string assembleOutput(patch_list &patches, std::string appendix) {
    cv::Mat matDst = stitchPicture(patches);

    //checks if the output directory exist or creates
    std::string output_path = boost::filesystem::current_path().string()+ "/Output";
    if(not boost::filesystem::exists(output_path))
        boost::filesystem::create_directory(output_path);

    //creates a sub directory for the image or clears an existing one
    output_path += "/Final_Picture";
    if(!boost::filesystem::exists(output_path)){
        boost::filesystem::create_directory(output_path);
    }
    std::string filename = output_path + "/result"+appendix;
    int count = 0;
    while (boost::filesystem::exists(filename)){
        count++;
        filename = output_path + "/result"+appendix+std::to_string(count);
    }
    boost::filesystem::create_directory(filename);

    //writes the images
    imwrite(filename+"/synthetic.tiff", matDst);

    //show image
    imshow("result",matDst);
    return filename;
}

void
generateCutMap(patch_list &patches, int outputDPI, double cutWidth, std::string outputPath, double textScale,
               bool flipV, bool flipH) {
    textScale *= outputDPI/100;

    //gather used wood textures
    std::vector<picture *>usedSources;
    usedSources.push_back(patches.patches[0][0].target.source);
    for (auto &col:patches.patches) {
        for (auto &p: col) {
            if(std::count(usedSources.begin(), usedSources.end(), p.source.source) == 0){
                usedSources.push_back(p.source.source);
            }
        }
    }

    //create a empty cutmap
    std::map<picture *, cv::Mat> m;
    for (picture *s:usedSources) {
        if(s->origImage.mask.empty()){
            m.insert({s, cv::Mat(s->origImage.img.rows,s->origImage.img.cols,CV_8U,255)});
        }
        else{
            m.insert({s, s->origImage.mask.clone()});
            if(flipH)cv::flip(m.find(s)->second,m.find(s)->second,0);
            if(flipV)cv::flip(m.find(s)->second,m.find(s)->second,1);
        }
        double scale = (double)outputDPI/s->origDPI;
        cv::resize(m.find(s)->second,m.find(s)->second, cv::Size(), scale,scale);
    }

    //generate cuts
    int count = 1;
    for (auto &col:patches.patches) {
        for (auto &c: col) {

            // draw patch in final location
            std::vector<cv::Point> transformedPoints1 = getTransformedPoints(outputDPI, false, false, m, c.target);

            cv::polylines(m.find(c.target.source)->second,transformedPoints1,true,0,(cutWidth*outputDPI/25.4)*2, cv::LINE_AA);
            cv::fillConvexPoly(m.find(c.target.source)->second,transformedPoints1,255);

            cv::Point center1;
            for (auto &e:transformedPoints1) {
                center1 += e;
            }
            center1 = center1 / (int) transformedPoints1.size();

            std::vector<cv::Point> orientationLine1;
            orientationLine1.push_back(transformedPoints1[0]);
            orientationLine1.push_back(0.5*(center1-transformedPoints1[0])+transformedPoints1[0]);
            cv::polylines(m.find(c.target.source)->second,orientationLine1,true,0,(cutWidth*outputDPI/25.4), cv::LINE_AA);

            std::string text1 = std::to_string(count);
            cv::putText(m.find(c.target.source)->second,text1,center1-cv::Point(2*text1.size()*textScale,-2*textScale), cv::FONT_HERSHEY_SIMPLEX,0.25*textScale,0,std::ceil(textScale*(1/1.5)));


            //draw patch on wood texture
            std::vector<cv::Point> transformedPoints = getTransformedPoints(outputDPI, flipV, flipH, m, c.source);

            cv::polylines(m.find(c.source.source)->second,transformedPoints,true,0,(cutWidth*outputDPI/25.4)*2, cv::LINE_AA);
            cv::fillConvexPoly(m.find(c.source.source)->second,transformedPoints,255);

            cv::Point center;
            for (auto &e:transformedPoints) {
                center += e;
            }
            center = center / (int) transformedPoints.size();

            std::vector<cv::Point> orientationLine;
            orientationLine.push_back(transformedPoints[0]);
            orientationLine.push_back(0.5*(center-transformedPoints[0])+transformedPoints[0]);
            cv::polylines(m.find(c.source.source)->second,orientationLine,true,0,(cutWidth*outputDPI/25.4), cv::LINE_AA);


            std::string text = std::to_string(count);
            cv::putText(m.find(c.source.source)->second,text,center-cv::Point(2*text.size()*textScale,-2*textScale), cv::FONT_HERSHEY_SIMPLEX,0.25*textScale,0,std::ceil(textScale*(1/1.5)));

            count++;
        }
    }
    for(auto &x: m){
        imwrite(outputPath+"/"+x.first->name+".tiff", x.second);
    }

}

std::vector<cv::Point> getTransformedPoints(int outputDPI, bool flipV, bool flipH, std::map<picture *, cv::Mat> &m, cell &c) {
    auto &src = c.source->images[0].mask;
    auto &cut = c.source->images[c.rot].mask;
    cv::Mat rotatedMask;
    double angle = -(c.rot*360.0/c.source->images.size());

    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f((cut.cols-1)/2.0, (cut.rows-1)/2.0), angle, 1.0);
    rotMat.at<double>(0,2) += src.cols/2.0 - cut.cols/2.0;
    rotMat.at<double>(1,2) += src.rows/2.0 - cut.rows/2.0;
    rotMat *= (double) outputDPI/c.source->currentDPI;

    std::vector<cv::Point> notTransformedPoints;
    std::vector<cv::Point> transformedPoints;
    for (auto &e: c.shape->points) {
        notTransformedPoints.emplace_back(e.x+c.x, e.y+c.y);
    }

    cv::transform(notTransformedPoints,transformedPoints,rotMat);
    if(flipV){
        for (auto &e: transformedPoints) {
            e.x *= -1;
            e.x += m.find(c.source)->second.cols;
        }
    }
    if(flipH){
        for (auto &e: transformedPoints) {
            e.y *= -1;
            e.y += m.find(c.source)->second.rows;
        }
    }
    return transformedPoints;
}
