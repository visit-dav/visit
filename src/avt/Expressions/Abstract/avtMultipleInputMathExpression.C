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
//                     avtMultipleInputMathExpression.C                      //
// ************************************************************************* //

#include <avtMultipleInputMathExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>


#include <ExpressionException.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtMultipleInputMathExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtMultipleInputMathExpression::avtMultipleInputMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression destructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtMultipleInputMathExpression::~avtMultipleInputMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::DeriveVariable
//
//  Purpose:
//      Creates data arrays of all the input arguments and determines their
//      centering. Performs any recentering as necessary. Performs the math
//      operation and returns the output of that operation.
//
//  Arguments:
//      in_ds   The vtkDataSet from which to extract the data arrays.
//      dummy   A dummy variable needed for the class hierarchy.
//
//  Returns:    The derived variable. The caller must free this memory.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

vtkDataArray*
avtMultipleInputMathExpression::DeriveVariable(vtkDataSet* in_ds, int dummy)
{
    debug3 << "Entering avtMultipleInputMathExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;

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
//  Method: avtMultipleInputMathExpression::ExractCenteredData
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


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::CreateOutputVariable
//
//  Purpose:
//      Generate the output variable with the correct number of components
//      and tuples.
//
//  Arguments:
//      arraysToConsider    Indicates how many arrays (in order of input) to
//                          consider in determining the component and tuple
//                          count. Default behavior is to use all of the
//                          processed arrays.
//
//  Returns:    Data array of appropriate size.
//
//  Programmer: Eddie Rusu
//  Creation:   Mon Sep 30 14:49:38 PDT 2019
//
// ****************************************************************************

vtkDataArray*
avtMultipleInputMathExpression::CreateOutputVariable()
{
    return CreateOutputVariable(dataArrays.size());
}

vtkDataArray*
avtMultipleInputMathExpression::CreateOutputVariable(int arraysToConsider)
{
    debug5 << "Entering avtMultipleInputMathExpression::"
              "CreateOutputVariable(int)" << std::endl;
    // Loop over all inputs and determine the number of components and
    // tuples
    int nComps = 1;
    int nVals = 1;
    for (int i = 0; i < arraysToConsider; ++i)
    {
        int nCompsi = dataArrays[i]->GetNumberOfComponents();
        if (nCompsi != 1 && nCompsi != nComps)
        {
            // We can support one-multi components, but we can only support
            // multi-multi if they are the same values.
            if (nComps == 1)
            {
                nComps = nCompsi;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, "Cannot "
                        "process variables with different number of "
                        "dimensions.");
            }
        }

        int nValsi = dataArrays[i]->GetNumberOfTuples();
        if (nValsi != 1 && nValsi != nVals)
        {
            // We can support singleton values but we cannot support mismatched
            // number of tuples
            if (nVals == 1)
            {
                nVals = nValsi;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, "Cannot "
                        "process variables with different number of "
                        "elements.");
            }
        }
    }

    // Setup the output variable
    vtkDataArray* output = vtkDoubleArray::New();
    output->SetNumberOfComponents(nComps);
    output->SetNumberOfTuples(nVals);
    debug5 << "Number of tuples: " << nVals << std::endl;
    debug5 << "Number of components: " << nComps << std::endl;

    debug5 << "Exiting  avtMultipleInputMathExpression::"
              "CreateOutputVariable(int)" << std::endl;
    return output;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::RecenterData
//
//  Purpose:
//      Determines the centering of the input variables. If there is mixed
//      centering, default to zone-centered.
//
//  Arguments:
//      in_ds   The vtkDataSet that holds all the arrays. Arrays and
//              centerings are already stored in dataArrays and centerings,
//              which are class vectors, so in_ds is only needed because
//              the call to avtExpressionFilter::Recenter requires it.
//
//  Programmer: Eddie Rusu
//  Creation:   Mon Sep 30 10:38:44 PDT 2019
//
// ****************************************************************************

void
avtMultipleInputMathExpression::RecenterData(vtkDataSet* in_ds)
{
    debug5 << "Entering avtMultipleInputMathExpression::RecenterData(vtkDataSet*)"
            << std::endl;

    // Determine the centering
    centering = centerings[0];
    for (int i = 1; i < nProcessedArgs; ++i)
    {
        if (centerings[i] != centering)
        {
            centering = AVT_ZONECENT;
            break;
        }
    }

    // Recenter variables as needed
    for (int i = 0; i < nProcessedArgs; ++i)
    {
        if (centerings[i] != centering)
        {
            dataArrays[i] = Recenter(in_ds, dataArrays[i], centerings[i],
                outputVariableName);
        }
    }
    debug5 << "Exiting  avtMultipleInputMathExpression::RecenterData(vtkDataSet*)"
            << std::endl;
}