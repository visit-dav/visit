// ************************************************************************* //
//                       avtDataObjectToDatasetFilter.C                      //
// ************************************************************************* //

#include <avtDataObjectToDatasetFilter.h>

#include <avtCommonDataFunctions.h>


// ****************************************************************************
//  Method: avtDataObjectToDatasetFilter::OutputSetActiveVariable
//
//  Purpose:
//      Sets the active variable in the output.
//
//  Arguments:
//      varname    The name of the new active variable.
//
//  Notes:     The method name contains "output" to prevent name collisions in
//             multiply inheriting derived types.  The compilers should be
//             able to handle this when given a scope resolution operator, but
//             I don't trust them.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Sat Dec 13 16:07:44 PST 2003
//    Set the dimension of the output variable.
//
//    Hank Childs, Sun Dec 14 14:15:02 PST 2003
//    Initialize the dimension for the cases of non-variables (like meshes and
//    subsets).
//
//    Mark C. Miller, Thu Jan 22 22:13:19 PST 2004
//    Initialized activeVarDim to -1 and success to false before call to
//    Traverse and protected call to SetVariableDimension with check for
//    success==true and activeVarDim!=-1
//
// ****************************************************************************

void
avtDataObjectToDatasetFilter::OutputSetActiveVariable(const char *varname)
{
    SetActiveVariableArgs args;
    args.varname = varname;
    args.activeVarDim = -1;
    avtDataTree_p tree = GetDataTree();
    bool success = false;
    tree->Traverse(CSetActiveVariable, (void *) &args, success);
    if (success && (args.activeVarDim != -1))
        GetOutput()->GetInfo().GetAttributes().
                                      SetVariableDimension(args.activeVarDim);
}


