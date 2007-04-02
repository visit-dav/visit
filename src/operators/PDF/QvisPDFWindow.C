/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include "QvisPDFWindow.h"

#include <PDFAttributes.h>
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
// Method: QvisPDFWindow::QvisPDFWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisPDFWindow::QvisPDFWindow(const int type,
                         PDFAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisPDFWindow::~QvisPDFWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisPDFWindow::~QvisPDFWindow()
{
}


// ****************************************************************************
// Method: QvisPDFWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPDFWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 27,2,  10, "mainLayout");


    var1Label = new QLabel("var1", central, "var1Label");
    mainLayout->addWidget(var1Label,0,0);
    var1 = new QLineEdit(central, "var1");
    connect(var1, SIGNAL(returnPressed()),
            this, SLOT(var1ProcessText()));
    mainLayout->addWidget(var1, 0,1);

    var1MinFlag = new QCheckBox("Use variable 1 min", central, "var1MinFlag");
    connect(var1MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MinFlagChanged(bool)));
    mainLayout->addWidget(var1MinFlag, 1,0);

    var1MaxFlag = new QCheckBox("Use variable 1 max", central, "var1MaxFlag");
    connect(var1MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var1MaxFlagChanged(bool)));
    mainLayout->addWidget(var1MaxFlag, 2,0);

    var1MinLabel = new QLabel("Variable 1 Min", central, "var1MinLabel");
    mainLayout->addWidget(var1MinLabel,3,0);
    var1Min = new QLineEdit(central, "var1Min");
    connect(var1Min, SIGNAL(returnPressed()),
            this, SLOT(var1MinProcessText()));
    mainLayout->addWidget(var1Min, 3,1);

    var1MaxLabel = new QLabel("Variable 1 Max", central, "var1MaxLabel");
    mainLayout->addWidget(var1MaxLabel,4,0);
    var1Max = new QLineEdit(central, "var1Max");
    connect(var1Max, SIGNAL(returnPressed()),
            this, SLOT(var1MaxProcessText()));
    mainLayout->addWidget(var1Max, 4,1);

    var1ScalingLabel = new QLabel("Variable 1 scale", central, "var1ScalingLabel");
    mainLayout->addWidget(var1ScalingLabel,5,0);
    var1Scaling = new QButtonGroup(central, "var1Scaling");
    var1Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var1ScalingLayout = new QHBoxLayout(var1Scaling);
    var1ScalingLayout->setSpacing(10);
    QRadioButton *var1ScalingScalingLinear = new QRadioButton("Linear", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingLinear);
    QRadioButton *var1ScalingScalingLog = new QRadioButton("Log", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingLog);
    QRadioButton *var1ScalingScalingSkew = new QRadioButton("Skew", var1Scaling);
    var1ScalingLayout->addWidget(var1ScalingScalingSkew);
    connect(var1Scaling, SIGNAL(clicked(int)),
            this, SLOT(var1ScalingChanged(int)));
    mainLayout->addWidget(var1Scaling, 5,1);

    var1SkewFactorLabel = new QLabel("Variable 1 skew factor", central, "var1SkewFactorLabel");
    mainLayout->addWidget(var1SkewFactorLabel,6,0);
    var1SkewFactor = new QLineEdit(central, "var1SkewFactor");
    connect(var1SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var1SkewFactorProcessText()));
    mainLayout->addWidget(var1SkewFactor, 6,1);

    var1NumSamplesLabel = new QLabel("Number of samples for variable 1?", central, "var1NumSamplesLabel");
    mainLayout->addWidget(var1NumSamplesLabel,7,0);
    var1NumSamples = new QLineEdit(central, "var1NumSamples");
    connect(var1NumSamples, SIGNAL(returnPressed()),
            this, SLOT(var1NumSamplesProcessText()));
    mainLayout->addWidget(var1NumSamples, 7,1);

    var2Label = new QLabel("Variable 2", central, "var2Label");
    mainLayout->addWidget(var2Label,8,0);
    var2 = new QLineEdit(central, "var2");
    connect(var2, SIGNAL(returnPressed()),
            this, SLOT(var2ProcessText()));
    mainLayout->addWidget(var2, 8,1);

    var2MinFlag = new QCheckBox("Use variable 2 min", central, "var2MinFlag");
    connect(var2MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MinFlagChanged(bool)));
    mainLayout->addWidget(var2MinFlag, 9,0);

    var2MaxFlag = new QCheckBox("Use variable 2 max", central, "var2MaxFlag");
    connect(var2MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var2MaxFlagChanged(bool)));
    mainLayout->addWidget(var2MaxFlag, 10,0);

    var2MinLabel = new QLabel("Variable 2 Min", central, "var2MinLabel");
    mainLayout->addWidget(var2MinLabel,11,0);
    var2Min = new QLineEdit(central, "var2Min");
    connect(var2Min, SIGNAL(returnPressed()),
            this, SLOT(var2MinProcessText()));
    mainLayout->addWidget(var2Min, 11,1);

    var2MaxLabel = new QLabel("Variable 2 Max", central, "var2MaxLabel");
    mainLayout->addWidget(var2MaxLabel,12,0);
    var2Max = new QLineEdit(central, "var2Max");
    connect(var2Max, SIGNAL(returnPressed()),
            this, SLOT(var2MaxProcessText()));
    mainLayout->addWidget(var2Max, 12,1);

    var2ScalingLabel = new QLabel("Variable 2 scale", central, "var2ScalingLabel");
    mainLayout->addWidget(var2ScalingLabel,13,0);
    var2Scaling = new QButtonGroup(central, "var2Scaling");
    var2Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var2ScalingLayout = new QHBoxLayout(var2Scaling);
    var2ScalingLayout->setSpacing(10);
    QRadioButton *var2ScalingScalingLinear = new QRadioButton("Linear", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingLinear);
    QRadioButton *var2ScalingScalingLog = new QRadioButton("Log", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingLog);
    QRadioButton *var2ScalingScalingSkew = new QRadioButton("Skew", var2Scaling);
    var2ScalingLayout->addWidget(var2ScalingScalingSkew);
    connect(var2Scaling, SIGNAL(clicked(int)),
            this, SLOT(var2ScalingChanged(int)));
    mainLayout->addWidget(var2Scaling, 13,1);

    var2SkewFactorLabel = new QLabel("Variable 2 skew factor", central, "var2SkewFactorLabel");
    mainLayout->addWidget(var2SkewFactorLabel,14,0);
    var2SkewFactor = new QLineEdit(central, "var2SkewFactor");
    connect(var2SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var2SkewFactorProcessText()));
    mainLayout->addWidget(var2SkewFactor, 14,1);

    var2NumSamplesLabel = new QLabel("Number of samples for variable 2?", central, "var2NumSamplesLabel");
    mainLayout->addWidget(var2NumSamplesLabel,15,0);
    var2NumSamples = new QLineEdit(central, "var2NumSamples");
    connect(var2NumSamples, SIGNAL(returnPressed()),
            this, SLOT(var2NumSamplesProcessText()));
    mainLayout->addWidget(var2NumSamples, 15,1);

    numAxesLabel = new QLabel("Number of variables", central, "numAxesLabel");
    mainLayout->addWidget(numAxesLabel,16,0);
    numAxes = new QButtonGroup(central, "numAxes");
    numAxes->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *numAxesLayout = new QHBoxLayout(numAxes);
    numAxesLayout->setSpacing(10);
    QRadioButton *numAxesNumAxesTwo = new QRadioButton("Two", numAxes);
    numAxesLayout->addWidget(numAxesNumAxesTwo);
    QRadioButton *numAxesNumAxesThree = new QRadioButton("Three", numAxes);
    numAxesLayout->addWidget(numAxesNumAxesThree);
    connect(numAxes, SIGNAL(clicked(int)),
            this, SLOT(numAxesChanged(int)));
    mainLayout->addWidget(numAxes, 16,1);

    var3Label = new QLabel("Variable 3", central, "var3Label");
    mainLayout->addWidget(var3Label,17,0);
    var3 = new QLineEdit(central, "var3");
    connect(var3, SIGNAL(returnPressed()),
            this, SLOT(var3ProcessText()));
    mainLayout->addWidget(var3, 17,1);

    var3MinFlag = new QCheckBox("Use variable 3 min", central, "var3MinFlag");
    connect(var3MinFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MinFlagChanged(bool)));
    mainLayout->addWidget(var3MinFlag, 18,0);

    var3MaxFlag = new QCheckBox("Use variable 3 max", central, "var3MaxFlag");
    connect(var3MaxFlag, SIGNAL(toggled(bool)),
            this, SLOT(var3MaxFlagChanged(bool)));
    mainLayout->addWidget(var3MaxFlag, 19,0);

    var3MinLabel = new QLabel("Variable 3 Min", central, "var3MinLabel");
    mainLayout->addWidget(var3MinLabel,20,0);
    var3Min = new QLineEdit(central, "var3Min");
    connect(var3Min, SIGNAL(returnPressed()),
            this, SLOT(var3MinProcessText()));
    mainLayout->addWidget(var3Min, 20,1);

    var3MaxLabel = new QLabel("Variable 3 Max", central, "var3MaxLabel");
    mainLayout->addWidget(var3MaxLabel,21,0);
    var3Max = new QLineEdit(central, "var3Max");
    connect(var3Max, SIGNAL(returnPressed()),
            this, SLOT(var3MaxProcessText()));
    mainLayout->addWidget(var3Max, 21,1);

    var3ScalingLabel = new QLabel("Variable 3 scale", central, "var3ScalingLabel");
    mainLayout->addWidget(var3ScalingLabel,22,0);
    var3Scaling = new QButtonGroup(central, "var3Scaling");
    var3Scaling->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *var3ScalingLayout = new QHBoxLayout(var3Scaling);
    var3ScalingLayout->setSpacing(10);
    QRadioButton *var3ScalingScalingLinear = new QRadioButton("Linear", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingLinear);
    QRadioButton *var3ScalingScalingLog = new QRadioButton("Log", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingLog);
    QRadioButton *var3ScalingScalingSkew = new QRadioButton("Skew", var3Scaling);
    var3ScalingLayout->addWidget(var3ScalingScalingSkew);
    connect(var3Scaling, SIGNAL(clicked(int)),
            this, SLOT(var3ScalingChanged(int)));
    mainLayout->addWidget(var3Scaling, 22,1);

    var3SkewFactorLabel = new QLabel("Variable 3 skew factor", central, "var3SkewFactorLabel");
    mainLayout->addWidget(var3SkewFactorLabel,23,0);
    var3SkewFactor = new QLineEdit(central, "var3SkewFactor");
    connect(var3SkewFactor, SIGNAL(returnPressed()),
            this, SLOT(var3SkewFactorProcessText()));
    mainLayout->addWidget(var3SkewFactor, 23,1);

    var3NumSamplesLabel = new QLabel("Number of samples for variable 3?", central, "var3NumSamplesLabel");
    mainLayout->addWidget(var3NumSamplesLabel,24,0);
    var3NumSamples = new QLineEdit(central, "var3NumSamples");
    connect(var3NumSamples, SIGNAL(returnPressed()),
            this, SLOT(var3NumSamplesProcessText()));
    mainLayout->addWidget(var3NumSamples, 24,1);

    scaleCube = new QCheckBox("Scale to cube", central, "scaleCube");
    connect(scaleCube, SIGNAL(toggled(bool)),
            this, SLOT(scaleCubeChanged(bool)));
    mainLayout->addWidget(scaleCube, 25,0);

    densityTypeLabel = new QLabel("Density Type", central, "densityTypeLabel");
    mainLayout->addWidget(densityTypeLabel,26,0);
    densityType = new QButtonGroup(central, "densityType");
    densityType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *densityTypeLayout = new QHBoxLayout(densityType);
    densityTypeLayout->setSpacing(10);
    QRadioButton *densityTypeDensityTypeProbability = new QRadioButton("Probability", densityType);
    densityTypeLayout->addWidget(densityTypeDensityTypeProbability);
    QRadioButton *densityTypeDensityTypeZoneCount = new QRadioButton("Number of zones", densityType);
    densityTypeLayout->addWidget(densityTypeDensityTypeZoneCount);
    connect(densityType, SIGNAL(clicked(int)),
            this, SLOT(densityTypeChanged(int)));
    mainLayout->addWidget(densityType, 26,1);

}


