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
//   Eric Brugger, Thu Nov 11 11:49:41 PST 2004
//   I added the navigation mode toggle buttons.
//   
//   Eric Brugger, Thu Nov 18 13:02:43 PST 2004
//   I added the fill viewport on zoom toggle button.
//
//   Eric Brugger, Mon Dec 27 11:50:27 PST 2004
//   I added a dolly navigation mode.
//
// ****************************************************************************

void
QvisInteractorWindow::CreateWindowContents()
{

    QGroupBox *zoomGroup = new QGroupBox(central, "zoomGroup");
    zoomGroup->setTitle("Zoom interaction:");
    topLayout->addWidget(zoomGroup);

    QVBoxLayout *zoomVBoxLayout = new QVBoxLayout(zoomGroup);
    zoomVBoxLayout->addSpacing(10);

    QGridLayout *zoomGridLayout = new QGridLayout(zoomVBoxLayout, 3, 2);
    zoomVBoxLayout->setSpacing(5);
    zoomGridLayout->setMargin(10);

    showGuidelines = new QCheckBox("Show Guidelines", zoomGroup, "showGuidelines");
    connect(showGuidelines, SIGNAL(toggled(bool)),
            this, SLOT(showGuidelinesChanged(bool)));
    zoomGridLayout->addWidget(showGuidelines, 1,0);

    clampSquare = new QCheckBox("Clamp to Square", zoomGroup, "clampSquare");
    connect(clampSquare, SIGNAL(toggled(bool)),
            this, SLOT(clampSquareChanged(bool)));
    zoomGridLayout->addWidget(clampSquare, 2,0);

    fillViewportOnZoom = new QCheckBox("Fill viewport on zoom", zoomGroup, "fillViewportOnZoom");
    connect(fillViewportOnZoom, SIGNAL(toggled(bool)),
            this, SLOT(fillViewportOnZoomChanged(bool)));
    zoomGridLayout->addWidget(fillViewportOnZoom, 3,0);

    QGroupBox *navigationGroup = new QGroupBox(central, "navigationGroup");
    navigationGroup->setTitle("Navigation mode:");
    topLayout->addWidget(navigationGroup);

    QVBoxLayout *navigationVBoxLayout = new QVBoxLayout(navigationGroup);
    navigationVBoxLayout->addSpacing(10);

    QGridLayout *navigationLayout = new QGridLayout(navigationVBoxLayout,
                                                    1, 3);
    navigationLayout->setSpacing(5);
    navigationLayout->setMargin(10);

    navigationMode = new QButtonGroup(0, "navigationMode");
    connect(navigationMode, SIGNAL(clicked(int)),
            this, SLOT(navigationModeChanged(int)));
    QRadioButton *trackball = new QRadioButton("Trackball", navigationGroup,
                                               "Trackball");
    navigationMode->insert(trackball);
    navigationLayout->addWidget(trackball, 1, 1);
    QRadioButton *dolly = new QRadioButton("Dolly", navigationGroup,
                                           "Dolly");
    navigationMode->insert(dolly);
    navigationLayout->addWidget(dolly, 1, 2);
    QRadioButton *flythrough = new QRadioButton("Flythrough", navigationGroup,
                                                "Flythrough");
    navigationMode->insert(flythrough);
    navigationLayout->addWidget(flythrough, 1, 3);
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
//   Eric Brugger, Thu Nov 11 11:49:41 PST 2004
//   I added the navigation mode toggle buttons.
//   
//   Eric Brugger, Thu Nov 18 13:02:43 PST 2004
//   I added the fill viewport on zoom toggle button.
//
//   Eric Brugger, Mon Dec 27 11:50:27 PST 2004
//   I added a dolly navigation mode.
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
          case 2: //fillViewportOnZoom
            fillViewportOnZoom->setChecked(atts->GetFillViewportOnZoom());
            break;
          case 3: //navigationMode
            if (atts->GetNavigationMode() == InteractorAttributes::Trackball)
                navigationMode->setButton(0);
            else if (atts->GetNavigationMode() == InteractorAttributes::Dolly)
                navigationMode->setButton(1);
            else
                navigationMode->setButton(2);
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
//   Eric Brugger, Thu Nov 11 11:49:41 PST 2004
//   I added the navigation mode toggle buttons.
//   
//   Eric Brugger, Thu Nov 18 13:02:43 PST 2004
//   I added the fill viewport on zoom toggle button.
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

    // Do fillViewportOnZoom
    if(which_widget == 2 || doAll)
    {
        // Nothing for fillViewportOnZoom
    }

    // Do navigateMode
    if(which_widget == 3 || doAll)
    {
        // Nothing for navigateMode
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


void
QvisInteractorWindow::fillViewportOnZoomChanged(bool val)
{
    atts->SetFillViewportOnZoom(val);
    SetUpdate(false);
    Apply();
}


void
QvisInteractorWindow::navigationModeChanged(int val)
{
    if (val == 0)
        atts->SetNavigationMode(InteractorAttributes::Trackball);
    else if (val == 1)
        atts->SetNavigationMode(InteractorAttributes::Dolly);
    else
        atts->SetNavigationMode(InteractorAttributes::Flythrough);
    SetUpdate(false);
    Apply();
}


