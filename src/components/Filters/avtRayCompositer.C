/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

// ************************************************************************* //
//                           avtRayCompositer.C                              //
// ************************************************************************* //

#include <avtRayCompositer.h>

#include <vtkImageData.h>

#include <avtImage.h>
#include <avtRayFunction.h>
#include <avtSamplePoints.h>
#include <avtVolume.h>

#include <ImproperUseException.h>


//
// Function Prototypes
//

static void RCPixelProgressCallback(void *, int, int);


// ****************************************************************************
//  Method: avtRayCompositer constructor
//
//  Arguments:
//      rf       The ray function the compositer should use to find the
//               intensity and variable values.
//     
//  Programmer:  Hank Childs
//  Creation:    December 4, 2000
//
//  Modifications:
//     Brad Whitlock, Wed Dec 5 10:43:13 PDT 2001
//     I added code to initialize the background mode, gradient background.
//
// ****************************************************************************

avtRayCompositer::avtRayCompositer(avtRayFunction *rf)
{
    rayfoo    = rf;
    unsigned char b[3];
    b[0] = b[1] = b[2] = 255;
    SetBackgroundColor(b);
    backgroundMode = BACKGROUND_SOLID;
    gradBG1[0] = 0.;
    gradBG1[1] = 0.;
    gradBG1[2] = 1.;
    gradBG2[0] = 0.;
    gradBG2[1] = 0.;
    gradBG2[2] = 0.;
    opaqueImage = NULL;
}


// ****************************************************************************
//  Method: avtRayCompositer::SetBackgroundColor
//
//  Purpose:
//      Sets the background color of the output image.
//
//  Arguments:
//      b       The new background color.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtRayCompositer::SetBackgroundColor(const unsigned char b[3])
{
    background[0] = b[0];
    background[1] = b[1];
    background[2] = b[2];
}

// ****************************************************************************
// Method: avtRayCompositer::SetBackgroundMode
//
// Purpose: 
//   Sets the background mode.
//
// Arguments:
//   mode : The new background mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 5 10:40:25 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtRayCompositer::SetBackgroundMode(int mode)
{
    if(mode >= BACKGROUND_SOLID && mode <= BACKGROUND_GRADIENT_RADIAL)
        backgroundMode = mode;
}

// ****************************************************************************
// Method: avtRayCompositer::SetGradientBackgroundColors
//
// Purpose: 
//   Sets the gradient background colors.
//
// Arguments:
//   bg1 : An array containing the first gradient background color.
//   bg2 : An array containing the second gradient background color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 5 10:40:51 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtRayCompositer::SetGradientBackgroundColors(const double bg1[3], const double bg2[3])
{
    gradBG1[0] = bg1[0];
    gradBG1[1] = bg1[1];
    gradBG1[2] = bg1[2];
    gradBG2[0] = bg2[0];
    gradBG2[1] = bg2[1];
    gradBG2[2] = bg2[2];
}

// ****************************************************************************
//  Method: avtRayCompositer::InsertOpaqueImage
//
//  Purpose:
//      Inserts an opaque image into the middle of the volume rendering.  The
//      image must have a zbuffer.
//
//  Arguments:
//      img     The image to be placed in the middle of the volume rendering.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
// ****************************************************************************

void
avtRayCompositer::InsertOpaqueImage(avtImage_p img)
{
    avtImageRepresentation &rep = img->GetImage();
    if (rep.GetZBuffer() == NULL)
    {
        //
        // How can we incorporate an image if it has no zbuffer?
        //
        EXCEPTION0(ImproperUseException);
    }

    opaqueImage = img;
}

// ****************************************************************************
//  Method: avtRayCompositer::FillBackground
//
//  Purpose: 
//      Draws the background color into the image data array and sets all
//      Z-values to 1.
//
//  Arguments:
//      data      A pointer to the image's pixels stored RGBRGBRGB...
//      zbuffer   A pointer to the image's z-buffer.
//      width     The width of the image.
//      height    The height of the image.
//
//  Programmer: Brad Whitlock
//  Creation:   December 4, 2001
//
//  Modifications:
//   
//    Hank Childs, Tue Jan  1 14:37:13 PST 2002
//    Removed zbuffer argument.
//
// ****************************************************************************