// ****************************************************************************
// Method: QvisPDFWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPDFWindow::UpdateWindow(bool doAll)
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
          case 0: //var1
            temp = atts->GetVar1().c_str();
            var1->setText(temp);
            break;
          case 1: //var1MinFlag
            if (atts->GetVar1MinFlag() == true)
            {
                var1Min->setEnabled(true);
                var1MinLabel->setEnabled(true);
            }
            else
            {
                var1Min->setEnabled(false);
                var1MinLabel->setEnabled(false);
            }
            var1MinFlag->setChecked(atts->GetVar1MinFlag());
            break;
          case 2: //var1MaxFlag
            if (atts->GetVar1MaxFlag() == true)
            {
                var1Max->setEnabled(true);
                var1MaxLabel->setEnabled(true);
            }
            else
            {
                var1Max->setEnabled(false);
                var1MaxLabel->setEnabled(false);
            }
            var1MaxFlag->setChecked(atts->GetVar1MaxFlag());
            break;
          case 3: //var1Min
            temp.setNum(atts->GetVar1Min());
            var1Min->setText(temp);
            break;
          case 4: //var1Max
            temp.setNum(atts->GetVar1Max());
            var1Max->setText(temp);
            break;
          case 5: //var1Scaling
            if (atts->GetVar1Scaling() == PDFAttributes::Skew)
            {
                var1SkewFactor->setEnabled(true);
                var1SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var1SkewFactor->setEnabled(false);
                var1SkewFactorLabel->setEnabled(false);
            }
            var1Scaling->setButton(atts->GetVar1Scaling());
            break;
          case 6: //var1SkewFactor
            temp.setNum(atts->GetVar1SkewFactor());
            var1SkewFactor->setText(temp);
            break;
          case 7: //var1NumSamples
            temp.sprintf("%d", atts->GetVar1NumSamples());
            var1NumSamples->setText(temp);
            break;
          case 8: //var2
            temp = atts->GetVar2().c_str();
            var2->setText(temp);
            break;
          case 9: //var2MinFlag
            if (atts->GetVar2MinFlag() == true)
            {
                var2Min->setEnabled(true);
                var2MinLabel->setEnabled(true);
            }
            else
            {
                var2Min->setEnabled(false);
                var2MinLabel->setEnabled(false);
            }
            var2MinFlag->setChecked(atts->GetVar2MinFlag());
            break;
          case 10: //var2MaxFlag
            if (atts->GetVar2MaxFlag() == true)
            {
                var2Max->setEnabled(true);
                var2MaxLabel->setEnabled(true);
            }
            else
            {
                var2Max->setEnabled(false);
                var2MaxLabel->setEnabled(false);
            }
            var2MaxFlag->setChecked(atts->GetVar2MaxFlag());
            break;
          case 11: //var2Min
            temp.setNum(atts->GetVar2Min());
            var2Min->setText(temp);
            break;
          case 12: //var2Max
            temp.setNum(atts->GetVar2Max());
            var2Max->setText(temp);
            break;
          case 13: //var2Scaling
            if (atts->GetVar2Scaling() == PDFAttributes::Skew)
            {
                var2SkewFactor->setEnabled(true);
                var2SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var2SkewFactor->setEnabled(false);
                var2SkewFactorLabel->setEnabled(false);
            }
            var2Scaling->setButton(atts->GetVar2Scaling());
            break;
          case 14: //var2SkewFactor
            temp.setNum(atts->GetVar2SkewFactor());
            var2SkewFactor->setText(temp);
            break;
          case 15: //var2NumSamples
            temp.sprintf("%d", atts->GetVar2NumSamples());
            var2NumSamples->setText(temp);
            break;
          case 16: //numAxes
            numAxes->setButton(atts->GetNumAxes());
            break;
          case 17: //var3
            temp = atts->GetVar3().c_str();
            var3->setText(temp);
            break;
          case 18: //var3MinFlag
            if (atts->GetVar3MinFlag() == true)
            {
                var3Min->setEnabled(true);
                var3MinLabel->setEnabled(true);
            }
            else
            {
                var3Min->setEnabled(false);
                var3MinLabel->setEnabled(false);
            }
            var3MinFlag->setChecked(atts->GetVar3MinFlag());
            break;
          case 19: //var3MaxFlag
            if (atts->GetVar3MaxFlag() == true)
            {
                var3Max->setEnabled(true);
                var3MaxLabel->setEnabled(true);
            }
            else
            {
                var3Max->setEnabled(false);
                var3MaxLabel->setEnabled(false);
            }
            var3MaxFlag->setChecked(atts->GetVar3MaxFlag());
            break;
          case 20: //var3Min
            temp.setNum(atts->GetVar3Min());
            var3Min->setText(temp);
            break;
          case 21: //var3Max
            temp.setNum(atts->GetVar3Max());
            var3Max->setText(temp);
            break;
          case 22: //var3Scaling
            if (atts->GetVar3Scaling() == PDFAttributes::Skew)
            {
                var3SkewFactor->setEnabled(true);
                var3SkewFactorLabel->setEnabled(true);
            }
            else
            {
                var3SkewFactor->setEnabled(false);
                var3SkewFactorLabel->setEnabled(false);
            }
            var3Scaling->setButton(atts->GetVar3Scaling());
            break;
          case 23: //var3SkewFactor
            temp.setNum(atts->GetVar3SkewFactor());
            var3SkewFactor->setText(temp);
            break;
          case 24: //var3NumSamples
            temp.sprintf("%d", atts->GetVar3NumSamples());
            var3NumSamples->setText(temp);
            break;
          case 25: //scaleCube
            scaleCube->setChecked(atts->GetScaleCube());
            break;
          case 26: //densityType
            densityType->setButton(atts->GetDensityType());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisPDFWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPDFWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do var1
    if(which_widget == 0 || doAll)
    {
        temp = var1->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVar1(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of var1 was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVar1().c_str());
            Message(msg);
            atts->SetVar1(atts->GetVar1());
        }
    }

    // Do var1MinFlag
    if(which_widget == 1 || doAll)
    {
        // Nothing for var1MinFlag
    }

    // Do var1MaxFlag
    if(which_widget == 2 || doAll)
    {
        // Nothing for var1MaxFlag
    }

    // Do var1Min
    if(which_widget == 3 || doAll)
    {
        temp = var1Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1Min());
            Message(msg);
            atts->SetVar1Min(atts->GetVar1Min());
        }
    }

    // Do var1Max
    if(which_widget == 4 || doAll)
    {
        temp = var1Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1Max());
            Message(msg);
            atts->SetVar1Max(atts->GetVar1Max());
        }
    }

    // Do var1Scaling
    if(which_widget == 5 || doAll)
    {
        // Nothing for var1Scaling
    }

    // Do var1SkewFactor
    if(which_widget == 6 || doAll)
    {
        temp = var1SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar1SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar1SkewFactor());
            Message(msg);
            atts->SetVar1SkewFactor(atts->GetVar1SkewFactor());
        }
    }

    // Do var1NumSamples
    if(which_widget == 7 || doAll)
    {
        temp = var1NumSamples->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetVar1NumSamples(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var1NumSamples was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetVar1NumSamples());
            Message(msg);
            atts->SetVar1NumSamples(atts->GetVar1NumSamples());
        }
    }

    // Do var2
    if(which_widget == 8 || doAll)
    {
        temp = var2->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVar2(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of var2 was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVar2().c_str());
            Message(msg);
            atts->SetVar2(atts->GetVar2());
        }
    }

    // Do var2MinFlag
    if(which_widget == 9 || doAll)
    {
        // Nothing for var2MinFlag
    }

    // Do var2MaxFlag
    if(which_widget == 10 || doAll)
    {
        // Nothing for var2MaxFlag
    }

    // Do var2Min
    if(which_widget == 11 || doAll)
    {
        temp = var2Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2Min());
            Message(msg);
            atts->SetVar2Min(atts->GetVar2Min());
        }
    }

    // Do var2Max
    if(which_widget == 12 || doAll)
    {
        temp = var2Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2Max());
            Message(msg);
            atts->SetVar2Max(atts->GetVar2Max());
        }
    }

    // Do var2Scaling
    if(which_widget == 13 || doAll)
    {
        // Nothing for var2Scaling
    }

    // Do var2SkewFactor
    if(which_widget == 14 || doAll)
    {
        temp = var2SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar2SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar2SkewFactor());
            Message(msg);
            atts->SetVar2SkewFactor(atts->GetVar2SkewFactor());
        }
    }

    // Do var2NumSamples
    if(which_widget == 15 || doAll)
    {
        temp = var2NumSamples->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetVar2NumSamples(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var2NumSamples was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetVar2NumSamples());
            Message(msg);
            atts->SetVar2NumSamples(atts->GetVar2NumSamples());
        }
    }

    // Do numAxes
    if(which_widget == 16 || doAll)
    {
        // Nothing for numAxes
    }

    // Do var3
    if(which_widget == 17 || doAll)
    {
        temp = var3->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVar3(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of var3 was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVar3().c_str());
            Message(msg);
            atts->SetVar3(atts->GetVar3());
        }
    }

    // Do var3MinFlag
    if(which_widget == 18 || doAll)
    {
        // Nothing for var3MinFlag
    }

    // Do var3MaxFlag
    if(which_widget == 19 || doAll)
    {
        // Nothing for var3MaxFlag
    }

    // Do var3Min
    if(which_widget == 20 || doAll)
    {
        temp = var3Min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3Min(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3Min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3Min());
            Message(msg);
            atts->SetVar3Min(atts->GetVar3Min());
        }
    }

    // Do var3Max
    if(which_widget == 21 || doAll)
    {
        temp = var3Max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3Max(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3Max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3Max());
            Message(msg);
            atts->SetVar3Max(atts->GetVar3Max());
        }
    }

    // Do var3Scaling
    if(which_widget == 22 || doAll)
    {
        // Nothing for var3Scaling
    }

    // Do var3SkewFactor
    if(which_widget == 23 || doAll)
    {
        temp = var3SkewFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetVar3SkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3SkewFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetVar3SkewFactor());
            Message(msg);
            atts->SetVar3SkewFactor(atts->GetVar3SkewFactor());
        }
    }

    // Do var3NumSamples
    if(which_widget == 24 || doAll)
    {
        temp = var3NumSamples->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetVar3NumSamples(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of var3NumSamples was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetVar3NumSamples());
            Message(msg);
            atts->SetVar3NumSamples(atts->GetVar3NumSamples());
        }
    }

    // Do scaleCube
    if(which_widget == 25 || doAll)
    {
        // Nothing for scaleCube
    }

    // Do densityType
    if(which_widget == 26 || doAll)
    {
        // Nothing for densityType
    }

}


