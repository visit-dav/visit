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
#include <QvisVariableButton.h>

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
// Creation:   Mon Sep 26 09:31:48 PDT 2005
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
// Creation:   Mon Sep 26 09:31:48 PDT 2005
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
// Creation:   Mon Sep 26 09:31:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExternalSurfaceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    removeGhosts = new QCheckBox("Remove ghost faces?", central, "removeGhosts");
    connect(removeGhosts, SIGNAL(toggled(bool)),
            this, SLOT(removeGhostsChanged(bool)));
    mainLayout->addWidget(removeGhosts, 0,0);

    edgesIn2D = new QCheckBox("Find external edges for 2D datasets", central, "edgesIn2D");
    connect(edgesIn2D, SIGNAL(toggled(bool)),
            this, SLOT(edgesIn2DChanged(bool)));
    mainLayout->addWidget(edgesIn2D, 1,0);

}


// ****************************************************************************
// Method: QvisExternalSurfaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Sep 26 09:31:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExternalSurfaceWindow::UpdateWindow(bool doAll)
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
          case 0: //removeGhosts
            removeGhosts->setChecked(atts->GetRemoveGhosts());
            break;
          case 1: //edgesIn2D
            edgesIn2D->setChecked(atts->GetEdgesIn2D());
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
// Creation:   Mon Sep 26 09:31:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExternalSurfaceWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do removeGhosts
    if(which_widget == 0 || doAll)
    {
        // Nothing for removeGhosts
    }

    // Do edgesIn2D
    if(which_widget == 1 || doAll)
    {
        // Nothing for edgesIn2D
    }

}


//
// Qt Slot functions
//


void
QvisExternalSurfaceWindow::removeGhostsChanged(bool val)
{
    atts->SetRemoveGhosts(val);
    Apply();
}


void
QvisExternalSurfaceWindow::edgesIn2DChanged(bool val)
{
    atts->SetEdgesIn2D(val);
    Apply();
}


