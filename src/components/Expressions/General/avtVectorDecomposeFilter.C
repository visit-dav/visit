// ************************************************************************* //
//                         avtVectorDecomposeFilter.C                        //
// ************************************************************************* //

#include <avtVectorDecomposeFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorDecomposeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorDecomposeFilter::avtVectorDecomposeFilter(int w)
{
    which_comp = w;
}


// ****************************************************************************
//  Method: avtVectorDecomposeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorDecomposeFilter::~avtVectorDecomposeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorDecomposeFilter::GetVariableDimension
//
//  Purpose:
//      Does its best to guess at the variable dimension will be after
//      decomposing a "vector".
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb 26 09:00:06 PST 2004
//    Account for multiple variables.
//
//    Hank Childs, Thu May  6 10:49:32 PDT 2004
//    Make sure that we are getting the dimension for the active variable.
//
// ****************************************************************************

int
avtVectorDecomposeFilter::GetVariableDimension(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (activeVariable == NULL)
        return 1;
    if (!atts.ValidVariable(activeVariable))
        return 1;

    int inDim = atts.GetVariableDimension(activeVariable);
    if (inDim == 9)
        return 3;
    else if (inDim == 3)
        return 1;

    // Who knows?!?
    return 1;
}


// ****************************************************************************
//  Method: avtVectorDecomposeFilter::DeriveVariable
//
//  Purpose:
//      Pulls out a component of a vector variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api.
//
//    Sean Ahern, Thu Mar  6 02:00:32 America/Los_Angeles 2003
//    Merged the vector component filters together.
//
//    Hank Childs, Fri Sep 19 15:00:11 PDT 2003
//    Account for tensor data as well.
//
// ****************************************************************************

vtkDataArray *
avtVectorDecomposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *arr = NULL;

    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;

    //
    // Get the array of interest.
    //
    if (in_ds->GetPointData()->GetVectors() != NULL)
    {
        arr = in_ds->GetPointData()->GetArray(varname);
    }
    else
    {
        arr = in_ds->GetCellData()->GetArray(varname);
    }
    if (arr == NULL)
    {
        EXCEPTION1(ExpressionException, "Cannot locate variable");
    }

    //
    // The logic here can be pretty tricky.  We want to decompose this "vector"
    // variable.  But the "vector" can be an everyday vector, or it can be a
    // symmetric on non-symmetric tensor.  Based on the dimensionality of the
    // dataset, the output could be a scalar or it could be a vector.
    //
    int ntuples = arr->GetNumberOfTuples();
    vtkDataArray *rv = arr->NewInstance();

    bool twoDVector =
            (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2);

    if (twoDVector)
    {
        if ((which_comp > 1) || (which_comp < 0))
        {
            EXCEPTION1(ExpressionException, "The only valid indices for "
                         "2D vectors are 0 and 1.");
        }
        if (arr->GetNumberOfComponents() == 3)
        {
            rv->SetNumberOfComponents(1);
            rv->SetNumberOfTuples(ntuples);
            for (int i = 0 ; i < ntuples ; i++)
            {
                float val = arr->GetComponent(i, which_comp);
                rv->SetTuple1(i, val);
            }
        }
        else if (arr->GetNumberOfComponents() == 9)
        {
            //
            // Give one row of the tensor back.  Since VTK dislikes vectors
            // of size 2, make sure that we have 3 components.
            //
            rv->SetNumberOfComponents(3);
            rv->SetNumberOfTuples(ntuples);
            for (int i = 0 ; i < ntuples ; i++)
            {
                float val1 = arr->GetComponent(i, 2*which_comp);
                float val2 = arr->GetComponent(i, 2*which_comp+1);
                rv->SetTuple3(i, val1, val2, 0.);
            }
        }
        else
        {
            EXCEPTION1(ExpressionException, "You can only decompose vectors "
                        "and tensors.");
        }
    }
    else
    {
        if ((which_comp > 2) || (which_comp < 0))
        {
            EXCEPTION1(ExpressionException, "The only valid indices for "
                        "3D vectors are 0, 1, and 2");
        }
        if (arr->GetNumberOfComponents() == 3)
        {
            //
            // Looks like an everyday 3D vector.
            //
            rv->SetNumberOfComponents(1);
            rv->SetNumberOfTuples(ntuples);
            for (int i = 0 ; i < ntuples ; i++)
            {
                float val = arr->GetComponent(i, which_comp);
                rv->SetTuple1(i, val);
            }
        }
        else if (arr->GetNumberOfComponents() == 9)
        {
            //
            // Tensor data.  Return a row (*X, *Y, *Z).
            //
            rv->SetNumberOfComponents(3);
            rv->SetNumberOfTuples(ntuples);
            for (int i = 0 ; i < ntuples ; i++)
            {
                float val1 = arr->GetComponent(i, 3*which_comp);
                float val2 = arr->GetComponent(i, 3*which_comp+1);
                float val3 = arr->GetComponent(i, 3*which_comp+2);
                rv->SetTuple3(i, val1, val2, val3);
            }
        }
        else
        {
            EXCEPTION1(ExpressionException, "You can only decompose vectors "
                        "and tensors.");
        }
    }

    return rv;
}


