#ifndef VIEWER_MULTIPLE_ACTION_H
#define VIEWER_MULTIPLE_ACTION_H
#include <viewer_exports.h>
#include <ViewerActionBase.h>
#include <qpixmap.h>
#include <vector>

class QAction;
class QActionGroup;
class QIconSet;
class QPopupMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerMultipleAction
//
// Purpose:
//   This is a base class for actions that multiple toolbar buttons or menu
//   options but still service a single RPC.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 13:49:49 PST 2003
//
// Modifications:
//   Sean Ahern, Thu Feb 20 01:51:35 America/Los_Angeles 2003
//   Added the ability to set large and small icons.
//
//   Brad Whitlock, Tue Aug 26 17:12:02 PST 2003
//   I added the isExclusive flag.
//
// ****************************************************************************

class VIEWER_API ViewerMultipleAction : public ViewerActionBase
{
    Q_OBJECT

    typedef std::vector<QAction *> ActionPointerVector;
public:
    ViewerMultipleAction(ViewerWindow *win, const char *name = 0);
    virtual ~ViewerMultipleAction();
    
    virtual void Setup();
    virtual void Execute();
    virtual void Update();

    virtual void Execute(int) = 0;

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceToggled(int i) const;

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QPopupMenu *menu);
    virtual void RemoveFromMenu(QPopupMenu *menu);
    virtual void ConstructToolbar(QToolBar *toolbar);
    virtual void RemoveFromToolbar(QToolBar *toolbar);
    virtual void UpdateConstruction();

    // Methods to set the action's attributes.
    virtual void SetAllText(const char *text);
    virtual void SetText(const char *text);
    virtual void SetMenuText(const char *text);
    virtual void SetToolTip(const char *text);
    virtual void SetIconSet(const QIconSet &icons);

    virtual void AddChoice(const char *menuText);
    virtual void AddChoice(const char *menuText, const char *toolTip, const
                           QPixmap &icon);
    virtual void AddChoice(const char *menuText, const char *toolTip,
                           const QPixmap &small_icon,
                           const QPixmap &large_icon);

    virtual void SetExclusive(bool val);
protected slots:
    // Helper slots for when the choices are not exclusive.
    void activate0();
    void activate1();
    void activate2();
    void activate3();
    void activate4();
    void activate5();
    void activate6();
    void activate7();
    void activate8();
    void activate9();
    void activate10();
    void activate11();
    void activate12();
    void activate13();
    void activate14();
    void activate15();
    void activate16();
    void activate17();
    void activate18();
    void activate19();

protected:
    void ConnectChildAction(QAction *newAction);
    void ActivateHelper(int i);

    bool                 iconSpecified;
    int                  activeAction;
    bool                 toggled;
    QActionGroup        *action;
    QPopupMenu          *actionMenu;
    int                  actionMenuId;
    bool                 isExclusive;
    ActionPointerVector  children;
};

#endif
