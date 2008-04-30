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

#include "QvisDeferExpressionWindow.h"

#include <DeferExpressionAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qstringlist.h>
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
// Method: QvisDeferExpressionWindow::QvisDeferExpressionWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisDeferExpressionWindow::QvisDeferExpressionWindow(const int type,
                         DeferExpressionAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::~QvisDeferExpressionWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisDeferExpressionWindow::~QvisDeferExpressionWindow()
{
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
//   Hank Childs, Wed Sep 21 11:34:19 PDT 2005
//   Use QvisVariableButton to create variable list.
//
//   Brad Whitlock, Fri Apr 25 09:17:09 PDT 2008
//   Added tr().
//
// ****************************************************************************

void
QvisDeferExpressionWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");

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


}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisDeferExpressionWindow::UpdateWindow(bool doAll)
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

        switch(i)
        {
          case 0: //exprs
            {
               str[0] = '\0';
               for (int j = 0 ; j < atts->GetExprs().size() ; j++)
                   sprintf(str + strlen(str), "%s%s", (j == 0 ? "" : " "),
                           atts->GetExprs()[j].c_str());
               temp = str;
               varsLineEdit->setText(temp);
            }
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisDeferExpressionWindow::GetCurrentValues(int which_widget)
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

        atts->SetExprs(userVars);
    }
}


//
// Qt Slot functions
//
void
QvisDeferExpressionWindow::variableProcessText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::addVariable
//
// Purpose:
//   This is a Qt slot function that is called when the user selects a new
//   pick variable.
//
// Arguments:
//   var : The pick variable to add.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 10 09:57:14 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisDeferExpressionWindow::addVariable(const QString &var)
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


