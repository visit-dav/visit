#include "QvisConnCompReduceWindow.h"

#include <ConnCompReduceAttributes.h>
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
// Method: QvisConnCompReduceWindow::QvisConnCompReduceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConnCompReduceWindow::QvisConnCompReduceWindow(const int type,
                         ConnCompReduceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::~QvisConnCompReduceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisConnCompReduceWindow::~QvisConnCompReduceWindow()
{
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConnCompReduceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Target Reduction ", central, "targetLabel"),0,0);
    target = new QLineEdit(central, "target");
    connect(target, SIGNAL(returnPressed()),
            this, SLOT(targetProcessText()));
    mainLayout->addWidget(target, 0,1);

}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConnCompReduceWindow::UpdateWindow(bool doAll)
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
          case 0: //target
            temp.sprintf("%g", atts->GetTarget());
            target->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisConnCompReduceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Oct 29 10:16:44 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisConnCompReduceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do target
    if(which_widget == 0 || doAll)
    {
        temp = target->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTarget(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of target was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTarget());
            Message(msg);
            atts->SetTarget(atts->GetTarget());
        }
    }

}


//
// Qt Slot functions
//


void
QvisConnCompReduceWindow::targetProcessText()
{
    GetCurrentValues(0);
    Apply();
}


