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

#include "QvisMergeWindow.h"

#include <MergeOperatorAttributes.h>
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
// Method: QvisMergeWindow::QvisMergeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Dec 17 11:54:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisMergeWindow::QvisMergeWindow(const int type,
                         MergeOperatorAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisMergeWindow::~QvisMergeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Dec 17 11:54:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisMergeWindow::~QvisMergeWindow()
{
}


// ****************************************************************************
// Method: QvisMergeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Dec 17 11:54:23 PDT 2007
//
// Modifications:
//   Brad Whitlock, Fri Apr 25 09:00:14 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisMergeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    parallelMerge = new QCheckBox(tr("Merge across all processors?"), central, "parallelMerge");
    connect(parallelMerge, SIGNAL(toggled(bool)),
            this, SLOT(parallelMergeChanged(bool)));
    mainLayout->addWidget(parallelMerge, 0,0);

    toleranceLabel = new QLabel(tr("Maximum distance between points that should be merged"), central, "toleranceLabel");
    mainLayout->addWidget(toleranceLabel,1,0);
    tolerance = new QLineEdit(central, "tolerance");
    connect(tolerance, SIGNAL(returnPressed()),
            this, SLOT(toleranceProcessText()));
    mainLayout->addWidget(tolerance, 1,1);

}


// ****************************************************************************
// Method: QvisMergeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Dec 17 11:54:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::UpdateWindow(bool doAll)
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
          case MergeOperatorAttributes::ID_parallelMerge:
            parallelMerge->blockSignals(true);
            parallelMerge->setChecked(atts->GetParallelMerge());
            parallelMerge->blockSignals(false);
            break;
          case MergeOperatorAttributes::ID_tolerance:
            tolerance->blockSignals(true);
            temp.setNum(atts->GetTolerance());
            tolerance->setText(temp);
            tolerance->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisMergeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Dec 17 11:54:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do tolerance
    if(which_widget == MergeOperatorAttributes::ID_tolerance || doAll)
    {
        temp = tolerance->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetTolerance(val);
        }

        if(!okay)
        {
            msg = tr("The value of tolerance was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetTolerance());
            Message(msg);
            atts->SetTolerance(atts->GetTolerance());
        }
    }

}


//
// Qt Slot functions
//


void
QvisMergeWindow::parallelMergeChanged(bool val)
{
    atts->SetParallelMerge(val);
    SetUpdate(false);
    Apply();
}


void
QvisMergeWindow::toleranceProcessText()
{
    GetCurrentValues(MergeOperatorAttributes::ID_tolerance);
    Apply();
}


