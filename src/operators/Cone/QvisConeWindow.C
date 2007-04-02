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

#include "QvisConeWindow.h"

#include <ConeAttributes.h>
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
// Method: QvisConeWindow::QvisConeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConeWindow::QvisConeWindow(const int type,
                         ConeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisConeWindow::~QvisConeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConeWindow::~QvisConeWindow()
{
}


// ****************************************************************************
// Method: QvisConeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Angle", central, "angleLabel"),0,0);
    angle = new QLineEdit(central, "angle");
    connect(angle, SIGNAL(returnPressed()),
            this, SLOT(angleProcessText()));
    mainLayout->addWidget(angle, 0,1);

    mainLayout->addWidget(new QLabel("Origin", central, "originLabel"),1,0);
    origin = new QLineEdit(central, "origin");
    connect(origin, SIGNAL(returnPressed()),
            this, SLOT(originProcessText()));
    mainLayout->addWidget(origin, 1,1);

    mainLayout->addWidget(new QLabel("Direction", central, "normalLabel"),2,0);
    normal = new QLineEdit(central, "normal");
    connect(normal, SIGNAL(returnPressed()),
            this, SLOT(normalProcessText()));
    mainLayout->addWidget(normal, 2,1);

    mainLayout->addWidget(new QLabel("Representation", central, "representationLabel"),3,0);
    representation = new QButtonGroup(central, "representation");
    representation->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *representationLayout = new QHBoxLayout(representation);
    representationLayout->setSpacing(10);
    QRadioButton *representationRepresentationThreeD = new QRadioButton("In 3D", representation);
    representationLayout->addWidget(representationRepresentationThreeD);
    QRadioButton *representationRepresentationFlattened = new QRadioButton("Projected to 2D", representation);
    representationLayout->addWidget(representationRepresentationFlattened);
    QRadioButton *representationRepresentationR_Theta = new QRadioButton("Cylindrical", representation);
    representationLayout->addWidget(representationRepresentationR_Theta);
    connect(representation, SIGNAL(clicked(int)),
            this, SLOT(representationChanged(int)));
    mainLayout->addWidget(representation, 3,1);

    mainLayout->addWidget(new QLabel("Up Axis", central, "upAxisLabel"),4,0);
    upAxis = new QLineEdit(central, "upAxis");
    connect(upAxis, SIGNAL(returnPressed()),
            this, SLOT(upAxisProcessText()));
    mainLayout->addWidget(upAxis, 4,1);

    cutByLength = new QCheckBox("Cut cone off?    Length", central, "cutByLength");
    connect(cutByLength, SIGNAL(toggled(bool)),
            this, SLOT(cutByLengthChanged(bool)));
    mainLayout->addWidget(cutByLength, 5,0);

    //mainLayout->addWidget(new QLabel("Cone Length", central, "lengthLabel"),5,1);
    length = new QLineEdit(central, "length");
    connect(length, SIGNAL(returnPressed()),
            this, SLOT(lengthProcessText()));
    mainLayout->addWidget(length, 5,1);

}


// ****************************************************************************
// Method: QvisConeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisConeWindow::UpdateWindow(bool doAll)
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
          case 0: //angle
            temp.setNum(atts->GetAngle());
            angle->setText(temp);
            break;
          case 1: //origin
            dptr = atts->GetOrigin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            origin->setText(temp);
            break;
          case 2: //normal
            dptr = atts->GetNormal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            normal->setText(temp);
            break;
          case 3: //representation
            if (atts->GetRepresentation() == ConeAttributes::Flattened || 
                atts->GetRepresentation() == ConeAttributes::R_Theta)
                upAxis->setEnabled(true);
            else
                upAxis->setEnabled(false);
            representation->setButton(atts->GetRepresentation());
            break;
          case 4: //upAxis
            dptr = atts->GetUpAxis();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            upAxis->setText(temp);
            break;
          case 5: //cutByLength
            if (atts->GetCutByLength() == true)
                length->setEnabled(true);
            else
                length->setEnabled(false);
            cutByLength->setChecked(atts->GetCutByLength());
            break;
          case 6: //length
            temp.setNum(atts->GetLength());
            length->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisConeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Jun 3 15:59:57 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003 
//   Disallow (0, 0, 0) for Normal and UpAxis.
//   
// ****************************************************************************

void
QvisConeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do angle
    if(which_widget == 0 || doAll)
    {
        temp = angle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        double val = temp.toDouble(&okay);
        if (val <= 0. || val >= 90.)
        {
            okay = false;
        }
        if(okay)
        {
            atts->SetAngle(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of angle was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetAngle());
            Message(msg);
            atts->SetAngle(atts->GetAngle());
        }
    }

    // Do origin
    if(which_widget == 1 || doAll)
    {
        temp = origin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
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

    // Do normal
    if(which_widget == 2 || doAll)
    {
        temp = normal->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
            {
                okay =  (val[0] != 0. || val[1] != 0. || val[2] != 0.);
                if (okay)
                    atts->SetNormal(val);
            }
        }

        if(!okay)
        {
            const double *val = atts->GetNormal();
            msg.sprintf("The value of normal was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetNormal(atts->GetNormal());
        }
    }

    // Do representation
    if(which_widget == 3 || doAll)
    {
        // Nothing for representation
    }

    // Do upAxis
    if(which_widget == 4 || doAll)
    {
        temp = upAxis->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
            {
                okay =  (val[0] != 0. || val[1] != 0. || val[2] != 0.);
                if (okay)
                    atts->SetUpAxis(val);
            }
        }

        if(!okay)
        {
            const double *val = atts->GetUpAxis();
            msg.sprintf("The value of upAxis was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetUpAxis(atts->GetUpAxis());
        }
    }

    // Do cutByLength
    if(which_widget == 5 || doAll)
    {
        // Nothing for cutByLength
    }

    // Do length
    if(which_widget == 6 || doAll)
    {
        temp = length->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        double val = temp.toDouble(&okay);
        if (val <= 0.)
        {
            okay = false;
        }
        if(okay)
        {
            atts->SetLength(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of length was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetLength());
            Message(msg);
            atts->SetLength(atts->GetLength());
        }
    }

}


//
// Qt Slot functions
//


void
QvisConeWindow::angleProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisConeWindow::originProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisConeWindow::normalProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisConeWindow::representationChanged(int val)
{
    if(val != atts->GetRepresentation())
    {
        atts->SetRepresentation(ConeAttributes::Representation(val));
        Apply();
    }
}


void
QvisConeWindow::upAxisProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisConeWindow::cutByLengthChanged(bool val)
{
    atts->SetCutByLength(val);
    Apply();
}


void
QvisConeWindow::lengthProcessText()
{
    GetCurrentValues(6);
    Apply();
}


