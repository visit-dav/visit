/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <avtMultiWindowSaver.h>
#include <BadIndexException.h>

#include <vtkImageData.h>

// ****************************************************************************
// Method: avtMultiWindowSaver::avtMultiWindowSaver
//
// Purpose: 
//   Constructor for the avtMultiWindowSaver class.
//
// Arguments:
//   atts    The attributes for the save.
//
// Programmer: Hank Childs
// Creation:   July 16, 2010
//
// Modifications:
//   
// ****************************************************************************

avtMultiWindowSaver::avtMultiWindowSaver(const SaveSubWindowsAttributes &a2)
{
    atts = a2;
    width  = 0;
    height = 0;
}

// ****************************************************************************
// Method: avtMultiWindowSaver::~avtMultiWindowSaver
//
// Purpose: 
//   Destructor for the avtMultiWindowSaver class.
//
// Programmer: Hank Childs
// Creation:   July 20, 2010
//
// Modifications:
//   
// ****************************************************************************

avtMultiWindowSaver::~avtMultiWindowSaver()
{
    // Delete the images in the array.
    for(int i = 0; i < 16; ++i)
        images[i] = 0;
}

// ****************************************************************************
// Method: avtMultiWindowSaver::AddImage
//
// Purpose: 
//   Adds an image to the list of images to be added as a tile.
//
// Arguments:
//   img : A pointer to the image to be added as a tile.
//
// Programmer: Hank Childs
// Creation:   July 20, 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtMultiWindowSaver::AddImage(avtImage_p img, int id)
{
    if(*img == 0)
    {
        EXCEPTION0(VisItException);
    }

    if(id >= 1 && id <= 16)
    {
        images[id-1] = img;
    }
    else
    {
        EXCEPTION2(BadIndexException,id, 16);
    }
}


// ****************************************************************************
// Method: avtMultiWindowSaver::CreateImage
//
// Purpose: 
//   Creates a combined image and returns it.
//
// Returns:    An avtImage_p containing the new image.
//
// Programmer: Hank Childs
// Creation:   July 20, 2010
//
// Modifications:
//   
// ****************************************************************************

avtImage_p
avtMultiWindowSaver::CreateImage()
{
    //
    // Create a vtkImageData object to contain the tiled image.
    //
    const int nColorComponents = 3;
    vtkImageData *newImage = vtkImageData::New();
    newImage->SetDimensions(width, height, 1);
    newImage->SetScalarTypeToUnsignedChar();
    newImage->SetNumberOfScalarComponents(nColorComponents);
    newImage->AllocateScalars();
    unsigned char *out_ptr = (unsigned char *)newImage->GetScalarPointer();

    int nLayers = 16;
    for (int j = nLayers-1 ; j >= 0 ; j--)
    {
        for (int i = 0 ; i < 16 ; i++)
        {
            SaveSubWindowAttributes winAtts = atts.GetAttsForWindow(i+1);
            if (winAtts.GetLayer() != j || winAtts.GetOmitWindow())
                continue;
            if (*(images[i]) == NULL)
                continue;

            int imgWidth = 0, imgHeight = 0;
            images[i]->GetImage().GetSize(&imgHeight, &imgWidth);
            if (imgWidth != winAtts.GetSize()[0] || imgHeight != winAtts.GetSize()[1])
            {
                EXCEPTION1(VisItException, "The image passed in to the multi-window save is "
                            "incorrectly sized.  This is an internal error.");
            }

            int *pos = winAtts.GetPosition();
            float t = winAtts.GetTransparency();
            vtkImageData *img = images[i]->GetImage().GetImageVTK();
            unsigned char *in_ptr = (unsigned char *)img->GetScalarPointer();
            for (int h = 0 ; h < imgHeight ; h++)
            {
                int hh = h+pos[1];
                if (hh < 0 || hh >= height)
                    continue;
                for (int w = 0 ; w < imgWidth ; w++)
                {
                    int ww = w+pos[0];
                    if (ww < 0 || ww >= width)
                        continue;
                    int indexOut = hh*width   + ww;
                    int indexIn  = h*imgWidth + w;
                    if (t == 0.0)
                    {
                        out_ptr[3*indexOut+0] = in_ptr[3*indexIn+0];
                        out_ptr[3*indexOut+1] = in_ptr[3*indexIn+1];
                        out_ptr[3*indexOut+2] = in_ptr[3*indexIn+2];
                    }
                    else
                    {
                        out_ptr[3*indexOut+0] = (1-t)*in_ptr[3*indexIn+0]+
                                                   t *out_ptr[3*indexOut+0];
                        out_ptr[3*indexOut+1] = (1-t)*in_ptr[3*indexIn+1]+
                                                   t *out_ptr[3*indexOut+1];
                        out_ptr[3*indexOut+2] = (1-t)*in_ptr[3*indexIn+2]+
                                                   t *out_ptr[3*indexOut+2];
                    }
                }
            }
        }
    }

    //
    // Now that we have a vtkImageData object, wrap it in something AVT likes.
    //
    avtImage_p retval = new avtImage(0);
    retval->GetImage() = avtImageRepresentation(newImage);
    newImage->Delete();

    return retval;
}
