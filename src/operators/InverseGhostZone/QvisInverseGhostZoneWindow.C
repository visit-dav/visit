#include "QvisInverseGhostZoneWindow.h"

#include <InverseGhostZoneAttributes.h>
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
// Method: QvisInverseGhostZoneWindow::QvisInverseGhostZoneWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisInverseGhostZoneWindow::QvisInverseGhostZoneWindow(const int type,
                         InverseGhostZoneAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::~QvisInverseGhostZoneWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisInverseGhostZoneWindow::~QvisInverseGhostZoneWindow()
{
}


// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    showTypeLabel = new QLabel("Zones to Display:", central, "showTypeLabel");
    mainLayout->addWidget(showTypeLabel,0,0);
    showType = new QButtonGroup(central, "showType");
    showType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *showTypeLayout = new QHBoxLayout(showType);
    showTypeLayout->setSpacing(10);
    QRadioButton *showTypeShowTypeGhostZonesOnly = new QRadioButton("Ghost zones only", showType);
    showTypeLayout->addWidget(showTypeShowTypeGhostZonesOnly);
    QRadioButton *showTypeShowTypeGhostZonesAndRealZones = new QRadioButton("Both ghost zones and real zones", showType);
    showTypeLayout->addWidget(showTypeShowTypeGhostZonesAndRealZones);
    connect(showType, SIGNAL(clicked(int)),
            this, SLOT(showTypeChanged(int)));
    mainLayout->addWidget(showType, 0,1);

}


// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::UpdateWindow(bool doAll)
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
          case 0: //showType
            showType->setButton(atts->GetShowType());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do showType
    if(which_widget == 0 || doAll)
    {
        // Nothing for showType
    }

}


//
// Qt Slot functions
//


void
QvisInverseGhostZoneWindow::showTypeChanged(int val)
{
    if(val != atts->GetShowType())
    {
        atts->SetShowType(InverseGhostZoneAttributes::ShowType(val));
        Apply();
    }
}


