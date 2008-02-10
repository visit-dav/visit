/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                          avtBinaryMultiplyExpression.C                        //
// ************************************************************************* //

#include <avtBinaryMultiplyExpression.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMultiplyExpression::avtBinaryMultiplyExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMultiplyExpression::~avtBinaryMultiplyExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression::DoOperation
//
//  Purpose:
//      Multiplies two arrays into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 18, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 07:35:07 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Hank Childs, Thu Aug 14 13:37:27 PDT 2003
//    Added support for mixing scalars and vectors.
//
//    Hank Childs, Mon Sep 22 17:06:01 PDT 2003
//    Added support for tensors.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Add support for singleton constants.
//
// ****************************************************************************
 
void
avtBinaryMultiplyExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                     vtkDataArray *out, int ncomponents,
                                     int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();

    if ((in1ncomps == 9) && (in2ncomps == 9))
    {
        float vals[9];
        for (int i = 0 ; i < ntuples ; i++)
        {
            int tup1 = (var1IsSingleton ? 0 : i);
            int tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 6); 
            vals[1] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 7); 
            vals[2] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 8); 
            vals[3] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 6); 
            vals[4] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 7); 
            vals[5] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 8); 
            vals[6] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 6); 
            vals[7] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 7); 
            vals[8] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 8); 
            out->SetTuple(i, vals);
        }
    }
    else if ((in1ncomps == 3) && (in2ncomps == 9))
    {
        float vals[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            int tup1 = (var1IsSingleton ? 0 : i);
            int tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 6); 
            vals[1] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 7); 
            vals[2] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 8); 
            out->SetTuple(i, vals);
        }
    }
    else if ((in1ncomps == 9) && (in2ncomps == 3))
    {
        float vals[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            int tup1 = (var1IsSingleton ? 0 : i);
            int tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 2); 
            vals[1] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 2); 
            vals[2] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 2);
            out->SetTuple(i, vals);
        }
    }
    else if (in1ncomps == in2ncomps)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float dot = 0.;
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                int tup1 = (var1IsSingleton ? 0 : i);
                int tup2 = (var2IsSingleton ? 0 : i);
                float val1 = in1->GetComponent(tup1, j);
                float val2 = in2->GetComponent(tup2, j);
                dot += val1*val2;
            }
            out->SetTuple1(i, dot);
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            int tup2 = (var2IsSingleton ? 0 : i);
            float val2 = in2->GetTuple1(tup2);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                int tup1 = (var1IsSingleton ? 0 : i);
                float val1 = in1->GetComponent(tup1, j);
                out->SetComponent(i, j, val1 * val2);
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            int tup1 = (var1IsSingleton ? 0 : i);
            float val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                int tup2 = (var2IsSingleton ? 0 : i);
                float val2 = in2->GetComponent(tup2, j);
                out->SetComponent(i, j, val1 * val2);
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                "Don't know how to multiply vectors of differing dimensions.");
    }
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression::GetNumberOfComponentsInOutput
//
//  Purpose:
//      Sets the number of components in the output.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 22 17:06:01 PDT 2003
//    Add matrix/tensor cases.
//
// ****************************************************************************

int
avtBinaryMultiplyExpression::GetNumberOfComponentsInOutput(int in1, int in2)
{
    if ((in1 == 3) && (in2 == 3))
        return 1;  // We will do a dot product.

    if ((in1 == 9) && (in2 == 9))
        return 9;  // Matrix multiply

    if ((in1 == 3) && (in2 == 9))
        return 3;  // Vector x Matrix = Vector

    if ((in1 == 9) && (in2 == 3))
        return 3;  // Matrix x Vector = Vector

    return (in1 > in2 ? in1 : in2);
}


