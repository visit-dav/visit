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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <snprintf.h>
#include <ImageObject.h>

#include <set>

#define TRANSITION_FADE     0
#define TRANSITION_LR_WIPE  1
#define TRANSITION_RL_WIPE  2
#define TRANSITION_BT_WIPE  3
#define TRANSITION_TB_WIPE  4
#define TRANSITION_LR_SLIDE 5
#define TRANSITION_RL_SLIDE 6
#define TRANSITION_BT_SLIDE 7
#define TRANSITION_TB_SLIDE 8
#define TRANSITION_CIRCLE   9
#define TRANSITION_RIPPLE   10
#define TRANSITION_BLOCK    11

#define COPY_RGB(dest, src) {\
    *dest++ = *src++;\
    *dest++ = *src++;\
    *dest = *src;\
    }

static const char *styleNames[] = {"fade", "lrwipe", "rlwipe", "btwipe", "tbwipe",
"lrslide", "rlslide", "btslide", "tbslide",
"circle", "ripple", "block"};


// ****************************************************************************
// Class: TransitionOptions
//
// Purpose:
//   Contains the command line arguments.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:51:29 PST 2006
//
// Modifications:
//   
// ****************************************************************************

struct TransitionOptions
{
    TransitionOptions()
    {
        transition_style = TRANSITION_FADE;
        color1[0] = 0;
        color1[1] = 0;
        color1[2] = 0;
        color2[0] = 255;
        color2[1] = 255;
        color2[2] = 255;
        strcpy(input1, "file1.ppm");
        strcpy(input2, "file2.ppm");
        strcpy(output, "output%04d.ppm");
        input1_uses_file = false;
        input2_uses_file = false;
        verbose = false;
        nFrames = 10;
    }

    int transition_style;
    int color1[3];
    int color2[3];
    char input1[500];
    char input2[500];
    char output[500];
    bool input1_uses_file;
    bool input2_uses_file;
    bool verbose;
    int nFrames;
};

// ****************************************************************************
// Method: BlendRGB
//
// Purpose: 
//   Blends 2 colors.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:42 PST 2006
//
// Modifications:
//   
// ****************************************************************************

inline void
BlendRGB(unsigned char *dest, const unsigned char *src1, const unsigned char *src2, const float t)
{
    float r0 = float(src1[0]);
    float g0 = float(src1[1]);
    float b0 = float(src1[2]);
    float r1 = float(src2[0]);
    float g1 = float(src2[1]);
    float b1 = float(src2[2]);

    float r = (1. - t) * r0 + t * r1;
    float g = (1. - t) * g0 + t * g1;
    float b = (1. - t) * b0 + t * b1;

    dest[0] = (unsigned char)((int)r);
    dest[1] = (unsigned char)((int)g);
    dest[2] = (unsigned char)((int)b);
}

