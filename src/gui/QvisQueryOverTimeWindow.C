#include "QvisQueryOverTimeWindow.h"

#include <QueryOverTimeAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
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
//   
// ****************************************************************************

QvisQueryOverTimeWindow::QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
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
// ****************************************************************************

void
QvisQueryOverTimeWindow::CreateWindowContents()
{
    //
    // TimeType
    //
    QGroupBox *timeTypeBox = new QGroupBox(central, "timeTypeBox");
    timeTypeBox->setTitle("X-Axis:");
    topLayout->addWidget(timeTypeBox);
   
    QGridLayout *timeTypeBoxLayout = new QGridLayout(timeTypeBox, 3, 2);
    timeTypeBoxLayout->setMargin(10);
    timeTypeBoxLayout->setSpacing(5);
    timeTypeBoxLayout->addRowSpacing(0, 10); 

    timeType = new QButtonGroup();
    connect(timeType, SIGNAL(clicked(int)), this, SLOT(timeTypeChanged(int)));

    QHBoxLayout *timeTypeLayout = new QHBoxLayout();
    //timeTypeLayout->setSpacing(10);


    QRadioButton *cycle    = new QRadioButton("Cycle",    timeTypeBox);
    QRadioButton *dtime    = new QRadioButton("Time",     timeTypeBox);
    QRadioButton *timestep = new QRadioButton("Timestep", timeTypeBox);
    timeType->insert(cycle);
    timeType->insert(dtime);
    timeType->insert(timestep);

    timeTypeLayout->addWidget(cycle);
    timeTypeLayout->addWidget(dtime);
    timeTypeLayout->addWidget(timestep);

    timeTypeBoxLayout->addLayout(timeTypeLayout, 1, 0);
    //mainLayout->addMultiCellWidget(timeType, 0,0, 1,2);

    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,3,  10, "mainLayout");

    //
    // StartTime 
    //
    startTimeFlag = new QCheckBox("Starting timestep", central, "startTimeFlag");
    connect(startTimeFlag, SIGNAL(toggled(bool)),
            this, SLOT(startTimeFlagChanged(bool)));
    mainLayout->addWidget(startTimeFlag, 1,0);

    startTime = new QNarrowLineEdit(central, "startTime");
    connect(startTime, SIGNAL(returnPressed()),
            this, SLOT(startTimeProcessText()));
    mainLayout->addWidget(startTime, 1,1);

    //
    // EndTime 
    //
    endTimeFlag = new QCheckBox("Ending timestep", central, "endTimeFlag");
    connect(endTimeFlag, SIGNAL(toggled(bool)),
            this, SLOT(endTimeFlagChanged(bool)));
    mainLayout->addWidget(endTimeFlag, 2,0);

    endTime = new QNarrowLineEdit(central, "endTime");
    connect(endTime, SIGNAL(returnPressed()),
            this, SLOT(endTimeProcessText()));
    mainLayout->addWidget(endTime, 2,1);

    //
    // Stride 
    //
    strideLabel = new QLabel("stride", central, "strideLabel");
    mainLayout->addWidget(strideLabel,3,0);
    stride = new QNarrowLineEdit(central, "stride");
    connect(stride, SIGNAL(returnPressed()),
            this, SLOT(strideProcessText()));
    mainLayout->addWidget(stride, 3,1);

    //
    // CreateWindow 
    //
    createWindow = new QCheckBox("Use 1st unused window or\ncreate new one.  All\nsubsequent queries will\nuse this same window.",
                                  central, "createWindow");
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addMultiCellWidget(createWindow, 4,4,0,1);

    //
    // WindowId 
    //
    windowIdLabel = new QLabel("Window #", central, "windowIdLabel");
    mainLayout->addWidget(windowIdLabel,5,0);
    windowId = new QNarrowLineEdit(central, "windowId");
    connect(windowId, SIGNAL(returnPressed()),
            this, SLOT(windowIdProcessText()));
    mainLayout->addWidget(windowId, 5,1);

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
          case 0: //timeType
            timeType->setButton(atts->GetTimeType());
            break;
          case 1: //startTimeFlag
            if (atts->GetStartTimeFlag() == true)
            {
                startTime->setEnabled(true);
            }
            else
            {
                startTime->setEnabled(false);
            }
            startTimeFlag->setChecked(atts->GetStartTimeFlag());
            break;
          case 2: //startTime
            temp.sprintf("%d", atts->GetStartTime());
            startTime->setText(temp);
            break;
          case 3: //endTimeFlag
            if (atts->GetEndTimeFlag() == true)
            {
                endTime->setEnabled(true);
            }
            else
            {
                endTime->setEnabled(false);
            }
            endTimeFlag->setChecked(atts->GetEndTimeFlag());
            break;
          case 4: //endTime
            temp.sprintf("%d", atts->GetEndTime());
            endTime->setText(temp);
            break;
          case 5: //stride
            temp.sprintf("%d", atts->GetStride());
            stride->setText(temp);
            break;
          case 6: //createWindow
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
          case 7: //windowId
            temp.sprintf("%d", atts->GetWindowId());
            windowId->setText(temp);
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
//   
// ****************************************************************************

void
QvisQueryOverTimeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do timeType
    if(which_widget == 0 || doAll)
    {
        // Nothing for timeType
    }

    // Do startTimeFlag
    if(which_widget == 1 || doAll)
    {
        // Nothing for startTimeFlag
    }

    // Do startTime
    if(which_widget == 2 || doAll)
    {
        temp = startTime->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetStartTime(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startTime was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetStartTime());
            Message(msg);
            atts->SetStartTime(atts->GetStartTime());
        }
    }

    // Do endTimeFlag
    if(which_widget == 3 || doAll)
    {
        // Nothing for endTimeFlag
    }

    // Do endTime
    if(which_widget == 4 || doAll)
    {
        temp = endTime->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetEndTime(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of endTime was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetEndTime());
            Message(msg);
            atts->SetEndTime(atts->GetEndTime());
        }
    }

    // Do stride
    if(which_widget == 5 || doAll)
    {
        temp = stride->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetStride(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of stride was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetStride());
            Message(msg);
            atts->SetStride(atts->GetStride());
        }
    }

    // Do createWindow
    if(which_widget == 6 || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == 7 || doAll)
    {
        temp = windowId->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetWindowId(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of windowId was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetWindowId());
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

        viewer->SetQueryOverTimeAttributes();
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
    viewer->SetDefaultQueryOverTimeAttributes();
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
    viewer->ResetQueryOverTimeAttributes();
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
QvisQueryOverTimeWindow::startTimeFlagChanged(bool val)
{
    atts->SetStartTimeFlag(val);
    Apply();
}


void
QvisQueryOverTimeWindow::startTimeProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisQueryOverTimeWindow::endTimeFlagChanged(bool val)
{
    atts->SetEndTimeFlag(val);
    Apply();
}


void
QvisQueryOverTimeWindow::endTimeProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisQueryOverTimeWindow::strideProcessText()
{
    GetCurrentValues(5);
    Apply();
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
    GetCurrentValues(7);
    Apply();
}


