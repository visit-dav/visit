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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <QvisViewWindow.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qslider.h>
#include <QNarrowLineEdit.h>

#include <DataNode.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ViewerProxy.h>
#include <enumtypes.h>

#define MIN_LINEEDIT_WIDTH 200
#define VIEW_WINDOW_HEIGHT_KLUDGE

// ****************************************************************************
// Method: QvisViewWindow::QvisViewWindow
//
// Purpose: 
//   This is the constructor for the QvisViewWindow class.
//
// Arguments:
//   caption   : The name of the window.
//   shortName : The posted name for the window.
//   notepad   : The notepad area into which the window posts.
//   
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:00:57 PDT 2001
//
// Modificcations:
//   Brad Whitlock, Mon Aug 27 12:28:47 PDT 2001
//   Added some more arguments to the constructor.
//
//   Brad Whitlock, Tue Sep 17 13:00:23 PST 2002
//   I added another subject.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//
// ****************************************************************************

QvisViewWindow::QvisViewWindow(const char *caption, const char *shortName,
    QvisNotepadArea *notepad) : QvisPostableWindowSimpleObserver(caption,
    shortName, notepad, ApplyButton)
{
    viewCurve = 0;
    view2d = 0;
    view3d = 0;
    windowInfo = 0;
    activeTab = 1;
    activeTabSetBySlot = false;
}

// ****************************************************************************
// Method: QvisViewWindow::~QvisViewWindow
//
// Purpose: 
//   The destructor for the QvisViewWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 17 13:00:23 PST 2002
//   I added another subject.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//
// ****************************************************************************

QvisViewWindow::~QvisViewWindow()
{
    if(viewCurve)
        viewCurve->Detach(this);

    if(view2d)
        view2d->Detach(this);

    if(view3d)
        view3d->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}

// ****************************************************************************
// Method: QvisViewWindow::CreateWindowContents
//
// Purpose: 
//   Create the view attributes window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:48:27 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Mon Aug 27 13:03:12 PST 2001
//   I added some default views for 3d.
//
//   Eric Brugger, Tue Aug  6 13:53:25 PDT 2002
//   I added a command line processor.
//
//   Brad Whitlock, Wed Sep 18 10:46:49 PDT 2002
//   I redesigned the window so it makes use of tabs.
//
//   Jeremy Meredith, Tue Feb  4 17:46:51 PST 2003
//   Added controls for camera and view keyframes.
//
//   Eric Brugger, Fri Apr 18 11:47:08 PDT 2003
//   I removed auto center view.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I added controls for image pan and image zoom. I renamed camera
//   to view normal in the view attributes.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//
//   Brad Whitlock, Thu Sep 11 09:32:01 PDT 2003
//   I added buttons to reset and recenter the view.
//
//   Hank Childs, Wed Oct 15 15:04:14 PDT 2003
//   Added eye angle slider.
//
//   Eric Brugger, Thu Oct 16 12:22:54 PDT 2003
//   I added full frame mode to the 2D view tab.
//
//   Eric Brugger, Tue Feb 10 10:30:15 PST 2004
//   I added center of rotation controls to the advanced tab.
//
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Added stuff for auto full frame mode
//
//   Kathleen Bonnell, Thu Mar 22 16:07:56 PDT 2007
//   I added radio buttons for log scaling.
//
// ****************************************************************************

