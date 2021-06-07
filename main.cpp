#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>
#include <Patch_List.hpp>

char filename[] = "input/Target3.jpg";
char filename2[] = "input/wood_paper.png";

int main( int argc, char ** argv ) {
    picture target(filename);
    picture sample(filename2);
    //target.show();
    //sample.show();

    auto plist = patch_list(target,20,20);

    startTimer();
    auto list = findMatchingPatches(plist, sample, compareGray);
    auto output = assembleOutput(list, target);
    endTimer();
    log();
}