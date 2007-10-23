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
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>

#include <QvisSliceWindow.h>
#include <FileServerList.h>
#include <SliceAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisSliceWindow::QvisSliceWindow
//
// Purpose: 
//   Cconstructor for the QvisSliceWindow class.
//
// Arguments:
//   subj      : The SliceAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:34:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 12 12:59:38 PDT 2002
//   Made it inherit from QvisOperatorWindow.
//
//    Jeremy Meredith, Mon May  5 14:40:32 PDT 2003
//    Added "sliderDragging" to keep track of when the user is dragging
//    the slice-by-percent slider.
//
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Initialize defaultItem, attach fileServer.
//
// ****************************************************************************

QvisSliceWindow::QvisSliceWindow(const int type,
    SliceAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    sliceAtts = subj;
    sliderDragging = false;

    defaultItem = "default";
    fileServer->Attach(this);   
}

// ****************************************************************************
// Method: QvisSliceWindow::~QvisSliceWindow
//
// Purpose: 
//   This is the destructor for the QvisSliceWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:36:26 PDT 2000
//
// Modifications:
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Detach fileServer.
//   
// ****************************************************************************

QvisSliceWindow::~QvisSliceWindow()
{
    if (fileServer)
        fileServer->Detach(this);
}

// ****************************************************************************
// Method: QvisSliceWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the Slice operator window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:37:08 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 17:36:46 PST 2001
//   Added an "interactive" toggle.
//
//   Brad Whitlock, Tue Feb 26 14:25:16 PST 2002
//   Added buttons that align the slice plane to an axis.
//
//   Jeremy Meredith, Sun Nov 17 17:35:41 PST 2002
//   Updated for orthogonal slice additions.
//
//   Brad Whitlock, Mon Nov 18 10:12:15 PDT 2002
//   I made the window work with autoupdate again and corrected some things.
//
//   Jeremy Meredith, Mon May  5 14:40:53 PDT 2003
//   Entirely changed the way origin works.
//
//   Jeremy Meredith, Fri Jun 13 12:08:47 PDT 2003
//   Added a domain number for slice-by-zone and -by-node.
//
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Added meshName combo box and label.
//
//   Dave Pugmire, Thu Oct 18 08:25:42 EDT 2007
//   Added theta-phi method of editing the plane normal.
//
// ****************************************************************************

