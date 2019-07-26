// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtActualExtentsFilter.h                          // 
// ************************************************************************* // 


#include <avtActualExtentsFilter.h>

#include <avtDataAttributes.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtActualExtentsFilter::Execute
//
//  Purpose:
//      This filter simply calculates extents.  Do that here.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001 
//
//  Modifications:
// 
//    Hank Childs, Thu Aug 26 16:36:30 PDT 2010
//    Call UpdateExtents.
//
// ****************************************************************************

void
avtActualExtentsFilter::Execute(void)
{
    UpdateExtents();
    SetOutputDataTree(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtActualExtentsFilter::UpdateExtents
//
//  Purpose:  Retrieves the actual data/spatial extents from the
//            input and stores them in output's info.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 3, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for avtDatasetExaminer.
//
//    Hank Childs, Thu Jul 17 17:40:24 PDT 2003
//    Treat 2D vectors as 3D since VTK will assume that vectors are 3D.
//
//    Hank Childs, Tue Feb 24 14:23:03 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 11:16:17 PST 2004 
//    DataExtents now always have only 2 components. 
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.  Only calculate the requested variables.
//
// ****************************************************************************

void
avtActualExtentsFilter::UpdateExtents(void)
{
    int t1 = visitTimer->StartTimer();
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    avtDataset_p ds = GetTypedInput();

    int nVars = atts.GetNumberOfVariables();
    double de[2];
    for (int i = 0 ; i < nVars ; i++)
    {
        const char *vname = atts.GetVariableName(i).c_str();
        if (! lastContract->ShouldCalculateVariableExtents(vname))
            continue;
    
        bool foundDE = avtDatasetExaminer::GetDataExtents(ds, de, vname);
        if (foundDE)
        {
            outAtts.GetThisProcsActualDataExtents(vname)->Merge(de);
        }
    }

    if (lastContract->ShouldCalculateMeshExtents())
    {
        double se[6];
        bool foundSE = avtDatasetExaminer::GetSpatialExtents(ds, se);
        if (foundSE)
        {
            outAtts.GetThisProcsActualSpatialExtents()->Merge(se);
        }
    }
    visitTimer->StopTimer(t1, "Calculating the actual extents");
}


// ****************************************************************************
//  Method:  avtActualExtentsFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtActualExtentsFilter::FilterUnderstandsTransformedRectMesh()
{
    // there were some changes made at lower levels which make
    // this filter safe
    return true;
}


// ****************************************************************************
//  Method: avtActualExtentsFilter::ModifyContract
//
//  Purpose:
//      The base class declares that this filter can only work on floats.
//      Reimplement this method to do nothing, just to prevent the base class
//      from doing the wrong thing.  The way the base class is working should
//      be overhauled, but I'm not going to address that now.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Aug 26 16:36:30 PDT 2010
//    Cache the last contract.
//
// ****************************************************************************

avtContract_p
avtActualExtentsFilter::ModifyContract(avtContract_p c)
{
    lastContract = c;
    return c;
}



