/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <math.h>
#include <stdio.h> // for sscanf

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>

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
//   Jeremy Meredith, Mon May  5 14:40:32 PDT 2003
//   Added "sliderDragging" to keep track of when the user is dragging
//   the slice-by-percent slider.
//
//   Kathleen Bonnell, Tue Jan 25 08:15:23 PST 2005
//   Initialize defaultItem, attach fileServer.
//
// ****************************************************************************

QvisSliceWindow::QvisSliceWindow(const int type,
    SliceAttributes *subj, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    sliceAtts = subj;
    sliderDragging = false;
    originTypeGroup = 0;
    normalTypeGroup = 0;
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
//   Brad Whitlock, Thu Apr 24 16:33:00 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Tue Jul  8 14:48:38 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisSliceWindow::CreateWindowContents()
{
    // Normal
    QGroupBox *normalBox = new QGroupBox(central);
    normalBox->setTitle(tr("Normal"));
    topLayout->addWidget(normalBox);

    QGridLayout *normalLayout = new QGridLayout(normalBox);
    normalLayout->setMargin(10);
    normalLayout->setSpacing(5);

    normalTypeGroup = new QButtonGroup(normalBox);
    connect(normalTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(normalTypeChanged(int)));

    //    Orthogonal
    normalLayout->addWidget(new QLabel(tr("Orthogonal"), normalBox), 1,0);

    QHBoxLayout *orthogonalLayout = new QHBoxLayout();

    QRadioButton *xAxis = new QRadioButton(tr("X Axis"), normalBox);
    QRadioButton *yAxis = new QRadioButton(tr("Y Axis"), normalBox);
    QRadioButton *zAxis = new QRadioButton(tr("Z Axis"), normalBox);
    
    normalTypeGroup->addButton(xAxis,0);
    normalTypeGroup->addButton(yAxis,1);
    normalTypeGroup->addButton(zAxis,2);
    
    orthogonalLayout->addWidget(xAxis);
    orthogonalLayout->addWidget(yAxis);
    orthogonalLayout->addWidget(zAxis);

    flipNormalToggle = new QCheckBox(tr("flip"), normalBox);
    connect(flipNormalToggle, SIGNAL(toggled(bool)),
            this, SLOT(flipNormalToggled(bool)));
    orthogonalLayout->addWidget(flipNormalToggle);

    normalLayout->addLayout(orthogonalLayout, 1,1);

    //    Arbitrary
    normalLayout->addWidget(new QLabel(tr("Arbitrary"), normalBox), 2,0);

    QHBoxLayout *arbitraryLayout = new QHBoxLayout();

    QRadioButton *arbAxis = new QRadioButton(" ", normalBox);
    normalTypeGroup->addButton(arbAxis,3);
    arbitraryLayout->addWidget(arbAxis);

    normalLineEdit = new QLineEdit(normalBox);
    connect(normalLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNormalText()));
    arbitraryLayout->addWidget(normalLineEdit);

    normalLayout->addLayout(arbitraryLayout, 2,1);


    //Theta-phi
    normalLayout->addWidget( new QLabel( tr("Theta-Phi"), normalBox ), 3, 0 );

    QHBoxLayout *thetaPhiLayout = new QHBoxLayout();

    QRadioButton *thetaPhiRadio = new QRadioButton(" ", normalBox);
    normalTypeGroup->addButton(thetaPhiRadio,4);
    thetaPhiLayout->addWidget(thetaPhiRadio);

    thetaPhiLineEdit = new QLineEdit(normalBox);
    connect( thetaPhiLineEdit, SIGNAL(returnPressed()),
             this, SLOT(processThetaPhiText()));
    thetaPhiLayout->addWidget( thetaPhiLineEdit );

    normalLayout->addLayout( thetaPhiLayout, 3, 1 );

    // Origin
    
    QGroupBox *originBox = new QGroupBox(central);
    originBox->setTitle(tr("Origin"));
    topLayout->addWidget(originBox);
    
    originTypeGroup = new QButtonGroup(originBox);
    connect(originTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(originTypeChanged(int)));

    
    QGridLayout *originLayout = new QGridLayout(originBox);

    QHBoxLayout *originTypeLayout = new QHBoxLayout();
    originTypeLayout->setMargin(0);
    originLayout->addLayout(originTypeLayout, 1,0,  1,1);

    QRadioButton *otPoint     = new QRadioButton(tr("Point"),     originBox);
    QRadioButton *otIntercept = new QRadioButton(tr("Intercept"), originBox);
    QRadioButton *otPercent   = new QRadioButton(tr("Percent"),   originBox);
    QRadioButton *otZone      = new QRadioButton(tr("Zone"),      originBox);
    QRadioButton *otNode      = new QRadioButton(tr("Node"),      originBox);
    originTypeGroup->addButton(otPoint,0);
    originTypeGroup->addButton(otIntercept,1);
    originTypeGroup->addButton(otPercent,2);
    originTypeGroup->addButton(otZone,3);
    originTypeGroup->addButton(otNode,4);
    originTypeLayout->addWidget(otPoint);
    originTypeLayout->addWidget(otIntercept);
    originTypeLayout->addWidget(otPercent);
    originTypeLayout->addWidget(otZone);
    originTypeLayout->addWidget(otNode);


    // -- origin (point)
    originPointLayout = new QHBoxLayout();
    originPointLayout->setMargin(0);
    originPointLabel = new QLabel(tr("Point"), originBox);

    originPointLineEdit = new QLineEdit(originBox);
    connect(originPointLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginPointText()));

    originPointLayout->addWidget(originPointLabel);
    originPointLayout->addWidget(originPointLineEdit);

    originLayout->addLayout(originPointLayout, 2,0);

    // -- origin (intercept)
    originInterceptLayout = new QHBoxLayout();
    originInterceptLayout->setMargin(0);
    originInterceptLabel = new QLabel(tr("Intercept"), originBox);

    originInterceptLineEdit = new QLineEdit(originBox);
    connect(originInterceptLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginInterceptText()));

    originInterceptLayout->addWidget(originInterceptLabel);
    originInterceptLayout->addWidget(originInterceptLineEdit);

    originLayout->addLayout(originInterceptLayout, 3,0);

    // -- origin (percent)
    originPercentLayout = new QHBoxLayout();
    originPercentLayout->setMargin(0);
    originPercentLabel = new QLabel(tr("Percent"), originBox);

    originPercentLineEdit = new QNarrowLineEdit(originBox);
    connect(originPercentLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginPercentText()));

    originPercentSlider = new QSlider(Qt::Horizontal, originBox);
    originPercentSlider->setRange(0,100);
    originPercentSlider->setSingleStep(10);
    originPercentSlider->setValue(50);

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
    originZoneLayout = new QHBoxLayout();
    originZoneLayout->setMargin(0);
    originZoneLabel = new QLabel(tr("Zone"), originBox);

    originZoneLineEdit = new QLineEdit(originBox);
    connect(originZoneLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginZoneText()));

    originZoneDomainLabel = new QLabel(tr("Domain"), originBox);
    originZoneDomainLineEdit = new QLineEdit(originBox);
    connect(originZoneDomainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginZoneDomainText()));

    originZoneLayout->addWidget(originZoneLabel);
    originZoneLayout->addWidget(originZoneLineEdit);
    originZoneLayout->addWidget(originZoneDomainLabel);
    originZoneLayout->addWidget(originZoneDomainLineEdit);

    originLayout->addLayout(originZoneLayout, 5,0);

    // -- origin (node)
    originNodeLayout = new QHBoxLayout();
    originNodeLayout->setMargin(0);
    originNodeLabel = new QLabel(tr("Node"), originBox);

    originNodeLineEdit = new QLineEdit(originBox);
    connect(originNodeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginNodeText()));

    originNodeDomainLabel = new QLabel(tr("Domain"), originBox);
    originNodeDomainLineEdit = new QLineEdit(originBox);
    connect(originNodeDomainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOriginNodeDomainText()));

    originNodeLayout->addWidget(originNodeLabel);
    originNodeLayout->addWidget(originNodeLineEdit);
    originNodeLayout->addWidget(originNodeDomainLabel);
    originNodeLayout->addWidget(originNodeDomainLineEdit);

    originLayout->addLayout(originNodeLayout, 6,0);

    // mesh name
    QHBoxLayout *meshLayout = new QHBoxLayout();
    meshLayout->setMargin(5);
    meshLabel = new QLabel(tr("Mesh"), originBox);
    meshName = new QComboBox(originBox);
    meshName->setEditable(true);
    meshName->setAutoCompletion(true);
    meshName->setInsertPolicy(QComboBox::NoInsert);
    meshName->addItem(defaultItem);
    meshName->setCurrentIndex(0);
    meshName->setEditText(defaultItem);
    connect(meshName, SIGNAL(activated(int)),   
            this, SLOT(meshNameChanged()));
    meshLayout->addWidget(meshLabel);
    meshLayout->addWidget(meshName);
    originLayout->addLayout(meshLayout, 7, 0);

    // Up Axis
    QGroupBox *upAxisBox = new QGroupBox(central);
    upAxisBox->setTitle(tr("Up Axis"));
    topLayout->addWidget(upAxisBox);

    QGridLayout *upAxisLayout = new QGridLayout(upAxisBox);
    upAxisLayout->setSpacing(5);

    projectToggle = new QCheckBox(tr("Project to 2D"), upAxisBox);
    connect(projectToggle, SIGNAL(toggled(bool)),
            this, SLOT(projectToggled(bool)));
    upAxisLayout->addWidget(projectToggle, 0, 0, 1, 2);

    upAxisLineEdit = new QLineEdit(upAxisBox);
    connect(upAxisLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processUpAxisText()));
    upAxisLayout->addWidget(upAxisLineEdit, 1, 1);
    upAxisLabel = new QLabel(tr("Direction"), upAxisBox);
    upAxisLayout->addWidget(upAxisLabel, 1, 0);

    interactiveToggle = new QCheckBox(tr("Interactive"), central);
    connect(interactiveToggle, SIGNAL(toggled(bool)),
            this, SLOT(interactiveToggled(bool)));
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
//   Cyrus Harrison, Tue Jul  8 14:48:38 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Wed Jul  9 13:48:07 PDT 2008
//   Made it use ids.
//
//   Hank Childs, Thu Oct  9 11:13:19 PDT 2008
//   Update test for what is "orthogonal" (theta-phi is not!).
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

    bool orthogonal = sliceAtts->GetAxisType() != SliceAttributes::Arbitrary &&
                      sliceAtts->GetAxisType() != SliceAttributes::ThetaPhi;


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
          case SliceAttributes::ID_originType:
            originTypeGroup->blockSignals(true);
            originTypeGroup->button(int(sliceAtts->GetOriginType()))->setChecked(true);
            originTypeGroup->blockSignals(false);
            break;
          case SliceAttributes::ID_originPoint:
            dptr = sliceAtts->GetOriginPoint();
            originPointLineEdit->setText(DoublesToQString(dptr,3));
            break;
          case SliceAttributes::ID_originIntercept:
            temp.setNum(sliceAtts->GetOriginIntercept());
            originInterceptLineEdit->setText(temp);
            break;
          case SliceAttributes::ID_originPercent:
            temp.setNum(sliceAtts->GetOriginPercent());
            originPercentLineEdit->setText(temp);
            ival = int(qMin(qMax(0.0,sliceAtts->GetOriginPercent()),100.0));
            originPercentSlider->blockSignals(true);
            originPercentSlider->setValue(ival);
            originPercentSlider->blockSignals(false);
            break;
          case SliceAttributes::ID_originZone:
            temp.sprintf("%d", sliceAtts->GetOriginZone());
            originZoneLineEdit->setText(temp);
            break;
          case SliceAttributes::ID_originNode:
            temp.sprintf("%d", sliceAtts->GetOriginNode());
            originNodeLineEdit->setText(temp);
            break;
          case SliceAttributes::ID_normal:
            dptr = sliceAtts->GetNormal();
            normalLineEdit->setText(DoublesToQString(dptr, 3));
            break;
          case SliceAttributes::ID_axisType:
            normalTypeGroup->blockSignals(true);
            normalTypeGroup->button(int(sliceAtts->GetAxisType()))->setChecked(true);
            normalTypeGroup->blockSignals(false);

            normalLineEdit->setEnabled(!orthogonal);
            thetaPhiLineEdit->setEnabled( sliceAtts->GetAxisType() == SliceAttributes::ThetaPhi );
            upAxisLineEdit->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            upAxisLabel->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            break;
          case SliceAttributes::ID_upAxis:
            dptr = sliceAtts->GetUpAxis();
            upAxisLineEdit->setText(DoublesToQString(dptr, 3));
            break;
          case SliceAttributes::ID_project2d:
            projectToggle->blockSignals(true);
            projectToggle->setChecked(sliceAtts->GetProject2d());
            projectToggle->blockSignals(false);

            flipNormalToggle->setEnabled(sliceAtts->GetProject2d());
            upAxisLineEdit->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            upAxisLabel->setEnabled(!orthogonal && sliceAtts->GetProject2d());
            break;
          case SliceAttributes::ID_interactive:
            interactiveToggle->blockSignals(true);
            interactiveToggle->setChecked(sliceAtts->GetInteractive());
            interactiveToggle->blockSignals(false);
            break;
          case SliceAttributes::ID_flip:
            flipNormalToggle->blockSignals(true);
            flipNormalToggle->setChecked(sliceAtts->GetFlip());
            flipNormalToggle->setEnabled(sliceAtts->GetProject2d());
            flipNormalToggle->blockSignals(false);
            break;
          case SliceAttributes::ID_originZoneDomain:
            temp.sprintf("%d", sliceAtts->GetOriginZoneDomain());
            originZoneDomainLineEdit->setText(temp);
            break;
          case SliceAttributes::ID_originNodeDomain:
            temp.sprintf("%d", sliceAtts->GetOriginNodeDomain());
            originNodeDomainLineEdit->setText(temp);
            break;
          case SliceAttributes::ID_theta:
          case SliceAttributes::ID_phi:
            temp.sprintf( "%g %g", sliceAtts->GetTheta(), sliceAtts->GetPhi());
            thetaPhiLineEdit->setText(temp);
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
//   Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//   Removed dependencies on vtkMath.
//
//   Cyrus Harrison, Tue Jul  8 14:48:38 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Wed Jul  9 13:56:57 PDT 2008
//   Made it use ids and used helper methods.
//
// ****************************************************************************

