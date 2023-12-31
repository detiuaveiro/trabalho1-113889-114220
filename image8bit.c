/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 113889 Name: Hugo Castro
// NMec: 114220 Name: Rita Silva
//
//
// Date: 2023-11-14
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image
{
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8 *pixel; // pixel data (a raster scan)
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
//
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char *errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char *ImageErrMsg()
{ ///
  return errCause;
}

// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success =
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
//
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
//
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
//
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)

// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char *failmsg)
{
  errCause = (char *)(condition ? "" : failmsg);
  return condition;
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void)
{ ///
  InstrCalibrate();
  InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
  InstrName[1] = "pixcmp"; // InstrCount[1] will count pixel comparisons
  // Name other counters here....
  // InstrName[1] = "negative";  // Name for negative transformation
  // InstrName[2] = "threshold"; // Name for threshold transformation
  // InstrName[3] = "rotate";    // Name for rotate transformation
  // InstrName[4] = "mirror";    // Name for mirror transformation
  // InstrName[5] = "crop";      // Name for crop transformation
  // InstrName[6] = "paste";     // Name for paste operation
  // InstrName[7] = "blend";     // Name for blend operation
  // InstrName[8] = "matchsub";  // Name for match subimage operation
  // InstrName[10] = "locatesub"; // Name for locate subimage operation
  // InstrName[9] = "blur"; // Name for blur operation
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
#define PIXCMP InstrCount[1]
// #define NEGATIVE InstrCount[1]
// #define THRESHOLD InstrCount[2]
// #define ROTATE InstrCount[3]
// #define MIRROR InstrCount[4]
// #define CROP InstrCount[5]
// #define PASTE InstrCount[6]
// #define BLEND InstrCount[7]
// #define MATCHSUB InstrCount[8]
// #define LOCATESUB InstrCount[10]
// #define BLUR InstrCount[9]
// Add more macros here if needed...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval)
{ ///
  assert(width >= 0);
  assert(height >= 0);
  assert(0 < maxval && maxval <= PixMax);
  // Insert your code here!

  // 1. allocate memory for the image structure
  Image img = (Image)malloc(sizeof(struct image));
  if (img == NULL)
  {
    errCause = "Memory allocation failed";
    return NULL;
  }

  // 2. allocate memory for the pixel array
  img->pixel = (uint8 *)malloc(width * height * sizeof(uint8));
  if (img->pixel == NULL)
  {
    errCause = "Memory allocation failed";
    free(img);
    return NULL;
  }

  // 3. initialize the image structure fields
  img->width = width;
  img->height = height;
  img->maxval = maxval;

  // 4. return the new image
  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image *imgp)
{ ///
  assert(imgp != NULL);
  // Insert your code here!

  // 1. free the pixel array
  free((*imgp)->pixel);

  // 2. free the image structure
  free(*imgp);

  // 3. set the image pointer to NULL
  *imgp = NULL;
}

/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE *f)
{
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n')
  {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char *filename)
{ ///
  int w, h;
  int maxval;
  char c;
  FILE *f = NULL;
  Image img = NULL;

  int success =
      check((f = fopen(filename, "rb")) != NULL, "Open failed") &&
      // Parse PGM header
      check(fscanf(f, "P%c ", &c) == 1 && c == '5', "Invalid file format") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &h) == 1 && h >= 0, "Invalid height") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax, "Invalid maxval") &&
      check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected") &&
      // Allocate image
      (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
      // Read pixels
      check(fread(img->pixel, sizeof(uint8), w * h, f) == w * h, "Reading pixels");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses

  // Cleanup
  if (!success)
  {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL)
    fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char *filename)
{ ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE *f = NULL;

  int success =
      check((f = fopen(filename, "wb")) != NULL, "Open failed") &&
      check(fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed") &&
      check(fwrite(img->pixel, sizeof(uint8), w * h, f) == w * h, "Writing pixels failed");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses

  // Cleanup
  if (f != NULL)
    fclose(f);
  return success;
}

/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img)
{ ///
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img)
{ ///
  assert(img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img)
{ ///
  assert(img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8 *min, uint8 *max)
{ ///
  assert(img != NULL);
  // Insert your code here!

  // 1. initialize min and max
  *min = PixMax;
  *max = 0;

  // 2. find min and max
  for (int i = 0; i < img->width * img->height; i++)
  {
    if (img->pixel[i] < *min)
    {
      *min = img->pixel[i];
    }
    if (img->pixel[i] > *max)
    {
      *max = img->pixel[i];
    }
  }

  assert(*min <= *max);
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y)
{ ///
  assert(img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h)
{ ///
  assert(img != NULL);
  // Insert your code here!

  // 1. check if the rectangle is inside the image, by checking if the coordinates are valid and if the dimensions are valid
  if (x < 0 || y < 0 || x + w > img->width || y + h > img->height)
  {
    return 0;
  }
  return 1;
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel.
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y)
{
  int index;
  // Insert your code here!

  // 1. compute the index
  index = y * img->width + x;

  // 2. return the index
  assert(0 <= index && index < img->width * img->height);
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y)
{ ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1;
  return img->pixel[G(img, x, y)];
}

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level)
{ ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1;
  img->pixel[G(img, x, y)] = level;
}

/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.

/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img)
{ ///
  assert(img != NULL);
  // Insert your code here!

  // 1. transform each pixel
  for (int i = 0; i < img->width * img->height; i++)
  {
    img->pixel[i] = PixMax - img->pixel[i];
  }

  assert(ImageMaxval(img) == PixMax);
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr)
{ ///
  assert(img != NULL);
  // Insert your code here!

  // 1. transform each pixel
  for (int i = 0; i < img->width * img->height; i++)
  {
    if (img->pixel[i] < thr)
    {
      img->pixel[i] = 0;
    }
    else
    {
      img->pixel[i] = PixMax;
    }
  }

  assert(ImageMaxval(img) == PixMax);
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor)
{
  assert(img != NULL);
  assert(factor >= 0.0);

  // 1. transform each pixel
  for (int i = 0; i < img->width * img->height; i++)
  {
    // Use casting to ensure accurate calculations
    int newPixelValue = (int)(img->pixel[i] * factor + 0.5);

    // Saturate at PixMax
    img->pixel[i] = (uint8_t)(newPixelValue > PixMax ? PixMax : newPixelValue);
  }
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint:
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees clockwise.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img)
{ ///
  assert(img != NULL);
  // Insert your code here!
  int imgInHEight = ImageHeight(img);
  int imgInWidth = ImageWidth(img);
  Image imgReturn = ImageCreate(imgInHEight, imgInWidth, ImageMaxval(img));
  for (int y = 0; y < imgInHEight; y++)
  {
    for (int x = 0; x < imgInWidth; x++)
    {
      ImageSetPixel(imgReturn, y, imgInWidth - x - 1, ImageGetPixel(img, x, y));
    }
  }

  return imgReturn;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img)
{ ///
  assert(img != NULL);
  // Insert your code here!

  // 1. create a new image
  Image img2 = ImageCreate(img->width, img->height, img->maxval);

  // 2. mirror the pixels from the original image to the new image, from right to left
  for (int i = 0; i < img->height; i++)
  {
    for (int j = 0; j < img->width; j++)
      img2->pixel[i * img->width + (img->width - j - 1)] = img->pixel[i * img->width + j];
  }

  // 3. return the new image
  return img2;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h)
{ ///
  assert(img != NULL);
  assert(ImageValidRect(img, x, y, w, h));
  // Insert your code here!

  // Verificar se as coordenadas e dimensões do recorte são válidas
  if (!ImageValidRect(img, x, y, w, h))
  {
    fprintf(stderr, "Error: Invalid crop region.\n");
    // Pode retornar NULL ou outro valor de erro
    return NULL;
  }

  // 1. create a new image
  Image img2 = ImageCreate(w, h, img->maxval);

  // 2. copy the pixels from the original image to the new image
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
      img2->pixel[i * w + j] = img->pixel[(i + y) * img->width + (j + x)];
  }

  // 3. return the new image
  return img2;
}

