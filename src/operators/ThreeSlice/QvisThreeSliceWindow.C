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

#include "QvisThreeSliceWindow.h"

#include <ThreeSliceAttributes.h>
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
// Method: QvisThreeSliceWindow::QvisThreeSliceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Jul 2 15:36:50 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisThreeSliceWindow::QvisThreeSliceWindow(const int type,
                         ThreeSliceAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::~QvisThreeSliceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Jul 2 15:36:50 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisThreeSliceWindow::~QvisThreeSliceWindow()
{
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Jul 2 15:36:50 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 16:01:55 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisThreeSliceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 4,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel(tr("X"), central, "xLabel"),0,0);
    x = new QLineEdit(central, "x");
    connect(x, SIGNAL(returnPressed()),
            this, SLOT(xProcessText()));
    mainLayout->addWidget(x, 0,1);

    mainLayout->addWidget(new QLabel(tr("Y"), central, "yLabel"),1,0);
    y = new QLineEdit(central, "y");
    connect(y, SIGNAL(returnPressed()),
            this, SLOT(yProcessText()));
    mainLayout->addWidget(y, 1,1);

    mainLayout->addWidget(new QLabel(tr("Z"), central, "zLabel"),2,0);
    z = new QLineEdit(central, "z");
    connect(z, SIGNAL(returnPressed()),
            this, SLOT(zProcessText()));
    mainLayout->addWidget(z, 2,1);

    interactive = new QCheckBox(tr("Interactive"), central, "interactive");
    connect(interactive, SIGNAL(toggled(bool)),
            this, SLOT(interactiveChanged(bool)));
    mainLayout->addWidget(interactive, 3,0);

}


// ****************************************************************************
// Method: QvisThreeSliceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Jul 2 15:36:50 PST 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisThreeSliceWindow::UpdateWindow(bool doAll)
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
          case 0: //x
            temp.setNum(atts->GetX());
            x->setText(temp);
            break;
          case 1: //y
            temp.setNum(atts->GetY());
            y->setText(temp);
            break;
          case 2: //z
            temp.setNum(atts->GetZ());
            z->setText(temp);
            break;
          case 3: //interactive
            interactive->setChecked(atts->GetInteractive());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Jul 2 15:36:50 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisThreeSliceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do x
    if(which_widget == 0 || doAll)
    {
        temp = x->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            if(okay)
                atts->SetX(val);
        }

        if(!okay)
        {
            msg = tr("The value of x was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetX());
            Message(msg);
            atts->SetX(atts->GetX());
        }
    }

    // Do y
    if(which_widget == 1 || doAll)
    {
        temp = y->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            if(okay)
               atts->SetY(val);
        }

        if(!okay)
        {
            msg = tr("The value of y was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetY());
            Message(msg);
            atts->SetY(atts->GetY());
        }
    }

    // Do z
    if(which_widget == 2 || doAll)
    {
        temp = z->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            if(okay)
                atts->SetZ(val);
        }

        if(!okay)
        {
            msg = tr("The value of z was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetZ());
            Message(msg);
            atts->SetZ(atts->GetZ());
        }
    }

    // Do interactive
    if(which_widget == 3 || doAll)
    {
        // Nothing for interactive
    }

}


//
// Qt Slot functions
//


void
QvisThreeSliceWindow::xProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisThreeSliceWindow::yProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisThreeSliceWindow::zProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisThreeSliceWindow::interactiveChanged(bool val)
{
    atts->SetInteractive(val);
    Apply();
}


