// ************************************************************************* //
//                              avtDataIdFilter.C                            //
// ************************************************************************* //

#include <avtDataIdFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <avtCallback.h>


// ****************************************************************************
//  Method: avtDataIdFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdFilter::avtDataIdFilter()
{
    doZoneIds = true;
    doGlobalNumbering = false;
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdFilter::~avtDataIdFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDataIdFilter::PreExecute
//
//  Purpose:
//      Reset haveIssuedWarning, so we can issue it only once per execution.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

void
avtDataIdFilter::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdFilter::DeriveVariable
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
avtDataIdFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int nvals = 0;
    if (doZoneIds)
        nvals = in_ds->GetNumberOfCells();
    else
        nvals = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
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
        for (int i = 0 ; i < nvals ; i++)
            rv->SetTuple1(i, i);
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

    for (int i = 0 ; i < nvals ; i++)
    {
        if (arr->GetNumberOfComponents() == 2)
            // When there are two components, they are typically packed as 
            // <dom, id>.  We will want the second one.
            rv->SetTuple1(i, arr->GetComponent(i, 1));
        else
            rv->SetTuple1(i, arr->GetComponent(i, 0));
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDataIdFilter::PerformRestriction
//
//  Purpose:
//      Tells the input create zone ids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtDataIdFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (doZoneIds && doGlobalNumbering)
        spec->GetDataSpecification()->TurnGlobalZoneNumbersOn();
    else if (doZoneIds && !doGlobalNumbering)
        spec->GetDataSpecification()->TurnZoneNumbersOn();
    else if (!doZoneIds && doGlobalNumbering)
        spec->GetDataSpecification()->TurnGlobalNodeNumbersOn();
    else if (!doZoneIds && !doGlobalNumbering)
        spec->GetDataSpecification()->TurnNodeNumbersOn();

    return spec;
}


