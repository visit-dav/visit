// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLOT_AND_OPERATOR_ACTIONS_UI_H
#define PLOT_AND_OPERATOR_ACTIONS_UI_H
#include <ViewerActionUISingle.h>
#include <ViewerActionUIMultiple.h>
#include <VariableMenuPopulator.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: AddOperatorActionUI
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

class AddOperatorActionUI : public ViewerActionUIMultiple
{
public:
    AddOperatorActionUI(ViewerActionLogic *L);
    virtual ~AddOperatorActionUI();

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;

    virtual void ConstructToolbar(QToolBar *toolbar);
private:
    intVector graphicalPlugins;
};

// ****************************************************************************
// Class: RemoveLastOperatorActionUI
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

class RemoveLastOperatorActionUI : public ViewerActionUISingle
{
public:
    RemoveLastOperatorActionUI(ViewerActionLogic *L);
    virtual ~RemoveLastOperatorActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: RemoveAllOperatorsActionUI
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

class RemoveAllOperatorsActionUI : public ViewerActionUISingle
{
public:
    RemoveAllOperatorsActionUI(ViewerActionLogic *L);
    virtual ~RemoveAllOperatorsActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: AddPlotActionUI
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
//   Brad Whitlock, Mon Sep 29 17:38:36 PST 2003
//   I separated host and database into two fields.
//
//   Brad Whitlock, Tue Mar 16 15:35:44 PST 2004
//   I added changeMenuIconSize.
//
//   Brad Whitlock, Fri Apr 15 13:57:12 PST 2005
//   I removed host and database.
//
//   Brad Whitlock, Thu May 29 16:42:43 PDT 2008
//   Removed menu and a slot.
//
//   Brad Whitlock, Fri Nov 19 15:05:34 PST 2010
//   I added CreatePlotMenu and DeletePlotMenu.
//
// ****************************************************************************

class AddPlotActionUI : public ViewerActionUIMultiple
{
    Q_OBJECT

    typedef struct
    {
        int                    index;
        QvisVariablePopupMenu *varMenu;
        int                    varTypes;
    } PluginEntry;

    typedef std::vector<PluginEntry> PluginEntryVector;
public:
    AddPlotActionUI(ViewerActionLogic *L);
    virtual ~AddPlotActionUI();

    virtual void Update();
    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;

    virtual void ConstructMenu(QMenu *);
    virtual void RemoveFromMenu(QMenu *);
    virtual void ConstructToolbar(QToolBar *toolbar);
private slots:
    void addPlot(int, const QString &);
    void addPlotAboutToShow();
    void changeMenuIconSize(bool);
private:
    void CreatePlotMenu(int);
    void DeletePlotMenu(int);
    int                   maxPixmapWidth, maxPixmapHeight;
    bool                  addPlotMenuHasBeenShown;
    PluginEntryVector     pluginEntries;
    VariableMenuPopulator menuPopulator;
};

// ****************************************************************************
// Class: DrawPlotsActionUI
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

class DrawPlotsActionUI : public ViewerActionUISingle
{
public:
    DrawPlotsActionUI(ViewerActionLogic *L);
    virtual ~DrawPlotsActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: HideActivePlotsActionUI
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

class HideActivePlotsActionUI : public ViewerActionUISingle
{
public:
    HideActivePlotsActionUI(ViewerActionLogic *L);
    virtual ~HideActivePlotsActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: DeleteActivePlotsActionUI
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

class DeleteActivePlotsActionUI : public ViewerActionUISingle
{
public:
    DeleteActivePlotsActionUI(ViewerActionLogic *L);
    virtual ~DeleteActivePlotsActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: CopyPlotActionUI
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

class CopyPlotActionUI : public ViewerActionUISingle
{
public:
    CopyPlotActionUI(ViewerActionLogic *L);
    virtual ~CopyPlotActionUI() { }
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: SetPlotFollowsTimeActionUI
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

class SetPlotFollowsTimeActionUI : public ViewerActionUISingle
{
public:
    SetPlotFollowsTimeActionUI(ViewerActionLogic *L);
    virtual ~SetPlotFollowsTimeActionUI() { }
    virtual bool Enabled() const;
};
#endif


