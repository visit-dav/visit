// ************************************************************************* //
//                     avtTotalRevolvedSurfaceAreaQuery.C                    //
// ************************************************************************* //

#include <avtTotalRevolvedSurfaceAreaQuery.h>

#include <avtFacelistFilter.h>
#include <avtRevolvedSurfaceArea.h>
#include <avtSourceFromAVTDataset.h>

#include <NonQueryableInputException.h>


using     std::string;


// ****************************************************************************
//  Method: avtTotalRevolvedSurfaceAreaQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   March 18, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
// ****************************************************************************

avtTotalRevolvedSurfaceAreaQuery::avtTotalRevolvedSurfaceAreaQuery() 
    : avtSummationQuery()
{
    string      varname = "revolved_surface_area";
    string      sum_type = "RevolvedSurfaceArea";    
    string      units_append = "^2";    

    surface_area = new avtRevolvedSurfaceArea;
    surface_area->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
    SumGhostValues(false);
}


// ****************************************************************************
//  Method: avtTotalRevolvedSurfaceAreaQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   March 18, 2003 
//
// ****************************************************************************

avtTotalRevolvedSurfaceAreaQuery::~avtTotalRevolvedSurfaceAreaQuery()
{
    if (surface_area != NULL)
    {
        delete surface_area;
        surface_area = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalRevolvedSurfaceAreaQuery::ApplyFilters 
//
//  Purpose:
//    Apply any avt filters necessary for the completion of this query.
//
//  Programmer: Hank Childs 
//  Creation:   March 18, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Jan 12 15:00:52 PST 2006
//    Add support for data sets that aren't already external edges.
//
// ****************************************************************************

avtDataObject_p 
avtTotalRevolvedSurfaceAreaQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    avtFacelistFilter ff;
    if (dob->GetInfo().GetAttributes().GetTopologicalDimension() == 2)
    {
        qualifier = "This query was calculated by finding the external line "
                    "segments of the data set and then calculating the surface"
                    " area of these line segments when revolved into 3D.";
        ff.SetCreateEdgeListFor2DDatasets(true);
        ff.SetInput(dob);
        dob = ff.GetOutput();
    }

    surface_area->SetInput(dob);
    avtDataObject_p objOut = surface_area->GetOutput();
    objOut->Update(pspec);
    return objOut;
}


// ****************************************************************************
//  Method: avtTotalRevolvedSurfaceAreaQuery::VerifyInput
//
//  Purpose:
//    Now that we have an input, we can determine what the units are 
//    and tell the base class.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 27, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 09:03:24 PDT 2004
//    Added test for topological dimension, as that is no longer performed by
//    base class.  
//
//    Hank Childs, Thu Jan 12 14:54:25 PST 2006
//    Add more checking.
//
// ****************************************************************************

void
avtTotalRevolvedSurfaceAreaQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtDataObjectQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() >= 3
        || GetInput()->GetInfo().GetAttributes().GetSpatialDimension() >= 3)
    {
        EXCEPTION1(NonQueryableInputException,
            "Can only revolve 2D data sets.  This data is already 3D.");
    }

    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}
