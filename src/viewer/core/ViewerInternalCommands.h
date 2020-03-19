// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_INTERNAL_COMMANDS_H
#define VIEWER_INTERNAL_COMMANDS_H
#include <ViewerBase.h>
#include <VisWindowTypes.h>
#include <string>

class ViewerWindow;

// ****************************************************************************
// Class: ViewerInternalCommand
//
// Purpose:
//   Base class for internal commands that we post to the command queue for
//   execution once the viewer returns to the event loop.
//
// Notes:    This new setup uses these command objects because they can better
//           represent state and it's safer than encoding a lot of parameters
//           into a string as in the old system. Another reason for this new
//           system is that it gets the command execution handlers for the
//           various commands out of ViewerSubject.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:16:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerInternalCommand : public ViewerBase
{
public:
    ViewerInternalCommand() : ViewerBase() { }
    virtual ~ViewerInternalCommand() { }
    virtual void Execute() { }
};

// ****************************************************************************
// Class: ViewerCommandUpdateWindow
//
// Purpose:
//   Internal command to update a window.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandUpdateWindow : public ViewerInternalCommand
{
public:
    ViewerCommandUpdateWindow(ViewerWindow *);
    virtual ~ViewerCommandUpdateWindow() { }
    virtual void Execute();
private:
    ViewerWindow *window;
};

// ****************************************************************************
// Class: ViewerCommandRedrawWindow
//
// Purpose:
//   Internal command to redraw a window.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandRedrawWindow : public ViewerInternalCommand
{
public:
    ViewerCommandRedrawWindow(ViewerWindow *);
    virtual ~ViewerCommandRedrawWindow() { }
    virtual void Execute();
private:
    ViewerWindow *window;
};

// ****************************************************************************
// Class: ViewerCommandDeleteWindow
//
// Purpose:
//   Internal command to delete a window.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandDeleteWindow : public ViewerInternalCommand
{
public:
    ViewerCommandDeleteWindow(ViewerWindow *);
    virtual ~ViewerCommandDeleteWindow() { }
    virtual void Execute();
private:
    ViewerWindow *window;
};

// ****************************************************************************
// Class: ViewerCommandActivateTool
//
// Purpose:
//   Internal command to activate a tool.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandActivateTool : public ViewerInternalCommand
{
public:
    ViewerCommandActivateTool(ViewerWindow *, int);
    virtual ~ViewerCommandActivateTool() { }
    virtual void Execute();
private:
    ViewerWindow *window;
    int           toolId;
};

// ****************************************************************************
// Class: ViewerCommandSetInteractionMode
//
// Purpose:
//   Internal command to set the window interaction mode.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandSetInteractionMode : public ViewerInternalCommand
{
public:
    ViewerCommandSetInteractionMode(ViewerWindow *, INTERACTION_MODE);
    virtual ~ViewerCommandSetInteractionMode() { }
    virtual void Execute();
private:
    ViewerWindow *window;
    INTERACTION_MODE mode;
};

// ****************************************************************************
// Class: ViewerCommandSetToolUpdateMode
//
// Purpose:
//   Internal command to set the tool interaction mode.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandSetToolUpdateMode : public ViewerInternalCommand
{
public:
    ViewerCommandSetToolUpdateMode(ViewerWindow *, TOOLUPDATE_MODE);
    virtual ~ViewerCommandSetToolUpdateMode() { }
    virtual void Execute();
private:
    ViewerWindow *window;
    TOOLUPDATE_MODE mode;
};

// ****************************************************************************
// Class: ViewerCommandUpdateFrame
//
// Purpose:
//   Internal command to update the frame for a window's plot list.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandUpdateFrame : public ViewerInternalCommand
{
public:
    ViewerCommandUpdateFrame(ViewerWindow *);
    virtual ~ViewerCommandUpdateFrame() { }
    virtual void Execute();
private:
    ViewerWindow *window;
};

// ****************************************************************************
// Class: ViewerCommandSetScalableRenderingMode
//
// Purpose:
//   Internal command to set the scalable rendering mode.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandSetScalableRenderingMode : public ViewerInternalCommand
{
public:
    ViewerCommandSetScalableRenderingMode(ViewerWindow *, bool);
    virtual ~ViewerCommandSetScalableRenderingMode() { }
    virtual void Execute();
private:
    ViewerWindow *window;
    bool          mode;
};

// ****************************************************************************
// Class: ViewerCommandFinishLineout
//
// Purpose:
//   Internal command to finish lineout.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandFinishLineout : public ViewerInternalCommand
{
public:
    ViewerCommandFinishLineout() : ViewerInternalCommand() { }
    virtual ~ViewerCommandFinishLineout() { }
    virtual void Execute();
};

// ****************************************************************************
// Class: ViewerCommandFinishLineQuery
//
// Purpose:
//   Internal command to finish line query.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandFinishLineQuery : public ViewerInternalCommand
{
public:
    ViewerCommandFinishLineQuery() : ViewerInternalCommand() { }
    virtual ~ViewerCommandFinishLineQuery() { }
    virtual void Execute();
};

// ****************************************************************************
// Class: ViewerCommandUpdateAnnotationObjectList
//
// Purpose:
//   Internal command to finish line query.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandUpdateAnnotationObjectList : public ViewerInternalCommand
{
public:
    ViewerCommandUpdateAnnotationObjectList() : ViewerInternalCommand() { }
    virtual ~ViewerCommandUpdateAnnotationObjectList() { }
    virtual void Execute();
};

// ****************************************************************************
// Class: ViewerCommandUpdateNamedSelection
//
// Purpose:
//   Internal command to update a window.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:18:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerCommandUpdateNamedSelection : public ViewerInternalCommand
{
public:
    ViewerCommandUpdateNamedSelection(const std::string &);
    virtual ~ViewerCommandUpdateNamedSelection() { }
    virtual void Execute();
private:
    std::string selectionName;
};

#endif
