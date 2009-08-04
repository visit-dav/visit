/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtTensorMaximumShearExpression.C                      //
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
// ****************************************************************************

void
avtTensorMaximumShearExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomps, int ntuples)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);   

            double pressure = -(vals[0] + vals[4] + vals[8]) / 3.;
            double dev0 = vals[0] + pressure;
            double dev1 = vals[4] + pressure;
            double dev2 = vals[8] + pressure;

            // double invariant0 = dev0 + dev1 + dev2;
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

            out->SetTuple1(i, (princ0 - princ2) * 0.5);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


