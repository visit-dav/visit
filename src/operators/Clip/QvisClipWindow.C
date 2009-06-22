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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QWidget>
#include <QWidget>

#include <QvisClipWindow.h>
#include <ClipAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QPlaneGroup::QPlaneGroup
//
// Purpose: 
//   Constructor for the QPlaneGroup class.
//
// Arguments:
//   title    : Group Title
//   parent   : Optional QWidget parent.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************
QPlaneGroup::QPlaneGroup(const QString &title, QWidget *parent)
: QGroupBox(title,parent)
{
    setCheckable(true);
    QGridLayout *layout = new QGridLayout(this);
    origin = new QLineEdit(this);
    normal = new QLineEdit(this);
    layout->addWidget(new QLabel(tr("Origin")),0,0);
    layout->addWidget(origin,0,1);
    layout->addWidget(new QLabel(tr("Normal")),1,0);
    layout->addWidget(normal,1,1);
        
    connect(origin,SIGNAL(returnPressed()),
            this,SIGNAL(OriginChanged()));
        
    connect(normal,SIGNAL(returnPressed()),
            this,SIGNAL(NormalChanged()));
}

// ****************************************************************************
// Method: QPlaneGroup::~QPlaneGroup
//
// Purpose: 
//   Destructor for the QPlaneGroup class.
//
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************
QPlaneGroup::~QPlaneGroup()
{}

// ****************************************************************************
// Method: QPlaneGroup::SetOrigin
//
// Purpose: 
//   Sets the plane origin.
//
// Arguments:
//        val origin point location.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************
void 
QPlaneGroup::SetOrigin(double val[3])
{
    origin->blockSignals(true);
    origin->setText(GUIBase::DoublesToQString(val,3));
    origin->blockSignals(false);
}
 
// ****************************************************************************
// Method: QPlaneGroup::SetNormal
//
// Purpose: 
//   Sets the plane normal.
//
// Arguments:
//        val normal vector value.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************   
void 
QPlaneGroup::SetNormal(double val[3])
{
    normal->blockSignals(true);
    normal->setText(GUIBase::DoublesToQString(val,3));
    normal->blockSignals(false);
}
   
// ****************************************************************************
// Method: QPlaneGroup::GetOrigin
//
// Purpose: 
//   Trys to parse the plane origin from input line edit.
//   
// Returns:
//   True if the input line edit contains 3 doubles.
//
// Arguments:
//        val output for plane origin point.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************    
bool
QPlaneGroup::GetOrigin(double val[3])
{
    return GUIBase::LineEditGetDoubles(origin, val, 3);
}

// ****************************************************************************
// Method: QPlaneGroup::GetNormal
//
// Purpose: 
//   Trys to parse the plane normal from input line edit.
//   
// Returns:
//   True if the input line edit contains 3 doubles.
//
// Arguments:
//        val output for plane normal vector.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 14:17:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************    
bool 
QPlaneGroup::GetNormal(double val[3])
{
    return GUIBase::LineEditGetDoubles(normal, val, 3);
}


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
    atts = subj;
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
//   Jeremy Meredith, Fri Apr  3 10:19:16 EDT 2009
//   Added missing connections for plane toggles.
//
// ****************************************************************************

