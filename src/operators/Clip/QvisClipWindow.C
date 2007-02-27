/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h> // for sscanf

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qgrid.h>
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
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   Changed plane*Status from groupButton to checkable QVGroupBox.  Modified
//   connect statements accordingly.
//
//   Brad Whitlock, Tue Dec 21 09:13:49 PDT 2004
//   Moved code into CreatePlaneGroup so it would be easier to add Qt
//   version-specific coding so we can still use versions older than 3.2. I
//   also changed how the Inverse buttons are created so they are more in 
//   line with how we create other check boxes.
//
// ****************************************************************************

void
QvisClipWindow::CreateWindowContents()
{
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
    planeBox->setSpacing(5);

    // Plane1 Group
    CreatePlaneGroup(planeBox, (QWidget **)&plane1Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane1Group,
#endif
        (QWidget **)&plane1Origin, (QWidget **)&plane1Normal,
        SLOT(plane1StatusToggled(bool)),
        SLOT(processPlane1Origin()),
        SLOT(processPlane1Normal()), 1);

    // Plane2 Group
    CreatePlaneGroup(planeBox, (QWidget **)&plane2Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane2Group,
#endif
        (QWidget **)&plane2Origin, (QWidget **)&plane2Normal,
        SLOT(plane2StatusToggled(bool)),
        SLOT(processPlane2Origin()),
        SLOT(processPlane2Normal()), 2);

    // Plane3 Group
    CreatePlaneGroup(planeBox, (QWidget **)&plane3Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane3Group,
#endif
        (QWidget **)&plane3Origin, (QWidget **)&plane3Normal,
        SLOT(plane3StatusToggled(bool)),
        SLOT(processPlane3Origin()),
        SLOT(processPlane3Normal()), 3);

    // Plane Inverse
    planeInverse = new QCheckBox("Inverse", planeBox, "planeInverse");
    planeInverse->setChecked(false);
    connect(planeInverse, SIGNAL(toggled(bool)),
            this, SLOT(planeInverseToggled(bool)));

    // 
    // Sphere tab
    // 
    QVBox *vertBox = new QVBox(central, "vertBox");
    sphereBox = new QGrid(2, Qt::Horizontal, vertBox, "sphereBox");
    sphereBox->setMargin(10);
    sphereBox->setSpacing(10);
    tabWidget->addTab(vertBox, "Sphere");

    // Sphere center
    new QLabel("Center", sphereBox, "centerLabel");
    centerLineEdit = new QLineEdit(sphereBox, "center");
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));

    // Sphere Radius 
    new QLabel("Radius", sphereBox, "radiusLabel");
    radiusLineEdit = new QLineEdit(sphereBox, "radius");
    connect(radiusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRadiusText()));

    // Sphere Inverse
    sphereInverse = new QCheckBox("Inverse", sphereBox, "sphereInverse");
    sphereInverse->setChecked(false);
    connect(sphereInverse, SIGNAL(toggled(bool)),
            this, SLOT(sphereInverseToggled(bool)));

    QHBox *sphereDummy = new QHBox(vertBox, "sphereDummy");
    vertBox->setStretchFactor(sphereDummy, 20);
}

// ****************************************************************************
// Method: QvisClipWindow::CreatePlaneGroup
//
// Purpose: 
//   Create a group of widgets in a group box. The widgets are used to 
//   control a plane.
//
// Arguments:
//   parent            : The widgets' parent widget.
//   planeStatus       : pointer to the plane status widget that we created.
//   pg                : pointer to the plane group widget that we created.
//   planeOrigin       : pointer to the plane origin widget that we created.
//   planeNormal       : pointer to the plane normal widget that we created.
//   statusToggledSlot : slot to call when plane status changes.
//   planeOriginSlot   : slot to call when plane origin changes.
//   planeNormalSlot   : slot to call when plane normal changes.
//   index             : The number to use in the title.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 21 09:47:49 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisClipWindow::CreatePlaneGroup(QWidget *parent, QWidget **planeStatus,
#if QT_VERSION < 0x030200
    QWidget **pg,
