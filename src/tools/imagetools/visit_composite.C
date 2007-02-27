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
#include <stdio.h>
#include <string.h>
#include <ImageObject.h>
#include <math.h>

#define M_OPAQUE       0
#define M_TRANSPARENT  1
#define M_COLORREPLACE 2

// ****************************************************************************
// Class: ViewportInfo
//
// Purpose:
//   Contains settings for a viewport.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 17:39:40 PST 2006
//
// Modifications:
//   
// ****************************************************************************

struct ViewportInfo
{
    ViewportInfo()
    {
        memset(imageName, 0, 1024);
        coordinates[0] = coordinates[1] = 0.f;
        coordinates[2] = coordinates[3] = 1.f;
        opacity = 1.;
        opaqueMode = M_OPAQUE;
        transparentColor[0] = transparentColor[0] = transparentColor[0] = 255;
        hasDropShadow = false;
        file = 0;
    }

    ~ViewportInfo()
    {
        delete file;
    }

    void ReadParameters(FILE *f)
    {
        fscanf(f, "%s\n", imageName);
        fscanf(f, "%g %g %g %g\n", &coordinates[0], &coordinates[1],
               &coordinates[2], &coordinates[3]);
        fscanf(f, "%d\n", &opaqueMode);
        fscanf(f, "%g\n", &opacity);
        int tmp[3];
        fscanf(f, "%d %d %d\n", &tmp[0], &tmp[1], &tmp[2]);
        transparentColor[0] = (unsigned char)tmp[0];
        transparentColor[1] = (unsigned char)tmp[1];
        transparentColor[2] = (unsigned char)tmp[2];
        fscanf(f, "%d\n", &tmp[0]);
        hasDropShadow = tmp[0] > 0;
    }

    char          imageName[1024];
    float         coordinates[4];
    int           opaqueMode;
    float         opacity;
    unsigned char transparentColor[3];
    bool          hasDropShadow;
    ImageObject    *file;
};

// ****************************************************************************
// Class: CompositeParameters
//
// Purpose:
//   Contains compisting parameters.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 17:39:58 PST 2006
//
// Modifications:
//   
// ****************************************************************************

struct CompositeParameters
{
    CompositeParameters()
    {
        nViewports = 0;
        outputSize[0] = outputSize[1] = 512;
        outputBackground[0] = outputBackground[1] = outputBackground[2] = 255;
        memset(outputFile, 0, 1024);
        memset(backgroundFile, 0, 1024);
        shadowOffsetX = shadowOffsetY = 0.01f;
        shadowBlurRadius = 0.008f;
        viewports = 0;
    }

    ~CompositeParameters()
    {
        for(int i = 0; i < nViewports; ++i)
            delete viewports[i];
        delete [] viewports;
    }

    bool ReadParameters(const char *filename)
    {
        bool retval = false;
        FILE *f = fopen(filename, "rt");
        if(f != 0)
        {
            fscanf(f, "%d\n", &nViewports);
            fscanf(f, "%d %d\n", &outputSize[0], &outputSize[1]);
            int tmp[3];
            fscanf(f, "%d %d %d\n", &tmp[0], &tmp[1], &tmp[2]);
            outputBackground[0] = (unsigned char)tmp[0];
            outputBackground[1] = (unsigned char)tmp[1];
            outputBackground[2] = (unsigned char)tmp[2];
            fscanf(f, "%s\n", outputFile);
            fscanf(f, "%s\n", backgroundFile);
            fscanf(f, "%g %g\n", &shadowOffsetX, &shadowOffsetY);
            fscanf(f, "%g\n", &shadowBlurRadius);

            viewports = new ViewportInfo*[nViewports];
            for(int i = 0; i < nViewports; ++i)
            {
                viewports[i] = new ViewportInfo;
                viewports[i]->ReadParameters(f);
            }

            fclose(f);
            retval = true;
        }

        return retval;
    }

    int            nViewports;
    int            outputSize[2];
    unsigned char  outputBackground[3];
    char           outputFile[1024];
    char           backgroundFile[1024];
    float          shadowOffsetX;
    float          shadowOffsetY;
    float          shadowBlurRadius;

