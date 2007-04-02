/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <ImageObject.h>
#include <stdio.h>
#include <string.h>

// ****************************************************************************
// Method: ImageObject::ImageObject
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:36:48 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ImageObject::ImageObject()
{
    pixels = 0;
    w = h = 0;
    depth = 3;
}

ImageObject::ImageObject(int width, int height)
{
    w = width;
    h = height;
    depth = 3;
    pixels = new unsigned char[w * h * depth];
    memset(pixels, 0, w * h * depth);        
}

ImageObject::ImageObject(int width, int height, int d)
{
    w = width;
    h = height;
    depth = d;
    pixels = new unsigned char[w * h * depth];
    memset(pixels, 0, w * h * depth);        
}

ImageObject::ImageObject(const ImageObject &img)
{
    pixels = 0;
    w = h = 0;
    depth = 0;
    Copy(img);
}

// ****************************************************************************
// Method: ImageObject::~ImageObject
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:37:03 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ImageObject::~ImageObject()
{
    if(pixels)
        delete [] pixels;
}

// ****************************************************************************
// Method: ImageObject::Write
//
// Purpose: 
//   Writes the pixels to an image file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Returns:    True on success; False otherwise.
//
// Note:       This method could be rewritten to use VTK image writers if
//             we ever want to do more than ppm.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 09:03:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ImageObject::Write(const char *filename)
{
    if(depth != 3)
        return false;

    // Check the extension.
    bool retval = true;
    int len = strlen(filename);
    int writeType = 0;
    if(len > 4)
    {
        const char *ext = filename + len - 4;
        if(strcmp(ext, ".ppm") == 0)
            writeType = 1;
    }

    FILE *ppm = fopen(filename, "wb");
    retval = (ppm != NULL);
    if(ppm)
    {
        // Figure out the header and write it to the file.
        fprintf(ppm, "P6\n%d %d\n255\n", w, h);
        fwrite(pixels, 3 * w * h, 1, ppm);
        fclose(ppm);
    }

    return retval;
}

// ****************************************************************************
// Method: ImageObject::Read
//
// Purpose: 
//   Reads the pixels from an image file.
//
// Arguments:
//   filename : The name of the file to read.
//
// Returns:    True on success; False otherwise.
//
// Note:       This method could be rewritten to use VTK image writers if
//             we ever want to do more than ppm.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 09:03:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ImageObject::Read(const char *filename)
{
    FILE *ppm = fopen(filename, "rb");
    bool retval = (ppm != NULL);
    if(ppm)
    {
        char line[1000];
        // Figure out the header and write it to the file.
        fgets(line, 1000, ppm);
        fgets(line, 1000, ppm);
        if(line[0] == '#')
            fgets(line, 1000, ppm);
        sscanf(line, "%d %d\n", &w, &h);
        fgets(line, 1000, ppm);

        if(pixels != 0)
            delete [] pixels;
        depth = 3;
        pixels = new unsigned char[w * h * 3];
        fread(pixels, 3 * w * h, 1, ppm);
        fclose(ppm);
    }
    return retval;
}

// ****************************************************************************
// Method: ImageObject::SetAllPixelsToColor
//
// Purpose: 
//   Sets the color of all pixels.
//
// Arguments:
//   rgba : The color to use
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:37:23 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ImageObject::SetAllPixelsToColor(const int *rgba)
{
    if(pixels != 0)
    {
        unsigned char *final = pixels + (depth * w * h);
        unsigned char *ptr = pixels;
        if(depth == 1)
        {
            unsigned char color = (unsigned char)rgba[0];

            while(ptr < final)
                *ptr++ = color;
        }
        else if(depth == 2)
        {
            unsigned char color[2];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];

            while(ptr < final)
            {
                *ptr++ = color[0];
                *ptr++ = color[1];
            }
        }
        else if(depth == 3)
        {
            unsigned char color[3];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];
            color[2] = (unsigned char)rgba[2];
            while(ptr < final)
            {
                *ptr++ = color[0];
                *ptr++ = color[1];
                *ptr++ = color[2];
            }
        }
        else if(depth == 4)
        {
            unsigned char color[4];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];
            color[2] = (unsigned char)rgba[2];
            color[3] = (unsigned char)rgba[3];
            while(ptr < final)
            {
                *ptr++ = color[0];
                *ptr++ = color[1];
                *ptr++ = color[2];
                *ptr++ = color[3];
            }
        }
    }
}

