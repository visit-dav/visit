#include "QvisMergeWindow.h"

#include <MergeOperatorAttributes.h>
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
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisMergeWindow::QvisMergeWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Sep 22 16:56:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisMergeWindow::QvisMergeWindow(const int type,
                         MergeOperatorAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisMergeWindow::~QvisMergeWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Sep 22 16:56:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisMergeWindow::~QvisMergeWindow()
{
}


// ****************************************************************************
// Method: QvisMergeWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Sep 22 16:56:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    parallelMerge = new QCheckBox("Merge across all processors?", central, "parallelMerge");
    connect(parallelMerge, SIGNAL(toggled(bool)),
            this, SLOT(parallelMergeChanged(bool)));
    mainLayout->addWidget(parallelMerge, 0,0);

    toleranceLabel = new QLabel("Maximum distance between points that should be merged", central, "toleranceLabel");
    mainLayout->addWidget(toleranceLabel,1,0);
    tolerance = new QLineEdit(central, "tolerance");
    connect(tolerance, SIGNAL(returnPressed()),
            this, SLOT(toleranceProcessText()));
    mainLayout->addWidget(tolerance, 1,1);

}


// ****************************************************************************
// Method: QvisMergeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Sep 22 16:56:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::UpdateWindow(bool doAll)
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
          case 0: //parallelMerge
            parallelMerge->setChecked(atts->GetParallelMerge());
            break;
          case 1: //tolerance
            temp.setNum(atts->GetTolerance());
            tolerance->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisMergeWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Sep 22 16:56:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do parallelMerge
    if(which_widget == 0 || doAll)
    {
        // Nothing for parallelMerge
    }

    // Do tolerance
    if(which_widget == 1 || doAll)
    {
        temp = tolerance->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTolerance(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tolerance was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTolerance());
            Message(msg);
            atts->SetTolerance(atts->GetTolerance());
        }
    }

}


//
// Qt Slot functions
//


void
QvisMergeWindow::parallelMergeChanged(bool val)
{
    atts->SetParallelMerge(val);
    Apply();
}


void
QvisMergeWindow::toleranceProcessText()
{
    GetCurrentValues(1);
    Apply();
}


