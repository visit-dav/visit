// ************************************************************************* //
//                              ViewerOperator.C                             //
// ************************************************************************* //

#include <ViewerOperatorFactory.h>

#include <AttributeSubject.h>
#include <DataNode.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <ViewerEngineManager.h>
#include <ViewerOperator.h>
#include <ViewerPlot.h>
#include <avtPluginFilter.h>
#include <VisItException.h>

// ****************************************************************************
//  Method: ViewerOperator constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   July 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Eric Brugger, Wed Jan 15 15:47:03 PST 2003
//    Modify the plot data member so that it is not const.
//
//    Jeremy Meredith, Wed May 21 13:07:59 PDT 2003
//    Added needsRecalculation.
//
//    Kathleen Bonnell, Thu Sep 11 11:37:51 PDT 2003 
//    Added fromDefault arg, passed to InitializeOperatorAtts. 
//
// ****************************************************************************

ViewerOperator::ViewerOperator(const int type_, 
                               ViewerOperatorPluginInfo *viewerPluginInfo_,
                               ViewerPlot *plot_, const bool fromDefault)
{
    type              = type_;
    viewerPluginInfo  = viewerPluginInfo_;
    plot              = plot_;
    operatorAtts      = viewerPluginInfo->AllocAttributes();
    avtfilter         = NULL;
    needsRecalculation= true;

    viewerPluginInfo->InitializeOperatorAtts(operatorAtts, plot, fromDefault);
}

// ****************************************************************************
//  Method: ViewerOperator destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   July 25, 2001
//
//  Modifications:
//
// ****************************************************************************

ViewerOperator::~ViewerOperator()
{
    delete operatorAtts;

    if (avtfilter)
        delete avtfilter;
}

// ****************************************************************************
//  Method:  ViewerOperator::ExecuteEngineRPC
//
//  Purpose:
//    Execute the engine RPC to create the operator.
//
//  Arguments:
//    engine     The engine to use
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Mon Oct 22 18:46:45 PST 2001
//    Removed exception code since it was catching all VisItExceptions and
//    just re-throwing them. That code was completely pointless.
//
//    Brad Whitlock, Fri Feb 22 17:28:32 PST 2002
//    Rewrote the method so it goes through the engine manager.
//
// ****************************************************************************

bool
ViewerOperator::ExecuteEngineRPC() const
{
    ViewerEngineManager *engineMgr = ViewerEngineManager::Instance();
    return engineMgr->ApplyOperator(plot->GetHostName().c_str(),
                                    viewerPluginInfo->GetID(),
                                    operatorAtts);
}

// ****************************************************************************
//  Method:  ViewerOperator::GetType
//
//  Purpose:
//    Return the type of the operator.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
// ****************************************************************************

int
ViewerOperator::GetType() const
{
    return type;
}

// ****************************************************************************
//  Method:  ViewerOperator::SetClientAttsFromOperator
//
//  Purpose:
//    Set the client attributes based on the operator attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
// ****************************************************************************

void
ViewerOperator::SetClientAttsFromOperator()
{
    //
    // Set the client attributes.
    //
    viewerPluginInfo->SetClientAtts(operatorAtts);
}

// ****************************************************************************
//  Method:  ViewerOperator::SetOperatorAttsFromClient
//
//  Purpose:
//    Set the operator attributes based on the client attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed May 21 13:07:02 PDT 2003
//    Made it record if the operator requires recalculation given its
//    current and new states.
//
// ****************************************************************************

void
ViewerOperator::SetOperatorAttsFromClient()
{
    //
    // Check to see if we need to recalculate when we're done
    //
    needsRecalculation = (!operatorAtts->EqualTo(
                                           viewerPluginInfo->GetClientAtts()));

    //
    // Copy the operator attributes to the client attributes and notify the
    // client.
    //
    viewerPluginInfo->GetClientAtts(operatorAtts);

    if (avtfilter != 0)
    {
        avtfilter->SetAtts(operatorAtts);
    }
}

// ****************************************************************************
// Method: ViewerOperator::SetOperatorAtts
//
// Purpose: 
//   Tries to copy the incoming atts into the operatorAtts. This only happens
//   if they are compatible types.
//
// Arguments:
//   atts : The new attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 9 15:30:41 PST 2001
//
// Modifications:
//   Eric Brugger, Wed Jan 15 15:47:03 PST 2003
//   Modify the routine to clear the actors associated with the plot if
//   the attributes were successfully changed.
//   
//    Jeremy Meredith, Wed May 21 13:07:42 PDT 2003
//    Made it record if the operator requires recalculation given its
//    current and new states.
//
// ****************************************************************************

