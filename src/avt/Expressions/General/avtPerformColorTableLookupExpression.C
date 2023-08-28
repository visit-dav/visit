// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//             avtPerformColorTableLookupExpression.C                        //
// ************************************************************************* //

#include <avtPerformColorTableLookupExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkSkewLookupTable.h>

#include <avtLookupTable.h>
#include <avtExprNode.h>

#include <ExpressionException.h>

#include <string>
#include <vector>

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
    size_t nargs = arguments->size();

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
    std::string second_type = second_tree->GetTypeName();

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
        std::string third_type = third_tree->GetTypeName();
        if ((third_type == "StringConst"))
        {
            std::string mapping =  dynamic_cast<StringConstExpr*>(third_tree)->GetValue();
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
                std::string fourth_type = fourth_tree->GetTypeName();
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
avtPerformColorTableLookupExpression::DoOperation(vtkDataArray *in, 
    vtkDataArray *out, int ncomponents, int ntuples)
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
            vtkLUT = avtLUT.GetSkewLookupTable();
            break;
        default: 
            vtkLUT = avtLUT.GetLookupTable();
            break;
    }

    // Set value range
    vtkLUT->SetRange(mExtents[0], mExtents[1]);

    // Perform lookup for each scalar in the data set
    for (int i = 0 ; i < ntuples ; i++)
    {
        const unsigned char *col;
        // Added KSB 4-6-2012.  Actually, MapValue is NOT virtual, that is
        // why it must be cast manually in order to use the right method.
        // The reason things work correctly with Log, is that 
        // vtkLogLookupTable is simply a shell for avtkLookupTable with
        // log scaling turned on.
        //
        // FIXME: Hack. Something strange is happening here. MapValue is 
        // virtual and should automatically call the method of the correct 
        // subclass of vtkLookupTable. However, this does not seem to work for
        // vtkSkewLookupTable. For vtkLogLookupTable it works fine, though. As 
        // a workaround, we manually check  whether we are dealing with "Skew" 
        // lookup and cast vtkLookupTable manually to vtkSkewLookupTable if
        // necessary. 
        if (mLUTMapping == Skew)
            col = dynamic_cast<vtkSkewLookupTable*>(vtkLUT)->MapValue(in->GetTuple1(i));
        else
            col = vtkLUT->MapValue(in->GetTuple1(i));
        out->SetTuple3(i, double(col[0]), double(col[1]), double(col[2]));
    }
}