void
QvisSliceWindow::CreateWindowContents()
{
    // Normal
    QGroupBox *normalBox = new QGroupBox(central, "normalBox");
    normalBox->setTitle("Normal");
    topLayout->addWidget(normalBox);

    QGridLayout *normalLayout = new QGridLayout(normalBox, 4, 2);
    normalLayout->setMargin(10);
    normalLayout->setSpacing(5);
    normalLayout->addRowSpacing(0, 10);

    normalTypeGroup = new QButtonGroup();
    connect(normalTypeGroup, SIGNAL(clicked(int)),
            this, SLOT(normalTypeChanged(int)));

    //    Orthogonal
    normalLayout->addWidget(new QLabel("Orthogonal", normalBox), 1,0);

    QHBoxLayout *orthogonalLayout = new QHBoxLayout();

    QRadioButton *xAxis = new QRadioButton("X Axis", normalBox, "xAxis");
    QRadioButton *yAxis = new QRadioButton("Y Axis", normalBox, "yAxis");
    QRadioButton *zAxis = new QRadioButton("Z Axis", normalBox, "zAxis");
    normalTypeGroup->insert(xAxis);
    normalTypeGroup->insert(yAxis);
    normalTypeGroup->insert(zAxis);
    orthogonalLayout->addWidget(xAxis);
    orthogonalLayout->addWidget(yAxis);
    orthogonalLayout->addWidget(zAxis);

    flipNormalToggle = new QCheckBox("flip", normalBox, "flipNormalToggle");
    connect(flipNormalToggle, SIGNAL(toggled(bool)), this, SLOT(flipNormalToggled(bool)));
    orthogonalLayout->addWidget(flipNormalToggle);

    normalLayout->addLayout(orthogonalLayout, 1,1);

    //    Arbitrary
    normalLayout->addWidget(new QLabel("Arbitrary", normalBox), 2,0);

    QHBoxLayout *arbitraryLayout = new QHBoxLayout();

    QRadioButton *arbAxis = new QRadioButton(" ", normalBox, "arbAxis");
    normalTypeGroup->insert(arbAxis);
    arbitraryLayout->addWidget(arbAxis);

    normalLineEdit = new QLineEdit(normalBox, "normalLineEdit");
    connect(normalLineEdit, SIGNAL(returnPressed()), this, SLOT(processNormalText()));
    arbitraryLayout->addWidget(normalLineEdit);

    normalLayout->addLayout(arbitraryLayout, 2,1);


    //Theta-phi
    normalLayout->addWidget( new QLabel( "Theta-Phi", normalBox ), 3, 0 );

    QHBoxLayout *thetaPhiLayout = new QHBoxLayout();

    QRadioButton *thetaPhiRadio = new QRadioButton(" ", normalBox, "thetaPhi");
    normalTypeGroup->insert( thetaPhiRadio );
    thetaPhiLayout->addWidget( thetaPhiRadio );

    thetaPhiLineEdit = new QLineEdit( normalBox, "thetaPhiLineEdit");
    connect( thetaPhiLineEdit, SIGNAL(returnPressed()), this, SLOT(processThetaPhiText()));
    thetaPhiLayout->addWidget( thetaPhiLineEdit );

    normalLayout->addLayout( thetaPhiLayout, 3, 1 );

    // Origin
    originTypeGroup = new QButtonGroup();
    connect(originTypeGroup, SIGNAL(clicked(int)),
            this, SLOT(originTypeChanged(int)));

    QGroupBox *originBox = new QGroupBox(central, "originBox");
    originBox->setTitle("Origin");
    topLayout->addWidget(originBox);

    QGridLayout *originLayout = new QGridLayout(originBox, 8, 1);
    originLayout->setMargin(10);
    originLayout->addRowSpacing(0, 10);

    QHBoxLayout *originTypeLayout = new QHBoxLayout(5);
    originLayout->addMultiCellLayout(originTypeLayout, 1,1,  0,0);

    QRadioButton *otPoint     = new QRadioButton("Point",     originBox, "otPoint");
    QRadioButton *otIntercept = new QRadioButton("Intercept", originBox, "otIntercept");
    QRadioButton *otPercent   = new QRadioButton("Percent",   originBox, "otPercent");
    QRadioButton *otZone      = new QRadioButton("Zone",      originBox, "otZone");
    QRadioButton *otNode      = new QRadioButton("Node",      originBox, "otNode");
    originTypeGroup->insert(otPoint);
    originTypeGroup->insert(otIntercept);
    originTypeGroup->insert(otPercent);
    originTypeGroup->insert(otZone);
    originTypeGroup->insert(otNode);
    originTypeLayout->addWidget(otPoint);
    originTypeLayout->addWidget(otIntercept);
    originTypeLayout->addWidget(otPercent);
    originTypeLayout->addWidget(otZone);
    originTypeLayout->addWidget(otNode);


    // -- origin (point)
    originPointLayout = new QHBoxLayout(5);
    originPointLabel = new QLabel("Point", originBox);

    originPointLineEdit = new QLineEdit(originBox, "originPointLineEdit");
    connect(originPointLineEdit, SIGNAL(returnPressed()), this, SLOT(processOriginPointText()));

    originPointLayout->addWidget(originPointLabel);
    originPointLayout->addWidget(originPointLineEdit);

    originLayout->addLayout(originPointLayout, 2,0);

    // -- origin (intercept)
    originInterceptLayout = new QHBoxLayout(5);
    originInterceptLabel = new QLabel("Intercept", originBox);

    originInterceptLineEdit = new QLineEdit(originBox, "originInterceptLineEdit");
    connect(originInterceptLineEdit, SIGNAL(returnPressed()), this, SLOT(processOriginInterceptText()));

    originInterceptLayout->addWidget(originInterceptLabel);
    originInterceptLayout->addWidget(originInterceptLineEdit);

    originLayout->addLayout(originInterceptLayout, 3,0);

    // -- origin (percent)
    originPercentLayout = new QHBoxLayout(5);
    originPercentLabel = new QLabel("Percent", originBox);

    originPercentLineEdit = new QNarrowLineEdit(originBox, "originPercentLineEdit");
    connect(originPercentLineEdit, SIGNAL(returnPressed()), this, SLOT(processOriginPercentText()));

    originPercentSlider = new QSlider(0, 100, 10, 50, Qt::Horizontal,
                                      originBox, "originPercentSlider");
    connect(originPercentSlider, SIGNAL(valueChanged(int)),
            this, SLOT(originPercentSliderChanged(int)));
    connect(originPercentSlider, SIGNAL(sliderPressed()),
            this, SLOT(originPercentSliderPressed()));
    connect(originPercentSlider, SIGNAL(sliderReleased()),
            this, SLOT(originPercentSliderReleased()));

    originPercentLayout->addWidget(originPercentLabel);
    originPercentLayout->addWidget(originPercentLineEdit);
    originPercentLayout->addWidget(originPercentSlider);

    originLayout->addLayout(originPercentLayout, 4,0);

    // -- origin (zone)
    originZoneLayout = new QHBoxLayout(5);
    originZoneLabel = new QLabel("Zone", originBox);

    originZoneLineEdit = new QLineEdit(originBox, "originZoneLineEdit");
    connect(originZoneLineEdit, SIGNAL(returnPressed()), this, SLOT(processOriginZoneText()));

    originZoneDomainLabel = new QLabel("Domain", originBox);
    originZoneDomainLineEdit = new QLineEdit(originBox, "originZoneDomainLineEdit");
    connect(originZoneDomainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginZoneDomainText()));

    originZoneLayout->addWidget(originZoneLabel);
    originZoneLayout->addWidget(originZoneLineEdit);
    originZoneLayout->addWidget(originZoneDomainLabel);
    originZoneLayout->addWidget(originZoneDomainLineEdit);

    originLayout->addLayout(originZoneLayout, 5,0);

    // -- origin (node)
    originNodeLayout = new QHBoxLayout(5);
    originNodeLabel = new QLabel("Node", originBox);

    originNodeLineEdit = new QLineEdit(originBox, "originNodeLineEdit");
    connect(originNodeLineEdit, SIGNAL(returnPressed()), this, SLOT(processOriginNodeText()));

    originNodeDomainLabel = new QLabel("Domain", originBox);
    originNodeDomainLineEdit = new QLineEdit(originBox, "originNodeDomainLineEdit");
    connect(originNodeDomainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginNodeDomainText()));

    originNodeLayout->addWidget(originNodeLabel);
    originNodeLayout->addWidget(originNodeLineEdit);
    originNodeLayout->addWidget(originNodeDomainLabel);
    originNodeLayout->addWidget(originNodeDomainLineEdit);

    originLayout->addLayout(originNodeLayout, 6,0);

    QHBoxLayout *meshLayout = new QHBoxLayout(5);
    meshLayout->setMargin(5);
    meshLabel = new QLabel("Mesh", originBox, "MeshLabel");
    meshName = new QComboBox(true, originBox, "meshName");
    meshName->setAutoCompletion(true);
    meshName->setInsertionPolicy(QComboBox::NoInsertion);
    meshName->insertItem(defaultItem);
    meshName->setCurrentItem(0);
    meshName->setEditText(defaultItem);
    connect(meshName, SIGNAL(activated(int)), this, SLOT(meshNameChanged()));
    meshLayout->addWidget(meshLabel);
    meshLayout->addWidget(meshName);
    originLayout->addLayout(meshLayout, 7, 0);

    // Up Axis
    QGroupBox *upAxisBox = new QGroupBox(central, "upAxisBox");
    upAxisBox->setTitle("Up Axis");
    topLayout->addWidget(upAxisBox);

    QVBoxLayout *upAxisLayoutTop = new QVBoxLayout(upAxisBox);
    upAxisLayoutTop->setMargin(10);
    upAxisLayoutTop->addSpacing(10);
    QGridLayout *upAxisLayout = new QGridLayout(upAxisLayoutTop, 2, 2);
    upAxisLayout->setSpacing(5);

    projectToggle = new QCheckBox("Project to 2D", upAxisBox, "projectToggle");
    connect(projectToggle, SIGNAL(toggled(bool)), this, SLOT(projectToggled(bool)));
    upAxisLayout->addMultiCellWidget(projectToggle, 0, 0, 0, 1);

    upAxisLineEdit = new QLineEdit(upAxisBox, "upAxisLineEdit");
    connect(upAxisLineEdit, SIGNAL(returnPressed()), this, SLOT(processUpAxisText()));
    upAxisLayout->addWidget(upAxisLineEdit, 1, 1);
    upAxisLabel = new QLabel(upAxisLineEdit, "Direction", upAxisBox, "Direction");
    upAxisLayout->addWidget(upAxisLabel, 1, 0);

    interactiveToggle = new QCheckBox("Interactive", central, "interactiveToggle");
    connect(interactiveToggle, SIGNAL(toggled(bool)), this, SLOT(interactiveToggled(bool)));
    topLayout->addWidget(interactiveToggle);
}

