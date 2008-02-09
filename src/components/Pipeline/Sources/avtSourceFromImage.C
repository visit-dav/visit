/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                           avtSourceFromImage.C                            //
// ************************************************************************* //

#include <avtSourceFromImage.h>

#include <vtkImageData.h>

#include <avtImage.h>


// ****************************************************************************
//  Method: avtSourceFromImage constructor
//
//  Arguments:
//     img      The image.
//     zb       The zbuffer.  This is owned by this object after this call.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

avtSourceFromImage::avtSourceFromImage(vtkImageData *img, float *zb)
{
    image = img;
    if (image != NULL)
    {
        image->Register(NULL);
    }
    zbuffer = zb;
}


// ****************************************************************************
//  Method: avtSourceFromImage destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

avtSourceFromImage::~avtSourceFromImage()
{
    if (image != NULL)
    {
        image->Delete();
    }

    //
    // The zbuffer is not deleted here because its output image owns it.  This
    // could be a problem if the output image has never claimed it, though.
    //
}


// ****************************************************************************
//  Method: avtSourceFromImage::SetImage
//
//  Purpose:
//      Set the image for this source.
//
//  Arguments:
//      img     The new image.
//      zb      The zbuffer.  This is owned by this object after this call.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

void
avtSourceFromImage::SetImage(vtkImageData *img, float *zb)
{
    if (image != NULL)
    {
        image->Delete();
    }

    image = img;
    zbuffer = zb;

    if (image != NULL)
    {
        image->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtSourceFromImage::FetchImage
//
//  Purpose:
//      This copies the image (or shares a reference) to the output image.
//
//  Arguments:
//      spec    The data specification.  It is unclear to me how to use this
//              right now.
//      rep     The image representation to copy this into.
//
//  Returns:    Should return something based on whether the image has changed,
//              but it always returns false for now.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

bool
avtSourceFromImage::FetchImage(avtDataRequest_p,
                               avtImageRepresentation &rep)
{
    if (zbuffer != NULL)
    {
        rep = avtImageRepresentation(image, zbuffer);
    }
    else
    {
        rep = avtImageRepresentation(image);
    }

    return false;
}


