#include "QvisIsovolumeWindow.h"

#include <IsovolumeAttributes.h>
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
// Method: QvisIsovolumeWindow::QvisIsovolumeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisIsovolumeWindow::QvisIsovolumeWindow(const int type,
                         IsovolumeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::~QvisIsovolumeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisIsovolumeWindow::~QvisIsovolumeWindow()
{
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisIsovolumeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    lboundLabel = new QLabel("Lower bound", central, "lboundLabel");
    mainLayout->addWidget(lboundLabel,0,0);
    lbound = new QLineEdit(central, "lbound");
    connect(lbound, SIGNAL(returnPressed()),
            this, SLOT(lboundProcessText()));
    mainLayout->addWidget(lbound, 0,1);

    uboundLabel = new QLabel("Upper bound", central, "uboundLabel");
    mainLayout->addWidget(uboundLabel,1,0);
    ubound = new QLineEdit(central, "ubound");
    connect(ubound, SIGNAL(returnPressed()),
            this, SLOT(uboundProcessText()));
    mainLayout->addWidget(ubound, 1,1);

    variableLabel = new QLabel("variable", central, "variableLabel");
    mainLayout->addWidget(variableLabel,2,0);
    variable = new QLineEdit(central, "variable");
    connect(variable, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(variable, 2,1);

}


// ****************************************************************************
// Method: QvisIsovolumeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//    Jeremy Meredith, Wed May  5 14:55:08 PDT 2004
//    Made it support "min" and "max" as legal values, respectively, for
//    the lower and upper bound fields.
//   
// ****************************************************************************

void
QvisIsovolumeWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for (int i = 0; i < atts->NumAttributes(); ++i)
    {
        if (!doAll)
        {
            if (!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch (i)
        {
          case 0: //lbound
            if (atts->GetLbound() == -1e+37)
                temp = "min";
            else
                temp.sprintf("%g", atts->GetLbound());
            lbound->setText(temp);
            break;
          case 1: //ubound
            if (atts->GetUbound() == +1e+37)
                temp = "max";
            else
                temp.sprintf("%g", atts->GetUbound());
            ubound->setText(temp);
            break;
          case 2: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisIsovolumeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Jan 30 14:50:02 PST 2004
//
// Modifications:
//    Jeremy Meredith, Wed May  5 14:55:08 PDT 2004
//    Made it support "min" and "max" as legal values, respectively, for
//    the lower and upper bound fields.
//   
// ****************************************************************************

void
QvisIsovolumeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do lbound
    if(which_widget == 0 || doAll)
    {
        temp = lbound->displayText().simplifyWhiteSpace();
        if (temp.latin1() == QString("min"))
            atts->SetLbound(-1e+37);
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
    if(which_widget == 1 || doAll)
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
    if(which_widget == 2 || doAll)
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
QvisIsovolumeWindow::lboundProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisIsovolumeWindow::uboundProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisIsovolumeWindow::variableProcessText()
{
    GetCurrentValues(2);
    Apply();
}


