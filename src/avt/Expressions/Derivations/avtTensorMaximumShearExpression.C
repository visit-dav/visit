// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtTensorMaximumShearExpression.C                      //
// ************************************************************************* //

#include <avtTensorMaximumShearExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTensorMaximumShearExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTensorMaximumShearExpression::avtTensorMaximumShearExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTensorMaximumShearExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTensorMaximumShearExpression::~avtTensorMaximumShearExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTensorMaximumShearExpression::DoOperation
//
//  Purpose:
//      Determines the maximum shear of a tensor.
//
//  Implements method described in...
//
//    J.C. Ugural and S.K. Fenster "Advanced Strength and Applied Elasticity",
//    Prentice Hall 4th Edition, Page 81
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Jun 17 08:16:40 PDT 2004
//    Fix typo that caused bad derivation.
//
//    Hank Childs, Fri Jun  9 14:22:43 PDT 2006
//    Comment out currently unused variable.
//
//    Eric Brugger, Mon Aug  8 09:32:30 PDT 2011
//    I reduced the tolerance on invariant1 to be less restrictive.
//
// ****************************************************************************

void
avtTensorMaximumShearExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomps, int ntuples)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            // 9 components of stress tensor
            double *vals = in->GetTuple9(i);   
            double s11 = vals[0], s12 = vals[1], s13 = vals[2];
            double s21 = vals[3], s22 = vals[4], s23 = vals[5];
            double s31 = vals[6], s32 = vals[7], s33 = vals[8];

            // Hydro-static component
            double pressure = (s11 + s22 + s33) / 3.;

            // Deviatoric stress components
            double dev0 = s11 - pressure;
            double dev1 = s22 - pressure;
            double dev2 = s33 - pressure;

            // double invariant0 = dev0 + dev1 + dev2;
            // Second invariant of stress deviator
            double invariant1 = 0.5*(dev0*dev0 + dev1*dev1 + dev2*dev2);
            invariant1 += s12*s12 + s13*s13 + s23*s23;

            // Third invariant of stress deviator
            double invariant2 = -dev0*dev1*dev2;
            invariant2 += -2.0*s12*s13*s23;
            invariant2 +=     dev0*s23*s23;
            invariant2 +=     dev1*s13*s13;
            invariant2 +=     dev2*s12*s12;

            // Cubic roots of the characteristic equation
            // http://mathworld.wolfram.com/CubicFormula.html
            double princ0 = 0.;
            double princ2 = 0.;
            if (invariant1 >= 1e-100)
            {
                double alpha = -0.5*sqrt(27./invariant1)
                                  *invariant2/invariant1;
                if (alpha < 0.)
                    alpha = (alpha < -1. ? -1 : alpha);
                if (alpha > 0.)
                    alpha = (alpha > +1. ? +1 : alpha);

                double angle = acos((double)alpha) / 3.;
                double value = 2.0 * sqrt(invariant1 / 3.);
                princ0 = value*cos(angle);
                // Displace the angle for princ1 (which we don't calculate)
                angle = angle - 2.0*vtkMath::Pi()/3.;
                // Now displace for princ2
                angle = angle + 4.0*vtkMath::Pi()/3.;
                princ2 = value*cos(angle);
            }

            // set the output value
            out->SetTuple1(i, (princ0 - princ2) * 0.5);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


