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

#include "QvisDisplaceWindow.h"

#include <DisplaceAttributes.h>
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
// Method: QvisDisplaceWindow::QvisDisplaceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisDisplaceWindow::QvisDisplaceWindow(const int type,
                         DisplaceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisDisplaceWindow::~QvisDisplaceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisDisplaceWindow::~QvisDisplaceWindow()
{
}


// ****************************************************************************
// Method: QvisDisplaceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   
//   Hank Childs, Tue Sep  5 15:51:59 PDT 2006
//   Remove vestiges of "DISPL".
//
// ****************************************************************************

void
QvisDisplaceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Displacement multiplier ", central, "factorLabel"),0,0);

    factor = new QLineEdit(central, "factor");
    connect(factor, SIGNAL(returnPressed()),
            this, SLOT(factorProcessText()));
    mainLayout->addWidget(factor, 0,1);

    mainLayout->addWidget(new QLabel("Displacement variable", central, "variableLabel"),1,0);
    variable = new QvisVariableButton(true, true, true,
        QvisVariableButton::Vectors, central, "variable");
    variable->setDefaultVariable("default");
    connect(variable, SIGNAL(activated(const QString&)),
            this, SLOT(variableChanged(const QString&)));
    mainLayout->addWidget(variable, 1,1);

}


// ****************************************************************************
// Method: QvisDisplaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDisplaceWindow::UpdateWindow(bool doAll)
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
          case 0: //factor
            temp.setNum(atts->GetFactor());
            factor->setText(temp);
            break;
          case 1: //variable
            variable->setText(atts->GetVariable().c_str());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisDisplaceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Dec 9 15:24:08 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 15:35:45 PST 2004
//   Removed the code for the variable name since it's a different widget now.
//
// ****************************************************************************

void
QvisDisplaceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do factor
    if(which_widget == 0 || doAll)
    {
        temp = factor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of factor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetFactor());
            Message(msg);
            atts->SetFactor(atts->GetFactor());
        }
    }

    // Do variable
    if(which_widget == 1 || doAll)
    {
        // Nothing for variable
    }

}


//
// Qt Slot functions
//


void
QvisDisplaceWindow::factorProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisDisplaceWindow::variableChanged(const QString &varName)
{
    atts->SetVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


