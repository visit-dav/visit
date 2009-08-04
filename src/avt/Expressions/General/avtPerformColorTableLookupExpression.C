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
//             avtPerformColorTableLookupExpression.C                            //
// ************************************************************************* //

#include <avtPerformColorTableLookupExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkSkewLookupTable.h>

#include <avtLookupTable.h>
#include <avtExprNode.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtPerformColorTableLookupExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the 
//      header because it causes problems for certain compilers.
//
//  Programmer: Gunther H. Weber
//  Creation:   Tue Jan  8 17:02:56 PST 2008
//
// ****************************************************************************

avtPerformColorTableLookupExpression::avtPerformColorTableLookupExpression()
{
}

// ****************************************************************************
//  Method: avtPerformColorTableLookupExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Gunther H. Weber
//  Creation:   Tue Jan  8 17:02:56 PST 2008
//
// ****************************************************************************

avtPerformColorTableLookupExpression::~avtPerformColorTableLookupExpression()
{
}

// ****************************************************************************
//  Method: avtPerformColorTableLookupExpression::PreExecute
//
//  Purpose:
//      Initializes a flag saying whether or not we've issued a warning.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
avtPerformColorTableLookupExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();

    // Get value range of default variable
    GetDataExtents(mExtents, 0);
}

// ****************************************************************************
//  Method: avtPerformColorTableLookupExpression::ProcessArguments
//
//  Purpose:
//      Parses optional algorithm argument.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer:   Gunther H. Weber
//  Creation:     Tue Jan  8 17:06:16 PST 2008
//
// ****************************************************************************
void
avtPerformColorTableLookupExpression::ProcessArguments(ArgsExpr *args,
                                    ExprPipelineState *state)
{
    // Get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // Check number of args
    if (nargs < 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "PerformColorTableLookup() Incorrect syntax.\n"
                   "Usage: PerformColorTableLookup(varname,colortablename, "
                   "mapping,skewfactor);\n with mapping being \"id\"/0, "
                   "\"log\"/1 or \"skew\"/2. The argument \"skewfactor\" "
                   "is only required for the mapping \"skew\".");
    }

    // First argument is the var name, let it do its own magic
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // Parse the color table argument
    ArgExpr *second_arg= (*arguments)[1];
    ExprParseTreeNode *second_tree= second_arg->GetExpr();
    string second_type = second_tree->GetTypeName();

    if ((second_type == "StringConst"))
        mLUTName = dynamic_cast<StringConstExpr*>(second_tree)->GetValue();
    else // invalid arg type
        EXCEPTION2(ExpressionException, outputVariableName,
                "avtPerformColorTableLookupExpression: Expected string (color"
                "table name) as second argument.");

    // Check for a mapping
    if (nargs > 2)
    {
        // Parse the color table argument
        ArgExpr *third_arg = (*arguments)[2];
        ExprParseTreeNode *third_tree= third_arg->GetExpr();
        string third_type = third_tree->GetTypeName();
        if ((third_type == "StringConst"))
        {
            string mapping =  dynamic_cast<StringConstExpr*>(third_tree)->GetValue();
            if (mapping == "id")
                mLUTMapping = Identity;
            else if (mapping == "log")
                mLUTMapping = Log;
            else if (mapping == "skew")
                mLUTMapping = Skew;
            else
                EXCEPTION2(ExpressionException, outputVariableName,
                        "avtPerformColorTableLookupExpression: Invalid third argument.\n"
                        "Valid options are: \"id\"/0, \"log\"/1 or \"skew\"/2.");
        }
        else if (third_type == "IntegerConst")
        {
            int mapping = dynamic_cast<IntegerConstExpr*>(third_tree)->GetValue();

            if(mapping < 0 || mapping > 2)
            {
                EXCEPTION2(ExpressionException, outputVariableName,
                        "avtPerformColorTableLookupExpression: Invalid third argument.\n"
                        "Valid options are: \"id\"/0, \"log\"/1 or \"skew\"/2.");
            }
            else
            {
                mLUTMapping = LookupMapping(mapping);
            }
        }

        if (mLUTMapping == Skew)
        {
            if (nargs < 4)
            {
                EXCEPTION2(ExpressionException, outputVariableName,
                        "avtPerformColorTableLookupExpression: Missing fourth argument.\n"
                        "Must specify skew factor for mapping \"skew\".");
            }
            else
            {
                ArgExpr *fourth_arg = (*arguments)[3];
                ExprParseTreeNode *fourth_tree= fourth_arg->GetExpr();
                string fourth_type = fourth_tree->GetTypeName();
                if (fourth_type == "FloatConst")
                {
                    mSkew = dynamic_cast<FloatConstExpr*>(fourth_tree)->GetValue();
                }
                else if (fourth_type == "IntegerConst")
                {
                    mSkew = double(dynamic_cast<IntegerConstExpr*>(fourth_tree)->GetValue());
                }

                else
                {
                    EXCEPTION2(ExpressionException, outputVariableName,
                            "avtPerformColorTableLookupExpression: Skew factor must "
                            "be a float constant."); 
                }
            }
        }
    }
    else
    {
        mLUTMapping = Identity;
        mSkew = 1.0;
    }
}

