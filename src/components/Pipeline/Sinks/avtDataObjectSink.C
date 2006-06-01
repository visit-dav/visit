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
//                            avtDataObjectSink.C                            //
// ************************************************************************* //

#include <avtDataObjectSink.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDataObjectSink constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSink::avtDataObjectSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSink::~avtDataObjectSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink::SetInput
//
//  Purpose:
//      Sets the input of the data object.  This only takes care of bookkeeping
//      and passes the type checking on to the derived types.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

void
avtDataObjectSink::SetInput(avtDataObject_p in)
{
    avtDataObject_p currentInput = GetInput();
    if (*in == *currentInput)
    {
        debug1 << "A data object sink's current input was fed back into "
               << "itself, ignoring..." << endl;
        return;
    }

    //
    // This will throw an exception if any problems occur, so we can just
    // assume that it works.
    //
    SetTypedInput(in);

    ChangedInput();
}


// ****************************************************************************
//  Method: avtDataObjectSink::ChangedInput
//
//  Purpose:
//      This is a hook for derived types to let them know that their input has
//      changed.  It is defined here to prevent all derived types from defining
//      it.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

void
avtDataObjectSink::ChangedInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink::UpdateInput
//
//  Purpose:
//      Updates the input.  This is defined here for all sinks that have one
//      input (ie all of them except the multiple input sink).
//
//  Arguments:
//      spec    The pipeline specification.
//
//  Returns:    Whether or not anything was modified up stream.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

bool
avtDataObjectSink::UpdateInput(avtPipelineSpecification_p spec)
{
    return GetInput()->Update(spec);
}


