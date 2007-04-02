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
//                             avtMinMaxExpression.C                         //
// ************************************************************************* //

#include <avtMinMaxExpression.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtMinMaxExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

avtMinMaxExpression::avtMinMaxExpression()
{
    doMin = false;
}


// ****************************************************************************
//  Method: avtMinMaxExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

avtMinMaxExpression::~avtMinMaxExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMinMaxExpression::DoOperation
//
//  Purpose:
//      Finds the minimum or maximum value.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

void
avtMinMaxExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps == in2ncomps)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                float val1 = in1->GetComponent(i, j);
                float val2 = in2->GetComponent(i, j);
                bool val1Bigger = (val1 > val2);
                // Circumflex (^) is the exclusive or.
                // doMin == true  && val1Bigger == true  --> val2
                // doMin == false && val1Bigger == true  --> val1
                // doMin == true  && val1Bigger == false --> val1
                // doMin == false && val1Bigger == false --> val2
                //  --> values same, then val2, values different, then val1
                float outval = (doMin ^ val1Bigger ? val1 : val2);
                out->SetComponent(i, j, outval);
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Don't know how to take minimums or "
                         "maximums with data of differing dimensions.");
    }
}