// ****************************************************************************
// Method: QvisSliceWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets to reflect changes made
//   in the SliceAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the SliceAttribute object's selected
//           states.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:37:41 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 17:40:33 PST 2001
//   Added code to update the interactive toggle.
//
//   Brad Whitlock, Tue Feb 12 12:21:37 PDT 2002
//   Removed the code to set the enabled state of the upAxis widgets. I also
//   fixed some format strings.
//
//   Jeremy Meredith, Fri Mar 15 17:26:24 PST 2002
//   Made origin be a Point.
//
//   Brad Whitlock, Tue Jul 23 10:08:50 PDT 2002
//   I added the code back in to set the enabled state of the upAxis widgets.
//   I don't remember why I removed it but there was a reason. Everyone else
//   thinks it should be back in.
//
//   Jeremy Meredith, Sun Nov 17 17:35:41 PST 2002
//   Updated for orthogonal slice addition.
//
//   Jeremy Meredith, Mon May  5 14:41:27 PDT 2003
//   Completely changed the way origin works.  Re-ordered almost everything.
//   Also, made project-to-2d be required for "flip" to be enabled.
//
//   Jeremy Meredith, Fri Jun 13 12:09:10 PDT 2003
//   Added a domain number for slice-by-zone and -by-node.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Added call to UpdateMeshNames.
//
//   Dave Pugmire, Thu Oct 18 08:25:42 EDT 2007
//   Added theta-phi method of editing the plane normal.
//
// ****************************************************************************

