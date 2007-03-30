#include "QvisTubeWindow.h"

#include <TubeAttributes.h>
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
// Method: QvisTubeWindow::QvisTubeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTubeWindow::QvisTubeWindow(const int type,
                         TubeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisTubeWindow::~QvisTubeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTubeWindow::~QvisTubeWindow()
{
}


// ****************************************************************************
// Method: QvisTubeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisTubeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("width", central, "widthLabel"),0,0);
    width = new QLineEdit(central, "width");
    connect(width, SIGNAL(returnPressed()),
            this, SLOT(widthProcessText()));
    mainLayout->addWidget(width, 0,1);

    mainLayout->addWidget(new QLabel("Fineness of tube", central, "finenessLabel"),1,0);
    fineness = new QLineEdit(central, "fineness");
    connect(fineness, SIGNAL(returnPressed()),
            this, SLOT(finenessProcessText()));
    mainLayout->addWidget(fineness, 1,1);

    capping = new QCheckBox("Cap Tubes?", central, "capping");
    connect(capping, SIGNAL(toggled(bool)),
            this, SLOT(cappingChanged(bool)));
    mainLayout->addWidget(capping, 2,0);

}


// ****************************************************************************
// Method: QvisTubeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisTubeWindow::UpdateWindow(bool doAll)
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
          case 0: //width
            temp.sprintf("%g", atts->GetWidth());
            width->setText(temp);
            break;
          case 1: //fineness
            temp.sprintf("%d", atts->GetFineness());
            fineness->setText(temp);
            break;
          case 2: //capping
            capping->setChecked(atts->GetCapping());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTubeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Oct 30 10:58:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisTubeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do width
    if(which_widget == 0 || doAll)
    {
        temp = width->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetWidth(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of width was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetWidth());
            Message(msg);
            atts->SetWidth(atts->GetWidth());
        }
    }

    // Do fineness
    if(which_widget == 1 || doAll)
    {
        temp = fineness->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if (val < 3 || val > 12)
            {
                msg.sprintf("The fineness of the tube is limited from 3 "
                    "(coarsest) to 12 (finest)");
                Message(msg);
                atts->SetFineness(atts->GetFineness());
            }
            else
            {
                atts->SetFineness(val);
            }
        }

        if(!okay)
        {
            msg.sprintf("The value of fineness was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetFineness());
            Message(msg);
            atts->SetFineness(atts->GetFineness());
        }
    }

    // Do capping
    if(which_widget == 2 || doAll)
    {
        // Nothing for capping
    }

}


//
// Qt Slot functions
//


void
QvisTubeWindow::widthProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisTubeWindow::finenessProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisTubeWindow::cappingChanged(bool val)
{
    atts->SetCapping(val);
    Apply();
}


