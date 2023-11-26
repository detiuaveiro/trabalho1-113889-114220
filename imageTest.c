#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"
#include "instrumentation.h"

int main(int argc, char *argv[])
{
    ImageInit();

    // criar pgm 1 pixel:
    Image img2 = ImageCreate(1, 1, 255);
    // defini-lo todo preto:
    ImageSetPixel(img2, 0, 0, 0);

    Image img3 = ImageCreate(640, 480, 255);
    // definir tudo branco
    ImageSetPixel(img3, 0, 0, 255);

    // testar a ImageLocateSubImage com img1 e img2 (worst case)
    int px, py;
    InstrReset();
    ImageLocateSubImage(img3, &px, &py, img2);
    InstrPrint();
}
