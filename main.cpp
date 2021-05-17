#include <Picture.hpp>

char filename[] = "input/Target.jpg";
char filename2[] = "input/16_wood_samples.jpg";

int main( int argc, char ** argv ) {
    Picture target(filename);
    Picture sample(filename2);
    target.show();
    sample.show();

}