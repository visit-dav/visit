#include "QvisInteractorWindow.h"

#include <InteractorAttributes.h>
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
// Method: QvisInteractorWindow::QvisInteractorWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisInteractorWindow::QvisInteractorWindow(
                         InteractorAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisInteractorWindow::~QvisInteractorWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisInteractorWindow::~QvisInteractorWindow()
{
}


// ****************************************************************************
// Method: QvisInteractorWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::CreateWindowContents()
{

    QGroupBox *zoomGroup = new QGroupBox(central, "zoomGroup");
    zoomGroup->setTitle("Zoom interaction:");
    topLayout->addWidget(zoomGroup, 5);

    QVBoxLayout *zoomVBoxLayout = new QVBoxLayout(zoomGroup);
    zoomVBoxLayout->setMargin(10);
    zoomVBoxLayout->addSpacing(15);

    QGridLayout *zoomGridLayout = new QGridLayout(zoomVBoxLayout, 2, 2);
    zoomGridLayout->setMargin(10);

    showGuidelines = new QCheckBox("Show Guidelines", zoomGroup, "showGuidelines");
    connect(showGuidelines, SIGNAL(toggled(bool)),
            this, SLOT(showGuidelinesChanged(bool)));
    zoomGridLayout->addWidget(showGuidelines, 1,0);

    clampSquare = new QCheckBox("Clamp to Square", zoomGroup, "clampSquare");
    connect(clampSquare, SIGNAL(toggled(bool)),
            this, SLOT(clampSquareChanged(bool)));
    zoomGridLayout->addWidget(clampSquare, 2,0);
}


// ****************************************************************************
// Method: QvisInteractorWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::UpdateWindow(bool doAll)
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
          case 0: //showGuidelines
            showGuidelines->setChecked(atts->GetShowGuidelines());
            break;
          case 1: //clampSquare
            clampSquare->setChecked(atts->GetClampSquare());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisInteractorWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do showGuidelines
    if(which_widget == 0 || doAll)
    {
        // Nothing for showGuidelines
    }

    // Do clampSquare
    if(which_widget == 1 || doAll)
    {
        // Nothing for clampSquare
    }

}


// ****************************************************************************
// Method: QvisInteractorWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();
        viewer->SetInteractorAttributes();
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisInteractorWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisInteractorWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    viewer->SetDefaultInteractorAttributes();
}


// ****************************************************************************
// Method: QvisInteractorWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInteractorWindow::reset()
{
    viewer->ResetInteractorAttributes();
}


void
QvisInteractorWindow::showGuidelinesChanged(bool val)
{
    atts->SetShowGuidelines(val);
    SetUpdate(false);
    Apply();
}


void
QvisInteractorWindow::clampSquareChanged(bool val)
{
    atts->SetClampSquare(val);
    SetUpdate(false);
    Apply();
}


