#include "QvisCylinderWindow.h"

#include <CylinderAttributes.h>
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
// Method: QvisCylinderWindow::QvisCylinderWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisCylinderWindow::QvisCylinderWindow(const int type,
                         CylinderAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisCylinderWindow::~QvisCylinderWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisCylinderWindow::~QvisCylinderWindow()
{
}


// ****************************************************************************
// Method: QvisCylinderWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisCylinderWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    point1Label = new QLabel("Endpoint 1", central, "point1Label");
    mainLayout->addWidget(point1Label,0,0);
    point1 = new QLineEdit(central, "point1");
    connect(point1, SIGNAL(returnPressed()),
            this, SLOT(point1ProcessText()));
    mainLayout->addWidget(point1, 0,1);

    point2Label = new QLabel("Endpoint 2", central, "point2Label");
    mainLayout->addWidget(point2Label,1,0);
    point2 = new QLineEdit(central, "point2");
    connect(point2, SIGNAL(returnPressed()),
            this, SLOT(point2ProcessText()));
    mainLayout->addWidget(point2, 1,1);

    radiusLabel = new QLabel("Radius", central, "radiusLabel");
    mainLayout->addWidget(radiusLabel,2,0);
    radius = new QLineEdit(central, "radius");
    connect(radius, SIGNAL(returnPressed()),
            this, SLOT(radiusProcessText()));
    mainLayout->addWidget(radius, 2,1);

}


// ****************************************************************************
// Method: QvisCylinderWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisCylinderWindow::UpdateWindow(bool doAll)
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
          case 2: //radius
            temp.setNum(atts->GetRadius());
            radius->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisCylinderWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Oct 21 13:17:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisCylinderWindow::GetCurrentValues(int which_widget)
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetPoint1(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint1();
            msg.sprintf("The value of point1 was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            atts->SetPoint2(val);
        }

        if(!okay)
        {
            const double *val = atts->GetPoint2();
            msg.sprintf("The value of point2 was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
            Message(msg);
            atts->SetPoint2(atts->GetPoint2());
        }
    }

    // Do radius
    if(which_widget == 2 || doAll)
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
QvisCylinderWindow::point1ProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisCylinderWindow::point2ProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisCylinderWindow::radiusProcessText()
{
    GetCurrentValues(2);
    Apply();
}


