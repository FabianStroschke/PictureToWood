//
// Created by fabian on 17.05.21.
//

#include "Picture.hpp"
using namespace cv;

/**
 * Load the Image specified by path into the picture object.
 * @param path Absolute path or relative path from the working directory.
 * @return Creates a picture Object containing the colored and grayscale version of the image.
 */
picture::picture(const std::string &path, unsigned int dpi) {
    this->images.resize(1);
    this->loadImg(path, 0, dpi);
}

/**
 * Show the colored and grayscale version of the image.
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
void picture::loadImg(const std::string &path, int filter_type, unsigned int dpi) {
    try {
        String file_path = samples::findFile(path);
        this->origImage.img = imread(file_path, IMREAD_COLOR);
        this->origImage.img_gray = imread(file_path, IMREAD_GRAYSCALE);
        this->origDPI = dpi;
        this->currentDPI = dpi;
        this->filterType = filter_type;
        this->name = file_path.substr(file_path.find_last_of('/')+1, file_path.find_first_of('.') - file_path.find_last_of('/')-1);

        updateImageSet();

    }catch (const std::exception& e) {
        std::cout << "Could not read image because of:\n" << e.what();
    }
}

void picture::updateImageSet() {
    images[0].img = origImage.img.clone() ;
    images[0].img_gray = origImage.img_gray.clone() ;
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
    images[0].mask = Mat(images[0].img_gray.rows, images[0].img_gray.cols , images[0].img_gray.type(), 255);
}

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

void picture::scaleTo(unsigned int dpi) {
    this->currentDPI = dpi;
    updateImageSet();
    addRotations(images.size());
}

void picture::transformHistTo(cv::Mat targetHist) {
    auto &s = this->origImage.img_gray;
    int channels[] = {0};
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

    namedWindow("Image", WINDOW_AUTOSIZE);
    imshow( "Image", origImage.img_gray);
    waitKey ( 10000);//TODO replace with better solution for waiting

    //apply map
    this->origImage.img_gray.forEach<uchar>(
            [map](uchar &x, const int * position){
                x= map[x];
            });
    this->updateImageSet();

    namedWindow("Image2", WINDOW_AUTOSIZE);
    imshow( "Image2", origImage.img_gray);
    waitKey ( 10000);//TODO replace with better solution for waiting


    cv::Mat newHist;
    cv::calcHist(&(origImage.img_gray),1,channels,cv::Mat(),newHist,1,histSize, ranges, true, false);
    cv::normalize(newHist, newHist, 1, 0, cv::NORM_L1);


    std::vector<double> cdf_n(256);
    cdf_n[0] = inputHist.at<float>(0);
    for (int i = 0; i<256; i++) {
        cdf_n[i+1] += cdf_n[i] + newHist.at<float>(i+1);
    }

    for (int i = 0; i < 256; ++i) {
        std::cout << i << " " << cdf_t[i] << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < 256; ++i) {
        std::cout << i << " " << cdf_i[i] << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < 256; ++i) {
        std::cout << map[i] << " " << cdf_i[i] << " ";
    }
    std::cout << "\n";

    for (int i = 0; i < 256; ++i) {
        std::cout << i << " " << cdf_n[i]-0.02 << " ";
    }
    std::cout << "\n";
}

cv::Mat cumulativeHist(std::vector<picture>& picList){
    cv::Mat hist;
    for (auto &p : picList) {
        auto &s = p.origImage.img_gray;
        int channels[] = {0};
        int histSize[] = {255};
        float r[] = {0,256};
        const float *ranges[] = {r};

        cv::calcHist(&s,1,channels,cv::Mat(),hist,1,histSize, ranges, true, true);

    }
    return hist;
}
