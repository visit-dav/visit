/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//   Brad Whitlock, Wed Apr  9 11:32:57 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisQueryOverTimeWindow::QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
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
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisQueryOverTimeWindow::CreateWindowContents()
{
    //
    // TimeType
    //
    QGroupBox *timeTypeBox = new QGroupBox(central, "timeTypeBox");
    timeTypeBox->setTitle(tr("X-Axis:"));
    topLayout->addWidget(timeTypeBox);
   
    QGridLayout *timeTypeBoxLayout = new QGridLayout(timeTypeBox, 3, 2);
    timeTypeBoxLayout->setMargin(10);
    timeTypeBoxLayout->setSpacing(5);
    timeTypeBoxLayout->addRowSpacing(0, 10); 

    timeType = new QButtonGroup();
    connect(timeType, SIGNAL(clicked(int)), this, SLOT(timeTypeChanged(int)));

    QHBoxLayout *timeTypeLayout = new QHBoxLayout();
    //timeTypeLayout->setSpacing(10);


    QRadioButton *cycle    = new QRadioButton(tr("Cycle"),    timeTypeBox);
    QRadioButton *dtime    = new QRadioButton(tr("Time"),     timeTypeBox);
    QRadioButton *timestep = new QRadioButton(tr("Timestep"), timeTypeBox);
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
    startTimeFlag = new QCheckBox(tr("Starting timestep"), central, "startTimeFlag");
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
    endTimeFlag = new QCheckBox(tr("Ending timestep"), central, "endTimeFlag");
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
    strideLabel = new QLabel(tr("stride"), central, "strideLabel");
    mainLayout->addWidget(strideLabel,3,0);
    stride = new QNarrowLineEdit(central, "stride");
    connect(stride, SIGNAL(returnPressed()),
            this, SLOT(strideProcessText()));
    mainLayout->addWidget(stride, 3,1);

    //
    // CreateWindow 
    //
    createWindow = new QCheckBox(tr("Use 1st unused window or\ncreate new one.  All\nsubsequent queries will\nuse this same window."),
                                  central, "createWindow");
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addMultiCellWidget(createWindow, 4,4,0,1);

    //
    // WindowId 
    //
    windowIdLabel = new QLabel(tr("Window #"), central, "windowIdLabel");
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
//   Brad Whitlock, Mon Dec 17 09:40:53 PST 2007
//   Made it use ids.
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
            timeType->setButton(atts->GetTimeType());
            break;
          case QueryOverTimeAttributes::ID_startTimeFlag:
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
          case QueryOverTimeAttributes::ID_startTime:
            temp.sprintf("%d", atts->GetStartTime());
            startTime->setText(temp);
            break;
          case QueryOverTimeAttributes::ID_endTimeFlag:
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
          case QueryOverTimeAttributes::ID_endTime:
            temp.sprintf("%d", atts->GetEndTime());
            endTime->setText(temp);
            break;
          case QueryOverTimeAttributes::ID_stride:
            temp.sprintf("%d", atts->GetStride());
            stride->setText(temp);
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
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
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

    // Do startTimeFlag
    if(which_widget == QueryOverTimeAttributes::ID_startTimeFlag || doAll)
    {
        // Nothing for startTimeFlag
    }

    // Do startTime
    if(which_widget == QueryOverTimeAttributes::ID_startTime || doAll)
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
            QString num; num.sprintf("%d", atts->GetStartTime());
            msg = tr("The value of startTime was invalid. "
                     "Resetting to the last good value of %1.");
            msg.replace("%1", num);
            Message(msg);
            atts->SetStartTime(atts->GetStartTime());
        }
    }

    // Do endTimeFlag
    if(which_widget == QueryOverTimeAttributes::ID_endTimeFlag || doAll)
    {
        // Nothing for endTimeFlag
    }

    // Do endTime
    if(which_widget == QueryOverTimeAttributes::ID_endTime || doAll)
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
            QString num; num.sprintf("%d", atts->GetEndTime());
            msg = tr("The value of endTime was invalid. "
                     "Resetting to the last good value of %1.");
            msg.replace("%1", num);
            Message(msg);
            atts->SetEndTime(atts->GetEndTime());
        }
    }

    // Do stride
    if(which_widget == QueryOverTimeAttributes::ID_stride || doAll)
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
            QString num; num.sprintf("%d", atts->GetStride());
            msg = tr("The value of stride was invalid. "
                     "Resetting to the last good value of %1.");
            msg.replace("%1", num);
            Message(msg);
            atts->SetStride(atts->GetStride());
        }
    }

    // Do createWindow
    if(which_widget == QueryOverTimeAttributes::ID_createWindow || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == QueryOverTimeAttributes::ID_windowId || doAll)
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
            QString num; num.sprintf("%d", atts->GetWindowId());
            msg = tr("The value of windowId was invalid. "
                     "Resetting to the last good value of %1.");
            msg.replace("%1", num);
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
QvisQueryOverTimeWindow::startTimeFlagChanged(bool val)
{
    atts->SetStartTimeFlag(val);
    Apply();
}


void
QvisQueryOverTimeWindow::startTimeProcessText()
{
    GetCurrentValues(QueryOverTimeAttributes::ID_startTime);
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
    GetCurrentValues(QueryOverTimeAttributes::ID_endTime);
    Apply();
}


void
QvisQueryOverTimeWindow::strideProcessText()
{
    GetCurrentValues(QueryOverTimeAttributes::ID_stride);
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
    GetCurrentValues(QueryOverTimeAttributes::ID_windowId);
    Apply();
}