void
QvisViewWindow::CreateWindowContents()
{
    // Group the options into curve, 2d, 3d and advanced tabs.
    tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->setSpacing(5);
    topLayout->addWidget(tabs);

    //
    // Add the controls for the curve view.
    //
    pageCurve = new QVBox(central, "pageCurve");
    pageCurve->setSpacing(5);
    pageCurve->setMargin(10);
    tabs->addTab(pageCurve, "Curve view");

    viewCurveGroup = new QGroupBox(pageCurve, "viewCurveGroup");
    viewCurveGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *internalLayoutCurve = new QVBoxLayout(viewCurveGroup);
    internalLayoutCurve->addSpacing(10);
    QGridLayout *LayoutCurve = new QGridLayout(internalLayoutCurve, 5, 4);
    LayoutCurve->setSpacing(5);

    viewportCurveLineEdit = new QLineEdit(viewCurveGroup, "viewportCurveLineEdit");
    connect(viewportCurveLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportCurveText()));
    LayoutCurve->addMultiCellWidget(viewportCurveLineEdit, 0,0, 1,3);
    QLabel *viewportCurveLabel = new QLabel(viewportCurveLineEdit, "Viewport",
                                       viewCurveGroup, "viewportCurveLabel");
    LayoutCurve->addWidget(viewportCurveLabel, 0, 0);

    domainLineEdit = new QLineEdit(viewCurveGroup, "domainLineEdit");
    connect(domainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDomainText()));
    LayoutCurve->addMultiCellWidget(domainLineEdit, 1,1, 1,3);
    QLabel *domainLabel = new QLabel(domainLineEdit, "Domain",
                                     viewCurveGroup, "domainLabel");
    LayoutCurve->addWidget(domainLabel, 1, 0);
    internalLayoutCurve->addStretch(10);

    rangeLineEdit = new QLineEdit(viewCurveGroup, "rangeLineEdit");
    connect(rangeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRangeText()));
    LayoutCurve->addMultiCellWidget(rangeLineEdit, 2,2, 1,3);
    QLabel *rangeLabel = new QLabel(rangeLineEdit, "Range",
                                    viewCurveGroup, "rangeLabel");
    LayoutCurve->addWidget(rangeLabel, 2, 0);

    QLabel *domainScaleLabel = new QLabel("Domain Scale", viewCurveGroup, 
                                          "domainScaleLabel");
    LayoutCurve->addWidget(domainScaleLabel, 3, 0);
    domainScaleMode = new QButtonGroup(0, "domainScaleMode");
    connect(domainScaleMode, SIGNAL(clicked(int)),
            this, SLOT(domainScaleModeChanged(int)));
    domainLinear = new QRadioButton("Linear", viewCurveGroup, "domainLinear");
    domainScaleMode->insert(domainLinear);
    LayoutCurve->addWidget(domainLinear, 3, 1);
    domainLog = new QRadioButton("Log", viewCurveGroup, "domainLog");
    domainScaleMode->insert(domainLog);
    LayoutCurve->addWidget(domainLog, 3, 2);

    QLabel *rangeScaleLabel = new QLabel("Range Scale", viewCurveGroup, 
                                          "rangeScaleLabel");
    LayoutCurve->addWidget(rangeScaleLabel, 4, 0);
    rangeScaleMode = new QButtonGroup(0, "rangeScaleMode");
    connect(rangeScaleMode, SIGNAL(clicked(int)),
            this, SLOT(rangeScaleModeChanged(int)));
    rangeLinear = new QRadioButton("Linear", viewCurveGroup, "rangeLinear");
    rangeScaleMode->insert(rangeLinear);
    LayoutCurve->addWidget(rangeLinear, 4, 1);
    rangeLog = new QRadioButton("Log", viewCurveGroup, "rangeLog");
    rangeScaleMode->insert(rangeLog);
    LayoutCurve->addWidget(rangeLog, 4, 2);

    internalLayoutCurve->addStretch(10);

    //
    // Add the controls for the 2d view.
    //
    page2D = new QVBox(central, "page2D");
    page2D->setSpacing(5);
    page2D->setMargin(10);
    tabs->addTab(page2D, "2D view");

    view2DGroup = new QGroupBox(page2D, "view2DGroup");
    view2DGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *internalLayout2d = new QVBoxLayout(view2DGroup);
    internalLayout2d->addSpacing(10);
    QGridLayout *Layout2d = new QGridLayout(internalLayout2d, 3, 4);
    Layout2d->setSpacing(5);

    viewportLineEdit = new QLineEdit(view2DGroup, "viewportLineEdit");
    connect(viewportLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportText()));
    Layout2d->addMultiCellWidget(viewportLineEdit, 0, 0, 1, 4);
    QLabel *viewportLabel = new QLabel(viewportLineEdit, "Viewport",
                                       view2DGroup, "viewportLabel");
    Layout2d->addWidget(viewportLabel, 0, 0);

    windowLineEdit = new QLineEdit(view2DGroup, "windowLineEdit");
    connect(windowLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processWindowText()));
    Layout2d->addMultiCellWidget(windowLineEdit, 1, 1, 1, 4);
    QLabel *windowLabel = new QLabel(windowLineEdit, "Window",
                                     view2DGroup, "windowLabel");
    Layout2d->addWidget(windowLabel, 1, 0);
    internalLayout2d->addStretch(10);

    QLabel *fullFrameLabel = new QLabel("Full Frame", view2DGroup, "fullFrameLabel");
    Layout2d->addWidget(fullFrameLabel, 2, 0);
    fullFrameActivationMode = new QButtonGroup(0, "fullFrameActivationMode");
    connect(fullFrameActivationMode, SIGNAL(clicked(int)),
            this, SLOT(fullFrameActivationModeChanged(int)));
    fullFrameAuto = new QRadioButton("Auto", view2DGroup, "Auto");
    fullFrameActivationMode->insert(fullFrameAuto);
    Layout2d->addWidget(fullFrameAuto, 2, 1);
    fullFrameOn = new QRadioButton("On", view2DGroup, "On");
    fullFrameActivationMode->insert(fullFrameOn);
    Layout2d->addWidget(fullFrameOn, 2, 2);
    fullFrameOff = new QRadioButton("Off", view2DGroup, "Off");
    fullFrameActivationMode->insert(fullFrameOff);
    Layout2d->addWidget(fullFrameOff, 2, 3);

    //
    // Add the simple controls for the 3d view.
    //
    page3D = new QVBox(central, "page3D");
    page3D->setSpacing(5);
    page3D->setMargin(10);
    tabs->addTab(page3D, "3D view");

    view3DGroup = new QGroupBox(page3D, "view3DGroup");
    view3DGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *internalLayout3d = new QVBoxLayout(view3DGroup);
    QGridLayout *Layout3d = new QGridLayout(internalLayout3d, 12, 3);
    Layout3d->setSpacing(5);

    normalLineEdit = new QLineEdit(view3DGroup, "normalLineEdit");
    normalLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(normalLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNormalText()));
    Layout3d->addMultiCellWidget(normalLineEdit, 0, 0, 1, 2);
    QLabel *normalLabel = new QLabel(normalLineEdit, "View normal",
                                     view3DGroup, "normalLabel");
    Layout3d->addWidget(normalLabel, 0, 0);

    focusLineEdit = new QLineEdit(view3DGroup, "focusLineEdit");
    focusLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(focusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFocusText()));
    Layout3d->addMultiCellWidget(focusLineEdit, 1, 1, 1, 2);
    QLabel *focusLabel = new QLabel(focusLineEdit, "Focus",
                                     view3DGroup, "focusLabel");
    Layout3d->addWidget(focusLabel, 1, 0);

    upvectorLineEdit = new QLineEdit(view3DGroup, "upvectorLineEdit");
    upvectorLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(upvectorLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processUpVectorText()));
    Layout3d->addMultiCellWidget(upvectorLineEdit, 2, 2, 1, 2);
    QLabel *upvectorLabel = new QLabel(upvectorLineEdit, "Up Vector",
                                       view3DGroup, "upvectorLabel");
    Layout3d->addWidget(upvectorLabel, 2, 0);

    viewAngleLineEdit = new QLineEdit(view3DGroup, "viewAngleLineEdit");
    viewAngleLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(viewAngleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewAngleText()));
    Layout3d->addMultiCellWidget(viewAngleLineEdit, 3, 3, 1, 2);
    QLabel *viewAngleLabel = new QLabel(viewAngleLineEdit, "Angle of view",
                                  view3DGroup, "viewAngleLabel");
    Layout3d->addWidget(viewAngleLabel, 3, 0);

    parallelScaleLineEdit = new QLineEdit(view3DGroup, "parallelScaleEdit");
    parallelScaleLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(parallelScaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processParallelScaleText()));
    Layout3d->addMultiCellWidget(parallelScaleLineEdit, 4, 4, 1, 2);
    QLabel *parallelScaleLabel = new QLabel(parallelScaleLineEdit, "Parallel scale",
                                  view3DGroup, "parallelScaleLabel");
    Layout3d->addWidget(parallelScaleLabel, 4, 0);

    nearLineEdit = new QLineEdit(view3DGroup, "nearLineEdit");
    nearLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(nearLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNearText()));
    Layout3d->addMultiCellWidget(nearLineEdit, 5, 5, 1, 2);
    QLabel *nearLabel = new QLabel(nearLineEdit, "Near clipping",
                                   view3DGroup, "nearLineEditLabel");
    Layout3d->addWidget(nearLabel, 5, 0);

    farLineEdit = new QLineEdit(view3DGroup, "farLineEdit");
    farLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(farLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFarText()));
    Layout3d->addMultiCellWidget(farLineEdit, 6, 6, 1, 2);
    QLabel *farLabel = new QLabel(farLineEdit, "Far clipping",
                                  view3DGroup, "farLineEditLabel");
    Layout3d->addWidget(farLabel, 6, 0);

    imagePanLineEdit = new QLineEdit(view3DGroup, "imagePanLineEdit");
    imagePanLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(imagePanLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processImagePanText()));
    Layout3d->addMultiCellWidget(imagePanLineEdit, 7, 7, 1, 2);
    QLabel *imagePanLabel = new QLabel(imagePanLineEdit, "Image pan",
                                  view3DGroup, "imagePanLineEditLabel");
    Layout3d->addWidget(imagePanLabel, 7, 0);

    imageZoomLineEdit = new QLineEdit(view3DGroup, "imageZoomLineEdit");
    imageZoomLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(imageZoomLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processImageZoomText()));
    Layout3d->addMultiCellWidget(imageZoomLineEdit, 8, 8, 1, 2);
    QLabel *imageZoomLabel = new QLabel(imageZoomLineEdit, "Image zoom",
                                  view3DGroup, "imageZoomLineEditLabel");
    Layout3d->addWidget(imageZoomLabel, 8, 0);

    // portion for modifying the eye angle.
    eyeAngleLineEdit = new QNarrowLineEdit(view3DGroup, "eyeAngleLineEdit");
    connect(eyeAngleLineEdit, SIGNAL(returnPressed()), this,
            SLOT(processEyeAngleText()));
    QLabel *eyeAngleLabel = new QLabel(eyeAngleLineEdit, "Eye Angle (stereo)",
                                  view3DGroup, "eyeAngleLabel");
    eyeAngleSlider = new QSlider(0, 80, 10, 40, Qt::Horizontal,
                                 view3DGroup, "eyeAngleSlider");
    connect(eyeAngleSlider, SIGNAL(valueChanged(int)), this,
            SLOT(eyeAngleSliderChanged(int)));
    Layout3d->addWidget(eyeAngleLabel, 9, 0);
    Layout3d->addWidget(eyeAngleLineEdit, 9, 1);
    Layout3d->addWidget(eyeAngleSlider, 9, 2);

    // Create the check boxes
    perspectiveToggle = new QCheckBox("Perspective", view3DGroup,
        "perspectiveToggle");
    connect(perspectiveToggle, SIGNAL(toggled(bool)),
            this, SLOT(perspectiveToggled(bool)));
    Layout3d->addWidget(perspectiveToggle, 10, 1);

    // Add alignment options
    alignComboBox = new QComboBox(view3DGroup, "");
    alignComboBox->insertItem("", 0);
    alignComboBox->insertItem("-X", 1);
    alignComboBox->insertItem("+X", 2);
    alignComboBox->insertItem("-Y", 3);
    alignComboBox->insertItem("+Y", 4);
    alignComboBox->insertItem("-Z", 5);
    alignComboBox->insertItem("+Z", 6);
    connect(alignComboBox, SIGNAL(activated(int)),
            this, SLOT(viewButtonClicked(int)));
    Layout3d->addWidget(alignComboBox, 11, 1);
    QLabel *alignLabel = new QLabel(alignComboBox, "Align to axis",
        view3DGroup, "alignLabel");
    Layout3d->addWidget(alignLabel, 11, 0);

    //
    // The advanced view options.
    //
    pageAdvanced = new QVBox(central, "pageAdvanced");
    pageAdvanced->setSpacing(5);
    pageAdvanced->setMargin(10);
    tabs->addTab(pageAdvanced, "Advanced");

    QGroupBox *advancedGroup = new QGroupBox(pageAdvanced, "advancedGroup");
    advancedGroup->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *advInternalLayout = new QVBoxLayout(advancedGroup);
    advInternalLayout->addSpacing(10);
    QGridLayout *advLayout = new QGridLayout(advInternalLayout, 7, 3);
    advLayout->setSpacing(5);
    advLayout->setColStretch(1, 10);

    extentComboBox = new QComboBox(advancedGroup, "extentCombo");
    extentComboBox->insertItem("Original spatial extents", 0);
    extentComboBox->insertItem("Actual spatial extents", 1);
    connect(extentComboBox, SIGNAL(activated(int)),
            this, SLOT(extentTypeChanged(int)));
    advLayout->addMultiCellWidget(extentComboBox, 0, 0, 1, 2);
    QLabel *l = new QLabel(extentComboBox, "View based on",
        advancedGroup, "extentsLabel");
    advLayout->addWidget(l, 0, 0);

    lockedViewToggle = new QCheckBox("Locked view", advancedGroup,
        "lockedViewToggle");
    connect(lockedViewToggle, SIGNAL(toggled(bool)),
            this, SLOT(lockedViewChecked(bool)));
    advLayout->addWidget(lockedViewToggle, 1, 0);

    QPushButton *resetViewButton = new QPushButton("Reset view",
        advancedGroup, "resetViewButton");
    connect(resetViewButton, SIGNAL(clicked()),
            this, SLOT(resetView()));
    advLayout->addWidget(resetViewButton, 2, 0);

    QPushButton *recenterButton = new QPushButton("Recenter view",
        advancedGroup, "recenterButton");
    connect(recenterButton, SIGNAL(clicked()),
            this, SLOT(recenterView()));
    advLayout->addWidget(recenterButton, 2, 1);

    QPushButton *undoButton = new QPushButton("Undo view",
        advancedGroup, "undoButton");
    connect(undoButton, SIGNAL(clicked()),
            this, SLOT(undoView()));
    advLayout->addWidget(undoButton, 2, 2);

    copyViewFromCameraToggle = new QCheckBox("Copy view from camera",
                                    advancedGroup, "copyViewFromCameraToggle");
    connect(copyViewFromCameraToggle, SIGNAL(toggled(bool)),
            this, SLOT(copyViewFromCameraChecked(bool)));
    advLayout->addMultiCellWidget(copyViewFromCameraToggle, 3,3, 0,2);

    makeViewKeyframeButton = new QPushButton("Make camera keyframe from view",
                                     advancedGroup, "makeViewKeyframeButton");
    connect(makeViewKeyframeButton, SIGNAL(clicked()),
            this, SLOT(makeViewKeyframe()));
    advLayout->addMultiCellWidget(makeViewKeyframeButton, 4,4, 0,2);

    centerToggle = new QCheckBox("User defined center of rotation",
                                 advancedGroup, "centerToggle");
    connect(centerToggle, SIGNAL(toggled(bool)),
            this, SLOT(centerChecked(bool)));
    advLayout->addMultiCellWidget(centerToggle, 5,5, 0,2);

    centerLineEdit = new QLineEdit(advancedGroup, "centerLineEdit");
    centerLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));
    advLayout->addMultiCellWidget(centerLineEdit, 6, 6, 1, 2);
    QLabel *centerLabel = new QLabel(centerLineEdit, "Center",
                                  advancedGroup, "centerLineEditLabel");
    advLayout->addWidget(centerLabel, 6, 0);

    advInternalLayout->addStretch(10);

    //
    // Add the command line.
    //
    QGridLayout *gLayout = new QGridLayout(topLayout, 1, 2);
 
    commandLineEdit = new QLineEdit(central, "commandLineEdit");
    connect(commandLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCommandText()));
    gLayout->addWidget(commandLineEdit, 0, 1);
    QLabel *commandLabel = new QLabel(commandLineEdit, "Commands",
                                      central, "commandLabel");
    gLayout->addWidget(commandLabel, 0, 0);

    // Initialize the widgets to the right values.
    UpdateWindow(true);
}

void
QvisViewWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == viewCurve)
        viewCurve = 0;
    else if(TheRemovedSubject == view2d)
        view2d = 0;
    else if(TheRemovedSubject == view3d)
        view3d = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

