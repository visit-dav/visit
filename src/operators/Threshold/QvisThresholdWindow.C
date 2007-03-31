#include "QvisThresholdWindow.h"

#include <ThresholdAttributes.h>
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
// Method: QvisThresholdWindow::QvisThresholdWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisThresholdWindow::QvisThresholdWindow(const int type,
                         ThresholdAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThresholdWindow::~QvisThresholdWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisThresholdWindow::~QvisThresholdWindow()
{
}


// ****************************************************************************
// Method: QvisThresholdWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisThresholdWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 4,2,  10, "mainLayout");


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

    mainLayout->addWidget(new QLabel("Lower bound", central, "lboundLabel"),1,0);
    lbound = new QLineEdit(central, "lbound");
    connect(lbound, SIGNAL(returnPressed()),
            this, SLOT(lboundProcessText()));
    mainLayout->addWidget(lbound, 1,1);

    mainLayout->addWidget(new QLabel("Upper bound", central, "uboundLabel"),2,0);
    ubound = new QLineEdit(central, "ubound");
    connect(ubound, SIGNAL(returnPressed()),
            this, SLOT(uboundProcessText()));
    mainLayout->addWidget(ubound, 2,1);

    mainLayout->addWidget(new QLabel("variable", central, "variableLabel"),3,0);
    variable = new QLineEdit(central, "variable");
    connect(variable, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(variable, 3,1);

}


// ****************************************************************************
// Method: QvisThresholdWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//   
// ****************************************************************************

void
QvisThresholdWindow::UpdateWindow(bool doAll)
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
          case 0: //amount
            amount->setButton(atts->GetAmount());
            break;
          case 1: //lbound
            if (atts->GetLbound() == -1e+37)
                temp = "min";
            else
                temp.sprintf("%g", atts->GetLbound());
            lbound->setText(temp);
            break;
          case 2: //ubound
            if (atts->GetUbound() == +1e+37)
                temp = "max";
            else
                temp.sprintf("%g", atts->GetUbound());
            ubound->setText(temp);
            break;
          case 3: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//
// ****************************************************************************

void
QvisThresholdWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do amount
    if(which_widget == 0 || doAll)
    {
        // Nothing for amount
    }

    // Do lbound
    if(which_widget == 1 || doAll)
    {
        temp = lbound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("min"))
            atts->SetUbound(-1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetLbound(val);
            }

            if(!okay)
            {
                msg.sprintf("The value of lbound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetLbound());
                Message(msg);
                atts->SetLbound(atts->GetLbound());
            }
        }
    }

    // Do ubound
    if(which_widget == 2 || doAll)
    {
        temp = ubound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("max"))
            atts->SetUbound(1e+37);
        else
        {
            okay = !temp.isEmpty();
            if(okay)
            {
                double val = temp.toDouble(&okay);
                atts->SetUbound(val);
            }
    
            if(!okay)
            {
                msg.sprintf("The value of ubound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetUbound());
                Message(msg);
                atts->SetUbound(atts->GetUbound());
            }
        }
    }

    // Do variable
    if(which_widget == 3 || doAll)
    {
        temp = variable->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVariable(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of variable was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVariable().c_str());
            Message(msg);
            atts->SetVariable(atts->GetVariable());
        }
    }

}


//
// Qt Slot functions
//


void
QvisThresholdWindow::amountChanged(int val)
{
    ThresholdAttributes::Amount newVal = ThresholdAttributes::Amount(val);
    if(newVal != atts->GetAmount())
    {
        atts->SetAmount(newVal);
        Apply();
    }
}


void
QvisThresholdWindow::lboundProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisThresholdWindow::uboundProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisThresholdWindow::variableProcessText()
{
    GetCurrentValues(3);
    Apply();
}


