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
//                        avtSourceFromAVTImage.C                          //
// ************************************************************************* //

#include <avtSourceFromAVTImage.h>

#include <avtCommonDataFunctions.h>
#include <avtImage.h>
#include <avtSILRestrictionTraverser.h>

#include <TimingsManager.h>

using     std::vector;

avtDataObject_p   DataObjectFromImage(avtImage_p);


// ****************************************************************************
//  Method: avtSourceFromAVTImage constructor
//
//  Arguments:
//      img     An avtImage_p
//
//  Programmer: Mark C. Miller
//  Creation:   June 19, 2001
//
// ****************************************************************************

avtSourceFromAVTImage::avtSourceFromAVTImage(avtImage_p img)
    : avtInlinePipelineSource(DataObjectFromImage(img))
{
    image = img->GetImage();
    GetTypedOutput()->GetInfo().Copy(img->GetInfo());
}


// ****************************************************************************
//  Method: avtSourceFromAVTImage destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSourceFromAVTImage::~avtSourceFromAVTImage()
{
    ;
}


// ****************************************************************************
//  Method: avtSourceFromAVTImage::FetchImage
//
//  Purpose:
//      Gets the image 
//
//  Arguments:
//      img    The output image.
//
//  Returns:    Whether or not this dataset is different from the last time it
//              was updated.
//       
//  Programmer: Mark C. Miller
//  Creation:   June 19, 2001
//
// ****************************************************************************

bool
avtSourceFromAVTImage::FetchImage(avtDataSpecification_p spec,
                          avtImageRepresentation &outImage)
{
    int timingsHandle = visitTimer->StartTimer();

    outImage = image;

    visitTimer->StopTimer(timingsHandle, "Fetching image AVT image");

    return false;
}


// ****************************************************************************
//  Function: DataObjectFromImage
//
//  Purpose:
//      Creates a pointer typed to the base class.  This should happen free
//      with C++, but does not because of reference pointers.
//
//  Programmer: Mark C. Miller
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtDataObject_p
DataObjectFromImage(avtImage_p img)
{
    avtDataObject_p rv;
    CopyTo(rv, img);

    return rv;
}


