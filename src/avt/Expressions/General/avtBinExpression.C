/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                        avtBinExpression.C                                 //
// ************************************************************************* //

#include <avtBinExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>

#include <algorithm>
#include <sstream>

// ****************************************************************************
//  Method: avtBinExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 12 16:49:00 PDT 2012
//
// ****************************************************************************

avtBinExpression::avtBinExpression() : avtMultipleInputExpressionFilter(), bins()
{
}


// ****************************************************************************
//  Method: avtMapExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 12 16:49:00 PDT 2012
//
// ****************************************************************************

avtBinExpression::~avtBinExpression()
{
}


// ****************************************************************************
//  Method: avtBinExpression::DeriveVariable
//
//  Purpose:
//      Transforms input array values by assigning values to a bin.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Notes: Adapted from Cyrus Harrison's avtApplyMapExpression.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 12 16:49:00 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtBinExpression::DeriveVariable(vtkDataSet *in_ds)
{
    std::ostringstream oss;

    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    vtkDataArray *var = in_ds->GetPointData()->GetArray(varnames[0]);
    if (var == NULL)
        var = in_ds->GetCellData()->GetArray(varnames[0]);

    oss << "Could not find input variable \"" << varnames[0] << "\"";
    if (var == NULL)
        ThrowError(oss.str());

    if (var->GetNumberOfComponents() != 1)
        ThrowError("The bin expression only supports scalar inputs.");

    vtkDataArray *rv;
    if (var->GetDataType() == VTK_DOUBLE)
        rv = vtkDoubleArray::New();
    else
        rv = vtkFloatArray::New();
    vtkIdType nvals = var->GetNumberOfTuples();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(nvals);

    for (vtkIdType i = 0 ; i < nvals ; i++)
    {
        double val = var->GetTuple1(i);

        int bin = 0;
        for(size_t j = 0; j < bins.size() && val > bins[j]; ++j)
            bin++;

        rv->SetComponent(i, 0, double(bin));
    }

    return rv;
}


// ****************************************************************************
//  Method: avtBinExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.
//      Parses 'to' and 'from' lists to define a mapping function.
//
//  Arguments:
//      args      Expression arguments.
//      state     Pipeline state.
//
//  Programmer:   Brad Whitlock
//  Creation:     Wed Sep 12 16:49:00 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

void
avtBinExpression::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    //
    // support the following use cases:
    //  2 args:
    //   map(var,[bins])

    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // first argument should be the var name, let it do its own magic
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>((*arguments)[0]->GetExpr());
    first_tree->CreateFilters(state);

    if ( nargs < 2)
        ThrowError("the bin expression requires two arguments");

    ExprParseTreeNode *second_node = (*arguments)[1]->GetExpr();
    if (second_node->GetTypeName() != "List")
        ThrowError("second argument must be a numeric list.");

    bins.clear();
    ListExpr *bin_list = dynamic_cast<ListExpr*>(second_node);
    if(!bin_list->ExtractNumericElements(bins))
        ThrowError("bin 'bin list' argument contain a numeric list.");

    // Sort the bins.
    std::sort(bins.begin(), bins.end());
}

// ****************************************************************************
//  Method: avtBinExpression::ThrowError
//
//  Purpose:
//      Helper that throws an exception with given error message.
//
//  Arguments:
//      msg    Error message.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 12 16:49:00 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

void
avtBinExpression::ThrowError(const std::string &msg)
{
    // note this undermines the exception line # reporting
    // but it cleans up the code above significantly.
    EXCEPTION2(ExpressionException, outputVariableName,
               msg + "\n'bin' usage:\n"
                     "  bin(value,[bin list])\n");
}

