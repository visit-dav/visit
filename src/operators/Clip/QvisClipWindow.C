/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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
    ClipAttributes *subj, const QString &caption, const QString &shortName,
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
//   Gunther H. Weber, Tue Aug 14 19:46:03 PDT 2007
//   Added radio buttons to select clip plane manipulated by plane tool
//
//   Sean Ahern, Wed Feb 27 16:17:11 EST 2008
//   Added 'fast' vs. 'accurate' quality options.  I also removed the tab
//   widget, as it shouldn't be used in the way it was.
//
//   Brad Whitlock, Fri Apr 25 09:34:26 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisClipWindow::CreateWindowContents()
{
    qualityGroup = new QButtonGroup(central, "Quality");
    QHBoxLayout *qualityLayout = new QHBoxLayout(qualityGroup);
    qualityLayout->addWidget(new QLabel(tr("Quality:"), qualityGroup));
    QRadioButton *fastQuality = new QRadioButton("Fast", qualityGroup);
    qualityLayout->addWidget(fastQuality);
    QRadioButton *accurateQuality = new QRadioButton(tr("Accurate"), qualityGroup);
    qualityLayout->addWidget(accurateQuality);
    topLayout->addWidget(qualityGroup);
    connect(qualityGroup, SIGNAL(clicked(int)),
            this, SLOT(qualityChanged(int)));

    typeGroup = new QButtonGroup(central, "Type");
    QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
    typeLayout->addWidget(new QLabel(tr("Slice type:"), typeGroup));
    QRadioButton *planeType = new QRadioButton(tr("Plane"), typeGroup);
    typeLayout->addWidget(planeType);
    QRadioButton *sphereType = new QRadioButton(tr("Sphere"), typeGroup);
    typeLayout->addWidget(sphereType);
    topLayout->addWidget(typeGroup);
    connect(typeGroup, SIGNAL(clicked(int)),
            this, SLOT(sliceTypeChanged(int)));

    QVGroupBox *frame = new QVGroupBox(tr("Clip parameters"), central,
                                       "clipParameters");
    topLayout->addWidget(frame);

    // 
    // Plane widgets 
    // 
    planeWidgets = new QVBox(frame, "planeWidgets");
    planeWidgets->setMargin(10);
    planeWidgets->setSpacing(5);

    // Plane1 Group
    CreatePlaneGroup(planeWidgets, (QWidget **)&plane1Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane1Group,
#endif
        (QWidget **)&plane1Origin, (QWidget **)&plane1Normal,
        SLOT(plane1StatusToggled(bool)),
        SLOT(processPlane1Origin()),
        SLOT(processPlane1Normal()), 1);

    // Plane2 Group
    CreatePlaneGroup(planeWidgets, (QWidget **)&plane2Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane2Group,
#endif
        (QWidget **)&plane2Origin, (QWidget **)&plane2Normal,
        SLOT(plane2StatusToggled(bool)),
        SLOT(processPlane2Origin()),
        SLOT(processPlane2Normal()), 2);

    // Plane3 Group
    CreatePlaneGroup(planeWidgets, (QWidget **)&plane3Status,
#if QT_VERSION < 0x030200
        (QWidget **)&plane3Group,
#endif
        (QWidget **)&plane3Origin, (QWidget **)&plane3Normal,
        SLOT(plane3StatusToggled(bool)),
        SLOT(processPlane3Origin()),
        SLOT(processPlane3Normal()), 3);

    // Plane Inverse
    planeInverse = new QCheckBox(tr("Inverse"), planeWidgets, "planeInverse");
    planeInverse->setChecked(false);
    connect(planeInverse, SIGNAL(toggled(bool)),
            this, SLOT(planeInverseToggled(bool)));

    // Plane tool controls
    QLabel *planeToolControlledClipPlaneLabel = new QLabel(tr("Plane tool controls:"), planeWidgets, "planeToolControlledClipPlaneLabel");
    planeToolControlledClipPlane = new QButtonGroup(planeWidgets, "planeToolControlledClipPlane");
    planeToolControlledClipPlane->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *planeToolControlledClipPlaneLayout = new QHBoxLayout(planeToolControlledClipPlane);
    planeToolControlledClipPlaneLayout->setSpacing(10);
    QRadioButton *planeToolControlledClipPlaneWhichClipPlaneNone = new QRadioButton(tr("Nothing"), planeToolControlledClipPlane);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlaneNone);
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane1 = new QRadioButton(tr("Plane 1"), planeToolControlledClipPlane);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane1);
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane2 = new QRadioButton(tr("Plane 2"), planeToolControlledClipPlane);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane2);
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane3 = new QRadioButton(tr("Plane 3"), planeToolControlledClipPlane);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane3);
    connect(planeToolControlledClipPlane, SIGNAL(clicked(int)),
            this, SLOT(planeToolControlledClipPlaneChanged(int)));
 
    // 
    // Sphere widgets
    // 
    sphereWidgets = new QWidget(frame, "sphereWidgets");
    QGridLayout *sphereLayout = new QGridLayout(sphereWidgets, 3, 2, 10, 10);

    // Sphere center
    sphereLayout->addWidget(new QLabel(tr("Center"), sphereWidgets,
                                       "centerLabel"),
                            0, 0, Qt::AlignRight);
    centerLineEdit = new QLineEdit(sphereWidgets, "center");
    sphereLayout->addWidget(centerLineEdit, 0, 1);
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));

    // Sphere Radius 
    sphereLayout->addWidget(new QLabel(tr("Radius"), sphereWidgets,
                                       "radiusLabel"),
                            1, 0, Qt::AlignRight);
    radiusLineEdit = new QLineEdit(sphereWidgets, "radius");
    sphereLayout->addWidget(radiusLineEdit, 1, 1);
    connect(radiusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRadiusText()));

    // Sphere Inverse
    sphereInverse = new QCheckBox(tr("Inverse"), sphereWidgets, "sphereInverse");
    sphereInverse->setChecked(false);
    sphereLayout->addMultiCellWidget(sphereInverse, 2, 2, 0, 1);
    connect(sphereInverse, SIGNAL(toggled(bool)),
            this, SLOT(sphereInverseToggled(bool)));
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
//   Brad Whitlock, Fri Apr 25 09:36:02 PDT 2008
//   Added tr()'s
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
    QString title(tr("Plane %1").arg(index));
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
    new QLabel(tr("Origin"), planeGroup, N(n+"OriginLabel"));
    QLineEdit *origin = new QLineEdit(planeGroup, N(n+"Origin"));
    *planeOrigin = origin;
    connect(origin, SIGNAL(returnPressed()),
            this, planeOriginSlot);

    // Plane normal 
    new QLabel(tr("Normal"), planeGroup, N(n+"NormalLabel"));
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
//   Gunther H. Weber, Tue Aug 14 19:46:03 PDT 2007
//   Added code to handle radio buttons to select clip plane manipulated by
//   plane tool
//
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
            typeGroup->setButton(clipAtts->GetFuncType());
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
                sphereWidgets->hide();
                planeWidgets->show();
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
                planeWidgets->hide();
                sphereWidgets->show();
            }
            central->updateGeometry();
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
        case 11: //planeToolControlledClipPlane
            planeToolControlledClipPlane->blockSignals(true);
            planeToolControlledClipPlane->setButton(clipAtts->GetPlaneToolControlledClipPlane());
            planeToolControlledClipPlane->blockSignals(false);
            break;
        case 12: // center
            dptr = clipAtts->GetCenter();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            centerLineEdit->setText(temp);
            break;
        case 13: // radius
            r = clipAtts->GetRadius();
            temp.setNum(r);
            radiusLineEdit->setText(temp);
            break;
        case 14: // sphereInverse
            sphereInverse->blockSignals(true);
            sphereInverse->setChecked(clipAtts->GetSphereInverse());
            sphereInverse->blockSignals(false);
            break;
        case 15: // quality
            qualityGroup->blockSignals(true);
            qualityGroup->setButton(clipAtts->GetQuality());
            qualityGroup->blockSignals(false);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The origin vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The origin vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The origin vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The normal vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The normal vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The normal vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The center was invalid. "
                     "Resetting to the last good value %1.").arg(num);
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
            msg = tr("The radius was invalid. "
                     "Resetting to the last good value %1.").arg(r); 
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
// Method: QvisClipWindow::qualityChanged
//
// Purpose:
//   This is a Qt slot function that is called when user selects a different 
//   clip quality.
//
// Arguments:
//   int : The currently active quality.
//
// Programmer: Sean Ahern
// Creation:   2 February 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisClipWindow::qualityChanged(int quality)
{
    if (quality == 0)
        clipAtts->SetQuality(ClipAttributes::Fast);
    else
        clipAtts->SetQuality(ClipAttributes::Accurate);

    Apply();
}

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
// Method: QvisClipWindow::sliceTypeChanged
//
// Purpose:
//   This is a Qt slot function that is called when user selects a different 
//   slice type.
//
// Arguments:
//   int : The currently active slice type.
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
//   Sean Ahern, Wed Feb 27 20:53:39 EST 2008
//   Removed the tabs, as it was an invalid way of using this visual
//   paradigm.
//
// ****************************************************************************

void
QvisClipWindow::sliceTypeChanged(int type)
{
    if (type == 0)
    {
        clipAtts->SetFuncType(ClipAttributes::Plane);
        sphereWidgets->hide();
        planeWidgets->show();
    }
    else
    {
        clipAtts->SetFuncType(ClipAttributes::Sphere);
        planeWidgets->hide();
        sphereWidgets->show();
    }

    QWidget *top = central->parentWidget();
    top->adjustSize();

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
// Method: QvisClipWindow::planeToolControlledClipPlaneChanged
//
// Purpose:
//   This is a Qt slot function that is called when another radio button in the
//   planeToolControlledClipPlane button group is selected.
//
// Arguments:
//   val  :  Which button is now active.
//
// Programmer: Gunther H. Weber
// Creation:   August 14, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisClipWindow::planeToolControlledClipPlaneChanged(int val)
{
    if(val != clipAtts->GetPlaneToolControlledClipPlane())
    {
        clipAtts->SetPlaneToolControlledClipPlane(ClipAttributes::WhichClipPlane(val));
        SetUpdate(false);
        Apply();
    }
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
