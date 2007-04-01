#include <ViewerWindow.h>
#include <ViewerToggleAction.h>
#include <qaction.h>

// ****************************************************************************
// Method: ViewerToggleAction::ViewerToggleAction
//
// Purpose: 
//   Constructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:10:26 PDT 2004
//   I turned the pixmaps into pointers to get rid a runtime warning about
//   creating pixmaps in -nowin mode.
//   
// ****************************************************************************

ViewerToggleAction::ViewerToggleAction(ViewerWindow *win, const char *name) : 
    ViewerAction(win, name)
{
    toggled = false;
    regularIcon = 0;
    toggledIcon = 0;
    SetToggleAction(true);
}

// ****************************************************************************
// Method: ViewerToggleAction::~ViewerToggleAction
//
// Purpose: 
//   Destructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:12:11 PDT 2004
//   Added code to delete pixmaps.
//
// ****************************************************************************

ViewerToggleAction::~ViewerToggleAction()
{
    delete toggledIcon;
    delete regularIcon;
}

// ****************************************************************************
// Method: ViewerToggleAction::SetIcons
//
// Purpose: 
//   Sets the on/off toggle icons.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:12:25 PDT 2004
//   Added code to copy icons.
//
// ****************************************************************************

void
ViewerToggleAction::SetIcons(const QPixmap &p1, const QPixmap &p2)
{
    if(!window->GetNoWinMode())
    {
        toggledIcon = new QPixmap(p1);
        regularIcon = new QPixmap(p2);
        SetIconSet(QIconSet(*regularIcon));
    }
}

// ****************************************************************************
// Method: ViewerToggleAction::PreExecute
//
// Purpose: 
//   
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToggleAction::PreExecute()
{
    toggled = Toggled();
}

// ****************************************************************************
// Method: ViewerToggleAction::Update
//
// Purpose: 
//   Updates the appearance of the action if it has a visual representation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003
//   Check for valid icons before attempting to set them.
// 
// ****************************************************************************

void
ViewerToggleAction::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isToggleAction())
    {
        bool actionShouldBeToggled = Toggled();
        if(toggled != actionShouldBeToggled)
        {
            // Set the appropriate icon into the action.
            if (!window->GetNoWinMode() &&
                !action->iconSet().pixmap().isNull())
            {
                if(actionShouldBeToggled)
                    SetIconSet(QIconSet(*toggledIcon));
                else
                    SetIconSet(QIconSet(*regularIcon));
            }
            action->blockSignals(true);
            action->setOn(actionShouldBeToggled);
            action->blockSignals(false);
        }
        toggled = actionShouldBeToggled;
    }
}
