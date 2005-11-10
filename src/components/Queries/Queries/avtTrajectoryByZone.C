// ************************************************************************* //
//                           avtTrajectoryByZone.                            //
// ************************************************************************* //

#include <avtTrajectoryByZone.h>
#include <avtParallel.h>
#include <float.h>
#include <snprintf.h>
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
            float cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            string msg;
            pickAtts.SetCellPoint(cp);
            pickAtts.CreateOutputString(msg);
            SetResultMessage(msg.c_str());
            doubleVector res;
            res.push_back(pickAtts.GetPickVarInfo(0).GetValues()[0]);
            res.push_back(pickAtts.GetPickVarInfo(1).GetValues()[0]);
            SetResultValues(res);

        }
        else
        {
            char msg[120]; 
            SNPRINTF(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", queryAtts.GetDomain(), queryAtts.GetElement());
            SetResultMessage(msg);
        }
    }
    pickAtts.PrepareForNewPick();
}

