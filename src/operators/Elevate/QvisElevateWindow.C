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

#include "QvisElevateWindow.h"

#include <ElevateAttributes.h>
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
// Method: QvisElevateWindow::QvisElevateWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisElevateWindow::QvisElevateWindow(const int type,
                         ElevateAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisElevateWindow::~QvisElevateWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisElevateWindow::~QvisElevateWindow()
{
}


// ****************************************************************************
// Method: QvisElevateWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Mar 3 13:30:11 PST 2005
//   Changed the layout a little.
//
//   Brad Whitlock, Fri Apr 25 09:11:02 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisElevateWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 10,2,  10, "mainLayout");


    useXYLimitsLabel = NULL;
    useXYLimits = new QCheckBox(tr("Elevation height relative to XY limits?"), central, "useXYLimits");
    connect(useXYLimits, SIGNAL(toggled(bool)),
            this, SLOT(useXYLimitsChanged(bool)));
    mainLayout->addMultiCellWidget(useXYLimits, 0,0,0,1);

    limitsModeLabel = new QLabel(tr("Limits mode"), central, "limitsModeLabel");
    mainLayout->addWidget(limitsModeLabel,1,0);
    limitsMode = new QButtonGroup(central, "limitsMode");
    limitsMode->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *limitsModeLayout = new QHBoxLayout(limitsMode);
    limitsModeLayout->setSpacing(10);
    QRadioButton *limitsModeLimitsModeOriginalData = new QRadioButton(tr("Original Data"), limitsMode);
    limitsModeLayout->addWidget(limitsModeLimitsModeOriginalData);
    QRadioButton *limitsModeLimitsModeCurrentPlot = new QRadioButton(tr("Current Plot"), limitsMode);
    limitsModeLayout->addWidget(limitsModeLimitsModeCurrentPlot);
    connect(limitsMode, SIGNAL(clicked(int)),
            this, SLOT(limitsModeChanged(int)));
    mainLayout->addWidget(limitsMode, 1,1);

    scalingLabel = new QLabel(tr("Scale"), central, "scalingLabel");
    mainLayout->addWidget(scalingLabel,2,0);
    scaling = new QButtonGroup(central, "scaling");
    scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *scalingLayout = new QHBoxLayout(scaling);
    scalingLayout->setSpacing(10);
    QRadioButton *scalingScalingLinear = new QRadioButton(tr("Linear"), scaling);
    scalingLayout->addWidget(scalingScalingLinear);
    QRadioButton *scalingScalingLog = new QRadioButton(tr("Log"), scaling);
    scalingLayout->addWidget(scalingScalingLog);
    QRadioButton *scalingScalingSkew = new QRadioButton(tr("Skew"), scaling);
    scalingLayout->addWidget(scalingScalingSkew);
    connect(scaling, SIGNAL(clicked(int)),
            this, SLOT(scalingChanged(int)));
    mainLayout->addWidget(scaling, 2,1);

    skewFactorLabel = new QLabel(tr("Skew factor"), central, "skewFactorLabel");
    mainLayout->addWidget(skewFactorLabel,3,0);
    skewFactor = new QLineEdit(central, "skewFactor");
    connect(skewFactor, SIGNAL(returnPressed()),
            this, SLOT(skewFactorProcessText()));
    mainLayout->addWidget(skewFactor, 3,1);

    minFlagLabel = NULL;
    minFlag = new QCheckBox(tr("Use min"), central, "minFlag");
    connect(minFlag, SIGNAL(toggled(bool)),
            this, SLOT(minFlagChanged(bool)));
    mainLayout->addWidget(minFlag, 4,0);

    minLabel = new QLabel(tr("Min"), central, "minLabel");
    mainLayout->addWidget(minLabel,5,0);
    min = new QLineEdit(central, "min");
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    mainLayout->addWidget(min, 5,1);

    maxFlagLabel = NULL;
    maxFlag = new QCheckBox(tr("Use max"), central, "maxFlag");
    connect(maxFlag, SIGNAL(toggled(bool)),
            this, SLOT(maxFlagChanged(bool)));
    mainLayout->addWidget(maxFlag, 6,0);

    maxLabel = new QLabel(tr("Max"), central, "maxLabel");
    mainLayout->addWidget(maxLabel,7,0);
    max = new QLineEdit(central, "max");
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    mainLayout->addWidget(max, 7,1);

    zeroFlagLabel = NULL;
    zeroFlag = new QCheckBox(tr("Elevate with zero height?"), central, "zeroFlag");
    connect(zeroFlag, SIGNAL(toggled(bool)),
            this, SLOT(zeroFlagChanged(bool)));
    mainLayout->addMultiCellWidget(zeroFlag, 8,8,0,1);

    variableLabel = new QLabel(tr("Elevate by variable"), central, "variableLabel");
    mainLayout->addWidget(variableLabel,9,0);
    int variableMask = QvisVariableButton::Scalars;
    variable = new QvisVariableButton(true, true, true, variableMask, central, "variable");
    variable->setDefaultVariable("default");
    connect(variable, SIGNAL(activated(const QString&)),
            this, SLOT(variableChanged(const QString&)));
    mainLayout->addWidget(variable, 9,1);

}