void
QvisViewWindow::ConnectCurveAttributes(ViewCurveAttributes *v)
{
    viewCurve = v;
    viewCurve->Attach(this);
}

void
QvisViewWindow::Connect2DAttributes(View2DAttributes *v)
{
    view2d = v;
    view2d->Attach(this);
}

void
QvisViewWindow::Connect3DAttributes(View3DAttributes *v)
{
    view3d = v;
    view3d->Attach(this);
}

void
QvisViewWindow::ConnectWindowInformation(WindowInformation *w)
{
    windowInfo = w;
    windowInfo->Attach(this);
}

// ****************************************************************************
// Method: QvisViewWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window must update itself.
//
// Arguments:
//   doAll : Whether or not to update the window using all of the data in
//           the state object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:03:56 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 17 13:09:44 PST 2002
//   I added a new subject.
//
//   Brad Whitlock, Tue Sep 17 13:11:04 PST 2002
//   I added another update method call.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//
// ****************************************************************************

void
QvisViewWindow::UpdateWindow(bool doAll)
{
    // Update the appropriate widgets.
    if(SelectedSubject() == viewCurve || doAll)
        UpdateCurve(doAll);
    if(SelectedSubject() == view2d || doAll)
        Update2D(doAll);
    if(SelectedSubject() == view3d || doAll)
        Update3D(doAll);
    if(SelectedSubject() == windowInfo || doAll)
        UpdateGlobal(doAll);
}

// ****************************************************************************
// Method: QvisViewWindow::UpdateCurve
//
// Purpose: 
//   Update the portion of the window for curve views.
//
// Programmer: Eric Brugger
// Creation:   Wed Aug 20 14:04:21 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Mar 22 16:07:56 PDT 2007
//   I added domainScale, RangeScale.
//
// ****************************************************************************

void
QvisViewWindow::UpdateCurve(bool doAll)
{
    if(viewCurve == 0)
        return;

    QString temp;

    for(int i = 0; i < viewCurve->NumAttributes(); ++i)
    {
        if(!viewCurve->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case 0: // domainCoords
          { const double *v = viewCurve->GetDomainCoords();
            temp.sprintf("%g %g", v[0], v[1]);
            domainLineEdit->setText(temp);
            break;
          }
        case 1: // rangeCoords
          { const double *v = viewCurve->GetRangeCoords();
            temp.sprintf("%g %g", v[0], v[1]);
            rangeLineEdit->setText(temp);
            break;
          }
        case 2: // viewportCoords
          { const double *v = viewCurve->GetViewportCoords();
            temp.sprintf("%g %g %g %g", v[0], v[1], v[2], v[3]);
            viewportCurveLineEdit->setText(temp);
            break;
          }
        case 3: // domainScale
          {
            domainScaleMode->blockSignals(true);
            domainScaleMode->setButton(viewCurve->GetDomainScale());
            domainScaleMode->blockSignals(false);
          }
          break;
        case 4: // rangeScale
          {
            rangeScaleMode->blockSignals(true);
            rangeScaleMode->setButton(viewCurve->GetRangeScale());
            rangeScaleMode->blockSignals(false);
          }
          break;
        }
    }
}

// ****************************************************************************
// Method: QvisViewWindow::Update2D
//
// Purpose: 
//   Update the portion of the window for 2d views.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:48:27 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Fri Feb 15 11:45:04 PDT 2002
//   Fixed format strings.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I renamed camera to view normal in the view attributes.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I split the view attributes into 2d and 3d parts.
//
//   Eric Brugger, Thu Oct 16 12:22:54 PDT 2003
//   I added full frame mode to the 2D view tab.
//
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Added logic for auto full frame mode
// ****************************************************************************

void
QvisViewWindow::Update2D(bool doAll)
{
    if(view2d == 0)
        return;

    QString temp;

    for(int i = 0; i < view2d->NumAttributes(); ++i)
    {
        if(!view2d->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case 0: // windowcoords
          { const double *w = view2d->GetWindowCoords();
            temp.sprintf("%g %g %g %g", w[0], w[1], w[2], w[3]);
            windowLineEdit->setText(temp);
            break;
          }
        case 1: // viewportcoords
          { const double *v = view2d->GetViewportCoords();
            temp.sprintf("%g %g %g %g", v[0], v[1], v[2], v[3]);
            viewportLineEdit->setText(temp);
            break;
          }
        case 2: // fullframe.
            View2DAttributes::TriStateMode itmp;
            itmp = view2d->GetFullFrameActivationMode();
            fullFrameActivationMode->blockSignals(true);
            if (itmp == View2DAttributes::On)
               fullFrameActivationMode->setButton(1);
            else if (itmp == View2DAttributes::Off)
               fullFrameActivationMode->setButton(2);
            else
               fullFrameActivationMode->setButton(0);
            fullFrameActivationMode->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisViewWindow::Update3D
//
// Purpose: 
//   Update the portion of the window for 3d views.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:48:27 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Fri Feb 15 11:46:46 PDT 2002
//   Fixed format strings.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I added controls for image pan and image zoom. I renamed camera
//   to view normal in the view attributes.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I split the view attributes into 2d and 3d parts.
//
//   Hank Childs, Wed Oct 15 15:26:03 PDT 2003
//   Added eye angle.
//
//   Eric Brugger, Tue Feb 10 10:30:15 PST 2004
//   I added center of rotation controls to the advanced tab.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Fixed confusion in indices for eyeAngle and perspective members
// ****************************************************************************

void
QvisViewWindow::Update3D(bool doAll)
{
    if(view3d == 0)
        return;

    QString temp;

    for(int i = 0; i < view3d->NumAttributes(); ++i)
    {
        if(!view3d->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case 0: // view normal.
            temp.sprintf("%g %g %g",
                         view3d->GetViewNormal()[0],
                         view3d->GetViewNormal()[1],
                         view3d->GetViewNormal()[2]);
            normalLineEdit->setText(temp);
            break;
        case 1: // focus
            temp.sprintf("%g %g %g",
                         view3d->GetFocus()[0],
                         view3d->GetFocus()[1],
                         view3d->GetFocus()[2]);
            focusLineEdit->setText(temp);
            break;
        case 2: // upvector
            temp.sprintf("%g %g %g",
                         view3d->GetViewUp()[0],
                         view3d->GetViewUp()[1],
                         view3d->GetViewUp()[2]);
            upvectorLineEdit->setText(temp);
            break;
        case 3: // viewAngle
            temp.setNum(view3d->GetViewAngle());
            viewAngleLineEdit->setText(temp);
            break;
        case 4: // parallelScale
            temp.setNum(view3d->GetParallelScale());
            parallelScaleLineEdit->setText(temp);
            break;
        case 5: // near
            temp.setNum(view3d->GetNearPlane());
            nearLineEdit->setText(temp);
            break;
        case 6: // far
            temp.setNum(view3d->GetFarPlane());
            farLineEdit->setText(temp);
            break;
        case 7: // imagePan
            temp.sprintf("%g %g",
                         view3d->GetImagePan()[0],
                         view3d->GetImagePan()[1]);
            imagePanLineEdit->setText(temp);
            break;
        case 8: // imageZoom
            temp.setNum(view3d->GetImageZoom());
            imageZoomLineEdit->setText(temp);
            break;
        case 9: // perspective.
            perspectiveToggle->blockSignals(true);
            perspectiveToggle->setChecked(view3d->GetPerspective());
            perspectiveToggle->blockSignals(false);
            break;
        case 10: // eyeAngle
            temp.setNum(view3d->GetEyeAngle());
            eyeAngleLineEdit->setText(temp);
            UpdateEyeAngleSliderFromAtts();
            break;
        case 11: // centerOfRotationSet.
            centerToggle->blockSignals(true);
            centerToggle->setChecked(view3d->GetCenterOfRotationSet());
            centerToggle->blockSignals(false);
            break;
        case 12: // centerOfRotation.
            temp.sprintf("%g %g %g",
                         view3d->GetCenterOfRotation()[0],
                         view3d->GetCenterOfRotation()[1],
                         view3d->GetCenterOfRotation()[2]);
            centerLineEdit->setText(temp);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisViewWindow::UpdateGlobal
//
// Purpose: 
//   Updates the global widgets.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 17 13:39:26 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Oct 16 10:16:18 PDT 2002
//   I removed the code to set the active pane.
//
//   Jeremy Meredith, Tue Feb  4 17:46:38 PST 2003
//   Added the camera view mode toggle.
//
//   Eric Brugger, Fri Apr 18 11:47:08 PDT 2003
//   I removed auto center view.
//
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Fixed confusion in indices of members of WindowInformation and case labels 
//   Added logic for setting tab to whatever the active window's mode is.
// ****************************************************************************

void
QvisViewWindow::UpdateGlobal(bool doAll)
{
    if(windowInfo == 0)
        return;

    for(int i = 0; i < windowInfo->NumAttributes(); ++i)
    {
        if(!windowInfo->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case 10: // lockView
            lockedViewToggle->blockSignals(true);
            lockedViewToggle->setChecked(windowInfo->GetLockView());
            lockedViewToggle->blockSignals(false);
            break;
        case 13: // viewExtentsType
            extentComboBox->blockSignals(true);
            extentComboBox->setCurrentItem(windowInfo->GetViewExtentsType());
            extentComboBox->blockSignals(false);
            break;
        case 16: // cameraViewMode
            copyViewFromCameraToggle->blockSignals(true);
            copyViewFromCameraToggle->setChecked(
                                              windowInfo->GetCameraViewMode());
            copyViewFromCameraToggle->blockSignals(false);
            break;
        case 24: // winMode
            if (!activeTabSetBySlot)
            {
                tabs->blockSignals(true);
                switch(windowInfo->GetWinMode())
                {
                    case 0: activeTab = 1; tabs->showPage(page2D); break;
                    case 1: activeTab = 2; tabs->showPage(page3D); break;
                    case 2: activeTab = 0; tabs->showPage(pageCurve); break;
                    default: break;
                }
                tabs->blockSignals(false);
            }
            break;
        default: break;
        }
    }
}

// ****************************************************************************
//  Method: QvisViewWindow::UpdateEyeAngleSliderFromAtts
//
//  Purpose:
//      Puts the slider at the correct position based on the current eye angle.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
QvisViewWindow::UpdateEyeAngleSliderFromAtts(void)
{
    float eyeAngle = view3d->GetEyeAngle();
    int val = 0;
    if (eyeAngle <= 0.5)
        val = 0;
    else if (eyeAngle >= 5.0)
        val = 80;
    else if (eyeAngle == 2.0)
        val = 40;
    else
    {
        //
        // The relation between the slider and the eye angle is a parabola.
        // Rather than solving the quadratic formula, just iterate over the
        // the 80 possible values.
        //
        // We want 2.0 degrees to be the middle (40), 0 to be 0.5 degrees,
        // and 80 to be 5 degrees.  So a parabola can fit this curve --
        // y = (ax-b)^2 + c.  (y = degrees)
        float a = 0.0217;
        float b = -0.433;
        float c = 0.3125;
        for (int i = 0 ; i < 80 ; i++)
        {
            float tmp = a*i-b;
            tmp *= tmp;
            tmp += c;
            if (eyeAngle < tmp)
            {
                val = i;
                break;
            }
        }
    }
        
    eyeAngleSlider->blockSignals(true);
    eyeAngleSlider->setValue(val);
    eyeAngleSlider->blockSignals(false);
}

// ****************************************************************************
// Method: QvisViewWindow::Apply
//
// Purpose: 
//   Applies the new view.
//
// Arguments:
//   ignore : If true then send to the viewer unconditionally.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 4 14:44:18 PST 2002
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//   
// ****************************************************************************

void
QvisViewWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        bool doCurve = (viewCurve->NumAttributesSelected() > 0);
        bool do2d    = (view2d->NumAttributesSelected() > 0);
        bool do3d    = (view3d->NumAttributesSelected() > 0);

        // Get the current view attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);

        // Tell the viewer to set the curve view attributes.
        if(doCurve || ignore)
        {
            viewCurve->Notify();
            GetViewerMethods()->SetViewCurve();
        }
        // Tell the viewer to set the 2D view attributes.
        if(do2d || ignore)
        {
            view2d->Notify();
            GetViewerMethods()->SetView2D();
        }
        // Tell the viewer to set the 3D view attributes.
        if(do3d || ignore)
        {
            view3d->Notify();
            GetViewerMethods()->SetView3D();
        }
    }
    else
    {
        // Send the new state to the viewer.
        viewCurve->Notify();
        view2d->Notify();
        view3d->Notify();
    }
}

// ****************************************************************************
// Method: QvisViewWindow::CreateNode
//
// Purpose: 
//   Writes the window's extra information to the config file.
//
// Arguments:
//   parentNode : The node to which the window's attributes are added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 18 10:40:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(std::string(caption().latin1()));

        // Save the current tab.
        node->AddNode(new DataNode("activeTab", activeTab));
    }
}

// ****************************************************************************
// Method: QvisViewWindow::SetFromNode
//
// Purpose: 
//   Reads window attributes from the DataNode representation of the config
//   file.
//
// Arguments:
//   parentNode : The data node that contains the window's attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 18 10:40:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node;
    if((node = winNode->GetNode("activeTab")) != 0)
    {
        activeTab = node->AsInt();
        if(activeTab != 0 && activeTab != 1 && activeTab != 2)
            activeTab = 0;
    }

    // Call the base class's function.
    QvisPostableWindowSimpleObserver::SetFromNode(parentNode, borders);
}

// ****************************************************************************
// Method: QvisViewWindow::GetCurrentValuesCurve
//
// Purpose: 
//   Get the current values for the curve text fields.
//
// Programmer: Eric Brugger
// Creation:   Wed Aug 20 14:04:21 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValuesCurve(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the viewport values.
    if(which_widget == 0 || doAll)
    {
        temp = viewportCurveLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double v[4];
            if(sscanf(temp.latin1(), "%lg %lg %lg %lg",
                      &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                viewCurve->SetViewportCoords(v);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The viewport values were invalid. "
                "Resetting to the last good values of %g %g %g %g.",
                 viewCurve->GetViewportCoords()[0],
                 viewCurve->GetViewportCoords()[1],
                 viewCurve->GetViewportCoords()[2],
                 viewCurve->GetViewportCoords()[3]);
            Error(msg);
            viewCurve->SetViewportCoords(viewCurve->GetViewportCoords());
        }
    }

    // Do the domain values.
    if(which_widget == 1 || doAll)
    {
        temp = domainLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double domain[2];
            if(sscanf(temp.latin1(), "%lg %lg",
                      &domain[0], &domain[1]) == 2)
            {
                viewCurve->SetDomainCoords(domain);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The domain values were invalid. "
                "Resetting to the last good values of %g %g.",
                 viewCurve->GetDomainCoords()[0],
                 viewCurve->GetDomainCoords()[1]);
            Error(msg);
            viewCurve->SetDomainCoords(viewCurve->GetDomainCoords());
        }
    }

    // Do the range values.
    if(which_widget == 2 || doAll)
    {
        temp = rangeLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double range[2];
            if(sscanf(temp.latin1(), "%lg %lg",
                      &range[0], &range[1]) == 2)
            {
                viewCurve->SetRangeCoords(range);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The range values were invalid. "
                "Resetting to the last good values of %g %g.",
                 viewCurve->GetRangeCoords()[0],
                 viewCurve->GetRangeCoords()[1]);
            Error(msg);
            viewCurve->SetRangeCoords(viewCurve->GetRangeCoords());
        }
    }
    // Do the domainScale value.
    if(which_widget == 3 || doAll)
    {
    }
}

