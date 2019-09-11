// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTrajectoryByZone.                            //
// ************************************************************************* //

#include <avtTrajectoryByZone.h>
#include <avtParallel.h>
#include <float.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtTrajectoryByZone::avtTrajectoryByZone
//
//  Purpose:
//      Construct an avtTrajectoryByZone object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByZone::avtTrajectoryByZone()
{
}

// ****************************************************************************
//  Method: avtTrajectoryByZone::~avtTrajectoryByZone
//
//  Purpose:
//      Destruct an avtTrajectoryByZone object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005 
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByZone::~avtTrajectoryByZone()
{
}

 
// ****************************************************************************
//  Method: avtTrajectoryByZone::Preparation
//
//  Purpose:
//    Sets pickAtts based on queryAtts. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//
// ****************************************************************************

void 
avtTrajectoryByZone::Preparation(const avtDataAttributes &inAtts)
{
    if (inAtts.ValidVariable(queryAtts.GetVariables()[0].c_str()))
        queryAtts.SetXUnits(inAtts.GetVariableUnits(queryAtts.GetVariables()[0].c_str()));
    if (inAtts.ValidVariable(queryAtts.GetVariables()[1].c_str()))
        queryAtts.SetYUnits(inAtts.GetVariableUnits(queryAtts.GetVariables()[1].c_str()));
 
    avtVariableByZoneQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtTrajectoryByZone::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 8, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue Mar 13 11:27:42 PDT 2007
//    Updated due to code generation changes.
//
// ****************************************************************************

void
avtTrajectoryByZone::PostExecute(void)
{
    avtPickQuery::PostExecute();
   
    if (PAR_Rank() == 0) 
    {
        if (pickAtts.GetFulfilled())
        {
            // Special indication that the pick point should not be displayed.
            double cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            string msg;
            pickAtts.SetCellPoint(cp);
            pickAtts.CreateOutputString(msg);
            SetResultMessage(msg.c_str());
            doubleVector res;
            res.push_back(pickAtts.GetVarInfo(0).GetValues()[0]);
            res.push_back(pickAtts.GetVarInfo(1).GetValues()[0]);
            SetResultValues(res);

        }
        else
        {
            char msg[120]; 
            snprintf(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", domain, zone);
            SetResultMessage(msg);
        }
    }
    pickAtts.PrepareForNewPick();
}


// ****************************************************************************
//  Method: avtTrajectoryByZone::GetTimeCurveSpecs
//
//  Purpose:
//    Override default TimeCurveSpecs 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 8, 2008 
//
//  Modifications:
//
// ****************************************************************************

const MapNode&
avtTrajectoryByZone::GetTimeCurveSpecs() 
{
    timeCurveSpecs["useTimeForXAxis"] = false;
    timeCurveSpecs["nResultsToStore"] = 2;
    return timeCurveSpecs;
}


// ****************************************************************************
// Method:  avtTrajectoryByZone::GetDefaultInputParams
//
// Programmer:  Kathleen Biagas 
// Creation:    July 26, 2011
//
// ****************************************************************************

void
avtTrajectoryByZone::GetDefaultInputParams(MapNode &params)
{
    stringVector v;
    v.push_back("var_for_x");
    v.push_back("var_for_y");
    params["vars"] = v;
    params["domain"] = 0;
    params["element"] = 0;
    params["use_global_id"] = 0;
}