void
avtRayCompositer::FillBackground(unsigned char *data, int width, int height)
{
#define INTERPOLATECOLOR(t, omt, a, b) \
    (unsigned char)((((a) * (omt)) + ((b) * (t))) * 255)

    unsigned char *pixptr = data;

    if(backgroundMode == BACKGROUND_SOLID)
    {
        for(int j = 0; j < height; ++j)
        {
            for(int i = 0; i < width; ++i)
            {
                *pixptr++ = background[0];
                *pixptr++ = background[1];
                *pixptr++ = background[2];
            }
        }
    }
    else if(backgroundMode == BACKGROUND_GRADIENT_RADIAL)
    {
        // Draw the radial gradient background
        DrawRadialGradient(data, width, height);
    }
    else
    {
        // Set the order of the gradient colors based on the mode.
        double bg1[3], bg2[3];
        if(backgroundMode == BACKGROUND_GRADIENT_TB ||
           backgroundMode == BACKGROUND_GRADIENT_RL)
        {
            bg1[0] = gradBG2[0];
            bg1[1] = gradBG2[1];
            bg1[2] = gradBG2[2];
            bg2[0] = gradBG1[0];
            bg2[1] = gradBG1[1];
            bg2[2] = gradBG1[2];
        }
        else
        {
            bg1[0] = gradBG1[0];
            bg1[1] = gradBG1[1];
            bg1[2] = gradBG1[2];
            bg2[0] = gradBG2[0];
            bg2[1] = gradBG2[1];
            bg2[2] = gradBG2[2];
        }

        if(backgroundMode == BACKGROUND_GRADIENT_TB ||
           backgroundMode == BACKGROUND_GRADIENT_BT)
        {
            float invHeight = 1. / float(height - 1);
            for(int j = 0; j < height; ++j)
            {
                // Figure out the color for the row.
                float t = float(j) * invHeight;
                float omt = 1. - t;
                unsigned char bgR = INTERPOLATECOLOR(t, omt, bg1[0], bg2[0]);
                unsigned char bgG = INTERPOLATECOLOR(t, omt, bg1[1], bg2[1]);
                unsigned char bgB = INTERPOLATECOLOR(t, omt, bg1[2], bg2[2]);

                for(int i = 0; i < width; ++i)
                {
                    *pixptr++ = bgR;
                    *pixptr++ = bgG;
                    *pixptr++ = bgB;
                }
            }
        }
        else // Colors are going horizontal.
        {
            // Calculate the colors for the first row
            float invWidth = 1. / float(width - 1);
            for(int i = 0; i < width; ++i)
            {
                // Figure out the color for the column.
                float t = float(i) * invWidth;
                float omt = 1. - t;
                *pixptr++ = INTERPOLATECOLOR(t, omt, bg1[0], bg2[0]);
                *pixptr++ = INTERPOLATECOLOR(t, omt, bg1[1], bg2[1]);
                *pixptr++ = INTERPOLATECOLOR(t, omt, bg1[2], bg2[2]);
            }

            // Duplicate the row height-1 times.
            for(int j = 1; j < height; ++j)
            {
                unsigned char *p = data;
                for(int i = 0; i < width; ++i)
                {
                    *pixptr++ = *p++;
                    *pixptr++ = *p++;
                    *pixptr++ = *p++;
                }
            }
        }
    }
}

