// imageTest - A program that performs some image processing.
//
// This program is an example use of the image8bit module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// João Manuel Rodrigues <jmr@ua.pt>
// 2023

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

  // Try changing the behaviour of the program by commenting/uncommenting
  // the appropriate lines.

  // croppar a img1 para ser uma imagem com o tamanho de um pixel e esse pixel ser igual ao primeiro da img1
  Image img2 = ImageCrop(img1, 0, 0, 1, 1);
  int width1 = ImageWidth(img1);
  int height1 = ImageHeight(img1);
  


  if (ImageSave(img3, argv[2]) == 0)
  {
    error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  }

  // testar a ImageLocateSubImage com img1 e img2
  int px, py;
  InstrReset();
  ImageLocateSubImage(img1, &px, &py, img3);
  InstrPrint();

  // Image img2 = ImageLoad(argv[1]);

  // Test ImageBlur(Image img, int dx, int dy)
  // InstrReset();
  // ImageBlur(img2, 5, 5);
  // InstrPrint();

  // ImageNegative(img2);
  // ImageThreshold(img2, 100);
  // ImageBrighten(img2, 1.3);
  // ImageRotate(img2);

  // if (ImageSave(img2, argv[2]) == 0)
  // {
  //   error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  // }

  ImageDestroy(&img1);
  // ImageDestroy(&img2);
  return 0;
}