#include <QvisVariablePopupMenu.h>

// ****************************************************************************
// Method: QvisVariablePopupMenu::QvisVariablePopupMenu
//
// Purpose: 
//   Constructor for the QvisVariablePopupMenu class.
//
// Arguments:
//   plotType_ : The plotType that will be passed along in the extended 
//               activate signal.
//   parent    : The widget's parent widget.
//   name      : The name of the popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:29:16 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:17:57 PDT 2002
//   Added initialization of varPath member.
//
// ****************************************************************************

QvisVariablePopupMenu::QvisVariablePopupMenu(int plotType_, QWidget *parent,
    const char *name) : QPopupMenu(parent, name), varPath("")
{
    plotType = plotType_;

    // Connect a slot to QPopupMenu's activate signal so we can re-emit a
    // signal that has more information.
    connect(this, SIGNAL(activated(int)),
            this, SLOT(activatedCaught(int)));
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::~QvisVariablePopupMenu
//
// Purpose: 
//   Destructor for the QvisVariablePopupMenu class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:30:25 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisVariablePopupMenu::~QvisVariablePopupMenu()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::activatedCaught
//
// Purpose: 
//   This is a Qt slot function that catches QPopupMenu's activated slot and
//   re-emits it with the plotType and variable name.
//
// Arguments:
//   index : The index of the menu item that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:30:48 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:19:02 PDT 2002
//   Modified to support cascading variable menus.
//
// ****************************************************************************

void
QvisVariablePopupMenu::activatedCaught(int index)
{
    // Re-emit the signal with different arguments.
    QString completeVar(varPath + text(index));
    emit activated(plotType, completeVar);
}