void
QvisClipWindow::CreateWindowContents()
{
    
    QWidget *qualityWidget= new QWidget(central);
    qualityGroup = new QButtonGroup(qualityWidget);
    
    QHBoxLayout *qualityLayout = new QHBoxLayout(qualityWidget);
    qualityLayout->setMargin(0);
    
    QRadioButton *fastQuality = new QRadioButton("Fast", qualityWidget);
    QRadioButton *accurateQuality = new QRadioButton(tr("Accurate"), qualityWidget);
    
    qualityLayout->addWidget(new QLabel(tr("Quality:"), qualityWidget));
    qualityGroup->addButton(fastQuality,0);
    qualityGroup->addButton(accurateQuality,1);
    
    qualityLayout->addWidget(fastQuality);
    qualityLayout->addWidget(accurateQuality);
    
    topLayout->addWidget(qualityWidget);
    connect(qualityGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(qualityChanged(int)));

    
    QWidget *typeWidget = new QWidget(central);
    typeGroup = new QButtonGroup(typeWidget);
    
    QHBoxLayout  *typeLayout = new QHBoxLayout(typeWidget);
    typeLayout->setMargin(0);
    QRadioButton *planeType = new QRadioButton(tr("Plane"), typeWidget);
    QRadioButton *sphereType = new QRadioButton(tr("Sphere"), typeWidget);
    
    typeLayout->addWidget(new QLabel(tr("Slice type:"), typeWidget));
    typeLayout->addWidget(planeType);
    typeLayout->addWidget(sphereType);
    topLayout->addWidget(typeWidget);

    typeGroup->addButton(planeType,0);
    typeGroup->addButton(sphereType,1);
    
    connect(typeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(sliceTypeChanged(int)));

    QGroupBox *frame = new QGroupBox(tr("Clip parameters"),central);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    topLayout->addWidget(frame);
    
    planeWidgets = new QWidget(frame);
    
    frameLayout->addWidget(planeWidgets);
    QVBoxLayout *planeWidgetsLayout = new QVBoxLayout(planeWidgets);

    // 
    // Plane widgets 
    // 
    
    plane1Group = new QPlaneGroup(tr("Plane 1"),planeWidgets);
    plane2Group = new QPlaneGroup(tr("Plane 2"),planeWidgets);
    plane3Group = new QPlaneGroup(tr("Plane 3"),planeWidgets);
    planeWidgetsLayout->addWidget(plane1Group);
    planeWidgetsLayout->addWidget(plane2Group);
    planeWidgetsLayout->addWidget(plane3Group);
    connect(plane1Group, SIGNAL(toggled(bool)),
            this, SLOT(plane1StatusToggled(bool)));
    connect(plane2Group, SIGNAL(toggled(bool)),
            this, SLOT(plane2StatusToggled(bool)));
    connect(plane3Group, SIGNAL(toggled(bool)),
            this, SLOT(plane3StatusToggled(bool)));
        
    // Plane Inverse
    planeInverse = new QCheckBox(tr("Inverse"), planeWidgets);
    planeInverse->setChecked(false);
    connect(planeInverse, SIGNAL(toggled(bool)),
            this, SLOT(planeInverseToggled(bool)));
    planeWidgetsLayout->addWidget(planeInverse);
            
    // Plane tool controls
    QLabel *planeToolControlledClipPlaneLabel = new QLabel(tr("Plane tool controls:"), planeWidgets);
    
    planeToolControlledClipPlane = new QButtonGroup(planeWidgets);
    QWidget *planeToolControlledClipPlaneWidget = new QWidget(planeWidgets);
    planeWidgetsLayout->addWidget(planeToolControlledClipPlaneLabel);
    planeWidgetsLayout->addWidget(planeToolControlledClipPlaneWidget);
    
    QHBoxLayout *planeToolControlledClipPlaneLayout = new QHBoxLayout(planeToolControlledClipPlaneWidget);
    planeToolControlledClipPlaneLayout->setMargin(0);
    
    QRadioButton *planeToolControlledClipPlaneWhichClipPlaneNone = new QRadioButton(tr("Nothing"),          
                                                                                    planeToolControlledClipPlaneWidget);
    
    
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane1 = new QRadioButton(tr("Plane 1"), 
                                                                                      planeToolControlledClipPlaneWidget);
    
    
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane2 = new QRadioButton(tr("Plane 2"), 
                                                                                      planeToolControlledClipPlaneWidget);
    
    QRadioButton *planeToolControlledClipPlaneWhichClipPlanePlane3 = new QRadioButton(tr("Plane 3"), 
                                                                                      planeToolControlledClipPlaneWidget);
    
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlaneNone);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane1);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane3);
    planeToolControlledClipPlaneLayout->addWidget(planeToolControlledClipPlaneWhichClipPlanePlane2);
    
    planeToolControlledClipPlane->addButton(planeToolControlledClipPlaneWhichClipPlaneNone,0);
    planeToolControlledClipPlane->addButton(planeToolControlledClipPlaneWhichClipPlanePlane1,1);
    planeToolControlledClipPlane->addButton(planeToolControlledClipPlaneWhichClipPlanePlane2,2);
    planeToolControlledClipPlane->addButton(planeToolControlledClipPlaneWhichClipPlanePlane3,3);
    
    connect(planeToolControlledClipPlane, SIGNAL(buttonClicked(int)),
            this, SLOT(planeToolControlledClipPlaneChanged(int)));
 
    // 
    // Sphere widgets
    // 
    
    sphereWidgets = new QWidget(frame);
    frameLayout->addWidget(sphereWidgets);
    
    QGridLayout *sphereLayout = new QGridLayout(sphereWidgets);
    
    // Sphere center
    sphereLayout->addWidget(new QLabel(tr("Center"), sphereWidgets),
                            0, 0, Qt::AlignRight);
    centerLineEdit = new QLineEdit(sphereWidgets);
    sphereLayout->addWidget(centerLineEdit, 0, 1);
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));

    // Sphere Radius 
    sphereLayout->addWidget(new QLabel(tr("Radius"), sphereWidgets),
                            1, 0, Qt::AlignRight);
    radiusLineEdit = new QLineEdit(sphereWidgets);
    sphereLayout->addWidget(radiusLineEdit, 1, 1);
    connect(radiusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRadiusText()));

    // Sphere Inverse
    sphereInverse = new QCheckBox(tr("Inverse"), sphereWidgets);
    sphereInverse->setChecked(false);
    sphereLayout->addWidget(sphereInverse, 2, 0, 1, 2);
    connect(sphereInverse, SIGNAL(toggled(bool)),
            this, SLOT(sphereInverseToggled(bool)));
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::UpdateWindow(bool doAll)
{

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
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
        case ClipAttributes::ID_funcType:
            typeGroup->button(atts->GetFuncType())->setChecked(true);
            if (atts->GetFuncType() == ClipAttributes::Plane)
            {
                plane1Group->setChecked(atts->GetPlane1Status());
                plane1Group->setChecked(atts->GetPlane2Status());
                plane1Group->setChecked(atts->GetPlane3Status());
                
                plane1Group->SetOrigin(atts->GetPlane1Origin());
                plane2Group->SetOrigin(atts->GetPlane2Origin());
                plane3Group->SetOrigin(atts->GetPlane3Origin());
                
                plane1Group->SetNormal(atts->GetPlane1Normal());
                plane2Group->SetNormal(atts->GetPlane2Normal());
                plane3Group->SetNormal(atts->GetPlane3Normal());

                planeInverse->setChecked(atts->GetPlaneInverse());
                sphereWidgets->hide();
                planeWidgets->show();
            }
            else if (atts->GetFuncType() == ClipAttributes::Sphere)
            {
                centerLineEdit->setText(DoublesToQString(atts->GetCenter(),3));
                radiusLineEdit->setText(DoubleToQString(atts->GetRadius()));
                sphereInverse->setChecked(atts->GetSphereInverse());
                planeWidgets->hide();
                sphereWidgets->show();
            }
            central->updateGeometry();
            break;
        case ClipAttributes::ID_plane1Status:
            plane1Group->blockSignals(true);
            plane1Group->setChecked(atts->GetPlane1Status());
            plane1Group->blockSignals(false);
            break;
        case ClipAttributes::ID_plane2Status:
            plane2Group->blockSignals(true);
            plane2Group->setChecked(atts->GetPlane2Status());
            plane2Group->blockSignals(false);
            break;
        case ClipAttributes::ID_plane3Status:
            plane3Group->blockSignals(true);
            plane3Group->setChecked(atts->GetPlane3Status());
            plane3Group->blockSignals(false);
            break;
        case ClipAttributes::ID_plane1Origin:
            plane1Group->SetOrigin(atts->GetPlane1Origin());
            break;
        case ClipAttributes::ID_plane2Origin:
            plane2Group->SetOrigin(atts->GetPlane2Origin());
            break;
        case ClipAttributes::ID_plane3Origin:
            plane3Group->SetOrigin(atts->GetPlane3Origin());
            break;
        case ClipAttributes::ID_plane1Normal:
            plane1Group->SetNormal(atts->GetPlane1Normal());
            break;
        case ClipAttributes::ID_plane2Normal:
            plane2Group->SetNormal(atts->GetPlane2Normal());
            break;
        case ClipAttributes::ID_plane3Normal:
            plane3Group->SetNormal(atts->GetPlane3Normal());
            break;
        case ClipAttributes::ID_planeInverse:
            planeInverse->blockSignals(true);
            planeInverse->setChecked(atts->GetPlaneInverse());
            planeInverse->blockSignals(false);
            break;
        case ClipAttributes::ID_planeToolControlledClipPlane:
            planeToolControlledClipPlane->blockSignals(true);
            planeToolControlledClipPlane->button(atts->GetPlaneToolControlledClipPlane())
                                                    ->setChecked(true);
            planeToolControlledClipPlane->blockSignals(false);
            break;
        case ClipAttributes::ID_center:
            centerLineEdit->setText(DoublesToQString(atts->GetCenter(),3));
            break;
        case ClipAttributes::ID_radius:
            radiusLineEdit->setText(DoubleToQString(atts->GetRadius()));
            break;
        case ClipAttributes::ID_sphereInverse:
            sphereInverse->blockSignals(true);
            sphereInverse->setChecked(atts->GetSphereInverse());
            sphereInverse->blockSignals(false);
            break;
        case ClipAttributes::ID_quality:
            qualityGroup->blockSignals(true);
            qualityGroup->button(atts->GetQuality())->setChecked(true);
            qualityGroup->blockSignals(false);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
//   Hank Childs, Fri Jan  9 15:10:11 PST 2009
//   Fix typo in Qt4 port.
//
// ****************************************************************************

void
QvisClipWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do plane1Origin
    if(which_widget == ClipAttributes::ID_plane1Origin || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane1Group->GetOrigin(val))
            atts->SetPlane1Origin(val);
        else
        {
            ResettingError(tr("Plane 1 Origin"),
                DoublesToQString(atts->GetPlane1Origin(),3));
            atts->SetPlane1Origin(atts->GetPlane1Origin());
        }
    }
    
    // Do plane2Origin
    if(which_widget == ClipAttributes::ID_plane2Origin || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane2Group->GetOrigin(val))
            atts->SetPlane2Origin(val);
        else
        {
            ResettingError(tr("Plane 2 Origin"),
                DoublesToQString(atts->GetPlane2Origin(),3));
            atts->SetPlane2Origin(atts->GetPlane2Origin());
        }
    }

    // Do plane3Origin
    if(which_widget == ClipAttributes::ID_plane3Origin || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane3Group->GetOrigin(val))
            atts->SetPlane3Origin(val);
        else
        {
            ResettingError(tr("Plane 3 Origin"),
                DoublesToQString(atts->GetPlane3Origin(),3));
            atts->SetPlane3Origin(atts->GetPlane3Origin());
        }
    }

    // Do plane1Normal
    if(which_widget == ClipAttributes::ID_plane1Normal || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane1Group->GetNormal(val))
            atts->SetPlane1Normal(val);
        else
        {
            ResettingError(tr("Plane 1 Normal"),
                DoublesToQString(atts->GetPlane1Normal(),3));
            atts->SetPlane1Normal(atts->GetPlane1Normal());
        }
    }

    // Do plane2Normal
    if(which_widget == ClipAttributes::ID_plane2Normal || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane2Group->GetNormal(val))
            atts->SetPlane2Normal(val);
        else
        {
            ResettingError(tr("Plane 2 Normal"),
                DoublesToQString(atts->GetPlane2Normal(),3));
            atts->SetPlane2Normal(atts->GetPlane2Normal());
        }

    }

    // Do plane3Normal
    if(which_widget == ClipAttributes::ID_plane3Normal || doAll)
    {
        // get from planeGroupWidget
        double val[3];
        if(plane3Group->GetNormal(val))
            atts->SetPlane3Normal(val);
        else
        {
            ResettingError(tr("Plane 3 Normal"),
                DoublesToQString(atts->GetPlane3Normal(),3));
            atts->SetPlane3Normal(atts->GetPlane3Normal());
        }

    }

    // Do center
    if(which_widget == ClipAttributes::ID_center || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(centerLineEdit, val, 3))
            atts->SetCenter(val);
        else
        {
            ResettingError(tr("Center"),
                DoublesToQString(atts->GetCenter(),3));
            atts->SetCenter(atts->GetCenter());
        }
    }

    // Do radius
    if(which_widget == ClipAttributes::ID_radius || doAll)
    {
        double val;
        if(LineEditGetDouble(radiusLineEdit, val))
            atts->SetRadius(val);
        else
        {
            ResettingError(tr("Radius"),
                DoubleToQString(atts->GetRadius()));
            atts->SetRadius(atts->GetRadius());
        }
    }
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
        atts->SetQuality(ClipAttributes::Fast);
    else
        atts->SetQuality(ClipAttributes::Accurate);

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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processPlane1Origin()
{
    GetCurrentValues(ClipAttributes::ID_plane1Origin);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processPlane2Origin()
{
    GetCurrentValues(ClipAttributes::ID_plane2Origin);
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
    GetCurrentValues(ClipAttributes::ID_plane3Origin);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processPlane1Normal()
{
    GetCurrentValues(ClipAttributes::ID_plane1Normal);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processPlane2Normal()
{
    GetCurrentValues(ClipAttributes::ID_plane2Normal);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processPlane3Normal()
{
    GetCurrentValues(ClipAttributes::ID_plane3Normal);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processCenterText()
{
    GetCurrentValues(ClipAttributes::ID_center);
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
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisClipWindow::processRadiusText()
{
    GetCurrentValues(ClipAttributes::ID_radius);
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
        atts->SetFuncType(ClipAttributes::Plane);
        sphereWidgets->hide();
        planeWidgets->show();
    }
    else
    {
        atts->SetFuncType(ClipAttributes::Sphere);
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
    atts->SetPlane1Status(val);
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
    atts->SetPlane2Status(val);
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
    atts->SetPlane3Status(val);
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
    atts->SetPlaneInverse(val);
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
    if(val != atts->GetPlaneToolControlledClipPlane())
    {
        atts->SetPlaneToolControlledClipPlane(ClipAttributes::WhichClipPlane(val));
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
    atts->SetSphereInverse(val);
    Apply();
}