void
QvisSliceWindow::GetCurrentValues(int which_widget)
{
    bool    okay, doAll = (which_widget == -1);
    double  vals[3];
    QString msg, temp;

    // Do the normal vector
    if(which_widget == SliceAttributes::ID_normal || doAll)
    {
        if((okay = LineEditGetDoubles(normalLineEdit, vals, 3)) == true)
        {
            okay = (vals[0] != 0. || vals[1] != 0. || vals[2] != 0. );
            if (okay)
            {
                sliceAtts->SetNormal(vals);

                if (sliceAtts->GetAxisType() != SliceAttributes::ThetaPhi)
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
                    sliceAtts->SetTheta(theta);
                    sliceAtts->SetPhi(phi);
                }
            }
        }

        if(!okay)
        {
            ResettingError(tr("normal vector"),
                DoublesToQString(sliceAtts->GetNormal(), 3));
            sliceAtts->SetNormal(sliceAtts->GetNormal());
        }
    }

    // Do the UpAxis vector
    if(which_widget == SliceAttributes::ID_upAxis || doAll)
    {
        if((okay = LineEditGetDoubles(upAxisLineEdit, vals, 3)) == true)
        {
            okay = (vals[0] != 0. || vals[1] != 0. || vals[2] != 0. );
            if (okay)
                sliceAtts->SetUpAxis(vals);
        }
        if(!okay)
        {
            ResettingError(tr("upAxis vector"),
                DoublesToQString(sliceAtts->GetUpAxis(), 3));
            sliceAtts->SetUpAxis(sliceAtts->GetUpAxis());
        }
    }

    // Do the origin (point)
    if(which_widget == SliceAttributes::ID_originPoint || doAll)
    {
        if(LineEditGetDoubles(originPointLineEdit, vals, 3))
            sliceAtts->SetOriginPoint(vals);
        else
        {
            ResettingError(tr("origin point vector"),
                DoublesToQString(sliceAtts->GetOriginPoint(), 3));
            sliceAtts->SetOriginPoint(sliceAtts->GetOriginPoint());
        }
    }

    // Do the origin (intercept)
    if(which_widget == SliceAttributes::ID_originIntercept || doAll)
    {
        if(LineEditGetDouble(originInterceptLineEdit, vals[0]))
            sliceAtts->SetOriginIntercept(vals[0]);
        else
        {
            ResettingError(tr("origin intercept"),
                DoubleToQString(sliceAtts->GetOriginIntercept()));
            sliceAtts->SetOriginIntercept(sliceAtts->GetOriginIntercept());
        }
    }

    
    // Do the origin (percent)
    if(which_widget == SliceAttributes::ID_originPercent || doAll)
    {
        if(LineEditGetDouble(originPercentLineEdit, vals[0]))
            sliceAtts->SetOriginPercent(vals[0]);
        else
        {
            ResettingError(tr("origin percent"),
                DoubleToQString(sliceAtts->GetOriginPercent()));
            sliceAtts->SetOriginPercent(sliceAtts->GetOriginPercent());
        }
    }

    // Do the origin (zone)
    if(which_widget == SliceAttributes::ID_originZone || doAll)
    {
        int val;
        if(LineEditGetInt(originZoneLineEdit, val))
            sliceAtts->SetOriginZone(val);
        else
        {
            ResettingError(tr("origin zone"),
                IntToQString(sliceAtts->GetOriginZone()));
            sliceAtts->SetOriginZone(sliceAtts->GetOriginZone());
        }
    }

    // Do the origin (node)
    if(which_widget == SliceAttributes::ID_originNode || doAll)
    {
        int val;
        if(LineEditGetInt(originNodeLineEdit, val))
            sliceAtts->SetOriginNode(val);
        else
        {
            ResettingError(tr("origin node"),
                IntToQString(sliceAtts->GetOriginNode()));
            sliceAtts->SetOriginNode(sliceAtts->GetOriginNode());
        }
    }

    // Do the origin zone domain
    if(which_widget == SliceAttributes::ID_originZoneDomain || doAll)
    {
        int val;
        if(LineEditGetInt(originZoneDomainLineEdit, val))
            sliceAtts->SetOriginZoneDomain(val);
        else
        {
            ResettingError(tr("origin domain"),
                IntToQString(sliceAtts->GetOriginZoneDomain()));
            sliceAtts->SetOriginZoneDomain(sliceAtts->GetOriginZoneDomain());
        }
    }

    // Do the origin node domain
    if(which_widget == SliceAttributes::ID_originNodeDomain || doAll)
    {
        int val;
        if(LineEditGetInt(originNodeDomainLineEdit, val))
            sliceAtts->SetOriginNodeDomain(val);
        else
        {
            ResettingError(tr("origin domain"),
                IntToQString(sliceAtts->GetOriginNodeDomain()));
            sliceAtts->SetOriginNodeDomain(sliceAtts->GetOriginNodeDomain());
        }
    }


    // Do the meshName 
    if(which_widget == SliceAttributes::ID_meshName || doAll)
    {
        temp = meshName->currentText();
        okay = !temp.isEmpty();
        if(okay)
        {
            sliceAtts->SetMeshName(temp.toStdString());
        }

        if(!okay)
        {
            ResettingError(tr("mesh name"), sliceAtts->GetMeshName().c_str());
            sliceAtts->SetMeshName(sliceAtts->GetMeshName());
        }
    }

    // Do theta/phi
    if(which_widget == SliceAttributes::ID_theta || doAll)
    {
        if(LineEditGetDoubles(thetaPhiLineEdit, vals, 2))
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
        else
        {
            double theta = sliceAtts->GetTheta();
            double phi = sliceAtts->GetPhi();
            QString angles; angles.sprintf("<%g %g>", theta, phi);
            ResettingError(tr("theta-phi angles"), angles);
            sliceAtts->SetTheta(theta);
            sliceAtts->SetPhi(phi);
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
    GetCurrentValues(SliceAttributes::ID_originPoint);
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
    GetCurrentValues(SliceAttributes::ID_originIntercept);
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
    GetCurrentValues(SliceAttributes::ID_originPercent);
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
    GetCurrentValues(SliceAttributes::ID_originZoneDomain);
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
    GetCurrentValues(SliceAttributes::ID_originNodeDomain);
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
    GetCurrentValues(SliceAttributes::ID_originZone);
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
    GetCurrentValues(SliceAttributes::ID_originNode);
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
    GetCurrentValues(SliceAttributes::ID_normal);
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
    GetCurrentValues(SliceAttributes::ID_theta);
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
    GetCurrentValues(SliceAttributes::ID_upAxis);
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
//   Brad Whitlock, Wed Jul  9 14:03:54 PDT 2008
//   Use helper methods.
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
    normalLineEdit->setText(DoublesToQString(sliceAtts->GetNormal(), 3));
    upAxisLineEdit->setText(DoublesToQString(sliceAtts->GetUpAxis(), 3));
    temp.sprintf("%g %g", sliceAtts->GetTheta(), sliceAtts->GetPhi());
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
//
//   Cyrus Harrison, Tue Jul  8 14:48:38 PDT 2008
//   Initial Qt4 Port.
//
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
                meshName->addItem(QString(meshNames[i].c_str()));
            }
        }
        if (meshName->count() != 0)
        {
            QString mn(sliceAtts->GetMeshName().c_str());
            if (mn == defaultItem)
            {
                meshName->setCurrentIndex(0);
            }
            else
            {
                int idx = meshName->findText(mn);
                idx = (idx == -1 ? 0 : idx);
                meshName->setCurrentIndex(idx);
            }
        }
    }
    if (meshName->count() == 0)
    {
        meshName->addItem(defaultItem);
        meshName->setCurrentIndex(0);
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
    GetCurrentValues(SliceAttributes::ID_meshName);
    Apply();
}
