#ifndef VIEWER_ACTION_BASE_H
#define VIEWER_ACTION_BASE_H
#include <viewer_exports.h>
#include <qobject.h>
#include <ViewerRPC.h>

class DataNode;
class QPopupMenu;
class QToolBar;
class ViewerWindow;
class ViewerWindowManager;

// ****************************************************************************
// Class: ViewerActionBase
//
// Purpose:
//   This is an abstract base class for actions that the viewer can perform.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 09:44:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 09:36:58 PDT 2003
//   I added the UpdateConstruction, RemoveFromMenu, RemoveFromToolbar methods.
//
//   Brad Whitlock, Tue Jul 1 10:17:46 PDT 2003
//   I added CreateNode and SetFromNode.
//
// ****************************************************************************

class VIEWER_API ViewerActionBase : public QObject
{
    Q_OBJECT
public:
    ViewerActionBase(ViewerWindow *win, const char *name = 0);
    virtual ~ViewerActionBase();
    
    ViewerWindow *GetWindow() const        { return window; }
    const char *GetName() const            { return name(); }

    virtual void Execute() = 0;
    virtual void Update() = 0;

    virtual bool Enabled() const           { return true; }
    virtual bool VisualEnabled() const     { return allowVisualRepresentation; }
    virtual bool MenuTopLevel() const      { return false; }
    virtual bool CanHaveOwnToolbar() const { return false; }
    virtual bool AllowInToolbar() const    { return true; }

    virtual bool CreateNode(DataNode *)    { return false; }
    virtual void SetFromNode(DataNode *)   { }

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QPopupMenu *menu) = 0;
    virtual void RemoveFromMenu(QPopupMenu *menu) = 0;
    virtual void ConstructToolbar(QToolBar *toolbar) = 0;
    virtual void RemoveFromToolbar(QToolBar *toolbar) = 0;
    virtual void UpdateConstruction() { }

    static  void SetArgs(const ViewerRPC &a);
public slots:
    virtual void Activate();
    virtual void Activate(bool setup);
protected:
    virtual void Setup() = 0;
    virtual void PreExecute() { }

    // Methods to set the action's attributes.
    virtual void SetText(const char *text) = 0;
    virtual void SetMenuText(const char *text) = 0;
    virtual void SetToolTip(const char *text) = 0;
    virtual void DisableVisual()       { allowVisualRepresentation = false; }
    virtual void EnableVisual()        { allowVisualRepresentation = true; }

    ViewerWindow               *window;
    int                         windowId;
    bool                        allowVisualRepresentation;

    static ViewerWindowManager *windowMgr;
    static ViewerRPC            args;
};

#endif
