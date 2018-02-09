#include <OpenGP/Image/Image.h>
using namespace OpenGP;


int main(int, char**){

    Image<Vec4> I;
    imread("dice.png", I);

    Image<Vec4> I2 = I.transpose();

    imshow(I2);

    imwrite("dice2.png", I2);

    return EXIT_SUCCESS;
}
