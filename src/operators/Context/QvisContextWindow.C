#include "QvisContextWindow.h"

#include <ContextAttributes.h>
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
// Method: QvisContextWindow::QvisContextWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisContextWindow::QvisContextWindow(const int type,
                         ContextAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisContextWindow::~QvisContextWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisContextWindow::~QvisContextWindow()
{
}


// ****************************************************************************
// Method: QvisContextWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisContextWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Offset the primary variable by ", central, "offsetLabel"),0,0);
    offset = new QLineEdit(central, "offset");
    connect(offset, SIGNAL(returnPressed()),
            this, SLOT(offsetProcessText()));
    mainLayout->addWidget(offset, 0,1);

    mainLayout->addWidget(new QLabel("Use the primary variable if it is above ", central, "lowLabel"),1,0);
    low = new QLineEdit(central, "low");
    connect(low, SIGNAL(returnPressed()),
            this, SLOT(lowProcessText()));
    mainLayout->addWidget(low, 1,1);

    mainLayout->addWidget(new QLabel("Use the primary variable if it is below ", central, "hiLabel"),2,0);
    hi = new QLineEdit(central, "hi");
    connect(hi, SIGNAL(returnPressed()),
            this, SLOT(hiProcessText()));
    mainLayout->addWidget(hi, 2,1);

    mainLayout->addWidget(new QLabel("Context Variable", central, "contextLabel"),3,0);
    context = new QLineEdit(central, "context");
    connect(context, SIGNAL(returnPressed()),
            this, SLOT(contextProcessText()));
    mainLayout->addWidget(context, 3,1);

    mainLayout->addWidget(new QLabel("Cutoff for context variable", central, "cutoffLabel"),4,0);
    cutoff = new QLineEdit(central, "cutoff");
    connect(cutoff, SIGNAL(returnPressed()),
            this, SLOT(cutoffProcessText()));
    mainLayout->addWidget(cutoff, 4,1);

    mainLayout->addWidget(new QLabel("Map values below the cutoff to ", central, "belowLabel"),5,0);
    below = new QLineEdit(central, "below");
    connect(below, SIGNAL(returnPressed()),
            this, SLOT(belowProcessText()));
    mainLayout->addWidget(below, 5,1);

    mainLayout->addWidget(new QLabel("Map values above the cutoff to ", central, "aboveLabel"),6,0);
    above = new QLineEdit(central, "above");
    connect(above, SIGNAL(returnPressed()),
            this, SLOT(aboveProcessText()));
    mainLayout->addWidget(above, 6,1);

}


// ****************************************************************************
// Method: QvisContextWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisContextWindow::UpdateWindow(bool doAll)
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
          case 0: //offset
            temp.setNum(atts->GetOffset());
            offset->setText(temp);
            break;
          case 1: //low
            temp.setNum(atts->GetLow());
            low->setText(temp);
            break;
          case 2: //hi
            temp.setNum(atts->GetHi());
            hi->setText(temp);
            break;
          case 3: //context
            temp = atts->GetContext().c_str();
            context->setText(temp);
            break;
          case 4: //cutoff
            temp.setNum(atts->GetCutoff());
            cutoff->setText(temp);
            break;
          case 5: //below
            temp.setNum(atts->GetBelow());
            below->setText(temp);
            break;
          case 6: //above
            temp.setNum(atts->GetAbove());
            above->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisContextWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisContextWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do offset
    if(which_widget == 0 || doAll)
    {
        temp = offset->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetOffset(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of offset was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetOffset());
            Message(msg);
            atts->SetOffset(atts->GetOffset());
        }
    }

    // Do low
    if(which_widget == 1 || doAll)
    {
        temp = low->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetLow(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of low was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetLow());
            Message(msg);
            atts->SetLow(atts->GetLow());
        }
    }

    // Do hi
    if(which_widget == 2 || doAll)
    {
        temp = hi->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetHi(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of hi was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetHi());
            Message(msg);
            atts->SetHi(atts->GetHi());
        }
    }

    // Do context
    if(which_widget == 3 || doAll)
    {
        temp = context->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetContext(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of context was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetContext().c_str());
            Message(msg);
            atts->SetContext(atts->GetContext());
        }
    }

    // Do cutoff
    if(which_widget == 4 || doAll)
    {
        temp = cutoff->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetCutoff(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of cutoff was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetCutoff());
            Message(msg);
            atts->SetCutoff(atts->GetCutoff());
        }
    }

    // Do below
    if(which_widget == 5 || doAll)
    {
        temp = below->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetBelow(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of below was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetBelow());
            Message(msg);
            atts->SetBelow(atts->GetBelow());
        }
    }

    // Do above
    if(which_widget == 6 || doAll)
    {
        temp = above->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetAbove(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of above was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetAbove());
            Message(msg);
            atts->SetAbove(atts->GetAbove());
        }
    }

}


//
// Qt Slot functions
//


void
QvisContextWindow::offsetProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisContextWindow::lowProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisContextWindow::hiProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisContextWindow::contextProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisContextWindow::cutoffProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisContextWindow::belowProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisContextWindow::aboveProcessText()
{
    GetCurrentValues(6);
    Apply();
}


