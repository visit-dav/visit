#include "QvisCoordSwapWindow.h"

#include <CoordSwapAttributes.h>
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
// Method: QvisCoordSwapWindow::QvisCoordSwapWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Feb 2 15:47:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisCoordSwapWindow::QvisCoordSwapWindow(const int type,
                         CoordSwapAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisCoordSwapWindow::~QvisCoordSwapWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Feb 2 15:47:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisCoordSwapWindow::~QvisCoordSwapWindow()
{
}


// ****************************************************************************
// Method: QvisCoordSwapWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Wed Feb 2 15:47:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCoordSwapWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");


    newCoord1Label = new QLabel("New Coordinate 1?", central, "newCoord1Label");
    mainLayout->addWidget(newCoord1Label,0,0);
    newCoord1 = new QButtonGroup(central, "newCoord1");
    newCoord1->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *newCoord1Layout = new QHBoxLayout(newCoord1);
    newCoord1Layout->setSpacing(10);
    QRadioButton *newCoord1CoordCoord1 = new QRadioButton("Old Coordinate 1", newCoord1);
    newCoord1Layout->addWidget(newCoord1CoordCoord1);
    QRadioButton *newCoord1CoordCoord2 = new QRadioButton("Old Coordinate 2", newCoord1);
    newCoord1Layout->addWidget(newCoord1CoordCoord2);
    QRadioButton *newCoord1CoordCoord3 = new QRadioButton("Old Coordinate 3", newCoord1);
    newCoord1Layout->addWidget(newCoord1CoordCoord3);
    connect(newCoord1, SIGNAL(clicked(int)),
            this, SLOT(newCoord1Changed(int)));
    mainLayout->addWidget(newCoord1, 0,1);

    newCoord2Label = new QLabel("New Coordinate 2?", central, "newCoord2Label");
    mainLayout->addWidget(newCoord2Label,1,0);
    newCoord2 = new QButtonGroup(central, "newCoord2");
    newCoord2->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *newCoord2Layout = new QHBoxLayout(newCoord2);
    newCoord2Layout->setSpacing(10);
    QRadioButton *newCoord2CoordCoord1 = new QRadioButton("Old Coordinate 1", newCoord2);
    newCoord2Layout->addWidget(newCoord2CoordCoord1);
    QRadioButton *newCoord2CoordCoord2 = new QRadioButton("Old Coordinate 2", newCoord2);
    newCoord2Layout->addWidget(newCoord2CoordCoord2);
    QRadioButton *newCoord2CoordCoord3 = new QRadioButton("Old Coordinate 3", newCoord2);
    newCoord2Layout->addWidget(newCoord2CoordCoord3);
    connect(newCoord2, SIGNAL(clicked(int)),
            this, SLOT(newCoord2Changed(int)));
    mainLayout->addWidget(newCoord2, 1,1);

    newCoord3Label = new QLabel("New Coordinate 3", central, "newCoord3Label");
    mainLayout->addWidget(newCoord3Label,2,0);
    newCoord3 = new QButtonGroup(central, "newCoord3");
    newCoord3->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *newCoord3Layout = new QHBoxLayout(newCoord3);
    newCoord3Layout->setSpacing(10);
    QRadioButton *newCoord3CoordCoord1 = new QRadioButton("Old Coordinate 1", newCoord3);
    newCoord3Layout->addWidget(newCoord3CoordCoord1);
    QRadioButton *newCoord3CoordCoord2 = new QRadioButton("Old Coordinate 2", newCoord3);
    newCoord3Layout->addWidget(newCoord3CoordCoord2);
    QRadioButton *newCoord3CoordCoord3 = new QRadioButton("Old Coordinate 3", newCoord3);
    newCoord3Layout->addWidget(newCoord3CoordCoord3);
    connect(newCoord3, SIGNAL(clicked(int)),
            this, SLOT(newCoord3Changed(int)));
    mainLayout->addWidget(newCoord3, 2,1);

}


// ****************************************************************************
// Method: QvisCoordSwapWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Wed Feb 2 15:47:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCoordSwapWindow::UpdateWindow(bool doAll)
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
          case 0: //newCoord1
            newCoord1->setButton(atts->GetNewCoord1());
            break;
          case 1: //newCoord2
            newCoord2->setButton(atts->GetNewCoord2());
            break;
          case 2: //newCoord3
            newCoord3->setButton(atts->GetNewCoord3());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisCoordSwapWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Feb 2 15:47:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCoordSwapWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do newCoord1
    if(which_widget == 0 || doAll)
    {
        // Nothing for newCoord1
    }

    // Do newCoord2
    if(which_widget == 1 || doAll)
    {
        // Nothing for newCoord2
    }

    // Do newCoord3
    if(which_widget == 2 || doAll)
    {
        // Nothing for newCoord3
    }

    bool haveCoord1 = false;
    bool haveCoord2 = false;
    bool haveCoord3 = false;
    
    CoordSwapAttributes::Coord c[3];
    c[0] = atts->GetNewCoord1();
    c[1] = atts->GetNewCoord2();
    c[2] = atts->GetNewCoord3();

    for (int i = 0 ; i < 3 ; i++)
    {
        switch (c[i])
        {
            case CoordSwapAttributes::Coord1:
              haveCoord1 = true;
              break;
            case CoordSwapAttributes::Coord2:
              haveCoord2 = true;
              break;
            case CoordSwapAttributes::Coord3:
              haveCoord3 = true;
              break;
        }
    }

    if (!haveCoord1 || !haveCoord2 || !haveCoord3)
    {
        msg.sprintf("One coordinate is being used more than one time.  "
                    "Resetting to the initial state.");
        Warning(msg);
        atts->SetNewCoord1(CoordSwapAttributes::Coord1);
        atts->SetNewCoord2(CoordSwapAttributes::Coord2);
        atts->SetNewCoord3(CoordSwapAttributes::Coord3);
    }
}


//
// Qt Slot functions
//


void
QvisCoordSwapWindow::newCoord1Changed(int val)
{
    if(val != atts->GetNewCoord1())
    {
        atts->SetNewCoord1(CoordSwapAttributes::Coord(val));
        Apply();
    }
}


void
QvisCoordSwapWindow::newCoord2Changed(int val)
{
    if(val != atts->GetNewCoord2())
    {
        atts->SetNewCoord2(CoordSwapAttributes::Coord(val));
        Apply();
    }
}


void
QvisCoordSwapWindow::newCoord3Changed(int val)
{
    if(val != atts->GetNewCoord3())
    {
        atts->SetNewCoord3(CoordSwapAttributes::Coord(val));
        Apply();
    }
}


