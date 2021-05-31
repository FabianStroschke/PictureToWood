#include <Picture.hpp>
#include <Find_Patches.hpp>

char filename[] = "input/Target.jpg";
char filename2[] = "input/16_wood_samples.jpg";

int main( int argc, char ** argv ) {
    Picture target(filename);
    Picture sample(filename2);
    target.show();
    //sample.show();
    target.cutIntoSquares(40, CENTER);
    sample.cutIntoSquares(40, CENTER);
    auto list = findMatchingPatches(target.patches, sample.patches, compareGray);
    auto output = assambleOutput(list, target);

}