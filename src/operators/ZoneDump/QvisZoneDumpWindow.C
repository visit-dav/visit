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

#include "QvisZoneDumpWindow.h"

#include <ZoneDumpAttributes.h>
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
// Method: QvisZoneDumpWindow::QvisZoneDumpWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisZoneDumpWindow::QvisZoneDumpWindow(const int type,
                         ZoneDumpAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisZoneDumpWindow::~QvisZoneDumpWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisZoneDumpWindow::~QvisZoneDumpWindow()
{
}


// ****************************************************************************
// Method: QvisZoneDumpWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//    Cyrus Harrison, Wed Apr  4 08:15:28 PDT 2007
//    Added range label.
//
//    Brad Whitlock, Thu Apr 24 15:47:48 PDT 2008
//    Added tr()'s
//
// ****************************************************************************

void
QvisZoneDumpWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5,2,  10, "mainLayout");


    variableLabel = new QLabel(tr("Dump Variable"), central, "variableLabel");
    mainLayout->addWidget(variableLabel,0,0);
    int variableMask = QvisVariableButton::Scalars;
    variable = new QvisVariableButton(true, true, true, variableMask, central, "variable");
    connect(variable, SIGNAL(activated(const QString&)),
            this, SLOT(variableChanged(const QString&)));
    mainLayout->addWidget(variable, 0,1);

    QLabel *range_label = new QLabel(tr("Dump Zones in Range:"), central, "range_label");
    mainLayout->addMultiCellWidget(range_label, 1,1,0,1);

    lowerBoundLabel = new QLabel(tr("Lower Bound"), central, "lowerBoundLabel");
    mainLayout->addWidget(lowerBoundLabel,2,0);
    lowerBound = new QLineEdit(central, "lowerBound");
    connect(lowerBound, SIGNAL(returnPressed()),
            this, SLOT(lowerBoundProcessText()));
    mainLayout->addWidget(lowerBound, 2,1);

    upperBoundLabel = new QLabel(tr("Upper Bound"), central, "upperBoundLabel");
    mainLayout->addWidget(upperBoundLabel,3,0);
    upperBound = new QLineEdit(central, "upperBound");
    connect(upperBound, SIGNAL(returnPressed()),
            this, SLOT(upperBoundProcessText()));
    mainLayout->addWidget(upperBound, 3,1);

    outputFileLabel = new QLabel(tr("Output File"), central, "outputFileLabel");
    mainLayout->addWidget(outputFileLabel,4,0);
    outputFile = new QLineEdit(central, "outputFile");
    connect(outputFile, SIGNAL(returnPressed()),
            this, SLOT(outputFileProcessText()));
    mainLayout->addWidget(outputFile, 4,1);

    enabledLabel = NULL;
    enabled = new QCheckBox(tr("Dump Enabled"), central, "enabled");
    connect(enabled, SIGNAL(toggled(bool)),
            this, SLOT(enabledChanged(bool)));
    mainLayout->addWidget(enabled, 5,0);

}


// ****************************************************************************
// Method: QvisZoneDumpWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//    Cyrus Harrison, Wed Apr  4 08:15:28 PDT 2007
//    Added support for min and max options
//
// ****************************************************************************

void
QvisZoneDumpWindow::UpdateWindow(bool doAll)
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
          case 0: //variable
            variable->blockSignals(true);
            variable->setText(atts->GetVariable().c_str());
            variable->blockSignals(false);
            break;
          case 1: //lowerBound
            lowerBound->blockSignals(true);
            if (atts->GetLowerBound() == -1e+37)
                temp = "min";
            else
                temp.setNum(atts->GetLowerBound());
            lowerBound->setText(temp);
            lowerBound->blockSignals(false);
            break;
          case 2: //upperBound
            upperBound->blockSignals(true);
            if (atts->GetUpperBound() == 1e+37)
                temp = "max";
            else
                temp.setNum(atts->GetUpperBound());
            upperBound->setText(temp);
            upperBound->blockSignals(false);
            break;
          case 3: //outputFile
            temp = atts->GetOutputFile().c_str();
            outputFile->blockSignals(true);
            outputFile->setText(temp);
            outputFile->blockSignals(false);
            break;
          case 4: //enabled
            enabled->blockSignals(true);
            enabled->setChecked(atts->GetEnabled());
            enabled->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisZoneDumpWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//    Cyrus Harrison, Wed Apr  4 08:15:28 PDT 2007
//    Added support for min and max options
//
// ****************************************************************************

void
QvisZoneDumpWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do variable
    if(which_widget == 0 || doAll)
    {
        // Nothing for variable
    }

    // Do lowerBound
    if(which_widget == 1 || doAll)
    {
        temp = lowerBound->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            if (temp.latin1() == QString("min"))
            {
                atts->SetLowerBound(-1e+37);
            }
            else
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetLowerBound(val);
            }
        }

        if(!okay)
        {
            msg = tr("The value of lowerBound was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetLowerBound());
            Message(msg);
            atts->SetLowerBound(atts->GetLowerBound());
        }
    }

    // Do upperBound
    if(which_widget == 2 || doAll)
    {
        temp = upperBound->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            if (temp.latin1() == QString("max"))
            {
                atts->SetUpperBound(+1e+37);
            }
            else
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetUpperBound(val);
            }
        }

        if(!okay)
        {
            msg = tr("The value of upperBound was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetUpperBound());
            Message(msg);
            atts->SetUpperBound(atts->GetUpperBound());
        }
    }

    // Do outputFile
    if(which_widget == 3 || doAll)
    {
        temp = outputFile->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetOutputFile(temp.latin1());
        }

        if(!okay)
        {
            msg = tr("The value of outputFile was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetOutputFile().c_str());
            Message(msg);
            atts->SetOutputFile(atts->GetOutputFile());
        }
    }

    // Do enabled
    if(which_widget == 4 || doAll)
    {
        // Nothing for enabled
    }

}


//
// Qt Slot functions
//


void
QvisZoneDumpWindow::variableChanged(const QString &varName)
{
    atts->SetVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisZoneDumpWindow::lowerBoundProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisZoneDumpWindow::upperBoundProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisZoneDumpWindow::outputFileProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisZoneDumpWindow::enabledChanged(bool val)
{
    atts->SetEnabled(val);
    SetUpdate(false);
    Apply();
}