//
// Qt Slot functions
//


void
QvisPDFWindow::var1ProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisPDFWindow::var1MinFlagChanged(bool val)
{
    atts->SetVar1MinFlag(val);
    Apply();
}


void
QvisPDFWindow::var1MaxFlagChanged(bool val)
{
    atts->SetVar1MaxFlag(val);
    Apply();
}


void
QvisPDFWindow::var1MinProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisPDFWindow::var1MaxProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisPDFWindow::var1ScalingChanged(int val)
{
    if(val != atts->GetVar1Scaling())
    {
        atts->SetVar1Scaling(PDFAttributes::Scaling(val));
        Apply();
    }
}


void
QvisPDFWindow::var1SkewFactorProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisPDFWindow::var1NumSamplesProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisPDFWindow::var2ProcessText()
{
    GetCurrentValues(8);
    Apply();
}


void
QvisPDFWindow::var2MinFlagChanged(bool val)
{
    atts->SetVar2MinFlag(val);
    Apply();
}


void
QvisPDFWindow::var2MaxFlagChanged(bool val)
{
    atts->SetVar2MaxFlag(val);
    Apply();
}


void
QvisPDFWindow::var2MinProcessText()
{
    GetCurrentValues(11);
    Apply();
}


void
QvisPDFWindow::var2MaxProcessText()
{
    GetCurrentValues(12);
    Apply();
}