// ****************************************************************************
// Method: QvisViewWindow::GetCurrentValues2d
//
// Purpose: 
//   Get the current values for the 2d text fields.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:48:27 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Fri Feb 15 11:47:34 PDT 2002
//   Fixed format strings.
//
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I corrected an error message.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValues2d(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the viewport values.
    if(which_widget == 0 || doAll)
    {
        temp = viewportLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double v[4];
            if(sscanf(temp.latin1(), "%lg %lg %lg %lg",
                      &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                view2d->SetViewportCoords(v);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The viewport values were invalid. "
                "Resetting to the last good values of %g %g %g %g.",
                 view2d->GetViewportCoords()[0],
                 view2d->GetViewportCoords()[1],
                 view2d->GetViewportCoords()[2],
                 view2d->GetViewportCoords()[3]);
            Error(msg);
            view2d->SetViewportCoords(view2d->GetViewportCoords());
        }
    }

    // Do the window values.
    if(which_widget == 1 || doAll)
    {
        temp = windowLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double win[4];
            if(sscanf(temp.latin1(), "%lg %lg %lg %lg",
                      &win[0], &win[1], &win[2], &win[3]) == 4)
            {
                view2d->SetWindowCoords(win);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The window values were invalid. "
                "Resetting to the last good values of %g %g %g %g.",
                 view2d->GetWindowCoords()[0],
                 view2d->GetWindowCoords()[1],
                 view2d->GetWindowCoords()[2],
                 view2d->GetWindowCoords()[3]);
            Error(msg);
            view2d->SetWindowCoords(view2d->GetWindowCoords());
        }
    }
}

// ****************************************************************************
// Method: QvisViewWindow::GetCurrentValues3d
//
// Purpose: 
//   Get the current values for the 3d text fields.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:01:27 PDT 2001
//
// Modifications:
//   Eric Brugger, Tue Aug 21 13:48:27 PDT 2001
//   I redesigned the window fairly extensively.
//
//   Brad Whitlock, Fri Feb 15 11:46:01 PDT 2002
//   Fixed format strings.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I added controls for image pan and image zoom. I renamed camera
//   to view normal in the view attributes.
//
//   Hank Childs, Wed Oct 15 15:26:03 PDT 2003
//   Added eye angle.
//
//   Eric Brugger, Tue Feb 10 10:30:15 PST 2004
//   I added center of rotation controls to the advanced tab.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValues3d(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the normal values.
    if(which_widget == 0 || doAll)
    {
        temp = normalLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double normal[3];
            if(sscanf(temp.latin1(), "%lg %lg %lg",
                      &normal[0], &normal[1], &normal[2]) == 3)
            {
                view3d->SetViewNormal(normal);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The normal location values were invalid. "
                "Resetting to the last good values of %g %g %g.",
                 view3d->GetViewNormal()[0],
                 view3d->GetViewNormal()[1],
                 view3d->GetViewNormal()[2]);
            Error(msg);
            view3d->SetViewNormal(view3d->GetViewNormal());
        }
    }

    // Do the focus values.
    if(which_widget == 1 || doAll)
    {
        temp = focusLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double focus[3];
            if(sscanf(temp.latin1(), "%lg %lg %lg",
                      &focus[0], &focus[1], &focus[2]) == 3)
            {
                view3d->SetFocus(focus);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The focus location values were invalid. "
                "Resetting to the last good values of %g %g %g.",
                 view3d->GetFocus()[0],
                 view3d->GetFocus()[1],
                 view3d->GetFocus()[2]);
            Error(msg);
            view3d->SetFocus(view3d->GetFocus());
        }
    }

    // Do the up vector values.
    if(which_widget == 2 || doAll)
    {
        temp = upvectorLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double upvector[3];
            if(sscanf(temp.latin1(), "%lg %lg %lg",
                      &upvector[0], &upvector[1], &upvector[2]) == 3)
            {
                view3d->SetViewUp(upvector);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The up vector values were invalid. "
                "Resetting to the last good values of %g %g %g.",
                 view3d->GetViewUp()[0],
                 view3d->GetViewUp()[1],
                 view3d->GetViewUp()[2]);
            Error(msg);
            view3d->SetViewUp(view3d->GetViewUp());
        }
    }

    // Do the view angle value.
    if(which_widget == 3 || doAll)
    {
        temp = viewAngleLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double viewAngle;
            if(sscanf(temp.latin1(), "%lg", &viewAngle) == 1)
            {
                if(viewAngle > 0.)
                    view3d->SetViewAngle(viewAngle);
                else
                    okay = false;
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The viewangle was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetViewAngle());
            Error(msg);
            view3d->SetViewAngle(view3d->GetViewAngle());
        }
    }

    // Do the parallel scale value.
    if(which_widget == 5 || doAll)
    {
        temp = parallelScaleLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double pscale;
            if(sscanf(temp.latin1(), "%lg", &pscale) == 1)
            {
                view3d->SetParallelScale(pscale);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The parallel scale was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetParallelScale());
            Error(msg);
            view3d->SetParallelScale(view3d->GetParallelScale());
        }
    }

    // Do the near value.
    if(which_widget == 6 || doAll)
    {
        temp = nearLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double nearPlane;
            if(sscanf(temp.latin1(), "%lg", &nearPlane) == 1)
            {
                view3d->SetNearPlane(nearPlane);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The near clipping value was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetNearPlane());
            Error(msg);
            view3d->SetNearPlane(view3d->GetNearPlane());
        }
    }

    // Do the far value.
    if(which_widget == 7 || doAll)
    {
        temp = farLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double farPlane;
            if(sscanf(temp.latin1(), "%lg", &farPlane) == 1)
            {
                view3d->SetFarPlane(farPlane);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The far clipping value was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetFarPlane());
            Error(msg);
            view3d->SetFarPlane(view3d->GetFarPlane());
        }
    }

    // Do the image pan value.
    if(which_widget == 8 || doAll)
    {
        temp = imagePanLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double imagePan[2];
            if(sscanf(temp.latin1(), "%lg %lg",
                      &imagePan[0], &imagePan[1]) == 2)
            {
                view3d->SetImagePan(imagePan);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The image pan values were invalid. "
                "Resetting to the last good values of %g %g.",
                 view3d->GetImagePan()[0],
                 view3d->GetImagePan()[1]);
            Error(msg);
            view3d->SetImagePan(view3d->GetImagePan());
        }
    }

    // Do the image zoom value.
    if(which_widget == 9 || doAll)
    {
        temp = imageZoomLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double imageZoom;
            if(sscanf(temp.latin1(), "%lg", &imageZoom) == 1)
            {
                view3d->SetImageZoom(imageZoom);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The image zoom value was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetImageZoom());
            Error(msg);
            view3d->SetImageZoom(view3d->GetImageZoom());
        }
    }

    // Do the eye angle value.
    if(which_widget == 13 || doAll)
    {
        temp = eyeAngleLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double eyeAngle;
            if(sscanf(temp.latin1(), "%lg", &eyeAngle) == 1)
            {
                view3d->SetEyeAngle(eyeAngle);
                UpdateEyeAngleSliderFromAtts();
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The eye angle value was invalid. "
                "Resetting to the last good value of %g.",
                 view3d->GetEyeAngle());
            Error(msg);
            view3d->SetEyeAngle(view3d->GetEyeAngle());
        }
    }

    // Do the center of rotation values.
    if(which_widget == 14 || doAll)
    {
        temp = centerLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double center[3];
            if(sscanf(temp.latin1(), "%lg %lg %lg",
                      &center[0], &center[1], &center[2]) == 3)
            {
                view3d->SetCenterOfRotation(center);
            }
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The center of rotation values were invalid. "
                "Resetting to the last good values of %g %g %g.",
                 view3d->GetCenterOfRotation()[0],
                 view3d->GetCenterOfRotation()[1],
                 view3d->GetCenterOfRotation()[2]);
            Error(msg);
            view3d->SetCenterOfRotation(view3d->GetCenterOfRotation());
        }
    }
}

