#include "QvisPreferencesWindow.h"

#include <GlobalAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

// ****************************************************************************
// Method: QvisPreferencesWindow::QvisPreferencesWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisPreferencesWindow::QvisPreferencesWindow(
                       GlobalAttributes *subj,
                       const char *caption,
                       const char *shortName,
                       QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                       QvisPostableWindowObserver::ApplyButton, false)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisPreferencesWindow::~QvisPreferencesWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisPreferencesWindow::~QvisPreferencesWindow()
{
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:45:16 PST 2003
//   I added a toggle for posting windows when showing them.
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateWindowContents()
{
    cloneWindowOnFirstRefToggle =
        new QCheckBox("Clone window on first reference",
                      central, "cloneWindowOnFirstRefToggle");
    connect(cloneWindowOnFirstRefToggle, SIGNAL(toggled(bool)),
            this, SLOT(cloneWindowOnFirstRefToggled(bool)));
    topLayout->addWidget(cloneWindowOnFirstRefToggle);

    postWindowsWhenShownToggle =
        new QCheckBox("Post windows when shown",
                      central, "postWindowsWhenShownToggle");
    connect(postWindowsWhenShownToggle, SIGNAL(toggled(bool)),
            this, SLOT(postWindowsWhenShownToggled(bool)));
    topLayout->addWidget(postWindowsWhenShownToggle);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:46:50 PST 2003
//   I added a toggle button for posting windows when they are shown.
//
// ****************************************************************************

void
QvisPreferencesWindow::UpdateWindow(bool doAll)
{
    if (doAll || atts->IsSelected(18))
    {
        //
        // Clone window on first reference
        //
        cloneWindowOnFirstRefToggle->blockSignals(true);
        cloneWindowOnFirstRefToggle->setChecked(
            atts->GetCloneWindowOnFirstRef());
        cloneWindowOnFirstRefToggle->blockSignals(false);
    }

    if(doAll)
    {
        postWindowsWhenShownToggle->blockSignals(true);
        postWindowsWhenShownToggle->setChecked(postWhenShown);
        postWindowsWhenShownToggle->blockSignals(false);
    }
}


// ****************************************************************************
// Method: QvisPreferencesWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        atts->Notify();
    }
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisPreferencesWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::apply()
{
    Apply(true);
}


void
QvisPreferencesWindow::cloneWindowOnFirstRefToggled(bool val)
{
    atts->SetCloneWindowOnFirstRef(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::postWindowsWhenShownToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag that tells the GUI whether
//   or not windows should automatically post when they are shown.
//
// Arguments:
//   val : The new post value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 5 15:47:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::postWindowsWhenShownToggled(bool val)
{
    postWhenShown = val;
}
