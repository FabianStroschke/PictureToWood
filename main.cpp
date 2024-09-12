#include "json.hpp"
#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>
#include <Patch_List.hpp>
#include <Pattern.hpp>

nlohmann::json checkInputs(int argc, char ** argv){
    if(argc < 1 || argc > 3){
        std::cout << "Expected 0 to 2 arguments but received " << argc <<  " arguments.\n\nUsage:\nmain <path to config.json> OR \nmain <path to config.json> <path to layout.json>" << "\n";
        exit(1);
    }

    std::string configPath;
    configPath.append("./input/Config/config.json"); //default location

    //get file path from config
    if(argc >= 2){
        configPath.clear();
        configPath.append(argv[1]);
    }

    //check default/custom filepath if valid
    if(configPath.find(".json", configPath.length()-6) == std::string::npos){
        std::cout << "Provided filepath is not a .json file." << "\n";
        exit(1);
    }

    if(argc == 3){
        std::string layoutPath;
        layoutPath.append(argv[2]);
        if(layoutPath.find(".json", layoutPath.length()-6) == std::string::npos){
            std::cout << "Provided filepath is not a .json file." << "\n";
            exit(1);
        }
    }

    //read config file
    std::ifstream i(configPath);
    nlohmann::json j;
    i >> j;

    unsigned long pathEnd = configPath.find_last_of('/');
    j["offset"] = configPath.substr(0, pathEnd+1);

    return j;
}

int main( int argc, char ** argv ) {
    nlohmann::json config  = checkInputs(argc,argv);

    //load target picture
    auto t = config["target"];
    picture target(config["offset"].get<std::string>() + t["path"].get<std::string>(),
                   1, config["filter_type"], config["filter_intens_ratio"]);
    target.origDPI = (target.origImage.img.cols / t["output_width_cm"].get<double>())*2.54;
    target.currentDPI = target.origDPI;
    target.scaleTo(config["cut_map"]["dpi"]);

    //load source textures
    std::vector<picture> texture_list;
    for (auto & e : config["woodTextures"]) {
        texture_list.emplace_back(config["offset"].get<std::string>() + e["path"].get<std::string>(), e["dpi"], config["filter_type"], config["filter_intens_ratio"]);
        texture_list.back().scaleTo(target.currentDPI);
        texture_list.back().addRotations(config["rotations"]);
        texture_list.back().addColorToMask(cv::Vec3b(0,0,0));
    }

    //histogram matching
    target.transformHistTo(cumulativeHist(texture_list, 0), 0, config["histogram_match_ratio"]);
    target.transformHistTo(cumulativeHist(texture_list, 1), 1, config["histogram_match_ratio"]);
    target.transformHistTo(cumulativeHist(texture_list, 2), 2, config["histogram_match_ratio"]);

    //read pattern
    Pattern pattern;
    if(argc == 3){
        pattern = Pattern(argv[2]);

    }else{
        pattern = Pattern(config["offset"].get<std::string>() + config["layout_path"].get<std::string>());
    }
    pattern.convertToCm(target.currentDPI);
    pattern.show(cv::Size(target.images[0].img.cols,target.images[0].img.rows));

    //create patchlist
    auto plist = patch_list(target, pattern);

    //find matches
    startTimer();
    findMatchingPatches(plist, texture_list, config["stepSize"]["x"], config["stepSize"]["y"], config["cut_map"]["cut_width_mm"], compareFilter);

    //generate & save output
    auto output = assembleOutput(plist, config["output"]["appendix"]);
    generateCutMap(plist, config["output"]["dpi"], config["cut_map"]["cut_width_mm"], output, config["cut_map"]["text_scale"], true, false);
    endTimer();
    log();
}