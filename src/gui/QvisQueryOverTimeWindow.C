// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisQueryOverTimeWindow.h"

#include <QueryOverTimeAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QNarrowLineEdit.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisQueryOverTimeWindow::QvisQueryOverTimeWindow
//
// Purpose:
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:32:57 PDT 2008
//   QString for caption, shortName.
//
//   Jeremy Meredith, Fri Jan  2 17:20:03 EST 2009
//   The base class postable window observer now defaults
//   to having load/save buttons, but to be consistent with
//   most control windows, we don't want them here.
//
// ****************************************************************************

QvisQueryOverTimeWindow::QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                                 QvisPostableWindowObserver::AllExtraButtons)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::~QvisQueryOverTimeWindow
//
// Purpose:
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//
// ****************************************************************************

QvisQueryOverTimeWindow::~QvisQueryOverTimeWindow()
{
}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::CreateWindowContents
//
// Purpose:
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Feb 27 12:36:41 PST 2006
//   Added more text to createWindow label, to clarify intent.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//   Kathleen Bonnell, Mon Feb  7 13:07:25 PST 2011
//   Added comments that explicitly state start/end times are TimeStates.
//   And that the 'Cycles' 'Times' and 'Timestate' options only apply to
//   values displayed for x-axis.
//
//   Kathleen Biagas, Fri Aug 26 17:12:13 PDT 2011
//   Removed start/end times and stride.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::CreateWindowContents()
{
    //
    // TimeType
    //
    QGroupBox *timeTypeBox = new QGroupBox(central);
    timeTypeBox->setTitle(tr("X-axis"));
    topLayout->addWidget(timeTypeBox);

    QGridLayout *timeTypeBoxLayout = new QGridLayout(timeTypeBox);

    QLabel *mLabel = new QLabel(tr("Choices entered here only apply to values displayed in the x-axis of the time curve."), central);
    mLabel->setWordWrap(true);
    timeTypeBoxLayout->addWidget(mLabel, 0, 0, 1, 3);

    timeType = new QButtonGroup(timeTypeBox);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(timeType, SIGNAL(buttonClicked(int)), this, SLOT(timeTypeChanged(int)));
#else
    connect(timeType, SIGNAL(idClicked(int)), this, SLOT(timeTypeChanged(int)));
#endif

    QHBoxLayout *timeTypeLayout = new QHBoxLayout();

    QRadioButton *cycle    = new QRadioButton(tr("Cycle"),timeTypeBox);
    QRadioButton *dtime    = new QRadioButton(tr("Time"),timeTypeBox);
    QRadioButton *timestep = new QRadioButton(tr("Timestep"),timeTypeBox);
    timeType->addButton(cycle,0);
    timeType->addButton(dtime,1);
    timeType->addButton(timestep,2);

    timeTypeLayout->addWidget(cycle);
    timeTypeLayout->addWidget(dtime);
    timeTypeLayout->addWidget(timestep);

    timeTypeBoxLayout->addLayout(timeTypeLayout, 1, 0);

    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    //
    // CreateWindow
    //
    createWindow = new QCheckBox(tr("Use 1st unused window or create new\none. All subsequent queries will use this\nsame window."),
                                  central);
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addWidget(createWindow, 0,0,3,2);

    //
    // WindowId
    //
    windowIdLabel = new QLabel(tr("Window #"), central);
    mainLayout->addWidget(windowIdLabel,3,0);
    windowId = new QNarrowLineEdit(central);
    connect(windowId, SIGNAL(returnPressed()),
            this, SLOT(windowIdProcessText()));
    mainLayout->addWidget(windowId, 3,1);

}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::UpdateWindow
//
// Purpose:
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Dec 17 09:40:53 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Biagas, Fri Aug 26 17:12:13 PDT 2011
//   Removed start/end times and stride.
//
//   Kathleen Biagas, Thu Jan 21, 2021
//   Replace QString.asprintf with QString.setNum.
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case QueryOverTimeAttributes::ID_timeType:
            timeType->button(atts->GetTimeType())->setChecked(true);
            break;
          case QueryOverTimeAttributes::ID_createWindow:
            if (atts->GetCreateWindow() == false)
            {
                windowId->setEnabled(true);
                windowIdLabel->setEnabled(true);
            }
            else
            {
                windowId->setEnabled(false);
                windowIdLabel->setEnabled(false);
            }
            createWindow->setChecked(atts->GetCreateWindow());
            break;
          case QueryOverTimeAttributes::ID_windowId:
            temp.setNum(atts->GetWindowId());
            windowId->setText(temp);
            break;
          default:
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::GetCurrentValues
//
// Purpose:
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Kathleen Biagas, Fri Aug 26 17:12:13 PDT 2011
//   Removed start/end times and stride.
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do timeType
    if(which_widget == QueryOverTimeAttributes::ID_timeType || doAll)
    {
        // Nothing for timeType
    }

    // Do createWindow
    if(which_widget == QueryOverTimeAttributes::ID_createWindow || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == QueryOverTimeAttributes::ID_windowId || doAll)
    {
        temp = windowId->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetWindowId(val);
        }

        if(!okay)
        {
            msg = tr("The value of windowId was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetWindowId());
            Message(msg);
            atts->SetWindowId(atts->GetWindowId());
        }
    }

}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::Apply
//
// Purpose:
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetQueryOverTimeAttributes();
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::apply
//
// Purpose:
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::makeDefault
//
// Purpose:
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultQueryOverTimeAttributes();
}


// ****************************************************************************
// Method: QvisQueryOverTimeWindow::reset
//
// Purpose:
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryOverTimeWindow::reset()
{
    GetViewerMethods()->ResetQueryOverTimeAttributes();
}


void
QvisQueryOverTimeWindow::timeTypeChanged(int val)
{
    if(val != atts->GetTimeType())
    {
        atts->SetTimeType(QueryOverTimeAttributes::TimeType(val));
        Apply();
    }
}


void
QvisQueryOverTimeWindow::createWindowChanged(bool val)
{
    atts->SetCreateWindow(val);
    Apply();
}


void
QvisQueryOverTimeWindow::windowIdProcessText()
{
    GetCurrentValues(QueryOverTimeAttributes::ID_windowId);
    Apply();
}