// ****************************************************************************
// Method: avtRayCompositer::DrawRadialGradient
//
// Purpose: 
//   This is a helper method for the FillBackground method that draws a 
//   radial gradient background into the image.
//
// Arguments:
//   data : A pointer to the image pixels.
//   w    : The image width.
//   h    : The image height.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 4 17:22:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtRayCompositer::DrawRadialGradient(unsigned char *data, int w, int h)
{
#define MACROPUTPIXEL(x, y) if( ((x)>=0 && (x)<w) && ((y)>=0 && (y)<h) ) \
    { \
        pixptr = data + ((((y)*w) + (x)) * 3); \
        *pixptr++ = c[0]; \
        *pixptr++ = c[1]; \
        *pixptr = c[2]; \
    }

    int i, j;
    int w2 = w >> 1;
    int h2 = h >> 1;
    float invR = 1. / ((sqrt((w*w*0.25) + (h*h*0.25)) * 1.02));
    unsigned char c[3], *pixptr;
    
    float dXdY = float(w) / float(h);
    float dYdX = float(h) / float(w);

    for(j = 0; j <= h2; ++j)
    {
        int x0 = int(dXdY * j);
        for(i = x0; i <= w2; ++i)
        {
            int dX = w2 - i; //x0 - i;
            int dY = h2 - j;
            float dist = sqrt(float(dX*dX + dY*dY));
            float t = dist * invR;
            float omt = 1. - t;

            c[0] = INTERPOLATECOLOR(t, omt, gradBG1[0], gradBG2[0]);
            c[1] = INTERPOLATECOLOR(t, omt, gradBG1[1], gradBG2[1]);
            c[2] = INTERPOLATECOLOR(t, omt, gradBG1[2], gradBG2[2]);

            MACROPUTPIXEL(i, j);
            MACROPUTPIXEL(w - i, j);
            MACROPUTPIXEL(i, h - j);
            MACROPUTPIXEL(w - i, h - j);
        }
    }

    for(j = 0; j <= w2; ++j)
    {
        int y0 = int(dYdX * j);
        for(i = y0; i <= h2; ++i)
        {
            int dY = h2 - i;
            int dX = w2 - j;
            float dist = sqrt(float(dX*dX + dY*dY));
            float t = dist * invR;
            float omt = 1. - t;

            c[0] = INTERPOLATECOLOR(t, omt, gradBG1[0], gradBG2[0]);
            c[1] = INTERPOLATECOLOR(t, omt, gradBG1[1], gradBG2[1]);
            c[2] = INTERPOLATECOLOR(t, omt, gradBG1[2], gradBG2[2]);

            MACROPUTPIXEL(j, i);
            MACROPUTPIXEL(w - j, i);
            MACROPUTPIXEL(j, h - i);
            MACROPUTPIXEL(w - j, h - i);
        }
    }
}

// ****************************************************************************
//  Method: avtRayCompositer::Execute
//
//  Purpose:
//      Creates the output image by compositing the rays into pixels (the
//      compositing is actually done by avtVolume).
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jan 25 23:42:20 PST 2001
//    Removed section to create vtkImageData so code could be consolidated in
//    avtImageRepresentation.
//
//    Hank Childs, Mon Jan 29 20:43:38 PST 2001
//    Use the restricted portion of the screen, rather than the whole thing.
//
//    Hank Childs, Sat Feb  3 20:17:20 PST 2001
//    Pulled out pixelizers.
//
//    Brad Whitlock, Wed Dec 5 10:44:31 PDT 2001
//    Modified to support gradient backgrounds.
//
//    Hank Childs, Thu Jan  3 09:50:29 PST 2002
//    Account for case where our partition contains nothing.
//
//    Hank Childs, Wed Jan 25 12:23:59 PST 2006
//    Add error checking.
//
// ****************************************************************************