void
QvisSliceWindow::UpdateWindow(bool doAll)
{
    QString temp;

    if (selectedSubject == fileServer)
    {
        UpdateMeshNames();
        return;
    }

    if (doAll)
        UpdateMeshNames();

    bool orthogonal = sliceAtts->GetAxisType() != SliceAttributes::Arbitrary;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < sliceAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!sliceAtts->IsSelected(i))
                continue;
        }

        const double *dptr;
        int ival;
        switch(i)
        {
          case 0: // origin type
            originTypeGroup->blockSignals(true);
            originTypeGroup->setButton(int(sliceAtts->GetOriginType()));
            originTypeGroup->blockSignals(false);
            break;
          case 1: // origin point
            dptr = sliceAtts->GetOriginPoint();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            originPointLineEdit->setText(temp);
            break;
          case 2: // origin intercept
            temp.setNum(sliceAtts->GetOriginIntercept());
            originInterceptLineEdit->setText(temp);
            break;
          case 3: // origin percent
            temp.setNum(sliceAtts->GetOriginPercent());
            originPercentLineEdit->setText(temp);
            ival = int(QMIN(QMAX(0,sliceAtts->GetOriginPercent()),100));
            originPercentSlider->blockSignals(true);
            originPercentSlider->setValue(ival);
            originPercentSlider->blockSignals(false);
            break;
          case 4: // origin zone
            temp.sprintf("%d", sliceAtts->GetOriginZone());
            originZoneLineEdit->setText(temp);
            break;
          case 5: // origin node
            temp.sprintf("%d", sliceAtts->GetOriginNode());
            originNodeLineEdit->setText(temp);
            break;
          case 6: // normal
            dptr = sliceAtts->GetNormal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            normalLineEdit->setText(temp);
            break;
          case 7: // axisType
            normalTypeGroup->blockSignals(true);
            normalTypeGroup->setButton(int(sliceAtts->GetAxisType()));
            normalTypeGroup->blockSignals(false);

            normalLineEdit->setEnabled(!orthogonal);
	    thetaPhiLineEdit->setEnabled( sliceAtts->GetAxisType() == SliceAttributes::ThetaPhi );
            upAxisLineEdit->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            upAxisLabel->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            break;
          case 8: // upAxis
            dptr = sliceAtts->GetUpAxis();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            upAxisLineEdit->setText(temp);
            break;
          case 9: // project2d;
            projectToggle->blockSignals(true);
            projectToggle->setChecked(sliceAtts->GetProject2d());
            projectToggle->blockSignals(false);

            flipNormalToggle->setEnabled(sliceAtts->GetProject2d());
            upAxisLineEdit->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            upAxisLabel->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            break;
          case 10: // interactive;
            interactiveToggle->blockSignals(true);
            interactiveToggle->setChecked(sliceAtts->GetInteractive());
            interactiveToggle->blockSignals(false);
            break;
          case 11: // flip
            flipNormalToggle->blockSignals(true);
            flipNormalToggle->setChecked(sliceAtts->GetFlip());
            flipNormalToggle->setEnabled(sliceAtts->GetProject2d());
            flipNormalToggle->blockSignals(false);
            break;
          case 12: // origin zone domain
            temp.sprintf("%d", sliceAtts->GetOriginZoneDomain());
            originZoneDomainLineEdit->setText(temp);
            break;
          case 13: // origin node domain
            temp.sprintf("%d", sliceAtts->GetOriginNodeDomain());
            originNodeDomainLineEdit->setText(temp);
            break;
	case 15: //theta
	case 16: //phi
	    temp.sprintf( "%g %g", sliceAtts->GetTheta(), sliceAtts->GetPhi() );
	    thetaPhiLineEdit->setText( temp );
	    break;
        }
    } // end for

    UpdateOriginArea();
}


