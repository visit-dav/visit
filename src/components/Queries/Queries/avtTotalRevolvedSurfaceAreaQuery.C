// ************************************************************************* //
//                     avtTotalRevolvedSurfaceAreaQuery.C                    //
// ************************************************************************* //

#include <avtTotalRevolvedSurfaceAreaQuery.h>

#include <avtRevolvedSurfaceArea.h>
#include <avtSourceFromAVTDataset.h>


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

    surface_area->SetInput(dob);
    avtDataObject_p objOut = surface_area->GetOutput();
    objOut->Update(pspec);
    return objOut;
}


