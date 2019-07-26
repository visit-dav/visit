// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DDT_ACTIONS_UI_H
#define DDT_ACTIONS_UI_H

#include <ViewerActionUISingle.h>
#include <ViewerActionUIToggle.h>
#include <ViewerActionUIMultiple.h>

// ****************************************************************************
// Class: ReleaseToDDTActionUI
//
// Purpose:
//   Handles the 'release to DDT' action.
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//
// ****************************************************************************

class ReleaseToDDTActionUI : public ViewerActionUISingle
{
public:
    ReleaseToDDTActionUI(ViewerActionLogic *L);
    virtual ~ReleaseToDDTActionUI(){}

    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: PlotDDTVispointVariablesActionUI
//
// Purpose:
//   Action to automatically plot variables at the current DDT vispoint.
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   July 15, 2013
//
// Modifications:
//
// ****************************************************************************

class PlotDDTVispointVariablesActionUI : public ViewerActionUISingle
{
public:
    PlotDDTVispointVariablesActionUI(ViewerActionLogic *L);
    virtual ~PlotDDTVispointVariablesActionUI(){}

    virtual bool Enabled() const;
};

#endif
