//
// Created by fabian on 17.05.21.
//

#include "Picture.hpp"
using namespace cv;

/**
 * Filtered version of the image.
 */
void picture::show() const {
    for (const auto & pair : this->images) {
        if(not pair.img.empty()){
            namedWindow("Image", WINDOW_AUTOSIZE);
            imshow( "Image", pair.img_filter);
            waitKey ( 10000);//TODO replace with better solution for waiting
        } else{
            std::cout << "No image to show.";
        }
    }
}

/**
 * Load the Image specified by path into the picture object, replacing the current images and replacing the name with the new file name.
 * @param path Absolute path or relative path from the working directory.
 */
picture::picture(const std::string &path, unsigned int dpi, int filter_type, double filter_ratio) {
    this->images.resize(1);
    try {
        String file_path = samples::findFile(path);
        this->origImage.img = imread(file_path, IMREAD_COLOR);
        this->origImage.img_gray = imread(file_path, IMREAD_GRAYSCALE);
        this->origImage.mask = cv::Mat(this->origImage.img.rows,this->origImage.img.cols,CV_8U,255);
        this->origDPI = dpi;
        this->currentDPI = dpi;
        this->filterType = filter_type;
        this->name = file_path.substr(file_path.find_last_of('/')+1, file_path.find_first_of('.') - file_path.find_last_of('/')-1);
        this->filter_ratio = filter_ratio;
        updateImageSet();

    }catch (const std::exception& e) {
        std::cout << "Could not read image because of:\n" << e.what();
    }
}

/**
 * Updates the imageset of the object
 */
void picture::updateImageSet() {
    images[0].img = origImage.img.clone() ;
    images[0].img_gray = origImage.img_gray.clone() ;

    //scale image
    if(currentDPI != origDPI){
        double scale = (double) currentDPI/origDPI;
        if(scale>1.0){
            resize(this->images[0].img, this->images[0].img, Size(), scale, scale, cv::INTER_CUBIC);
            resize(this->images[0].img_gray, this->images[0].img_gray, Size(), scale, scale, cv::INTER_CUBIC);
        }else{
            resize(this->images[0].img, this->images[0].img, Size(), scale, scale, cv::INTER_AREA);
            resize(this->images[0].img_gray, this->images[0].img_gray, Size(), scale, scale, cv::INTER_AREA);
        }
    }

    //create filter
    switch(this->filterType){
        case 0:
        {
            Mat sobelX;
            Mat sobelY;
            Sobel(images[0].img_gray, sobelX, CV_16S, 1, 0, 3);
            Sobel(images[0].img_gray, sobelY, CV_16S, 0, 1, 3);

            // converting back to CV_8U
            Mat absX;
            Mat absY;
            convertScaleAbs(sobelX, absX);
            convertScaleAbs(sobelY, absY);

            addWeighted(absX, 0.5, absY, 0.5, 0, images[0].img_filter);

            break;
        }
        case 1:
            Canny(images[0].img_gray, images[0].img_filter, 255 / 3, 255);
            break;
        case 2: {
            Mat filter16S;
            Laplacian(images[0].img_gray, filter16S, CV_16S);
            convertScaleAbs(filter16S, images[0].img_filter);
            break;
        }
        default:
            std::cout << "Ignoring filter";
            break;
    }
    images[0].img_filter = images[0].img_filter * this->filter_ratio + images[0].img_gray * (1-this->filter_ratio);
    images[0].mask = Mat(images[0].img_gray.rows, images[0].img_gray.cols , images[0].img_gray.type(), 255);
}

/**
 * Creates rotations of the original image.
 * @param n Number of rotations that should be added to standard rotation.
 */
