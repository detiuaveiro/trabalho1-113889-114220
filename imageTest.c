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
    if (argc != 3)
    {
        error(1, 0, "Usage: imageTest input.pgm output.pgm");
    }

    ImageInit();

    printf("# LOAD image");
    InstrReset(); // to reset instrumentation
    Image img1 = ImageLoad(argv[1]);
    if (img1 == NULL)
    {
        error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
    }
    InstrPrint(); // to print instrumentation

    // croppar a img1 para ser uma imagem com o tamanho de um pixel e esse pixel ser igual ao primeiro da img1
    Image img2 = ImageCrop(img1, 0, 0, 1, 1);

    if (ImageSave(img2, argv[2]) == 0)
    {
        error(2, errno, "%s: %s", argv[2], ImageErrMsg());
    }

    // testar a ImageLocateSubImage com img1 e img2
    int px, py;
    InstrReset();
    ImageLocateSubImage(img1, &px, &py, img2);
    InstrPrint();
}
