// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtRecenterExpression.C                          //
// ************************************************************************* //

#include <avtRecenterExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtRecenterExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//      Sean Ahern, Wed Sep 10 12:15:05 EDT 2008
//      Set the default recentering to "toggle".
//
// ****************************************************************************

avtRecenterExpression::avtRecenterExpression()
{
    recenterMode = Toggle;
}


// ****************************************************************************
//  Method: avtRecenterExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRecenterExpression::~avtRecenterExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRecenterExpression::ProcessArguments
//
//  Purpose:
//      Parses optional centering argument.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer:   Sean Ahern
//  Creation:     Wed Sep 10 12:04:12 EDT 2008
//
// ****************************************************************************
void
avtRecenterExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Get the argument list and number of arguments.
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();

    // Check for a call with no arguments.
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "recenter(): Incorrect syntax.\n"
                   " usage: recenter(varname, [centering])\n"
                   " The centering parameter is optional "
                   " and specifies nodal or zonal centering.\n"
                   " Valid values of centering: \"nodal\", \"zonal\", and \"toggle\".\n"
                   " The default centering is to toggle, that is, to convert "
                   " nodal to zonal or zonal to nodal.");
    }

    // Grab off the first argument.
    ArgExpr *firstArg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstArg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if we have a second optional argument that tells us how to
    // center.
    //
    // Options:
    //  nodal:  Recenter to nodes
    //  zonal:  Recenter to zones
    //  toggle: Toggle centering
    // Default: toggle

    if (nargs > 1)
    {
        ArgExpr *secondArg = (*arguments)[1];
        ExprParseTreeNode *secondTree = secondArg->GetExpr();
        std::string secondType = secondTree->GetTypeName();

        // Check for argument passed as string
        if (secondType == "StringConst")
        {
            std::string sval = dynamic_cast<StringConstExpr*>(secondTree)->GetValue();

            if (sval == "toggle")
                recenterMode = Toggle;
            else if (sval == "nodal")
                recenterMode = Nodal;
            else if (sval == "zonal")
                recenterMode = Zonal;
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName,
                           "avtRecenterExpression: Invalid second argument.\n"
                           " Valid options are: \"nodal\", \"zonal\", or \"toggle\".");
            }
        }
        else    // Invalid argument type
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                       "avtRecenterExpression: Invalid second argument type.\n"
                       "Must be a string with one of: \"nodal\", \"zonal\", \"toggle\".");
        }
    }
}


// ****************************************************************************
//  Method: avtRecenterExpression::DeriveVariable
//
//  Purpose:
//      Assigns a recenter number to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     December 10, 2003
//
// ****************************************************************************

vtkDataArray *
avtRecenterExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (activeVariable == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Asked to recenter, but did not "
                   "specify which variable to recenter");
    }

    debug5 << "avtRecenterExpression: recentering mode: ";
    if (recenterMode == Toggle)
    {
        debug5 << "toggle" << endl;
    }
    if (recenterMode == Nodal)
    {
        debug5 << "nodal" << endl;
    }
    if (recenterMode == Zonal)
    {
        debug5 << "zonal" << endl;
    }

    vtkDataArray *cell_data = in_ds->GetCellData()->GetArray(activeVariable);
    vtkDataArray *pt_data   = in_ds->GetPointData()->GetArray(activeVariable);

    vtkDataArray *rv = NULL;

    avtCentering target = AVT_UNKNOWN_CENT; 
    switch(recenterMode)
    {
    case Nodal:
        target = AVT_NODECENT;
        break;
    case Toggle:
        target = AVT_UNKNOWN_CENT;
        break;
    case Zonal:
        target = AVT_ZONECENT;
        break;
    }

    if (cell_data != NULL)
        rv = Recenter(in_ds, cell_data, AVT_ZONECENT, outputVariableName, target);
    else if (pt_data != NULL)
        rv = Recenter(in_ds, pt_data, AVT_NODECENT, outputVariableName, target);
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Was not able to locate variable to recenter.");
    }

    return rv;
}


// ****************************************************************************
//  Method: avtRecenterExpression::IsPointVariable
//
//  Purpose:
//      Switch the centering of the point variable.
//
//  Programmer: Hank Childs
//  Creation:   December 10, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Jan  9 10:26:39 PST 2008
//    Make sure we are giving information about the correct variable.
//
// ****************************************************************************

bool
avtRecenterExpression::IsPointVariable(void)
{
    bool isPoint  = true;
    bool foundOne = false;
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (activeVariable != NULL)
    {
        if (atts.ValidVariable(activeVariable))
        {
            isPoint = (atts.GetCentering(activeVariable) != AVT_ZONECENT);
            foundOne = true;
        }
    }

    if (!foundOne)
        if (atts.ValidActiveVariable())
            isPoint = (atts.GetCentering() != AVT_ZONECENT);

    return (!isPoint);
}


// ****************************************************************************
//  Method: avtRecenterExpression::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2005
//
// ****************************************************************************

int
avtRecenterExpression::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtSingleInputExpressionFilter::GetVariableDimension();

    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;
    if (varname == NULL)
        return avtSingleInputExpressionFilter::GetVariableDimension();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varname))
        return avtSingleInputExpressionFilter::GetVariableDimension();
    int ncomp = atts.GetVariableDimension(varname);
    return ncomp;
}

// ****************************************************************************
//  Method: avtRecenterExpression::ModifyContract
//
//  Purpose:
//      Request ghost zones.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Apr 17 13:45:02 PDT 2013
//
// ****************************************************************************

avtContract_p
avtRecenterExpression::ModifyContract(avtContract_p in_spec)
{
    avtContract_p spec2 =
                   avtSingleInputExpressionFilter::ModifyContract(in_spec);
    spec2->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec2;
}


