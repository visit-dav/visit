// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtNaturalLogExpression.C                         //
// ************************************************************************* //

#include <avtNaturalLogExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>

#include <ExpressionException.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtNaturalLogExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri May  8 13:21:52 PDT 2009
//    Initialize defaultErrorValue, useDefaultOnError.
//
// ****************************************************************************

avtNaturalLogExpression::avtNaturalLogExpression()
{
    defaultErrorValue = 0.;
    useDefaultOnError = false;
}


// ****************************************************************************
//  Method: avtNaturalLogExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNaturalLogExpression::~avtNaturalLogExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtNaturalLogExpression::DoOperation
//
//  Purpose:
//      Performs the natural logarithm on each component,tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 15, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:25:26 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Kathleen Bonnell, Fri May  8 13:01:58 PDT 2009
//    Added support for a default value to be used as a return value for
//    non-positive values.  Updated exception message.
//
// ****************************************************************************
 
void
avtNaturalLogExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                     int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            if (val <= 0)
            {
                if (useDefaultOnError)
                    out->SetComponent(i, j, defaultErrorValue);
                else
                {
                    std::string msg = "you cannot take the logarithm of values";
                    msg += "<=0.  You might want to try ln(var, ";
                    msg +=  "some-default-numeric-value).";
                    EXCEPTION2(ExpressionException, outputVariableName,
                            msg.c_str()); 
                }
            }
            else
                out->SetComponent(i, j, log(val));
        }
    }
}



// ****************************************************************************
//  Method: avtNaturalLogExpression::ProcessArguments
//
//  Purpose:
//      Parses optional arguments. 
//      Allows the user to pass a default value to be used in error cases.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     May 8, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtNaturalLogExpression::ProcessArguments(ArgsExpr *args, 
                                          ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "avtNaturalLogExpression: No arguments given.");
    }

    // First arg should be an expression, let it gen is filters. 
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // If we have two arguments, we expect a numerical const for the second.
    if (nargs == 2)
    {
        ArgExpr *sec = (*arguments)[1];
        avtExprNode *second_tree = dynamic_cast<avtExprNode*>(sec->GetExpr());
        double v = 0;
        if (GetNumericVal(second_tree, v))
        {
            useDefaultOnError = true;
            defaultErrorValue = v;
            debug4 << "avtNaturalLogExpression:" << "Using " << v 
                   << " as default value in error conditions" << endl;
        }
        else
        {
            std::string error_msg = "avtNaturalLogExpression: "
                               "Invalid second argument."
                               "Should be float or int";
            debug5 << error_msg << endl;
            EXCEPTION2(ExpressionException, outputVariableName, 
                       error_msg.c_str());
        }
    }
}


