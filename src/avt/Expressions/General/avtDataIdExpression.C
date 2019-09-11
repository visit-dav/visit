// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDataIdExpression.C                            //
// ************************************************************************* //

#include <avtDataIdExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>

#include <avtCallback.h>


// ****************************************************************************
//  Method: avtDataIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdExpression::avtDataIdExpression()
{
    doZoneIds = true;
    doGlobalNumbering = false;
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdExpression::~avtDataIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDataIdExpression::PreExecute
//
//  Purpose:
//      Reset haveIssuedWarning, so we can issue it only once per execution.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

void
avtDataIdExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 23, 2002
//
// ****************************************************************************

vtkDataArray *
avtDataIdExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType nvals = 0;
    if (doZoneIds)
        nvals = in_ds->GetNumberOfCells();
    else
        nvals = in_ds->GetNumberOfPoints();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(nvals);

    vtkDataArray *arr = NULL;
    if (doZoneIds && doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtGlobalZoneNumbers");
    else if (doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    else if (!doZoneIds && doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtGlobalNodeNumbers");
    else if (!doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtOriginalNodeNumbers");

    if (arr == NULL)
    {
        for (vtkIdType i = 0 ; i < nvals ; i++)
            rv->SetValue(i, (int)i);
        char standard_msg[1024] = "VisIt was not able to create the requested"
                " ids.  Please see a VisIt developer.";
        char globalmsg[1024] = "VisIt was not able to create global ids, most "
                "likely because ids of this type were not stored in the file.";
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning(
                               (doGlobalNumbering ? globalmsg : standard_msg));
            haveIssuedWarning = true;
        }

        return rv;
    }

    for (vtkIdType i = 0 ; i < nvals ; i++)
    {
        if (arr->GetNumberOfComponents() == 2)
            // When there are two components, they are typically packed as 
            // <dom, id>.  We will want the second one.
            rv->SetValue(i, (int)arr->GetComponent(i, 1));
        else
            rv->SetValue(i, (int)arr->GetComponent(i, 0));
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDataIdExpression::ModifyContract
//
//  Purpose:
//      Tells the input create zone ids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtContract_p
avtDataIdExpression::ModifyContract(avtContract_p spec)
{
    if (doZoneIds && doGlobalNumbering)
        spec->GetDataRequest()->TurnGlobalZoneNumbersOn();
    else if (doZoneIds && !doGlobalNumbering)
        spec->GetDataRequest()->TurnZoneNumbersOn();
    else if (!doZoneIds && doGlobalNumbering)
        spec->GetDataRequest()->TurnGlobalNodeNumbersOn();
    else if (!doZoneIds && !doGlobalNumbering)
        spec->GetDataRequest()->TurnNodeNumbersOn();

    return spec;
}


