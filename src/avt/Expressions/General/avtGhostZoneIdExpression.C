// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtGhostZoneIdExpression.C                       //
// ************************************************************************* //

#include <avtGhostZoneIdExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtGhostZoneIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   Wed Mar 30 11:46:46 PDT 2022
//
// ****************************************************************************

avtGhostZoneIdExpression::avtGhostZoneIdExpression()
{
    doGhostZoneIds = false;
}


// ****************************************************************************
//  Method: avtGhostZoneIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   Wed Mar 30 11:46:46 PDT 2022
//
// ****************************************************************************

avtGhostZoneIdExpression::~avtGhostZoneIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGhostZoneIdExpression::PreExecute
//
//  Purpose:
//      Call parent PreExecute().
//
//  Programmer: Justin Privitera
//  Creation:   Wed Mar 30 11:46:46 PDT 2022
//
// ****************************************************************************

void
avtGhostZoneIdExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtGhostZoneIdExpression::DeriveVariable
//
//  Purpose:      Gets the ghost zone ids and copies them to a vtkDataArray.
//
//  Arguments:
//      vtkDataSet *in_ds        The input dataset.
//      int currentDomainsIndex  unused integer
//
//  Returns:      A vtkDataArray containing the ghost zone ids.
//
//  Programmer:   Justin Privitera
//  Creation:     Wed Mar 30 11:50:02 PDT 2022
//
// ****************************************************************************

vtkDataArray *
avtGhostZoneIdExpression::DeriveVariable(vtkDataSet *in_ds, int /*currentDomainsIndex*/)
{
    vtkIdType nvals = 0;
    nvals = in_ds->GetNumberOfCells();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(nvals);

    vtkDataArray *arr = NULL;
    arr = in_ds->GetCellData()->GetArray("avtGhostZones");

    if (arr == NULL)
    {
        for (vtkIdType i = 0 ; i < nvals ; i ++)
        {
            rv->SetValue(i, 0);
        }
    }
    else
    {
        for (vtkIdType i = 0 ; i < nvals ; i ++)
        {
            rv->SetValue(i, (int)arr->GetComponent(i, 0));
        }
    }

    return rv;
}

// ****************************************************************************
//  Method: avtGhostZoneIdExpression::ModifyContract
//
//  Purpose:      Adds a data request to get the ghost zone ids to the 
//                avtContract pointer.
//
//  Programmer:   Justin Privitera
//  Creation:     Wed Mar 30 11:50:02 PDT 2022
//
// ****************************************************************************

avtContract_p
avtGhostZoneIdExpression::ModifyContract(avtContract_p spec)
{
    spec->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec;
}
