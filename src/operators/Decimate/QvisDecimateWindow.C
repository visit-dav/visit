#include "QvisDecimateWindow.h"

#include <DecimateAttributes.h>
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
// Method: QvisDecimateWindow::QvisDecimateWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisDecimateWindow::QvisDecimateWindow(const int type,
                         DecimateAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisDecimateWindow::~QvisDecimateWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisDecimateWindow::~QvisDecimateWindow()
{
}


// ****************************************************************************
// Method: QvisDecimateWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisDecimateWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Target Reduction ", central, "targetLabel"),0,0);
    target = new QLineEdit(central, "target");
    connect(target, SIGNAL(returnPressed()),
            this, SLOT(targetProcessText()));
    mainLayout->addWidget(target, 0,1);

}


// ****************************************************************************
// Method: QvisDecimateWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisDecimateWindow::UpdateWindow(bool doAll)
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
            temp.setNum(atts->GetTarget());
            target->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisDecimateWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   
//   Hank Childs, Sun Aug 11 08:41:51 PDT 2002
//   Modified to print out an error message if the target is not within range.
//
// ****************************************************************************

void
QvisDecimateWindow::GetCurrentValues(int which_widget)
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
            if (val < 0. || val > 1.)
            {
                msg.sprintf("The reduction target field is only valid between 0"
                    " 1.  The number specified is the proportion of number of "
                    "polygonal cells in the outputted dataset \"over\" the "
                    "number of polygonal cells in the original dataset.  "
                    "Resetting to the last good value of %g.",
                    atts->GetTarget());
                Message(msg);
                atts->SetTarget(atts->GetTarget());
            }
            else
            {
                atts->SetTarget(val);
            }
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
QvisDecimateWindow::targetProcessText()
{
    GetCurrentValues(0);
    Apply();
}


