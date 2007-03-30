#include <stdio.h> // for sscanf

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <QvisInverseGhostZoneWindow.h>
#include <InverseGhostZoneAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::QvisInverseGhostZoneWindow
//
// Purpose: 
//   Cconstructor for the QvisInverseGhostZoneWindow class.
//
// Arguments:
//   subj      : The InverseGhostZoneAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Hank Childs
// Creation:   July 27, 2001
//
// Modifications:
//   
// ****************************************************************************

QvisInverseGhostZoneWindow::QvisInverseGhostZoneWindow(const int type,
    InverseGhostZoneAttributes *subj, const char *caption, 
    const char *shortName, QvisNotepadArea *notepad) :
    QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    igzAtts = subj;
}

// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::~QvisInverseGhostZoneWindow
//
// Purpose: 
//   This is the destructor for the QvisInverseGhostZoneWindow class.
//
// Programmer: Hank Childs
// Creation:   Fri Sep 15 11:36:26 PDT 2000
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
//   This method creates the widgets for the InverseGhostZone operator window.
//
// Programmer: Hank Childs
// Creation:   Fri Sep 15 11:37:08 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::CreateWindowContents()
{
    QGroupBox *planeBox = new QGroupBox(central, "planeBox");
    planeBox->setTitle("");
    topLayout->addWidget(planeBox);

    QGridLayout *planeLayout = new QGridLayout(planeBox, 1, 1);
    planeLayout->setMargin(10);
    planeLayout->setSpacing(5);
    
    // Add spacing to the top empty row to space it away from the QGroupBox label.
    planeLayout->addRowSpacing(0, 10);
    
    constantData = new QCheckBox("Make data constant?", planeBox, "constantData");
    planeLayout->addWidget(constantData, 1, 0);
    connect(constantData, SIGNAL(toggled(bool)), this, SLOT(constantDataToggled(bool)));
}

// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets to reflect changes made
//   in the InverseGhostZoneAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the InverseGhostZoneAttribute object's selected
//           states.
//
// Programmer: Hank Childs
// Creation:   Fri Sep 15 11:37:41 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < igzAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!igzAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 1: // constantData
            constantData->blockSignals(true);
            constantData->setChecked(igzAtts->GetConstantData());
            constantData->blockSignals(false);
            break;
        }
    } // end for
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisInverseGhostZoneWindow::constantDataToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the slice should be projected to 2d.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Hank Childs
// Creation:   Fri Sep 15 11:43:06 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisInverseGhostZoneWindow::constantDataToggled(bool val)
{
    igzAtts->SetConstantData(val);
    Apply();
}