    ViewportInfo **viewports;
};

// ****************************************************************************
// Function: CreateDropShadowMask
//
// Purpose: 
//   Creates a drop shadow image.
//
// Arguments:
//   vpInfo : The information about the viewport.
//   offx   : The shadow offset in x.
//   offy   : The shadow offset in y.
//   r      : The blur radius.
//
// Returns:    A 1 channel drop shadow mask.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 18:08:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ImageObject *
CreateDropShadowMask(const ViewportInfo *vpInfo, int offx, int offy, int r)
{
    int rx2 = r * 2;
    int img_w = vpInfo->file->Width();
    int img_h = vpInfo->file->Height();

    ImageObject *mask = new ImageObject(
        offx + r + img_w, offy + r + img_h, 1);

    // Create the blur image
    float *blur = new float[rx2 * rx2];
    float root2 = sqrt(2.);
    float blurSum = 0.;
    for (int i=0; i<rx2; i++)
    {
        for (int j=0; j<rx2; j++)
        {
            float u = float(i)/float(rx2-1);
            float v = float(j)/float(rx2-1);

            float gu = exp(-pow((4*(u - .5)), 2));
            float gv = exp(-pow((4*(v - .5)), 2));
            float a = gu * gv;
            blur[i*rx2+j] = a;
            blurSum += a;
        }
    }
    for(int j = 0; j < rx2; ++j)
        for(int i = 0; i < rx2; ++i)
            blur[j*rx2 + i] /= blurSum;

    if(vpInfo->opaqueMode == M_OPAQUE)
    {
        int white = 255;
        mask->SetBlockToColor(offx, offy, offx + img_w, offy + img_h, &white);
    }
    else if(vpInfo->opaqueMode == M_TRANSPARENT)
    {
        int gray = int(vpInfo->opacity * 255.);
        mask->SetBlockToColor(offx, offy, offx + img_w, offy + img_h, &gray);
    }
    else
    {
        unsigned char rr, rg, rb;
        rr = vpInfo->transparentColor[0];
        rg = vpInfo->transparentColor[1];
        rb = vpInfo->transparentColor[2];

        // Everywhere that does not match the bg color, color white.
        for(int y = 0; y < img_h; ++y)
        {
            for(int x = 0; x < img_w; ++x)
            {
                unsigned char *src = vpInfo->file->Pixel(x, y);
                unsigned char *dest = mask->Pixel(x+offx, y+offy);

                if(src[0] != rr || src[1] != rg || src[2] != rb)
                    *dest = 255;
            }
        }
    }

    // Blur the mask by convolving it with the blur kernel.
    for(int y = offy; y < offy + img_h; ++y)
    {
        for(int x = offx; x < offx + img_w; ++x)
        {
            int ksrcx = x - r;
            int ksrcy = y - r;
            float channelSum = 0.;
            for(int ky = 0; ky < rx2; ++ky)
            {
                for(int kx = 0; kx < rx2; ++kx)
                {
                    unsigned char *mask_ptr = mask->Pixel(ksrcx + kx, ksrcy + ky);
                    channelSum += float(*mask_ptr) * blur[ky*rx2+kx];
                }
            }
            unsigned char *dest = mask->Pixel(x, y);
            *dest = (unsigned char)(int)(channelSum);
        }
    }

    if(vpInfo->opaqueMode == M_TRANSPARENT)
    {
        int black = 0;
        mask->SetBlockToColor(0, 0, img_w, img_h, &black);
    }
    delete [] blur;

    return mask;
}

