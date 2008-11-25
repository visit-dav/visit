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

#include "QvisGlobalLineoutWindow.h"

#include <GlobalLineoutAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QNarrowLineEdit.h>
#include <stdio.h>


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::QvisGlobalLineoutWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:47:34 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisGlobalLineoutWindow::QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                         QvisPostableWindowObserver::ApplyButton, false)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow()
{
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Dec 15 11:07:46 PDT 2004
//   I ifdef'd some code so it builds with Qt versions older than 3.2.
//
//   Kathleen Bonnell, Thu Feb  3 15:51:06 PST 2005 
//   Added curveOptions and colorOptions for Dynamic mode. 
//
//   Kathleen Bonnell, Mon Feb 27 12:36:41 PST 2006
//   Added more text to createWindow label, to clarify intent. 
//   
//   Kathleen Bonnell, Thu Nov  2 14:01:01 PST 2006 
//   Added freezInTime.
//   
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Wed Jun 11 13:19:35 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisGlobalLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);
    //
    // CreateWindow
    //
    createWindow = new QCheckBox(tr("Use 1st unused window or create\nnew one. All  subsequent lineouts\nwill use this same window."), 
                                  central);
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addWidget(createWindow, 0,0,1,2);

    //
    // WindowId
    //
    windowIdLabel = new QLabel(tr("Window #"), central);
    mainLayout->addWidget(windowIdLabel,1,0);
    windowId = new QNarrowLineEdit(central);
    connect(windowId, SIGNAL(returnPressed()),
            this, SLOT(windowIdProcessText()));
    mainLayout->addWidget(windowId, 1,1);


    // Freeze In Time
    freezeInTime = new QCheckBox(tr("Freeze In Time"), central);
    connect(freezeInTime, SIGNAL(toggled(bool)),
            this, SLOT(freezeInTimeChanged(bool)));
    mainLayout->addWidget(freezeInTime,2,0,1,2);

    //
    // Dynamic 
    //
    dynamic = new QGroupBox(tr("Synchronize with originating plot"),central);
    dynamic->setCheckable(true);
    connect(dynamic, SIGNAL(toggled(bool)),
            this, SLOT(dynamicChanged(bool)));
    topLayout->addWidget(dynamic);

    QVBoxLayout *dlayout = new QVBoxLayout(dynamic);
    dlayout->setMargin(10);
    dlayout->addSpacing(15);

    QGridLayout *dgrid = new QGridLayout();
    dlayout->addLayout(dgrid);

    //
    // curve options
    //
    curveOptions = new QComboBox(dynamic);
    curveOptions->addItem(tr("updates curve"));
    curveOptions->addItem(tr("creates new curve"));
    connect(curveOptions, SIGNAL(activated(int)),
            this, SLOT(curveOptionsChanged(int)));
    curveLabel = new QLabel(tr("Time change "), dynamic);

    dgrid->addWidget(curveLabel, 1, 0);
    dgrid->addWidget(curveOptions, 1, 1);
    dgrid->setRowMinimumHeight(2, 10);

    //
    // color options
    //
    colorOptions = new QComboBox(dynamic);
    colorOptions->addItem(tr("repeats color"));
    colorOptions->addItem(tr("creates new color"));
    connect(colorOptions, SIGNAL(activated(int)),
            this, SLOT(colorOptionsChanged(int)));
    colorLabel = new QLabel(tr("New curve "), dynamic);

    dgrid->addWidget(colorLabel, 3, 0);
    dgrid->addWidget(colorOptions, 3, 1);

    //
    // Want the next items grouped.
    //
    QGroupBox *gbox = new QGroupBox(central);
    topLayout->addWidget(gbox);

    QVBoxLayout *blayout = new QVBoxLayout(gbox);
    blayout->setMargin(5);

    QGridLayout *qgrid = new QGridLayout();
    blayout->addLayout(qgrid);
    qgrid->setMargin(5);
    
    QLabel *msg = new QLabel(gbox); 
    msg->setText(tr("These items can be overridden\nby Lineout Operator"));
    msg->setAlignment(Qt::AlignCenter);
    qgrid->addWidget(msg, 0,0,1,2);
    qgrid->setRowMinimumHeight(1,10);

    //
    // SamplingOn
    //
    samplingOn = new QCheckBox(tr("Use Sampling"), gbox);
    connect(samplingOn, SIGNAL(toggled(bool)),
            this, SLOT(samplingOnChanged(bool)));
    qgrid->addWidget(samplingOn, 2,0);

    //
    // NumSamples
    //
    numSamplesLabel = new QLabel(tr("Sample Points "),gbox);
    numSamplesLabel->setAlignment(Qt::AlignCenter);
    qgrid->addWidget(numSamplesLabel,3,0);

    numSamples = new QNarrowLineEdit(gbox);
    connect(numSamples, SIGNAL(returnPressed()),
            this, SLOT(numSamplesProcessText()));
    qgrid->addWidget(numSamples, 3,1);

    //
    // ReflineLabels
    //
    createReflineLabels = new QCheckBox(tr("Create refline labels"),gbox);
    connect(createReflineLabels, SIGNAL(toggled(bool)),
            this, SLOT(createReflineLabelsChanged(bool)));
    qgrid->addWidget(createReflineLabels, 4,0);
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Feb  3 15:51:06 PST 2005 
//   Added curveOptions and colorOptions for Dynamic mode. 
//
//   Kathleen Bonnell, Thu Nov  2 14:01:01 PST 2006 
//   Added freezInTime.
//
//   Brad Whitlock, Fri Dec 14 17:22:35 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Wed Jun 11 13:19:35 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisGlobalLineoutWindow::UpdateWindow(bool doAll)
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
          case GlobalLineoutAttributes::ID_Dynamic:
            dynamic->setChecked(atts->GetDynamic());
            if (atts->GetDynamic())
            {
                freezeInTime->setChecked(false); 
            }
            freezeInTime->setEnabled(!atts->GetDynamic()); 
            curveOptions->setEnabled(atts->GetDynamic()) ;
            curveLabel->setEnabled(atts->GetDynamic()) ;
            colorOptions->setEnabled(atts->GetDynamic() && 
                atts->GetCurveOption() == GlobalLineoutAttributes::CreateCurve);
            colorLabel->setEnabled(atts->GetDynamic() && 
                atts->GetCurveOption() == GlobalLineoutAttributes::CreateCurve);
            break;
          case GlobalLineoutAttributes::ID_createWindow:
            if (atts->GetCreateWindow() == false)
            {
                windowId->setEnabled(true);
                windowIdLabel->setEnabled(true);
            }
            else
            {
                windowId->setEnabled(false);
                windowIdLabel->setEnabled(false);
            }
            createWindow->setChecked(atts->GetCreateWindow());
            break;
          case GlobalLineoutAttributes::ID_windowId:
            temp.sprintf("%d", atts->GetWindowId());
            windowId->setText(temp);
            break;
          case GlobalLineoutAttributes::ID_samplingOn:
            if (atts->GetSamplingOn() == true)
            {
                numSamples->setEnabled(true);
                numSamplesLabel->setEnabled(true);
            }
            else
            {
                numSamples->setEnabled(false);
                numSamplesLabel->setEnabled(false);
            }
            samplingOn->setChecked(atts->GetSamplingOn());
            break;
          case GlobalLineoutAttributes::ID_numSamples:
            temp.sprintf("%d", atts->GetNumSamples());
            numSamples->setText(temp);
            break;
          case GlobalLineoutAttributes::ID_createReflineLabels:
            createReflineLabels->setChecked(atts->GetCreateReflineLabels());
            break;
          case GlobalLineoutAttributes::ID_curveOption:
            curveOptions->setCurrentIndex(atts->GetCurveOption());
            curveOptions->setEnabled(atts->GetDynamic()) ;
            curveLabel->setEnabled(atts->GetDynamic()) ;
            colorOptions->setEnabled(atts->GetDynamic() && 
                atts->GetCurveOption() == GlobalLineoutAttributes::CreateCurve);
            colorLabel->setEnabled(atts->GetDynamic() && 
                atts->GetCurveOption() == GlobalLineoutAttributes::CreateCurve);
            break;
          case GlobalLineoutAttributes::ID_colorOption:
            colorOptions->setCurrentIndex(atts->GetColorOption());
            break;
          case GlobalLineoutAttributes::ID_freezeInTime:
            freezeInTime->setChecked(atts->GetFreezeInTime());
            if (atts->GetFreezeInTime())
                dynamic->setChecked(false);
            dynamic->setEnabled(!atts->GetFreezeInTime());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do Dynamic
    if(which_widget == 0 || doAll)
    {
        // Nothing for Dynamic
    }

    // Do createWindow
    if(which_widget == 1 || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == 2 || doAll)
    {
        temp = windowId->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetWindowId(val);
        }

        if(!okay)
        {
            msg = tr("The value of windowId was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetWindowId());
            Message(msg);
            atts->SetWindowId(atts->GetWindowId());
        }
    }

    // Do samplingOn
    if(which_widget == 3 || doAll)
    {
        // Nothing for samplingOn
    }

    // Do numSamples
    if(which_widget == 4 || doAll)
    {
        temp = numSamples->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetNumSamples(val);
        }

        if(!okay)
        {
            msg = tr("The value of numSamples was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetNumSamples());
            Message(msg);
            atts->SetNumSamples(atts->GetNumSamples());
        }
    }

    // Do createReflineLabels
    if(which_widget == 5 || doAll)
    {
        // Nothing for createReflineLabels
    }

}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetGlobalLineoutAttributes();
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::apply()
{
    Apply(true);
}


void
QvisGlobalLineoutWindow::dynamicChanged(bool val)
{
    atts->SetDynamic(val);
    Apply();
}


void
QvisGlobalLineoutWindow::createWindowChanged(bool val)
{
    atts->SetCreateWindow(val);
    Apply();
}


void
QvisGlobalLineoutWindow::windowIdProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisGlobalLineoutWindow::samplingOnChanged(bool val)
{
    atts->SetSamplingOn(val);
    Apply();
}


void
QvisGlobalLineoutWindow::numSamplesProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisGlobalLineoutWindow::createReflineLabelsChanged(bool val)
{
    atts->SetCreateReflineLabels(val);
    Apply();
}


void
QvisGlobalLineoutWindow::curveOptionsChanged(int mode)
{
    atts->SetCurveOption(GlobalLineoutAttributes::CurveOptions(mode));
    Apply();
}


void
QvisGlobalLineoutWindow::colorOptionsChanged(int mode)
{
    atts->SetColorOption(GlobalLineoutAttributes::ColorOptions(mode));
    Apply();
}

void
QvisGlobalLineoutWindow::freezeInTimeChanged(bool val)
{
    atts->SetFreezeInTime(val);
    Apply();
}
