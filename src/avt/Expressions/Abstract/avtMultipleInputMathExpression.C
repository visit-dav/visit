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
//                     avtMultipleInputMathExpression.h                      //
// ************************************************************************* //

#include <avtMultipleInputMathExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>
#include <DebugStream.h>

avtMultipleInputMathExpression::avtMultipleInputMathExpression()
{
    ;
}

avtMultipleInputMathExpression::~avtMultipleInputMathExpression()
{
    ;
}

vtkDataArray*
avtMultipleInputMathExpression::DeriveVariable(vtkDataSet* in_ds, int dummy)
{
    debug3 << "Entering avtMultipleInputMathExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;

    std::cout << "nProcessedArgs: " << nProcessedArgs << std::endl;
    std::cout << "varnames size: " << varnames.size() << std::endl;

    // Get the variables and their centering
    for (int i = 0; i < nProcessedArgs; ++i)
    {
        avtCentering currentCenter;
        dataArrays.push_back(ExractCenteredData(&currentCenter, in_ds, varnames[i]));
        centerings.push_back(currentCenter);
    }

    RecenterData(in_ds);
    vtkDataArray* output = DoOperation();

    debug3 << "Exiting  avtMultipleInputMathExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;
    return output;
}

// ****************************************************************************
//  Method: avtBinaryMathExpression::DetermineCentering
//
//  Purpose:
//      Determines the centering of an input variable and outputs the
//      data array.
//
//  Arguments:
//      centering_out   A pointer to the centering variable.
//      in_ds           Dataset from which to extract the array.
//      varname         The name of the variable we want to extract.
//
//  Returns:    The cell- or node-centered data array.
//
//  Programmer: Eddie Rusu
//  Creation:   Wed Sep 11 08:59:52 PDT 2019
//
// ****************************************************************************

vtkDataArray*
avtMultipleInputMathExpression::ExractCenteredData(avtCentering *centering_out,
        vtkDataSet *in_ds, const char *varname)
{
    debug5 << "Entering avtMultipleInputMathExpression::ExractCenteredData("
            "avtCentering*, vtkDataSet*, const char*)" << std::endl;
    debug5 << "\t For " << varname << std::endl;
    vtkDataArray* out = in_ds->GetCellData()->GetArray(varname);
    if (out == NULL)
    {
        out = in_ds->GetPointData()->GetArray(varname);
        if (out == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                    "An internal error occurred when calculating an "
                    "expression. Please contact a VisIt developer.");
        }
        else
        {
            *(centering_out) = AVT_NODECENT;
            debug5 << "Exiting  "
                    "avtMultipleInputMathExpression::ExractCenteredData("
                    "avtCentering*, vtkDataSet*, const char*)" << std::endl;
            return out;
        }
    }
    else
    {
        *(centering_out) = AVT_ZONECENT;
        debug5 << "Exiting  avtMultipleInputMathExpression::ExractCenteredData("
                "avtCentering*, vtkDataSet*, const char*)" << std::endl;
        return out;
    }
}
