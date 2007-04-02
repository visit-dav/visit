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

#include "QvisResampleWindow.h"

#include <ResamplePluginAttributes.h>
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
// Method: QvisResampleWindow::QvisResampleWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Sep 30 09:26:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisResampleWindow::QvisResampleWindow(const int type,
                         ResamplePluginAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisResampleWindow::~QvisResampleWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Sep 30 09:26:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisResampleWindow::~QvisResampleWindow()
{
}


// ****************************************************************************
// Method: QvisResampleWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Sep 30 09:26:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisResampleWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 14,2,  10, "mainLayout");


    startXLabel = new QLabel("Start X", central, "startXLabel");
    mainLayout->addWidget(startXLabel,0,0);
    startX = new QLineEdit(central, "startX");
    connect(startX, SIGNAL(returnPressed()),
            this, SLOT(startXProcessText()));
    mainLayout->addWidget(startX, 0,1);

    endXLabel = new QLabel("End X", central, "endXLabel");
    mainLayout->addWidget(endXLabel,1,0);
    endX = new QLineEdit(central, "endX");
    connect(endX, SIGNAL(returnPressed()),
            this, SLOT(endXProcessText()));
    mainLayout->addWidget(endX, 1,1);

    samplesXLabel = new QLabel("Samples in X", central, "samplesXLabel");
    mainLayout->addWidget(samplesXLabel,2,0);
    samplesX = new QLineEdit(central, "samplesX");
    connect(samplesX, SIGNAL(returnPressed()),
            this, SLOT(samplesXProcessText()));
    mainLayout->addWidget(samplesX, 2,1);

    startYLabel = new QLabel("Start Y", central, "startYLabel");
    mainLayout->addWidget(startYLabel,3,0);
    startY = new QLineEdit(central, "startY");
    connect(startY, SIGNAL(returnPressed()),
            this, SLOT(startYProcessText()));
    mainLayout->addWidget(startY, 3,1);

    endYLabel = new QLabel("End Y", central, "endYLabel");
    mainLayout->addWidget(endYLabel,4,0);
    endY = new QLineEdit(central, "endY");
    connect(endY, SIGNAL(returnPressed()),
            this, SLOT(endYProcessText()));
    mainLayout->addWidget(endY, 4,1);

    samplesYLabel = new QLabel("Samples in Y", central, "samplesYLabel");
    mainLayout->addWidget(samplesYLabel,5,0);
    samplesY = new QLineEdit(central, "samplesY");
    connect(samplesY, SIGNAL(returnPressed()),
            this, SLOT(samplesYProcessText()));
    mainLayout->addWidget(samplesY, 5,1);

    is3D = new QCheckBox("3D resampling", central, "is3D");
    connect(is3D, SIGNAL(toggled(bool)),
            this, SLOT(is3DChanged(bool)));
    mainLayout->addWidget(is3D, 6,0);

    startZLabel = new QLabel("Start Z", central, "startZLabel");
    mainLayout->addWidget(startZLabel,7,0);
    startZ = new QLineEdit(central, "startZ");
    connect(startZ, SIGNAL(returnPressed()),
            this, SLOT(startZProcessText()));
    mainLayout->addWidget(startZ, 7,1);

    endZLabel = new QLabel("End Z", central, "endZLabel");
    mainLayout->addWidget(endZLabel,8,0);
    endZ = new QLineEdit(central, "endZ");
    connect(endZ, SIGNAL(returnPressed()),
            this, SLOT(endZProcessText()));
    mainLayout->addWidget(endZ, 8,1);

    samplesZLabel = new QLabel("Samples in Z", central, "samplesZLabel");
    mainLayout->addWidget(samplesZLabel,9,0);
    samplesZ = new QLineEdit(central, "samplesZ");
    connect(samplesZ, SIGNAL(returnPressed()),
            this, SLOT(samplesZProcessText()));
    mainLayout->addWidget(samplesZ, 9,1);

    tieResolverLabel = new QLabel("Resolve ties", central, "tieResolverLabel");
    mainLayout->addWidget(tieResolverLabel,10,0);
    tieResolver = new QButtonGroup(central, "tieResolver");
    tieResolver->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *tieResolverLayout = new QHBoxLayout(tieResolver);
    tieResolverLayout->setSpacing(10);
    QRadioButton *tieResolverTieResolverrandom = new QRadioButton("random", tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolverrandom);
    QRadioButton *tieResolverTieResolverlargest = new QRadioButton("largest", tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolverlargest);
    QRadioButton *tieResolverTieResolversmallest = new QRadioButton("smallest", tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolversmallest);
    connect(tieResolver, SIGNAL(clicked(int)),
            this, SLOT(tieResolverChanged(int)));
    mainLayout->addWidget(tieResolver, 10,1);

    tieResolverVariableLabel = new QLabel("Variable to resolve ties", central, "tieResolverVariableLabel");
    mainLayout->addWidget(tieResolverVariableLabel,11,0);
    int tieResolverVariableMask = QvisVariableButton::Scalars;
    tieResolverVariable = new QvisVariableButton(true, true, true, tieResolverVariableMask, central, "tieResolverVariable");
    tieResolverVariable->setDefaultVariable("default");
    connect(tieResolverVariable, SIGNAL(activated(const QString&)),
            this, SLOT(tieResolverVariableChanged(const QString&)));
    mainLayout->addWidget(tieResolverVariable, 11,1);

    defaultValueLabel = new QLabel("Value for uncovered regions", central, "defaultValueLabel");
    mainLayout->addWidget(defaultValueLabel,12,0);
    defaultValue = new QLineEdit(central, "defaultValue");
    connect(defaultValue, SIGNAL(returnPressed()),
            this, SLOT(defaultValueProcessText()));
    mainLayout->addWidget(defaultValue, 12,1);

    distributedResample = new QCheckBox("Distribute resampled data\nset across all processors\n(parallel only)?", central, "distributedResample");
    connect(distributedResample, SIGNAL(toggled(bool)),
            this, SLOT(distributedResampleChanged(bool)));
    mainLayout->addWidget(distributedResample, 13,0);

}


