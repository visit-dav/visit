#include "QvisThreeSliceWindow.h"

#include <ThreeSliceAttributes.h>
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
// Method: QvisThreeSliceWindow::QvisThreeSliceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisThreeSliceWindow::QvisThreeSliceWindow(const int type,
                         ThreeSliceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::~QvisThreeSliceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisThreeSliceWindow::~QvisThreeSliceWindow()
{
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisThreeSliceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("x", central, "xLabel"),0,0);
    x = new QLineEdit(central, "x");
    connect(x, SIGNAL(returnPressed()),
            this, SLOT(xProcessText()));
    mainLayout->addWidget(x, 0,1);

    mainLayout->addWidget(new QLabel("y", central, "yLabel"),1,0);
    y = new QLineEdit(central, "y");
    connect(y, SIGNAL(returnPressed()),
            this, SLOT(yProcessText()));
    mainLayout->addWidget(y, 1,1);

    mainLayout->addWidget(new QLabel("z", central, "zLabel"),2,0);
    z = new QLineEdit(central, "z");
    connect(z, SIGNAL(returnPressed()),
            this, SLOT(zProcessText()));
    mainLayout->addWidget(z, 2,1);

}


// ****************************************************************************
// Method: QvisThreeSliceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisThreeSliceWindow::UpdateWindow(bool doAll)
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
          case 0: //x
            temp.sprintf("%g", atts->GetX());
            x->setText(temp);
            break;
          case 1: //y
            temp.sprintf("%g", atts->GetY());
            y->setText(temp);
            break;
          case 2: //z
            temp.sprintf("%g", atts->GetZ());
            z->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisThreeSliceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Jun 9 13:18:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisThreeSliceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do x
    if(which_widget == 0 || doAll)
    {
        temp = x->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetX(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of x was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetX());
            Message(msg);
            atts->SetX(atts->GetX());
        }
    }

    // Do y
    if(which_widget == 1 || doAll)
    {
        temp = y->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetY(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of y was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetY());
            Message(msg);
            atts->SetY(atts->GetY());
        }
    }

    // Do z
    if(which_widget == 2 || doAll)
    {
        temp = z->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetZ(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of z was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetZ());
            Message(msg);
            atts->SetZ(atts->GetZ());
        }
    }

}


//
// Qt Slot functions
//


void
QvisThreeSliceWindow::xProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisThreeSliceWindow::yProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisThreeSliceWindow::zProcessText()
{
    GetCurrentValues(2);
    Apply();
}