// ****************************************************************************
//  Method:  QvisSliceWindow::UpdateOriginArea
//
//  Purpose:
//    Shows the correct widgets in the origin area.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  5, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jun 13 12:09:21 PDT 2003
//    Added a domain number for slice-by-zone and -by-node.
//
//    Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//    Added meshName and meshLabel. 
//
// ****************************************************************************
void
QvisSliceWindow::UpdateOriginArea()
{
    int originType = sliceAtts->GetOriginType();

    originPointLabel->hide();
    originPointLineEdit->hide();
    originInterceptLabel->hide();
    originInterceptLineEdit->hide();
    originPercentLabel->hide();
    originPercentLineEdit->hide();
    originPercentSlider->hide();
    originZoneLabel->hide();
    originZoneLineEdit->hide();
    originNodeLabel->hide();
    originNodeLineEdit->hide();
    originZoneDomainLabel->hide();
    originZoneDomainLineEdit->hide();
    originNodeDomainLabel->hide();
    originNodeDomainLineEdit->hide();
    meshLabel->hide();
    meshName->hide();

    if (originType==SliceAttributes::Point)
    {
        originPointLabel->show();
        originPointLineEdit->show();
    }

    if (originType==SliceAttributes::Intercept)
    {
        originInterceptLabel->show();
        originInterceptLineEdit->show();
    }

    if (originType==SliceAttributes::Percent)
    {
        originPercentLabel->show();
        originPercentLineEdit->show();
        originPercentSlider->show();
    }

    if (originType==SliceAttributes::Zone)
    {
        originZoneLabel->show();
        originZoneLineEdit->show();
        originZoneDomainLabel->show();
        originZoneDomainLineEdit->show();
        if (meshName->count() > 1)
        {
            meshLabel->show();
            meshName->show();
        }
    }

    if (originType==SliceAttributes::Node)
    {
        originNodeLabel->show();
        originNodeLineEdit->show();
        originNodeDomainLabel->show();
        originNodeDomainLineEdit->show();
        if (meshName->count() > 1)
        {
            meshLabel->show();
            meshName->show();
        }
    }
}


// ****************************************************************************
// Method: QvisSliceWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:11:42 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 17:47:31 PST 2001
//   Made it conform to the regenerated attributes.
//
//   Brad Whitlock, Tue Feb 12 12:23:10 PDT 2002
//   Fixed some format strings.
//
//   Jeremy Meredith, Fri Mar 15 17:26:24 PST 2002
//   Made origin be a Point.
//
//   Jeremy Meredith, Sun Nov 17 17:35:41 PST 2002
//   Updated for orthogonal slice additions.
//
//   Jeremy Meredith, Mon May  5 14:44:10 PDT 2003
//   Completely changed the way "origin" works.
//
//   Kathleen Bonnell, Tue May 20 16:02:52 PDT 2003
//   Disallow (0, 0, 0) for Normal and UpAxis. 
//
//   Jeremy Meredith, Fri Jun 13 12:09:30 PDT 2003
//   Added a domain number for slice-by-zone and -by-node.
//
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Added meshName.
//
//   Dave Pugmire, Thu Oct 18 08:25:42 EDT 2007
//   Added theta-phi method of editing the plane normal.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Removed dependencies on vtkMath.
//
// ****************************************************************************

