// ************************************************************************* //
//                             avtMagnitudeFilter.C                          //
// ************************************************************************* //

#include <avtMagnitudeFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtMagnitudeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeFilter::avtMagnitudeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMagnitudeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeFilter::~avtMagnitudeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMagnitudeFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Matthew Haddox
//  Creation:     July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 15:11:11 PST 2002
//    Beefed up error code a little.
//
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api. 
//
// ****************************************************************************

vtkDataArray *
avtMagnitudeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *vectorValues = in_ds->GetPointData()->GetVectors();
    if (vectorValues == NULL)
    {
        vectorValues = in_ds->GetCellData()->GetVectors();
    }
    if (vectorValues == NULL)
    {
        EXCEPTION1(ExpressionException, "Unable to locate variable for "
                                        "magnitude expression");
    }

    if (vectorValues->GetNumberOfComponents() != 3)
    {
        EXCEPTION1(ExpressionException, "Can only take magnitude of vectors.");
    }
    int ntuples = vectorValues->GetNumberOfTuples();

    vtkDataArray *results = vectorValues->NewInstance();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(ntuples);
    
    for (int i = 0 ; i < ntuples ; i++)
    {
        float *x   = vectorValues->GetTuple(i);
        float  mag = sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
        results->SetTuple(i, &mag);
    }

    return results;
}


