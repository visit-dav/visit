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

#include "QvisThresholdWindow.h"

#include <ThresholdAttributes.h>
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
// Method: QvisThresholdWindow::QvisThresholdWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisThresholdWindow::QvisThresholdWindow(const int type,
                         ThresholdAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThresholdWindow::~QvisThresholdWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisThresholdWindow::~QvisThresholdWindow()
{
}


// ****************************************************************************
// Method: QvisThresholdWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 10 09:40:53 PDT 2004
//   Changed so it uses a variable button. I also improved the widget spacing.
//
//   Hank Childs, Tue Sep 13 09:25:35 PDT 2005
//   Add support for "PointsOnly".
//
// ****************************************************************************

void
QvisThresholdWindow::CreateWindowContents()
{
    QGroupBox *rangeBox = new QGroupBox(central, "rangeBox");
    rangeBox->setTitle("Threshold Range");
    topLayout->addWidget(rangeBox);

    QGridLayout *mainLayout = new QGridLayout(rangeBox, 4,2);
    mainLayout->setMargin(10);
    mainLayout->setSpacing(5);
    mainLayout->addRowSpacing(0, 10);
    mainLayout->addWidget(new QLabel("Lower bound", rangeBox, "lboundLabel"),1,0);
    lbound = new QLineEdit(rangeBox, "lbound");
    connect(lbound, SIGNAL(returnPressed()),
            this, SLOT(lboundProcessText()));
    mainLayout->addMultiCellWidget(lbound, 1,1, 1,1);

    mainLayout->addWidget(new QLabel("Upper bound", rangeBox, "uboundLabel"),2,0);
    ubound = new QLineEdit(rangeBox, "ubound");
    connect(ubound, SIGNAL(returnPressed()),
            this, SLOT(uboundProcessText()));
    mainLayout->addMultiCellWidget(ubound, 2, 2, 1, 1);

    mainLayout->addWidget(new QLabel("Variable", rangeBox, "variableLabel"),3,0);
    variable = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars, rangeBox, "variable");
    connect(variable, SIGNAL(activated(const QString &)),
            this, SLOT(variableChanged(const QString &)));
    mainLayout->addMultiCellWidget(variable, 3,3, 1, 1);

    QGroupBox *nodalBox = new QGroupBox(central, "nodalBox");
    nodalBox->setTitle("Nodal Quantities Only");
    topLayout->addWidget(nodalBox);

    QGridLayout *nodalLayout = new QGridLayout(nodalBox, 3,3);
    nodalLayout->setMargin(10);
    nodalLayout->setSpacing(5);
    nodalLayout->addRowSpacing(0, 10);

    nodalLayout->addMultiCellWidget(new QLabel("Output mesh is ",
        nodalBox, "meshType"),1,1,0,0);
    meshType = new QButtonGroup(nodalBox, "meshType");
    meshType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *meshTypeLayout = new QHBoxLayout(meshType);
    meshTypeLayout->setSpacing(10);
    QRadioButton *meshTypeCells = new QRadioButton("Cells from input", meshType);
    meshTypeLayout->addWidget(meshTypeCells);
    QRadioButton *meshTypePoints = new QRadioButton("Point mesh", meshType);
    meshTypeLayout->addWidget(meshTypePoints);
    connect(meshType, SIGNAL(clicked(int)),
            this, SLOT(meshTypeChanged(int)));
    nodalLayout->addMultiCellWidget(meshType, 1,1, 1,2);

    amountLabel = new QLabel("Nodes in range ", nodalBox, "meshType");
    nodalLayout->addMultiCellWidget(amountLabel, 2,2,0,0);
    amount = new QButtonGroup(nodalBox, "amount");
    amount->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *amountLayout = new QHBoxLayout(amount);
    amountLayout->setSpacing(10);
    QRadioButton *amountAll = new QRadioButton("All", amount);
    amountLayout->addWidget(amountAll);
    QRadioButton *amountOne = new QRadioButton("At least one", amount);
    amountLayout->addWidget(amountOne);
    connect(amount, SIGNAL(clicked(int)),
            this, SLOT(amountChanged(int)));
    nodalLayout->addMultiCellWidget(amount, 2,2, 1,2);
}


// ****************************************************************************
// Method: QvisThresholdWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//   
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Hank Childs, Thu Sep 15 15:31:34 PDT 2005
//   Add support for meshType.
//
// ****************************************************************************

void
QvisThresholdWindow::UpdateWindow(bool doAll)
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
          case 0: //amount
            if (atts->GetAmount() == 2)
            {
                amountLabel->setEnabled(false);
                amount->setEnabled(false);
                meshType->setButton(1);
            }
            else
            {
                amountLabel->setEnabled(true);
                amount->setEnabled(true);
                meshType->setButton(0);
                if (atts->GetAmount() == ThresholdAttributes::All)
                    amount->setButton(0);
                else if (atts->GetAmount() == ThresholdAttributes::Some)
                    amount->setButton(1);
            }
            break;
          case 1: //lbound
            if (atts->GetLbound() == -1e+37)
                temp = "min";
            else
                temp.setNum(atts->GetLbound());
            lbound->setText(temp);
            break;
          case 2: //ubound
            if (atts->GetUbound() == +1e+37)
                temp = "max";
            else
                temp.setNum(atts->GetUbound());
            ubound->setText(temp);
            break;
          case 3: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//
//   Jeremy Meredith, Wed Mar  3 16:02:43 PST 2004
//   Fixed a type with using "min".
//
//   Brad Whitlock, Fri Dec 10 09:43:19 PDT 2004
//   Removed code to get the variable.
//
// ****************************************************************************

void
QvisThresholdWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do amount
    if(which_widget == 0 || doAll)
    {
        // Nothing for amount
    }

    // Do lbound
    if(which_widget == 1 || doAll)
    {
        temp = lbound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("min"))
            atts->SetLbound(-1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetLbound(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of lbound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetLbound());
                Message(msg);
                atts->SetLbound(atts->GetLbound());
            }
        }
    }

    // Do ubound
    if(which_widget == 2 || doAll)
    {
        temp = ubound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("max"))
            atts->SetUbound(1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetUbound(val);
            }
    
            if(!okay)
            {
                msg.sprintf("The value of ubound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetUbound());
                Message(msg);
                atts->SetUbound(atts->GetUbound());
            }
        }
    }
}


//
// Qt Slot functions
//


void
QvisThresholdWindow::amountChanged(int val)
{
    ThresholdAttributes::Amount newVal = (val == 0 
                                       ? ThresholdAttributes::All
                                       : ThresholdAttributes::Some);
    if(newVal != atts->GetAmount())
    {
        atts->SetAmount(newVal);
        Apply();
    }
}

void
QvisThresholdWindow::meshTypeChanged(int val)
{
    ThresholdAttributes::Amount newVal;
    if (val == 1)
        newVal = ThresholdAttributes::PointsOnly;
    else
    {
        int selectedId = amount->id(amount->selected());
        newVal = (selectedId == 0 ? ThresholdAttributes::All
                                  : ThresholdAttributes::Some);
    }

    if(newVal != atts->GetAmount())
    {
        atts->SetAmount(newVal);
        Apply();
    }
}


void
QvisThresholdWindow::lboundProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisThresholdWindow::uboundProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisThresholdWindow::variableChanged(const QString &var)
{
    atts->SetVariable(var.latin1());
    SetUpdate(false);
    Apply();
}


