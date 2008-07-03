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
// Creation:   Wed Jan 2 13:43:29 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisResampleWindow::QvisResampleWindow(const int type,
                         ResamplePluginAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
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
// Creation:   Wed Jan 2 13:43:29 PST 2008
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
// Creation:   Wed Jan 2 13:43:29 PST 2008
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 16:43:27 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisResampleWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 15,2,  10, "mainLayout");


    useExtents = new QCheckBox(tr("Resample Entire Extents"), central, "useExtents");
    connect(useExtents, SIGNAL(toggled(bool)),
            this, SLOT(useExtentsChanged(bool)));
    mainLayout->addWidget(useExtents, 0,0);

    startXLabel = new QLabel(tr("Start X"), central, "startXLabel");
    mainLayout->addWidget(startXLabel,1,0);
    startX = new QLineEdit(central, "startX");
    connect(startX, SIGNAL(returnPressed()),
            this, SLOT(startXProcessText()));
    mainLayout->addWidget(startX, 1,1);

    endXLabel = new QLabel(tr("End X"), central, "endXLabel");
    mainLayout->addWidget(endXLabel,2,0);
    endX = new QLineEdit(central, "endX");
    connect(endX, SIGNAL(returnPressed()),
            this, SLOT(endXProcessText()));
    mainLayout->addWidget(endX, 2,1);

    samplesXLabel = new QLabel(tr("Samples in X"), central, "samplesXLabel");
    mainLayout->addWidget(samplesXLabel,3,0);
    samplesX = new QLineEdit(central, "samplesX");
    connect(samplesX, SIGNAL(returnPressed()),
            this, SLOT(samplesXProcessText()));
    mainLayout->addWidget(samplesX, 3,1);

    startYLabel = new QLabel(tr("Start Y"), central, "startYLabel");
    mainLayout->addWidget(startYLabel,4,0);
    startY = new QLineEdit(central, "startY");
    connect(startY, SIGNAL(returnPressed()),
            this, SLOT(startYProcessText()));
    mainLayout->addWidget(startY, 4,1);

    endYLabel = new QLabel(tr("End Y"), central, "endYLabel");
    mainLayout->addWidget(endYLabel,5,0);
    endY = new QLineEdit(central, "endY");
    connect(endY, SIGNAL(returnPressed()),
            this, SLOT(endYProcessText()));
    mainLayout->addWidget(endY, 5,1);

    samplesYLabel = new QLabel(tr("Samples in Y"), central, "samplesYLabel");
    mainLayout->addWidget(samplesYLabel,6,0);
    samplesY = new QLineEdit(central, "samplesY");
    connect(samplesY, SIGNAL(returnPressed()),
            this, SLOT(samplesYProcessText()));
    mainLayout->addWidget(samplesY, 6,1);

    is3D = new QCheckBox(tr("3D resampling"), central, "is3D");
    connect(is3D, SIGNAL(toggled(bool)),
            this, SLOT(is3DChanged(bool)));
    mainLayout->addWidget(is3D, 7,0);

    startZLabel = new QLabel(tr("Start Z"), central, "startZLabel");
    mainLayout->addWidget(startZLabel,8,0);
    startZ = new QLineEdit(central, "startZ");
    connect(startZ, SIGNAL(returnPressed()),
            this, SLOT(startZProcessText()));
    mainLayout->addWidget(startZ, 8,1);

    endZLabel = new QLabel(tr("End Z"), central, "endZLabel");
    mainLayout->addWidget(endZLabel,9,0);
    endZ = new QLineEdit(central, "endZ");
    connect(endZ, SIGNAL(returnPressed()),
            this, SLOT(endZProcessText()));
    mainLayout->addWidget(endZ, 9,1);

    samplesZLabel = new QLabel(tr("Samples in Z"), central, "samplesZLabel");
    mainLayout->addWidget(samplesZLabel,10,0);
    samplesZ = new QLineEdit(central, "samplesZ");
    connect(samplesZ, SIGNAL(returnPressed()),
            this, SLOT(samplesZProcessText()));
    mainLayout->addWidget(samplesZ, 10,1);

    tieResolverLabel = new QLabel(tr("Resolve ties"), central, "tieResolverLabel");
    mainLayout->addWidget(tieResolverLabel,11,0);
    tieResolver = new QButtonGroup(central, "tieResolver");
    tieResolver->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *tieResolverLayout = new QHBoxLayout(tieResolver);
    tieResolverLayout->setSpacing(10);
    QRadioButton *tieResolverTieResolverrandom = new QRadioButton(tr("random"), tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolverrandom);
    QRadioButton *tieResolverTieResolverlargest = new QRadioButton(tr("largest"), tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolverlargest);
    QRadioButton *tieResolverTieResolversmallest = new QRadioButton(tr("smallest"), tieResolver);
    tieResolverLayout->addWidget(tieResolverTieResolversmallest);
    connect(tieResolver, SIGNAL(clicked(int)),
            this, SLOT(tieResolverChanged(int)));
    mainLayout->addWidget(tieResolver, 11,1);

    tieResolverVariableLabel = new QLabel(tr("Variable to resolve ties"), central, "tieResolverVariableLabel");
    mainLayout->addWidget(tieResolverVariableLabel,12,0);
    int tieResolverVariableMask = QvisVariableButton::Scalars;
    tieResolverVariable = new QvisVariableButton(true, true, true, tieResolverVariableMask, central, "tieResolverVariable");
    tieResolverVariable->setDefaultVariable("default");
    connect(tieResolverVariable, SIGNAL(activated(const QString&)),
            this, SLOT(tieResolverVariableChanged(const QString&)));
    mainLayout->addWidget(tieResolverVariable, 12,1);

    defaultValueLabel = new QLabel(tr("Value for uncovered regions"), central, "defaultValueLabel");
    mainLayout->addWidget(defaultValueLabel,13,0);
    defaultValue = new QLineEdit(central, "defaultValue");
    connect(defaultValue, SIGNAL(returnPressed()),
            this, SLOT(defaultValueProcessText()));
    mainLayout->addWidget(defaultValue, 13,1);

    distributedResample = new QCheckBox(tr("Distribute resampled data\nset across all processors\n(parallel only)?"), central, "distributedResample");
    connect(distributedResample, SIGNAL(toggled(bool)),
            this, SLOT(distributedResampleChanged(bool)));
    mainLayout->addWidget(distributedResample, 14,0);

}