// ****************************************************************************
//  Method: avtPerformColorTableLookupExpression::DoOperation
//
//  Purpose:
//      Performs the color table lookup for each tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Gunther H. Weber
//  Creation:   Tue Jan  8 17:04:08 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Aug  6 17:23:47 EDT 2008
//    Assumed an "if (a=b)" line should have read "if (a==b)".
//
// ****************************************************************************

void
avtPerformColorTableLookupExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                             int ncomponents, int ntuples)
{
    // Ensure we are dealing with scalar data
    if (ncomponents != 1)
        EXCEPTION2(ExpressionException, outputVariableName,
                   "PerformColorTableLookup() Expected scalar mesh variable.\n");
 
    // Get color table
    avtLookupTable avtLUT;
    avtLUT.SetColorTable(mLUTName.c_str(), true);
    vtkLookupTable *vtkLUT;
    switch (mLUTMapping)
    {
        case Identity:
            vtkLUT = avtLUT.GetLookupTable();
            break;
        case Log:
            vtkLUT = avtLUT.GetLogLookupTable();
            break;
        case Skew:
            avtLUT.SetSkewFactor(mSkew);
            std::cout << "avtLUT.SetSkewFactor("<<mSkew<<")"<<std::endl;
            vtkLUT = avtLUT.GetSkewLookupTable();
            std::cout << (dynamic_cast<vtkSkewLookupTable*>(vtkLUT)->GetSkewFactor()) << std::endl;
            break;
    }

    // Set value range
    vtkLUT->SetRange(mExtents[0], mExtents[1]);
    if (mLUTMapping == Skew) 
        std::cout << dynamic_cast<vtkSkewLookupTable*>(vtkLUT)->GetSkewFactor()
                   << std::endl;

    // Perform lookup for each scalar in the data set
    for (int i = 0 ; i < ntuples ; i++)
    {
        unsigned char *col;
        // FIXME: Hack. Something strange is happening here. MapValue is virtual and should automatically call
        // the method of the correct subclass of vtkLookupTable. However, this does not seem to work for
        // vtkSkewLookupTable. For vtkLogLookupTable it works fine, though. As a workaround, we manually check
        // whether we are dealing with "Skew" lookup and cast vtkLookupTable manually to vtkSkewLookupTable if
        // necessary. 
        if (mLUTMapping == Skew)
            col = dynamic_cast<vtkSkewLookupTable*>(vtkLUT)->MapValue(in->GetTuple1(i));
        else
            col = vtkLUT->MapValue(in->GetTuple1(i));
        out->SetTuple3(i, double(col[0]), double(col[1]), double(col[2]));
    }
}
