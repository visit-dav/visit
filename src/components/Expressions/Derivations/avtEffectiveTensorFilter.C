// ************************************************************************* //
//                           avtEffectiveTensorFilter.C                      //
// ************************************************************************* //

#include <avtEffectiveTensorFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEffectiveTensorFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEffectiveTensorFilter::avtEffectiveTensorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtEffectiveTensorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEffectiveTensorFilter::~avtEffectiveTensorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtEffectiveTensorFilter::DoOperation
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
// ****************************************************************************

void
avtEffectiveTensorFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomps, int ntuples)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float *vals = in->GetTuple9(i);   

            double trace = -(vals[0] + vals[4] + vals[8]) / 3.;
            double dev0 = vals[0] + trace;
            double dev1 = vals[4] + trace;
            double dev2 = vals[8] + trace;

            float out2 = 0.5*(dev0*dev0 + dev1*dev1 + dev2*dev2) +
                         vals[1]*vals[1] + vals[2]*vals[2] +
                         vals[5]*vals[5];
            out2 = sqrt(3.*out2);
            out->SetTuple1(i, out2);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


