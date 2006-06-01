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
// Creation:   Tue Feb 1 11:37:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisElevateWindow::QvisElevateWindow(const int type,
                         ElevateAttributes *subj,
                         const char *caption,
                         const char *shortName,
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
// Creation:   Tue Feb 1 11:37:30 PDT 2005
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
// Creation:   Tue Feb 1 11:37:30 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Mar 3 13:30:11 PST 2005
//   Changed the layout a little.
//
// ****************************************************************************

void
QvisElevateWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,2,  10, "mainLayout");


    useXYLimits = new QCheckBox("Elevation height relative to XY limits?", central, "useXYLimits");
    connect(useXYLimits, SIGNAL(toggled(bool)),
            this, SLOT(useXYLimitsChanged(bool)));
    mainLayout->addMultiCellWidget(useXYLimits, 0,0, 0,1);

    limitsModeLabel = new QLabel("LimitsMode", central, "limitsModeLabel");
    mainLayout->addWidget(limitsModeLabel,1,0);
    limitsMode = new QButtonGroup(central, "limitsMode");
    limitsMode->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *limitsModeLayout = new QHBoxLayout(limitsMode);
    limitsModeLayout->setSpacing(10);
    QRadioButton *limitsModeLimitsModeOriginalData = new QRadioButton("OriginalData", limitsMode);
    limitsModeLayout->addWidget(limitsModeLimitsModeOriginalData);
    QRadioButton *limitsModeLimitsModeCurrentPlot = new QRadioButton("CurrentPlot", limitsMode);
    limitsModeLayout->addWidget(limitsModeLimitsModeCurrentPlot);
    connect(limitsMode, SIGNAL(clicked(int)),
            this, SLOT(limitsModeChanged(int)));
    mainLayout->addWidget(limitsMode, 1,1);

    scalingLabel = new QLabel("Scale", central, "scalingLabel");
    mainLayout->addWidget(scalingLabel,2,0);
    scaling = new QButtonGroup(central, "scaling");
    scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *scalingLayout = new QHBoxLayout(scaling);
    scalingLayout->setSpacing(10);
    QRadioButton *scalingScalingLinear = new QRadioButton("Linear", scaling);
    scalingLayout->addWidget(scalingScalingLinear);
    QRadioButton *scalingScalingLog = new QRadioButton("Log", scaling);
    scalingLayout->addWidget(scalingScalingLog);
    QRadioButton *scalingScalingSkew = new QRadioButton("Skew", scaling);
    scalingLayout->addWidget(scalingScalingSkew);
    connect(scaling, SIGNAL(clicked(int)),
            this, SLOT(scalingChanged(int)));
    mainLayout->addWidget(scaling, 2,1);

    skewFactorLabel = new QLabel("Skew factor", central, "skewFactorLabel");
    mainLayout->addWidget(skewFactorLabel,3,0);
    skewFactor = new QLineEdit(central, "skewFactor");
    connect(skewFactor, SIGNAL(returnPressed()),
            this, SLOT(skewFactorProcessText()));
    mainLayout->addWidget(skewFactor, 3,1);

    minFlag = new QCheckBox("Min", central, "minFlag");
    connect(minFlag, SIGNAL(toggled(bool)),
            this, SLOT(minFlagChanged(bool)));
    mainLayout->addWidget(minFlag, 4,0);

    min = new QLineEdit(central, "min");
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    mainLayout->addWidget(min, 4,1);

    maxFlag = new QCheckBox("Max", central, "maxFlag");
    connect(maxFlag, SIGNAL(toggled(bool)),
            this, SLOT(maxFlagChanged(bool)));
    mainLayout->addWidget(maxFlag, 5,0);

    max = new QLineEdit(central, "max");
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    mainLayout->addWidget(max, 5,1);

    variableLabel = new QLabel("Elevate by Variable", central, "variableLabel");
    mainLayout->addWidget(variableLabel,6,0);
    int variableMask = QvisVariableButton::Scalars;
    variable = new QvisVariableButton(true, true, true, variableMask, central, "variable");
    variable->setDefaultVariable("default");
    connect(variable, SIGNAL(activated(const QString&)),
            this, SLOT(variableChanged(const QString&)));
    mainLayout->addWidget(variable, 6,1);

}


// ****************************************************************************
// Method: QvisElevateWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Feb 1 11:37:30 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Mar 3 13:32:37 PST 2005
//   I removed unused variables.
//
//   Hank Childs, Mon Nov 14 14:23:09 PST 2005
//   Only active min and max text fields when appropriate.  ['6784]
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
                limitsModeLabel->setEnabled(true);
            }
            else
            {
                limitsMode->setEnabled(false);
                limitsModeLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                scaling->setEnabled(true);
                scalingLabel->setEnabled(true);
            }
            else
            {
                scaling->setEnabled(false);
                scalingLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                if (atts->GetMinFlag())
                    min->setEnabled(true);
                else
                    min->setEnabled(false);
                minFlag->setEnabled(true);
                //minFlagLabel->setEnabled(true);
            }
            else
            {
                minFlag->setEnabled(false);
                min->setEnabled(false);
                //minFlagLabel->setEnabled(false);
            }
            if (atts->GetUseXYLimits() == true)
            {
                if (atts->GetMaxFlag())
                    max->setEnabled(true);
                else
                    max->setEnabled(false);
                maxFlag->setEnabled(true);
                //maxFlagLabel->setEnabled(true);
            }
            else
            {
                max->setEnabled(false);
                maxFlag->setEnabled(false);
                //maxFlagLabel->setEnabled(false);
            }
            useXYLimits->setChecked(atts->GetUseXYLimits());
            break;
          case 1: //limitsMode
            limitsMode->setButton(atts->GetLimitsMode());
            break;
          case 2: //scaling
            if (atts->GetScaling() == ElevateAttributes::Skew)
            {
                skewFactor->setEnabled(true);
                skewFactorLabel->setEnabled(true);
            }
            else
            {
                skewFactor->setEnabled(false);
                skewFactorLabel->setEnabled(false);
            }
            scaling->setButton(atts->GetScaling());
            break;
          case 3: //skewFactor
            temp.setNum(atts->GetSkewFactor());
            skewFactor->setText(temp);
            break;
          case 4: //minFlag
            if (atts->GetMinFlag() == true)
            {
                min->setEnabled(true);
                //minLabel->setEnabled(true);
            }
            else
            {
                min->setEnabled(false);
                //minLabel->setEnabled(false);
            }
            minFlag->setChecked(atts->GetMinFlag());
            break;
          case 5: //min
            temp.setNum(atts->GetMin());
            min->setText(temp);
            break;
          case 6: //maxFlag
            if (atts->GetMaxFlag() == true)
            {
                max->setEnabled(true);
                //maxLabel->setEnabled(true);
            }
            else
            {
                max->setEnabled(false);
                //maxLabel->setEnabled(false);
            }
            maxFlag->setChecked(atts->GetMaxFlag());
            break;
          case 7: //max
            temp.setNum(atts->GetMax());
            max->setText(temp);
            break;
          case 8: //variable
            variable->setText(atts->GetVariable().c_str());
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
// Creation:   Tue Feb 1 11:37:30 PDT 2005
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
            atts->SetSkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of skewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetSkewFactor());
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
            atts->SetMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMin());
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
            atts->SetMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMax());
            Message(msg);
            atts->SetMax(atts->GetMax());
        }
    }

    // Do variable
    if(which_widget == 8 || doAll)
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
QvisElevateWindow::variableChanged(const QString &varName)
{
    atts->SetVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


