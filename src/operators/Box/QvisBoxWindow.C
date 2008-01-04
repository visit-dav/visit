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

#include "QvisBoxWindow.h"

#include <BoxAttributes.h>
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
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisBoxWindow::QvisBoxWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisBoxWindow::QvisBoxWindow(const int type,
                         BoxAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisBoxWindow::~QvisBoxWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisBoxWindow::~QvisBoxWindow()
{
}


// ****************************************************************************
// Method: QvisBoxWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoxWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Amount of cell in the range", central, "amountLabel"),0,0);
    amount = new QButtonGroup(central, "amount");
    amount->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *amountLayout = new QHBoxLayout(amount);
    amountLayout->setSpacing(10);
    QRadioButton *amountAmountSome = new QRadioButton("Some", amount);
    amountLayout->addWidget(amountAmountSome);
    QRadioButton *amountAmountAll = new QRadioButton("All", amount);
    amountLayout->addWidget(amountAmountAll);
    connect(amount, SIGNAL(clicked(int)),
            this, SLOT(amountChanged(int)));
    mainLayout->addWidget(amount, 0,1);

    mainLayout->addWidget(new QLabel("X-Minimum", central, "minxLabel"),1,0);
    minx = new QLineEdit(central, "minx");
    connect(minx, SIGNAL(returnPressed()),
            this, SLOT(minxProcessText()));
    mainLayout->addWidget(minx, 1,1);

    mainLayout->addWidget(new QLabel("X-Maximum", central, "maxxLabel"),2,0);
    maxx = new QLineEdit(central, "maxx");
    connect(maxx, SIGNAL(returnPressed()),
            this, SLOT(maxxProcessText()));
    mainLayout->addWidget(maxx, 2,1);

    mainLayout->addWidget(new QLabel("Y-Minimum", central, "minyLabel"),3,0);
    miny = new QLineEdit(central, "miny");
    connect(miny, SIGNAL(returnPressed()),
            this, SLOT(minyProcessText()));
    mainLayout->addWidget(miny, 3,1);

    mainLayout->addWidget(new QLabel("Y-Maximum", central, "maxyLabel"),4,0);
    maxy = new QLineEdit(central, "maxy");
    connect(maxy, SIGNAL(returnPressed()),
            this, SLOT(maxyProcessText()));
    mainLayout->addWidget(maxy, 4,1);

    mainLayout->addWidget(new QLabel("Z-Minimum", central, "minzLabel"),5,0);
    minz = new QLineEdit(central, "minz");
    connect(minz, SIGNAL(returnPressed()),
            this, SLOT(minzProcessText()));
    mainLayout->addWidget(minz, 5,1);

    mainLayout->addWidget(new QLabel("Z-Maximum", central, "maxzLabel"),6,0);
    maxz = new QLineEdit(central, "maxz");
    connect(maxz, SIGNAL(returnPressed()),
            this, SLOT(maxzProcessText()));
    mainLayout->addWidget(maxz, 6,1);

}


// ****************************************************************************
// Method: QvisBoxWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisBoxWindow::UpdateWindow(bool doAll)
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
          case BoxAttributes::ID_amount:
            amount->setButton(atts->GetAmount());
            break;
          case BoxAttributes::ID_minx:
            temp.setNum(atts->GetMinx());
            minx->setText(temp);
            break;
          case BoxAttributes::ID_maxx:
            temp.setNum(atts->GetMaxx());
            maxx->setText(temp);
            break;
          case BoxAttributes::ID_miny:
            temp.setNum(atts->GetMiny());
            miny->setText(temp);
            break;
          case BoxAttributes::ID_maxy:
            temp.setNum(atts->GetMaxy());
            maxy->setText(temp);
            break;
          case BoxAttributes::ID_minz:
            temp.setNum(atts->GetMinz());
            minz->setText(temp);
            break;
          case BoxAttributes::ID_maxz:
            temp.setNum(atts->GetMaxz());
            maxz->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisBoxWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoxWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do minx
    if(which_widget == BoxAttributes::ID_minx || doAll)
    {
        temp = minx->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMinx(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of minx was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMinx());
            Message(msg);
            atts->SetMinx(atts->GetMinx());
        }
    }

    // Do maxx
    if(which_widget == BoxAttributes::ID_maxx || doAll)
    {
        temp = maxx->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxx(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxx was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxx());
            Message(msg);
            atts->SetMaxx(atts->GetMaxx());
        }
    }

    // Do miny
    if(which_widget == BoxAttributes::ID_miny || doAll)
    {
        temp = miny->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMiny(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of miny was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMiny());
            Message(msg);
            atts->SetMiny(atts->GetMiny());
        }
    }

    // Do maxy
    if(which_widget == BoxAttributes::ID_maxy || doAll)
    {
        temp = maxy->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxy(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxy was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxy());
            Message(msg);
            atts->SetMaxy(atts->GetMaxy());
        }
    }

    // Do minz
    if(which_widget == BoxAttributes::ID_minz || doAll)
    {
        temp = minz->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMinz(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of minz was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMinz());
            Message(msg);
            atts->SetMinz(atts->GetMinz());
        }
    }

    // Do maxz
    if(which_widget == BoxAttributes::ID_maxz || doAll)
    {
        temp = maxz->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxz(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxz was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxz());
            Message(msg);
            atts->SetMaxz(atts->GetMaxz());
        }
    }

}


//
// Qt Slot functions
//


void
QvisBoxWindow::amountChanged(int val)
{
    BoxAttributes::Amount val2;
    val2 = (val == 0) ? BoxAttributes::Some : BoxAttributes::All;

    if(val2 != atts->GetAmount())
    {
        atts->SetAmount(val2);
        Apply();
    }
}


void
QvisBoxWindow::minxProcessText()
{
    GetCurrentValues(BoxAttributes::ID_minx);
    Apply();
}


void
QvisBoxWindow::maxxProcessText()
{
    GetCurrentValues(BoxAttributes::ID_maxx);
    Apply();
}


void
QvisBoxWindow::minyProcessText()
{
    GetCurrentValues(BoxAttributes::ID_miny);
    Apply();
}


void
QvisBoxWindow::maxyProcessText()
{
    GetCurrentValues(BoxAttributes::ID_maxy);
    Apply();
}


void
QvisBoxWindow::minzProcessText()
{
    GetCurrentValues(BoxAttributes::ID_minz);
    Apply();
}


void
QvisBoxWindow::maxzProcessText()
{
    GetCurrentValues(BoxAttributes::ID_maxz);
    Apply();
}


