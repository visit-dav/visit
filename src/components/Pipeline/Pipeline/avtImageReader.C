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
//                              avtImageReader.C                             //
// ************************************************************************* //

#include <avtImageReader.h>


// ****************************************************************************
//  Method: avtImageReader constructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

avtImageReader::avtImageReader()
{
    haveReadImage = false;
}


// ****************************************************************************
//  Method: avtImageReader destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageReader::~avtImageReader()
{
    ;
}


// ****************************************************************************
//  Method: avtImageReader::Read
//
//  Purpose:
//      Takes in a character string and reads the image out of it.
//
//  Arguments:
//      input  the string to read
//
//  Returns:    The size of the image in bytes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 18, 2000
//
// ****************************************************************************

int
avtImageReader::Read(char *input)
{
    int  size = 0;

    //
    // Find out how long the image is.
    //
    int  length;
    memcpy(&length, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
 
    //
    // Save the image as one of our data members.
    //
    imgRep = avtImageRepresentation(input, length);
    size += length;

    haveReadImage = false;

    GetOutputImage() = imgRep;
    return size;
}


// ****************************************************************************
//  Method: avtImageReader::FetchImage
//
//  Purpose:
//      Meets the interface of its base type and returns the image
//      representation to the base class.
//
//  Arguments:
//      <unnamed>   The data specification.  It is unclear how to use this with
//                  images right now.
//      rep         Should be assigned the image representation for this image.
//
//  Returns:        true if the image has changed since the last time it was
//                  fetched.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

bool
avtImageReader::FetchImage(avtDataSpecification_p ,avtImageRepresentation &rep)
{
    rep = imgRep;

    bool rv = !haveReadImage;
    haveReadImage = true;

    return rv;
}


