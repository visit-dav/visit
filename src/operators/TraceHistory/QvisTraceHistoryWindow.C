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

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QButtonGroup>
#include <QRadioButton>

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
//   Cyrus Harrison, Fri Dec  5 09:19:42 PST 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisTraceHistoryWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    varsButton = new QvisVariableButton(true, false, true, -1,central);
    varsButton->setText(tr("Variables"));
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    mainLayout->addWidget(varsButton, 0, 0);

    varsLineEdit = new QLineEdit(central);
    varsLineEdit->setText("");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(varsLineEdit, 0, 1);

    displacementLabel = new QLabel(tr("Displacement variable"), central);
    mainLayout->addWidget(displacementLabel,1,0);
    int displacementMask = QvisVariableButton::Vectors;
    displacement = new QvisVariableButton(true, true, true, displacementMask, central);
    connect(displacement, SIGNAL(activated(const QString&)),
            this, SLOT(displacementChanged(const QString&)));
    mainLayout->addWidget(displacement, 1,1);

    numiterLabel = new QLabel(tr("Number of iterations"), central);
    mainLayout->addWidget(numiterLabel,2,0);
    numiter = new QLineEdit(central);
    connect(numiter, SIGNAL(returnPressed()),
            this, SLOT(numiterProcessText()));
    mainLayout->addWidget(numiter, 2,1);

    outputLabel = new QLabel(tr("Output file"), central);
    mainLayout->addWidget(outputLabel,3,0);
    output = new QLineEdit(central);
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
//   Kathleen Bonnell, Mon Jun 30 15:16:12 PDT 2008
//   Removed unreferenced variables.
//
//   Cyrus Harrison, Fri Dec  5 09:19:42 PST 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisTraceHistoryWindow::UpdateWindow(bool doAll)
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
            case 0: //vars
            {
                temp = "";
                for (int j = 0 ; j < atts->GetVars().size() ; j++)
                    temp += QString(atts->GetVars()[j].c_str()) + QString(" ");
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
//   Cyrus Harrison, Fri Dec  5 09:19:42 PST 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisTraceHistoryWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do exprs
    if(which_widget == 0 || doAll)
    {
        stringVector userVars;
        temp = varsLineEdit->text().simplified();
        QStringList lst = temp.split(" ",QString::SkipEmptyParts);

        QStringListIterator it(lst);
        while(it.hasNext())
            userVars.push_back(it.next().toStdString());
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
    
        int val;
        if(LineEditGetInt(numiter, val))
            atts->SetNumiter(val);
        else
        {
            ResettingError(tr("number of iterations"),
                           IntToQString(atts->GetNumiter()));
            atts->SetNumiter(atts->GetNumiter());
        }
    }

    // Do output
    if(which_widget == 3 || doAll)
    {
        temp = output->text();
        if(!temp.isEmpty())
        {
            atts->SetOutput(temp.toStdString());
        }
        else
        {
            ResettingError(tr("output"),
                           atts->GetOutput().c_str());
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
    atts->SetDisplacement(varName.toStdString());
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
//   Cyrus Harrison, Fri Dec  5 09:19:42 PST 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisTraceHistoryWindow::addVariable(const QString &var)
{
    // Add the new variable to the pick variable line edit.
    QString varString(varsLineEdit->text());
    if(varString.length() > 0)
        varString += " ";
    varString += var;
    varsLineEdit->setText(varString);

    // Process the list of pick vars.
    variableProcessText();
}



