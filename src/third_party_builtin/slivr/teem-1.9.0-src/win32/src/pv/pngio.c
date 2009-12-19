/*
 * COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
 *
 * If you modify libpng you may insert additional notices immediately following
 * this sentence.
 *
 * Copyright 2002, Milan Ikits
 * Copyright 2000, Willem van Schaik
 *
 * libpng versions 1.0.7, July 1, 2000, through 1.2.5, October 3, 2002, are
 * Copyright (c) 2000-2002 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-1.0.6
 * with the following individuals added to the list of Contributing Authors
 *
 *    Simon-Pierre Cadieux
 *    Eric S. Raymond
 *    Gilles Vollant
 *
 * and with the following additions to the disclaimer:
 *
 *    There is no warranty against interference with your enjoyment of the
 *    library or against infringement.  There is no warranty that our
 *    efforts or the library will fulfill any of your particular purposes
 *    or needs.  This library is provided with all faults, and the entire
 *    risk of satisfactory quality, performance, accuracy, and effort is with
 *    the user.
 *
 * libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
 * Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson
 * Distributed according to the same disclaimer and license as libpng-0.96,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    Tom Lane
 *    Glenn Randers-Pehrson
 *    Willem van Schaik
 *
 * libpng versions 0.89, June 1996, through 0.96, May 1997, are
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Distributed according to the same disclaimer and license as libpng-0.88,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    John Bowler
 *    Kevin Bracey
 *    Sam Bushell
 *    Magnus Holmgren
 *    Greg Roelofs
 *    Tom Tanner
 *
 * libpng versions 0.5, May 1995, through 0.88, January 1996, are
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * For the purposes of this copyright and license, "Contributing Authors"
 * is defined as the following set of individuals:
 *
 *    Andreas Dilger
 *    Dave Martindale
 *    Guy Eric Schalnat
 *    Paul Schmidt
 *    Tim Wegner
 *
 * The PNG Reference Library is supplied "AS IS".  The Contributing Authors
 * and Group 42, Inc. disclaim all warranties, expressed or implied,
 * including, without limitation, the warranties of merchantability and of
 * fitness for any purpose.  The Contributing Authors and Group 42, Inc.
 * assume no liability for direct, indirect, incidental, special, exemplary,
 * or consequential damages, which may result from the use of the PNG
 * Reference Library, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented.
 *
 * 2. Altered versions must be plainly marked as such and
 * must not be misrepresented as being the original source.
 *
 * 3. This Copyright notice may not be removed or altered from
 *    any source or altered source distribution.
 *
 * The Contributing Authors and Group 42, Inc. specifically permit, without
 * fee, and encourage the use of this source code as a component to
 * supporting the PNG file format in commercial products.  If you use this
 * source code in a product, acknowledgment is not required but would be
 * appreciated.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <png.h>
// needed for stdin
#include <io.h>
#include <fcntl.h>
#include <pngio.h>

#ifdef PNG_NO_STDIO
static void _ReadDataPNG (png_structp png, png_bytep data, png_size_t length)
{
  png_size_t check;
  check = (png_size_t)fread(data, (png_size_t)1, length, (FILE*)png->io_ptr);
  if (check != length) png_error(png, "file read error");
}
#endif

static void _ErrorHandlerPNG (png_structp png, png_const_charp message)
{
  longjmp(png->jmpbuf, 1);
}

// PNG image handler functions
int LoadImagePNG (char* filename, char** image, int *width, int *height, int *channels)
{
  FILE *file;
  png_structp png = NULL;
  png_infop info = NULL;
  png_bytepp row = NULL;
  png_byte sig[8];
  png_uint_32 rowsize;
  int depth;
  int type;
  double gamma;
  int i;

  // open the PNG input file
  if (!filename)
  {
    *image = NULL;
    return 1;
  }
  if (!strcmp(filename, "-"))
  {
    file = stdin;
    _setmode(_fileno(file), _O_BINARY);
  }
  else if (!(file = fopen(filename, "rb")))
  {
    *image = NULL;
    return 1;
  }
  // first check the eight byte PNG signature
  fread(sig, 1, 8, file);
  if (!png_check_sig(sig, 8))
  {  
    *image = NULL;
    return 1;
  }
  /* create png struct with the error handlers above */
  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
			       (png_error_ptr)_ErrorHandlerPNG, (png_error_ptr)NULL);
  if (!png)
  {
    *image = NULL;
    return 1;
  }
  /* create image info struct */
  info = png_create_info_struct(png);
  if (!info)
  {
    png_destroy_read_struct(&png, NULL, NULL);
    *image = NULL;
    return 1;
  }
  /* set up png style error handling */
  if (setjmp(png->jmpbuf))
  {
    /* the error is reported inside the handler, 
       but we still need to clean up and return */
    png_destroy_read_struct(&png, &info, NULL);
    if (*image) free(*image);
    if (row) free(row);
    if (file != stdin) fclose(file);
    *image = NULL;
    return 1;
  }
  /* initialize png I/O */
#ifdef PNG_NO_STDIO
  png_set_read_fn(png, (png_voidp)file, _ReadDataPNG);
#else
  png_init_io(png, file);
#endif
  /* if we are here, we have already read 8 bytes from the file */
  png_set_sig_bytes(png, 8);
  /* png_read_info() returns all information from the file 
     before the first data chunk */
  png_read_info(png, info);
  /* query required image info */
  png_get_IHDR(png, info, width, height, &depth, &type, 
	       NULL, NULL, NULL);
  /* expand images of all color-type and bit-depth to 3x8 bit RGB images
     let the library process things like alpha, transparency, background */
  if (depth == 16) png_set_strip_16(png);
  /* expand paletted colors into rgb triplets */
  if (type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  /* expand grayscale images to 8 bits from 1, 2, or 4 bits */
  if (type == PNG_COLOR_TYPE_GRAY && depth < 8)
    png_set_gray_1_2_4_to_8(png);
  /* expand paletted or rgb images with transparency to full alpha
     channels so the data will be available as rgba quartets */
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  /* convert grayscale images to rgb */
  if (type == PNG_COLOR_TYPE_GRAY ||
      type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
  /* set gamma conversion */
  if (png_get_gAMA(png, info, &gamma))
    png_set_gamma(png, (double)2.2, gamma);
  /* after the transformations have been registered update info */
  png_read_update_info(png, info);
  /* get width, height again and the new bit-depth and color-type */
  png_get_IHDR(png, info, width, height, &depth,
	       &type, NULL, NULL, NULL);
  *channels = png_get_channels(png, info);
  /* row_bytes is the width x number of channels */
  rowsize = png_get_rowbytes(png, info);
  /* now we can allocate memory to store the image */
  if (*image)
  {
    free(*image); *image = NULL;
  }
  *image = (png_byte*)malloc(rowsize*(*height)*sizeof(png_byte));
  if (*image == NULL) png_error(png, "out of memory");
  /* set up row pointers */
  row = (png_bytepp)malloc((*height)*sizeof(png_bytep));
  if (row == NULL) png_error(png, "out of memory");
  for (i=0; i<(*height); i++)
    row[i] = &((png_bytep)(*image))[i*rowsize];
  /* read the whole image */
  png_read_image(png, row);
  /* finish reading */
  png_read_end(png, NULL);
  /* clean up */
  png_destroy_read_struct(&png, &info, NULL);
  free(row); row = NULL;
  if (file != stdin) fclose(file);
  /* return image data */
  return 0;
}
