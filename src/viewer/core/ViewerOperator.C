// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ViewerOperator.C                             //
// ************************************************************************* //

#include <ViewerOperatorFactory.h>

#include <AttributeSubject.h>
#include <AttributeSubjectMap.h>
#include <DataNode.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerOperator.h>
#include <ViewerPlot.h>
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
//    Brad Whitlock, Mon Feb 12 17:50:29 PST 2007
//    Added ViewerBase base class.
//
//    Brad Whitlock, Fri May  9 14:37:00 PDT 2008
//    Qt 4.
//
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

ViewerOperator::ViewerOperator(const int type_, 
    ViewerOperatorPluginInfo *viewerPluginInfo_,
    const bool keyframeMode_,
    const int cacheIndex_, const int cacheSize_,
    ViewerPlot *plot_, const bool fromDefault) : ViewerBase()
{
    type               = type_;
    viewerPluginInfo   = viewerPluginInfo_;
    curOperatorAtts    = viewerPluginInfo->AllocAttributes();
    keyframeMode       = keyframeMode_;
    cacheIndex         = cacheIndex_;
    cacheSize          = cacheSize_;
    plot               = plot_;
    needsRecalculation = true;

    avtPlotMetaData pmd(plot->GetPlotMetaData());
    viewerPluginInfo->InitializeOperatorAtts(curOperatorAtts, pmd, fromDefault);

    operatorAtts = new AttributeSubjectMap;
    operatorAtts->SetAtts(0, curOperatorAtts);
}

// ****************************************************************************
// Method: ViewerOperator::ViewerOperator
//
// Purpose: 
//   Copy constructor for the ViewerOperator class.
//
// Arguments:
//   obj : The operator to copy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 11:43:39 PDT 2004
//
// Modifications:
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

ViewerOperator::ViewerOperator(const ViewerOperator &obj)
{
    type               = obj.type;
    viewerPluginInfo   = obj.viewerPluginInfo;
    operatorAtts       = new AttributeSubjectMap(*(obj.operatorAtts));
    curOperatorAtts    = obj.curOperatorAtts->NewInstance(true);
    keyframeMode       = obj.keyframeMode;
    cacheIndex         = obj.cacheIndex;
    cacheSize          = obj.cacheSize;
    plot               = obj.plot;
    needsRecalculation = true;
}

// ****************************************************************************
//  Method: ViewerOperator destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   July 25, 2001
//
//  Modifications:
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

ViewerOperator::~ViewerOperator()
{
    delete operatorAtts;
    delete curOperatorAtts;
}

