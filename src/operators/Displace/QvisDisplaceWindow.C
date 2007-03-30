#include "QvisDisplaceWindow.h"

#include <DisplaceAttributes.h>
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
// Method: QvisDisplaceWindow::QvisDisplaceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisDisplaceWindow::QvisDisplaceWindow(const int type,
                         DisplaceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisDisplaceWindow::~QvisDisplaceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisDisplaceWindow::~QvisDisplaceWindow()
{
}


// ****************************************************************************
// Method: QvisDisplaceWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisDisplaceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Displacement multiplier ", central, "factorLabel"),0,0);
    factor = new QLineEdit(central, "factor");
    connect(factor, SIGNAL(returnPressed()),
            this, SLOT(factorProcessText()));
    mainLayout->addWidget(factor, 0,1);

    mainLayout->addWidget(new QLabel("variable", central, "variableLabel"),1,0);
    variable = new QLineEdit(central, "variable");
    connect(variable, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(variable, 1,1);

}


// ****************************************************************************
// Method: QvisDisplaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisDisplaceWindow::UpdateWindow(bool doAll)
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
          case 0: //factor
            temp.sprintf("%g", atts->GetFactor());
            factor->setText(temp);
            break;
          case 1: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisDisplaceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisDisplaceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do factor
    if(which_widget == 0 || doAll)
    {
        temp = factor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of factor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetFactor());
            Message(msg);
            atts->SetFactor(atts->GetFactor());
        }
    }

    // Do variable
    if(which_widget == 1 || doAll)
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
QvisDisplaceWindow::factorProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisDisplaceWindow::variableProcessText()
{
    GetCurrentValues(1);
    Apply();
}


