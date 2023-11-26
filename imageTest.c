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

  // img2 = ImageCrop(img1, ImageWidth(img1)/4, ImageHeight(img1)/4, ImageWidth(img1)/2, ImageHeight(img1)/2);
  // Image img2 = ImageCrop(img1, 5, 78, ImageWidth(img1) / 2, ImageHeight(img1) / 3);

  // best case
  Image img2 = ImageCrop(img1, 0, 0, ImageWidth(img1), ImageHeight(img1));
  // worst case
  // Image img3 = ImageLoad(argv[2]);
  // copiar com crop uma foto exatamente igual à img 1 so que sera img3
  Image img3 = ImageCrop(img1, 0, 0, ImageWidth(img1), ImageHeight(img1));
  // mudar o ultimo pixel para preto
  ImageSetPixel(img3, ImageWidth(img3) - 1, ImageHeight(img3) - 1, 0);

  // test ImageLocateSubImage(Image img1, int *px, int *py, Image img2) para best e worst case
  // best case
  int px, py;
  InstrReset();
  ImageLocateSubImage(img1, &px, &py, img2);
  InstrPrint();
  printf("px = %d, py = %d\n", px, py);

  // worst case
  int px2, py2;
  InstrReset();
  ImageLocateSubImage(img1, &px2, &py2, img3);
  InstrPrint();
  printf("px = %d, py = %d\n", px2, py2);

  // InstrReset();
  // ImageLocateSubImage(img1, &px, &py, img3);
  // InstrPrint();
  // printf("px = %d, py = %d\n", px, py);

  // test ImageLocateSubImage(Image img1, int *px, int *py, Image img2)
  // int px, py;
  // InstrReset();
  // ImageLocateSubImage(img1, &px, &py, img2);
  // InstrPrint();
  // printf("px = %d, py = %d\n", px, py);

  // if (img2 == NULL)
  // {
  //   error(2, errno, "Rotating img2: %s", ImageErrMsg());
  // }

  // testar a função ImageLocateSubImage
  // printf("# Teste da função ImageLocateSubImage\n");

  // // criar uma imagem branca com o pixel ultimo pixel preto
  // Image branca = ImageCrop(img1, 0, 0, ImageWidth(img1), ImageHeight(img1));
  // ImageThreshold(branca, 0);

  // // ciclo para criar varias janelas e testar a função
  // for (int width = 1; width < ImageWidth(branca); width *= 2)
  // {
  //   int px, py;
  //   // criar uma janela para o melhor cenário
  //   Image subBest = ImageCrop(branca, 0, 0, width, width);

  //   // criar uma janela para o pior cenário
  //   Image subWorst = ImageCrop(branca, 0, 0, width, width);
  //   ImageSetPixel(subWorst, ImageWidth(subWorst) - 1, ImageHeight(subWorst) - 1, 0);

  //   InstrReset(); // to reset instrumentation
  //   printf("\n# IMAGELOCATESUBIMAGE BEST CASE (size: %d)\n", width);
  //   ImageLocateSubImage(branca, &px, &py, subBest);
  //   InstrPrint();

  //   InstrReset(); // to reset instrumentation
  //   printf("\n# IMAGELOCATESUBIMAGE WORST CASE (size: %d)\n", width);
  //   ImageLocateSubImage(branca, &px, &py, subWorst);
  //   InstrPrint();

  //   printf("\n");
  // }

  // Image img2 = ImageLoad(argv[1]);

  // Test ImageBlur(Image img, int dx, int dy)
  // ImageBlur(img2, 3, 3);
  // mais blur
  // InstrReset();
  // ImageBlur(img2, 5, 5);
  // InstrPrint();
  // ainda mais blured
  // InstrReset();
  // ImageBlur(img2, 7, 7);
  // InstrPrint();
  // InstrReset();
  // ImageBlur(img2, 15, 15);
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