void
QvisPDFWindow::var2ScalingChanged(int val)
{
    if(val != atts->GetVar2Scaling())
    {
        atts->SetVar2Scaling(PDFAttributes::Scaling(val));
        Apply();
    }
}


void
QvisPDFWindow::var2SkewFactorProcessText()
{
    GetCurrentValues(14);
    Apply();
}


void
QvisPDFWindow::var2NumSamplesProcessText()
{
    GetCurrentValues(15);
    Apply();
}


void
QvisPDFWindow::numAxesChanged(int val)
{
    if(val != atts->GetNumAxes())
    {
        atts->SetNumAxes(PDFAttributes::NumAxes(val));
        Apply();
    }
}


void
QvisPDFWindow::var3ProcessText()
{
    GetCurrentValues(17);
    Apply();
}


void
QvisPDFWindow::var3MinFlagChanged(bool val)
{
    atts->SetVar3MinFlag(val);
    Apply();
}


void
QvisPDFWindow::var3MaxFlagChanged(bool val)
{
    atts->SetVar3MaxFlag(val);
    Apply();
}


void
QvisPDFWindow::var3MinProcessText()
{
    GetCurrentValues(20);
    Apply();
}


void
QvisPDFWindow::var3MaxProcessText()
{
    GetCurrentValues(21);
    Apply();
}


void
QvisPDFWindow::var3ScalingChanged(int val)
{
    if(val != atts->GetVar3Scaling())
    {
        atts->SetVar3Scaling(PDFAttributes::Scaling(val));
        Apply();
    }
}


void
QvisPDFWindow::var3SkewFactorProcessText()
{
    GetCurrentValues(23);
    Apply();
}


void
QvisPDFWindow::var3NumSamplesProcessText()
{
    GetCurrentValues(24);
    Apply();
}


void
QvisPDFWindow::scaleCubeChanged(bool val)
{
    atts->SetScaleCube(val);
    Apply();
}


void
QvisPDFWindow::densityTypeChanged(int val)
{
    if(val != atts->GetDensityType())
    {
        atts->SetDensityType(PDFAttributes::DensityType(val));
        Apply();
    }
}


