#include "QvisPreferencesWindow.h"

#include <GlobalAttributes.h>
#include <ViewerProxy.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

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
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added tsFormat and made the window stretch.
//   
// ****************************************************************************

QvisPreferencesWindow::QvisPreferencesWindow(
                       GlobalAttributes *subj,
                       const char *caption,
                       const char *shortName,
                       QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                       QvisPostableWindowObserver::ApplyButton, true),
    tsFormat()
{
    atts = subj;
    timeStateDisplayMode = 0;
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
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added timeStateDisplayMode.
//
// ****************************************************************************

QvisPreferencesWindow::~QvisPreferencesWindow()
{
    delete timeStateDisplayMode;
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
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added radio buttons for changing the timestate display mode.
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

    //
    // Create group box for time controls.
    //
    QGroupBox *timeControlsGroup = new QGroupBox(central, "timeControlsGroup");
    timeControlsGroup->setTitle("Time formatting");
    topLayout->addWidget(timeControlsGroup, 5);
    QVBoxLayout *innerTopLayout = new QVBoxLayout(timeControlsGroup);
    innerTopLayout->setMargin(10);
    innerTopLayout->addSpacing(15);
    innerTopLayout->setSpacing(10);

    //
    // Create radio button controls to let us change the timestate display mode.
    //
    QGridLayout *tsModeLayout = new QGridLayout(innerTopLayout, 3, 3);
    tsModeLayout->setSpacing(5);
    tsModeLayout->addMultiCellWidget(new QLabel("Display time using:",
        timeControlsGroup), 0, 0, 0, 2);
    timeStateDisplayMode = new QButtonGroup(0, "timeStateDisplayMode");
    QRadioButton *rb = new QRadioButton("Cycles", timeControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton("Times", timeControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton("Cycles and times", timeControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 1, 2);
    timeStateDisplayMode->setButton(int(tsFormat.GetDisplayMode()));
    connect(timeStateDisplayMode, SIGNAL(clicked(int)),
            this, SLOT(handleTimeStateDisplayModeChange(int)));

    //
    // Create widgets that let you set the time format.
    //
    tsModeLayout->addMultiCellWidget(
        new QLabel("Number of significant digits", timeControlsGroup),
        2, 2, 0, 1);
    timeStateNDigits = new QSpinBox(1, 16, 1, timeControlsGroup, "timeStateNDigits");
    timeStateNDigits->setValue(tsFormat.GetPrecision());
    connect(timeStateNDigits, SIGNAL(valueChanged(int)),
            this, SLOT(timeStateNDigitsChanged(int)));
    tsModeLayout->addWidget(timeStateNDigits, 2, 2);

    topLayout->addStretch(100);
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
// Method: QvisPreferencesWindow::SetTimeStateDisplayMode
//
// Purpose: 
//   Sets the timeDisplayMode toggle.
//
// Arguments:
//   mode : The new display mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:05:17 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::SetTimeStateFormat(const TimeFormat &fmt)
{
    tsFormat = fmt;
    if(timeStateDisplayMode != 0 && timeStateNDigits != 0)
    {
        timeStateDisplayMode->blockSignals(true);
        timeStateDisplayMode->setButton(int(tsFormat.GetDisplayMode()));
        timeStateDisplayMode->blockSignals(false);

        timeStateNDigits->blockSignals(true);
        timeStateNDigits->setValue(tsFormat.GetPrecision());
        timeStateNDigits->blockSignals(false);
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

// ****************************************************************************
// Method: QvisPreferencesWindow::handleTimeStateDisplayModeChange
//
// Purpose: 
//   This is a Qt slot function that is called when the display mode radio
//   buttons are clicked.
//
// Arguments:
//   val : The new timestate display mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 09:57:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::handleTimeStateDisplayModeChange(int val)
{
    tsFormat.SetDisplayMode((TimeFormat::DisplayMode)val);
    emit changeTimeFormat(tsFormat);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::timeStateNDigitsChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the time format spin box
//   is changed.
//
// Arguments:
//   val : The new number of digits in the time format.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 13:34:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::timeStateNDigitsChanged(int val)
{
    tsFormat.SetPrecision(val);
    emit changeTimeFormat(tsFormat);
}