void picture::addRotations(int n) {
    this->images.resize(n);
    auto src = images[0].img;
    auto src_gray = images[0].img_gray;
    auto src_filter = images[0].img_filter;
    auto src_mask = images[0].mask;
    for (int i = 1; i <images.size(); i++) {
        auto &pair = images[i];
        double angle = i*360.0/n;

        // get rotation matrix for rotating the image around its center in pixel coordinates
        cv::Point2f center((src.cols-1)/2.0, (src.rows-1)/2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
        // determine bounding rectangle, center not relevant
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
        // adjust transformation matrix
        rot.at<double>(0,2) += bbox.width/2.0 - src.cols/2.0;
        rot.at<double>(1,2) += bbox.height/2.0 - src.rows/2.0;

        cv::warpAffine(src, pair.img, rot, bbox.size());
        cv::warpAffine(src_gray, pair.img_gray, rot, bbox.size());
        cv::warpAffine(src_filter, pair.img_filter, rot, bbox.size());
        cv::warpAffine(src_mask, pair.mask, rot, bbox.size());

    }
}
/**
 * Updates the masks of all rotations.
 */
void picture::updateMasks() {
    auto src = images[0].mask;
    for (int i = 1; i < images.size(); i++) {
        auto &pair = images[i];
        double angle = i * 360.0 / images.size();

        // get rotation matrix for rotating the image around its center in pixel coordinates
        cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
        // determine bounding rectangle, center not relevant
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
        // adjust transformation matrix
        rot.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
        rot.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;

        cv::warpAffine(src, pair.mask, rot, bbox.size());
    }
}
/**
 * Scales the images and updates the rotations
 */
void picture::scaleTo(unsigned int dpi) {
    this->currentDPI = dpi;
    updateImageSet();
    addRotations(images.size());
}
/**
 * Performs histogram matching based on the given histogram
 * @param targetHist Histogram that should be matched.
 * @param channel Color channel that should be used for histogram matching.
 * @param ratio Ratio between original color values and histogram matched color values.
 */
void picture::transformHistTo(cv::Mat targetHist, int channel, double ratio) {
    auto &s = this->origImage.img;
    int channels[] = {channel};
    int histSize[] = {255};
    float r[] = {0,256};
    const float *ranges[] = {r};

    cv::Mat inputHist;
    cv::calcHist(&s, 1, channels, cv::Mat(), inputHist, 1, histSize, ranges, true, false);

    //normalize
    cv::normalize(targetHist, targetHist, 1, 0, cv::NORM_L1);
    cv::normalize(inputHist, inputHist, 1, 0, cv::NORM_L1);

    //generate cdfs
    std::vector<double> cdf_t(256);
    std::vector<double> cdf_i(256);

    cdf_t[0] = targetHist.at<float>(0);
    cdf_i[0] = inputHist.at<float>(0);
    for (int j = 0; j < 256; j++) {
        cdf_t[j + 1] += cdf_t[j] + targetHist.at<float>(j + 1);
        cdf_i[j + 1] += cdf_i[j] + inputHist.at<float>(j + 1);
    }


    std::vector<uchar> T_target(256);
    std::vector<uchar> T_input(256);
    for (int j = 0; j<256; j++) {
        T_target[j] = floor(255 * cdf_t[j]);
        T_input[j] = floor(255 * cdf_i[j]);
    }

    //create mapping
    std::vector<int> map(256);

    for (int i = 0; i < 256; ++i) {
        int diff = abs(T_input[i]-T_target[0]);
        map[i] = 0;
        for (int j = 0; j < 256; ++j) {
            if(abs(T_input[i]-T_target[j])<diff){
                map[i] = j;
                diff = abs(T_input[i]-T_target[j]);
            }
        }
    }

    //apply map
    cv::Mat workingChannel;
    cv::Mat origChannel;
    cv::extractChannel(this->origImage.img, workingChannel,channel);
    cv::extractChannel(this->origImage.img, origChannel,channel);

    workingChannel.forEach<uchar>(
            [map](uchar &x, const int * position){
                x= map[x];
            });
    workingChannel =  workingChannel*ratio+origChannel*(1-ratio);
    cv::insertChannel(workingChannel,this->origImage.img, channel);
    cvtColor(this->origImage.img, this->origImage.img_gray, cv::COLOR_BGR2GRAY);
    this->updateImageSet();

}

/**
 * Removes pixel of a certain color from the image and adds them to the mask.
 * @param color Color that should be part of the mask
 */
void picture::addColorToMask(Vec3b color) {

    for(int i = 0; i < this->origImage.img.rows; i++)
    {
        const cv::Vec3b *Pi = this->origImage.img.ptr<cv::Vec3b >(i);
        auto *Mi = this->origImage.mask.ptr<uchar>(i);
        for(int j = 0; j < this->origImage.img.cols; j++)
            if(Pi[j] == color) {
                Mi[j] = 0;
            }
    }
    this->images[0].mask = images[0].mask.clone();

    cv::resize(this->origImage.mask,this->images[0].mask, cv::Size(images[0].mask.cols,images[0].mask.rows));
    updateMasks();
}

/**
 * Creates a combined histogram of a list of images.
 * @param picList List of images.
 * @param channel Color channel that should be used for histogram creation.
 * @returns Cumulative Histogram of the images.
 */
cv::Mat cumulativeHist(std::vector<picture> &picList, int channel) {
    cv::Mat hist;
    for (auto &p : picList) {
        auto &s = p.origImage.img;
        int channels[] = {channel};
        int histSize[] = {255};
        float r[] = {0,256};
        const float *ranges[] = {r};

        cv::calcHist(&s,1,channels,cv::Mat(),hist,1,histSize, ranges, true, true);

    }
    return hist;
}