bool
ViewerOperator::SetOperatorAtts(const AttributeSubject *atts)
{
    //
    // Check to see if we need to recalculate when we're done
    //
    bool mightNeedRecalculation = (!operatorAtts->EqualTo(atts));

    //
    // Copy the operator attributes.  If it is successful, then they
    // must be compatible, so set the attributes of the filter and
    // clear the actors.
    //
    bool retval = false;
    if (operatorAtts->CopyAttributes(atts))
    {
        if (avtfilter != 0)
            avtfilter->SetAtts(operatorAtts);

        if (mightNeedRecalculation)
        {
            plot->ClearActors();
            retval = true;
            needsRecalculation = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerOperator::GetOperatorAtts
//
// Purpose: 
//   Returns a const pointer to the operator attributes.
//
// Returns:    A const pointer to the operator attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 11 14:17:39 PST 2002
//
// Modifications:
//   
// ****************************************************************************

const AttributeSubject *
ViewerOperator::GetOperatorAtts() const
{
    return operatorAtts;
}


// ****************************************************************************
//  Method:  ViewerOperator::GetName
//
//  Purpose:
//    Return the name of the operator.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 5, 2002 
//
// ****************************************************************************

const char *
ViewerOperator::GetName() const
{
    return viewerPluginInfo->GetName();
}

// ****************************************************************************
// Method: ViewerOperator::GetPluginID
//
// Purpose: Returns the operator's plugin id.
//   
// Returns:    A string that tells which plugin created the operator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 09:20:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

const char *
ViewerOperator::GetPluginID() const
{
    return viewerPluginInfo->GetID();
}

// ****************************************************************************
// Method: ViewerOperator::Removeable
//
// Purpose: 
//   Returns whether or not the operator can be removed.
//
// Returns:    True if the operator can be removed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 11:31:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerOperator::Removeable() const
{
    return viewerPluginInfo->Removeable();
}

// ****************************************************************************
// Method: ViewerOperator::Moveable
//
// Purpose: 
//   Returns whether or not the operator can be moved.
//
// Returns:    True if the operator can be removed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 11:31:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerOperator::Moveable() const
{
    return viewerPluginInfo->Moveable();
}

// ****************************************************************************
// Method: ViewerOperator::AllowsSubsequentOperators
//
// Purpose: 
//   Returns whether or not the operator allows more operators to be applied
//   after it.
//
// Returns:    True if the operator allows subsequent operators; false
//             otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 11:31:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerOperator::AllowsSubsequentOperators() const
{
    return viewerPluginInfo->AllowsSubsequentOperators();
}


// ****************************************************************************
//  Method:  
//
//  Purpose:
//    Returns true if the operator attributes have changed enough to
//    require recalculation.
//
//  Note:
//    Right now, this only checks of the attributes have changed at all;
//    only plots have attributes which change the display but not the
//    data pipeline.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 21, 2003
//
// ****************************************************************************

bool
ViewerOperator::NeedsRecalculation() const
{
    return needsRecalculation;
}


// ****************************************************************************
// Method: ViewerOperator::CreateNode
//
// Purpose: 
//   Lets the operator save its information for a config file's DataNode.
//
// Arguments:
//   parentNode : The node to which we're saving information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:09:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 15 16:30:48 PST 2003
//   I added the new completeSave argument to AttributeSubject::CreateNode to
//   force the operator attributes to write out all of their fields to avoid
//   unwanted settings from the system configs. This makes the session file
//   reproduce the same thing each time without having to run -noconfig.
//   
// ****************************************************************************

void
ViewerOperator::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *operatorNode = new DataNode("ViewerOperator");

    // Add the operator attributes.
    if(operatorAtts->CreateNode(operatorNode, true, true))
        parentNode->AddNode(operatorNode);
    else
        delete operatorNode;
}

// ****************************************************************************
// Method: ViewerOperator::SetFromNode
//
// Purpose: 
//   Lets the operator reset its values from a config file.
//
// Arguments:
//   parentNode : The config file information DataNode pointer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:10:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerOperator::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *operatorNode = parentNode->GetNode("ViewerOperator");
    if(operatorNode == 0)
        return;

    // Let the operator try to initialize its attributes.
    operatorAtts->SetFromNode(operatorNode);
}
