#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>
#include <Patch_List.hpp>
#include <nlohmann/json.hpp>

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

    picture target(config["offset"].get<std::string>() + config["target"]["path"].get<std::string>());
    picture sample(config["offset"].get<std::string>() + config["woodTextures"].at(0)["path"].get<std::string>());
    //TODO add multiple samples

    //target.show();
    //sample.show();

    sample.addRotations(config["rotations"]);
    //sample.show();

    auto plist = patch_list(target,config["patchSize"]["x"],config["patchSize"]["y"]);

    startTimer();
    auto list = findMatchingPatches(plist, sample, config["stepSize"]["x"], config["stepSize"]["y"],compareGray);
    auto output = assembleOutput(list, target);
    endTimer();
    log();
}