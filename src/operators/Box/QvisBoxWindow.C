#include "QvisBoxWindow.h"

#include <BoxAttributes.h>
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
// Method: QvisBoxWindow::QvisBoxWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisBoxWindow::QvisBoxWindow(const int type,
                         BoxAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisBoxWindow::~QvisBoxWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisBoxWindow::~QvisBoxWindow()
{
}


// ****************************************************************************
// Method: QvisBoxWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoxWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,2,  10, "mainLayout");


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

    mainLayout->addWidget(new QLabel("X-Minimum", central, "minxLabel"),1,0);
    minx = new QLineEdit(central, "minx");
    connect(minx, SIGNAL(returnPressed()),
            this, SLOT(minxProcessText()));
    mainLayout->addWidget(minx, 1,1);

    mainLayout->addWidget(new QLabel("X-Maximum", central, "maxxLabel"),2,0);
    maxx = new QLineEdit(central, "maxx");
    connect(maxx, SIGNAL(returnPressed()),
            this, SLOT(maxxProcessText()));
    mainLayout->addWidget(maxx, 2,1);

    mainLayout->addWidget(new QLabel("Y-Minimum", central, "minyLabel"),3,0);
    miny = new QLineEdit(central, "miny");
    connect(miny, SIGNAL(returnPressed()),
            this, SLOT(minyProcessText()));
    mainLayout->addWidget(miny, 3,1);

    mainLayout->addWidget(new QLabel("Y-Maximum", central, "maxyLabel"),4,0);
    maxy = new QLineEdit(central, "maxy");
    connect(maxy, SIGNAL(returnPressed()),
            this, SLOT(maxyProcessText()));
    mainLayout->addWidget(maxy, 4,1);

    mainLayout->addWidget(new QLabel("Z-Minimum", central, "minzLabel"),5,0);
    minz = new QLineEdit(central, "minz");
    connect(minz, SIGNAL(returnPressed()),
            this, SLOT(minzProcessText()));
    mainLayout->addWidget(minz, 5,1);

    mainLayout->addWidget(new QLabel("Z-Maximum", central, "maxzLabel"),6,0);
    maxz = new QLineEdit(central, "maxz");
    connect(maxz, SIGNAL(returnPressed()),
            this, SLOT(maxzProcessText()));
    mainLayout->addWidget(maxz, 6,1);

}


// ****************************************************************************
// Method: QvisBoxWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoxWindow::UpdateWindow(bool doAll)
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
          case 1: //minx
            temp.sprintf("%g", atts->GetMinx());
            minx->setText(temp);
            break;
          case 2: //maxx
            temp.sprintf("%g", atts->GetMaxx());
            maxx->setText(temp);
            break;
          case 3: //miny
            temp.sprintf("%g", atts->GetMiny());
            miny->setText(temp);
            break;
          case 4: //maxy
            temp.sprintf("%g", atts->GetMaxy());
            maxy->setText(temp);
            break;
          case 5: //minz
            temp.sprintf("%g", atts->GetMinz());
            minz->setText(temp);
            break;
          case 6: //maxz
            temp.sprintf("%g", atts->GetMaxz());
            maxz->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisBoxWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoxWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do amount
    if(which_widget == 0 || doAll)
    {
        // Nothing for amount
    }

    // Do minx
    if(which_widget == 1 || doAll)
    {
        temp = minx->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMinx(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of minx was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMinx());
            Message(msg);
            atts->SetMinx(atts->GetMinx());
        }
    }

    // Do maxx
    if(which_widget == 2 || doAll)
    {
        temp = maxx->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxx(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxx was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxx());
            Message(msg);
            atts->SetMaxx(atts->GetMaxx());
        }
    }

    // Do miny
    if(which_widget == 3 || doAll)
    {
        temp = miny->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMiny(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of miny was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMiny());
            Message(msg);
            atts->SetMiny(atts->GetMiny());
        }
    }

    // Do maxy
    if(which_widget == 4 || doAll)
    {
        temp = maxy->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxy(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxy was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxy());
            Message(msg);
            atts->SetMaxy(atts->GetMaxy());
        }
    }

    // Do minz
    if(which_widget == 5 || doAll)
    {
        temp = minz->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMinz(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of minz was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMinz());
            Message(msg);
            atts->SetMinz(atts->GetMinz());
        }
    }

    // Do maxz
    if(which_widget == 6 || doAll)
    {
        temp = maxz->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMaxz(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxz was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMaxz());
            Message(msg);
            atts->SetMaxz(atts->GetMaxz());
        }
    }

}


//
// Qt Slot functions
//


void
QvisBoxWindow::amountChanged(int val)
{
    BoxAttributes::Amount val2;
    val2 = (val == 0) ? BoxAttributes::Some : BoxAttributes::All;

    if(val2 != atts->GetAmount())
    {
        atts->SetAmount(val2);
        Apply();
    }
}


void
QvisBoxWindow::minxProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisBoxWindow::maxxProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisBoxWindow::minyProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisBoxWindow::maxyProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisBoxWindow::minzProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisBoxWindow::maxzProcessText()
{
    GetCurrentValues(6);
    Apply();
}


