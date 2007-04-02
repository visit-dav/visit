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

#include "QvisReplicateWindow.h"

#include <ReplicateAttributes.h>
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
// Method: QvisReplicateWindow::QvisReplicateWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Jun 23 15:54:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisReplicateWindow::QvisReplicateWindow(const int type,
                         ReplicateAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisReplicateWindow::~QvisReplicateWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Jun 23 15:54:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisReplicateWindow::~QvisReplicateWindow()
{
}


// ****************************************************************************
// Method: QvisReplicateWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Jun 23 15:54:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisReplicateWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 8,2,  10, "mainLayout");


    useUnitCellVectors = new QCheckBox("Use provided unit cell vectors", central, "useUnitCellVectors");
    connect(useUnitCellVectors, SIGNAL(toggled(bool)),
            this, SLOT(useUnitCellVectorsChanged(bool)));
    mainLayout->addWidget(useUnitCellVectors, 0,0);

    xVectorLabel = new QLabel("Vector for X", central, "xVectorLabel");
    mainLayout->addWidget(xVectorLabel,1,0);
    xVector = new QLineEdit(central, "xVector");
    connect(xVector, SIGNAL(returnPressed()),
            this, SLOT(xVectorProcessText()));
    mainLayout->addWidget(xVector, 1,1);

    yVectorLabel = new QLabel("Vector for Y", central, "yVectorLabel");
    mainLayout->addWidget(yVectorLabel,2,0);
    yVector = new QLineEdit(central, "yVector");
    connect(yVector, SIGNAL(returnPressed()),
            this, SLOT(yVectorProcessText()));
    mainLayout->addWidget(yVector, 2,1);

    zVectorLabel = new QLabel("Vector for Z", central, "zVectorLabel");
    mainLayout->addWidget(zVectorLabel,3,0);
    zVector = new QLineEdit(central, "zVector");
    connect(zVector, SIGNAL(returnPressed()),
            this, SLOT(zVectorProcessText()));
    mainLayout->addWidget(zVector, 3,1);

    xReplicationsLabel = new QLabel("Replications in X", central, "xReplicationsLabel");
    mainLayout->addWidget(xReplicationsLabel,4,0);
    xReplications = new QLineEdit(central, "xReplications");
    connect(xReplications, SIGNAL(returnPressed()),
            this, SLOT(xReplicationsProcessText()));
    mainLayout->addWidget(xReplications, 4,1);

    yReplicationsLabel = new QLabel("Replications in Y", central, "yReplicationsLabel");
    mainLayout->addWidget(yReplicationsLabel,5,0);
    yReplications = new QLineEdit(central, "yReplications");
    connect(yReplications, SIGNAL(returnPressed()),
            this, SLOT(yReplicationsProcessText()));
    mainLayout->addWidget(yReplications, 5,1);

    zReplicationsLabel = new QLabel("Replications in Z", central, "zReplicationsLabel");
    mainLayout->addWidget(zReplicationsLabel,6,0);
    zReplications = new QLineEdit(central, "zReplications");
    connect(zReplications, SIGNAL(returnPressed()),
            this, SLOT(zReplicationsProcessText()));
    mainLayout->addWidget(zReplications, 6,1);

    mergeResults = new QCheckBox("Merge into one block when possible", central, "mergeResults");
    connect(mergeResults, SIGNAL(toggled(bool)),
            this, SLOT(mergeResultsChanged(bool)));
    mainLayout->addWidget(mergeResults, 7,0);

}


