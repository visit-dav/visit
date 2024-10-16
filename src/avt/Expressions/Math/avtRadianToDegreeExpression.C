// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtRadianToDegreeExpression.C                    //
// ************************************************************************* //

#include <avtRadianToDegreeExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#if defined(_WIN32) && !defined(M_PI)
// Windows does not seem to have this in math.h
#define M_PI 3.14159265358979323846
#endif


// ****************************************************************************
//  Method: avtRadianToDegreeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRadianToDegreeExpression::avtRadianToDegreeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRadianToDegreeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRadianToDegreeExpression::~avtRadianToDegreeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRadianToDegreeExpression::DoOperation
//
//  Purpose:
//      Converts radian angles to degree angles.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

void
avtRadianToDegreeExpression::DoOperation(vtkDataArray *in, vtkDataArray *out, 
                                     int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            out->SetComponent(i, j, val*(360./(2.*M_PI)));
        }
    }
}


