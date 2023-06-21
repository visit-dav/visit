// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisBoundaryOpWindow.h"

#include <BoundaryOpAttributes.h>

#include <QLabel>
#include <QLayout>
#include <QButtonGroup>
#include <QRadioButton>


// ****************************************************************************
// Method: QvisBoundaryOpWindow::QvisBoundaryOpWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisBoundaryOpWindow::QvisBoundaryOpWindow(const int type,
                         BoundaryOpAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::~QvisBoundaryOpWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisBoundaryOpWindow::~QvisBoundaryOpWindow()
{
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   Brad Whitlock, Fri Apr 25 09:47:48 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Mon Aug 18 21:11:25 PDT 2008
//   Qt4 Port.  
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisBoundaryOpWindow::CreateWindowContents()
{
    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#else
    connect(smoothingLevelButtons, SIGNAL(idClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#endif
    QGridLayout *smoothingLayout = new QGridLayout();
    topLayout->addLayout(smoothingLayout);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColumnStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel(tr("Geometry smoothing"), central), 0,0);
    QRadioButton *rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb,0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb,1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb,2);
    smoothingLayout->addWidget(rb, 0, 3);
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 15:11:27 PDT 2008
//   Removed unreferenced variables.
//
//   Cyrus Harrison, Mon Aug 18 21:11:25 PDT 2008
//   Qt4 Port.  
//
// ****************************************************************************

void
QvisBoundaryOpWindow::UpdateWindow(bool doAll)
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
            case BoundaryOpAttributes::ID_smoothingLevel:
                smoothingLevelButtons->blockSignals(true);
        smoothingLevelButtons->button(atts->GetSmoothingLevel())
                                                            ->setChecked(true);
                smoothingLevelButtons->blockSignals(false);
                break;
        }
    }
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryOpWindow::GetCurrentValues(int which_widget)
{
}


//
// Qt Slot functions
//

// ****************************************************************************
//  Method:  QvisBoundaryPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer:  Gunther H. Weber
//  Creation:    August 14, 2007
//
//  Note:  Taken verbatim from the Boundary plot
//
//  Modifications:
//
// ****************************************************************************

void
QvisBoundaryOpWindow::smoothingLevelChanged(int level)
{
    atts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}
