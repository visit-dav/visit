// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_ACTION_UI_TOGGLE_H
#define VIEWER_ACTION_UI_TOGGLE_H
#include <viewer_exports.h>
#include <ViewerActionUISingle.h>
#include <QPixmap>

// ****************************************************************************
// Class: ViewerActionUIToggle
//
// Purpose:
//   This class represents a viewer action that can be toggled.
//
// Notes:       It provides options for having the toggled icon be different
//              than the regular icon.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 3 13:39:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:10:26 PDT 2004
//   I turned the pixmaps into pointers to get rid a runtime warning about
//   creating pixmaps in -nowin mode.
//
// ****************************************************************************

class VIEWER_API ViewerActionUIToggle : public ViewerActionUISingle
{
public:
    ViewerActionUIToggle(ViewerActionLogic *L);
    virtual ~ViewerActionUIToggle();

    virtual bool Checked() const;
    virtual void Update();

protected:
    virtual void PreExecute();
    virtual void PostponeExecute();

    virtual void SetIcons(const QPixmap &p1, const QPixmap &p2);

    bool     toggled;
    QPixmap *regularIcon;
    QPixmap *toggledIcon;
};

#endif
