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
//   
// ****************************************************************************

ViewerToggleAction::ViewerToggleAction(ViewerWindow *win, const char *name) : 
    ViewerAction(win, name), regularIcon(), toggledIcon()
{
    toggled = false;
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
//   
// ****************************************************************************

ViewerToggleAction::~ViewerToggleAction()
{
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
//   
// ****************************************************************************

void
ViewerToggleAction::SetIcons(const QPixmap &p1, const QPixmap &p2)
{
    toggledIcon = p1;
    regularIcon = p2;
    SetIconSet(QIconSet(regularIcon));
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
            if (!action->iconSet().pixmap().isNull())
            {
                if(actionShouldBeToggled)
                    SetIconSet(QIconSet(toggledIcon));
                else
                    SetIconSet(QIconSet(regularIcon));
            }
            action->blockSignals(true);
            action->setOn(actionShouldBeToggled);
            action->blockSignals(false);
        }
        toggled = actionShouldBeToggled;
    }
}