void
QvisSliceWindow::GetCurrentValues(int which_widget)
{
    bool    okay, doAll = (which_widget == -1);
    double  vals[3];
    QString msg, temp;

    // Do the origin type
    if(which_widget == 0 || doAll)
    {
        // do nothing
    }

    // Do the normal vector
    if(which_widget == 1 || doAll)
    {
        temp = normalLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.latin1(), "%lg %lg %lg",
                           &vals[0], &vals[1], &vals[2]) == 3);
            if(okay)
            {
                okay = (vals[0] != 0. || vals[1] != 0. || vals[2] != 0. );
                if (okay)
		{
                    sliceAtts->SetNormal(vals);

		    if ( sliceAtts->GetAxisType() != SliceAttributes::ThetaPhi )
		    {
			double len = sqrt(vals[0]*vals[0] + vals[1]*vals[1] + vals[2]*vals[2]);
			vals[0] /= len;
			vals[1] /= len;
			vals[2] /= len;
			len = 1.0;
			double theta = atan2( vals[1], vals[0] ) * 57.29577951308232;
			double phi = acos( vals[2] / len ) * 57.29577951308232;
			theta -= 90;
			phi -= 90;
			phi = - phi;

			theta = (fabs(theta) < 1e-5 ? 0 : theta);
			phi = (fabs(phi) < 1e-5 ? 0 : phi);
			sliceAtts->SetTheta( theta );
			sliceAtts->SetPhi( phi );
		    }
		}
	    }
        }

        if(!okay)
        {
            const double *d = sliceAtts->GetNormal();
            msg.sprintf("The normal vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", d[0], d[1], d[2]);
            Message(msg);
            sliceAtts->SetNormal(d);
        }
    }

    // Do the UpAxis vector
    if(which_widget == 2 || doAll)
    {
        temp = upAxisLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.latin1(), "%lg %lg %lg",
                           &vals[0], &vals[1], &vals[2]) == 3);
            if(okay)
            {
                okay = (vals[0] != 0. || vals[1] != 0. || vals[2] != 0. );
                if(okay)
                    sliceAtts->SetUpAxis(vals);
            }
        }

        if(!okay)
        {
            const double *d = sliceAtts->GetUpAxis();
            msg.sprintf("The upAxis vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", d[0], d[1], d[2]);
            Message(msg);
            sliceAtts->SetUpAxis(d);
        }
    }

    // Do the origin (point)
    if(which_widget == 3 || doAll)
    {
        temp = originPointLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.latin1(), "%lg %lg %lg",
                           &vals[0], &vals[1], &vals[2]) == 3);
            if(okay)
                sliceAtts->SetOriginPoint(vals);
        }

        if(!okay)
        {
            const double *d = sliceAtts->GetOriginPoint();
            msg.sprintf("The origin point vector was invalid. "
                "Resetting to the last good value <%g %g %g>.", d[0], d[1], d[2]);
            Message(msg);
            sliceAtts->SetOriginPoint(d);
        }
    }

    // Do the origin (intercept)
    if(which_widget == 4 || doAll)
    {
        temp = originInterceptLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val;
            okay = (sscanf(temp.latin1(), "%lg", &val) == 1);
            if(okay)
                sliceAtts->SetOriginIntercept(val);
        }

        if(!okay)
        {
            const double d = sliceAtts->GetOriginIntercept();
            msg.sprintf("The origin intercept was invalid. "
                "Resetting to the last good value %g.", d);
            Message(msg);
            sliceAtts->SetOriginIntercept(d);
        }
    }

    
    // Do the origin (percent)
    if(which_widget == 5 || doAll)
    {
        temp = originPercentLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val;
            okay = (sscanf(temp.latin1(), "%lg", &val) == 1);
            if(okay)
                sliceAtts->SetOriginPercent(val);
        }

        if(!okay)
        {
            const double d = sliceAtts->GetOriginPercent();
            msg.sprintf("The origin percent was invalid. "
                "Resetting to the last good value %g.", d);
            Message(msg);
            sliceAtts->SetOriginPercent(d);
        }
    }

    // Do the origin (zone)
    if(which_widget == 6 || doAll)
    {
        temp = originZoneLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val;
            okay = (sscanf(temp.latin1(), "%d", &val) == 1);
            if(okay)
                sliceAtts->SetOriginZone(val);
        }

        if(!okay)
        {
            const int d = sliceAtts->GetOriginZone();
            msg.sprintf("The origin zone was invalid. "
                "Resetting to the last good value %d.", d);
            Message(msg);
            sliceAtts->SetOriginZone(d);
        }
    }

    // Do the origin (node)
    if(which_widget == 7 || doAll)
    {
        temp = originNodeLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val;
            okay = (sscanf(temp.latin1(), "%d", &val) == 1);
            if(okay)
                sliceAtts->SetOriginNode(val);
        }

        if(!okay)
        {
            const int d = sliceAtts->GetOriginNode();
            msg.sprintf("The origin node was invalid. "
                "Resetting to the last good value %d.", d);
            Message(msg);
            sliceAtts->SetOriginNode(d);
        }
    }

    // Do the origin zone domain
    if(which_widget == 8 || doAll)
    {
        temp = originZoneDomainLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val;
            okay = (sscanf(temp.latin1(), "%d", &val) == 1);
            if(okay)
                sliceAtts->SetOriginZoneDomain(val);
        }

        if(!okay)
        {
            const int d = sliceAtts->GetOriginZoneDomain();
            msg.sprintf("The origin domain was invalid. "
                "Resetting to the last good value %d.", d);
            Message(msg);
            sliceAtts->SetOriginZoneDomain(d);
        }
    }

    // Do the origin node domain
    if(which_widget == 9 || doAll)
    {
        temp = originNodeDomainLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val;
            okay = (sscanf(temp.latin1(), "%d", &val) == 1);
            if(okay)
                sliceAtts->SetOriginNodeDomain(val);
        }

        if(!okay)
        {
            const int d = sliceAtts->GetOriginNodeDomain();
            msg.sprintf("The origin domain was invalid. "
                "Resetting to the last good value %d.", d);
            Message(msg);
            sliceAtts->SetOriginNodeDomain(d);
        }
    }
    // Do the meshName 
    if(which_widget == 10 || doAll)
    {
        temp = meshName->currentText();
        okay = !temp.isEmpty();
        if(okay)
        {
            sliceAtts->SetMeshName(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of meshName was invalid. " 
                "Resetting to the last good value of %s.", 
                sliceAtts->GetMeshName().c_str());
            Message(msg);
            sliceAtts->SetMeshName(sliceAtts->GetMeshName());
        }
    }

    // Do theta/phi
    if(which_widget == 11 || doAll)
    {
        temp = thetaPhiLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.latin1(), "%lg %lg", &vals[0], &vals[1]) == 2);
            if(okay)
            {
		sliceAtts->SetTheta( vals[0] );
		sliceAtts->SetPhi( vals[1] );
		//Calculate the normal.
		if ( sliceAtts->GetAxisType() == SliceAttributes::ThetaPhi )
		{
		    vals[0] -= 90;
		    vals[1] -= 90;
		    vals[0] *= 0.017453292519943295;
		    vals[1] *= 0.017453292519943295;
		    double n[3] = { cos(vals[0])*sin(vals[1]),
				    sin(vals[0])*sin(vals[1]),
				    cos(vals[1]) };
		    static double eps = 1e-5;
		    n[0] = (fabs(n[0]) < eps ? 0 : n[0]);
		    n[1] = (fabs(n[1]) < eps ? 0 : n[1]);
		    n[2] = (fabs(n[2]) < eps ? 0 : n[2]);
		    sliceAtts->SetNormal( n );
		}
            }
        }

        if(!okay)
        {
	    double theta = sliceAtts->GetTheta();
	    double phi = sliceAtts->GetPhi();
            msg.sprintf( "The theta-phi angles were invalid. "
			 "Resetting to the last good value <%g %g>.", theta, phi );
            Message(msg);
	    sliceAtts->SetTheta( theta );
	    sliceAtts->SetPhi( phi );
        }
    }
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisSliceWindow::originTypeChanged
//
// Purpose: 
//   This is a Qt slot function that sets the type of origin being used.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Jeremy Meredith
// Creation:   April 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::originTypeChanged(int index)
{
    sliceAtts->SetOriginType(SliceAttributes::OriginType(index));
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginText
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:41:28 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginPointText()
{
    GetCurrentValues(3);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginInterceptText
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector.
//
// Programmer: Jeremy Meredith
// Creation:   April 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginInterceptText()
{
    GetCurrentValues(4);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginPercentText
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector.
//
// Programmer: Jeremy Meredith
// Creation:   April 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginPercentText()
{
    GetCurrentValues(5);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::originPercentSliderChanged
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector using a slider.
//
// Programmer: Jeremy Meredith
// Creation:   May  5, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::originPercentSliderChanged(int pct)
{
    sliceAtts->SetOriginPercent(pct);

    QString tmp;
    tmp.sprintf("%d", pct);
    originPercentLineEdit->setText(tmp);

    if (!sliderDragging)
        Apply();
}

// ****************************************************************************
//  Method:  QvisSliceWindow::originPercentSliderPressed
//
//  Purpose:
//    Keep track of when the user is dragging the slider.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  5, 2003
//
// ****************************************************************************

void
QvisSliceWindow::originPercentSliderPressed()
{
    sliderDragging = true;
}

// ****************************************************************************
//  Method:  QvisSliceWindow::originPercentSliderReleased
//
//  Purpose:
//    Keep track of when the user is dragging the slider.
//    Update everything when they let go.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  5, 2003
//
// ****************************************************************************

void
QvisSliceWindow::originPercentSliderReleased()
{
    sliderDragging = false;
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginZoneDomainText
//
// Purpose: 
//   This is a Qt slot function that sets node/zone domain number.
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginZoneDomainText()
{
    GetCurrentValues(8);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginNodeDomainText
//
// Purpose: 
//   This is a Qt slot function that sets node/zone domain number.
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginNodeDomainText()
{
    GetCurrentValues(9);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginZoneText
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector.
//
// Programmer: Jeremy Meredith
// Creation:   April 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginZoneText()
{
    GetCurrentValues(6);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processOriginNodeText
//
// Purpose: 
//   This is a Qt slot function that sets the origin vector.
//
// Programmer: Jeremy Meredith
// Creation:   April 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processOriginNodeText()
{
    GetCurrentValues(7);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processNormalText
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:41:28 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processNormalText()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processThetaPhiText
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Dave Pugmire
// Creation:   Thu Oct 18 08:25:42 EDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processThetaPhiText()
{
    GetCurrentValues(11);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::processUpAxisText
//
// Purpose: 
//   This is a Qt slot function that sets the upAxis vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:41:28 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::processUpAxisText()
{
    GetCurrentValues(2);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::flipNormalToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the normal/upaxis should be set to flip the projected axes.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Jeremy Meredith
// Creation:   November 17, 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::flipNormalToggled(bool val)
{
    sliceAtts->SetFlip(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::projectToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the slice should be projected to 2d.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:43:06 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::projectToggled(bool val)
{
    sliceAtts->SetProject2d(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::interactiveToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the slice should be set from the interactive tool.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 9 17:41:53 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSliceWindow::interactiveToggled(bool val)
{
    sliceAtts->SetInteractive(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSliceWindow::normalTypeChanged
//
// Purpose: 
//   This is a Qt slot function that sets the orthogonal normal type.
//
// Arguments:
//   index : Which axis to use.
//
// Programmer: Jeremy Meredith
// Creation:   November 17, 2002
//
// Modifications:
//   Brad Whitlock, Mon Nov 18 10:33:31 PDT 2002
//   I set the text in the lineedits so it doesn't get wiped out when
//   autoupdate is on.
//
//   Jeremy Meredith, Mon Aug 18 15:59:16 PDT 2003
//   I added "smart" switching from point-origin to intercept-origin
//   based on if you are switching from arbitrary to orthogonal axis
//   types, or back again.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Dave Pugmire, Thu Oct 18 08:25:42 EDT 2007
//   Added theta-phi method of editing the plane normal.
//
// ****************************************************************************

void
QvisSliceWindow::normalTypeChanged(int index)
{
    QString temp;

    // If we're switching from arbitrary to orthogonal 
    if (sliceAtts->GetAxisType() == SliceAttributes::Arbitrary &&
        sliceAtts->GetOriginType() == SliceAttributes::Point   &&
        (index==0 || index==1 || index==2))
    {
        double intercept; 
        if (index == 0) // x axis
        {
            intercept = sliceAtts->GetOriginPoint()[0];
        }
        else if (index == 1) // y axis
        {
            intercept = sliceAtts->GetOriginPoint()[1];
        }
        else // (index == 2) // z axis
        {
            intercept = sliceAtts->GetOriginPoint()[2];
        }
        sliceAtts->SetOriginType(SliceAttributes::Intercept);
        sliceAtts->SetOriginIntercept(intercept);
        temp.setNum(intercept);
        originInterceptLineEdit->setText(temp);
        UpdateOriginArea();
    }

    // If we're switching from orthogonal to arbitrary
    if ((sliceAtts->GetAxisType() == SliceAttributes::XAxis ||
         sliceAtts->GetAxisType() == SliceAttributes::YAxis ||
         sliceAtts->GetAxisType() == SliceAttributes::ZAxis)     &&
        sliceAtts->GetOriginType() == SliceAttributes::Intercept &&
        index == 3)
    {
        sliceAtts->SetOriginType(SliceAttributes::Point);
        UpdateOriginArea();
    }

    sliceAtts->SetAxisType(SliceAttributes::AxisType(index));

    // Change the normal and upaxis here or it will not change if autoupdate is
    // on because of Apply's call to GetCurrentValues.
    double *dptr = sliceAtts->GetNormal();
    temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
    normalLineEdit->setText(temp);
    dptr = sliceAtts->GetUpAxis();
    temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
    upAxisLineEdit->setText(temp);
    temp.sprintf("%g %g", sliceAtts->GetTheta(), sliceAtts->GetPhi() );
    thetaPhiLineEdit->setText( temp );

    Apply();
}

// ****************************************************************************
//  Method: QvisSliceWindow::UpdateMeshNames
//
//  Purpose: 
//    This method retrieves the mesh names from the fileServer and stores
//    them in the meshName combo box.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 25, 2005 
//
//  Modifications:
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

void
QvisSliceWindow::UpdateMeshNames()
{
    meshName->blockSignals(true);
    meshName->clear();
    if (fileServer) 
    {
        const avtDatabaseMetaData *md = 
            fileServer->GetMetaData(fileServer->GetOpenFile(),
                                    GetStateForSource(fileServer->GetOpenFile()),
                                    !FileServerList::ANY_STATE,
                                    !FileServerList::GET_NEW_MD);
        if (md)
        {
            stringVector meshNames = md->GetAllMeshNames();
            for (int i = 0; i < meshNames.size(); i++)
            {
                meshName->insertItem(QString(meshNames[i].c_str()));
            }
        }
        if (meshName->count() != 0)
        {
            QString mn(sliceAtts->GetMeshName().c_str());
            if (mn == defaultItem)
            {
                meshName->setCurrentItem (0);
            }
            else
            {
                QListBox *lb = meshName->listBox();
                int idx = lb->index(lb->findItem(mn));
                idx = (idx == -1 ? 0 : idx);
                meshName->setCurrentItem (idx);
            }
        }
    }
    if (meshName->count() == 0)
    {
        meshName->insertItem(defaultItem);
        meshName->setCurrentItem(0);
        meshName->setEnabled(false);
        meshLabel->setEnabled(false);
    }
    else if (meshName->count() == 1)
    {
        meshName->setEnabled(false);
        meshLabel->setEnabled(false);
    }
    else 
    {
        meshName->setEnabled(true);
        meshLabel->setEnabled(true);
    }
}


// ****************************************************************************
//  Method: QvisSliceWindow::meshNameChanged
//
//  Purpose: 
//   This is a Qt slot function that sets the meshName. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 25, 2005 
//
//  Modifications:
//
// ****************************************************************************

void
QvisSliceWindow::meshNameChanged()
{
    GetCurrentValues(10);
    Apply();
}