// ****************************************************************************
// Method: ViewerOperator::SetPlot
//
// Purpose: 
//   Sets the plot pointer so we can use the copy constructor and then
//   reparent the operator to a different plot.
//
// Arguments:
//   p : The new parent plot.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 14:19:34 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerOperator::SetPlot(ViewerPlot *p)
{
    plot = p;
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
//    Jeremy Meredith, Tue Mar 30 10:38:13 PST 2004
//    Made engines be accessed using a key instead of a hostname.
//
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

bool
ViewerOperator::ExecuteEngineRPC() const
{
    return GetViewerEngineManager()->ApplyOperator(plot->GetEngineKey(),
                                                   viewerPluginInfo->GetID(),
                                                   curOperatorAtts);
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
//  Modifications:
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

void
ViewerOperator::SetClientAttsFromOperator()
{
    //
    // Set the client attributes.
    //
    viewerPluginInfo->SetClientAtts(curOperatorAtts);
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
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

void
ViewerOperator::SetOperatorAttsFromClient(const bool activePlot,
                                          const bool applyToAll)
{
    //
    // Check to see if we need to recalculate when we're done
    //
    needsRecalculation =
      (!curOperatorAtts->EqualTo( viewerPluginInfo->GetClientAtts() ));

    //
    // Copy the operator attributes to the client attributes and notify the
    // client.
    //
    viewerPluginInfo->GetClientAtts(curOperatorAtts, activePlot, applyToAll);

    SetOperatorAtts(curOperatorAtts);

    if( activePlot )
      viewerPluginInfo->SetClientAtts(curOperatorAtts);      
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
//   Jeremy Meredith, Wed May 21 13:07:42 PDT 2003
//   Made it record if the operator requires recalculation given its
//   current and new states.
//
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//
// ****************************************************************************

bool
ViewerOperator::SetOperatorAtts(const AttributeSubject *atts)
{
    //
    // Check to see if we need to recalculate when we're done
    //
    bool mightNeedRecalculation = (!curOperatorAtts->EqualTo(atts));

    //
    // Copy the operator attributes.  If it is successful, then they
    // must be compatible, so set the attributes of the filter and
    // clear the actors.
    //
    bool retval = false;
    if (curOperatorAtts->CopyAttributes(atts))
    {
        if (mightNeedRecalculation)
        {
            plot->ClearActors();
            retval = true;
            needsRecalculation = true;
        }
    }

    if(keyframeMode)
    {
        //
        // Set the operator attributes for the current operator state.
        // SetAtts returns the range of plots that were invalidated.
        // The maximum value is clamped to frame1 since SetAtts may
        // return INT_MAX to indicate the end of the plot.
        //
        int i0, i1;
        operatorAtts->SetAtts(cacheIndex, curOperatorAtts, i0, i1);
        i1 = (i1 <= cacheIndex) ? i1 : cacheIndex;

        // Invalidate the cache if necessary for items i0..i1
        plot->CheckCache(i0, i1, false);
    }
    else
    {
        //
        // Set the operator attributes for the entire plot.
        //
        operatorAtts->SetAtts(curOperatorAtts);
        
        // Invalidate the cache if necessary for all items.
        plot->CheckCache(0, cacheSize-1, false);
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
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

const AttributeSubject *
ViewerOperator::GetOperatorAtts() const
{
    return curOperatorAtts;
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
//  Method:  ViewerOperator::GetMenuName
//
//  Purpose:
//    Return the menu name of the operator. This is the only version of the
//    operator name that may appear in another language. The translation
//    happens when we make the menus.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Apr 29 15:13:57 PDT 2008
//
// ****************************************************************************

const char *
ViewerOperator::GetMenuName() const
{
    return viewerPluginInfo->GetMenuName();
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
//  Method:  ViewerOperator::GetCreatedVariableNames
//
//  Purpose:
//    Return whether or not the given viewer operator creates new
//    variables.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 18, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 19 15:39:42 EST 2008
//    Allow operators to construct full expressions for their new variables.
//
//    Hank Childs, Tue Aug 31 08:12:20 PDT 2010
//    Use the GetCreatedExpressions method instead.
//
//    Hank Childs, Thu Dec 30 12:56:21 PST 2010
//    Change argument to meta data, since we can now create expressions for
//    scalars, vectors, tensors, etc.
//
// ****************************************************************************
ExpressionList *
ViewerOperator::GetCreatedVariables(const avtDatabaseMetaData *md)
{
    return viewerPluginInfo->GetCreatedExpressions(md);
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
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

void
ViewerOperator::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *operatorNode = new DataNode("ViewerOperator");
    parentNode->AddNode(operatorNode);

    //
    // Add the keyframed operator attributes.
    //
    DataNode *operatorKFNode = new DataNode("operatorKeyframes");
    if(operatorAtts->CreateNode(operatorKFNode))
        operatorNode->AddNode(operatorKFNode);
    else
        delete operatorKFNode;
}

// ****************************************************************************
// Method: ViewerOperator::SetFromNode
//
// Purpose: 
//   Lets the operator reset its values from a config file.
//
// Arguments:
//   parentNode    : The config file information DataNode pointer.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:10:51 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

void
ViewerOperator::SetFromNode(DataNode *parentNode, const std::string &configVersion)
{
    DataNode *node;

    if(parentNode == 0)
        return;

    DataNode *operatorNode = parentNode->GetNode("ViewerOperator");
    if(operatorNode == 0)
        return;

    // Give the operator a chance to try and update its data node representation
    // from an older representation before we set from node.
    curOperatorAtts->ProcessOldVersions(operatorNode, configVersion.c_str());

    // Read in the current operator attributes.
    curOperatorAtts->SetFromNode(operatorNode);
    SetOperatorAtts(curOperatorAtts);

    //
    // Read in the plot keyframes.
    //
    if((node = operatorNode->GetNode("operatorKeyframes")) != 0)
    {
        operatorAtts->ProcessOldVersions(node, configVersion, curOperatorAtts);
        operatorAtts->SetFromNode(node, curOperatorAtts);
    }
}

// ****************************************************************************
// Method: ViewerOperator::UpdateOperatorAtts
//
// Purpose: Updates the attributes be set based on the operator
//   expression variable.
//
// Arguments:
//
// Programmer: Allen Sanderson
// Creation:   Wed Apr 10 13:10:51 PST 2013
//
// Modifications:
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

void
ViewerOperator::UpdateOperatorAtts()
{
    viewerPluginInfo->UpdateOperatorAtts(curOperatorAtts, plot->GetPlotMetaData());
}

// ****************************************************************************
// Method: ViewerOperator::UpdateOperatorAtts
//
// Purpose: Updates the attributes be set based on the operator
//   expression variable.
//
// Arguments:
//
// Programmer: Allen Sanderson
// Creation:   Wed Apr 10 13:10:51 PST 2013
//
// Modifications:
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Add operator keyframing.
//   
// ****************************************************************************

std::string
ViewerOperator::GetOperatorVarDescription()
{
  return viewerPluginInfo->GetOperatorVarDescription(curOperatorAtts, plot->GetPlotMetaData());
}

// ****************************************************************************
// Method: ViewerOperator::SetKeyframeMode
//
// Purpose:
//   Sets the operator's keyframe mode.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Wed Mar 22 16:23:12 PDT 2023
//
// Modifications:
//   
// ****************************************************************************

void
ViewerOperator::SetKeyframeMode(const bool keyframeMode_)
{
    //
    // Only set the keyframe mode and cache index in this function.
    // UpdateCacheSize will get called to handle the rest of the changes.
    //
    keyframeMode = keyframeMode_;
    cacheIndex = 0;
}

// ****************************************************************************
// Method: ViewerOperator::SetCacheIndex
//
// Purpose:
//   Sets the operator's cache index, which is used for keyframing
//   opertor attributes.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Wed Mar 22 16:23:12 PDT 2023
//
// Modifications:
//   
// ****************************************************************************

void
ViewerOperator::SetCacheIndex(const int cacheIndex_)
{
    cacheIndex = cacheIndex_;
    operatorAtts->GetAtts(cacheIndex, curOperatorAtts);
}

// ****************************************************************************
// Method: ViewerOperator::UpdateCacheSize
//
// Purpose: 
//   Sets the operator cache size, which is used for keyframing the
//   operator attributes.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Wed Mar 22 16:23:12 PDT 2023
//
// Modifications:
//   
// ****************************************************************************

void
ViewerOperator::UpdateCacheSize(const int cacheSize_)
{
    cacheSize = cacheSize_;

    if(!keyframeMode)
    {
        operatorAtts->ClearAtts();
        operatorAtts->SetAtts(0, curOperatorAtts);
    }
}
