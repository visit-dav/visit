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

#include "QvisIsovolumeWindow.h"

#include <IsovolumeAttributes.h>
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
// Method: QvisIsovolumeWindow::QvisIsovolumeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisIsovolumeWindow::QvisIsovolumeWindow(const int type,
                         IsovolumeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::~QvisIsovolumeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisIsovolumeWindow::~QvisIsovolumeWindow()
{
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 10 09:22:21 PDT 2004
//   I changed it so it uses a variable button.
//
// ****************************************************************************

void
QvisIsovolumeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    lboundLabel = new QLabel("Lower bound", central, "lboundLabel");
    mainLayout->addWidget(lboundLabel,0,0);
    lbound = new QLineEdit(central, "lbound");
    connect(lbound, SIGNAL(returnPressed()),
            this, SLOT(lboundProcessText()));
    mainLayout->addWidget(lbound, 0,1);

    uboundLabel = new QLabel("Upper bound", central, "uboundLabel");
    mainLayout->addWidget(uboundLabel,1,0);
    ubound = new QLineEdit(central, "ubound");
    connect(ubound, SIGNAL(returnPressed()),
            this, SLOT(uboundProcessText()));
    mainLayout->addWidget(ubound, 1,1);

    variableLabel = new QLabel("variable", central, "variableLabel");
    mainLayout->addWidget(variableLabel,2,0);
    int varMask = QvisVariableButton::Scalars;
    variable = new QvisVariableButton(true, true, true, varMask,
        central, "Variable");
    connect(variable, SIGNAL(activated(const QString &)),
            this, SLOT(variableChanged(const QString &)));
    mainLayout->addWidget(variable, 2,1);
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//    Jeremy Meredith, Wed May  5 14:55:08 PDT 2004
//    Made it support "min" and "max" as legal values, respectively, for
//    the lower and upper bound fields.
//   
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisIsovolumeWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for (int i = 0; i < atts->NumAttributes(); ++i)
    {
        if (!doAll)
        {
            if (!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch (i)
        {
          case 0: //lbound
            if (atts->GetLbound() == -1e+37)
                temp = "min";
            else
                temp.setNum(atts->GetLbound());
            lbound->setText(temp);
            break;
          case 1: //ubound
            if (atts->GetUbound() == +1e+37)
                temp = "max";
            else
                temp.setNum(atts->GetUbound());
            ubound->setText(temp);
            break;
          case 2: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//    Jeremy Meredith, Wed May  5 14:55:08 PDT 2004
//    Made it support "min" and "max" as legal values, respectively, for
//    the lower and upper bound fields.
//
//    Brad Whitlock, Fri Dec 10 09:23:59 PDT 2004
//    I removed the code to get the variable name.
//
// ****************************************************************************

void
QvisIsovolumeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do lbound
    if(which_widget == 0 || doAll)
    {
        temp = lbound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("min"))
            atts->SetLbound(-1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetLbound(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of lbound was invalid. "
                            "Resetting to the last good value of %g.",
                            atts->GetLbound());
                Message(msg);
                atts->SetLbound(atts->GetLbound());
            }
        }
    }

    // Do ubound
    if(which_widget == 1 || doAll)
    {
        temp = ubound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("max"))
            atts->SetUbound(1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetUbound(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of ubound was invalid. "
                            "Resetting to the last good value of %g.",
                            atts->GetUbound());
                Message(msg);
                atts->SetUbound(atts->GetUbound());
            }
        }
    }
}


//
// Qt Slot functions
//


void
QvisIsovolumeWindow::lboundProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisIsovolumeWindow::uboundProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisIsovolumeWindow::variableChanged(const QString &var)
{
    atts->SetVariable(var.latin1());
    SetUpdate(false);
    Apply();
}


