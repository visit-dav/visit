#include "QvisGlobalLineoutWindow.h"

#include <GlobalLineoutAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QNarrowLineEdit.h>
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
//   Kathleen Bonnell, Thu Jul 22 15:57:23 PDT 2004
//   Add createWindow checkbox and windowId line edit.
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");

    //
    // CreateWindow 
    //
    createWindow = new QCheckBox("Use 1st unused window\nor create new one.", 
                                  central, "createWindow");
    connect(createWindow, SIGNAL(toggled(bool)),
            this, SLOT(createWindowChanged(bool)));
    mainLayout->addMultiCellWidget(createWindow, 0,0,0,1);

    //
    // WindowId 
    //
    windowIdLabel = new QLabel("Window #", central, "windowIdLabel");
    mainLayout->addWidget(windowIdLabel,1,0);
    windowId = new QNarrowLineEdit(central, "windowId");
    connect(windowId, SIGNAL(returnPressed()),
            this, SLOT(windowIdProcessText()));
    mainLayout->addWidget(windowId, 1,1);


    //
    // Dynamic 
    //

    Dynamic = new QCheckBox("Dynamic", central, "Dynamic");
    connect(Dynamic, SIGNAL(toggled(bool)),
            this, SLOT(DynamicChanged(bool)));
    mainLayout->addWidget(Dynamic, 2,0);

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
//   Kathleen Bonnell, Thu Jul 22 15:57:23 PDT 2004
//   Update createWindow and windowId.
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::UpdateWindow(bool doAll)
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
          case 0: //Dynamic
            Dynamic->setChecked(atts->GetDynamic());
            break;
          case 1: //createWindow
            if (atts->GetCreateWindow() == false)
            {
                windowId->setEnabled(true);
                windowIdLabel->setEnabled(true);
            }
            else
            {
                windowId->setEnabled(false);
                windowIdLabel->setEnabled(false);
            }
            createWindow->setChecked(atts->GetCreateWindow());
            break;
          case 2: //windowId
            temp.sprintf("%d", atts->GetWindowId());
            windowId->setText(temp);
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
//   Kathleen Bonnell, Thu Jul 22 15:57:23 PDT 2004
//   Get createWindow and windowId.
//   
// ****************************************************************************

void
QvisGlobalLineoutWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do Dynamic
    if(which_widget == 0 || doAll)
    {
        // Nothing for Dynamic
    }

    // Do createWindow
    if(which_widget == 1 || doAll)
    {
        // Nothing for createWindow
    }

    // Do windowId
    if(which_widget == 2 || doAll)
    {
        temp = windowId->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetWindowId(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of windowId was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetWindowId());
            Message(msg);
            atts->SetWindowId(atts->GetWindowId());
        }
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


void
QvisGlobalLineoutWindow::createWindowChanged(bool val)
{
    atts->SetCreateWindow(val);
    Apply();
}


void
QvisGlobalLineoutWindow::windowIdProcessText()
{
    GetCurrentValues(2);
    Apply();
}




