#include "QvisProjectWindow.h"

#include <ProjectAttributes.h>
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
// Method: QvisProjectWindow::QvisProjectWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisProjectWindow::QvisProjectWindow(const int type,
                         ProjectAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisProjectWindow::~QvisProjectWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisProjectWindow::~QvisProjectWindow()
{
}


// ****************************************************************************
// Method: QvisProjectWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisProjectWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    projectionTypeLabel = new QLabel("projectionType", central, "projectionTypeLabel");
    mainLayout->addWidget(projectionTypeLabel,0,0);
    projectionType = new QButtonGroup(central, "projectionType");
    projectionType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *projectionTypeLayout = new QHBoxLayout(projectionType);
    projectionTypeLayout->setSpacing(10);
    QRadioButton *projectionTypeProjectionTypeXYCartesian = new QRadioButton("XYCartesian", projectionType);
    projectionTypeLayout->addWidget(projectionTypeProjectionTypeXYCartesian);
    QRadioButton *projectionTypeProjectionTypeZRCylindrical = new QRadioButton("ZRCylindrical", projectionType);
    projectionTypeLayout->addWidget(projectionTypeProjectionTypeZRCylindrical);
    connect(projectionType, SIGNAL(clicked(int)),
            this, SLOT(projectionTypeChanged(int)));
    mainLayout->addWidget(projectionType, 0,1);

}


// ****************************************************************************
// Method: QvisProjectWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisProjectWindow::UpdateWindow(bool doAll)
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
          case 0: //projectionType
            projectionType->setButton(atts->GetProjectionType());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisProjectWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisProjectWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do projectionType
    if(which_widget == 0 || doAll)
    {
        // Nothing for projectionType
    }

}


//
// Qt Slot functions
//


void
QvisProjectWindow::projectionTypeChanged(int val)
{
    if(val != atts->GetProjectionType())
    {
        atts->SetProjectionType(ProjectAttributes::ProjectionType(val));
        Apply();
    }
}