// ****************************************************************************
// Method: ImageObject::SetBlockToColor
//
// Purpose: 
//   Sets a block of the image to the specified color.
//
// Arguments:
//   x0 : the min x value.
//   y0 : the min y value.
//   x1 : the max x value.
//   y1 : the max y value.
//   rgba : The color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:38:15 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ImageObject::SetBlockToColor(int x0, int y0, int x1, int y1,
    const int *rgba)
{
    if(pixels != 0)
    {
        if(depth == 1)
        {
            unsigned char color = (unsigned char)rgba[0];

#define BEGIN_COPY_PIXEL_BLOCK \
            for(int y = y0; y < y1; ++y)\
            {\
                for(int x = x0; x < x1; ++x)\
                {\
                    if(x >= 0 && x < Width() &&\
                       y >= 0 && y < Height())\
                    {\
                        unsigned char *ptr = Pixel(x, y);\

#define END_COPY_PIXEL_BLOCK \
                    }\
                }\
            }

            BEGIN_COPY_PIXEL_BLOCK
                *ptr = color;
            END_COPY_PIXEL_BLOCK
        }
        else if(depth == 2)
        {
            unsigned char color[2];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];

            BEGIN_COPY_PIXEL_BLOCK
                *ptr++ = color[0];
                *ptr++ = color[1];
            END_COPY_PIXEL_BLOCK
        }
        else if(depth == 3)
        {
            unsigned char color[3];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];
            color[2] = (unsigned char)rgba[2];
            BEGIN_COPY_PIXEL_BLOCK
                *ptr++ = color[0];
                *ptr++ = color[1];
                *ptr++ = color[2];
            END_COPY_PIXEL_BLOCK
        }
        else if(depth == 4)
        {
            unsigned char color[4];
            color[0] = (unsigned char)rgba[0];
            color[1] = (unsigned char)rgba[1];
            color[2] = (unsigned char)rgba[2];
            color[3] = (unsigned char)rgba[3];
            BEGIN_COPY_PIXEL_BLOCK
                *ptr++ = color[0];
                *ptr++ = color[1];
                *ptr++ = color[2];
                *ptr++ = color[3];
            END_COPY_PIXEL_BLOCK
        }
    }
}

// ****************************************************************************
// Method: ImageObject::Copy
//
// Purpose: 
//   Copy an image into the current object.
//
// Arguments:
//   img : The image to copy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:39:17 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ImageObject::Copy(const ImageObject &img)
{
    if(pixels)
        delete [] pixels;

    w = img.Width();
    h = img.Height();
    depth = img.Depth();
    pixels = new unsigned char[w * h * depth];
    memcpy(pixels, img.pixels, w * h * depth);
}

// ****************************************************************************
// Method: ImageObject::CopyBlock
//
// Purpose: 
//   Copy a block from one image into the current image.
//
// Arguments:
//   img : The image to copy.
//   x0 : the min x value.
//   y0 : the min y value.
//   x1 : the max x value.
//   y1 : the max y value.
//   destX : The destination x value in the current image.
//   destY : The destination y value in the current image.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:39:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ImageObject::CopyBlock(const ImageObject &img, int x0, int y0, int w0, int h0,
    int destX, int destY)
{
    if(img.depth != depth)
        return;

    int yi = 0;
    for(int y = y0; y < y0 + h0; ++y, ++yi)
    {
        int xi = 0;
        for(int x = x0; x < x0 + w0; ++x, ++xi)
        {
            int X = destX + xi;
            int Y = destY + yi;
            if(x >= 0 && x < img.Width() &&
               y >= 0 && y < img.Height() &&
               X >= 0 && X < Width() &&
               Y >= 0 && Y < Height())
            {
                const unsigned char *src = img.Pixel(x, y);
                unsigned char *dest = Pixel(X, Y);
                unsigned char *final = dest + depth;
                while(dest < final)
                    *dest++ = *src++;
            }
        } 
    }
}
