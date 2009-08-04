/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
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
//    Cyrus Harrison, Tue Aug 19 08:13:21 PDT 2008
//    Qt4 Port.
// 
// ****************************************************************************

void
QvisZoneDumpWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);


    variableLabel = new QLabel(tr("Dump Variable"), central);
    mainLayout->addWidget(variableLabel,0,0);
    int variableMask = QvisVariableButton::Scalars;
    variable = new QvisVariableButton(true, true, true, variableMask, central);
    connect(variable, SIGNAL(activated(const QString&)),
            this, SLOT(variableChanged(const QString&)));
    mainLayout->addWidget(variable, 0,1);

    QLabel *range_label = new QLabel(tr("Dump Zones in Range:"), central);
    mainLayout->addWidget(range_label, 1,0,1,2);

    lowerBoundLabel = new QLabel(tr("Lower Bound"), central);
    mainLayout->addWidget(lowerBoundLabel,2,0);
    lowerBound = new QLineEdit(central);
    connect(lowerBound, SIGNAL(returnPressed()),
            this, SLOT(lowerBoundProcessText()));
    mainLayout->addWidget(lowerBound, 2,1);

    upperBoundLabel = new QLabel(tr("Upper Bound"), central);
    mainLayout->addWidget(upperBoundLabel,3,0);
    upperBound = new QLineEdit(central);
    connect(upperBound, SIGNAL(returnPressed()),
            this, SLOT(upperBoundProcessText()));
    mainLayout->addWidget(upperBound, 3,1);

    outputFileLabel = new QLabel(tr("Output File"), central);
    mainLayout->addWidget(outputFileLabel,4,0);
    outputFile = new QLineEdit(central);
    connect(outputFile, SIGNAL(returnPressed()),
            this, SLOT(outputFileProcessText()));
    mainLayout->addWidget(outputFile, 4,1);

    enabledLabel = NULL;
    enabled = new QCheckBox(tr("Dump Enabled"), central);
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
//    Kathleen Bonnell, Tue Jul 1 12:00:05 PDT 2008
//    Removed unreferenced variables.
//
// ****************************************************************************

void
QvisZoneDumpWindow::UpdateWindow(bool doAll)
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
//    Cyrus Harrison, Tue Aug 19 08:13:21 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisZoneDumpWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString temp;

    // Do lowerBound
    if(which_widget == ZoneDumpAttributes::ID_lowerBound || doAll)
    {
        temp = lowerBound->displayText().simplified();
        if (temp == QString("min"))
        {
            atts->SetLowerBound(-1e+37);
        }
        else
        {
            double val;
            if(LineEditGetDouble(lowerBound, val))
                atts->SetLowerBound(val);
            else
            {
                ResettingError(tr("Lower Bound"),
                    DoubleToQString(atts->GetLowerBound()));
                atts->SetLowerBound(atts->GetLowerBound());
            }
        }
    }

    // Do upperBound
    if(which_widget == ZoneDumpAttributes::ID_upperBound|| doAll)
    {
        temp = upperBound->displayText().simplified();
        if (temp == QString("max"))
        {
            atts->SetUpperBound(+1e+37);
        }
        else
        {
            double val;
            if(LineEditGetDouble(upperBound, val))
                atts->SetUpperBound(val);
            else
            {
                ResettingError(tr("Upper Bound"),
                    DoubleToQString(atts->GetUpperBound()));
                atts->SetUpperBound(atts->GetUpperBound());
            }
        }
    }

    // Do outputFile
    if(which_widget == ZoneDumpAttributes::ID_outputFile || doAll)
    {
        temp = outputFile->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetOutputFile(temp.toStdString());
        }
        else if(!okay)
        {
            ResettingError(tr("Output File "),
                           atts->GetOutputFile().c_str());
            atts->SetOutputFile(atts->GetOutputFile());
        }
    }

}


//
// Qt Slot functions
//


void
QvisZoneDumpWindow::variableChanged(const QString &varName)
{
    atts->SetVariable(varName.toStdString());
    SetUpdate(false);
    Apply();
}


void
QvisZoneDumpWindow::lowerBoundProcessText()
{
    GetCurrentValues(ZoneDumpAttributes::ID_lowerBound);
    Apply();
}


void
QvisZoneDumpWindow::upperBoundProcessText()
{
    GetCurrentValues(ZoneDumpAttributes::ID_upperBound);
    Apply();
}


void
QvisZoneDumpWindow::outputFileProcessText()
{
    GetCurrentValues(ZoneDumpAttributes::ID_outputFile);
    Apply();
}


void
QvisZoneDumpWindow::enabledChanged(bool val)
{
    atts->SetEnabled(val);
    SetUpdate(false);
    Apply();
}


