/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                          avtSmartDivideExpression.C                       //
// ************************************************************************* //

#include <avtSmartDivideExpression.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>

#include <ExpressionException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtSmartDivideExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtSmartDivideExpression::avtSmartDivideExpression()
{
    tolerance = 1e-16;
    value_if_zero = 0.0;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtSmartDivideExpression::~avtSmartDivideExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSmartDivideExpression::RecenterData
//
//  Purpose:
//      Uses only the first two inputs to determine the centering of the data.
//      If there is mixed centering, default to zone-centered.
//
//  Arguments:
//      in_ds   The vtkDataSet that holds all the arrays. Arrays and
//              centerings are already stored in dataArrays and centerings,
//              which are class vectors, so in_ds is only needed because
//              the call to avtExpressionFilter::Recenter requires it.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

void
avtSmartDivideExpression::RecenterData(vtkDataSet* in_ds)
{
    debug5 << "Entering avtSmartDivideExpression::RecenterData(vtkDataSet*)"
            << std::endl;
    if (centerings[0] != centerings[1])
    {
        centering = AVT_ZONECENT;
        if (centerings[0] == AVT_NODECENT)
        {
            dataArrays[0] = Recenter(in_ds, dataArrays[0], centerings[0],
                outputVariableName);
        }
        else
        {
            dataArrays[1] = Recenter(in_ds, dataArrays[1], centerings[1],
                outputVariableName);
        }
    }
    else
    {
        centering = centerings[0];
    }
    debug5 << "Exiting  avtSmartDivideExpression::RecenterData(vtkDataSet*)"
            << std::endl;
}

// ****************************************************************************
//  Method: avtSmartDivideExpression::DoOperation
//
//  Purpose:
//      Divides the contents of the first array by the second array and outputs
//      according to the specifications for divide-by-zero-value and tolerance.
//
//  Returns: The result of the division operation.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019.
//
//  Modifications:
//
//      Eddie Rusu, Mon Sep 30 14:49:38 PDT 2019
//      Replaced output variable setup with
//      avtMultiInputMathExpression::CreateOutputVariable.
//
// ****************************************************************************

vtkDataArray*
avtSmartDivideExpression::DoOperation()
{
    debug4 << "Entering avtSmartDivideExpression::DoOperation()" << std::endl;
    // NOTE: This feature can be easily enhanced to handle a
    // multi-variable divide by zero case. For example, suppose the user
    // wants to specify varying tolernaces over the mesh as represented by
    // some variable or varying div_zero_value. This can be easily done here.

    vtkDataArray* data1 = dataArrays[0];
    vtkDataArray* data2 = dataArrays[1];

    // Setup the extra options
    if (dataArrays.size() >= 3)
    {
        value_if_zero = dataArrays[2]->GetTuple1(0);
        debug5 << "avtSmartDivideExpression::DoOperation: User "
                "specified a divide_by_zero_value of " << value_if_zero
                << "." << std::endl;
    }
    if (dataArrays.size() >= 4)
    {
        tolerance = dataArrays[3]->GetTuple1(0);
        if (tolerance < 0.0)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                    "tolerance must be nonnegative.");
        }
        debug5 << "avtSmartDivideExpression::DoOperation: User "
                "specified a tolerance of " << tolerance << "."
                << std::endl;
    }

    // Setup the output variable
    vtkDataArray *output = CreateOutputVariable(2);
    int nVals = output->GetNumberOfTuples();

    // Perform the division
    bool var1IsSingleton = (data1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (data2->GetNumberOfTuples() == 1);
    int in1ncomps = data1->GetNumberOfComponents();
    int in2ncomps = data2->GetNumberOfComponents();
    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        debug5 << "avtSmartDivideExpression::DoOperation: Scalar top and "
                "bottom." << std::endl;
        for (int i = 0 ; i < nVals ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = data1->GetTuple1(tup1);
            double val2 = data2->GetTuple1(tup2);
            output->SetTuple1(i, CheckZero(val1, val2));
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        debug5 << "avtSmartDivideExpression::DoOperation: Vector top, scalar "
                "bottom." << std::endl;
        for (int i = 0 ; i < nVals ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = data2->GetTuple1(tup2);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = data1->GetComponent(tup1, j);
                output->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        debug5 << "avtSmartDivideExpression::DoOperation: Scalar top, vector "
                "bottom." << std::endl;
        for (int i = 0 ; i < nVals; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = data1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = data2->GetComponent(tup2, j);
                output->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Division of vectors in undefined.");
    }
    debug4 << "Exiting  avtSmartDivideExpression::DoOperation()" << std::endl;

    return output;
}

// ****************************************************************************
//  Method: avtSmartDivideExpression::CheckZero
//
//  Purpose:
//      Checks the values involved in the division. If the denominator is
//      within tolerance of zero, then we return the specified divide by zero
//      value.
//
//  Arguments:
//      top           The numerator.
//      bottom        The denominator.
//
//  Returns: the result of the smart division of the two numbers. If the
//           denominator is within tolerance of zero, then return the divide-
//           by-zero value.
//
//  Programmer: Eddie Rusu
//  Creation:   Thu Aug 29 15:05:08 PDT 2019
//
// ****************************************************************************

double
avtSmartDivideExpression::CheckZero(double top, double bottom)
{
    if (fabs(bottom) < this->tolerance)
    {
        return this->value_if_zero;
    }
    else
    {
        return top / bottom;
    }
}


