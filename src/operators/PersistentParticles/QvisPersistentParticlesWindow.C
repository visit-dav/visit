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

#include "QvisPersistentParticlesWindow.h"

#include <PersistentParticlesAttributes.h>
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
// Method: QvisPersistentParticlesWindow::QvisPersistentParticlesWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Jan 25 11:27:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisPersistentParticlesWindow::QvisPersistentParticlesWindow(const int type,
                         PersistentParticlesAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisPersistentParticlesWindow::~QvisPersistentParticlesWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Jan 25 11:27:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisPersistentParticlesWindow::~QvisPersistentParticlesWindow()
{
}


// ****************************************************************************
// Method: QvisPersistentParticlesWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Jan 25 11:27:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPersistentParticlesWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    startIndexLabel = new QLabel("Index of first time slice", central, "startIndexLabel");
    mainLayout->addWidget(startIndexLabel,0,0);
    startIndex = new QLineEdit(central, "startIndex");
    connect(startIndex, SIGNAL(returnPressed()),
            this, SLOT(startIndexProcessText()));
    mainLayout->addWidget(startIndex, 0,1);

    stopIndexLabel = new QLabel("Index of last time slice", central, "stopIndexLabel");
    mainLayout->addWidget(stopIndexLabel,1,0);
    stopIndex = new QLineEdit(central, "stopIndex");
    connect(stopIndex, SIGNAL(returnPressed()),
            this, SLOT(stopIndexProcessText()));
    mainLayout->addWidget(stopIndex, 1,1);

    strideLabel = new QLabel("Skip rate between time slices", central, "strideLabel");
    mainLayout->addWidget(strideLabel,2,0);
    stride = new QLineEdit(central, "stride");
    connect(stride, SIGNAL(returnPressed()),
            this, SLOT(strideProcessText()));
    mainLayout->addWidget(stride, 2,1);

}


// ****************************************************************************
// Method: QvisPersistentParticlesWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Jan 25 11:27:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPersistentParticlesWindow::UpdateWindow(bool doAll)
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
          case PersistentParticlesAttributes::ID_startIndex:
            startIndex->blockSignals(true);
            temp.sprintf("%d", atts->GetStartIndex());
            startIndex->setText(temp);
            startIndex->blockSignals(false);
            break;
          case PersistentParticlesAttributes::ID_stopIndex:
            stopIndex->blockSignals(true);
            temp.sprintf("%d", atts->GetStopIndex());
            stopIndex->setText(temp);
            stopIndex->blockSignals(false);
            break;
          case PersistentParticlesAttributes::ID_stride:
            stride->blockSignals(true);
            temp.sprintf("%d", atts->GetStride());
            stride->setText(temp);
            stride->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisPersistentParticlesWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Jan 25 11:27:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPersistentParticlesWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do startIndex
    if(which_widget == PersistentParticlesAttributes::ID_startIndex || doAll)
    {
        temp = startIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetStartIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetStartIndex());
            Message(msg);
            atts->SetStartIndex(atts->GetStartIndex());
        }
    }

    // Do stopIndex
    if(which_widget == PersistentParticlesAttributes::ID_stopIndex || doAll)
    {
        temp = stopIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetStopIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of stopIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetStopIndex());
            Message(msg);
            atts->SetStopIndex(atts->GetStopIndex());
        }
    }

    // Do stride
    if(which_widget == PersistentParticlesAttributes::ID_stride || doAll)
    {
        temp = stride->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetStride(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of stride was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetStride());
            Message(msg);
            atts->SetStride(atts->GetStride());
        }
    }

}


//
// Qt Slot functions
//


void
QvisPersistentParticlesWindow::startIndexProcessText()
{
    GetCurrentValues(PersistentParticlesAttributes::ID_startIndex);
    Apply();
}


void
QvisPersistentParticlesWindow::stopIndexProcessText()
{
    GetCurrentValues(PersistentParticlesAttributes::ID_stopIndex);
    Apply();
}


void
QvisPersistentParticlesWindow::strideProcessText()
{
    GetCurrentValues(PersistentParticlesAttributes::ID_stride);
    Apply();
}


