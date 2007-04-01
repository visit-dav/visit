#ifndef VIEWER_TOGGLE_ACTION_H
#define VIEWER_TOGGLE_ACTION_H
#include <ViewerAction.h>
#include <qpixmap.h>

// ****************************************************************************
// Class: ViewerToggleAction
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

class VIEWER_API ViewerToggleAction : public ViewerAction
{
public:
    ViewerToggleAction(ViewerWindow *win, const char *name = 0);
    virtual ~ViewerToggleAction();

    virtual void Update();

protected:
    virtual void PreExecute();
    virtual void SetIcons(const QPixmap &p1, const QPixmap &p2);

    bool     toggled;
    QPixmap *regularIcon;
    QPixmap *toggledIcon;
};

#endif
