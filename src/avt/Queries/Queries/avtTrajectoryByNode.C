// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTrajectoryByNode.C                           //
// ************************************************************************* //

#include <avtTrajectoryByNode.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <float.h>
#include <snprintf.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtTrajectoryByNode::avtTrajectoryByNode
//
//  Purpose:
//      Construct an avtTrajectoryByNode object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByNode::avtTrajectoryByNode()
{
}

// ****************************************************************************
//  Method: avtTrajectoryByNode::~avtTrajectoryByNode
//
//  Purpose:
//      Destruct an avtTrajectoryByNode object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005 
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByNode::~avtTrajectoryByNode()
{
}

 
// ****************************************************************************
//  Method: avtTrajectoryByNode::Preparation
//
//  Purpose:
//    Sets pickAtts information from queryAtts. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr  2 10:23:35 PDT 2008
//    Removed unused dataRequest.
//
// ****************************************************************************

void 
avtTrajectoryByNode::Preparation(const avtDataAttributes &inAtts)
{
    if (inAtts.ValidVariable(queryAtts.GetVariables()[0].c_str()))
        queryAtts.SetXUnits(inAtts.GetVariableUnits(
                            queryAtts.GetVariables()[0].c_str()));
    if (inAtts.ValidVariable(queryAtts.GetVariables()[1].c_str()))
        queryAtts.SetYUnits(inAtts.GetVariableUnits(
                            queryAtts.GetVariables()[1].c_str()));

    avtVariableByNodeQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtTrajectoryByNode::PostExecute
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
//    Brad Whitlock, Tue Mar 13 11:26:59 PDT 2007
//    Updated due to code generation changes.
//
// ****************************************************************************

void
avtTrajectoryByNode::PostExecute(void)
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
            SNPRINTF(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", domain, node);
            SetResultMessage(msg);
        }
    }
    pickAtts.PrepareForNewPick();
}

// ****************************************************************************
//  Method: avtTrajectoryByNode::GetTimeCurveSpecs
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
avtTrajectoryByNode::GetTimeCurveSpecs() 
{
    timeCurveSpecs["useTimeForXAxis"] = false;
    timeCurveSpecs["nResultsToStore"] = 2;
    return timeCurveSpecs;
}


// ****************************************************************************
// Method:  avtTrajectoryByNode::GetDefaultInputParams
//
// Programmer:  Kathleen Biagas 
// Creation:    July 26, 2011
//
// ****************************************************************************

void
avtTrajectoryByNode::GetDefaultInputParams(MapNode &params)
{
    stringVector v;
    v.push_back("var_for_x");
    v.push_back("var_for_y");
    params["vars"] = v;
    params["domain"] = 0;
    params["element"] = 0;
    params["use_global_id"] = 0;
}