// ****************************************************************************
// Method: QvisResampleWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Sep 30 09:26:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisResampleWindow::UpdateWindow(bool doAll)
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
          case 0: //startX
            temp.setNum(atts->GetStartX());
            startX->setText(temp);
            break;
          case 1: //endX
            temp.setNum(atts->GetEndX());
            endX->setText(temp);
            break;
          case 2: //samplesX
            temp.sprintf("%d", atts->GetSamplesX());
            samplesX->setText(temp);
            break;
          case 3: //startY
            temp.setNum(atts->GetStartY());
            startY->setText(temp);
            break;
          case 4: //endY
            temp.setNum(atts->GetEndY());
            endY->setText(temp);
            break;
          case 5: //samplesY
            temp.sprintf("%d", atts->GetSamplesY());
            samplesY->setText(temp);
            break;
          case 6: //is3D
            if (atts->GetIs3D() == true)
            {
                startZ->setEnabled(true);
                startZLabel->setEnabled(true);
            }
            else
            {
                startZ->setEnabled(false);
                startZLabel->setEnabled(false);
            }
            if (atts->GetIs3D() == true)
            {
                endZ->setEnabled(true);
                endZLabel->setEnabled(true);
            }
            else
            {
                endZ->setEnabled(false);
                endZLabel->setEnabled(false);
            }
            if (atts->GetIs3D() == true)
            {
                samplesZ->setEnabled(true);
                samplesZLabel->setEnabled(true);
            }
            else
            {
                samplesZ->setEnabled(false);
                samplesZLabel->setEnabled(false);
            }
            is3D->setChecked(atts->GetIs3D());
            break;
          case 7: //startZ
            temp.setNum(atts->GetStartZ());
            startZ->setText(temp);
            break;
          case 8: //endZ
            temp.setNum(atts->GetEndZ());
            endZ->setText(temp);
            break;
          case 9: //samplesZ
            temp.sprintf("%d", atts->GetSamplesZ());
            samplesZ->setText(temp);
            break;
          case 10: //tieResolver
            if (atts->GetTieResolver() == ResamplePluginAttributes::smallest || atts->GetTieResolver() == ResamplePluginAttributes::largest)
            {
                tieResolverVariable->setEnabled(true);
                tieResolverVariableLabel->setEnabled(true);
            }
            else
            {
                tieResolverVariable->setEnabled(false);
                tieResolverVariableLabel->setEnabled(false);
            }
            tieResolver->setButton(atts->GetTieResolver());
            break;
          case 11: //tieResolverVariable
            tieResolverVariable->setText(atts->GetTieResolverVariable().c_str());
            break;
          case 12: //defaultValue
            temp.setNum(atts->GetDefaultValue());
            defaultValue->setText(temp);
            break;
          case 13: //distributedResample
            distributedResample->setChecked(atts->GetDistributedResample());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisResampleWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Sep 30 09:26:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisResampleWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do startX
    if(which_widget == 0 || doAll)
    {
        temp = startX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStartX(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startX was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStartX());
            Message(msg);
            atts->SetStartX(atts->GetStartX());
        }
    }

    // Do endX
    if(which_widget == 1 || doAll)
    {
        temp = endX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetEndX(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of endX was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetEndX());
            Message(msg);
            atts->SetEndX(atts->GetEndX());
        }
    }

    // Do samplesX
    if(which_widget == 2 || doAll)
    {
        temp = samplesX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetSamplesX(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of samplesX was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetSamplesX());
            Message(msg);
            atts->SetSamplesX(atts->GetSamplesX());
        }
    }

    // Do startY
    if(which_widget == 3 || doAll)
    {
        temp = startY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStartY(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startY was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStartY());
            Message(msg);
            atts->SetStartY(atts->GetStartY());
        }
    }

    // Do endY
    if(which_widget == 4 || doAll)
    {
        temp = endY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetEndY(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of endY was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetEndY());
            Message(msg);
            atts->SetEndY(atts->GetEndY());
        }
    }

    // Do samplesY
    if(which_widget == 5 || doAll)
    {
        temp = samplesY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetSamplesY(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of samplesY was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetSamplesY());
            Message(msg);
            atts->SetSamplesY(atts->GetSamplesY());
        }
    }

    // Do is3D
    if(which_widget == 6 || doAll)
    {
        // Nothing for is3D
    }

    // Do startZ
    if(which_widget == 7 || doAll)
    {
        temp = startZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStartZ(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startZ was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStartZ());
            Message(msg);
            atts->SetStartZ(atts->GetStartZ());
        }
    }

    // Do endZ
    if(which_widget == 8 || doAll)
    {
        temp = endZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetEndZ(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of endZ was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetEndZ());
            Message(msg);
            atts->SetEndZ(atts->GetEndZ());
        }
    }

    // Do samplesZ
    if(which_widget == 9 || doAll)
    {
        temp = samplesZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetSamplesZ(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of samplesZ was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetSamplesZ());
            Message(msg);
            atts->SetSamplesZ(atts->GetSamplesZ());
        }
    }

    // Do tieResolver
    if(which_widget == 10 || doAll)
    {
        // Nothing for tieResolver
    }

    // Do tieResolverVariable
    if(which_widget == 11 || doAll)
    {
        // Nothing for tieResolverVariable
    }

    // Do defaultValue
    if(which_widget == 12 || doAll)
    {
        temp = defaultValue->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetDefaultValue(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of defaultValue was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetDefaultValue());
            Message(msg);
            atts->SetDefaultValue(atts->GetDefaultValue());
        }
    }

    // Do distributedResample
    if(which_widget == 13 || doAll)
    {
        // Nothing for distributedResample
    }

}


//
// Qt Slot functions
//


void
QvisResampleWindow::startXProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisResampleWindow::endXProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisResampleWindow::samplesXProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisResampleWindow::startYProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisResampleWindow::endYProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisResampleWindow::samplesYProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisResampleWindow::is3DChanged(bool val)
{
    atts->SetIs3D(val);
    Apply();
}


void
QvisResampleWindow::startZProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisResampleWindow::endZProcessText()
{
    GetCurrentValues(8);
    Apply();
}


void
QvisResampleWindow::samplesZProcessText()
{
    GetCurrentValues(9);
    Apply();
}


void
QvisResampleWindow::tieResolverChanged(int val)
{
    if(val != atts->GetTieResolver())
    {
        atts->SetTieResolver(ResamplePluginAttributes::TieResolver(val));
        Apply();
    }
}


void
QvisResampleWindow::tieResolverVariableChanged(const QString &varName)
{
    atts->SetTieResolverVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisResampleWindow::defaultValueProcessText()
{
    GetCurrentValues(12);
    Apply();
}


void
QvisResampleWindow::distributedResampleChanged(bool val)
{
    atts->SetDistributedResample(val);
    Apply();
}


