// ************************************************************************* //
//                       avtDataObjectToDatasetFilter.C                      //
// ************************************************************************* //

#include <avtDataObjectToDatasetFilter.h>

#include <avtCommonDataFunctions.h>


// ****************************************************************************
//  Method: avtDataObjectToDatasetFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectToDatasetFilter::avtDataObjectToDatasetFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectToDatasetFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectToDatasetFilter::~avtDataObjectToDatasetFilter()
{
    ;
}


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
//    Hank Childs, Fri Feb 20 15:14:32 PST 2004
//    Use the new data attributes method to set the active variable.
//    Remove logic dealing with setting variable dimension -- it is now
//    managed correctly without pseudo-logic.
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

    GetOutput()->GetInfo().GetAttributes().SetActiveVariable(varname);
}


// ****************************************************************************
//  Method: avtDataObjectToDatasetFilter::PostExecute
//
//  Purpose:
//      Walks through the data and determines if we have polygonal data stored
//      as an unstructured grid.
//
//  Programmer: Hank Childs
//  Creation:   July 27, 2004
//
// ****************************************************************************

void
avtDataObjectToDatasetFilter::PostExecute(void)
{
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    if ((atts.GetSpatialDimension()==3 && atts.GetTopologicalDimension()<3) ||
        (atts.GetSpatialDimension()==2 && atts.GetTopologicalDimension()<2))
    {
        avtDataTree_p tree = GetDataTree();
        bool dummy;
        tree->Traverse(CConvertUnstructuredGridToPolyData, NULL, dummy);
    }
}


