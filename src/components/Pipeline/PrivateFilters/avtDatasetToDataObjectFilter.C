// ************************************************************************* //
//                       avtDatasetToDataObjectFilter.C                      //
// ************************************************************************* //

#include <avtDatasetToDataObjectFilter.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToDataObjectFilter::avtDatasetToDataObjectFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToDataObjectFilter::~avtDatasetToDataObjectFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::InputSetActiveVariable
//
//  Purpose:
//      Sets the active variable in the input.
//
//  Arguments:
//      varname    The name of the new active variable.
//
//  Notes:     The method name contains "input" to prevent name collisions in
//             multiply inheriting derived types.  The compilers should be
//             able to handle this when given a scope resolution operator, but
//             I don't trust them.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::InputSetActiveVariable(const char *varname)
{
    SetActiveVariableArgs args;
    args.varname = varname;
    avtDataTree_p tree = GetInputDataTree();
    bool success;
    tree->Traverse(CSetActiveVariable, (void *) &args, success);

    //
    // Set up our data members for derived types that need this information.
    //
    activeVariableIsPointData = args.activeIsPoint;
    hasPointVars              = args.hasPointVars;
    hasCellVars               = args.hasCellVars;
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::SearchDataForDataExtents
//
//  Purpose:
//      Searches the dataset for the current variable's extents.
//
//  Programmer: Hank Childs
//  Creation:   April 17, 2002
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::SearchDataForDataExtents(double *extents)
{
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetDataExtents(input, extents);
}


