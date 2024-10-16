// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTimeExpression.C                           //
// ************************************************************************* //

#include <avtTimeExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Method: avtTimeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 11 12:35:29 EDT 2009
//    Added support for cycle and timestep values.
//
// ****************************************************************************

avtTimeExpression::avtTimeExpression(TimeMode m)
{
    mode = m;
}


// ****************************************************************************
//  Method: avtTimeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
// ****************************************************************************

avtTimeExpression::~avtTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeExpression::DoOperation
//
//  Purpose:
//      Generates new constants (the time).
//
//  Arguments:
//      in            The input data array (ignored).
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 11 12:35:29 EDT 2009
//    Added support for cycle and timestep values.
//
// ****************************************************************************
 
void
avtTimeExpression::DoOperation(vtkDataArray *, vtkDataArray *out,
                               int /*ncomponents*/, int ntuples, vtkDataSet *in_ds)
{
    double val = 0;
    switch (mode)
    {
      case MODE_TIME:
        val = (double) GetInput()->GetInfo().GetAttributes().GetTime();
        break;
      case MODE_CYCLE:
        val = (double) GetInput()->GetInfo().GetAttributes().GetCycle();
        break;
      case MODE_INDEX:
        val = (double)(GetInput()->GetContractFromPreviousExecution()->GetDataRequest()->GetTimestep());
        break;
    }
    for (int i = 0 ; i < ntuples ; i++)
        out->SetTuple1(i, val);
}


// ****************************************************************************
//  Method: avtTimeExpression::CreateArray
//
//  Purpose:
//      Creates an array of constant value.  Note that this routine will not
//      return uchar arrays.  The motivation for this is that most uchar arrays
//      come from the conditional expression (if), which uses a uchar as the
//      conditional, but really wants floats as the then- and else- variables.
//      (ie if(gt(X, 2.), 1., 0.) -- the constant 1. should be a double
//      (or whatever X is, not a uchar).
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2006
//
// ****************************************************************************

vtkDataArray *
avtTimeExpression::CreateArray(vtkDataArray *in)
{
    if (in->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        return vtkDoubleArray::New();
    }

    return in->NewInstance();
}


