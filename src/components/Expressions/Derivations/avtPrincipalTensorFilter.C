// ************************************************************************* //
//                           avtPrincipalTensorFilter.C                      //
// ************************************************************************* //

#include <avtPrincipalTensorFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtPrincipalTensorFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPrincipalTensorFilter::avtPrincipalTensorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtPrincipalTensorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPrincipalTensorFilter::~avtPrincipalTensorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtPrincipalTensorFilter::DoOperation
//
//  Purpose:
//      Determines the principals of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Jun 17 08:16:40 PDT 2004
//    Fix typo that caused bad derivation.
//
// ****************************************************************************

void
avtPrincipalTensorFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomps, int ntuples)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float *vals = in->GetTuple9(i);   

            double pressure = -(vals[0] + vals[4] + vals[8]) / 3.;
            double dev0 = vals[0] + pressure;
            double dev1 = vals[4] + pressure;
            double dev2 = vals[8] + pressure;

            double invariant0 = dev0 + dev1 + dev2;
            double invariant1 = 0.5*(dev0*dev0 + dev1*dev1 + dev2*dev2);
            invariant1 += vals[1]*vals[1] + vals[2]*vals[2] + vals[5]*vals[5];
            double invariant2 = -dev0*dev1*dev2;
            invariant2 += -2.0 *vals[1]*vals[2]*vals[5];
            invariant2 += dev0*vals[5]*vals[5];
            invariant2 += dev1*vals[2]*vals[2];
            invariant2 += dev2*vals[1]*vals[1];

            double princ0 = 0.;
            double princ1 = 0.;
            double princ2 = 0.;
            if (invariant1 >= 1e-7)
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
                angle = angle - 2.0*vtkMath::Pi()/3.;
                princ1 = value*cos(angle);
                angle = angle + 4.0*vtkMath::Pi()/3.;
                princ2 = value*cos(angle);
            }

            float out3[3];
            out3[0] = princ0-pressure;
            out3[1] = princ1-pressure;
            out3[2] = princ2-pressure;
            out->SetTuple(i, out3);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


