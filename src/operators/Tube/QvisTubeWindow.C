/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "QvisTubeWindow.h"

#include <TubeAttributes.h>
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
// Method: QvisTubeWindow::QvisTubeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTubeWindow::QvisTubeWindow(const int type,
                         TubeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisTubeWindow::~QvisTubeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTubeWindow::~QvisTubeWindow()
{
}


// ****************************************************************************
// Method: QvisTubeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisTubeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("width", central, "widthLabel"),0,0);
    width = new QLineEdit(central, "width");
    connect(width, SIGNAL(returnPressed()),
            this, SLOT(widthProcessText()));
    mainLayout->addWidget(width, 0,1);

    mainLayout->addWidget(new QLabel("Fineness of tube", central, "finenessLabel"),1,0);
    fineness = new QLineEdit(central, "fineness");
    connect(fineness, SIGNAL(returnPressed()),
            this, SLOT(finenessProcessText()));
    mainLayout->addWidget(fineness, 1,1);

    capping = new QCheckBox("Cap Tubes?", central, "capping");
    connect(capping, SIGNAL(toggled(bool)),
            this, SLOT(cappingChanged(bool)));
    mainLayout->addWidget(capping, 2,0);

}


// ****************************************************************************
// Method: QvisTubeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisTubeWindow::UpdateWindow(bool doAll)
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
          case 0: //width
            temp.setNum(atts->GetWidth());
            width->setText(temp);
            break;
          case 1: //fineness
            temp.sprintf("%d", atts->GetFineness());
            fineness->setText(temp);
            break;
          case 2: //capping
            capping->setChecked(atts->GetCapping());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTubeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisTubeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do width
    if(which_widget == 0 || doAll)
    {
        temp = width->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetWidth(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of width was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetWidth());
            Message(msg);
            atts->SetWidth(atts->GetWidth());
        }
    }

    // Do fineness
    if(which_widget == 1 || doAll)
    {
        temp = fineness->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if (val < 3 || val > 12)
            {
                msg.sprintf("The fineness of the tube is limited from 3 "
                    "(coarsest) to 12 (finest)");
                Message(msg);
                atts->SetFineness(atts->GetFineness());
            }
            else
            {
                atts->SetFineness(val);
            }
        }

        if(!okay)
        {
            msg.sprintf("The value of fineness was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetFineness());
            Message(msg);
            atts->SetFineness(atts->GetFineness());
        }
    }

    // Do capping
    if(which_widget == 2 || doAll)
    {
        // Nothing for capping
    }

}


//
// Qt Slot functions
//


void
QvisTubeWindow::widthProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisTubeWindow::finenessProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisTubeWindow::cappingChanged(bool val)
{
    atts->SetCapping(val);
    Apply();
}


