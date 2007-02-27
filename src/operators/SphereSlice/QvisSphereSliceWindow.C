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

#include "QvisSphereSliceWindow.h"

#include <SphereSliceAttributes.h>
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
// Method: QvisSphereSliceWindow::QvisSphereSliceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisSphereSliceWindow::QvisSphereSliceWindow(const int type,
                         SphereSliceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisSphereSliceWindow::~QvisSphereSliceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisSphereSliceWindow::~QvisSphereSliceWindow()
{
}


// ****************************************************************************
// Method: QvisSphereSliceWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSphereSliceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("origin", central, "originLabel"),0,0);
    origin = new QLineEdit(central, "origin");
    connect(origin, SIGNAL(returnPressed()),
            this, SLOT(originProcessText()));
    mainLayout->addWidget(origin, 0,1);

    mainLayout->addWidget(new QLabel("radius", central, "radiusLabel"),1,0);
    radius = new QLineEdit(central, "radius");
    connect(radius, SIGNAL(returnPressed()),
            this, SLOT(radiusProcessText()));
    mainLayout->addWidget(radius, 1,1);

}


// ****************************************************************************
// Method: QvisSphereSliceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisSphereSliceWindow::UpdateWindow(bool doAll)
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
          case 0: //origin
            dptr = atts->GetOrigin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            origin->setText(temp);
            break;
          case 1: //radius
            temp.setNum(atts->GetRadius());
            radius->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisSphereSliceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSphereSliceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do origin
    if(which_widget == 0 || doAll)
    {
        temp = origin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetOrigin(val);
        }

        if(!okay)
        {
            const double *val = atts->GetOrigin();
            msg.sprintf("The value of origin was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetOrigin(atts->GetOrigin());
        }
    }

    // Do radius
    if(which_widget == 1 || doAll)
    {
        temp = radius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of radius was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRadius());
            Message(msg);
            atts->SetRadius(atts->GetRadius());
        }
    }

}


//
// Qt Slot functions
//


void
QvisSphereSliceWindow::originProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisSphereSliceWindow::radiusProcessText()
{
    GetCurrentValues(1);
    Apply();
}


