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
// ****************************************************************************

void
avtCurrentExtentFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    int dataDim = atts.GetVariableDimension();

    double *de = new double[dataDim*2];
    avtDataset_p ds = GetTypedInput();
    bool foundDE = avtDatasetExaminer::GetDataExtents(ds, de);

    if (foundDE)
    {
        outAtts.GetCumulativeCurrentDataExtents()->Merge(de);
    }

    delete [] de;

    double se[6];
    bool foundSE = avtDatasetExaminer::GetSpatialExtents(ds, se);
    if (foundSE)
    {
        outAtts.GetCumulativeCurrentSpatialExtents()->Merge(se);
    }
}

