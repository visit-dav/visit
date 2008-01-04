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

#include "QvisSmoothWindow.h"

#include <SmoothOperatorAttributes.h>
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
// Method: QvisSmoothWindow::QvisSmoothWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisSmoothWindow::QvisSmoothWindow(const int type,
                         SmoothOperatorAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisSmoothWindow::~QvisSmoothWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisSmoothWindow::~QvisSmoothWindow()
{
}


// ****************************************************************************
// Method: QvisSmoothWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSmoothWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,2,  10, "mainLayout");


    numIterationsLabel = new QLabel("Maximum Number of Iterations", central, "numIterationsLabel");
    mainLayout->addWidget(numIterationsLabel,0,0);
    numIterations = new QLineEdit(central, "numIterations");
    connect(numIterations, SIGNAL(returnPressed()),
            this, SLOT(numIterationsProcessText()));
    mainLayout->addWidget(numIterations, 0,1);

    relaxationFactorLabel = new QLabel("Relaxation Factor", central, "relaxationFactorLabel");
    mainLayout->addWidget(relaxationFactorLabel,1,0);
    relaxationFactor = new QLineEdit(central, "relaxationFactor");
    connect(relaxationFactor, SIGNAL(returnPressed()),
            this, SLOT(relaxationFactorProcessText()));
    mainLayout->addWidget(relaxationFactor, 1,1);

    convergenceLabel = new QLabel("Convergence", central, "convergenceLabel");
    mainLayout->addWidget(convergenceLabel,2,0);
    convergence = new QLineEdit(central, "convergence");
    connect(convergence, SIGNAL(returnPressed()),
            this, SLOT(convergenceProcessText()));
    mainLayout->addWidget(convergence, 2,1);

    maintainFeatures = new QCheckBox("Maintain Features", central, "maintainFeatures");
    connect(maintainFeatures, SIGNAL(toggled(bool)),
            this, SLOT(maintainFeaturesChanged(bool)));
    mainLayout->addWidget(maintainFeatures, 3,0);

    featureAngleLabel = new QLabel("Feature Angle", central, "featureAngleLabel");
    mainLayout->addWidget(featureAngleLabel,4,0);
    featureAngle = new QLineEdit(central, "featureAngle");
    connect(featureAngle, SIGNAL(returnPressed()),
            this, SLOT(featureAngleProcessText()));
    mainLayout->addWidget(featureAngle, 4,1);

    edgeAngleLabel = new QLabel("Max Edge Angle", central, "edgeAngleLabel");
    mainLayout->addWidget(edgeAngleLabel,5,0);
    edgeAngle = new QLineEdit(central, "edgeAngle");
    connect(edgeAngle, SIGNAL(returnPressed()),
            this, SLOT(edgeAngleProcessText()));
    mainLayout->addWidget(edgeAngle, 5,1);

    smoothBoundaries = new QCheckBox("Smooth Along Boundaries", central, "smoothBoundaries");
    connect(smoothBoundaries, SIGNAL(toggled(bool)),
            this, SLOT(smoothBoundariesChanged(bool)));
    mainLayout->addWidget(smoothBoundaries, 6,0);

}


// ****************************************************************************
// Method: QvisSmoothWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSmoothWindow::UpdateWindow(bool doAll)
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
          case 0: //numIterations
            temp.sprintf("%d", atts->GetNumIterations());
            numIterations->setText(temp);
            break;
          case 1: //relaxationFactor
            temp.setNum(atts->GetRelaxationFactor());
            relaxationFactor->setText(temp);
            break;
          case 2: //convergence
            temp.setNum(atts->GetConvergence());
            convergence->setText(temp);
            break;
          case 3: //maintainFeatures
            if (atts->GetMaintainFeatures() == true)
            {
                featureAngle->setEnabled(true);
                featureAngleLabel->setEnabled(true);
            }
            else
            {
                featureAngle->setEnabled(false);
                featureAngleLabel->setEnabled(false);
            }
            maintainFeatures->setChecked(atts->GetMaintainFeatures());
            break;
          case 4: //featureAngle
            temp.setNum(atts->GetFeatureAngle());
            featureAngle->setText(temp);
            break;
          case 5: //edgeAngle
            temp.setNum(atts->GetEdgeAngle());
            edgeAngle->setText(temp);
            break;
          case 6: //smoothBoundaries
            smoothBoundaries->setChecked(atts->GetSmoothBoundaries());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisSmoothWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSmoothWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do numIterations
    if(which_widget == 0 || doAll)
    {
        temp = numIterations->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetNumIterations(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of numIterations was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetNumIterations());
            Message(msg);
            atts->SetNumIterations(atts->GetNumIterations());
        }
    }

    // Do relaxationFactor
    if(which_widget == 1 || doAll)
    {
        temp = relaxationFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetRelaxationFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of relaxationFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRelaxationFactor());
            Message(msg);
            atts->SetRelaxationFactor(atts->GetRelaxationFactor());
        }
    }

    // Do convergence
    if(which_widget == 2 || doAll)
    {
        temp = convergence->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetConvergence(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of convergence was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetConvergence());
            Message(msg);
            atts->SetConvergence(atts->GetConvergence());
        }
    }

    // Do maintainFeatures
    if(which_widget == 3 || doAll)
    {
        // Nothing for maintainFeatures
    }

    // Do featureAngle
    if(which_widget == 4 || doAll)
    {
        temp = featureAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetFeatureAngle(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of featureAngle was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetFeatureAngle());
            Message(msg);
            atts->SetFeatureAngle(atts->GetFeatureAngle());
        }
    }

    // Do edgeAngle
    if(which_widget == 5 || doAll)
    {
        temp = edgeAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetEdgeAngle(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of edgeAngle was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetEdgeAngle());
            Message(msg);
            atts->SetEdgeAngle(atts->GetEdgeAngle());
        }
    }

    // Do smoothBoundaries
    if(which_widget == 6 || doAll)
    {
        // Nothing for smoothBoundaries
    }

}


//
// Qt Slot functions
//


void
QvisSmoothWindow::numIterationsProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisSmoothWindow::relaxationFactorProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisSmoothWindow::convergenceProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisSmoothWindow::maintainFeaturesChanged(bool val)
{
    atts->SetMaintainFeatures(val);
    Apply();
}


void
QvisSmoothWindow::featureAngleProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisSmoothWindow::edgeAngleProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisSmoothWindow::smoothBoundariesChanged(bool val)
{
    atts->SetSmoothBoundaries(val);
    Apply();
}


