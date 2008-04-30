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

#include "QvisTraceHistoryWindow.h"

#include <TraceHistoryAttributes.h>
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
// Method: QvisTraceHistoryWindow::QvisTraceHistoryWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Sun Apr 8 17:04:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisTraceHistoryWindow::QvisTraceHistoryWindow(const int type,
                         TraceHistoryAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisTraceHistoryWindow::~QvisTraceHistoryWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Sun Apr 8 17:04:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisTraceHistoryWindow::~QvisTraceHistoryWindow()
{
}


// ****************************************************************************
// Method: QvisTraceHistoryWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Sun Apr 8 17:04:23 PST 2007
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 15:56:45 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisTraceHistoryWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 4,2,  10, "mainLayout");

    varsButton = new QvisVariableButton(true, false, true, -1,
        central, "varsButton");
    varsButton->setText(tr("Variables"));
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    mainLayout->addWidget(varsButton, 0, 0);

    varsLineEdit = new QLineEdit(central, "varsLineEdit");
    varsLineEdit->setText("");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(varsLineEdit, 0, 1);

    displacementLabel = new QLabel(tr("Displacement variable"), central, "displacementLabel");
    mainLayout->addWidget(displacementLabel,1,0);
    int displacementMask = QvisVariableButton::Vectors;
    displacement = new QvisVariableButton(true, true, true, displacementMask, central, "displacement");
    connect(displacement, SIGNAL(activated(const QString&)),
            this, SLOT(displacementChanged(const QString&)));
    mainLayout->addWidget(displacement, 1,1);

    numiterLabel = new QLabel(tr("Number of iterations"), central, "numiterLabel");
    mainLayout->addWidget(numiterLabel,2,0);
    numiter = new QLineEdit(central, "numiter");
    connect(numiter, SIGNAL(returnPressed()),
            this, SLOT(numiterProcessText()));
    mainLayout->addWidget(numiter, 2,1);

    outputLabel = new QLabel(tr("Output file"), central, "outputLabel");
    mainLayout->addWidget(outputLabel,3,0);
    output = new QLineEdit(central, "output");
    connect(output, SIGNAL(returnPressed()),
            this, SLOT(outputProcessText()));
    mainLayout->addWidget(output, 3,1);

}


// ****************************************************************************
// Method: QvisTraceHistoryWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sun Apr 8 17:04:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisTraceHistoryWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;
    char str[1024];

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
          case 0: //vars
            {
               str[0] = '\0';
               for (int j = 0 ; j < atts->GetVars().size() ; j++)
                   sprintf(str + strlen(str), "%s%s", (j == 0 ? "" : " "),
                           atts->GetVars()[j].c_str());
               temp = str;
               varsLineEdit->setText(temp);
            }

            break;
          case 1: //displacement
            displacement->blockSignals(true);
            displacement->setText(atts->GetDisplacement().c_str());
            displacement->blockSignals(false);
            break;
          case 2: //numiter
            numiter->blockSignals(true);
            temp.sprintf("%d", atts->GetNumiter());
            numiter->setText(temp);
            numiter->blockSignals(false);
            break;
          case 3: //output
            temp = atts->GetOutput().c_str();
            output->blockSignals(true);
            output->setText(temp);
            output->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTraceHistoryWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Sun Apr 8 17:04:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisTraceHistoryWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do exprs
    if(which_widget == 0 || doAll)
    {
        stringVector userVars;
        temp = varsLineEdit->displayText().simplifyWhiteSpace();
        QStringList lst(QStringList::split(" ", temp));

        QStringList::Iterator it;

        for (it = lst.begin(); it != lst.end(); ++it)
        {
            userVars.push_back((*it).latin1());
        }

        atts->SetVars(userVars);
    }

    // Do displacement
    if(which_widget == 1 || doAll)
    {
        // Nothing for displacement
    }

    // Do numiter
    if(which_widget == 2 || doAll)
    {
        temp = numiter->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetNumiter(val);
        }

        if(!okay)
        {
            msg = tr("The value of numiter was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetNumiter());
            Message(msg);
            atts->SetNumiter(atts->GetNumiter());
        }
    }

    // Do output
    if(which_widget == 3 || doAll)
    {
        temp = output->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetOutput(temp.latin1());
        }

        if(!okay)
        {
            msg = tr("The value of output was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetOutput().c_str());
            Message(msg);
            atts->SetOutput(atts->GetOutput());
        }
    }

}


//
// Qt Slot functions
//


//writeSourceCallback unknown for stringVector (variable exprs)


void
QvisTraceHistoryWindow::displacementChanged(const QString &varName)
{
    atts->SetDisplacement(varName.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisTraceHistoryWindow::numiterProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisTraceHistoryWindow::outputProcessText()
{
    GetCurrentValues(3);
    Apply();
}

//
// Qt Slot functions
//
void
QvisTraceHistoryWindow::variableProcessText()
{
    GetCurrentValues(0);
    Apply();
}



// ****************************************************************************
// Method: QvisTraceHistoryWindow::addVariable
//
// Purpose:
//   This is a Qt slot function that is called when the user selects a new
//   pick variable.
//
// Arguments:
//   var : The pick variable to add.
//
// Programmer: Hank Childs
// Creation:   April 8, 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisTraceHistoryWindow::addVariable(const QString &var)
{
    // Add the new variable to the pick variable line edit.
    QString varString(varsLineEdit->displayText());
    if(varString.length() > 0)
        varString += " ";
    varString += var;
    varsLineEdit->setText(varString);

    // Process the list of pick vars.
    variableProcessText();
}



