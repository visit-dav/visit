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
    haveIssuedWarning = false;
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
//      Reset haveIssuedWarning, so we can issue it only once per execution.
//
//  Programmer: Justin Privitera
//  Creation:   Wed Mar 30 11:46:46 PDT 2022
//
// ****************************************************************************

void
avtGhostZoneIdExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtGhostZoneIdExpression::DeriveVariable
//
//  Purpose:
//      TODO
//
//  Arguments:
//      vtkDataSet *in_ds        The input dataset.
//      int currentDomainsIndex  TODO
//
//  Returns:      TODO
//
//  Programmer:   Justin Privitera
//  Creation:     Wed Mar 30 11:50:02 PDT 2022
//
// ****************************************************************************

vtkDataArray *
avtGhostZoneIdExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType nvals = 0;
    // Q? hmmm which is it? - have a look at dataidexpr for an idea
    // nvals = in_ds->GetNumberOfCells();
    nvals = in_ds->GetNumberOfPoints();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(nvals);

    vtkDataArray *arr = NULL;
    arr = in_ds->GetPointData()->GetArray("avtGhostZones");

    // Q? could this ever be NULL? It doesn't look like it but I thought I should check
    // if (arr == NULL)
    // {
    //     for (vtkIdType i = 0 ; i < nvals ; i++)
    //         rv->SetValue(i, (int)i);
    //     char standard_msg[1024] = "VisIt was not able to create the requested"
    //             " ids.  Please see a VisIt developer.";
    //     char globalmsg[1024] = "VisIt was not able to create global ids, most "
    //             "likely because ids of this type were not stored in the file.";
    //     if (!haveIssuedWarning)
    //     {
    //         avtCallback::IssueWarning(
    //                            (doGlobalNumbering ? globalmsg : standard_msg));
    //         haveIssuedWarning = true;
    //     }

    //     return rv;
    // }

    rv->SetNumberOfTuples(nvals);

    for (vtkIdType i = 0 ; i < nvals ; i ++)
    {
        rv->SetValue(i, (int)arr->GetComponent(i, 0));
    }

    return rv;
}

// ****************************************************************************
//  Method: avtGhostZoneIdExpression::ModifyContract
//
//  Purpose:
//      TODO
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