void
QvisViewWindow::GetCurrentValues(int which_widget)
{
    GetCurrentValuesCurve(which_widget);
    GetCurrentValues2d(which_widget);
    GetCurrentValues3d(which_widget);
}

// ****************************************************************************
// Method: QvisViewWindow::ParseViewCommands
//
// Purpose:
//   This method parses a semi-colon seperated list of view commands.
//
// Arguments:
//   str     : The string containing the commands.
//
// Programmer: Eric Brugger
// Creation:   August 6, 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 17 13:41:20 PST 2002
//   I made it understand "reset" and "recenter" and "undo".
//
//   Eric Brugger, Fri Oct 25 16:36:04 PDT 2002
//   I added the commands "pan3", "panx", "pany", "rx", "ry", "rx",
//   and "zoom3".
//
//   Eric Brugger, Mon Jan 13 15:06:42 PST 2003
//   I added the commands "vp" and "wp".
//
//   Eric Brugger, Wed Dec 24 10:20:47 PST 2003
//   I added the commands "xtrans", "ytrans" and "zf".
//
// ****************************************************************************
 
void
QvisViewWindow::ParseViewCommands(const char *str)
{
    bool doApply = true;
    char *strCopy;
    char *command;

    strCopy = new char[strlen(str)+1];
    strcpy(strCopy, str);

    //
    // Loop over the commands, parsing one at a time. 
    //
    command = strtok(strCopy, ";");
    while (command != NULL)
    {
        bool okay = true;

        if (strncmp(command, "pan ", 4) == 0)
        {
            double panx, pany;

            if (sscanf(&command[4], "%lg %lg", &panx, &pany) == 2)
            {
                Pan(panx, pany);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "pan3 ", 5) == 0)
        {
            double panx, pany;

            if (sscanf(&command[5], "%lg %lg", &panx, &pany) == 2)
            {
                Pan(panx, pany);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "panx ", 5) == 0)
        {
            double panx;

            if (sscanf(&command[5], "%lg", &panx) == 1)
            {
                Pan(panx, 0.);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "xtrans ", 7) == 0)
        {
            double panx;

            if (sscanf(&command[7], "%lg", &panx) == 1)
            {
                Pan(panx, 0.);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "pany ", 5) == 0)
        {
            double pany;

            if (sscanf(&command[5], "%lg", &pany) == 1)
            {
                Pan(0., pany);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "ytrans ", 7) == 0)
        {
            double pany;

            if (sscanf(&command[7], "%lg", &pany) == 1)
            {
                Pan(0., pany);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "rotx ", 5) == 0)
        {
            double angle;
 
            if (sscanf(&command[5], "%lg", &angle) == 1)
            {
                RotateAxis(0, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "rx ", 3) == 0)
        {
            double angle;
 
            if (sscanf(&command[3], "%lg", &angle) == 1)
            {
                RotateAxis(0, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "roty ", 5) == 0)
        {
            double angle;
 
            if (sscanf(&command[5], "%lg", &angle) == 1)
            {
                RotateAxis(1, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "ry ", 3) == 0)
        {
            double angle;
 
            if (sscanf(&command[3], "%lg", &angle) == 1)
            {
                RotateAxis(1, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "rotz ", 5) == 0)
        {
            double angle;
 
            if (sscanf(&command[5], "%lg", &angle) == 1)
            {
                RotateAxis(2, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "rz ", 3) == 0)
        {
            double angle;
 
            if (sscanf(&command[3], "%lg", &angle) == 1)
            {
                RotateAxis(2, angle);
                doApply = true;
            }
            else
                okay = false;
        }
        else if (strncmp(command, "zoom ", 5) == 0)
        {
            double zoom;

            if (sscanf(&command[5], "%lg", &zoom) == 1)
            {
                Zoom(zoom);
                doApply = true;
            }
            else
                okay = false;
        }
        else if (strncmp(command, "zoom3 ", 6) == 0)
        {
            double zoom;

            if (sscanf(&command[6], "%lg", &zoom) == 1)
            {
                Zoom(zoom);
                doApply = true;
            }
            else
                okay = false;
        }
        else if (strncmp(command, "zf ", 3) == 0)
        {
            double zoom;

            if (sscanf(&command[3], "%lg", &zoom) == 1)
            {
                Zoom(zoom);
                doApply = true;
            }
            else
                okay = false;
        }
        else if (strncmp(command, "vp ", 3) == 0)
        {
            double viewport[4];

            if (sscanf(&command[3], "%lg %lg %lg %lg", &viewport[0],
                       &viewport[1], &viewport[2], &viewport[3]) == 4)
            {
                Viewport(viewport);
                doApply = true;
            }
            else
                okay = false;
        }
        else if (strncmp(command, "wp ", 3) == 0)
        {
            double window[4];

            if (sscanf(&command[3], "%lg %lg %lg %lg", &window[0],
                       &window[1], &window[2], &window[3]) == 4)
            {
                Window(window);
                doApply = true;
            }
            else
                okay = false;
        }
        else if(strncmp(command, "reset", 5) == 0)
        {
            GetViewerMethods()->ResetView();
            doApply = false;
        }
        else if(strncmp(command, "recenter", 7) == 0)
        {
            GetViewerMethods()->RecenterView();
            doApply = false;
        }
        else if(strncmp(command, "undo", 4) == 0)
        {
            GetViewerMethods()->UndoView();
            doApply = false;
        }
        else
            okay = false;
 
        if(!okay)
        {
            QString msg;

            msg.sprintf("Bad command >> %s <<", command);
            Error(msg);
        }

        command = strtok(NULL, ";"); 
    }

    delete [] strCopy;

    //
    // Only update the viewer after processing all the commands.
    //
    if(doApply)
        Apply(true);
}

// ****************************************************************************
// Method: QvisViewWindow::Pan
//
// Purpose:
//   This method pans the 3d view.
//
// Arguments:
//   panx    : The fraction of the screen to pan in the x direction.
//   pany    : The fraction of the screen to pan in the y direction.
//
// Programmer: Eric Brugger
// Creation:   August 6, 2002
//
// Modifications:
//   Eric Brugger, Thu Jun 12 09:59:42 PDT 2003  
//   Modify the command to change the image pan instead of the focus.
//
//   Eric Brugger, Tue Dec 23 07:58:34 PST 2003
//   Modify the command to divide the pan amounts by the zoom factor so that
//   they are a fraction of the current image width and height.
//
// ****************************************************************************
 
void
QvisViewWindow::Pan(double panx, double pany)
{
    double imagePan[2];
 
    imagePan[0] = view3d->GetImagePan()[0] + panx / view3d->GetImageZoom();
    imagePan[1] = view3d->GetImagePan()[1] + pany / view3d->GetImageZoom();

    view3d->SetImagePan(imagePan);
 
    Update3D(true);
}
 
// ****************************************************************************
// Method: QvisViewWindow::RotateAxis
//
// Purpose:
//   This method rotates the 3d view about the specified axis.
//
// Arguments:
//   axis    : The axis about which to rotate (0 = x, 1 = y, 2 = z).
//   angle   : The number of degrees to rotate the image.
//
// Programmer: Eric Brugger
// Creation:   August 6, 2002
//
// Modifications:
//   Eric Brugger, Wed May 21 12:52:06 PDT 2003
//   Modify the routine to perform the rotations in screen space instead
//   of object space.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I added controls for image pan and image zoom. I renamed camera
//   to view normal in the view attributes.
//
//   Eric Brugger, Tue Feb 10 10:30:15 PST 2004
//   I modified the routine to rotate about the center of rotation if one
//   is specified.
//
// ****************************************************************************
 
void
QvisViewWindow::RotateAxis(int axis, double angle)
{
    double angleRadians;
    double v1[3], v2[3], v3[3];
    double t1[16], t2[16], m1[16], m2[16], r[16];
    double ma[16], mb[16], mc[16];
    double rM[16];
    double viewNormal[3];
    double viewUp[3];
    double viewFocus[3];
    double dist;
 
    //
    // Calculate the rotation matrix in screen coordinates.
    //
    angleRadians = angle * (3.141592653589793 / 180.);
    switch (axis)
    {
      case 0:
        r[0]  = 1.;
        r[1]  = 0.;
        r[2]  = 0.;
        r[3]  = 0.;
        r[4]  = 0.;
        r[5]  = cos(angleRadians);
        r[6]  = - sin(angleRadians);
        r[7]  = 0.;
        r[8]  = 0.;
        r[9]  = sin(angleRadians);
        r[10] = cos(angleRadians);
        r[11] = 0.;
        r[12] = 0.;
        r[13] = 0.;
        r[14] = 0.;
        r[15] = 1.;
        break;

      case 1:
        r[0]  = cos(angleRadians);
        r[1]  = 0.;
        r[2]  = sin(angleRadians);
        r[3]  = 0.;
        r[4]  = 0.;
        r[5]  = 1.;
        r[6]  = 0.;
        r[7]  = 0.;
        r[8]  = - sin(angleRadians);
        r[9]  = 0.;
        r[10]  = cos(angleRadians);
        r[11] = 0.;
        r[12] = 0.;
        r[13] = 0.;
        r[14] = 0.;
        r[15] = 1.;
        break;
 
      case 2:
        r[0]  = cos(angleRadians);
        r[1]  = - sin(angleRadians);
        r[2]  = 0.;
        r[3]  = 0.;
        r[4]  = sin(angleRadians);
        r[5]  = cos(angleRadians);
        r[6]  = 0.;
        r[7]  = 0.;
        r[8]  = 0.;
        r[9]  = 0.;
        r[10]  = 1.;
        r[11] = 0.;
        r[12] = 0.;
        r[13] = 0.;
        r[14] = 0.;
        r[15] = 1.;
        break;
    }
 
    //
    // Calculate the matrix to rotate from object coordinates to screen
    // coordinates and its inverse.
    //
    v1[0] = view3d->GetViewNormal()[0];
    v1[1] = view3d->GetViewNormal()[1];
    v1[2] = view3d->GetViewNormal()[2];
 
    v2[0] = view3d->GetViewUp()[0];
    v2[1] = view3d->GetViewUp()[1];
    v2[2] = view3d->GetViewUp()[2];
 
    v3[0] =   v2[1]*v1[2] - v2[2]*v1[1];
    v3[1] = - v2[0]*v1[2] + v2[2]*v1[0];
    v3[2] =   v2[0]*v1[1] - v2[1]*v1[0];

    m1[0]  = v3[0];
    m1[1]  = v2[0];
    m1[2]  = v1[0];
    m1[3]  = 0.;
    m1[4]  = v3[1];
    m1[5]  = v2[1];
    m1[6]  = v1[1];
    m1[7]  = 0.;
    m1[8]  = v3[2];
    m1[9]  = v2[2];
    m1[10] = v1[2];
    m1[11] = 0.;
    m1[12] = 0.;
    m1[13] = 0.;
    m1[14] = 0.;
    m1[15] = 1.;

    m2[0]  = m1[0];
    m2[1]  = m1[4];
    m2[2]  = m1[8];
    m2[3]  = m1[12];
    m2[4]  = m1[1];
    m2[5]  = m1[5];
    m2[6]  = m1[9];
    m2[7]  = m1[13];
    m2[8]  = m1[2];
    m2[9]  = m1[6];
    m2[10] = m1[10];
    m2[11] = m1[14];
    m2[12] = m1[3];
    m2[13] = m1[7];
    m2[14] = m1[11];
    m2[15] = m1[15];

    //
    // Calculate the translation to the center of rotation (and its
    // inverse).
    //
    t1[0]  = 1.;
    t1[1]  = 0.;
    t1[2]  = 0.;
    t1[3]  = 0.;
    t1[4]  = 0.;
    t1[5]  = 1.;
    t1[6]  = 0.;
    t1[7]  = 0.;
    t1[8]  = 0.;
    t1[9]  = 0.;
    t1[10] = 1.;
    t1[11] = 0.;
    t1[12] = -view3d->GetCenterOfRotation()[0];
    t1[13] = -view3d->GetCenterOfRotation()[1];
    t1[14] = -view3d->GetCenterOfRotation()[2];
    t1[15] = 1.;

    t2[0]  = 1.;
    t2[1]  = 0.;
    t2[2]  = 0.;
    t2[3]  = 0.;
    t2[4]  = 0.;
    t2[5]  = 1.;
    t2[6]  = 0.;
    t2[7]  = 0.;
    t2[8]  = 0.;
    t2[9]  = 0.;
    t2[10] = 1.;
    t2[11] = 0.;
    t2[12] = view3d->GetCenterOfRotation()[0];
    t2[13] = view3d->GetCenterOfRotation()[1];
    t2[14] = view3d->GetCenterOfRotation()[2];
    t2[15] = 1.;

    //
    // Form the composite transformation matrix t1 X m1 X r X m2 X t2.
    //
    ma[0]  = t1[0]*m1[0]  + t1[1]*m1[4]  + t1[2]*m1[8]   + t1[3]*m1[12];
    ma[1]  = t1[0]*m1[1]  + t1[1]*m1[5]  + t1[2]*m1[9]   + t1[3]*m1[13];
    ma[2]  = t1[0]*m1[2]  + t1[1]*m1[6]  + t1[2]*m1[10]  + t1[3]*m1[14];
    ma[3]  = t1[0]*m1[3]  + t1[1]*m1[7]  + t1[2]*m1[11]  + t1[3]*m1[15];
    ma[4]  = t1[4]*m1[0]  + t1[5]*m1[4]  + t1[6]*m1[8]   + t1[7]*m1[12];
    ma[5]  = t1[4]*m1[1]  + t1[5]*m1[5]  + t1[6]*m1[9]   + t1[7]*m1[13];
    ma[6]  = t1[4]*m1[2]  + t1[5]*m1[6]  + t1[6]*m1[10]  + t1[7]*m1[14];
    ma[7]  = t1[4]*m1[3]  + t1[5]*m1[7]  + t1[6]*m1[11]  + t1[7]*m1[15];
    ma[8]  = t1[8]*m1[0]  + t1[9]*m1[4]  + t1[10]*m1[8]  + t1[11]*m1[12];
    ma[9]  = t1[8]*m1[1]  + t1[9]*m1[5]  + t1[10]*m1[9]  + t1[11]*m1[13];
    ma[10] = t1[8]*m1[2]  + t1[9]*m1[6]  + t1[10]*m1[10] + t1[11]*m1[14];
    ma[11] = t1[8]*m1[3]  + t1[9]*m1[7]  + t1[10]*m1[11] + t1[11]*m1[15];
    ma[12] = t1[12]*m1[0] + t1[13]*m1[4] + t1[14]*m1[8]  + t1[15]*m1[12];
    ma[13] = t1[12]*m1[1] + t1[13]*m1[5] + t1[14]*m1[9]  + t1[15]*m1[13];
    ma[14] = t1[12]*m1[2] + t1[13]*m1[6] + t1[14]*m1[10] + t1[15]*m1[14];
    ma[15] = t1[12]*m1[3] + t1[13]*m1[7] + t1[14]*m1[11] + t1[15]*m1[15];

    mb[0]  = ma[0]*r[0]  + ma[1]*r[4]  + ma[2]*r[8]   + ma[3]*r[12];
    mb[1]  = ma[0]*r[1]  + ma[1]*r[5]  + ma[2]*r[9]   + ma[3]*r[13];
    mb[2]  = ma[0]*r[2]  + ma[1]*r[6]  + ma[2]*r[10]  + ma[3]*r[14];
    mb[3]  = ma[0]*r[3]  + ma[1]*r[7]  + ma[2]*r[11]  + ma[3]*r[15];
    mb[4]  = ma[4]*r[0]  + ma[5]*r[4]  + ma[6]*r[8]   + ma[7]*r[12];
    mb[5]  = ma[4]*r[1]  + ma[5]*r[5]  + ma[6]*r[9]   + ma[7]*r[13];
    mb[6]  = ma[4]*r[2]  + ma[5]*r[6]  + ma[6]*r[10]  + ma[7]*r[14];
    mb[7]  = ma[4]*r[3]  + ma[5]*r[7]  + ma[6]*r[11]  + ma[7]*r[15];
    mb[8]  = ma[8]*r[0]  + ma[9]*r[4]  + ma[10]*r[8]  + ma[11]*r[12];
    mb[9]  = ma[8]*r[1]  + ma[9]*r[5]  + ma[10]*r[9]  + ma[11]*r[13];
    mb[10] = ma[8]*r[2]  + ma[9]*r[6]  + ma[10]*r[10] + ma[11]*r[14];
    mb[11] = ma[8]*r[3]  + ma[9]*r[7]  + ma[10]*r[11] + ma[11]*r[15];
    mb[12] = ma[12]*r[0] + ma[13]*r[4] + ma[14]*r[8]  + ma[15]*r[12];
    mb[13] = ma[12]*r[1] + ma[13]*r[5] + ma[14]*r[9]  + ma[15]*r[13];
    mb[14] = ma[12]*r[2] + ma[13]*r[6] + ma[14]*r[10] + ma[15]*r[14];
    mb[15] = ma[12]*r[3] + ma[13]*r[7] + ma[14]*r[11] + ma[15]*r[15];
 
    mc[0]  = mb[0]*m2[0]  + mb[1]*m2[4]  + mb[2]*m2[8]   + mb[3]*m2[12];
    mc[1]  = mb[0]*m2[1]  + mb[1]*m2[5]  + mb[2]*m2[9]   + mb[3]*m2[13];
    mc[2]  = mb[0]*m2[2]  + mb[1]*m2[6]  + mb[2]*m2[10]  + mb[3]*m2[14];
    mc[3]  = mb[0]*m2[3]  + mb[1]*m2[7]  + mb[2]*m2[11]  + mb[3]*m2[15];
    mc[4]  = mb[4]*m2[0]  + mb[5]*m2[4]  + mb[6]*m2[8]   + mb[7]*m2[12];
    mc[5]  = mb[4]*m2[1]  + mb[5]*m2[5]  + mb[6]*m2[9]   + mb[7]*m2[13];
    mc[6]  = mb[4]*m2[2]  + mb[5]*m2[6]  + mb[6]*m2[10]  + mb[7]*m2[14];
    mc[7]  = mb[4]*m2[3]  + mb[5]*m2[7]  + mb[6]*m2[11]  + mb[7]*m2[15];
    mc[8]  = mb[8]*m2[0]  + mb[9]*m2[4]  + mb[10]*m2[8]  + mb[11]*m2[12];
    mc[9]  = mb[8]*m2[1]  + mb[9]*m2[5]  + mb[10]*m2[9]  + mb[11]*m2[13];
    mc[10] = mb[8]*m2[2]  + mb[9]*m2[6]  + mb[10]*m2[10] + mb[11]*m2[14];
    mc[11] = mb[8]*m2[3]  + mb[9]*m2[7]  + mb[10]*m2[11] + mb[11]*m2[15];
    mc[12] = mb[12]*m2[0] + mb[13]*m2[4] + mb[14]*m2[8]  + mb[15]*m2[12];
    mc[13] = mb[12]*m2[1] + mb[13]*m2[5] + mb[14]*m2[9]  + mb[15]*m2[13];
    mc[14] = mb[12]*m2[2] + mb[13]*m2[6] + mb[14]*m2[10] + mb[15]*m2[14];
    mc[15] = mb[12]*m2[3] + mb[13]*m2[7] + mb[14]*m2[11] + mb[15]*m2[15];
 
    rM[0]  = mc[0]*t2[0]  + mc[1]*t2[4]  + mc[2]*t2[8]   + mc[3]*t2[12];
    rM[1]  = mc[0]*t2[1]  + mc[1]*t2[5]  + mc[2]*t2[9]   + mc[3]*t2[13];
    rM[2]  = mc[0]*t2[2]  + mc[1]*t2[6]  + mc[2]*t2[10]  + mc[3]*t2[14];
    rM[3]  = mc[0]*t2[3]  + mc[1]*t2[7]  + mc[2]*t2[11]  + mc[3]*t2[15];
    rM[4]  = mc[4]*t2[0]  + mc[5]*t2[4]  + mc[6]*t2[8]   + mc[7]*t2[12];
    rM[5]  = mc[4]*t2[1]  + mc[5]*t2[5]  + mc[6]*t2[9]   + mc[7]*t2[13];
    rM[6]  = mc[4]*t2[2]  + mc[5]*t2[6]  + mc[6]*t2[10]  + mc[7]*t2[14];
    rM[7]  = mc[4]*t2[3]  + mc[5]*t2[7]  + mc[6]*t2[11]  + mc[7]*t2[15];
    rM[8]  = mc[8]*t2[0]  + mc[9]*t2[4]  + mc[10]*t2[8]  + mc[11]*t2[12];
    rM[9]  = mc[8]*t2[1]  + mc[9]*t2[5]  + mc[10]*t2[9]  + mc[11]*t2[13];
    rM[10] = mc[8]*t2[2]  + mc[9]*t2[6]  + mc[10]*t2[10] + mc[11]*t2[14];
    rM[11] = mc[8]*t2[3]  + mc[9]*t2[7]  + mc[10]*t2[11] + mc[11]*t2[15];
    rM[12] = mc[12]*t2[0] + mc[13]*t2[4] + mc[14]*t2[8]  + mc[15]*t2[12];
    rM[13] = mc[12]*t2[1] + mc[13]*t2[5] + mc[14]*t2[9]  + mc[15]*t2[13];
    rM[14] = mc[12]*t2[2] + mc[13]*t2[6] + mc[14]*t2[10] + mc[15]*t2[14];
    rM[15] = mc[12]*t2[3] + mc[13]*t2[7] + mc[14]*t2[11] + mc[15]*t2[15];

    //
    // Calculate the new view normal and view up.
    //
    viewNormal[0] = view3d->GetViewNormal()[0] * rM[0] +
                    view3d->GetViewNormal()[1] * rM[4] +
                    view3d->GetViewNormal()[2] * rM[8];
    viewNormal[1] = view3d->GetViewNormal()[0] * rM[1] +
                    view3d->GetViewNormal()[1] * rM[5] +
                    view3d->GetViewNormal()[2] * rM[9];
    viewNormal[2] = view3d->GetViewNormal()[0] * rM[2] +
                    view3d->GetViewNormal()[1] * rM[6] +
                    view3d->GetViewNormal()[2] * rM[10];
    dist = sqrt(viewNormal[0]*viewNormal[0] + viewNormal[1]*viewNormal[1] +
                viewNormal[2]*viewNormal[2]);
    viewNormal[0] /= dist;
    viewNormal[1] /= dist;
    viewNormal[2] /= dist;
 
    view3d->SetViewNormal(viewNormal);
 
    viewUp[0] = view3d->GetViewUp()[0] * rM[0] +
                view3d->GetViewUp()[1] * rM[4] +
                view3d->GetViewUp()[2] * rM[8];
    viewUp[1] = view3d->GetViewUp()[0] * rM[1] +
                view3d->GetViewUp()[1] * rM[5] +
                view3d->GetViewUp()[2] * rM[9];
    viewUp[2] = view3d->GetViewUp()[0] * rM[2] +
                view3d->GetViewUp()[1] * rM[6] +
                view3d->GetViewUp()[2] * rM[10];
    dist = sqrt(viewUp[0]*viewUp[0] + viewUp[1]*viewUp[1] +
                viewUp[2]*viewUp[2]);
    viewUp[0] /= dist;
    viewUp[1] /= dist;
    viewUp[2] /= dist;
 
    view3d->SetViewUp(viewUp);
 
    if (view3d->GetCenterOfRotationSet())
    {
        viewFocus[0] = view3d->GetFocus()[0] * rM[0]  +
                       view3d->GetFocus()[1] * rM[4]  +
                       view3d->GetFocus()[2] * rM[8]  +
                       rM[12];
        viewFocus[1] = view3d->GetFocus()[0] * rM[1]  +
                       view3d->GetFocus()[1] * rM[5]  +
                       view3d->GetFocus()[2] * rM[9]  +
                       rM[13];
        viewFocus[2] = view3d->GetFocus()[0] * rM[2]  +
                       view3d->GetFocus()[1] * rM[6]  +
                       view3d->GetFocus()[2] * rM[10] +
                       rM[14];

        view3d->SetFocus(viewFocus);
    }

    Update3D(true);
}
 
// ****************************************************************************
// Method: QvisViewWindow::Zoom
//
// Purpose:
//   This method zooms the 3d view.
//
// Arguments:
//   zoom    : The amount to zoom the view.
//
// Programmer: Eric Brugger
// Creation:   August 6, 2002
//
// Modifications:
//   Eric Brugger, Thu Jun 12 09:59:42 PDT 2003  
//   Modify the command to change the image zoom instead of the parallel
//   scale.
//
// ****************************************************************************

void
QvisViewWindow::Zoom(double zoom)
{
    view3d->SetImageZoom(view3d->GetImageZoom() * zoom);
 
    Update3D(true);
}

// ****************************************************************************
// Method: QvisViewWindow::Viewport
//
// Purpose:
//   This method sets the viewport for the 2d view.
//
// Arguments:
//   viewport : The 2d viewport.
//
// Programmer: Eric Brugger
// Creation:   January 13, 2003
//
// ****************************************************************************

void
QvisViewWindow::Viewport(const double *viewport)
{
    view2d->SetViewportCoords(viewport);
 
    Update2D(true);
}

// ****************************************************************************
// Method: QvisViewWindow::Window
//
// Purpose:
//   This method sets the window for the 2d view.
//
// Arguments:
//   window  : The 2d window.
//
// Programmer: Eric Brugger
// Creation:   January 13, 2003
//
// Modifications:
//
//   Hank Childs, Mon Nov 14 14:05:52 PST 2005
//   Check for window specifications.
//
// ****************************************************************************

void
QvisViewWindow::Window(const double *window)
{
    if (window[0] >= window[1] || window[2] >= window[3])
    {
        Warning("The window should be specified as \"minX, maxX, minY, maxY\"."
                "\nYou have specified the coordinates out of order.\nNote "
                "that minX *must* be less than maxX and minY *must* be less "
                "than maxY");
        return;
    }
    view2d->SetWindowCoords(window);
 
    Update2D(true);
}

//
// Qt Slot functions...
//

void
QvisViewWindow::show()
{
    QvisPostableWindowSimpleObserver::show();

    if (windowInfo != 0)
    {
        switch(windowInfo->GetWinMode())
        {
            case 0: activeTab = 1; break;
            case 1: activeTab = 2; break;
            case 2: activeTab = 0; break;
            default: break;
        }
        activeTabSetBySlot = false;
    }

    tabs->blockSignals(true);
    if(activeTab == 0)
        tabs->showPage(pageCurve);
    else if(activeTab == 1)
        tabs->showPage(page2D);
    else if(activeTab == 2)
        tabs->showPage(page3D);
    else
        tabs->showPage(pageAdvanced);
    tabs->blockSignals(false);
}

void
QvisViewWindow::apply()
{
    Apply(true);
}

void
QvisViewWindow::processCommandText()
{
    QString temp;
 
    temp = commandLineEdit->displayText().stripWhiteSpace();
    if(!temp.isEmpty())
    {
        ParseViewCommands(temp.latin1()); 
    }

    commandLineEdit->setText("");
}

// ****************************************************************************
// Method: QvisViewWindow::tabSelected
//
// Purpose: 
//   This is a Qt slot function that is called when the tabs are changed.
//
// Arguments:
//   index : The new active tab.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 18 10:18:20 PDT 2002
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:04:21 PDT 2003
//   I added support for curve views.
//   
// ****************************************************************************

void
QvisViewWindow::tabSelected(const QString &tabLabel)
{
    if(tabLabel == QString("Curve view"))
        activeTab = 0;
    else if(tabLabel == QString("2D view"))
        activeTab = 1;
    else if(tabLabel == QString("3D view"))
        activeTab = 2;
    else
        activeTab = 3;
    activeTabSetBySlot = true;
}

//
// Slots for curve Widgets.
//

void
QvisViewWindow::processViewportCurveText()
{
    GetCurrentValues(0);
    Apply();    
}

void
QvisViewWindow::processDomainText()
{
    GetCurrentValues(1);
    Apply();    
}

void
QvisViewWindow::processRangeText()
{
    GetCurrentValues(2);
    Apply();    
}

//
// Slots for 2d Widgets.
//

void
QvisViewWindow::processViewportText()
{
    GetCurrentValues(0);
    Apply();    
}

void
QvisViewWindow::processWindowText()
{
    GetCurrentValues(1);
    Apply();    
}

//
// Slots for 3d Widgets.
//

void
QvisViewWindow::processNormalText()
{
    GetCurrentValues(0);
    Apply();    
}

void
QvisViewWindow::processFocusText()
{
    GetCurrentValues(1);
    Apply();    
}

void
QvisViewWindow::processUpVectorText()
{
    GetCurrentValues(2);
    Apply();    
}

void
QvisViewWindow::processViewAngleText()
{
    GetCurrentValues(3);
    Apply();    
}

void
QvisViewWindow::processParallelScaleText()
{
    GetCurrentValues(5);
    Apply();    
}

void
QvisViewWindow::processNearText()
{
    GetCurrentValues(6);
    Apply();    
}

void
QvisViewWindow::processFarText()
{
    GetCurrentValues(7);
    Apply();    
}

void
QvisViewWindow::processImagePanText()
{
    GetCurrentValues(8);
    Apply();    
}

void
QvisViewWindow::processImageZoomText()
{
    GetCurrentValues(9);
    Apply();    
}

void
QvisViewWindow::processEyeAngleText()
{
    GetCurrentValues(13);
    Apply();    
}

//  Modifications:
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
void
QvisViewWindow::eyeAngleSliderChanged(int val)
{
    // We want 2.0 degrees to be the middle (40), 0 to be 0.5 degrees,
    // and 80 to be 5 degrees.  So a parabola can fit this curve --
    // y = (ax-b)^2 + c.  (y = degrees)
    float a = 0.0217;
    float b = -0.433;
    float c = 0.3125;
    float angle = 0.;
    if (val == 80)
        angle = 5.0;
    else if (val == 40)
        angle = 2.0;
    else if (val == 0)
        angle = 0.5;
    else
    {
        angle = (a*val - b)*(a*val - b) + c;
        // Only take most significant two digits.
        int most = (int) (angle*100);
        angle = ((float)most)/100.;
    }
    
    view3d->SetEyeAngle(angle);
 
    QString temp;
    temp.setNum(view3d->GetEyeAngle());
    eyeAngleLineEdit->setText(temp);
    GetCurrentValues(13);
    Apply();
}

void
QvisViewWindow::perspectiveToggled(bool val)
{
    view3d->SetPerspective(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisViewWindow::viewButtonClicked
//
// Purpose: 
//   This is a Qt slot function that is called when one of the default view
//   buttons is clicked.
//
// Arguments:
//   index : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 13:30:53 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Aug 30 16:36:26 PST 2001
//   Removed the code to set the camera focus.
//
//   Brad Whitlock, Mon Mar 4 15:03:52 PST 2002
//   Added some code to update the camera nad viewUp text fields. We normally
//   discourage updating widgets outside the UpdateWindow method but it is
//   necessary in this case.
//
//   Brad Whitlock, Wed Sep 18 11:56:22 PDT 2002
//   Changed it to use a combo box.
//
//   Eric Brugger, Tue Jun 10 12:25:05 PDT 2003
//   I renamed camera to view normal in the view attributes.
//
// ****************************************************************************

void
QvisViewWindow::viewButtonClicked(int index)
{
    // The first option is not valid.
    --index;
    if(index < 0)
        return;
    else
    {
        alignComboBox->blockSignals(true);
        alignComboBox->setCurrentItem(0);
        alignComboBox->blockSignals(false);
    }

    double viewNormal[3];
    double viewUp[3];

    switch(index)
    {
    case 0: // -X
        viewNormal[0] = 1.; viewNormal[1] = 0.; viewNormal[2] = 0.;
        viewUp[0] = 0.; viewUp[1] = 1.; viewUp[2] = 0.;
        break;
    case 1: // +X
        viewNormal[0] = -1.; viewNormal[1] = 0.; viewNormal[2] = 0.;
        viewUp[0] = 0.; viewUp[1] = 1.; viewUp[2] = 0.;
        break;
    case 2: // -Y
        viewNormal[0] = 0.; viewNormal[1] = 1.; viewNormal[2] = 0.;
        viewUp[0] = 0.; viewUp[1] = 0.; viewUp[2] = -1.;
        break;
    case 3: // +Y
        viewNormal[0] = 0.; viewNormal[1] = -1.; viewNormal[2] = 0.;
        viewUp[0] = 0.; viewUp[1] = 0.; viewUp[2] = 1.;
        break;
    case 4: // -Z
        viewNormal[0] = 0.; viewNormal[1] = 0.; viewNormal[2] = 1.;
        viewUp[0] = 0.; viewUp[1] = 1.; viewUp[2] = 0.;
        break;
    case 5: // +Z
        viewNormal[0] = 0.; viewNormal[1] = 0.; viewNormal[2] = -1.;
        viewUp[0] = 0.; viewUp[1] = 1.; viewUp[2] = 0.;
        break;
    }

    // Set the view normal information into the state object and notify.
    view3d->SetViewNormal(viewNormal);
    view3d->SetViewUp(viewUp);

    // Set the viewNormal and viewUp text fields. We have to do this
    // because the apply method calls GetCurrentValues and that will
    // undo the changes that we've just made to the state object.
    QString temp;
    temp.sprintf("%g %g %g",
                 view3d->GetViewNormal()[0],
                 view3d->GetViewNormal()[1],
                 view3d->GetViewNormal()[2]);
    normalLineEdit->setText(temp);
    temp.sprintf("%g %g %g",
                 view3d->GetViewUp()[0],
                 view3d->GetViewUp()[1],
                 view3d->GetViewUp()[2]);
                 upvectorLineEdit->setText(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisViewWindow::lockedViewChecked
//
// Purpose: 
//   This Qt slot function tells the viewer to toggle its window locking flag.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 17 15:40:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::lockedViewChecked(bool)
{
    GetViewerMethods()->ToggleLockViewMode();
}

// ****************************************************************************
// Method: QvisViewWindow::extentTypeChanged
//
// Purpose: 
//   This Qt slot function sets the viewer's view extent type.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 17 15:40:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::extentTypeChanged(int val)
{
    GetViewerMethods()->SetViewExtentsType(val);
}

// ****************************************************************************
// Method: QvisViewWindow::resetView
//
// Purpose: 
//   This Qt slot function tells the viewer to reset the view.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 09:34:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::resetView()
{
    GetViewerMethods()->ResetView();
}

// ****************************************************************************
// Method: QvisViewWindow::recenterView
//
// Purpose: 
//   This Qt slot function tells the viewer to recenter the view.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 09:34:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::recenterView()
{
    GetViewerMethods()->RecenterView();
}

// ****************************************************************************
// Method: QvisViewWindow::undoView
//
// Purpose: 
//   This Qt slot function tells the viewer to undo the last view change.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 17 17:28:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::undoView()
{
    GetViewerMethods()->UndoView();
}

// ****************************************************************************
// Method: QvisViewWindow::copyViewFromCameraChecked
//
// Purpose: 
//   This Qt slot function tells the viewer to change the camera view mode.
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::copyViewFromCameraChecked(bool)
{
    GetViewerMethods()->ToggleCameraViewMode();
}

// ****************************************************************************
// Method: QvisViewWindow::undoView
//
// Purpose: 
//   This Qt slot function tells the viewer to make a new view keyframe.
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::makeViewKeyframe()
{
    GetViewerMethods()->SetViewKeyframe();
}

// ****************************************************************************
// Method: QvisViewWindow::centerChecked
//
// Purpose: 
//   This Qt slot function tells the viewer that the center of rotation
//   was set.
//
// Programmer: Eric Brugger
// Creation:   February 10, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::centerChecked(bool val)
{
    view3d->SetCenterOfRotationSet(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisViewWindow::processCenterText
//
// Purpose: 
//   This Qt slot function handles the center of rotation changing.
//
// Programmer: Eric Brugger
// Creation:   February 10, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewWindow::processCenterText()
{
    GetCurrentValues(14);
    Apply();    
}

// ****************************************************************************
// Method: QvisViewWindow::fullFrameActivationModeChanged
//
// Purpose: Qt slot function to handle changes in full frame mode 
//
// Programmer: Mark C. Miller 
// Creation:   July 5, 2005 
//
// ****************************************************************************
void
QvisViewWindow::fullFrameActivationModeChanged(int val)
{
    if (val == 0)
        view2d->SetFullFrameActivationMode(View2DAttributes::Auto);
    else if (val == 1)
        view2d->SetFullFrameActivationMode(View2DAttributes::On);
    else
        view2d->SetFullFrameActivationMode(View2DAttributes::Off);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisViewWindow::domainScaleModeChanged
//
// Purpose: Qt slot function to handle changes in domain scale
//
// Programmer: Kathleen Bonnell 
// Creation:   November 29, 2006 
//
// ****************************************************************************
void
QvisViewWindow::domainScaleModeChanged(int val)
{
    if (val != viewCurve->GetDomainScale())
    {
        viewCurve->SetDomainScale(val);
    }
}


// ****************************************************************************
// Method: QvisViewWindow::rangeScaleModeChanged
//
// Purpose: Qt slot function to handle changes in range scale
//
// Programmer: Kathleen Bonnell 
// Creation:   November 29, 2006 
//
// ****************************************************************************
void
QvisViewWindow::rangeScaleModeChanged(int val)
{
    if (val != viewCurve->GetRangeScale())
    {
        viewCurve->SetRangeScale(val);
    }
}
