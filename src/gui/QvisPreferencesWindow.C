#include "QvisPreferencesWindow.h"

#include <GlobalAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <stdio.h>

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
//   
// ****************************************************************************

void
QvisPreferencesWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1, 2, 10);

    cloneWindowOnFirstRefToggle =
        new QCheckBox("Clone window on first reference",
                      central, "cloneWindowOnFirstRefToggle");
    connect(cloneWindowOnFirstRefToggle, SIGNAL(toggled(bool)),
            this, SLOT(cloneWindowOnFirstRefToggled(bool)));
    mainLayout->addWidget(cloneWindowOnFirstRefToggle, 0, 0);
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
