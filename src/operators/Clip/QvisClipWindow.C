#include <stdio.h> // for sscanf

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qhbox.h>

#include <QvisClipWindow.h>
#include <ClipAttributes.h>
#include <ViewerProxy.h>


// ****************************************************************************
// Method: QvisClipWindow::QvisClipWindow
//
// Purpose: 
//   Constructor for the QvisClipWindow class.
//
// Arguments:
//   subj      : The ClipAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   Brad Whitlock, Fri Apr 12 13:05:23 PST 2002
//   Made it inherit from QvisOperatorWindow.
//
// ****************************************************************************

QvisClipWindow::QvisClipWindow(const int type,
    ClipAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    clipAtts = subj;
}

// ****************************************************************************
// Method: QvisClipWindow::~QvisClipWindow
//
// Purpose: 
//   This is the destructor for the QvisClipWindow class.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//   
// ****************************************************************************

QvisClipWindow::~QvisClipWindow()
{
}

// ****************************************************************************
// Method: QvisClipWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the Clip operator window.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Use one temp label for all labels (removes compile warning).
//   Reduced spacing between parts. Removed code that set current
//   page for tabWidget.
//   
//   Kathleen Bonnell, Tue Apr  2 16:44:30 PST 2002 
//   Removed UMR's that came about from labels setting a 'buddy' widget
//   before that widget had been created. 
//   
// ****************************************************************************

