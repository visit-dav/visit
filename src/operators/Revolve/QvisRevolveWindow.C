#include "QvisRevolveWindow.h"

#include <RevolveAttributes.h>
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
// Method: QvisRevolveWindow::QvisRevolveWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisRevolveWindow::QvisRevolveWindow(const int type,
                         RevolveAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisRevolveWindow::~QvisRevolveWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisRevolveWindow::~QvisRevolveWindow()
{
}


// ****************************************************************************
// Method: QvisRevolveWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRevolveWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 4,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Axis of revolution", central, "axisLabel"),0,0);
    axis = new QLineEdit(central, "axis");
    connect(axis, SIGNAL(returnPressed()),
            this, SLOT(axisProcessText()));
    mainLayout->addWidget(axis, 0,1);

    mainLayout->addWidget(new QLabel("Start angle", central, "startAngleLabel"),1,0);
    startAngle = new QLineEdit(central, "startAngle");
    connect(startAngle, SIGNAL(returnPressed()),
            this, SLOT(startAngleProcessText()));
    mainLayout->addWidget(startAngle, 1,1);

    mainLayout->addWidget(new QLabel("Stop angle", central, "stopAngleLabel"),2,0);
    stopAngle = new QLineEdit(central, "stopAngle");
    connect(stopAngle, SIGNAL(returnPressed()),
            this, SLOT(stopAngleProcessText()));
    mainLayout->addWidget(stopAngle, 2,1);

    mainLayout->addWidget(new QLabel("Number of steps", central, "stepsLabel"),3,0);
    steps = new QLineEdit(central, "steps");
    connect(steps, SIGNAL(returnPressed()),
            this, SLOT(stepsProcessText()));
    mainLayout->addWidget(steps, 3,1);

}


// ****************************************************************************
// Method: QvisRevolveWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRevolveWindow::UpdateWindow(bool doAll)
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
          case 0: //axis
            dptr = atts->GetAxis();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            axis->setText(temp);
            break;
          case 1: //startAngle
            temp.sprintf("%g", atts->GetStartAngle());
            startAngle->setText(temp);
            break;
          case 2: //stopAngle
            temp.sprintf("%g", atts->GetStopAngle());
            stopAngle->setText(temp);
            break;
          case 3: //steps
            temp.sprintf("%d", atts->GetSteps());
            steps->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisRevolveWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed May 21 11:10:58 PDT 2003   
//   Disallow (0, 0, 0) as axis of revolution.
//   
// ****************************************************************************

void
QvisRevolveWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do axis
    if(which_widget == 0 || doAll)
    {
        temp = axis->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
            {
                okay = (val[0] != 0. || val[1] != 0. || val[2] != 0.);
                if (okay)
                    atts->SetAxis(val);
            }
        }

        if(!okay)
        {
            const double *val = atts->GetAxis();
            msg.sprintf("The value of axis was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetAxis(atts->GetAxis());
        }
    }

    // Do startAngle
    if(which_widget == 1 || doAll)
    {
        temp = startAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStartAngle(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of startAngle was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStartAngle());
            Message(msg);
            atts->SetStartAngle(atts->GetStartAngle());
        }
    }

    // Do stopAngle
    if(which_widget == 2 || doAll)
    {
        temp = stopAngle->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetStopAngle(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of stopAngle was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetStopAngle());
            Message(msg);
            atts->SetStopAngle(atts->GetStopAngle());
        }
    }

    // Do steps
    if(which_widget == 3 || doAll)
    {
        temp = steps->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetSteps(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of steps was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetSteps());
            Message(msg);
            atts->SetSteps(atts->GetSteps());
        }
    }

}


//
// Qt Slot functions
//


void
QvisRevolveWindow::axisProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisRevolveWindow::startAngleProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisRevolveWindow::stopAngleProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisRevolveWindow::stepsProcessText()
{
    GetCurrentValues(3);
    Apply();
}


