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

#include "QvisCylinderWindow.h"

#include <CylinderAttributes.h>
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
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisCylinderWindow::QvisCylinderWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisCylinderWindow::QvisCylinderWindow(const int type,
                         CylinderAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisCylinderWindow::~QvisCylinderWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisCylinderWindow::~QvisCylinderWindow()
{
}


// ****************************************************************************
// Method: QvisCylinderWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 25 09:18:48 PDT 2008
//   Added tr().
//
// ****************************************************************************

void
QvisCylinderWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    point1Label = new QLabel(tr("Endpoint 1"), central, "point1Label");
    mainLayout->addWidget(point1Label,0,0);
    point1 = new QLineEdit(central, "point1");
    connect(point1, SIGNAL(returnPressed()),
            this, SLOT(point1ProcessText()));
    mainLayout->addWidget(point1, 0,1);

    point2Label = new QLabel(tr("Endpoint 2"), central, "point2Label");
    mainLayout->addWidget(point2Label,1,0);
    point2 = new QLineEdit(central, "point2");
    connect(point2, SIGNAL(returnPressed()),
            this, SLOT(point2ProcessText()));
    mainLayout->addWidget(point2, 1,1);

    radiusLabel = new QLabel(tr("Radius"), central, "radiusLabel");
    mainLayout->addWidget(radiusLabel,2,0);
    radius = new QLineEdit(central, "radius");
    connect(radius, SIGNAL(returnPressed()),
            this, SLOT(radiusProcessText()));
    mainLayout->addWidget(radius, 2,1);

}


// ****************************************************************************
// Method: QvisCylinderWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisCylinderWindow::UpdateWindow(bool doAll)
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
          case 0: //point1
            dptr = atts->GetPoint1();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point1->setText(temp);
            break;
          case 1: //point2
            dptr = atts->GetPoint2();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point2->setText(temp);
            break;
          case 2: //radius
            temp.setNum(atts->GetRadius());
            radius->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisCylinderWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisCylinderWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do point1
    if(which_widget == 0 || doAll)
    {
        temp = point1->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2])==3;
            if(okay)
                atts->SetPoint1(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint1();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of point1 was invalid. "
                     "Resetting to the last good value of %1.").arg(num);
            Message(msg);
            atts->SetPoint1(atts->GetPoint1());
        }
    }

    // Do point2
    if(which_widget == 1 || doAll)
    {
        temp = point2->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2])==3;
            if(okay)
                atts->SetPoint2(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint2();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of point2 was invalid. "
                     "Resetting to the last good value of %1.").arg(num);
            Message(msg);
            atts->SetPoint2(atts->GetPoint2());
        }
    }

    // Do radius
    if(which_widget == 2 || doAll)
    {
        temp = radius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetRadius(val);
        }

        if(!okay)
        {
            msg = tr("The value of radius was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetRadius());
            Message(msg);
            atts->SetRadius(atts->GetRadius());
        }
    }

}


//
// Qt Slot functions
//


void
QvisCylinderWindow::point1ProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisCylinderWindow::point2ProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisCylinderWindow::radiusProcessText()
{
    GetCurrentValues(2);
    Apply();
}


