// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ImageObject.h>
#include <stdio.h>
#include <string.h>

#include <vtkImageData.h>
#include <vtkPNGWriter.h>
#include <vtkBMPWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkRGBWriter.h>
#include <vtkPNGWriter.h>

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
//   format   : <optional> The format to write.
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
//    Kathleen Biagas, Mon Jan 12 16:36:30 PST 2015
//    Add 'format' argument. Use VTK to write non-ppm formats.
//
// ****************************************************************************

bool
ImageObject::Write(const char *filename, const char *format)
{
    if(depth != 3)
        return false;


    bool retval = false;

    // Check the requested extension.
    if (format && strcmp(format, "ppm") != 0)
    {
        vtkImageWriter *writer = NULL;
        if(strcmp(format, "tiff") != 0)
        {
            writer = vtkTIFFWriter::New();
        }
        else if(strcmp(format, "jpeg") != 0)
        {
            writer = vtkJPEGWriter::New();
            // quality? progressive?
        }
        else if(strcmp(format, "bmp") != 0)
        {
            writer = vtkBMPWriter::New();
        }
        else if(strcmp(format, "rgb") != 0)
        {
            writer = vtkRGBWriter::New();
        }
        else if(strcmp(format, "png") != 0)
        {
            writer = vtkPNGWriter::New();
        }
        if (writer != NULL)
        {
            // create vtkImageData as input for vtkImageWriter
            vtkImageData *image = vtkImageData::New();
            image->SetDimensions(w,h,1);
            image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
            for (int x = 0; x < w; ++x)
            {
                for (int y = 0; y < h; ++y)
                {
                unsigned char *ip = static_cast<unsigned char*>(image->GetScalarPointer(x, h-y-1, 0));
                unsigned char *P = Pixel(x,y);
                ip[0] = P[0]; 
                ip[1] = P[1]; 
                ip[2] = P[2];
                }
            }
            writer->SetFileName(filename);
            writer->SetInputData(image);
            writer->Write();
            writer->Delete();
            image->Delete();
            retval = true;
        }
        else
        {
            retval = false;
        }
    }
    else
    {
        FILE *ppm = fopen(filename, "wb");
        retval = (ppm != NULL);
        if(ppm)
        {
            // Figure out the header and write it to the file.
            fprintf(ppm, "P6\n%d %d\n255\n", w, h);
            fwrite(pixels, 3 * w * h, 1, ppm);
            fclose(ppm);
        }
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
    char* result = NULL; (void) result;
    if(ppm)
    {
        char line[1000];
        // Figure out the header and write it to the file.
        result = fgets(line, 1000, ppm);
        result = fgets(line, 1000, ppm);
        if(line[0] == '#')
            result = fgets(line, 1000, ppm);
        sscanf(line, "%d %d\n", &w, &h);
        result = fgets(line, 1000, ppm);

        if(pixels != 0)
            delete [] pixels;
        depth = 3;
        pixels = new unsigned char[w * h * 3];
        size_t res = fread(pixels, 3 * w * h, 1, ppm); (void) res;
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
