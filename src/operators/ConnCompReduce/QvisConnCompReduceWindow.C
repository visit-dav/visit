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

#include "QvisConnCompReduceWindow.h"

#include <ConnCompReduceAttributes.h>
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
// Method: QvisConnCompReduceWindow::QvisConnCompReduceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConnCompReduceWindow::QvisConnCompReduceWindow(const int type,
                         ConnCompReduceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::~QvisConnCompReduceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConnCompReduceWindow::~QvisConnCompReduceWindow()
{
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConnCompReduceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Target Reduction ", central, "targetLabel"),0,0);
    target = new QLineEdit(central, "target");
    connect(target, SIGNAL(returnPressed()),
            this, SLOT(targetProcessText()));
    mainLayout->addWidget(target, 0,1);

}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisConnCompReduceWindow::UpdateWindow(bool doAll)
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
          case 0: //target
            temp.setNum(atts->GetTarget());
            target->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConnCompReduceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do target
    if(which_widget == 0 || doAll)
    {
        temp = target->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTarget(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of target was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTarget());
            Message(msg);
            atts->SetTarget(atts->GetTarget());
        }
    }

}


//
// Qt Slot functions
//


void
QvisConnCompReduceWindow::targetProcessText()
{
    GetCurrentValues(0);
    Apply();
}