// ****************************************************************************
// Function: CreateCompositeImage
//
// Purpose:
//   Takes the compositing parameters and reads in all of the files and creates
//   an output composite image.
//
// Arguments:      
//   params : The compositing parameters.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 17:40:26 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
CreateCompositeImage(CompositeParameters &params, const char *outName)
{
    // Try to read in all of the inputs.
    bool noError = true;
    int i;
    for(i = 0; i < params.nViewports && noError; ++i)
    {
        params.viewports[i]->file = new ImageObject;
        noError = params.viewports[i]->file->Read(
            params.viewports[i]->imageName);
    }
    if(!noError)
    {
        fprintf(stderr, "The following files could not be read:\n");
        for(i = 0; i < params.nViewports; ++i)
            fprintf(stderr, "\t%s\n", params.viewports[i]->imageName);
        return -3;
    }

    int startViewport = 0;
    ImageObject *output = 0;
    int width, height;
    if(strcmp(params.backgroundFile, params.viewports[0]->imageName) == 0)
    {
        startViewport = 1;
        width = params.viewports[0]->file->Width();
        height = params.viewports[0]->file->Height();
        output = params.viewports[0]->file;
    }
    else
    {
        width = params.outputSize[0];
        height = params.outputSize[1];
        output = new ImageObject(width, height);

        // Set the image's background color.
        unsigned char *cptr = output->Pixel(0,0);
        unsigned int sz = width * height;
        for(unsigned int s = 0; s < sz; ++s)
        {
            *cptr++ = params.outputBackground[0];
            *cptr++ = params.outputBackground[1];
            *cptr++ = params.outputBackground[2];
        }
    }

    // Calculate some shadow parameters based on the image size.
    int offx = int(params.shadowOffsetX * width);
    int offy = int(params.shadowOffsetY * height);
    if (offx > offy)
        offy = offx;
    else
        offx = offy;
    int blurRad = int(params.shadowBlurRadius * width);

    // Now that we have all of the images in memory and we've decided
    // which one is the background image, we should begin compositing.
    for(i = startViewport; i < params.nViewports; ++i)
    {
        int start_x = int(params.viewports[i]->coordinates[0] * width);
        int start_y = int(height - params.viewports[i]->coordinates[1] * height);

        int img_w = params.viewports[i]->file->Width();
        int img_h = params.viewports[i]->file->Height();

        if(params.viewports[i]->hasDropShadow)
        {
            ImageObject *mask = CreateDropShadowMask(params.viewports[i], 
                offx, offy, blurRad);

            if(mask != 0)
            {
                // Now that we have a drop shadow mask, let's use it to blend
                // the destination pixels with black. Values in the mask that
                // are white should be more black in the destination image.
                // Of course, we should limit the amount of black that we can
                // make a shadow.
                for(int masky = 0; masky < mask->Height(); ++masky)
                {
                    int real_y = start_y - img_h + masky;
                    for(int maskx = 0; maskx < mask->Width(); ++maskx)
                    {
                        int real_x = start_x + maskx;
                        if(real_x >= 0 && real_x < width &&
                           real_y >= 0 && real_y < height)
                        {
                            unsigned char *mask_value = mask->Pixel(maskx, masky);
                            unsigned char *dest = output->Pixel(real_x, real_y);
                            float t = (float(*mask_value) / 255.f) * 0.5;
                            float r = float(dest[0]);
                            float g = float(dest[1]);
                            float b = float(dest[2]);
                            float new_r = (1. - t) * r + t * 0.2;
                            float new_g = (1. - t) * g + t * 0.2;
                            float new_b = (1. - t) * b + t * 0.2;
                            dest[0] = (unsigned char)((int)(new_r));
                            dest[1] = (unsigned char)((int)(new_g));
                            dest[2] = (unsigned char)((int)(new_b));
                        }
                    }
                }
                delete mask;
            }
        }

        // Now that we've drawn a drop shadow, if needed, draw the image.
        if(params.viewports[i]->opaqueMode == M_OPAQUE)
        {
            for(int y = 0; y < img_h; ++y)
            {
                int real_y = start_y - img_h + y;
                for(int x = 0; x < img_w; ++x)
                {
                    int real_x = start_x + x;
                    if(real_x >= 0 && real_x < width &&
                       real_y >= 0 && real_y < height)
                    {
                        unsigned char *src = params.viewports[i]->file->Pixel(x, y);
                        unsigned char *dest = output->Pixel(real_x, real_y);

                        *dest++ = *src++;
                        *dest++ = *src++;
                        *dest = *src;
                    }
                }
            }
        }
        else if(params.viewports[i]->opaqueMode == M_TRANSPARENT)
        {
            for(int y = 0; y < img_h; ++y)
            {
                int real_y = start_y - img_h + y;
                for(int x = 0; x < img_w; ++x)
                {
                    int real_x = start_x + x;
                    if(real_x >= 0 && real_x < width &&
                       real_y >= 0 && real_y < height)
                    {
                        unsigned char *src = params.viewports[i]->file->Pixel(x, y);
                        unsigned char *dest = output->Pixel(real_x, real_y);

                        float dest_r = float(dest[0]);
                        float dest_g = float(dest[1]);
                        float dest_b = float(dest[2]);
                        float src_r = float(src[0]);
                        float src_g = float(src[1]);
                        float src_b = float(src[2]);

                        float t = params.viewports[i]->opacity;
                        float r = (1. - t) * dest_r + t * src_r;
                        float g = (1. - t) * dest_g + t * src_g;
                        float b = (1. - t) * dest_b + t * src_b;

                        *dest++ = (unsigned char)((int)r);
                        *dest++ = (unsigned char)((int)g);
                        *dest++ = (unsigned char)((int)b);
                    }
                }
            }
        }
        else if(params.viewports[i]->opaqueMode == M_COLORREPLACE)
        {
            unsigned char rr, rg, rb;
            rr = params.viewports[i]->transparentColor[0];
            rg = params.viewports[i]->transparentColor[1];
            rb = params.viewports[i]->transparentColor[2];

            for(int y = 0; y < img_h; ++y)
            {
                int real_y = start_y - img_h + y;
                for(int x = 0; x < img_w; ++x)
                {
                    int real_x = start_x + x;
                    if(real_x >= 0 && real_x < width &&
                       real_y >= 0 && real_y < height)
                    {
                        unsigned char *src = params.viewports[i]->file->Pixel(x, y);
                        unsigned char *dest = output->Pixel(real_x, real_y);

                        if(src[0] != rr || src[1] != rg || src[2] != rb)
                        {
                            *dest++ = *src++;
                            *dest++ = *src++;
                            *dest = *src;
                        }
                    }
                }
            }
        }
    }

    // Write the output image.
    output->Write(outName);

    if(startViewport == 0)
        delete output;

    return 0;
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
    CompositeParameters params;    

    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s paramfile outfile\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "The parameter file controls the inputs to this program and\n");
        fprintf(stderr, "it is composed of a global section followed by one or more\n");
        fprintf(stderr, "viewport sections that describe how each viewport is incorporated\n");
        fprintf(stderr, "into the final image. All sections are listed in the parameter\n");
        fprintf(stderr, "file one after the next with no blank lines.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Global section format:\n");
        fprintf(stderr, "%%d              # Number of viewports\n");
        fprintf(stderr, "%%d %%d           # Size of output image\n");
        fprintf(stderr, "%%d %%d %%d        # Output image background color [0,255]\n");
        fprintf(stderr, "%%s              # Output image filename\n");
        fprintf(stderr, "%%s              # Input background image\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Viewport section format:\n");
        fprintf(stderr, "%%s              # Viewport input image filename\n");
        fprintf(stderr, "%%g %%g %%g %%g     # Viewport coordinates [0.,.1] lower left, upper right \n");
        fprintf(stderr, "%%d              # Opacity mode 0=opaque, 1=transparent, 2=color replace\n");
        fprintf(stderr, "%%g              # Opacity [0.,1.] used when opacity mode == 1\n");
        fprintf(stderr, "%%d %%d %%d        # Replace color [0,255] used when opacity mode == 2\n");
        fprintf(stderr, "\n");
        return -1;
    }

    if(!params.ReadParameters(argv[1]))
    {
        fprintf(stderr, "Could not open parameter file %s\n", argv[1]);
        return -2;
    }

    return CreateCompositeImage(params, argv[2]);
}
