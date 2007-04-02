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

// ************************************************************************* //
//                                 avtImage.C                                //
// ************************************************************************* //

#include <avtImage.h>

#include <avtImageSource.h>
#include <avtImageWriter.h>


// ****************************************************************************
//  Method: avtImage constructor
//
//  Arguments:
//      src     An data object source that is the upstream object.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImage::avtImage(avtDataObjectSource *src)
    : avtDataObject(src)
{
    ;
}


// ****************************************************************************
//  Method: avtImage destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImage::~avtImage()
{
    ;
}


// ****************************************************************************
//  Method: avtImage::SetImage
//
//  Purpose:
//      Sets this objects image.  This is a protected method that should only
//      be accessed by avtImageSource.
//
//  Arguments:
//      ir      The image representation of the new image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

void
avtImage::SetImage(const avtImageRepresentation &ir)
{
    image = ir;
}


// ****************************************************************************
//  Method: avtImage::GetImage
//
//  Purpose:
//      Gets the image as an avtImageRepresentation.
//
//  Returns:    the image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation &
avtImage::GetImage(void)
{
    return image;
}

// ****************************************************************************
//  Method: avtImage::GetSize
//
//  Purpose:
//      Gets size of the image 
//
//  Programmer: Mark C. Miller 
//  Creation:   31Mar04 
//
// ****************************************************************************

void
avtImage::GetSize(int *width, int *height) const
{
   // argument order is inverted due to fact that image rep uses 'rowsize' and
   // 'colsize' instead of width and height
   image.GetSize(height, width);
}

// ****************************************************************************
//  Method: avtImage::GetNumberOfCells
//
//  Purpose:
//      Gets number of cells in an image 
//
//  Programmer: Mark C. Miller 
//  Creation:   19Aug03 
//
//  Modificaitons:
//
//    Mark C. Miller, Wed Nov  5 09:48:13 PST 2003
//    Added option to count polygons only
//
// ****************************************************************************

int
avtImage::GetNumberOfCells(bool polysOnly) const
{
   return image.GetNumberOfCells(polysOnly);
}


// ****************************************************************************
//  Method: avtImage::InstantiateWriter
//
//  Purpose:
//      Instantiates a writer that is appropriate for an avtImage.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2001
//
// ****************************************************************************

avtDataObjectWriter *
avtImage::InstantiateWriter(void)
{
    return new avtImageWriter;
}


// ****************************************************************************
//  Method: avtImage::ReleaseData
//
//  Purpose:
//      Free the data associated with this image.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
avtImage::ReleaseData(void)
{
    image.ReleaseData();
}

// ****************************************************************************
//  Method: avtImage::Instance
//
//  Purpose:
//      Creates an instance of an avtImage.
//
//  Programmer: Mark C. Miller
//  Creation:   February 4, 2004 
//
// ****************************************************************************

avtDataObject *
avtImage::Instance(void)
{
    avtDataObjectSource *src = NULL;
    avtImage *img = new avtImage(src);
    return img;
}

// ****************************************************************************
//  Method: avtImage::DerivedCopy
//
//  Purpose:
//      Copy over the image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
avtImage::DerivedCopy(avtDataObject *dob)
{
    avtImage *img = (avtImage *) dob;
    image = img->image;
}


