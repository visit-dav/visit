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
//  File: avtNamedSelectionFilter.C
// ************************************************************************* //

#include <avtNamedSelectionFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>

#include <avtNamedSelection.h>
#include <avtNamedSelectionManager.h>
#include <avtSILRestriction.h>


// ****************************************************************************
//  Method: avtNamedSelectionFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtNamedSelectionFilter::avtNamedSelectionFilter()
{
}


// ****************************************************************************
//  Method: avtNamedSelectionFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtNamedSelectionFilter::~avtNamedSelectionFilter()
{
}


// ****************************************************************************
//  Method: avtNamedSelectionFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the NamedSelection filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

vtkDataSet *
avtNamedSelectionFilter::ExecuteData(vtkDataSet *in_ds, int dom, std::string)
{
    int   i;

    //
    // See if the input reader applied the named selection on read.
    //
    if (selectionId >= 0)
        if (GetInput()->GetInfo().GetAttributes().GetSelectionApplied(selectionId))
            return in_ds;

    vtkDataArray *ocn=in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    if (ocn == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    unsigned int *ptr = (unsigned int *) ocn->GetVoidPointer(0);
    if (ptr == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    avtNamedSelection *ns = nsm->GetNamedSelection(selName);
    if (ns == NULL || ns->GetType() != avtNamedSelection::ZONE_ID)
    {
        EXCEPTION0(ImproperUseException);
    }

    avtZoneIdNamedSelection *zins = (avtZoneIdNamedSelection *) ns;
    vector<int> ids;
    int ncells = in_ds->GetNumberOfCells();
    zins->GetMatchingIds(ptr, ncells, ids);

    if (ids.size() == 0)
        return NULL;

    vtkDataSet *ds = in_ds->NewInstance();
    ds->ShallowCopy(in_ds);
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(ncells);
    arr->SetName("_avt_thresh_var");
    for (i = 0 ; i < ncells ; i++)
        arr->SetValue(i, 0.);
    for (i = 0 ; i < ids.size() ; i++)
        arr->SetValue(ids[i], 1.);
    ds->GetCellData()->AddArray(arr);
    arr->Delete();
    vtkThreshold *thres = vtkThreshold::New();
    thres->SetInput(ds);
    thres->ThresholdBetween(0.5, 1.5);
    thres->SetInputArrayToProcess(0, 0, 0, 
          vtkDataObject::FIELD_ASSOCIATION_CELLS, "_avt_thresh_var");
    vtkUnstructuredGrid *rv = thres->GetOutput();
    rv->Update();
    ManageMemory(rv);
    thres->Delete();
 
    return rv;
}


// ****************************************************************************
//  Method: avtNamedSelectionFilter::ModifyContract
//
//  Purpose:
//    Turn on Zone numbers flag if needed, so that original cell array
//    will be propagated throught the pipeline.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtContract_p
avtNamedSelectionFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = new avtContract(contract);
    rv->GetDataRequest()->TurnZoneNumbersOn();

    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    avtNamedSelection *ns = nsm->GetNamedSelection(selName);
    if (ns == NULL)
    {
        EXCEPTION1(VisItException, "The named selection was not valid");
    }

    vector<int> domains;
    bool useList = ns->GetDomainList(domains);
    if (useList)
    {
        rv->GetDataRequest()->GetRestriction()->RestrictDomains(domains);
    }

    avtDataSelection *ds = ns->CreateSelection();
    selectionId = -1;
    if (ds != NULL)
        selectionId = rv->GetDataRequest()->AddDataSelection(ds);
    
    return rv;
}


