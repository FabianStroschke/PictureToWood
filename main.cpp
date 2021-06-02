#include <Picture.hpp>
#include <Find_Patches.hpp>
#include <Time_Measure.hpp>

char filename[] = "input/Target.jpg";
char filename2[] = "input/16_wood_samples.jpg";

int main( int argc, char ** argv ) {
    Picture target(filename);
    Picture sample(filename2);
    target.show();
    //sample.show();
    target.cutIntoSquares(20, CENTER);
    sample.cutIntoSquares(20, CENTER);
    startTimer();
    auto list = findMatchingPatches(target.patches, sample, compareGray);
    auto output = assembleOutput(list, target);
    endTimer();
    log();
}