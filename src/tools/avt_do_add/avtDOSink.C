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
//                           avtDO_REPL_STRINGSink.C                         //
// ************************************************************************* //

#include <avtDO_REPL_STRINGSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink constructor
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSink::avtDO_REPL_STRINGSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSink::~avtDO_REPL_STRINGSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The image as a data object.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRINGSink::SetTypedInput(avtDataObject_p in)
{
    if (*in != NULL && strcmp(in->GetType(), "avtDO_REPL_STRING") != 0)
    {
        //
        // Should create a new exception here, but I'm under time constraints.
        //
        debug1 << "Looking for avtDO_REPL_STRING, but found type \""
               << in->GetType() << "\"." << endl;
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink::GetInput
//
//  Purpose:
//      Gets the input of the sink (properly typed as an avtDataObject).
//
//  Returns:    The input of the sink.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDataObject_p
avtDO_REPL_STRINGSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}


