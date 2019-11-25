// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtEffectiveTensorExpression.C                      //
// ************************************************************************* //

#include <avtEffectiveTensorExpression.h>

#include <math.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEffectiveTensorExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEffectiveTensorExpression::avtEffectiveTensorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEffectiveTensorExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEffectiveTensorExpression::~avtEffectiveTensorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEffectiveTensorExpression::DoOperation
//
//  Purpose:
//      Determines the effective part of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Jan 30 11:38:50 PST 2004
//    Fix mistake in calculating ('4085).
//
//    Mark C. Miller, Mon Nov 11 14:21:38 PST 2019
//    Added comments to try to document what this function is computing.
// ****************************************************************************

void
avtEffectiveTensorExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomps, int ntuples)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);   
            double s11 = vals[0], s12 = vals[1], s13 = vals[2];
            double s21 = vals[3], s22 = vals[4], s23 = vals[5];
            double s31 = vals[6], s32 = vals[7], s33 = vals[8];

            // First invariant of the stress tensor
            // aka "pressure" of incompressible fluid in motion
            // aka "mean effective stress"
            double trace = (s11 + s22 + s33) / 3.;

            // components of the deviatoric stress
            double dev0 = s11 - trace;
            double dev1 = s22 - trace;
            double dev2 = s33 - trace;

            // The second invariant of the stress deviator
            // aka "J2"
            double out2 = 0.5*(dev0*dev0 + dev1*dev1 + dev2*dev2) +
                         s12*s12 + s13*s13 + s23*s23;

            // stress deviator
            out2 = sqrt(3.*out2);

            out->SetTuple1(i, out2);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


