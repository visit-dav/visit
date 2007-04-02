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
//                            avtMultipleInputSink.C                         //
// ************************************************************************* //

#include <avtMultipleInputSink.h>

#include <avtDatasetSink.h>

#include <BadIndexException.h>


// ****************************************************************************
//  Method: avtMultipleInputSink constructor
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

avtMultipleInputSink::avtMultipleInputSink()
{
    sinks  = NULL;
    nSinks = 0;
}


// ****************************************************************************
//  Method: avtMultipleInputSink destructor
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

avtMultipleInputSink::~avtMultipleInputSink()
{
    if (sinks != NULL)
    {
        // Delete each sink in the array?
        delete [] sinks;
        sinks = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetNumSinks
//
//  Purpose:
//      Sets the number of sinks for this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetNumSinks(int ns)
{
    if (sinks != NULL)
    {
        delete [] sinks;
    }

    nSinks = ns;

    sinks = new avtDataObjectSink*[nSinks];
    for (int i = 0 ; i < nSinks ; i++)
    {
        sinks[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetSink
//
//  Purpose:
//      Sets a sink as one of the sinks for the multiple inputs.
//
//  Arguments:
//      s       The sink.
//      n       The sink index.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetSink(avtDataObjectSink *s, int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    // Delete old sink?
    sinks[n] = s;
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetNthInput
//
//  Purpose:
//      Sets the input to one of the data object sinks.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetNthInput(avtDataObject_p input, int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    sinks[n]->SetInput(input);
}


// ****************************************************************************
//  Method: avtMultipleInputSink::GetInput
//
//  Purpose:
//      The GetInput routine must be defined for sinks.  Meet that interface.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

avtDataObject_p
avtMultipleInputSink::GetInput(void)
{
    if (nSinks <= 0)
    {
        EXCEPTION2(BadIndexException, 0, nSinks);
    }

    return sinks[0]->GetInput();
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetTypedInput
//
//  Purpose:
//      The SetTypedInput routine must be defined for sinks.  Meet that
//      interface.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetTypedInput(avtDataObject_p dob)
{
    if (nSinks <= 0)
    {
        EXCEPTION2(BadIndexException, 0, nSinks);
    }

    sinks[0]->SetTypedInput(dob);
}


// ****************************************************************************
//  Method: avtMultipleInputSink::GetNthInput
//
//  Purpose:
//      Gets the input to one of the data object sinks.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

avtDataObject_p
avtMultipleInputSink::GetNthInput(int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    return sinks[n]->GetInput();
}


// ****************************************************************************
//  Method: avtMultipleInputSink::UpdateInput
//
//  Purpose:
//      Does an Update on all of the inputs.
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
avtMultipleInputSink::UpdateInput(avtPipelineSpecification_p spec)
{
    bool modifiedUpstream = false;
    for (int i = 0 ; i < nSinks ; i++)
    {
        avtDataObject_p dob = sinks[i]->GetInput();
        bool modifiedHere = dob->Update(spec);
        modifiedUpstream = modifiedUpstream || modifiedHere;
    }

    return modifiedUpstream;
}


// ****************************************************************************
//  Method: avtMultipleInputSink::GetTreeFromSink
//
//  Purpose:
//      Gets a data tree from a dataset sink.
//      This is leveraging the friend access of the multiple input sink.
//
//  NOTE:       I feel pretty bad about doing this.  This is essentially
//              breaking encapsulation.  Pretty unavoidable with the current
//              design.  The "right" thing to do is to somehow allow the
//              multiple input sink to have friend access to the data object
//              sink.  However, it cannot access data trees at that level.
//              So the MISink has to know about the derived type
//              avtDatasetSink.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

avtDataTree_p
avtMultipleInputSink::GetTreeFromSink(avtDatasetSink *dss)
{
    return dss->GetInputDataTree();
}


