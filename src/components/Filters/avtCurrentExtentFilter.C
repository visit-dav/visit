// ************************************************************************* //
//                         avtCurrentExtentFilter.h                          // 
// ************************************************************************* // 


#include <avtCurrentExtentFilter.h>

#include <avtDataAttributes.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtCurrentExtentFilter::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001 
//
// ****************************************************************************

void
avtCurrentExtentFilter::Execute(void)
{
    SetOutputDataTree(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtCurrentExtentFilter::RefashionDataObjectInfo
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
// ****************************************************************************

void
avtCurrentExtentFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    avtDataset_p ds = GetTypedInput();

    int nVars = atts.GetNumberOfVariables();
    double de[2];
    for (int i = 0 ; i < nVars ; i++)
    {
        const char *vname = atts.GetVariableName(i).c_str();
    
        bool foundDE = avtDatasetExaminer::GetDataExtents(ds, de, vname);
        if (foundDE)
        {
            outAtts.GetCumulativeCurrentDataExtents(vname)->Merge(de);
        }
    }

    double se[6];
    bool foundSE = avtDatasetExaminer::GetSpatialExtents(ds, se);
    if (foundSE)
    {
        outAtts.GetCumulativeCurrentSpatialExtents()->Merge(se);
    }
}

