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

#include "QvisLineoutWindow.h"

#include <LineoutAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QNarrowLineEdit.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisLineoutWindow::QvisLineoutWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisLineoutWindow::QvisLineoutWindow(const int type,
                         LineoutAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisLineoutWindow::~QvisLineoutWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisLineoutWindow::~QvisLineoutWindow()
{
}


// ****************************************************************************
// Method: QvisLineoutWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Dec 21 11:53:09 PDT 2004
//   Added code to support Qt pre-3.2.
//
//   Brad Whitlock, Fri Apr 25 09:00:46 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Tue Aug 19 11:37:56 PDT 2008
//   Qt4 Port. (Removed pre Qt 3.2 logic)
//
// ****************************************************************************

void
QvisLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    //
    // Point1
    //
    point1 = new QLineEdit(central);
    connect(point1, SIGNAL(returnPressed()),
            this, SLOT(point1ProcessText()));
    mainLayout->addWidget(new QLabel(tr("Point 1"), central), 0,0);
    mainLayout->addWidget(point1, 0,1);

    //
    // Point2
    //
    point2 = new QLineEdit(central);
    connect(point2, SIGNAL(returnPressed()),
            this, SLOT(point2ProcessText()));
    mainLayout->addWidget(new QLabel(tr("Point 2"), central), 1,0);
    mainLayout->addWidget(point2, 1,1);

    //
    // Interactive
    //
    interactive = new QCheckBox(tr("Interactive"), central);
    connect(interactive, SIGNAL(toggled(bool)),
            this, SLOT(interactiveChanged(bool)));
    mainLayout->addWidget(interactive, 2,0,1,2);

    //
    // IgnoreGlobal
    //
    QGroupBox *globalGroup;

    ignoreGlobal = new QGroupBox(tr("Override Global Lineout Settings"),
                                 central);
    ignoreGlobal->setCheckable(true);
    globalGroup = ignoreGlobal;
    connect(ignoreGlobal, SIGNAL(toggled(bool)),
            this, SLOT(ignoreGlobalChanged(bool)));
    topLayout->addWidget(ignoreGlobal);

    QVBoxLayout *blayout = new QVBoxLayout(globalGroup);
    QGridLayout *qgrid  = new QGridLayout();
    blayout->addLayout(qgrid);

    //
    // SamplingOn
    //
    samplingOn = new QCheckBox(tr("Use Sampling"), globalGroup);
    connect(samplingOn, SIGNAL(toggled(bool)),
            this, SLOT(samplingOnChanged(bool)));
    qgrid->addWidget(samplingOn, 1,0);

    //
    // NumberOfSamplePoints
    //
    numberOfSamplePointsLabel = new QLabel(tr("Samples"),  globalGroup);
    numberOfSamplePointsLabel->setAlignment(Qt::AlignCenter);
    qgrid->addWidget(numberOfSamplePointsLabel,2,0);
    numberOfSamplePoints = new QNarrowLineEdit(globalGroup);
    connect(numberOfSamplePoints, SIGNAL(returnPressed()),
            this, SLOT(numberOfSamplePointsProcessText()));
    qgrid->addWidget(numberOfSamplePoints, 2,1);

    //
    // ReflineLabels
    //
    reflineLabels = new QCheckBox(tr("Refline Labels"), globalGroup);
    connect(reflineLabels, SIGNAL(toggled(bool)),
            this, SLOT(reflineLabelsChanged(bool)));
    qgrid->addWidget(reflineLabels, 3,0);
}


// ****************************************************************************
// Method: QvisLineoutWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Dec 21 11:51:46 PDT 2004
//   Added code to block signals and also some code to support Qt pre-3.2.
//
//   Cyrus Harrison, Tue Aug 19 11:37:56 PDT 2008
//   Qt4 Port. (Removed pre Qt 3.2 logic)
//
// ****************************************************************************