// ****************************************************************************
// Method: QvisResampleWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Jan 2 13:43:29 PST 2008
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 11:44:57 PDT 2008 
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisResampleWindow::UpdateWindow(bool doAll)
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
          case ResamplePluginAttributes::ID_useExtents:
            if (atts->GetUseExtents() == false)
            {
                // Not using full extents.  Prompt for the dimensions.
                startX->setEnabled(true);
                startXLabel->setEnabled(true);
                endX->setEnabled(true);
                endXLabel->setEnabled(true);
                startY->setEnabled(true);
                startYLabel->setEnabled(true);
                endY->setEnabled(true);
                endYLabel->setEnabled(true);
                if (atts->GetIs3D() == true)
                {
                    startZ->setEnabled(true);
                    startZLabel->setEnabled(true);
                    endZ->setEnabled(true);
                    endZLabel->setEnabled(true);
                }
            }
            else
            {
                // Using full extents.
                startX->setEnabled(false);
                startXLabel->setEnabled(false);
                endX->setEnabled(false);
                endXLabel->setEnabled(false);
                startY->setEnabled(false);
                startYLabel->setEnabled(false);
                endY->setEnabled(false);
                endYLabel->setEnabled(false);
                startZ->setEnabled(false);
                startZLabel->setEnabled(false);
                endZ->setEnabled(false);
                endZLabel->setEnabled(false);
            }
            useExtents->blockSignals(true);
            useExtents->setChecked(atts->GetUseExtents());
            useExtents->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_startX:
            startX->blockSignals(true);
            temp.setNum(atts->GetStartX());
            startX->setText(temp);
            startX->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_endX:
            endX->blockSignals(true);
            temp.setNum(atts->GetEndX());
            endX->setText(temp);
            endX->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_samplesX:
            samplesX->blockSignals(true);
            temp.sprintf("%d", atts->GetSamplesX());
            samplesX->setText(temp);
            samplesX->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_startY:
            startY->blockSignals(true);
            temp.setNum(atts->GetStartY());
            startY->setText(temp);
            startY->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_endY:
            endY->blockSignals(true);
            temp.setNum(atts->GetEndY());
            endY->setText(temp);
            endY->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_samplesY:
            samplesY->blockSignals(true);
            temp.sprintf("%d", atts->GetSamplesY());
            samplesY->setText(temp);
            samplesY->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_is3D:
            if (atts->GetIs3D() == true)
            {
                samplesZ->setEnabled(true);
                samplesZLabel->setEnabled(true);
                if (atts->GetUseExtents() == false)
                {
                    startZ->setEnabled(true);
                    startZLabel->setEnabled(true);
                    endZ->setEnabled(true);
                    endZLabel->setEnabled(true);
                }
                else
                {
                    startZ->setEnabled(false);
                    startZLabel->setEnabled(false);
                    endZ->setEnabled(false);
                    endZLabel->setEnabled(false);
                }
            }
            else
            {
                startZ->setEnabled(false);
                startZLabel->setEnabled(false);
                endZ->setEnabled(false);
                endZLabel->setEnabled(false);
                samplesZ->setEnabled(false);
                samplesZLabel->setEnabled(false);
            }
            is3D->blockSignals(true);
            is3D->setChecked(atts->GetIs3D());
            is3D->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_startZ:
            startZ->blockSignals(true);
            temp.setNum(atts->GetStartZ());
            startZ->setText(temp);
            startZ->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_endZ:
            endZ->blockSignals(true);
            temp.setNum(atts->GetEndZ());
            endZ->setText(temp);
            endZ->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_samplesZ:
            samplesZ->blockSignals(true);
            temp.sprintf("%d", atts->GetSamplesZ());
            samplesZ->setText(temp);
            samplesZ->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_tieResolver:
            if (atts->GetTieResolver() == ResamplePluginAttributes::smallest || atts->GetTieResolver() == ResamplePluginAttributes::largest)
            {
                tieResolverVariable->setEnabled(true);
                if(tieResolverVariableLabel)
                    tieResolverVariableLabel->setEnabled(true);
            }
            else
            {
                tieResolverVariable->setEnabled(false);
                if(tieResolverVariableLabel)
                    tieResolverVariableLabel->setEnabled(false);
            }
            tieResolver->blockSignals(true);
            tieResolver->setButton(atts->GetTieResolver());
            tieResolver->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_tieResolverVariable:
            tieResolverVariable->blockSignals(true);
            tieResolverVariable->setText(atts->GetTieResolverVariable().c_str());
            tieResolverVariable->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_defaultValue:
            defaultValue->blockSignals(true);
            temp.setNum(atts->GetDefaultValue());
            defaultValue->setText(temp);
            defaultValue->blockSignals(false);
            break;
          case ResamplePluginAttributes::ID_distributedResample:
            distributedResample->blockSignals(true);
            distributedResample->setChecked(atts->GetDistributedResample());
            distributedResample->blockSignals(false);
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
// Creation:   Wed Jan 2 13:43:29 PST 2008
//
// Modifications:
//   Sean Ahern, Fri Jan  4 16:38:06 EST 2008
//   Prevented the text widgets from setting themselves (thus turning off
//   useExtents) if useExtents is set.
//
//   Brad Whitlock, Thu Apr 24 16:44:40 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisResampleWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do startX
    if (atts->GetUseExtents() == false)
    {
        if(which_widget == ResamplePluginAttributes::ID_startX || doAll)
        {
            temp = startX->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetStartX(val);
            }

            if(!okay)
            {
                msg = tr("The value of startX was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetStartX());
                Message(msg);
                atts->SetStartX(atts->GetStartX());
            }
        }

        // Do endX
        if(which_widget == ResamplePluginAttributes::ID_endX || doAll)
        {
            temp = endX->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetEndX(val);
            }

            if(!okay)
            {
                msg = tr("The value of endX was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetEndX());
                Message(msg);
                atts->SetEndX(atts->GetEndX());
            }
        }
    }

    // Do samplesX
    if(which_widget == ResamplePluginAttributes::ID_samplesX || doAll)
    {
        temp = samplesX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetSamplesX(val);
        }

        if(!okay)
        {
            msg = tr("The value of samplesX was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetSamplesX());
            Message(msg);
            atts->SetSamplesX(atts->GetSamplesX());
        }
    }

    // Do startY
    if (atts->GetUseExtents() == false)
    {
        if(which_widget == ResamplePluginAttributes::ID_startY || doAll)
        {
            temp = startY->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetStartY(val);
            }

            if(!okay)
            {
                msg = tr("The value of startY was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetStartY());
                Message(msg);
                atts->SetStartY(atts->GetStartY());
            }
        }

        // Do endY
        if(which_widget == ResamplePluginAttributes::ID_endY || doAll)
        {
            temp = endY->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetEndY(val);
            }

            if(!okay)
            {
                msg = tr("The value of endY was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetEndY());
                Message(msg);
                atts->SetEndY(atts->GetEndY());
            }
        }
    }

    // Do samplesY
    if(which_widget == ResamplePluginAttributes::ID_samplesY || doAll)
    {
        temp = samplesY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetSamplesY(val);
        }

        if(!okay)
        {
            msg = tr("The value of samplesY was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetSamplesY());
            Message(msg);
            atts->SetSamplesY(atts->GetSamplesY());
        }
    }

    // Do startZ
    if (atts->GetUseExtents() == false)
    {
        if(which_widget == ResamplePluginAttributes::ID_startZ || doAll)
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
                msg = tr("The value of startZ was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetStartZ());
                Message(msg);
                atts->SetStartZ(atts->GetStartZ());
            }
        }

        // Do endZ
        if(which_widget == ResamplePluginAttributes::ID_endZ || doAll)
        {
            temp = endZ->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                if(okay)
                    atts->SetEndZ(val);
            }

            if(!okay)
            {
                msg = tr("The value of endZ was invalid. "
                         "Resetting to the last good value of %1.").
                      arg(atts->GetEndZ());
                Message(msg);
                atts->SetEndZ(atts->GetEndZ());
            }
        }
    }

    // Do samplesZ
    if(which_widget == ResamplePluginAttributes::ID_samplesZ || doAll)
    {
        temp = samplesZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetSamplesZ(val);
        }

        if(!okay)
        {
            msg = tr("The value of samplesZ was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetSamplesZ());
            Message(msg);
            atts->SetSamplesZ(atts->GetSamplesZ());
        }
    }

    // Do defaultValue
    if(which_widget == ResamplePluginAttributes::ID_defaultValue || doAll)
    {
        temp = defaultValue->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetDefaultValue(val);
        }

        if(!okay)
        {
            msg = tr("The value of defaultValue was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetDefaultValue());
            Message(msg);
            atts->SetDefaultValue(atts->GetDefaultValue());
        }
    }

}


//
// Qt Slot functions
//


void
QvisResampleWindow::useExtentsChanged(bool val)
{
    atts->SetUseExtents(val);
    Apply();
}


void
QvisResampleWindow::startXProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_startX);
    Apply();
}


void
QvisResampleWindow::endXProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_endX);
    Apply();
}


void
QvisResampleWindow::samplesXProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_samplesX);
    Apply();
}


void
QvisResampleWindow::startYProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_startY);
    Apply();
}


void
QvisResampleWindow::endYProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_endY);
    Apply();
}


void
QvisResampleWindow::samplesYProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_samplesY);
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
    GetCurrentValues(ResamplePluginAttributes::ID_startZ);
    Apply();
}


void
QvisResampleWindow::endZProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_endZ);
    Apply();
}


void
QvisResampleWindow::samplesZProcessText()
{
    GetCurrentValues(ResamplePluginAttributes::ID_samplesZ);
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
    GetCurrentValues(ResamplePluginAttributes::ID_defaultValue);
    Apply();
}


void
QvisResampleWindow::distributedResampleChanged(bool val)
{
    atts->SetDistributedResample(val);
    SetUpdate(false);
    Apply();
}