void
QvisClipWindow::CreateWindowContents()
{
    QLabel *tempLabel;

    tabWidget = new QTabWidget(central, "tabWidget");
    connect(tabWidget, SIGNAL(currentChanged(QWidget *)),
            this, SLOT(tabWidgetChanged(QWidget *)));
    topLayout->addWidget(tabWidget);
    // 
    // Plane tab 
    // 
    planeBox = new QVBox(central, "planeBox");
    tabWidget->addTab(planeBox, "Plane(s)");
    planeBox->setMargin(10);

    // Plane1 Group
    QVGroupBox *plane1GBox = new QVGroupBox(planeBox, "p1GBox");

    // Create a group of radio buttons for Plane1 Status
    plane1Status = new QButtonGroup(plane1GBox, "plane1StatusRG" );
    plane1Status->setFrameStyle(QFrame::NoFrame);

    QHBoxLayout *plane1StatusButtonsLayout =
        new QHBoxLayout(plane1Status, 0, -1, "plane1StatusButtonsLayout");
    plane1StatusButtonsLayout->setSpacing(10);
    plane1StatusButtonsLayout->setMargin(5);
    tempLabel = new QLabel("Plane 1     ", plane1Status, "p1Label");
    plane1StatusButtonsLayout->addWidget(tempLabel);
    QRadioButton *rb = new QRadioButton("Off", plane1Status );
    plane1StatusButtonsLayout->addWidget(rb);
    rb = new QRadioButton( plane1Status );
    rb->setText( "On" );
    rb->setChecked( TRUE );
    plane1StatusButtonsLayout->addWidget(rb);
    plane1StatusButtonsLayout->addStretch();
    // Each time a radio button is clicked, call the plane1StatusClicked slot.
    connect(plane1Status, SIGNAL(clicked(int)),
            this, SLOT(plane1StatusClicked(int)));

    // Plane1 Origin 
    QHBox *p1OrigBox = new QHBox(plane1GBox, "p1OrigBox");
    p1OrigBox->setMargin(5);
    p1OrigBox->setSpacing(10);

    tempLabel = new QLabel("Origin", p1OrigBox, "plane1OriginLabel");
    plane1Origin = new QLineEdit(p1OrigBox, "plane1Origin");
    connect(plane1Origin, SIGNAL(returnPressed()),
            this, SLOT(processPlane1Origin()));

    // Plane1 Normal 
    QHBox *p1NormBox = new QHBox(plane1GBox, "p1NormBox");
    p1NormBox->setMargin(5);
    p1NormBox->setSpacing(10);
    tempLabel = new QLabel("Normal", p1NormBox, "plane1NormalLabel");
    plane1Normal = new QLineEdit(p1NormBox, "plane1Normal");
    connect(plane1Normal, SIGNAL(returnPressed()),
            this, SLOT(processPlane1Normal()));

    

    // Plane2 Group
    QVGroupBox *plane2GBox = new QVGroupBox(planeBox, "p2GBox");
    plane2GBox->setMargin(5);


    // Create a group of radio buttons for Plane2 Status
    plane2Status = new QButtonGroup(plane2GBox, "plane2StatusRG" );
    plane2Status->setFrameStyle(QFrame::NoFrame);

    QHBoxLayout *plane2StatusButtonsLayout =
        new QHBoxLayout(plane2Status, 0, -1, "plane2StatusButtonsLayout");
    plane2StatusButtonsLayout->setSpacing(10);
    plane2StatusButtonsLayout->setMargin(5);
    tempLabel = new QLabel("Plane 2     ", plane2Status, "p2Label");
    plane2StatusButtonsLayout->addWidget(tempLabel);
    rb = new QRadioButton("Off", plane2Status );
    rb->setChecked( TRUE );
    plane2StatusButtonsLayout->addWidget(rb);
    rb = new QRadioButton( plane2Status );
    rb->setText( "On" );
    plane2StatusButtonsLayout->addWidget(rb);
    plane2StatusButtonsLayout->addStretch();
    // Each time a radio button is clicked, call the plane2StatusClicked slot.
    connect(plane2Status, SIGNAL(clicked(int)),
            this, SLOT(plane2StatusClicked(int)));

    // Plane2 Origin 
    QHBox *p2OrigBox = new QHBox(plane2GBox, "p2OrigBox");
    p2OrigBox->setMargin(5);
    p2OrigBox->setSpacing(10);

    tempLabel = new QLabel("Origin", p2OrigBox, "plane2OriginLabel");
    plane2Origin = new QLineEdit(p2OrigBox, "plane2Origin");
    connect(plane2Origin, SIGNAL(returnPressed()),
            this, SLOT(processPlane2Origin()));

    // Plane2 Normal 
    QHBox *p2NormBox = new QHBox(plane2GBox, "p2NormBox");
    p2NormBox->setMargin(5);
    p2NormBox->setSpacing(10);
    tempLabel = new QLabel("Normal", p2NormBox, "plane2NormalLabel");
    plane2Normal = new QLineEdit(p2NormBox, "plane2Normal");
    connect(plane2Normal, SIGNAL(returnPressed()),
            this, SLOT(processPlane2Normal()));


    // Plane3 Group
    QVGroupBox *plane3GBox = new QVGroupBox(planeBox, "p3GBox");

    // Create a group of radio buttons for Plane3 Status
    plane3Status = new QButtonGroup(plane3GBox, "plane3StatusRG" );
    plane3Status->setFrameStyle(QFrame::NoFrame);

    QHBoxLayout *plane3StatusButtonsLayout =
        new QHBoxLayout(plane3Status, 0, -1, "plane3StatusButtonsLayout");
    plane3StatusButtonsLayout->setSpacing(10);
    plane3StatusButtonsLayout->setMargin(5);
    tempLabel = new QLabel("Plane 3     ", plane3Status, "p3Label");
    plane3StatusButtonsLayout->addWidget(tempLabel);
    rb = new QRadioButton("Off", plane3Status );
    rb->setChecked( TRUE );
    plane3StatusButtonsLayout->addWidget(rb);
    rb = new QRadioButton( plane3Status );
    rb->setText( "On" );
    plane3StatusButtonsLayout->addWidget(rb);
    plane3StatusButtonsLayout->addStretch();
    // Each time a radio button is clicked, call the plane3StatusClicked slot.
    connect(plane3Status, SIGNAL(clicked(int)),
            this, SLOT(plane3StatusClicked(int)));

    // Plane3 Origin 
    QHBox *p3OrigBox = new QHBox(plane3GBox, "p3OrigBox");
    p3OrigBox->setMargin(5);
    p3OrigBox->setSpacing(10);

    tempLabel = new QLabel("Origin", p3OrigBox, "plane3OriginLabel");
    plane3Origin = new QLineEdit(p3OrigBox, "plane3Origin");
    connect(plane3Origin, SIGNAL(returnPressed()),
            this, SLOT(processPlane3Origin()));

    // Plane3 Normal 
    QHBox *p3NormBox = new QHBox(plane3GBox, "p3NormBox");
    p3NormBox->setMargin(5);
    p3NormBox->setSpacing(10);
    tempLabel = new QLabel("Normal", p3NormBox, "plane3NormalLabel");
    plane3Normal = new QLineEdit(p3NormBox, "plane3Normal");
    connect(plane3Normal, SIGNAL(returnPressed()),
            this, SLOT(processPlane3Normal()));
    p3NormBox->setStretchFactor(plane3Normal, 20);

    // Plane Inverse
    QHBox *planeInvBox = new QHBox(planeBox, "planeInvBox");
    planeInvBox->setMargin(5);
    planeInvBox->setSpacing(5);

    tempLabel = new QLabel("Inverse ", planeInvBox, "pInvLabel");
    planeInverse = new QCheckBox(planeInvBox, "planeInverse");
    connect(planeInverse, SIGNAL(toggled(bool)),
            this, SLOT(planeInverseToggled(bool)));
    planeInverse->setChecked(false);
    planeInvBox->setStretchFactor(planeInverse, 20);


    // 
    // Sphere tab
    // 
    sphereBox = new QVBox(central, "sphereBox");
    tabWidget->addTab(sphereBox, "Sphere");

    // Sphere Center 
    QHBox *centerBox = new QHBox(sphereBox, "centerBox");
    centerBox->setMargin(10);
    centerBox->setSpacing(10);

    tempLabel = new QLabel("Center", centerBox, "centerLabel");
    centerLineEdit = new QLineEdit(centerBox, "center");
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));

    // Sphere Radius 
    QHBox *radiusBox = new QHBox(sphereBox, "radiusBox");
    radiusBox->setMargin(10);
    radiusBox->setSpacing(10);
    tempLabel = new QLabel("Radius", radiusBox, "radiusLabel");
    radiusLineEdit = new QLineEdit(radiusBox, "radius");
    connect(radiusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRadiusText()));

    // Sphere Inverse
    QHBox *sphereInvBox = new QHBox(sphereBox, "sphereInvBox");
    sphereInvBox->setMargin(10);
    sphereInvBox->setSpacing(5);

    tempLabel = new QLabel("Inverse ", sphereInvBox, "sInvLabel");
    sphereInverse = new QCheckBox(sphereInvBox, "sphereInverse");
    connect(sphereInverse, SIGNAL(toggled(bool)),
            this, SLOT(sphereInverseToggled(bool)));
    sphereInverse->setChecked(false);
    sphereInvBox->setStretchFactor(sphereInverse, 20);

    QHBox *sphereDummy = new QHBox(sphereBox, "sphereDummy");
    sphereBox->setStretchFactor(sphereDummy, 20);
}

