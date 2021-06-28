#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>
#include <Patch_List.hpp>
#include <nlohmann/json.hpp>

char filename[] = "input/Target4.png";
char filename2[] = "input/wood_sample2.png";
//char filename2[] = "input/16_wood_samples.jpg";

void readJSON(){
    std::ifstream i("input/config.json");
    nlohmann::json j;
    i >> j;
    std::cout << j["rotations"] << "\n";

}

int main( int argc, char ** argv ) {
    readJSON();

    picture target(filename);
    picture sample(filename2);
    //target.show();
    //sample.show();

    sample.addRotations(13);
    //sample.show();

    auto plist = patch_list(target,20,20);

    startTimer();
    auto list = findMatchingPatches(plist, sample, compareGray);
    auto output = assembleOutput(list, target);
    endTimer();
    log();
}