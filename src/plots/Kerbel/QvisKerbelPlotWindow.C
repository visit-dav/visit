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

#include "QvisKerbelPlotWindow.h"

#include <KerbelAttributes.h>
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
// Method: QvisKerbelPlotWindow::QvisKerbelPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisKerbelPlotWindow::QvisKerbelPlotWindow(const int type,
                         KerbelAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::~QvisKerbelPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisKerbelPlotWindow::~QvisKerbelPlotWindow()
{
}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Center", central, "CenterLabel"),0,0);
    Center = new QLineEdit(central, "Center");
    connect(Center, SIGNAL(returnPressed()),
            this, SLOT(CenterProcessText()));
    mainLayout->addWidget(Center, 0,1);

    mainLayout->addWidget(new QLabel("Radius", central, "RadiusLabel"),1,0);
    Radius = new QLineEdit(central, "Radius");
    connect(Radius, SIGNAL(returnPressed()),
            this, SLOT(RadiusProcessText()));
    mainLayout->addWidget(Radius, 1,1);

    mainLayout->addWidget(new QLabel("Opacity", central, "opacityLabel"),2,0);
    opacity = new QvisOpacitySlider(0,255,25,255, central, "opacity", NULL);
    opacity->setTickInterval(64);
    opacity->setGradientColor(QColor(0, 0, 0));
    connect(opacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    mainLayout->addWidget(opacity, 2,1);

    mainLayout->addWidget(new QLabel("Color table", central, "colorTableNameLabel"),3,0);
    colorTableName = new QvisColorTableButton(central, "colorTableName");
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    mainLayout->addWidget(colorTableName, 3,1);

    legendFlag = new QCheckBox("Legend", central, "legendFlag");
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    mainLayout->addWidget(legendFlag, 4,0);

    mainLayout->addWidget(new QLabel("Filename", central, "filenameLabel"),5,0);
    filename = new QLineEdit(central, "filename");
    connect(filename, SIGNAL(returnPressed()),
            this, SLOT(filenameProcessText()));
    mainLayout->addWidget(filename, 5,1);

}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisKerbelPlotWindow::UpdateWindow(bool doAll)
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

        const float          *fptr;
        switch(i)
        {
          case 0: //Center
            fptr = atts->GetCenter();
            temp.sprintf("%g %g %g", fptr[0], fptr[1], fptr[2]);
            Center->setText(temp);
            break;
          case 1: //Radius
            temp.setNum(atts->GetRadius());
            Radius->setText(temp);
            break;
          case 2: //opacity
            opacity->setValue(int(atts->GetOpacity()*255.));
            break;
          case 3: //colorTableName
            colorTableName->setColorTable(atts->GetColorTableName().c_str());
            break;
          case 4: //legendFlag
            legendFlag->setChecked(atts->GetLegendFlag());
            break;
          case 5: //filename
            temp = atts->GetFilename().c_str();
            filename->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do Center
    if(which_widget == 0 || doAll)
    {
        temp = Center->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val[3];
            sscanf(temp.latin1(), "%g %g %g", &val[0], &val[1], &val[2]);
            atts->SetCenter(val);
        }

        if(!okay)
        {
            const float *val = atts->GetCenter();
            msg.sprintf("The value of Center was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetCenter(atts->GetCenter());
        }
    }

    // Do Radius
    if(which_widget == 1 || doAll)
    {
        temp = Radius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of Radius was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRadius());
            Message(msg);
            atts->SetRadius(atts->GetRadius());
        }
    }

    // Do opacity
    if(which_widget == 2 || doAll)
    {
        // Nothing for opacity
    }

    // Do colorTableName
    if(which_widget == 3 || doAll)
    {
        // Nothing for colorTableName
    }

    // Do legendFlag
    if(which_widget == 4 || doAll)
    {
        // Nothing for legendFlag
    }

    // Do filename
    if(which_widget == 5 || doAll)
    {
        temp = filename->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetFilename(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of filename was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetFilename().c_str());
            Message(msg);
            atts->SetFilename(atts->GetFilename());
        }
    }

}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        viewer->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisKerbelPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisKerbelPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Fri Nov 15 10:46:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisKerbelPlotWindow::reset()
{
    viewer->ResetPlotOptions(plotType);
}


void
QvisKerbelPlotWindow::CenterProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisKerbelPlotWindow::RadiusProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisKerbelPlotWindow::opacityChanged(int opacity, const void*)
{
    atts->SetOpacity((float)opacity/255.);
    Apply();
}


void
QvisKerbelPlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    atts->SetColorTableName(ctName.latin1());
    Apply();
}


void
QvisKerbelPlotWindow::legendFlagChanged(bool val)
{
    atts->SetLegendFlag(val);
    Apply();
}


void
QvisKerbelPlotWindow::filenameProcessText()
{
    GetCurrentValues(5);
    Apply();
}