// ****************************************************************************
// Method: QvisClipWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets to reflect changes made
//   in the ClipAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the ClipAttribute object's selected
//           states.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Fleshed out the update for FuncType so that window gets updated
//   properly.
//   
//   Jeremy Meredith, Fri Aug 17 13:45:17 PDT 2001
//   Fixed a copy and paste typo.
//
//   Brad Whitlock, Fri Feb 15 11:53:47 PDT 2002
//   Fixed format strings.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisClipWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < clipAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!clipAtts->IsSelected(i))
            {
                continue;
            }
        }

        double *dptr;
        switch(i)
        {
        case 0: // funcType
            tabWidget->blockSignals(true);
            if (clipAtts->GetFuncType() == ClipAttributes::Plane)
            {
                plane1Status->setButton(clipAtts->GetPlane1Status());
                plane2Status->setButton(clipAtts->GetPlane2Status());
                plane3Status->setButton(clipAtts->GetPlane3Status());
                dptr = clipAtts->GetPlane1Origin();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane1Origin->setText(temp);
                dptr = clipAtts->GetPlane2Origin();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane2Origin->setText(temp);
                dptr = clipAtts->GetPlane3Origin();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane3Origin->setText(temp);
                dptr = clipAtts->GetPlane1Normal();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane1Normal->setText(temp);
                dptr = clipAtts->GetPlane2Normal();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane2Normal->setText(temp);
                dptr = clipAtts->GetPlane3Normal();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                plane3Normal->setText(temp);
                planeInverse->setChecked(clipAtts->GetPlaneInverse());
                tabWidget->showPage(planeBox);
            }
            else if (clipAtts->GetFuncType() == ClipAttributes::Sphere)
            {
                dptr = clipAtts->GetCenter();
                temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
                centerLineEdit->setText(temp);
                r = clipAtts->GetRadius();
                temp.setNum(r);
                radiusLineEdit->setText(temp);
                sphereInverse->setChecked(clipAtts->GetSphereInverse());
                tabWidget->showPage(sphereBox);
            }
            tabWidget->blockSignals(false);
            break;
        case 1: // plane1Status
            plane1Status->setButton(clipAtts->GetPlane1Status());
            break;
        case 2: // plane2Status
            plane2Status->setButton(clipAtts->GetPlane2Status());
            break;
        case 3: // plane3Status
            plane3Status->setButton(clipAtts->GetPlane3Status());
            break;
        case 4: // plane1Origin
            dptr = clipAtts->GetPlane1Origin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane1Origin->setText(temp);
            break;
        case 5: // plane2Origin
            dptr = clipAtts->GetPlane2Origin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane2Origin->setText(temp);
            break;
        case 6: // plane3Origin
            dptr = clipAtts->GetPlane3Origin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane3Origin->setText(temp);
            break;
        case 7: // plane1Normal
            dptr = clipAtts->GetPlane1Normal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane1Normal->setText(temp);
            break;
        case 8: // plane2Normal
            dptr = clipAtts->GetPlane2Normal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane2Normal->setText(temp);
            break;
        case 9: // plane3Normal
            dptr = clipAtts->GetPlane3Normal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            plane3Normal->setText(temp);
            break;
        case 10: // planeInverse
            planeInverse->setChecked(clipAtts->GetPlaneInverse());
            break;
        case 11: // center
            dptr = clipAtts->GetCenter();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            centerLineEdit->setText(temp);
            break;
        case 12: // radius
            r = clipAtts->GetRadius();
            temp.setNum(r);
            radiusLineEdit->setText(temp);
            break;
        case 13: // sphereInverse
            sphereInverse->setChecked(clipAtts->GetSphereInverse());
            break;
        }
   } // end for
}