void
avtRayCompositer::Execute(void)
{
    int  i, j;
    avtVolume *volume = GetTypedInput()->GetVolume();
    if (volume == NULL)
    {
        // This comes up in the following scenario:
        // An internal error occurs in the sampling phase.  An exception is
        // thrown.  That exception causes avtSamplePoints::SetVolume to be
        // not called.  When its not called, its data member "volume" is
        // not initialized.  So we get a NULL here.
        //
        // So: in summary, we only get into this situation if there was an
        // error before this module was called.
        EXCEPTION0(ImproperUseException);
    }

    //
    // Determine the size of the screen.
    //
    int  height = volume->GetRestrictedVolumeHeight();
    int  width  = volume->GetRestrictedVolumeWidth();

    //
    // This is a test to determine if there is nothing in the partition we
    // are supposed to composite -- since we don't have access to the 
    // partition, this is a bit of a hack and assumes how the partitioning
    // is done.
    // 
    if (volume->GetRestrictedMinHeight() >= volume->GetVolumeHeight() ||
        height <= 0 || width <= 0)
    {
        SetOutputImage(NULL);
        return;
    }

    volume->SetProgressCallback(RCPixelProgressCallback, this);

    //
    // Create an image that we can place each pixel into.
    //
    vtkImageData *image = avtImageRepresentation::NewImage(width, height);

    //
    // Populate an initial image, either with the background or with an
    // opaque image that is to be inserted into the middle of the rendering.
    //
    unsigned char *data = (unsigned char *)image->GetScalarPointer(0, 0, 0);
    int nPixels = width*height;
    float *zbuffer = new float[nPixels];
    for (i = 0 ; i < nPixels ; i++)
    {
        zbuffer[i] = 1.;
    }

    //
    // Draw the initial background into the image.
    //
    int fullHeight = volume->GetVolumeHeight();
    int fullWidth  = volume->GetVolumeWidth();
    vtkImageData *fullImage = avtImageRepresentation::NewImage(fullWidth,
                                                               fullHeight);
    unsigned char *fulldata = (unsigned char *) 
                                          fullImage->GetScalarPointer(0, 0, 0);
    FillBackground(fulldata, fullWidth, fullHeight);

    //
    // Now that we have the background in the full image, copy it into what
    // we need for this image.
    //
    int minWidth  = volume->GetRestrictedMinWidth();
    int minHeight = volume->GetRestrictedMinHeight();
    for (i = 0 ; i < nPixels ; i++)
    {
        int restrictedWidth  = i % width;
        int restrictedHeight = i / width;
        int realWidth  = restrictedWidth + minWidth;
        int realHeight = restrictedHeight + minHeight;
        int indexIntoFullData = realHeight*fullWidth + realWidth;
        for (j = 0 ; j < 3 ; j++)
        {
            data[3*i+j] = fulldata[3*indexIntoFullData+j];
        }
    }

    //
    // We were given an opaque image to insert into the picture.  Worse,
    // the image probably has different dimensions if we are running in
    // parallel.  This is captured by the notion of a restricted volume.
    //
    if (*opaqueImage != NULL)
    {
        int minW = volume->GetRestrictedMinWidth();
        int minH = volume->GetRestrictedMinHeight();
        vtkImageData  *opaqueImageVTK = opaqueImage->GetImage().GetImageVTK();
        float         *opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();
        unsigned char *opaqueImageData =
                    (unsigned char *)opaqueImageVTK->GetScalarPointer(0, 0, 0);

        for (int i = 0 ; i < width ; i++)
        {
            for (int j = 0 ; j < height ; j++)
            {
                 int index = j*width + i;
                 int opaqueImageIndex = (j+minH)*fullWidth + (i+minW);
                 zbuffer[index] = opaqueImageZB[opaqueImageIndex];
                 if (zbuffer[index] != 1.)
                 {
                     data[3*index    ] = opaqueImageData[3*opaqueImageIndex];
                     data[3*index + 1] = opaqueImageData[3*opaqueImageIndex+1];
                     data[3*index + 2] = opaqueImageData[3*opaqueImageIndex+2];
                 }
            }
        }
    }
    
    //
    // Have the volume cast our ray function on all of its valid rays and put
    // the output in this screen.  There is a lot of work here.
    //
    volume->GetPixels(rayfoo, data, zbuffer);

    //
    // Tell our output what its new image is.
    //
    SetOutputImage(image);

    //
    // Clean up memory.
    //
    image->Delete();
    fullImage->Delete();
    delete [] zbuffer;
}


// ****************************************************************************
//  Method: avtRayCompositer::UpdateCompositeProgress
//
//  Purpose:
//      A public method that allows for progress routines to be accessed.
//
//  Arguments:
//      amount  The amount of work done.
//      total   The total amount of work to be done.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2001
//
// ****************************************************************************

void
avtRayCompositer::UpdateCompositeProgress(int amount, int total)
{
    UpdateProgress(amount, total);
}


// ****************************************************************************
//  Function: RCPixelProgressCallback
//
//  Purpose:
//      A pixel progress callback that assumes the void * argument is a 
//      ray compositer.
//
//  Arguments:
//      rc      The ray compositer.
//      amount  The amount of work done.
//      total   The total amount of work to be done.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2001
//
// ****************************************************************************

static void
RCPixelProgressCallback(void *rc, int amount, int total)
{
    avtRayCompositer *typeRC = (avtRayCompositer *) rc;
    typeRC->UpdateCompositeProgress(amount, total);
}