/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!

  // 1. copy the pixels from img2 to img1
  for (int i = 0; i < img2->height; i++)
  {
    for (int j = 0; j < img2->width; j++)
      img1->pixel[(i + y) * img1->width + (j + x)] = img2->pixel[i * img2->width + j];
  }

  assert(ImageMaxval(img1) == ImageMaxval(img2));
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  uint8 level = 0;
  for (int yh = 0; yh < img2->height; yh++) // yh = y height
  {
    for (int xw = 0; xw < img2->width; xw++) // xw = x width
    {
      // level is the new pixel value after the blend. level = (1 - alpha) * img1 + alpha * img2
      level = (int)((1 - alpha) * ImageGetPixel(img1, x + xw, y + yh) +
                    (alpha)*ImageGetPixel(img2, xw, yh) + 0.5);
      //
      ImageSetPixel(img1, x + xw, y + yh, level);
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2)
{
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidPos(img1, x, y));

  //
  for (int i = 0; i < img2->height; i++)
  {
    for (int j = 0; j < img2->width; j++)
    {
      uint8 pixelSubImage = ImageGetPixel(img1, x + j, y + i);
      uint8 pixelImg2 = ImageGetPixel(img2, j, i);

      PIXCMP += 1;

      if (pixelSubImage != pixelImg2)
      {
        return 0; // the images don't match
      }
    }
  }
  return 1; // the images match
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int *px, int *py, Image img2)
{
  assert(img1 != NULL);
  assert(img2 != NULL);

  // Check if the dimensions of img2 are smaller than img1
  if (img2->width > img1->width || img2->height > img1->height)
  {
    return 0; // img2 is larger than img1, no match possible
  }

  // Iterate through img1 to find a match with img2
  for (int i = 0; i < img1->height - img2->height; i++)
  {
    for (int j = 0; j < img1->width - img2->width; j++)
    {
      if (ImageMatchSubImage(img1, j, i, img2))
      {
        // Match found, set the coordinates and return 1
        *px = j;
        *py = i;
        return 1;
      }
    }
  }

  // No match found
  return 0;
}
/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy)
{
  assert(img != NULL);
  int width = img->width;
  int height = img->height;

  // Temporary storage for the intermediate pixel values
  // double intermediatePixels[width * height];

  // Allocate memory for the intermediate pixel values
  double *intermediatePixels = (double *)malloc(width * height * sizeof(double));
  if (intermediatePixels == NULL)
  {
    // ERROR
    fprintf(stderr, "Falha na alocação de memória para intermediatePixels\n");
    return;
  }

  // Blur in the x-direction
  for (int y = 0; y < height; y++)
  {
    double total = 0.0;
    int count = 0;

    // Initialize the window for the first pixel
    for (int cx = 0; cx <= dx; cx++)
    {
      if (cx < width)
      {
        total += ImageGetPixel(img, cx, y);
        count++;
      }
    }

    // Process the first pixel in the row
    intermediatePixels[y * width] = total / count;

    // Update the window for subsequent pixels in the row
    for (int x = 1; x < width; x++)
    {
      // Remove the leftmost pixel from the previous window
      if (x - dx - 1 >= 0)
      {
        total -= ImageGetPixel(img, x - dx - 1, y);
        count--;
      }

      // Add the rightmost pixel to the window
      if (x + dx < width)
      {
        total += ImageGetPixel(img, x + dx, y);
        count++;
      }

      // Calculate the mean value
      intermediatePixels[y * width + x] = total / count;
    }
  }

  // Blur in the y-direction and update the image with the final pixel values
  for (int x = 0; x < width; x++)
  {
    double total = 0.0;
    int count = 0;

    // Initialize the window for the first pixel
    for (int cy = 0; cy <= dy; cy++)
    {
      total += intermediatePixels[cy * width + x];
      count++;
    }

    // Process the first pixel in the column
    int meanValue = (int)(total / count + 0.5);

    // Clamp the result to the valid range for uint8_t
    meanValue = (meanValue < 0) ? 0 : (meanValue > 255) ? 255
                                                        : meanValue;

    // Update the pixel value
    ImageSetPixel(img, x, 0, (uint8_t)meanValue);

    // Update the window for subsequent pixels in the column
    for (int y = 1; y < height; y++)
    {
      // Remove the topmost pixel from the previous window
      if (y - dy - 1 >= 0)
      {
        total -= intermediatePixels[(y - dy - 1) * width + x];
        count--;
      }

      // Add the bottommost pixel to the window
      if (y + dy < height)
      {
        total += intermediatePixels[(y + dy) * width + x];
        count++;
      }

      // Calculate the mean value
      meanValue = (int)(total / count + 0.5);

      // Clamp the result to the valid range for uint8_t
      meanValue = (meanValue < 0) ? 0 : (meanValue > 255) ? 255
                                                          : meanValue;

      // Update the pixel value
      ImageSetPixel(img, x, y, (uint8_t)meanValue);
    }
  }

  // Liberar a memória alocada dinamicamente quando não for mais necessária
  free(intermediatePixels);
}
