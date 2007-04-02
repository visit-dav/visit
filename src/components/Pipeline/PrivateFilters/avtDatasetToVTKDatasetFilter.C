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
//                       avtDatasetToVTKDatasetFilter.C                      //
// ************************************************************************* //

#include <vtkAppendFilter.h>

#include <avtCommonDataFunctions.h>
#include <avtDomainList.h>
#include <IncompatibleDomainListsException.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <avtDatasetToVTKDatasetFilter.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetToVTKDatasetFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

avtDatasetToVTKDatasetFilter::avtDatasetToVTKDatasetFilter()
{
    bridge       = vtkAVTPipelineBridge::New();
    bridge->SetFilter(this);
    appendFilter = vtkAppendFilter::New();

    //
    // We want the outputted dataset to believe its source is really the
    // bridge even though it comes from the append filter.  Get the output
    // from the appender and set the bridge to have the same output.  Finally,
    // tell the dataset that its source is actually the bridge.
    //
    vtkUnstructuredGrid *ds = appendFilter->GetOutput();
    bridge->SetOutput(ds);
    ds->SetSource(bridge);
}


// ****************************************************************************
//  Method: avtDatasetToVTKDatasetFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

avtDatasetToVTKDatasetFilter::~avtDatasetToVTKDatasetFilter()
{
    if (bridge != NULL)
    {
        bridge->Delete();
    }
    if (appendFilter != NULL)
    {
        appendFilter->Delete();
    }
}


// ****************************************************************************
//  Method: avtDatasetToVTKDatasetFilter::GetVTKOutput
//
//  Purpose:
//      Gets the dataset outputted from the bridge/append filter.
//
//  Returns:    A vtkDataSet.  This will be run through an append filter if
//              there is more than one block.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

vtkDataSet *
avtDatasetToVTKDatasetFilter::GetVTKOutput(void)
{
    //
    // The output passes between the append filter and the bridge (alas it
    // cannot belong to both).  This module keeps it in the bridge when the
    // append filter is not using it, so get it from the bridge.
    //
    return bridge->GetOutput();
}


// ****************************************************************************
//  Method: avtDatasetToVTKDatasetFilter::CalcDomainList
//
//  Purpose:
//      Defines pure virtual function from base type.  This sets its domain 
//      list to want all domains.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

void
avtDatasetToVTKDatasetFilter::CalcDomainList(void)
{
    domains->UseAllDomains();
}


// ****************************************************************************
//  Method: avtDatasetToVTKDatasetFilter::Execute
//
//  Purpose:
//      Actually sets up the input to the append filter and forces it to
//      re-execute.
//
//  Arguments:
//      dl      The domain list that this filter should execute on.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 26 09:42:26 PDT 2000
//    Added call to GetDomainVTK since GetInputDomain returns an avtDomain.
//
//    Kathleen Bonnell, Thu Feb  8 13:32:17 PST 2001 
//    Added call to GetDomain since GetInputDomain returns an avtDomainTree.
//
//    Kathleen Bonnell, Mon Apr  9 13:15:11 PDT 2001 
//    Reflect that data stored as single tree, with domains as children.
//
// ****************************************************************************

void
avtDatasetToVTKDatasetFilter::Execute(avtDomainList *dl)
{
    int   i;

    //
    // We previously set the dataset that was naturally the output of the
    // append filter as the output of the bridge.  Because VTK does not want
    // to have two objects share that output, it has code under the covers
    // that removes the dataset as output from the append filter.  Restore
    // that now.
    //
    appendFilter->SetOutput(bridge->GetOutput());

    //
    // Remove all of the inputs from the appender.  Count down so there are
    // no issues with the array being squeezed and indices changing.
    //
    int numInputs = appendFilter->GetNumberOfInputs();
    for (i = numInputs-1 ; i >= 0 ; i--)
    {
        vtkDataSet *ds = appendFilter->GetInput(i);
        if (ds != NULL)
        {
            appendFilter->RemoveInput(ds);
        }
    }

    //
    // Set the input to the appender to fit with the domain list.
    //
    
    avtDataTree_p tree = GetInputDataTree();
    int nc = tree->GetNChildren();
    int nd = dl->NumDomains();
    if (nd != nc)
    {
        EXCEPTION2(IncompatibleDomainListsException, nd, nc);
    }

    for (i = 0 ; i < dl->NumDomains() ; i++)
    {
        if (dl->Domain(i) && tree->ChildIsPresent(i))
        {
            bool dummy;
            tree->GetChild(i)->Traverse(CAddInputToAppendFilter, 
                                        appendFilter, dummy);
        }
    }

    //
    // Force an execution of the append filter.  We set all of the input to its
    // inputs as NULL, so we don't have to worry about it trying to go back up
    // the (most certainly defunct) pipeline.
    //
    appendFilter->Update();

    //
    // The output of the bridge and the append filter is shared.  Give it back
    // to the bridge, so our pipeline will work properly.
    //
    bridge->SetOutput(appendFilter->GetOutput());
}


// ****************************************************************************
//  Method: vtkAVTPipelineBridge::New
//
//  Purpose:
//      Follow VTK's memory allocation pattern.
//
//  Returns:    An instance to a vtkAVTPipelineBridge object.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

vtkAVTPipelineBridge *
vtkAVTPipelineBridge::New()
{
    return new vtkAVTPipelineBridge;
}


// ****************************************************************************
//  Method: vtkAVTPipelineBridge constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
// 
// ****************************************************************************

vtkAVTPipelineBridge::vtkAVTPipelineBridge()
{
    filter = NULL;
}


// ****************************************************************************
//  Method: vtkAVTPipelineBridge::SetFilter
//
//  Purpose:
//      Sets the avt filter associated with this object.
//
//  Arguments:
//      f       The filter this object should notify on updates.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

void
vtkAVTPipelineBridge::SetFilter(avtDatasetToVTKDatasetFilter *f)
{
    filter = f;
}


// ****************************************************************************
//  Method: vtkAVTPipelineBridge::UpdateData
//
//  Purpose:
//      Called by the its "output".  The output is actually stolen from the
//      append filter, but the output has been duped to believe that this
//      object is actually its source.  It relays the message onto the
//      avtDatasetToVTKDatasetFilter object.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

void
vtkAVTPipelineBridge::UpdateData(vtkDataObject *)
{
    debug4 << "Bridge starting AVT pipeline." << endl;
    filter->StartUpdate();
}
