// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDataObjectToDatasetFilter.C                      //
// ************************************************************************* //

#include <avtDataObjectToDatasetFilter.h>

#include <avtCommonDataFunctions.h>

#include <DebugStream.h>
#include <TimingsManager.h>

bool avtDataObjectToDatasetFilter::vtkDebugMode = false;

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
//  Modifications:
//
//    Hank Childs, Tue Jan 16 11:24:53 PST 2007
//    Break all VTK pipeline connections.
//
//    Cyrus Harrison, Sat Aug 11 19:48:30 PDT 2007
//    Add support for vtk-debug mode.
//
//    Kathleen Biagas, Thu Apr 26 13:57:42 PDT 2012
//    Add debug statement to aid in debugging new filters that inadvertently
//    trigger the transformation.
//
// ****************************************************************************

void
avtDataObjectToDatasetFilter::PostExecute(void)
{
    avtDataTree_p tree = GetDataTree();
    bool dummy;

    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    if ((atts.GetSpatialDimension()==3 && atts.GetTopologicalDimension()<3) ||
        (atts.GetSpatialDimension()==2 && atts.GetTopologicalDimension()<2))
    {
        char dimDataStr[32];
        debug3 << "avtDataObjectToDatastFilter converting ugrids to polydata "
               << "in postex." << endl;
        int t0 = visitTimer->StartTimer();
        snprintf(dimDataStr, sizeof(dimDataStr),
            "avtTopoDim=%d", atts.GetTopologicalDimension());
        tree->Traverse(CConvertUnstructuredGridToPolyData, dimDataStr, dummy);
        visitTimer->StopTimer(t0, "converting ugrids to polydata in postex");
    }

    int t0 = visitTimer->StartTimer();
    tree->Traverse(CBreakVTKPipelineConnections, (void*)&vtkDebugMode, dummy);
    visitTimer->StopTimer(t0, "Breaking pipeline connections in postex");
}


