/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                              avtImageWriter.C                             //
// ************************************************************************* //

#include <avtImageWriter.h>

#include <avtDataObjectString.h>


// ****************************************************************************
//  Method: avtImageWriter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageWriter::avtImageWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtImageWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageWriter::~avtImageWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtImageWriter::DataObjectWrite
//
//  Arguments:
//      str         A string to append the image to.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 18, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Dec 28 16:33:13 PST 2000
//    Changed to account for changes in avtDataObjectWriter.
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Mon Oct  1 09:31:50 PDT 2001
//    Renamed to DataObjectWrite.
//
//    Mark C. Miller, Mon Oct 31 18:12:49 PST 2005
//    Added code to support compression of data object string
//
// ****************************************************************************

void
avtImageWriter::DataObjectWrite(avtDataObjectString &str)
{
    avtImageRepresentation &image = GetImageRep();
    int length;
    unsigned char *imagestr = useCompression ?
                              image.GetCompressedImageString(length) :
                              image.GetImageString(length);

    //
    // Write out how long the image string is.
    //
    WriteInt(str, length);

    //
    // Append on our image.
    //
    str.Append((char *)imagestr, length,
               avtDataObjectString::DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE);
}


