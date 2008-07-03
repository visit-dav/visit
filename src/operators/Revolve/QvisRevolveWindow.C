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

#include "QvisRevolveWindow.h"

#include <RevolveAttributes.h>
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
// Method: QvisRevolveWindow::QvisRevolveWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Sun Mar 18 10:37:59 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisRevolveWindow::QvisRevolveWindow(const int type,
                         RevolveAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisRevolveWindow::~QvisRevolveWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Sun Mar 18 10:37:59 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisRevolveWindow::~QvisRevolveWindow()
{
}


// ****************************************************************************
// Method: QvisRevolveWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Sun Mar 18 10:37:59 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 16:40:16 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisRevolveWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,2,  10, "mainLayout");


    meshTypeLabel = new QLabel(tr("Type of Mesh?"), central, "meshTypeLabel");
    mainLayout->addWidget(meshTypeLabel,0,0);
    meshType = new QButtonGroup(central, "meshType");
    meshType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *meshTypeLayout = new QHBoxLayout(meshType);
    meshTypeLayout->setSpacing(10);
    QRadioButton *meshTypeMeshTypeAuto = new QRadioButton(tr("Auto"), meshType);
    meshTypeLayout->addWidget(meshTypeMeshTypeAuto);
    QRadioButton *meshTypeMeshTypeXY = new QRadioButton(tr("XY"), meshType);
    meshTypeLayout->addWidget(meshTypeMeshTypeXY);
    QRadioButton *meshTypeMeshTypeRZ = new QRadioButton(tr("RZ"), meshType);
    meshTypeLayout->addWidget(meshTypeMeshTypeRZ);
    QRadioButton *meshTypeMeshTypeZR = new QRadioButton(tr("ZR"), meshType);
    meshTypeLayout->addWidget(meshTypeMeshTypeZR);
    connect(meshType, SIGNAL(clicked(int)),
            this, SLOT(meshTypeChanged(int)));
    mainLayout->addWidget(meshType, 0,1);

    autoAxisLabel = NULL;
    autoAxis = new QCheckBox(tr("Choose axis based on mesh type?"), central, "autoAxis");
    connect(autoAxis, SIGNAL(toggled(bool)),
            this, SLOT(autoAxisChanged(bool)));
    mainLayout->addWidget(autoAxis, 1,0);

    axisLabel = new QLabel(tr("Axis of revolution"), central, "axisLabel");
    mainLayout->addWidget(axisLabel,2,0);
    axis = new QLineEdit(central, "axis");
    connect(axis, SIGNAL(returnPressed()),
            this, SLOT(axisProcessText()));
    mainLayout->addWidget(axis, 2,1);

    startAngleLabel = new QLabel(tr("Start angle"), central, "startAngleLabel");
    mainLayout->addWidget(startAngleLabel,3,0);
    startAngle = new QLineEdit(central, "startAngle");
    connect(startAngle, SIGNAL(returnPressed()),
            this, SLOT(startAngleProcessText()));
    mainLayout->addWidget(startAngle, 3,1);

    stopAngleLabel = new QLabel(tr("Stop angle"), central, "stopAngleLabel");
    mainLayout->addWidget(stopAngleLabel,4,0);
    stopAngle = new QLineEdit(central, "stopAngle");
    connect(stopAngle, SIGNAL(returnPressed()),
            this, SLOT(stopAngleProcessText()));
    mainLayout->addWidget(stopAngle, 4,1);

    stepsLabel = new QLabel(tr("Number of steps"), central, "stepsLabel");
    mainLayout->addWidget(stepsLabel,5,0);
    steps = new QLineEdit(central, "steps");
    connect(steps, SIGNAL(returnPressed()),
            this, SLOT(stepsProcessText()));
    mainLayout->addWidget(steps, 5,1);

}


// ****************************************************************************
// Method: QvisRevolveWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sun Mar 18 10:37:59 PDT 2007
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 11:52:41 PDT 2008
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisRevolveWindow::UpdateWindow(bool doAll)
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

        const double         *dptr;
        switch(i)
        {
          case 0: //meshType
            meshType->blockSignals(true);
            meshType->setButton(atts->GetMeshType());
            meshType->blockSignals(false);
            break;
          case 1: //autoAxis
            if (atts->GetAutoAxis() == false)
            {
                axis->setEnabled(true);
                if(axisLabel)
                    axisLabel->setEnabled(true);
            }
            else
            {
                axis->setEnabled(false);
                if(axisLabel)
                    axisLabel->setEnabled(false);
            }
            autoAxis->blockSignals(true);
            autoAxis->setChecked(atts->GetAutoAxis());
            autoAxis->blockSignals(false);
            break;
          case 2: //axis
            dptr = atts->GetAxis();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            axis->blockSignals(true);
            axis->setText(temp);
            axis->blockSignals(false);
            break;
          case 3: //startAngle
            startAngle->blockSignals(true);
            temp.setNum(atts->GetStartAngle());
            startAngle->setText(temp);
            startAngle->blockSignals(false);
            break;
          case 4: //stopAngle
            stopAngle->blockSignals(true);
            temp.setNum(atts->GetStopAngle());
            stopAngle->setText(temp);
            stopAngle->blockSignals(false);
            break;
          case 5: //steps
            steps->blockSignals(true);
            temp.sprintf("%d", atts->GetSteps());
            steps->setText(temp);
            steps->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisRevolveWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Sun Mar 18 10:37:59 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisRevolveWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do meshType
    if(which_widget == 0 || doAll)
    {
        // Nothing for meshType
    }

    // Do autoAxis
    if(which_widget == 1 || doAll)
    {
        // Nothing for autoAxis
    }

    // Do axis
    if(which_widget == 2 || doAll)
    {
        temp = axis->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2])==3;
            if(okay)
                atts->SetAxis(val);
        }

        if(!okay)
        {
            const double *val = atts->GetAxis();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of axis was invalid. "
                     "Resetting to the last good value of %1.").arg(num); 
            Message(msg);
            atts->SetAxis(atts->GetAxis());
        }
    }

    // Do startAngle
    if(which_widget == 3 || doAll)
    {
        temp = startAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetStartAngle(val);
        }

        if(!okay)
        {
            msg = tr("The value of startAngle was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetStartAngle());
            Message(msg);
            atts->SetStartAngle(atts->GetStartAngle());
        }
    }

    // Do stopAngle
    if(which_widget == 4 || doAll)
    {
        temp = stopAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStopAngle(val);
        }

        if(!okay)
        {
            msg = tr("The value of stopAngle was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetStopAngle());
            Message(msg);
            atts->SetStopAngle(atts->GetStopAngle());
        }
    }

    // Do steps
    if(which_widget == 5 || doAll)
    {
        temp = steps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetSteps(val);
        }

        if(!okay)
        {
            msg = tr("The value of steps was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetSteps());
            Message(msg);
            atts->SetSteps(atts->GetSteps());
        }
    }

}


//
// Qt Slot functions
//


void
QvisRevolveWindow::meshTypeChanged(int val)
{
    if(val != atts->GetMeshType())
    {
        atts->SetMeshType(RevolveAttributes::MeshType(val));
        SetUpdate(false);
        Apply();
    }
}


void
QvisRevolveWindow::autoAxisChanged(bool val)
{
    atts->SetAutoAxis(val);
    Apply();
}


void
QvisRevolveWindow::axisProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisRevolveWindow::startAngleProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisRevolveWindow::stopAngleProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisRevolveWindow::stepsProcessText()
{
    GetCurrentValues(5);
    Apply();
}


