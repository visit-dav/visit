// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtConstantCreatorExpression.C                    //
// ************************************************************************* //

#include <avtConstantCreatorExpression.h>

#include <vtkDataArray.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtConstantCreatorExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:04:05 PDT 2020
//    Setting canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtConstantCreatorExpression::avtConstantCreatorExpression()
{
    value = 0;
    isSingleton = true;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtConstantCreatorExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConstantCreatorExpression::~avtConstantCreatorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConstantCreatorExpression::DoOperation
//
//  Purpose:
//      Generates new constants.
//
//  Arguments:
//      in            The input data array (ignored).
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern
//  Creation:   Sat Feb 22 00:42:35 America/Los_Angeles 2003
//
//  Modifications:
//
// ****************************************************************************
 
void
avtConstantCreatorExpression::DoOperation(vtkDataArray *, vtkDataArray *out,
                                      int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
        out->SetTuple1(i, value);
}


// ****************************************************************************
//  Method: avtConstantCreatorExpression::CreateArray
//
//  Purpose:
//      Creates an array of constant value.  Note that this routine will not
//      return uchar arrays.  The motivation for this is that most uchar arrays
//      come from the conditional expression (if), which uses a uchar as the
//      conditional, but really wants floats as the then- and else- variables.
//      (ie if(gt(X, 2.), 1., 0.) -- the constant 1. should be a float
//      (or whatever X is, not a uchar).
//
//  Programmer: Hank Childs
//  Creation:   November 3, 2003
//
//  Modifications:
//    Brad Whitlock, Wed Apr 18 13:35:09 PDT 2012
//    Return float unless the input is double.
//
// ****************************************************************************

vtkDataArray *
avtConstantCreatorExpression::CreateArray(vtkDataArray *in)
{
    if (in->GetDataType() == VTK_DOUBLE)
        return in->NewInstance();

    return vtkFloatArray::New();
}


