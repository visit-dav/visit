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
//                              avtImageSource.C                             //
// ************************************************************************* //

#include <avtImageSource.h>

#include <vtkImageData.h>


// ****************************************************************************
//  Method: avtImageSource constructor
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

avtImageSource::avtImageSource()
{
    image = new avtImage(this);
}


// ****************************************************************************
//  Method: avtImageSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageSource::~avtImageSource()
{
    ;
}


// ****************************************************************************
//  Method: avtImageSource::GetOutput
//
//  Purpose:
//      Gets the output avtImage as an avtDataObject.
//
//  Returns:    The source's image, typed as a data object.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 15:27:03 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtImageSource::GetOutput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, image);

    return rv;
}


// ****************************************************************************
//  Method: avtImageSource::GetVTKOutput
//
//  Purpose:
//      Gets the output as a vtkImageData.
//
//  Returns:    The output of the source as a VTK object.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

vtkImageData *
avtImageSource::GetVTKOutput(void)
{
    return image->GetImage().GetImageVTK();
}


// ****************************************************************************
//  Method: avtImageSource::SetOutputImage
//
//  Purpose:
//      Sets the output image.
//
//  Arguments:
//      img     The image as an avtImageRepresentation.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

void
avtImageSource::SetOutputImage(const avtImageRepresentation &img)
{
    image->SetImage(img);
}


// ****************************************************************************
//  Method: avtImageSource::SetOutput
//
//  Purpose:
//      Sets the output to be the same as the argument.
//
//  Arguments:
//      img     The new image.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

void
avtImageSource::SetOutput(avtImage_p img)
{
    image->SetImage(img->GetImage());
}


// ****************************************************************************
//  Method: avtImageSource::GetOutputImage
//
//  Purpose:
//      Gets the output image.
//
//  Returns:    The image as an avtImageRepresentation.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

avtImageRepresentation &
avtImageSource::GetOutputImage(void)
{
    return image->GetImage();
}


