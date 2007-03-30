#include "QvisExternalSurfaceWindow.h"

#include <ExternalSurfaceAttributes.h>
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
// Method: QvisExternalSurfaceWindow::QvisExternalSurfaceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisExternalSurfaceWindow::QvisExternalSurfaceWindow(const int type,
                         ExternalSurfaceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisExternalSurfaceWindow::~QvisExternalSurfaceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisExternalSurfaceWindow::~QvisExternalSurfaceWindow()
{
}


// ****************************************************************************
// Method: QvisExternalSurfaceWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Sun Aug  4 10:18:59 PDT 2002
//    Never added dummy widget to window.
//
// ****************************************************************************

void
QvisExternalSurfaceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    dummy = new QCheckBox("dummy", NULL, "dummy");
    connect(dummy, SIGNAL(toggled(bool)),
            this, SLOT(dummyChanged(bool)));
    //mainLayout->addWidget(dummy, 0,0);

}


// ****************************************************************************
// Method: QvisExternalSurfaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisExternalSurfaceWindow::UpdateWindow(bool doAll)
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
          case 0: //dummy
            dummy->setChecked(atts->GetDummy());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisExternalSurfaceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisExternalSurfaceWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
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
QvisExternalSurfaceWindow::dummyChanged(bool val)
{
    atts->SetDummy(val);
    Apply();
}