// ****************************************************************************
// Method: QvisReplicateWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Jun 23 15:54:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisReplicateWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    xReplications->blockSignals(true);
    yReplications->blockSignals(true);
    zReplications->blockSignals(true);
    xVector->blockSignals(true);
    yVector->blockSignals(true);
    zVector->blockSignals(true);
    mergeResults->blockSignals(true);
    useUnitCellVectors->blockSignals(true);

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
          case 0: //useUnitCellVectors
            if (atts->GetUseUnitCellVectors() == false)
            {
                xVector->setEnabled(true);
                xVectorLabel->setEnabled(true);
            }
            else
            {
                xVector->setEnabled(false);
                xVectorLabel->setEnabled(false);
            }
            if (atts->GetUseUnitCellVectors() == false)
            {
                yVector->setEnabled(true);
                yVectorLabel->setEnabled(true);
            }
            else
            {
                yVector->setEnabled(false);
                yVectorLabel->setEnabled(false);
            }
            if (atts->GetUseUnitCellVectors() == false)
            {
                zVector->setEnabled(true);
                zVectorLabel->setEnabled(true);
            }
            else
            {
                zVector->setEnabled(false);
                zVectorLabel->setEnabled(false);
            }
            useUnitCellVectors->setChecked(atts->GetUseUnitCellVectors());
            break;
          case 1: //xVector
            dptr = atts->GetXVector();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            xVector->setText(temp);
            break;
          case 2: //yVector
            dptr = atts->GetYVector();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            yVector->setText(temp);
            break;
          case 3: //zVector
            dptr = atts->GetZVector();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            zVector->setText(temp);
            break;
          case 4: //xReplications
            temp.sprintf("%d", atts->GetXReplications());
            xReplications->setText(temp);
            break;
          case 5: //yReplications
            temp.sprintf("%d", atts->GetYReplications());
            yReplications->setText(temp);
            break;
          case 6: //zReplications
            temp.sprintf("%d", atts->GetZReplications());
            zReplications->setText(temp);
            break;
          case 7: //mergeResults
            mergeResults->setChecked(atts->GetMergeResults());
            break;
        }
    }

    xReplications->blockSignals(false);
    yReplications->blockSignals(false);
    zReplications->blockSignals(false);
    xVector->blockSignals(false);
    yVector->blockSignals(false);
    zVector->blockSignals(false);
    mergeResults->blockSignals(false);
    useUnitCellVectors->blockSignals(false);
}


// ****************************************************************************
// Method: QvisReplicateWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Jun 23 15:54:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisReplicateWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do useUnitCellVectors
    if(which_widget == 0 || doAll)
    {
        // Nothing for useUnitCellVectors
    }

    // Do xVector
    if(which_widget == 1 || doAll)
    {
        temp = xVector->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetXVector(val);
        }

        if(!okay)
        {
            const double *val = atts->GetXVector();
            msg.sprintf("The value of xVector was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetXVector(atts->GetXVector());
        }
    }

    // Do yVector
    if(which_widget == 2 || doAll)
    {
        temp = yVector->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetYVector(val);
        }

        if(!okay)
        {
            const double *val = atts->GetYVector();
            msg.sprintf("The value of yVector was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetYVector(atts->GetYVector());
        }
    }

    // Do zVector
    if(which_widget == 3 || doAll)
    {
        temp = zVector->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetZVector(val);
        }

        if(!okay)
        {
            const double *val = atts->GetZVector();
            msg.sprintf("The value of zVector was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetZVector(atts->GetZVector());
        }
    }

    // Do xReplications
    if(which_widget == 4 || doAll)
    {
        temp = xReplications->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetXReplications(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of xReplications was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetXReplications());
            Message(msg);
            atts->SetXReplications(atts->GetXReplications());
        }
    }

    // Do yReplications
    if(which_widget == 5 || doAll)
    {
        temp = yReplications->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetYReplications(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of yReplications was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetYReplications());
            Message(msg);
            atts->SetYReplications(atts->GetYReplications());
        }
    }

    // Do zReplications
    if(which_widget == 6 || doAll)
    {
        temp = zReplications->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetZReplications(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of zReplications was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetZReplications());
            Message(msg);
            atts->SetZReplications(atts->GetZReplications());
        }
    }

    // Do mergeResults
    if(which_widget == 7 || doAll)
    {
        // Nothing for mergeResults
    }

}


//
// Qt Slot functions
//


void
QvisReplicateWindow::useUnitCellVectorsChanged(bool val)
{
    atts->SetUseUnitCellVectors(val);
    Apply();
}


void
QvisReplicateWindow::xVectorProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisReplicateWindow::yVectorProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisReplicateWindow::zVectorProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisReplicateWindow::xReplicationsProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisReplicateWindow::yReplicationsProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisReplicateWindow::zReplicationsProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisReplicateWindow::mergeResultsChanged(bool val)
{
    atts->SetMergeResults(val);
    Apply();
}