#endif
    QWidget **planeOrigin, QWidget **planeNormal,
    const char *statusToggledSlot,
    const char *planeOriginSlot,
    const char *planeNormalSlot, int index)
{
    QString title; title.sprintf("Plane %d", index);
    QString n; n.sprintf("plane%d", index);
#define N(s) QString(s).latin1()

    QVGroupBox *planeGroup;
#if QT_VERSION >= 0x030200
    planeGroup = new QVGroupBox(parent, N(n + "GBox"));
    planeGroup->setTitle(title);
    planeGroup->setCheckable(true);
    planeGroup->setColumns(2);
    *planeStatus = planeGroup;
    connect(planeGroup, SIGNAL(toggled(bool)),
            this, statusToggledSlot);
#else
    QCheckBox *c = new QCheckBox(title, parent, N(n+"checkbox"));
    *planeStatus = c;
    connect(c, SIGNAL(toggled(bool)),
            this, statusToggledSlot);
    planeGroup = new QVGroupBox(title, parent, N(n + "GBox"));
    planeGroup->setTitle(title);
    planeGroup->setColumns(2);
    *pg = planeGroup;
#endif

    // Plane origin
    new QLabel("Origin", planeGroup, N(n+"OriginLabel"));
    QLineEdit *origin = new QLineEdit(planeGroup, N(n+"Origin"));
    *planeOrigin = origin;
    connect(origin, SIGNAL(returnPressed()),
            this, planeOriginSlot);

    // Plane normal 
    new QLabel("Normal", planeGroup, N(n+"NormalLabel"));
    QLineEdit *normal = new QLineEdit(planeGroup, N(n+"Normal"));
    *planeNormal = normal;
    connect(normal, SIGNAL(returnPressed()),
            this, planeNormalSlot); 
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
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   plane*Status widgets are now QVGroupBoxes, modify set method calls
//   accordingly. 
//
//   Brad Whitlock, Tue Dec 21 09:54:20 PDT 2004
//   I added code to block signals for checkboxes.
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
                plane1Status->setChecked(clipAtts->GetPlane1Status());
                plane2Status->setChecked(clipAtts->GetPlane2Status());
                plane3Status->setChecked(clipAtts->GetPlane3Status());
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
            plane1Status->blockSignals(true);
            plane1Status->setChecked(clipAtts->GetPlane1Status());
            plane1Status->blockSignals(false);
            break;
        case 2: // plane2Status
            plane2Status->blockSignals(true);
            plane2Status->setChecked(clipAtts->GetPlane2Status());
            plane2Status->blockSignals(false);
            break;
        case 3: // plane3Status
            plane3Status->blockSignals(true);
            plane3Status->setChecked(clipAtts->GetPlane3Status());
            plane3Status->blockSignals(false);
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
            planeInverse->blockSignals(true);
            planeInverse->setChecked(clipAtts->GetPlaneInverse());
            planeInverse->blockSignals(false);
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
            sphereInverse->blockSignals(true);
            sphereInverse->setChecked(clipAtts->GetSphereInverse());
            sphereInverse->blockSignals(false);
            break;
        }
    } // end for

#if QT_VERSION < 0x030200
    // Set the enabled state for the plane groups if Qt version < 3.2.
    plane1Group->setEnabled(clipAtts->GetPlane1Status());
    plane2Group->setEnabled(clipAtts->GetPlane2Status());
    plane3Group->setEnabled(clipAtts->GetPlane3Status());
#endif
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
// Method: QvisClipWindow::plane1StatusToggled
//
// Purpose:
//   This is a Qt slot function that is called when a status is changed.
//
// Arguments:
//   val  :    The new state of the checkbox 
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   Changed name from 'Clicked' to 'Toggled'. 
//
// ****************************************************************************

void
QvisClipWindow::plane1StatusToggled(bool val)
{
    clipAtts->SetPlane1Status(val);
    Apply();
}

// ****************************************************************************
// Method: QvisClipWindow::plane2StatusToggled
//
// Purpose:
//   This is a Qt slot function that is called when a status is changed.
//
// Arguments:
//   val  :    The new state of the checkbox. 
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   Changed name from 'Clicked' to 'Toggled'. 
//
// ****************************************************************************

void
QvisClipWindow::plane2StatusToggled(bool val)
{
    clipAtts->SetPlane2Status(val);
    Apply();
}


// ****************************************************************************
// Method: QvisClipWindow::plane3StatusToggled
//
// Purpose:
//   This is a Qt slot function that is called when a status is changed.
//
// Arguments:
//   val  :    The new state of the checkbox.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2001
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Jun  5 15:11:09 PDT 2001
//   Changed 'apply' to 'Apply'.
//   
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   Changed name from 'Clicked' to 'Toggled'. 
//
// ****************************************************************************

void
QvisClipWindow::plane3StatusToggled(bool val)
{
    clipAtts->SetPlane3Status(val);
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

