#include "QvisLineoutWindow.h"

#include <LineoutAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QNarrowLineEdit.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisLineoutWindow::QvisLineoutWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisLineoutWindow::QvisLineoutWindow(const int type,
                         LineoutAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisLineoutWindow::~QvisLineoutWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisLineoutWindow::~QvisLineoutWindow()
{
}


// ****************************************************************************
// Method: QvisLineoutWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");

    //
    // Point1
    //
    mainLayout->addWidget(new QLabel("Point 1", central, "point1Label"), 0,0);
    point1 = new QLineEdit(central, "point1");
    connect(point1, SIGNAL(returnPressed()),
            this, SLOT(point1ProcessText()));
    mainLayout->addWidget(point1, 0,1);

    //
    // Point2
    //
    mainLayout->addWidget(new QLabel("Point 2", central, "point2Label"), 1,0);
    point2 = new QLineEdit(central, "point2");
    connect(point2, SIGNAL(returnPressed()),
            this, SLOT(point2ProcessText()));
    mainLayout->addWidget(point2, 1,1);

    //
    // Interactive
    //
    interactive = new QCheckBox("Interactive", central, "interactive");
    connect(interactive, SIGNAL(toggled(bool)),
            this, SLOT(interactiveChanged(bool)));
    mainLayout->addWidget(interactive, 2,0);


    //
    // IgnoreGlobal
    //
    ignoreGlobal = new QGroupBox("Override Global Lineout Settings",
                                  central, "ignoreGlobal");
    ignoreGlobal->setCheckable(true);
    connect(ignoreGlobal, SIGNAL(toggled(bool)),
            this, SLOT(ignoreGlobalChanged(bool)));
    topLayout->addWidget(ignoreGlobal);

    QVBoxLayout *blayout = new QVBoxLayout(ignoreGlobal);
    blayout->setMargin(10);
 
    QGridLayout *qgrid  = new QGridLayout(blayout, 5, 2);
    qgrid->setMargin(5);

    qgrid->setRowSpacing(0, 15);
    //
    // SamplingOn
    //
    samplingOn = new QCheckBox("Use Sampling", ignoreGlobal, "samplingOn");
    connect(samplingOn, SIGNAL(toggled(bool)),
            this, SLOT(samplingOnChanged(bool)));
    qgrid->addWidget(samplingOn, 1,0);

    //
    // NumberOfSamplePoints
    //
    numberOfSamplePointsLabel = new QLabel("Samples",  ignoreGlobal,
                                            "numberOfSamplePointsLabel");
    numberOfSamplePointsLabel->setAlignment(Qt::AlignCenter);
    qgrid->addWidget(numberOfSamplePointsLabel,2,0);
    numberOfSamplePoints = new QNarrowLineEdit(ignoreGlobal, "numberOfSamplePoints");
    connect(numberOfSamplePoints, SIGNAL(returnPressed()),
            this, SLOT(numberOfSamplePointsProcessText()));
    qgrid->addWidget(numberOfSamplePoints, 2,1);

    //
    // ReflineLabels
    //
    reflineLabels = new QCheckBox("Refline Labels", ignoreGlobal, "reflineLabels");
    connect(reflineLabels, SIGNAL(toggled(bool)),
            this, SLOT(reflineLabelsChanged(bool)));
    qgrid->addWidget(reflineLabels, 3,0);
}


// ****************************************************************************
// Method: QvisLineoutWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineoutWindow::UpdateWindow(bool doAll)
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
        bool flag;
        switch(i)
        {
          case 0: //point1
            dptr = atts->GetPoint1();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point1->setText(temp);
            break;
          case 1: //point2
            dptr = atts->GetPoint2();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            point2->setText(temp);
            break;
          case 2: //interactive
            interactive->setChecked(atts->GetInteractive());
            break;
          case 3: //ignoreGlobal
            ignoreGlobal->setChecked(atts->GetIgnoreGlobal());
            break;
          case 4: //samplingOn
            flag = atts->GetSamplingOn();
            numberOfSamplePoints->setEnabled(flag);
            numberOfSamplePointsLabel->setEnabled(flag);
            samplingOn->setChecked(flag);
            break;
          case 5: //numberOfSamplePoints
            temp.sprintf("%d", atts->GetNumberOfSamplePoints());
            numberOfSamplePoints->setText(temp);
            break;
          case 6: //reflineLabels
            reflineLabels->setChecked(atts->GetReflineLabels());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisLineoutWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineoutWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do point1
    if(which_widget == 0 || doAll)
    {
        temp = point1->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            val[2] = 0.;
            int numscanned = sscanf(temp.latin1(), "%lg %lg %lg", 
                                    &val[0], &val[1], &val[2]);
            okay = (numscanned == 2 || numscanned == 3);
            if (okay)
            {
                atts->SetPoint1(val);
            }
        }

        if(!okay)
        {
            const double *val = atts->GetPoint1();
            msg.sprintf("The value of point1 is not valid. It should consist "
                "of two or three real world coordinate values.  Resetting to "
                "the last good value of <%g %g %g>", val[0], val[1], val[2]);
            Message(msg);
            atts->SetPoint1(atts->GetPoint1());
        }
    }

    // Do point2
    if(which_widget == 1 || doAll)
    {
        temp = point2->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            val[2] = 0.;
            int numscanned = sscanf(temp.latin1(), "%lg %lg %lg", 
                                    &val[0], &val[1], &val[2]);
            okay = (numscanned == 2 || numscanned == 3);
            if (okay)
            {
                atts->SetPoint2(val);
            }
        }

        if(!okay)
        {
            const double *val = atts->GetPoint2();
            msg.sprintf("The value of point2 is not valid. It should consist "
                "of two or three real world coordinate values.  Resetting to "
                "the last good value of <%g %g %g>", val[0], val[1], val[2]);
            Message(msg);
            atts->SetPoint2(atts->GetPoint2());
        }
    }

    // Do interactive
    if(which_widget == 2 || doAll)
    {
        // Nothing for interactive
    }

    // Do ignoreGlobal
    if(which_widget == 3 || doAll)
    {
        // Nothing for ignoreGlobal
    }

    // Do samplingOn
    if(which_widget == 4 || doAll)
    {
        // Nothing for samplingOn
    }

    // Do numberOfSamplePoints
    if(which_widget == 5 || doAll)
    {
        temp = numberOfSamplePoints->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetNumberOfSamplePoints(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of numberOfSamplePoints was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetNumberOfSamplePoints());
            Message(msg);
            atts->SetNumberOfSamplePoints(atts->GetNumberOfSamplePoints());
        }
    }

    // Do reflineLabels
    if(which_widget == 6 || doAll)
    {
        // Nothing for reflineLabels
    }

}


//
// Qt Slot functions
//


void
QvisLineoutWindow::point1ProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisLineoutWindow::point2ProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisLineoutWindow::interactiveChanged(bool val)
{
    atts->SetInteractive(val);
    Apply();
}


void
QvisLineoutWindow::ignoreGlobalChanged(bool val)
{
    atts->SetIgnoreGlobal(val);
    Apply();
}


void
QvisLineoutWindow::samplingOnChanged(bool val)
{
    atts->SetSamplingOn(val);
    Apply();
}


void
QvisLineoutWindow::numberOfSamplePointsProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisLineoutWindow::reflineLabelsChanged(bool val)
{
    atts->SetReflineLabels(val);
    Apply();
}


