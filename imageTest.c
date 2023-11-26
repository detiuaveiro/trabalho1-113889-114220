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

  Image img1 = ImageLoad(argv[1]);
  // img2 é uma imagem cropped da img1
  Image img2 = ImageCrop(img1, 320, 0, 320, 480); // Correção aqui

  ImageSave(img2, "img2.pgm");
  // testar a ImageLocateSubImage com img1 e img2
  int px, py;
  InstrReset();
  ImageLocateSubImage(img1, &px, &py, img2);
  InstrPrint();
  printf("px = %d, py = %d\n", px, py);
}