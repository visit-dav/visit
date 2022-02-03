// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtNamedSelectionFilter.C
// ************************************************************************* //

#include <avtNamedSelectionFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>

#include <avtNamedSelection.h>
#include <avtNamedSelectionManager.h>
#include <avtSILRestriction.h>

#include <vector>


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
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//    Brad Whitlock, Mon Jun 20 17:06:33 PST 2011
//    Delete the copied dataset so we don't leak memory.
//
//    Brad Whitlock, Thu Oct 27 15:51:06 PDT 2011
//    Extend to other selection types.
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Support for location named selections.
//
//    Eric Brugger, Mon Jul 21 16:42:08 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Tue Aug 25 10:32:57 PDT 2015
//    Modified the routine to return NULL if the output data set was NULL.
//
// ****************************************************************************

avtDataRepresentation *
avtNamedSelectionFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    //
    // See if the input reader applied the named selection on read.
    //
    if (selectionId >= 0)
        if (GetInput()->GetInfo().GetAttributes().GetSelectionApplied(selectionId))
            return in_dr;

    vtkDataSet *out_ds = NULL;
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    avtNamedSelection *ns = nsm->GetNamedSelection(selName);
    if(ns == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    else
    {
        std::vector<vtkIdType> ids;
        ns->GetMatchingIds(in_ds, ids);
        out_ds = SelectedData(in_ds, ids);
    }

    avtDataRepresentation *out_dr = NULL;
    if (out_ds != NULL)
    {
        out_dr = new avtDataRepresentation(out_ds,
            in_dr->GetDomain(), in_dr->GetLabel());
        out_ds->Delete();
    }

    return out_dr;
}

// ****************************************************************************
//  Method: avtNamedSelectionFilter::SelectedData
//
//  Purpose:
//      Isolate the specified cell ids from the input dataset.
//
//  Arguments:
//      in_ds      The input dataset.
//      ids        The cell ids of the data we care about.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//    Brad Whitlock, Mon Jun 20 17:06:33 PST 2011
//    I moved this code out from another function to make it more general.
//
//    Kathleen Biagas, Mon Jan 28 10:56:30 PST 2013
//    Call upate on filter not data object.
//
//    Eric Brugger, Mon Jul 21 16:42:08 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

vtkDataSet *
avtNamedSelectionFilter::SelectedData(vtkDataSet *in_ds, 
    const std::vector<vtkIdType> &ids)
{
    vtkDataSet *rv = NULL;

    if (!ids.empty())
    {
        vtkIdType ncells = in_ds->GetNumberOfCells();
        vtkDataSet *ds = in_ds->NewInstance();
        ds->ShallowCopy(in_ds);
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(ncells);
        arr->SetName("_avt_thresh_var");
        for (vtkIdType i = 0 ; i < ncells ; i++)
            arr->SetValue(i, 0.);
        for (vtkIdType i = 0 ; i < (vtkIdType)ids.size() ; i++)
            arr->SetValue(ids[i], 1.);
        ds->GetCellData()->AddArray(arr);
        arr->Delete();
        vtkThreshold *thres = vtkThreshold::New();
        thres->SetInputData(ds);
        thres->SetLowerThreshold(0.5);
        thres->SetUpperThreshold(1.5);
        thres->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
        thres->SetInputArrayToProcess(0, 0, 0, 
              vtkDataObject::FIELD_ASSOCIATION_CELLS, "_avt_thresh_var");
        thres->Update();
        rv = thres->GetOutput();
        rv->Register(NULL);
        thres->Delete();
        ds->Delete();
    }
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
//  Modifications:
//    Brad Whitlock, Mon Nov  7 13:36:54 PST 2011
//    I moved some code into the named selections so they can modify the 
//    contract as needed.
//
// ****************************************************************************

avtContract_p
avtNamedSelectionFilter::ModifyContract(avtContract_p contract)
{
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    avtNamedSelection *ns = nsm->GetNamedSelection(selName);
    if (ns == NULL)
    {
        EXCEPTION1(VisItException, "The named selection was not valid");
    }

    // Let the named selection derived types modify the contract.
    avtContract_p rv = ns->ModifyContract(contract);

    // Try and apply a data selection based on the named selection.
    avtDataSelection *ds = ns->CreateSelection();
    selectionId = -1;
    if (ds != NULL)
        selectionId = rv->GetDataRequest()->AddDataSelection(ds);
    
    return rv;
}

// ****************************************************************************
//  Method: avtNamedSelectionFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Changes to the output.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//

void
avtNamedSelectionFilter::UpdateDataObjectInfo(void)
{
    avtDataValidity &outValidity = GetOutput()->GetInfo().GetValidity();
   
    // We need to set these as invalid, or else caching could
    // kick in and we might end up using acceleration structures
    // across pipeline executions that were no longer valid.
    outValidity.InvalidateNodes();
    outValidity.InvalidateZones();
}
