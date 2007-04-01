// ************************************************************************* //
//                   avtTotalSurfaceAreaQuery.C                              //
// ************************************************************************* //

#include <avtTotalSurfaceAreaQuery.h>

#include <avtFacelistFilter.h>
#include <avtVMetricArea.h>
#include <avtSourceFromAVTDataset.h>
#include <InvalidDimensionsException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
// ****************************************************************************

avtTotalSurfaceAreaQuery::avtTotalSurfaceAreaQuery() : avtSummationQuery()
{
    string      varname = "area";
    string      sum_type = "Surface Area";    
    string      units_append = "^2";    

    facelist  = new avtFacelistFilter;

    area      = new avtVMetricArea;
    area->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
    SumGhostValues(false);

    // avtVMetricArea will return -1 for cells that are neither triangles
    // nor quads.  If the dataset has lines, will rack up a bunch of -1's,
    // so turn off the summing of negative values.
    SumOnlyPositiveValues(true);
}


// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtTotalSurfaceAreaQuery::~avtTotalSurfaceAreaQuery()
{
    if (area != NULL)
    {
        delete area;
        area = NULL;
    }
    if (facelist != NULL)
    {
        delete facelist;
        facelist = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery::VerifyInput 
//
//  Purpose:  Throw an exception for invalid input. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
// ****************************************************************************

void 
avtTotalSurfaceAreaQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    if  (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() < 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Surface Area query", 
                   "2D or 3D surface");
    }
}



// ****************************************************************************
//  Method: avtTotalSurfaceAreaQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Notes:  Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter,
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002   
//    Moved artificial pipeline creation from base class.
//
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004 
//    Added logic for time-varying case. 
//
// ****************************************************************************

avtDataObject_p 
avtTotalSurfaceAreaQuery::ApplyFilters(avtDataObject_p inData)
{
    if (!timeVarying)
    {
        //
        // Create an artificial pipeline.
        //
        avtPipelineSpecification_p pspec  = 
              inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
        avtDataset_p ds;
        CopyTo(ds, inData);
        avtSourceFromAVTDataset termsrc(ds);
        avtDataObject_p dob = termsrc.GetOutput();

        facelist->SetInput(dob);
        area->SetInput(facelist->GetOutput());
        avtDataObject_p objOut = area->GetOutput();
        objOut->Update(pspec);

        return objOut;
    }
    else
    {
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec->GetVariable(), queryAtts.GetTimeStep(), 
                                 oldSpec->GetRestriction());

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        newDS->GetRestriction()->TurnOnAll();
        for (int i = 0; i < silUseSet.size(); i++)
        {
            if (silUseSet[i] == 0)
                newDS->GetRestriction()->TurnOffSet(i);
        }

        avtDataObject_p dob;
        CopyTo(dob, inData);
        facelist->SetInput(dob);
        area->SetInput(facelist->GetOutput());
        avtDataObject_p objOut = area->GetOutput();
        objOut->Update(pspec); 

        return objOut;
    }
}


