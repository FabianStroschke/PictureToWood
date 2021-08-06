#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>
#include <Patch_List.hpp>
#include <Pattern.hpp>
#include "json.hpp"

void checkInputs(int argc, char ** argv){
    if(argc != 2){
        std::cout << "Expected 1 argument but received " << argc <<  " arguments.\n\nUsage: main <path to config.json>" << "\n";
        exit(1);
    }
    std::string filepath;
    filepath.append(argv[1]);
    if(filepath.find(".json", filepath.length()-6) == std::string::npos){
        std::cout << "Provided filepath is not a .json file." << "\n";
        exit(1);
    }
}

nlohmann::json readJSON(char ** argv){
    std::ifstream i(argv[1]);
    nlohmann::json j;
    i >> j;

    std::string filepath;
    filepath.append(argv[1]);
    unsigned long pathEnd = filepath.find_last_of('/');
    j["offset"] = filepath.substr(0, pathEnd+1);

    //TODO check inputs

    return j;
}

int main( int argc, char ** argv ) {
    checkInputs(argc,argv);
    nlohmann::json config  = readJSON(argv);

    auto t = config["target"];
    picture target(config["offset"].get<std::string>() + t["path"].get<std::string>(),
                   t["dpi"], config["filter_type"], config["filter_intens_ratio"]);

    std::vector<picture> texture_list;
    for (auto & e : config["woodTextures"]) {
        texture_list.emplace_back(config["offset"].get<std::string>() + e["path"].get<std::string>(), e["dpi"], config["filter_type"], config["filter_intens_ratio"]);
        texture_list.back().scaleTo(target.origDPI);
        texture_list.back().addRotations(config["rotations"]);
    }
    target.scaleTo(150);
    target.transformHistTo(cumulativeHist(texture_list, 0), 0);
    target.transformHistTo(cumulativeHist(texture_list, 1), 1);
    target.transformHistTo(cumulativeHist(texture_list, 2), 2);

    /**TEST**/
    char str[] = "./input/layout.json";
    auto test = Pattern(str);
    test.scalePattern(2,2);

    cv::Mat shape(config["patchSize"]["y"],config["patchSize"]["x"], CV_8U, cv::Scalar(0));
    std::vector<cv::Point> pts;
    pts.emplace_back(0,0);
    pts.emplace_back(40,0);
    pts.emplace_back(60,20);
    pts.emplace_back(40,40);
    pts.emplace_back(0,40);
    pts.emplace_back(20,20);

    cv::fillConvexPoly(shape,pts,255);

    auto plist = patch_list(target,test);


    /**TEST END**/

    //auto plist = patch_list(target,config["patchSize"]["y"],config["patchSize"]["x"]);

    startTimer();
    findMatchingPatches(plist, texture_list, config["stepSize"]["x"], config["stepSize"]["y"],compareFilter);
    auto output = assembleOutput(plist);
    endTimer();
    log();
}