#include "QvisGlobalLineoutWindow.h"

#include <GlobalLineoutAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <stdio.h>

// ****************************************************************************
// Method: QvisGlobalLineoutWindow::QvisGlobalLineoutWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisGlobalLineoutWindow::QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                         QvisPostableWindowObserver::ApplyButton, false)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisGlobalLineoutWindow::~QvisGlobalLineoutWindow()
{
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    Dynamic = new QCheckBox("Dynamic", central, "Dynamic");
    connect(Dynamic, SIGNAL(toggled(bool)),
            this, SLOT(DynamicChanged(bool)));
    mainLayout->addWidget(Dynamic, 0,0);

}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::UpdateWindow(bool doAll)
{
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
          case 0: //Dynamic
            Dynamic->setChecked(atts->GetDynamic());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do Dynamic
    if(which_widget == 0 || doAll)
    {
        // Nothing for Dynamic
    }

}


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();
        viewer->SetGlobalLineoutAttributes();
    }
    else
    {
        atts->Notify();
    }
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisGlobalLineoutWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::apply()
{
    Apply(true);
}


void
QvisGlobalLineoutWindow::DynamicChanged(bool val)
{
    atts->SetDynamic(val);
    Apply();
}


