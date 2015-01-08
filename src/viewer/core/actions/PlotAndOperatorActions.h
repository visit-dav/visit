/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef PLOT_AND_OPERATOR_ACTIONS_H
#define PLOT_AND_OPERATOR_ACTIONS_H
#include <ViewerActionLogic.h>
#include <vectortypes.h>

// ****************************************************************************
// class VIEWERCORE_API : AddOperatorAction
//
// Purpose:
//   This action adds an operator to the plots in a window's plot list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:24:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AddOperatorAction : public ViewerActionLogic
{
public:
    AddOperatorAction(ViewerWindow *win);
    virtual ~AddOperatorAction() {}

    virtual void Execute();

protected:
    virtual void Setup(int activeAction, bool toggled);
private:
    intVector graphicalPlugins;
};

// ****************************************************************************
// class VIEWERCORE_API : PromoteOperatorAction
//
// Purpose:
//   This action promotes an operator in the window's selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:38:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API PromoteOperatorAction : public ViewerActionLogic
{
public:
    PromoteOperatorAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~PromoteOperatorAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : DemoteOperatorAction
//
// Purpose:
//   This action demotes an operator in the window's selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:38:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DemoteOperatorAction : public ViewerActionLogic
{
public:
    DemoteOperatorAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DemoteOperatorAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : RemoveOperatorAction
//
// Purpose:
//   This action removes an operator from the window's selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:38:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RemoveOperatorAction : public ViewerActionLogic
{
public:
    RemoveOperatorAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~RemoveOperatorAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : RemoveLastOperatorAction
//
// Purpose:
//   This action removes the last operator from the window's selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:51:18 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RemoveLastOperatorAction : public ViewerActionLogic
{
public:
    RemoveLastOperatorAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~RemoveLastOperatorAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : RemoveAllOperatorsAction
//
// Purpose:
//   This action removes all operators from the window's selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:51:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RemoveAllOperatorsAction : public ViewerActionLogic
{
public:
    RemoveAllOperatorsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~RemoveAllOperatorsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetOperatorOptionsAction
//
// Purpose:
//   This action sets the operator attributes for an operator.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:48:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetOperatorOptionsAction : public ViewerActionLogic
{
public:
    SetOperatorOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetOperatorOptionsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : AddPlotAction
//
// Purpose:
//   This action adds a plot to the window's plot list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:24:19 PDT 2003
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API AddPlotAction : public ViewerActionLogic
{
public:
    AddPlotAction(ViewerWindow *win);
    virtual ~AddPlotAction() {}

    void SetArguments(int plotType, const std::string &var);
    virtual void Execute();
private:
    intVector graphicalPlugins;
};

// ****************************************************************************
// class VIEWERCORE_API : AddEmbeddedPlotAction
//
// Purpose:
//   This action adds a plot to the window's plot list, using an id specified
//   by the embedding code.
//   It also differs from the regular AddPlot because it isn't exposed in a menu
//
// Notes:      
//
// Programmer: Marc Durant
// Creation:   June 19, 2011
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API AddEmbeddedPlotAction : public ViewerActionLogic
{
public:
    AddEmbeddedPlotAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~AddEmbeddedPlotAction() {}  
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : DrawPlotsAction
//
// Purpose:
//   This action draws the window's plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:51:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DrawPlotsAction : public ViewerActionLogic
{
public:
    DrawPlotsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DrawPlotsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : HideActivePlotsAction
//
// Purpose:
//   This action hides the window's active plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:51:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API HideActivePlotsAction : public ViewerActionLogic
{
public:
    HideActivePlotsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~HideActivePlotsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotFollowsTimeAction
//
// Purpose:
//   This action disconnects the window's active plots from the time slider.
//
// Notes:      
//
// Programmer: Ellen Tarwater
// Creation:   Thurs, Dec 6, 2007
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotFollowsTimeAction : public ViewerActionLogic
{
public:
    SetPlotFollowsTimeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotFollowsTimeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : DeleteActivePlotsAction
//
// Purpose:
//   This action deletes the window's active plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:51:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteActivePlotsAction : public ViewerActionLogic
{
public:
    DeleteActivePlotsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeleteActivePlotsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetActivePlotsAction
//
// Purpose:
//   This action sets the window's active plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:30:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetActivePlotsAction : public ViewerActionLogic
{
public:
    SetActivePlotsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetActivePlotsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : ChangeActivePlotsVarAction
//
// Purpose:
//   This action sets the plotted variable for the selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:30:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ChangeActivePlotsVarAction : public ViewerActionLogic
{
public:
    ChangeActivePlotsVarAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ChangeActivePlotsVarAction() {}
    virtual void Execute();
};


// ****************************************************************************
// class VIEWERCORE_API : SetPlotSILRestrictionAction
//
// Purpose:
//   This action sets the SIL restriction for a plot.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:47:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotSILRestrictionAction : public ViewerActionLogic
{
public:
    SetPlotSILRestrictionAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotSILRestrictionAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotOptionsAction
//
// Purpose:
//   This action sets the plot attributes for the selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:48:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotOptionsAction : public ViewerActionLogic
{
public:
    SetPlotOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotOptionsAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotFrameRangeAction
//
// Purpose:
//   This action setes the frames over which a plot exists.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:48:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotFrameRangeAction : public ViewerActionLogic
{
public:
    SetPlotFrameRangeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotFrameRangeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : DeletePlotKeyframeAction
//
// Purpose:
//   Deletes a plot keyframe.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:48:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeletePlotKeyframeAction : public ViewerActionLogic
{
public:
    DeletePlotKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeletePlotKeyframeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : MovePlotKeyframeAction
//
// Purpose:
//   Moves a plot keyframe.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MovePlotKeyframeAction : public ViewerActionLogic
{
public:
    MovePlotKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MovePlotKeyframeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotDatabaseStateAction
//
// Purpose:
//   Sets a database keyframe for a plot.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:49:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotDatabaseStateAction : public ViewerActionLogic
{
public:
    SetPlotDatabaseStateAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotDatabaseStateAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : DeletePlotDatabaseKeyframeAction
//
// Purpose:
//   Deletes a database keyframe.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:49:02 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeletePlotDatabaseKeyframeAction : public ViewerActionLogic
{
public:
    DeletePlotDatabaseKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeletePlotDatabaseKeyframeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : MovePlotDatabaseKeyframeAction
//
// Purpose:
//   Moves a database keyframe.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:49:03 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MovePlotDatabaseKeyframeAction : public ViewerActionLogic
{
public:
    MovePlotDatabaseKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MovePlotDatabaseKeyframeAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : CopyPlotAction
//
// Purpose:
//   This action copies the window's active plots.
//
// Notes:      
//
// Programmer: Ellen Tarwater
// Creation:   Fri Sept 28  2007
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CopyPlotAction : public ViewerActionLogic
{
public:
    CopyPlotAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CopyPlotAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotDescriptionAction
//
// Purpose:
//   This action sets the plot description
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 20 13:45:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotDescriptionAction : public ViewerActionLogic
{
public:
    SetPlotDescriptionAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotDescriptionAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : MovePlotOrderTowardFirstAction
//
// Purpose:
//   This action moves a plot one slot closer to the plot list start.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 20 13:45:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MovePlotOrderTowardFirstAction : public ViewerActionLogic
{
public:
    MovePlotOrderTowardFirstAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MovePlotOrderTowardFirstAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : MovePlotOrderTowardLastAction
//
// Purpose:
//   This action moves a plot one slot closer to the plot list end.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 20 13:45:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MovePlotOrderTowardLastAction : public ViewerActionLogic
{
public:
    MovePlotOrderTowardLastAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MovePlotOrderTowardLastAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotOrderToFirstAction
//
// Purpose:
//   This action moves a plot to the plot list start.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 20 13:45:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotOrderToFirstAction : public ViewerActionLogic
{
public:
    SetPlotOrderToFirstAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotOrderToFirstAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetPlotOrderToLastAction
//
// Purpose:
//   This action moves a plot to the plot list start.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 20 13:45:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPlotOrderToLastAction : public ViewerActionLogic
{
public:
    SetPlotOrderToLastAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPlotOrderToLastAction() {}
    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : AddInitializedOperatorAction
//
// Purpose:
//   Handles ViewerRPC::AddInitializedOperatorRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AddInitializedOperatorAction : public ViewerActionLogic
{
public:
    AddInitializedOperatorAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~AddInitializedOperatorAction() {}

    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : ResetOperatorOptionsAction
//
// Purpose:
//   Handles ViewerRPC::ResetOperatorOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetOperatorOptionsAction : public ViewerActionLogic
{
public:
    ResetOperatorOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetOperatorOptionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : ResetPlotOptionsAction
//
// Purpose:
//   Handles ViewerRPC::ResetPlotOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetPlotOptionsAction : public ViewerActionLogic
{
public:
    ResetPlotOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetPlotOptionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetDefaultOperatorOptionsAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultOperatorOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultOperatorOptionsAction : public ViewerActionLogic
{
public:
    SetDefaultOperatorOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultOperatorOptionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// class VIEWERCORE_API : SetDefaultPlotOptionsAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultPlotOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultPlotOptionsAction : public ViewerActionLogic
{
public:
    SetDefaultPlotOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultPlotOptionsAction() {}

    virtual void Execute();
};

#endif


