// ************************************************************************* //
//                          avtVariableByNodeQuery.C                         //
// ************************************************************************* //

#include <avtVariableByNodeQuery.h>
#include <avtTerminatingSource.h>
#include <avtParallel.h>
#include <snprintf.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtVariableByNodeQuery::avtVariableByNodeQuery
//
//  Purpose:
//      Construct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtVariableByNodeQuery::avtVariableByNodeQuery()
{
}

// ****************************************************************************
//  Method: avtVariableByNodeQuery::~avtVariableByNodeQuery
//
//  Purpose:
//      Destruct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtVariableByNodeQuery::~avtVariableByNodeQuery()
{
}

 
// ****************************************************************************
//  Method: avtVariableByNodeQuery::Preparation
//
//  Purpose:
//    Sets pickAtts information from queryAtts. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

void 
avtVariableByNodeQuery::Preparation()
{
    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();

    pickAtts.SetTimeStep(queryAtts.GetTimeStep());
    pickAtts.SetActiveVariable(dspec->GetVariable());
    pickAtts.SetDomain(queryAtts.GetDomain());
    pickAtts.SetElementNumber(queryAtts.GetElement());
    pickAtts.SetVariables(queryAtts.GetVariables());
    pickAtts.SetPickType(PickAttributes::DomainNode);

    avtPickByNodeQuery::Preparation();
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtVariableByNodeQuery::PostExecute(void)
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
            SetResultValues(pickAtts.GetPickVarInfo(0).GetValues());
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

