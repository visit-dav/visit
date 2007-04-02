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
// Creation:   Sun Aug 14 17:22:31 PST 2005
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
// Creation:   Sun Aug 14 17:22:31 PST 2005
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
// Creation:   Sun Aug 14 17:22:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    dummy = new QCheckBox("Unused field", central, "dummy");
    connect(dummy, SIGNAL(toggled(bool)),
            this, SLOT(dummyChanged(bool)));
    mainLayout->addWidget(dummy, 0,0);

}


// ****************************************************************************
// Method: QvisMergeWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sun Aug 14 17:22:31 PST 2005
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
          case 0: //dummy
            dummy->setChecked(atts->GetDummy());
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
// Creation:   Sun Aug 14 17:22:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMergeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do dummy
    if(which_widget == 0 || doAll)
    {
        // Nothing for dummy
    }

}


//
// Qt Slot functions
//


void
QvisMergeWindow::dummyChanged(bool val)
{
    atts->SetDummy(val);
    Apply();
}


