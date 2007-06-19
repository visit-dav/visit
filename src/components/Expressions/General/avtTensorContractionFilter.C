/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtTensorContractionFilter.C                      //
// ************************************************************************* //

#include <avtTensorContractionFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTensorContractionFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

avtTensorContractionFilter::avtTensorContractionFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTensorContractionFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

avtTensorContractionFilter::~avtTensorContractionFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtPrincipalTensorFilter::DoOperation
//
//  Purpose:
//      Calculates the contraction of a tensor
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

void
avtTensorContractionFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                        int ncomps, int ntuples)
{
    if (ncomps == 9)
    {

        for (int i = 0 ; i < ntuples ; i++)
        {
            double ctract = 0.0;
            double *vals = in->GetTuple9(i);

            //
            // For a rank 2 tensor, the contraction collapses to a scalar.
            // Conceptually it is like as doting each column vector with
            // itself and adding the column results
            //

            ctract +=vals[0] * vals[0] + vals[1] * vals[1] + vals[2] * vals[2];
            ctract +=vals[3] * vals[3] + vals[4] * vals[4] + vals[5] * vals[5];
            ctract +=vals[6] * vals[6] + vals[7] * vals[7] + vals[8] * vals[8];

            out->SetTuple(i, &ctract);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


