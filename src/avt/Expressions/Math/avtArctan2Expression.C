// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtArctan2Expression.C                             //
// ************************************************************************* //

#include <avtArctan2Expression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtArctan2Expression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtArctan2Expression::avtArctan2Expression()
{
    ;
}


// ****************************************************************************
//  Method: avtArctan2Expression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtArctan2Expression::~avtArctan2Expression()
{
    ;
}


// ****************************************************************************
//  Method: avtArctan2Expression::DoOperation
//
//  Purpose:
//      Performs the atan2 operation to the two arguments.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern
//  Creation:   May 8, 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtArctan2Expression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                             vtkDataArray *out, int ncomponents,
                             int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val1 = in1->GetComponent(i, j);
            double val2 = in2->GetComponent(i, j);
            out->SetComponent(i, j, atan2(val1, val2));
        }
    }
}