// ****************************************************************************
// Method: QvisElevateWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 11:51:37 PDT 2008
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisElevateWindow::UpdateWindow(bool doAll)
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
          case 0: //useXYLimits
            if (atts->GetUseXYLimits() == true)
            {
                limitsMode->setEnabled(true);
                if(limitsModeLabel)
                    limitsModeLabel->setEnabled(true);
            }
            else
            {
                limitsMode->setEnabled(false);
                if(limitsModeLabel)
                    limitsModeLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                scaling->setEnabled(true);
                if(scalingLabel)
                    scalingLabel->setEnabled(true);
            }
            else
            {
                scaling->setEnabled(false);
                if(scalingLabel)
                    scalingLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                minFlag->setEnabled(true);
                if(minFlagLabel)
                    minFlagLabel->setEnabled(true);
            }
            else
            {
                minFlag->setEnabled(false);
                if(minFlagLabel)
                    minFlagLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                maxFlag->setEnabled(true);
                if(maxFlagLabel)
                    maxFlagLabel->setEnabled(true);
            }
            else
            {
                maxFlag->setEnabled(false);
                if(maxFlagLabel)
                    maxFlagLabel->setEnabled(false);
            }
            useXYLimits->blockSignals(true);
            useXYLimits->setChecked(atts->GetUseXYLimits());
            useXYLimits->blockSignals(false);
            break;
          case 1: //limitsMode
            limitsMode->blockSignals(true);
            limitsMode->setButton(atts->GetLimitsMode());
            limitsMode->blockSignals(false);
            break;
          case 2: //scaling
            if (atts->GetScaling() == ElevateAttributes::Skew)
            {
                skewFactor->setEnabled(true);
                if(skewFactorLabel)
                    skewFactorLabel->setEnabled(true);
            }
            else
            {
                skewFactor->setEnabled(false);
                if(skewFactorLabel)
                    skewFactorLabel->setEnabled(false);
            }
            scaling->blockSignals(true);
            scaling->setButton(atts->GetScaling());
            scaling->blockSignals(false);
            break;
          case 3: //skewFactor
            skewFactor->blockSignals(true);
            temp.setNum(atts->GetSkewFactor());
            skewFactor->setText(temp);
            skewFactor->blockSignals(false);
            break;
          case 4: //minFlag
            if (atts->GetMinFlag() == true)
            {
                min->setEnabled(true);
                if(minLabel)
                    minLabel->setEnabled(true);
            }
            else
            {
                min->setEnabled(false);
                if(minLabel)
                    minLabel->setEnabled(false);
            }
            minFlag->blockSignals(true);
            minFlag->setChecked(atts->GetMinFlag());
            minFlag->blockSignals(false);
            break;
          case 5: //min
            min->blockSignals(true);
            temp.setNum(atts->GetMin());
            min->setText(temp);
            min->blockSignals(false);
            break;
          case 6: //maxFlag
            if (atts->GetMaxFlag() == true)
            {
                max->setEnabled(true);
                if(maxLabel)
                    maxLabel->setEnabled(true);
            }
            else
            {
                max->setEnabled(false);
                if(maxLabel)
                    maxLabel->setEnabled(false);
            }
            maxFlag->blockSignals(true);
            maxFlag->setChecked(atts->GetMaxFlag());
            maxFlag->blockSignals(false);
            break;
          case 7: //max
            max->blockSignals(true);
            temp.setNum(atts->GetMax());
            max->setText(temp);
            max->blockSignals(false);
            break;
          case 8: //zeroFlag
            if (atts->GetZeroFlag() == false)
            {
                variable->setEnabled(true);
                if(variableLabel)
                    variableLabel->setEnabled(true);
            }
            else
            {
                variable->setEnabled(false);
                if(variableLabel)
                    variableLabel->setEnabled(false);
            }
            zeroFlag->blockSignals(true);
            zeroFlag->setChecked(atts->GetZeroFlag());
            zeroFlag->blockSignals(false);
            break;
          case 9: //variable
            variable->blockSignals(true);
            variable->setText(atts->GetVariable().c_str());
            variable->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisElevateWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisElevateWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do useXYLimits
    if(which_widget == 0 || doAll)
    {
        // Nothing for useXYLimits
    }

    // Do limitsMode
    if(which_widget == 1 || doAll)
    {
        // Nothing for limitsMode
    }

    // Do scaling
    if(which_widget == 2 || doAll)
    {
        // Nothing for scaling
    }

    // Do skewFactor
    if(which_widget == 3 || doAll)
    {
        temp = skewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetSkewFactor(val);
        }

        if(!okay)
        {
            msg = tr("The value of skewFactor was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetSkewFactor());
            Message(msg);
            atts->SetSkewFactor(atts->GetSkewFactor());
        }
    }

    // Do minFlag
    if(which_widget == 4 || doAll)
    {
        // Nothing for minFlag
    }

    // Do min
    if(which_widget == 5 || doAll)
    {
        temp = min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetMin(val);
        }

        if(!okay)
        {
            msg = tr("The value of min was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetMin());
            Message(msg);
            atts->SetMin(atts->GetMin());
        }
    }

    // Do maxFlag
    if(which_widget == 6 || doAll)
    {
        // Nothing for maxFlag
    }

    // Do max
    if(which_widget == 7 || doAll)
    {
        temp = max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetMax(val);
        }

        if(!okay)
        {
            msg = tr("The value of max was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetMax());
            Message(msg);
            atts->SetMax(atts->GetMax());
        }
    }

    // Do zeroFlag
    if(which_widget == 8 || doAll)
    {
        // Nothing for zeroFlag
    }

    // Do variable
    if(which_widget == 9 || doAll)
    {
        // Nothing for variable
    }

}


//
// Qt Slot functions
//


void
QvisElevateWindow::useXYLimitsChanged(bool val)
{
    atts->SetUseXYLimits(val);
    Apply();
}


void
QvisElevateWindow::limitsModeChanged(int val)
{
    if(val != atts->GetLimitsMode())
    {
        atts->SetLimitsMode(ElevateAttributes::LimitsMode(val));
        SetUpdate(false);
        Apply();
    }
}


void
QvisElevateWindow::scalingChanged(int val)
{
    if(val != atts->GetScaling())
    {
        atts->SetScaling(ElevateAttributes::Scaling(val));
        Apply();
    }
}


void
QvisElevateWindow::skewFactorProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisElevateWindow::minFlagChanged(bool val)
{
    atts->SetMinFlag(val);
    Apply();
}


void
QvisElevateWindow::minProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisElevateWindow::maxFlagChanged(bool val)
{
    atts->SetMaxFlag(val);
    Apply();
}


void
QvisElevateWindow::maxProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisElevateWindow::zeroFlagChanged(bool val)
{
    atts->SetZeroFlag(val);
    Apply();
}


void
QvisElevateWindow::variableChanged(const QString &varName)
{
    atts->SetVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