void
QvisLineoutWindow::UpdateWindow(bool doAll)
{
    bool flag;
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
          case LineoutAttributes::ID_point1:
            point1->setText(DoublesToQString(atts->GetPoint1(),3));
            break;
          case LineoutAttributes::ID_point2:
            point2->setText(DoublesToQString(atts->GetPoint2(),3));
            break;
          case LineoutAttributes::ID_interactive:
            interactive->blockSignals(true);
            interactive->setChecked(atts->GetInteractive());
            interactive->blockSignals(false);
            break;
          case LineoutAttributes::ID_ignoreGlobal: 
            ignoreGlobal->blockSignals(true);
            ignoreGlobal->setChecked(atts->GetIgnoreGlobal());
            ignoreGlobal->blockSignals(false);
            break;
          case LineoutAttributes::ID_samplingOn:
            flag = atts->GetSamplingOn();
            numberOfSamplePoints->setEnabled(flag);
            numberOfSamplePointsLabel->setEnabled(flag);
            samplingOn->setChecked(flag);
            break;
          case LineoutAttributes::ID_numberOfSamplePoints:
            numberOfSamplePoints->setText(IntToQString(atts->GetNumberOfSamplePoints()));
            break;
          case LineoutAttributes::ID_reflineLabels:
            reflineLabels->blockSignals(true);
            reflineLabels->setChecked(atts->GetReflineLabels());
            reflineLabels->blockSignals(false);
            break;
        }
    }

}


// ****************************************************************************
// Method: QvisLineoutWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   Cyrus Harrison, Tue Aug 19 11:37:56 PDT 2008
//   Qt4 Port. (Removed pre Qt 3.2 logic)
//
// ****************************************************************************

void
QvisLineoutWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do point1
    if(which_widget == LineoutAttributes::ID_point1 || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(point1, val, 3))
            atts->SetPoint1(val);
        else if(LineEditGetDoubles(point1, val, 2))
        {
            val[2] = 0.0;
            atts->SetPoint1(val);
        }
        else
        {
            ResettingError(tr("Point 1"),
                DoublesToQString(atts->GetPoint1(),3));
            atts->SetPoint1(atts->GetPoint1());
        }
    }

    // Do point2
    if(which_widget == LineoutAttributes::ID_point2 || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(point2, val, 3))
            atts->SetPoint2(val);
        else if(LineEditGetDoubles(point2, val, 2))
        {
            val[2] = 0.0;
            atts->SetPoint2(val);
        }
        else
        {
            ResettingError(tr("Point 2"),
                DoublesToQString(atts->GetPoint2(),3));
            atts->SetPoint2(atts->GetPoint2());
        }
    }

    // Do numberOfSamplePoints
    if(which_widget == LineoutAttributes::ID_numberOfSamplePoints || doAll)
    {
        int val;
        if(LineEditGetInt(numberOfSamplePoints, val))
            atts->SetNumberOfSamplePoints(val);
        else
        {
            ResettingError(tr("Number of Sample Points "),
                IntToQString(atts->GetNumberOfSamplePoints()));
            atts->SetNumberOfSamplePoints(atts->GetNumberOfSamplePoints());
        }
    }

}


//
// Qt Slot functions
//


void
QvisLineoutWindow::point1ProcessText()
{
    GetCurrentValues(LineoutAttributes::ID_point1);
    Apply();
}


void
QvisLineoutWindow::point2ProcessText()
{
    GetCurrentValues(LineoutAttributes::ID_point2);
    Apply();
}


void
QvisLineoutWindow::interactiveChanged(bool val)
{
    atts->SetInteractive(val);
    Apply();
}


void
QvisLineoutWindow::ignoreGlobalChanged(bool val)
{
    atts->SetIgnoreGlobal(val);
    Apply();
}


void
QvisLineoutWindow::samplingOnChanged(bool val)
{
    atts->SetSamplingOn(val);
    Apply();
}


void
QvisLineoutWindow::numberOfSamplePointsProcessText()
{
    GetCurrentValues(LineoutAttributes::ID_numberOfSamplePoints);
    Apply();
}


void
QvisLineoutWindow::reflineLabelsChanged(bool val)
{
    atts->SetReflineLabels(val);
    Apply();
}