// ****************************************************************************
// Method: QvisClipWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Kathleen Bonnell
// Creation:   Mon Sep 25 15:11:42 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Aug 17 13:45:27 PDT 2001
//   Fixed a possible free of memory we don't own.
//
//   Brad Whitlock, Fri Feb 15 11:54:50 PDT 2002
//   Fixed format strings.
//
//   Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003  
//   Disallow (0, 0, 0) for a normal vector.
// 
// ****************************************************************************

void
QvisClipWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;
    double *d = new double[3];

    // Do the plane 1 origin vector
    if(which_widget == 0 || doAll)
    {
        temp = plane1Origin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
                clipAtts->SetPlane1Origin(d);
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane1Origin();
            msg.sprintf("The origin vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane1Origin(d);
        }
    }

    // Do the plane 2 origin vector
    if(which_widget == 1 || doAll)
    {
        temp = plane2Origin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
                clipAtts->SetPlane2Origin(d);
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane2Origin();
            msg.sprintf("The origin vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane2Origin(d);
        }
    }


    // Do the plane 3 origin vector
    if(which_widget == 2 || doAll)
    {
        temp = plane3Origin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
                clipAtts->SetPlane3Origin(d);
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane3Origin();
            msg.sprintf("The origin vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane3Origin(d);
        }
    }

    // Do the plane1 normal vector
    if(which_widget == 3 || doAll)
    {
        temp = plane1Normal->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
            {
                okay = (d[0] != 0. || d[1] != 0. || d[2] != 0.);
                if(okay)
                    clipAtts->SetPlane1Normal(d);
            }
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane1Normal();
            msg.sprintf("The normal vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                 d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane1Normal(d);
        }
    }

    // Do the plane2 normal vector
    if(which_widget == 4 || doAll)
    {
        temp = plane2Normal->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
            {
                okay = (d[0] != 0. || d[1] != 0. || d[2] != 0.);
                if(okay)
                    clipAtts->SetPlane2Normal(d);
            }
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane2Normal();
            msg.sprintf("The normal vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                 d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane2Normal(d);
        }
    }

    // Do the plane3 normal vector
    if(which_widget == 5 || doAll)
    {
        temp = plane3Normal->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
            {
                okay = (d[0] != 0. || d[1] != 0. || d[2] != 0.);
                if(okay)
                    clipAtts->SetPlane3Normal(d);
            }
        }

        if(!okay)
        {
            const double *d = clipAtts->GetPlane3Normal();
            msg.sprintf("The normal vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                 d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetPlane3Normal(d);
        }
    }

    // Do the center vector
    if(which_widget == 6 || doAll)
    {
        temp = centerLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            d[0] = d[1] = d[2] = 0;
            okay = (sscanf(temp.latin1(), "%lg %lg %lg", 
                           &d[0], &d[1], &d[2]) == 3);
            if(okay)
                clipAtts->SetCenter(d);
        }

        if(!okay)
        {
            const double *d = clipAtts->GetCenter();
            msg.sprintf("The center was invalid. "
                "Resetting to the last good value <%g %g %g>.", 
                 d[0], d[1], d[2]);
            Message(msg);
            clipAtts->SetCenter(d);
        }
    }

    // Do the radius vector
    if(which_widget == 7 || doAll)
    {
        temp = radiusLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double r;
            okay = (sscanf(temp.latin1(), "%lg", &r) == 1);
            if(okay)
                clipAtts->SetRadius(r);
        }

        if(!okay)
        {
            double r = clipAtts->GetRadius();
            msg.sprintf("The radius was invalid. "
                "Resetting to the last good value <%g>.", r); 
            Message(msg);
            clipAtts->SetRadius(r);
        }
    }
    delete [] d;
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisClipWindow::processPlane1Origin
//
// Purpose: 
//   This is a Qt slot function that sets an origin vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane1Origin()
{
    GetCurrentValues(0);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::processPlane2Origin
//
// Purpose: 
//   This is a Qt slot function that sets an origin vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane2Origin()
{
    GetCurrentValues(1);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::processPlane3Origin
//
// Purpose: 
//   This is a Qt slot function that sets an origin vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane3Origin()
{
    GetCurrentValues(2);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::processPlane1Normal
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane1Normal()
{
    GetCurrentValues(3);
    Apply();
}

// ****************************************************************************
// Method: QvisClipWindow::processPlane2Normal
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane2Normal()
{
    GetCurrentValues(4);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::processPlane3Normal
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processPlane3Normal()
{
    GetCurrentValues(5);
    Apply();
}

// ****************************************************************************
// Method: QvisClipWindow::processCenterText
//
// Purpose: 
//   This is a Qt slot function that sets the center vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processCenterText()
{
    GetCurrentValues(6);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::processRadiusText
//
// Purpose: 
//   This is a Qt slot function that sets the upAxis vector.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::processRadiusText()
{
    GetCurrentValues(7);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::tabWidgetChanged
//
// Purpose:
//   This is a Qt slot function that is called when user selects a different 
//   tab in the tab widget.
//
// Arguments:
//   which : The currently active tab.
//
// Programmer: Kathleen Bonnell  
// Creation:   May 7, 2001 
//
// Modifications:
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
//   Brad Whitlock, Thu Oct 31 11:53:01 PDT 2002
//   Changed to work with the updated state object.
//
// ****************************************************************************

void
QvisClipWindow::tabWidgetChanged(QWidget *which)
{
    if (which == planeBox)
       clipAtts->SetFuncType(ClipAttributes::Plane);
    else
       clipAtts->SetFuncType(ClipAttributes::Sphere);

    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::plane1StatusClicked
//
// Purpose:
//   This is a Qt slot function that is called when a status button is clicked.
//
// Arguments:
//   button  :  Which status button was selected.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::plane1StatusClicked(int button)
{
    clipAtts->SetPlane1Status(button);
    Apply();
}

// ****************************************************************************
// Method: QvisClipWindow::plane2StatusClicked
//
// Purpose:
//   This is a Qt slot function that is called when a status button is clicked.
//
// Arguments:
//   button  :  Which status button was selected.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::plane2StatusClicked(int button)
{
    clipAtts->SetPlane2Status(button);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::plane3StatusClicked
//
// Purpose:
//   This is a Qt slot function that is called when a status button is clicked.
//
// Arguments:
//   button  :  Which status button was selected.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::plane3StatusClicked(int button)
{
    clipAtts->SetPlane3Status(button);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::planeInverseToggled
//
// Purpose:
//   This is a Qt slot function that is called when the plane inverse
//   button is toggled.
//
// Arguments:
//   val  :  On/off.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::planeInverseToggled(bool val)
{
    clipAtts->SetPlaneInverse(val);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::sphereInverseToggled
//
// Purpose:
//   This is a Qt slot function that is called when the sphere inverse
//   button is toggled.
//
// Arguments:
//   val  :  On/off. 
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
// ****************************************************************************

void
QvisClipWindow::sphereInverseToggled(bool val)
{
    clipAtts->SetSphereInverse(val);
    Apply();
}

