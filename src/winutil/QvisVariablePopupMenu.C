// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Fri May  9 10:30:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisVariablePopupMenu::QvisVariablePopupMenu(int plotType_, QWidget *parent) :
    QMenu(parent), varPath("")
{
    plotType = plotType_;
    myHashVal = 0;

    actions = new QActionGroup(this);
    connect(actions, SIGNAL(triggered(QAction*)),
            this, SLOT(caughtTriggered(QAction*)));
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
//   Mark C. Miller, Tue May  8 18:37:52 PDT 2018
//   Add defensive logic to clear the object's actions as well as the
//   member actions before deleting. This does indeed fix leaks.
// ****************************************************************************

QvisVariablePopupMenu::~QvisVariablePopupMenu()
{
    this->clear();
    actions->actions().clear();
    delete actions;
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::addVar
//
// Purpose: 
//   Adds a variable entry to the menu and does bookkeeping to ensure that the
//   signals will still be emitted by this class properly.
//
// Arguments:
//   var   : The variable name.
//   valid : Whether the variable is valid.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  9 11:10:38 PDT 2008
//
// Modifications:
//   
//   Mark C. Miller, Tue May  8 18:38:37 PDT 2018
//   Defensively ensure calling this objects addAction and not some other
//   method.
// ****************************************************************************

QAction *
QvisVariablePopupMenu::addVar(const QString &var, bool valid)
{
    QAction *a = this->addAction(var);
    a->setEnabled(valid);
    actions->addAction(a);
    return a;
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::count
//
// Purpose: 
//   Returns the number of actions in the menu.
//
// Returns:    The number of actions in the menu.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  9 12:25:06 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Apr 29 09:43:43 PDT 2010
//   I added John Schreiner's fix.
//
// ****************************************************************************

int
QvisVariablePopupMenu::count() const
{
    return actions->actions().count() + QMenu::actions().count();
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisVariablePopupMenu::caughtTriggered
//
// Purpose: 
//   This is a Qt slot function that catches QMenu's activated slot and
//   re-emits it with the plotType and variable name.
//
// Arguments:
//   action : The action for the variable that caused the signal.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  9 11:09:22 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariablePopupMenu::caughtTriggered(QAction *action)
{
    emit activated(plotType, varPath + action->text());
}
