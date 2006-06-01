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

#include "QvisCracksClipperWindow.h"

#include <CracksClipperAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisCracksClipperWindow::QvisCracksClipperWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisCracksClipperWindow::QvisCracksClipperWindow(const int type,
                         CracksClipperAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisCracksClipperWindow::~QvisCracksClipperWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisCracksClipperWindow::~QvisCracksClipperWindow()
{
}


// ****************************************************************************
// Method: QvisCracksClipperWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCracksClipperWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,4,10, "mainLayout");

    // Use Crack 1
    useCrack1 = new QCheckBox("Use Crack 1", central, "useCrack1");
    connect(useCrack1, SIGNAL(toggled(bool)),
        this, SLOT(useCrack1Changed(bool)));
    mainLayout->addWidget(useCrack1, 0, 0);

    // Crack 1 Variable
    mainLayout->addWidget(new QLabel("Crack 1 Variable", central, 
        "crack1VarLabel"), 1, 0);
    crack1Var = new QvisVariableButton(true, true, true,
        QvisVariableButton::Vectors, central, "crack1Var");
    connect(crack1Var, SIGNAL(activated(const QString &)),
        this, SLOT(crack1VarChanged(const QString &)));
    mainLayout->addMultiCellWidget(crack1Var, 1, 1, 1, 3);

    // Use Crack 2
    useCrack2 = new QCheckBox("Use Crack 2", central, "useCrack2");
    connect(useCrack2, SIGNAL(toggled(bool)),
        this, SLOT(useCrack2Changed(bool)));
    mainLayout->addWidget(useCrack2, 2, 0);

    // Crack 2 Variable
    mainLayout->addWidget(new QLabel("Crack 2 Variable", central, 
        "crack2VarLabel"), 3, 0);
    crack2Var = new QvisVariableButton(true, true, true,
        QvisVariableButton::Vectors, central, "crack2Var");
    connect(crack2Var, SIGNAL(activated(const QString &)),
        this, SLOT(crack2VarChanged(const QString &)));
    mainLayout->addMultiCellWidget(crack2Var, 3, 3, 1, 3);

    // Use Crack 3
    useCrack3 = new QCheckBox("Use Crack 3", central, "useCrack3");
    connect(useCrack3, SIGNAL(toggled(bool)),
        this, SLOT(useCrack3Changed(bool)));
    mainLayout->addWidget(useCrack3, 4, 0);

    // Crack 3 Variable
    mainLayout->addWidget(new QLabel("Crack 3 Variable", central, 
        "crack3VarLabel"), 5, 0);
    crack3Var = new QvisVariableButton(true, true, true,
        QvisVariableButton::Vectors, central, "crack3Var");
    connect(crack3Var, SIGNAL(activated(const QString &)),
        this, SLOT(crack3VarChanged(const QString &)));
    mainLayout->addMultiCellWidget(crack3Var, 5, 5, 1, 3);

    // Strain Variable
    mainLayout->addWidget(new QLabel("Strain Variable", central, 
        "strainVarLabel"), 6, 0);
    strainVar = new QvisVariableButton(true, true, true,
        QvisVariableButton::SymmetricTensors, central, "strainVar");
    connect(strainVar, SIGNAL(activated(const QString &)),
        this, SLOT(strainVarChanged(const QString &)));
    mainLayout->addMultiCellWidget(strainVar, 6, 6, 1, 3);
}


// ****************************************************************************
// Method: QvisCracksClipperWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCracksClipperWindow::UpdateWindow(bool doAll)
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
        switch(i)
        {
          case 0: //crack1Var
            temp = atts->GetCrack1Var().c_str();
            crack1Var->setText(temp);
            break;
          case 1: //crack2Var
            temp = atts->GetCrack2Var().c_str();
            crack2Var->setText(temp);
            break;
          case 2: //crack3Var
            temp = atts->GetCrack3Var().c_str();
            crack3Var->setText(temp);
            break;
          case 3: //strainVar
            temp = atts->GetStrainVar().c_str();
            strainVar->setText(temp);
            break;
          case 4: //useCrack1
            useCrack1->setChecked(atts->GetUseCrack1());
            break;
          case 5: //useCrack2
            useCrack2->setChecked(atts->GetUseCrack2());
            break;
          case 6: //useCrack3
            useCrack3->setChecked(atts->GetUseCrack3());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisCracksClipperWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCracksClipperWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do crack1Var
    if(which_widget == 0 || doAll)
    {
        // Nothing for crack1Var
    }

    // Do crack2Var
    if(which_widget == 1 || doAll)
    {
        // Nothing for crack2Var
    }

    // Do crack3Var
    if(which_widget == 2 || doAll)
    {
        // Nothing for crack3Var
    }

    // Do strainVar
    if(which_widget == 3 || doAll)
    {
        // Nothing for strainVar
    }

    // Do useCrack1
    if(which_widget == 4 || doAll)
    {
        // Nothing for useCrack1
    }

    // Do useCrack2
    if(which_widget == 5 || doAll)
    {
        // Nothing for useCrack2
    }

    // Do useCrack3
    if(which_widget == 6 || doAll)
    {
        // Nothing for useCrack3
    }

}


//
// Qt Slot functions
//


void
QvisCracksClipperWindow::crack1VarChanged(const QString &var)
{
    atts->SetCrack1Var(var.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisCracksClipperWindow::crack2VarChanged(const QString &var)
{
    atts->SetCrack2Var(var.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisCracksClipperWindow::crack3VarChanged(const QString &var)
{
    atts->SetCrack3Var(var.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisCracksClipperWindow::strainVarChanged(const QString &var)
{
    atts->SetStrainVar(var.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisCracksClipperWindow::useCrack1Changed(bool val)
{
    atts->SetUseCrack1(val);
    Apply();
}


void
QvisCracksClipperWindow::useCrack2Changed(bool val)
{
    atts->SetUseCrack2(val);
    Apply();
}


void
QvisCracksClipperWindow::useCrack3Changed(bool val)
{
    atts->SetUseCrack3(val);
    Apply();
}

