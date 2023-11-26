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
  // if (argc != 2)
  // {
  //     error(1, 0, "Usage: imageTest input.pgm output.pgm");
  // }

  ImageInit();

  printf("# LOAD image\n");
  InstrReset(); // to reset instrumentation
  // Image img1 = ImageLoad(argv[1]);
  // if (img1 == NULL)
  // {
  //   error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
  // }
  // InstrPrint(); // to print instrumentation

  // criar pgm 1 pixel:
  Image img2 = ImageCreate(1, 1, 255);
  // defini-lo todo preto:
  ImageSetPixel(img2, 0, 0, 0);

  Image img3 = ImageCreate(640, 480, 255);
  // definir tudo branco
  for (int i = 0; i < 640; i++)
  {
    for (int j = 0; j < 480; j++)
    {
      ImageSetPixel(img3, i, j, 255);
    }
  }

  // if (ImageSave(img2, argv[2]) == 0)
  // {
  //     error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  // }
  // if (ImageSave(img3, argv[2]) == 0)
  // {
  //     error(2, errno, "%s: %s", argv[2], ImageErrMsg());
  // }

  // testar a ImageLocateSubImage:
  InstrReset(); // to reset instrumentation

  int x, y;
  if (ImageLocateSubImage(img3, &x, &y, img2))
  {
    printf("# FOUND (%d,%d)\n", x, y);
  }
  else
  {
    printf("# NOTFOUND\n");
  }
  InstrPrint();
}