// ****************************************************************************
// Function: Transition_Fade
//
// Purpose: 
//   Creates a transition that blends one image into another.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_Fade(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;

        unsigned char *src1 = image1->Pixel(0,0);
        unsigned char *src2 = image2->Pixel(0,0);
        unsigned char *dest = output.Pixel(0,0);
 
        unsigned int npix = image1->Width() * image1->Height();
        for(unsigned int p = 0; p < npix; ++p)
        {
            BlendRGB(dest, src1, src2, t);
            dest += 3;
            src1 += 3;
            src2 += 3;
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_LR_Wipe
//
// Purpose: 
//   Creates a transition that does a left to right wipe.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_LR_Wipe(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float xsize = float(image1->Width()) / float(options.nFrames - 1);

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        int currentX = int(float(frame) * xsize);

        for(int y = 0; y < image1->Height(); ++y)
        {
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = image1->Pixel(x, y);
                unsigned char *src2 = image2->Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                if(frame == 0)
                    COPY_RGB(dest, src1)
                else if(frame == options.nFrames-1)
                    COPY_RGB(dest, src2)
                else
                {
                    if(x < currentX)
                        COPY_RGB(dest, src2)
                    else
                        COPY_RGB(dest, src1)
                }
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_RL_Wipe
//
// Purpose: 
//   Creates a transition that does a right to left wipe.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_RL_Wipe(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float xsize = float(image1->Width()) / float(options.nFrames - 1);

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        int currentX = int(float(options.nFrames-1-frame) * xsize);

        for(int y = 0; y < image1->Height(); ++y)
        {
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = image1->Pixel(x, y);
                unsigned char *src2 = image2->Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                if(frame == 0)
                    COPY_RGB(dest, src1)
                else if(frame == options.nFrames-1)
                    COPY_RGB(dest, src2)
                else
                {
                    if(x > currentX)
                        COPY_RGB(dest, src2)
                    else
                        COPY_RGB(dest, src1)
                }
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_BT_Wipe
//
// Purpose: 
//   Creates a transition that bottom to top wipe.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_BT_Wipe(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float ysize = float(image1->Height()) / float(options.nFrames - 1);

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        int currentY = int(float(options.nFrames-1-frame) * ysize);

        for(int y = 0; y < image1->Height(); ++y)
        {
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = image1->Pixel(x, y);
                unsigned char *src2 = image2->Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                if(frame == 0)
                    COPY_RGB(dest, src1)
                else if(frame == options.nFrames-1)
                    COPY_RGB(dest, src2)
                else
                {
                    if(y > currentY)
                        COPY_RGB(dest, src2)
                    else
                        COPY_RGB(dest, src1)
                }
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_TB_Wipe
//
// Purpose: 
//   Creates a transition that does a top to bottom wipe.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_TB_Wipe(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float ysize = float(image1->Height()) / float(options.nFrames - 1);

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        int currentY = int(float(frame) * ysize);

        for(int y = 0; y < image1->Height(); ++y)
        {
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = image1->Pixel(x, y);
                unsigned char *src2 = image2->Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                if(frame == 0)
                    COPY_RGB(dest, src1)
                else if(frame == options.nFrames-1)
                    COPY_RGB(dest, src2)
                else
                {
                    if(y < currentY)
                        COPY_RGB(dest, src2)
                    else
                        COPY_RGB(dest, src1)
                }
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_LR_Slide
//
// Purpose: 
//   Creates a transition that slides in a new image from the left.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_LR_Slide(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    int startX = -image1->Width();

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;
        int currentX = int(t * image1->Width());

        ImageObject output(*image1);
        output.CopyBlock(*image2, 0, 0, image1->Width(), image1->Height(), startX + currentX, 0);

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_RL_Slide
//
// Purpose: 
//   Creates a transition that slides in a new image from the right.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_RL_Slide(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    int startX = image1->Width();

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;
        int currentX = int(t * image1->Width());

        ImageObject output(*image1);
        output.CopyBlock(*image2, 0, 0, image1->Width(), image1->Height(), startX - currentX,0);

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_BT_Slide
//
// Purpose: 
//   Creates a transition that slides in a new image from the bottom.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_BT_Slide(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    int startY = image1->Height();

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;
        int currentY = int(t * image1->Height());

        ImageObject output(*image1);
        output.CopyBlock(*image2, 0, 0, image1->Width(), image1->Height(), 0, startY - currentY);

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_TB_Slide
//
// Purpose: 
//   Creates a transition that slides in a new image from the top.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_TB_Slide(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    int startY = -image1->Height();

    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;
        int currentY = int(t * image1->Height());

        ImageObject output(*image1);
        output.CopyBlock(*image2, 0, 0, image1->Width(), image1->Height(), 0, startY + currentY);

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_Circle
//
// Purpose: 
//   Creates a transition that reveals the 2nd image from the center outward.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_Circle(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(image1->Width(), image2->Height());

    float hW = float(image1->Width()) / 2.f + 1.;
    float hH = float(image1->Height()) / 2.f + 1.;
    float imageRad = sqrt(hW * hW + hH * hH);

    float div = (options.nFrames > 1) ? float(options.nFrames-2) : 1.;
    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;
        float currentRad = t * imageRad;
        float shadowRad = currentRad - (imageRad/div);
        float dRad = (currentRad - shadowRad);

        for(int y = 0; y < image1->Height(); ++y)
        {
            float dY = float(y) - hH;
            float dY2 = dY*dY;
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = image1->Pixel(x, y);
                unsigned char *src2 = image2->Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                float dX = float(x) - hW;
                float r = sqrt(dX*dX + dY2);

                if(r < shadowRad)
                    COPY_RGB(dest, src2)
                else if(r < currentRad && dRad > 0.)
                {
                    float shadowT = (r - shadowRad) / dRad;
                    unsigned char black[3] = {0,0,0};
                    BlendRGB(dest, src2, black, shadowT * 0.8);
                }
                else
                    COPY_RGB(dest, src1)
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_Ripple
//
// Purpose: 
//   Creates a transition that blends one image to another using a ripple effect.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_Ripple(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];

    float div = (options.nFrames > 1) ? float(options.nFrames-1) : 1.;
    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / div;

        ImageObject output(image1->Width(), image2->Height());

        unsigned char *src1 = image1->Pixel(0,0);
        unsigned char *src2 = image2->Pixel(0,0);
        unsigned char *dest = output.Pixel(0,0);
 
        // Create a blended image of the 2 images.
        unsigned int npix = image1->Width() * image1->Height();
        for(unsigned int p = 0; p < npix; ++p)
        {
            BlendRGB(dest, src1, src2, t);
            dest += 3;
            src1 += 3;
            src2 += 3;
        }

        // Figure out what the maximum opacity of the ripples should be.
        float rippleAlpha;
        if(frame ==0 || frame == options.nFrames-1)
            rippleAlpha = 0.;
        else
        {
            float gx = t * 5.5 - 2.75;
            rippleAlpha = ((1 / (2. * M_PI)) * exp(-(gx*gx) / 2.)) / 0.159147;
        }

        // Draw the ripples on the image.
        float hW = float(image1->Width()) / 2.f + 1.;
        float hH = float(image1->Height()) / 2.f + 1.;
        float imageRad = sqrt(hW * hW + hH * hH);
        float currentRad = t * imageRad;
        unsigned char black[3] = {0,0,0};

        for(int y = 0; y < image1->Height(); ++y)
        {
            float dY = float(y) - hH;
            float dY2 = dY*dY;
            for(int x = 0; x < image1->Width(); ++x)
            {
                unsigned char *src1 = output.Pixel(x, y);
                unsigned char *dest = output.Pixel(x, y);

                float dX = float(x) - hW;
                float r = sqrt(dX*dX + dY2);

                float ripple1 = (r / imageRad) * 32. * M_PI;
                float ripple2 = t * 6. * M_PI;
                float s = sin(ripple1 - ripple2);
                float multiplier;
                if(s > 0.)
                {
                    unsigned char tmp[3];
                    unsigned char gray[3] = {50,50,50};
                    BlendRGB(tmp, src1, gray, 0.8);

                    multiplier = rippleAlpha * s;
                    BlendRGB(dest, src1, tmp, multiplier);
                }
                else
                {
                    unsigned char tmp[3];
                    unsigned char white[3]={255,255,255};
                    BlendRGB(tmp, src1, white, 0.6);

                    multiplier = rippleAlpha * -s;
                    BlendRGB(dest, src1, tmp, multiplier);
                }

            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}

// ****************************************************************************
// Function: Transition_Block
//
// Purpose: 
//   Creates a transition that moves one image to another in little blocks.
//
// Arguments:
//   options : The command line options.
//   image1  : One source image.
//   image2  : 2nd source image.
//
// Returns:    0 on success.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
Transition_Block(const TransitionOptions &options, ImageObject *image1, ImageObject *image2)
{
    int ret = 0;
    char outputName[500];
    ImageObject output(*image1);

    int nxBlocks = image1->Width() / 50;
    int nyBlocks = image1->Height() / 50;
    if(nxBlocks == 0)
        nxBlocks = 1;
    if(nyBlocks == 0)
        nyBlocks = 1;

    int nBlocks = nxBlocks * nyBlocks;
    int *order = new int[nBlocks];
    std::set<int> taken;
    for(int block = 0; block < nBlocks; ++block)
    {
        int index;
        do
        {
            index = rand() % nBlocks;
        } while(taken.find(index) != taken.end());

        order[block] = index;
        taken.insert(index);
    }

    float bwidth = float(image1->Width()) / float(nxBlocks);
    float bheight = float(image1->Height()) / float(nyBlocks);

    int blocksPerFrame = nBlocks / (options.nFrames-1);
    if(blocksPerFrame == 0)
        blocksPerFrame = 1;

    int currentBlock = 0;
    for(int frame = 0; frame < options.nFrames; ++frame)
    {
        float t = float(frame) / float(options.nFrames-1);
        if(frame != 0)
        {
            int lastBlock = int(t * nBlocks);
            if(frame == options.nFrames-1)
                lastBlock = nBlocks;
            while(currentBlock < lastBlock)
            {
                int blockX = order[currentBlock] % nxBlocks;
                int blockY = order[currentBlock] / nxBlocks;

                // Calculate block box coordinates.
                int x0 = int(bwidth * float(blockX));
                int y0 = int(bheight * float(blockY));
                int bw = int(bwidth) + 1;
                int bh = int(bheight) + 1;
                if(blockX == nxBlocks-1)
                    bw = image1->Width() - x0;
                if(blockY == nyBlocks-1)
                    bh = image1->Height() - y0;

                output.CopyBlock(*image2, x0, y0, bw, bh, x0, y0);
                ++currentBlock;
            }
        }

        // Write the output image.
        SNPRINTF(outputName, 500, options.output, frame);
        if(!output.Write(outputName))
        {
            fprintf(stderr, "Could not write file %s\n", outputName);
            ret = -6;
        }
        else if(options.verbose)
            printf("Wrote %s\n", outputName);
    }

    return ret;
}


// ****************************************************************************
// Function: PrintUsage
//
// Purpose: 
//   Prints the usage and exits.
//
// Arguments:
//   argv0 : The name of the program.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
PrintUsage(const char *argv0)
{
    fprintf(stderr, "Usage: %s -style transition [-input1 filename] \n", argv0);
    fprintf(stderr, "        [-input2 filename] [-color1 r g b] [-color2 r g b]\n");
    fprintf(stderr, "        [-nframes number] [-output filebase] [-verbose]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The \"transition\" value used for with the -style argument can be one of \n");
    fprintf(stderr, "the following:\n");
    for(int j = 0; j <= TRANSITION_BLOCK; ++j)
         fprintf(stderr, "\t%s\n", styleNames[j]);
    exit(-1);
}

// ****************************************************************************
// Function: NotEnoughArgs
//
// Purpose: 
//   Prints the not enough arguments message and quits.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 11 19:52:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
NotEnoughArgs(const char *argv0, const char *argName, int nArgs)
{
    fprintf(stderr, "The %s argument requires %d additional arguments.\n",
            argName, nArgs);

    PrintUsage(argv0);
}

// ****************************************************************************
// Function:  main
//
// Purpose:
//   The main function for the program.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 17:41:24 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    TransitionOptions options;
    ImageObject *image1 = 0, *image2 = 0;

    if(argc < 3)
        PrintUsage(argv[0]);

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-style") == 0)
        {
            if(i+1 < argc)
            {
                int s = -1;
                for(int j = 0; j <= TRANSITION_BLOCK; ++j)
                {
                    if(strcmp(styleNames[j], argv[i+1]) == 0)
                    {
                        s = j;
                        break;
                    }
                }

                if(s == -1)
                {
                    fprintf(stderr, "An invalid -style option was provided.\n");
                    PrintUsage(argv[0]);
                }
                else
                    options.transition_style = s;

                ++i;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 1);
        }
        else if(strcmp(argv[i], "-input1") == 0)
        {
            if(i+1 < argc)
            {
                strncpy(options.input1, argv[i+1], 500);
                options.input1_uses_file = true;
                ++i;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 1);
        }
        else if(strcmp(argv[i], "-color1") == 0)
        {
            if(i+1 < argc)
            {
                int tmp[3] = {0,0,0};
                for(int j = 0; j < 3; ++j)
                {
                    bool err = sscanf(argv[i+1+j], "%d", &tmp[j]) != 1;

                    // Check range
                    if(!err && (tmp[j] < 0 || tmp[j] > 255))
                        err = true;

                    if(err)
                    {
                        fprintf(stderr, "An invalid color option was provided "
                            "for component %d of the -color1 argument.\n",
                            j+1);
                        PrintUsage(argv[0]);
                    }
                }
                options.color1[0] = tmp[0];
                options.color1[1] = tmp[1];
                options.color1[2] = tmp[2];
                options.input1_uses_file = false;
                i += 3;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 3);
        }
        else if(strcmp(argv[i], "-input2") == 0)
        {
            if(i+1 < argc)
            {
                strncpy(options.input2, argv[i+1], 500);
                options.input2_uses_file = true;
                ++i;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 1);
        }
        else if(strcmp(argv[i], "-color2") == 0)
        {
            if(i+1 < argc)
            {
                int tmp[3] = {0,0,0};
                for(int j = 0; j < 3; ++j)
                {
                    bool err = sscanf(argv[i+1+j], "%d", &tmp[j]) != 1;

                    // Check range
                    if(!err && (tmp[j] < 0 || tmp[j] > 255))
                        err = true;

                    if(err)
                    {
                        fprintf(stderr, "An invalid color option was provided "
                            "for component %d of the -color2 argument.\n",
                            j+1);
                        PrintUsage(argv[0]);
                    }
                }
                options.color2[0] = tmp[0];
                options.color2[1] = tmp[1];
                options.color2[2] = tmp[2];
                options.input2_uses_file = false;
                i += 3;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 3);
        }
        else if(strcmp(argv[i], "-nframes") == 0)
        {
            if(i+1 < argc)
            {
                int nf = 10;
                bool err = sscanf(argv[i+1], "%d", &nf) != 1;

                // Check range
                if(!err && nf < 0)
                    err = true;

                if(err)
                {
                    fprintf(stderr, "An invalid number of frames was provided "
                        "for -nframes argument.\n");
                    PrintUsage(argv[0]);
                }
                options.nFrames = nf;
                ++i;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 3);
        }
        else if(strcmp(argv[i], "-output") == 0)
        {
            if(i+1 < argc)
            {
                strncpy(options.output, argv[i+1], 500);
                ++i;
            }
            else
                NotEnoughArgs(argv[0], argv[i], 1);
        }
        else if(strcmp(argv[i], "-verbose") == 0)
        {
            options.verbose = true;
        }
        else
        {
            fprintf(stderr, "%s is not a valid argument.\n\n", argv[i]);
            PrintUsage(argv[0]);
        }
    }

    //
    // Create input images.
    //
    if(options.input1_uses_file)
    {
        image1 = new ImageObject();
        if(!image1->Read(options.input1))
        {
            fprintf(stderr, "The input1 file, %s, could not be read.\n", options.input1);
            exit(-2);
        }
    }
    if(options.input2_uses_file)
    {
        image2 = new ImageObject();
        if(!image2->Read(options.input2))
        {
            fprintf(stderr, "The input2 file, %s, could not be read.\n", options.input2);
            exit(-3);
        }
    }

    if(options.input1_uses_file && !options.input2_uses_file)
    {
        image2 = new ImageObject(image1->Width(), image1->Height());
        image2->SetAllPixelsToColor(options.color2);
    }
    else if(!options.input1_uses_file && options.input2_uses_file)
    {
        image1 = new ImageObject(image2->Width(), image2->Height());
        image1->SetAllPixelsToColor(options.color1);
    }
    else if(!options.input1_uses_file && !options.input2_uses_file)
    {
        int w = 400;
        int h = 400;

        image1 = new ImageObject(w, h);
        image1->SetAllPixelsToColor(options.color1);

        image2 = new ImageObject(w, h);
        image2->SetAllPixelsToColor(options.color2);
    }

    //
    // Make sure that the input images have the same sizes.
    //
    if(image1->Width() != image2->Width() ||
       image1->Height() != image2->Height())
    {
        fprintf(stderr, "The input images are difference sizes.\n");
        exit(-4);
    }

    int ret = -5;
    if(options.transition_style == TRANSITION_FADE)
        ret = Transition_Fade(options, image1, image2);
    else if(options.transition_style == TRANSITION_LR_WIPE)
        ret = Transition_LR_Wipe(options, image1, image2);
    else if(options.transition_style == TRANSITION_RL_WIPE)
        ret = Transition_RL_Wipe(options, image1, image2);
    else if(options.transition_style == TRANSITION_BT_WIPE)
        ret = Transition_BT_Wipe(options, image1, image2);
    else if(options.transition_style == TRANSITION_TB_WIPE)
        ret = Transition_TB_Wipe(options, image1, image2);
    else if(options.transition_style == TRANSITION_LR_SLIDE)
        ret = Transition_LR_Slide(options, image1, image2);
    else if(options.transition_style == TRANSITION_RL_SLIDE)
        ret = Transition_RL_Slide(options, image1, image2);
    else if(options.transition_style == TRANSITION_BT_SLIDE)
        ret = Transition_BT_Slide(options, image1, image2);
    else if(options.transition_style == TRANSITION_TB_SLIDE)
        ret = Transition_TB_Slide(options, image1, image2);
    else if(options.transition_style == TRANSITION_CIRCLE)
        ret = Transition_Circle(options, image1, image2);
    else if(options.transition_style == TRANSITION_RIPPLE)
        ret = Transition_Ripple(options, image1, image2);
    else if(options.transition_style == TRANSITION_BLOCK)
        ret = Transition_Block(options, image1, image2);

    delete image1;
    delete image2;

    return ret;
}
