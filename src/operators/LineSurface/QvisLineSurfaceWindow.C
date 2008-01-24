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

#include "QvisLineSurfaceWindow.h"

#include <LineSurfaceAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisLineSurfaceWindow::QvisLineSurfaceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Jan 23 11:30:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisLineSurfaceWindow::QvisLineSurfaceWindow(const int type,
                         LineSurfaceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisLineSurfaceWindow::~QvisLineSurfaceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Jan 23 11:30:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisLineSurfaceWindow::~QvisLineSurfaceWindow()
{
}


// ****************************************************************************
// Method: QvisLineSurfaceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Jan 23 11:30:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineSurfaceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5,2,  10, "mainLayout");


    startTimeLabel = new QLabel("Index of first time slice", central, "startTimeLabel");
    mainLayout->addWidget(startTimeLabel,0,0);
    startTime = new QLineEdit(central, "startTime");
    connect(startTime, SIGNAL(returnPressed()),
            this, SLOT(startTimeProcessText()));
    mainLayout->addWidget(startTime, 0,1);

    endTimeLabel = new QLabel("Index of last time slice", central, "endTimeLabel");
    mainLayout->addWidget(endTimeLabel,1,0);
    endTime = new QLineEdit(central, "endTime");
    connect(endTime, SIGNAL(returnPressed()),
            this, SLOT(endTimeProcessText()));
    mainLayout->addWidget(endTime, 1,1);

    strideLabel = new QLabel("Stride through time slices", central, "strideLabel");
    mainLayout->addWidget(strideLabel,2,0);
    stride = new QLineEdit(central, "stride");
    connect(stride, SIGNAL(returnPressed()),
            this, SLOT(strideProcessText()));
    mainLayout->addWidget(stride, 2,1);

    point1Label = new QLabel("Point 1", central, "point1Label");
    mainLayout->addWidget(point1Label,3,0);
    point1 = new QLineEdit(central, "point1");
    connect(point1, SIGNAL(returnPressed()),
            this, SLOT(point1ProcessText()));
    mainLayout->addWidget(point1, 3,1);

    point2Label = new QLabel("Point 2", central, "point2Label");
    mainLayout->addWidget(point2Label,4,0);
    point2 = new QLineEdit(central, "point2");
    connect(point2, SIGNAL(returnPressed()),
            this, SLOT(point2ProcessText()));
    mainLayout->addWidget(point2, 4,1);

}


// ****************************************************************************
// Method: QvisLineSurfaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Jan 23 11:30:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineSurfaceWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case LineSurfaceAttributes::ID_startTime:
            startTime->blockSignals(true);
            temp.sprintf("%d", atts->GetStartTime());
            startTime->setText(temp);
            startTime->blockSignals(false);
            break;
          case LineSurfaceAttributes::ID_endTime:
            endTime->blockSignals(true);
            temp.sprintf("%d", atts->GetEndTime());
            endTime->setText(temp);
            endTime->blockSignals(false);
            break;
          case LineSurfaceAttributes::ID_stride:
            stride->blockSignals(true);
            temp.sprintf("%d", atts->GetStride());
            stride->setText(temp);
            stride->blockSignals(false);
            break;
          case LineSurfaceAttributes::ID_point1:
            dptr = atts->GetPoint1();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point1->blockSignals(true);
            point1->setText(temp);
            point1->blockSignals(false);
            break;
          case LineSurfaceAttributes::ID_point2:
            dptr = atts->GetPoint2();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point2->blockSignals(true);
            point2->setText(temp);
            point2->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisLineSurfaceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Jan 23 11:30:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineSurfaceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do startTime
    if(which_widget == LineSurfaceAttributes::ID_startTime || doAll)
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

    // Do endTime
    if(which_widget == LineSurfaceAttributes::ID_endTime || doAll)
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
    if(which_widget == LineSurfaceAttributes::ID_stride || doAll)
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

    // Do point1
    if(which_widget == LineSurfaceAttributes::ID_point1 || doAll)
    {
        temp = point1->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetPoint1(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint1();
            msg.sprintf("The value of point1 was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetPoint1(atts->GetPoint1());
        }
    }

    // Do point2
    if(which_widget == LineSurfaceAttributes::ID_point2 || doAll)
    {
        temp = point2->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetPoint2(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint2();
            msg.sprintf("The value of point2 was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetPoint2(atts->GetPoint2());
        }
    }

}


//
// Qt Slot functions
//


void
QvisLineSurfaceWindow::startTimeProcessText()
{
    GetCurrentValues(LineSurfaceAttributes::ID_startTime);
    Apply();
}


void
QvisLineSurfaceWindow::endTimeProcessText()
{
    GetCurrentValues(LineSurfaceAttributes::ID_endTime);
    Apply();
}


void
QvisLineSurfaceWindow::strideProcessText()
{
    GetCurrentValues(LineSurfaceAttributes::ID_stride);
    Apply();
}


void
QvisLineSurfaceWindow::point1ProcessText()
{
    GetCurrentValues(LineSurfaceAttributes::ID_point1);
    Apply();
}


void
QvisLineSurfaceWindow::point2ProcessText()
{
    GetCurrentValues(LineSurfaceAttributes::ID_point2);
    Apply();
}


