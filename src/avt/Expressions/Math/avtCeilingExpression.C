// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCeilingExpression.C                        //
// ************************************************************************* //

#include <avtCeilingExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtCeilingExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtCeilingExpression::avtCeilingExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCeilingExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtCeilingExpression::~avtCeilingExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCeilingExpression::DoOperation
//
//  Purpose:
//      Takes the ceiling of each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************
 
void
avtCeilingExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                  int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            out->SetComponent(i, j, ceil(val));
        }
    }
}


