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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <QvisViewWindow.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QWidget>
#include <QSlider>
#include <QNarrowLineEdit.h>

#include <DataNode.h>
#include <ViewAxisArrayAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ViewerProxy.h>
#include <enumtypes.h>

#define MIN_LINEEDIT_WIDTH 200
#define VIEW_WINDOW_SPACING 10

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
//   Jeremy Meredith, Mon Feb  4 13:44:33 EST 2008
//   Added support for axis-array views.
//
//   Brad Whitlock, Wed Apr  9 11:02:57 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisViewWindow::QvisViewWindow(const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) : QvisPostableWindowSimpleObserver(caption,
    shortName, notepad, ApplyButton)
{
    viewAxisArray = 0;
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
//   Jeremy Meredith, Mon Feb  4 13:44:33 EST 2008
//   Added support for axis-array views.
//
// ****************************************************************************

QvisViewWindow::~QvisViewWindow()
{
    if(viewAxisArray)
        viewAxisArray->Detach(this);

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
//   Kathleen Bonnell, Wed May  9 11:15:13 PDT 2007 
//   I added radio buttons for 2d log scaling.
//
//   Jeremy Meredith, Mon Feb  4 13:44:33 EST 2008
//   Added support for axis-array views.  Renamed some curve view
//   buttons to avoid namespace collisions.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun 18 13:44:44 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Thu Dec 18 09:36:57 PST 2008
//   Changed the signal used and the argument for tabSelected slot to 
//   an integer for Qt4.
//
// ****************************************************************************

void
QvisViewWindow::CreateWindowContents()
{
    // Group the options into curve, 2d, 3d and advanced tabs.
    tabs = new QTabWidget(central);
    connect(tabs, SIGNAL(currentChanged(int)),
            this, SLOT(tabSelected(int)));
    topLayout->setSpacing(5);
    topLayout->addWidget(tabs);

    //
    // Add the controls for the curve view.
    //
    pageCurve = new QWidget(central);
    QVBoxLayout *pageCurveLayout = new QVBoxLayout(pageCurve);
    pageCurveLayout->setSpacing(5);
    pageCurveLayout->setMargin(10);
    tabs->addTab(pageCurve, tr("Curve view"));

    QGridLayout *layoutCurve = new QGridLayout(0);
    pageCurveLayout->addLayout(layoutCurve);
    layoutCurve->setSpacing(VIEW_WINDOW_SPACING);

    viewportCurveLineEdit = new QLineEdit(pageCurve);
    connect(viewportCurveLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportCurveText()));
    layoutCurve->addWidget(viewportCurveLineEdit, 0, 1, 1, 3);
    QLabel *viewportCurveLabel = new QLabel(tr("Viewport"), pageCurve);
    viewportCurveLabel->setBuddy(viewportCurveLineEdit);
    layoutCurve->addWidget(viewportCurveLabel, 0, 0);

    domainCurveLineEdit = new QLineEdit(pageCurve);
    connect(domainCurveLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDomainText()));
    layoutCurve->addWidget(domainCurveLineEdit, 1, 1, 1, 3);
    QLabel *domainCurveLabel = new QLabel(tr("Domain"), pageCurve);
    domainCurveLabel->setBuddy(domainCurveLineEdit);
    layoutCurve->addWidget(domainCurveLabel, 1, 0);

    rangeCurveLineEdit = new QLineEdit(pageCurve);
    connect(rangeCurveLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRangeText()));
    layoutCurve->addWidget(rangeCurveLineEdit, 2, 1, 1, 3);
    QLabel *rangeCurveLabel = new QLabel(tr("Range"), pageCurve);
    rangeCurveLabel->setBuddy(rangeCurveLineEdit);
    layoutCurve->addWidget(rangeCurveLabel, 2, 0);

    QLabel *domainScaleLabel = new QLabel(tr("Domain Scale"), pageCurve);
    layoutCurve->addWidget(domainScaleLabel, 3, 0);
    domainScaleMode = new QButtonGroup(pageCurve);
    connect(domainScaleMode, SIGNAL(buttonClicked(int)),
            this, SLOT(domainScaleModeChanged(int)));
    domainLinear = new QRadioButton(tr("Linear"), pageCurve);
    domainScaleMode->addButton(domainLinear, 0);
    layoutCurve->addWidget(domainLinear, 3, 1);
    domainLog = new QRadioButton(tr("Log"), pageCurve);
    domainScaleMode->addButton(domainLog, 1);
    layoutCurve->addWidget(domainLog, 3, 2);

    QLabel *rangeScaleLabel = new QLabel(tr("Range Scale"), pageCurve);
    layoutCurve->addWidget(rangeScaleLabel, 4, 0);
    rangeScaleMode = new QButtonGroup(pageCurve);
    connect(rangeScaleMode, SIGNAL(buttonClicked(int)),
            this, SLOT(rangeScaleModeChanged(int)));
    rangeLinear = new QRadioButton(tr("Linear"), pageCurve);
    rangeScaleMode->addButton(rangeLinear, 0);
    layoutCurve->addWidget(rangeLinear, 4, 1);
    rangeLog = new QRadioButton(tr("Log"), pageCurve);
    rangeScaleMode->addButton(rangeLog, 1);
    layoutCurve->addWidget(rangeLog, 4, 2);
    pageCurveLayout->addStretch(10);

    //
    // Add the controls for the 2d view.
    //
    page2D = new QWidget(central);
    QVBoxLayout *page2DLayout = new QVBoxLayout(page2D);
    page2DLayout->setSpacing(5);
    page2DLayout->setMargin(10);
    tabs->addTab(page2D, tr("2D view"));

    QGridLayout *layout2D = new QGridLayout(0);
    page2DLayout->addLayout(layout2D);
    layout2D->setSpacing(VIEW_WINDOW_SPACING);

    viewportLineEdit = new QLineEdit(page2D);
    connect(viewportLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportText()));
    layout2D->addWidget(viewportLineEdit, 0, 1, 1, 4);
    QLabel *viewportLabel = new QLabel(tr("Viewport"), page2D);
    viewportLabel->setBuddy(viewportLineEdit);
    layout2D->addWidget(viewportLabel, 0, 0);

    windowLineEdit = new QLineEdit(page2D);
    connect(windowLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processWindowText()));
    layout2D->addWidget(windowLineEdit, 1, 1, 1, 4);
    QLabel *windowLabel = new QLabel(tr("Window"), page2D);
    windowLabel->setBuddy(windowLineEdit);
    layout2D->addWidget(windowLabel, 1, 0);

    QLabel *fullFrameLabel = new QLabel(tr("Full Frame"), page2D);
    layout2D->addWidget(fullFrameLabel, 2, 0);
    fullFrameActivationMode = new QButtonGroup(page2D);
    connect(fullFrameActivationMode, SIGNAL(buttonClicked(int)),
            this, SLOT(fullFrameActivationModeChanged(int)));
    fullFrameAuto = new QRadioButton(tr("Auto"), page2D);
    fullFrameActivationMode->addButton(fullFrameAuto, 0);
    layout2D->addWidget(fullFrameAuto, 2, 1);
    fullFrameOn = new QRadioButton(tr("On"), page2D);
    fullFrameActivationMode->addButton(fullFrameOn, 1);
    layout2D->addWidget(fullFrameOn, 2, 2);
    fullFrameOff = new QRadioButton(tr("Off"), page2D);
    fullFrameActivationMode->addButton(fullFrameOff, 2);
    layout2D->addWidget(fullFrameOff, 2, 3);

    QLabel *xScaleLabel = new QLabel(tr("X Scale"), page2D);
    layout2D->addWidget(xScaleLabel, 3, 0);
    xScaleMode = new QButtonGroup(page2D);
    connect(xScaleMode, SIGNAL(buttonClicked(int)),
            this, SLOT(xScaleModeChanged(int)));
    xLinear = new QRadioButton(tr("Linear"), page2D);
    xScaleMode->addButton(xLinear, 0);
    layout2D->addWidget(xLinear, 3, 1);
    xLog = new QRadioButton(tr("Log"), page2D);
    xScaleMode->addButton(xLog, 1);
    layout2D->addWidget(xLog, 3, 2);

    QLabel *yScaleLabel = new QLabel(tr("Y Scale"), page2D);
    layout2D->addWidget(yScaleLabel, 4, 0);
    yScaleMode = new QButtonGroup(page2D);
    connect(yScaleMode, SIGNAL(buttonClicked(int)),
            this, SLOT(yScaleModeChanged(int)));
    yLinear = new QRadioButton(tr("Linear"), page2D);
    yScaleMode->addButton(yLinear, 0);
    layout2D->addWidget(yLinear, 4, 1);
    yLog = new QRadioButton(tr("Log"), page2D);
    yScaleMode->addButton(yLog, 1);
    layout2D->addWidget(yLog, 4, 2);
    page2DLayout->addStretch(10);

    //
    // Add the simple controls for the 3d view.
    //
    page3D = new QWidget(central);
    QVBoxLayout *page3DLayout = new QVBoxLayout(page3D);
    page3DLayout->setSpacing(5);
    page3DLayout->setMargin(10);
    tabs->addTab(page3D, tr("3D view"));

    QGridLayout *layout3D = new QGridLayout(0);
    page3DLayout->addLayout(layout3D);
    layout3D->setSpacing(VIEW_WINDOW_SPACING);

    normalLineEdit = new QLineEdit(page3D);
    normalLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(normalLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNormalText()));
    layout3D->addWidget(normalLineEdit, 0, 1, 1, 2);
    QLabel *normalLabel = new QLabel(tr("View normal"), page3D);
    normalLabel->setBuddy(normalLineEdit);
    layout3D->addWidget(normalLabel, 0, 0);

    focusLineEdit = new QLineEdit(page3D);
    focusLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(focusLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFocusText()));
    layout3D->addWidget(focusLineEdit, 1, 1, 1, 2);
    QLabel *focusLabel = new QLabel(tr("Focus"), page3D);
    focusLabel->setBuddy(focusLineEdit);
    layout3D->addWidget(focusLabel, 1, 0);

    upvectorLineEdit = new QLineEdit(page3D);
    upvectorLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(upvectorLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processUpVectorText()));
    layout3D->addWidget(upvectorLineEdit, 2, 1, 1, 2);
    QLabel *upvectorLabel = new QLabel(tr("Up Vector"), page3D);
    upvectorLabel->setBuddy(upvectorLineEdit);
    layout3D->addWidget(upvectorLabel, 2, 0);

    viewAngleLineEdit = new QLineEdit(page3D);
    viewAngleLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(viewAngleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewAngleText()));
    layout3D->addWidget(viewAngleLineEdit, 3, 1, 1, 2);
    QLabel *viewAngleLabel = new QLabel(tr("Angle of view"), page3D);
    viewAngleLabel->setBuddy(viewAngleLineEdit);
    layout3D->addWidget(viewAngleLabel, 3, 0);

    parallelScaleLineEdit = new QLineEdit(page3D);
    parallelScaleLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(parallelScaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processParallelScaleText()));
    layout3D->addWidget(parallelScaleLineEdit, 4, 1, 1, 2);
    QLabel *parallelScaleLabel = new QLabel(tr("Parallel scale"), page3D);
    parallelScaleLabel->setBuddy(parallelScaleLineEdit);
    layout3D->addWidget(parallelScaleLabel, 4, 0);

    nearLineEdit = new QLineEdit(page3D);
    nearLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(nearLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNearText()));
    layout3D->addWidget(nearLineEdit, 5, 1, 1, 2);
    QLabel *nearLabel = new QLabel(tr("Near clipping"), page3D);
    nearLabel->setBuddy(nearLineEdit);
    layout3D->addWidget(nearLabel, 5, 0);

    farLineEdit = new QLineEdit(page3D);
    farLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(farLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFarText()));
    layout3D->addWidget(farLineEdit, 6, 1, 1, 2);
    QLabel *farLabel = new QLabel(tr("Far clipping"), page3D);
    farLabel->setBuddy(farLineEdit);
    layout3D->addWidget(farLabel, 6, 0);

    imagePanLineEdit = new QLineEdit(page3D);
    imagePanLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(imagePanLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processImagePanText()));
    layout3D->addWidget(imagePanLineEdit, 7, 1, 1, 2);
    QLabel *imagePanLabel = new QLabel(tr("Image pan"), page3D);
    imagePanLabel->setBuddy(imagePanLineEdit);
    layout3D->addWidget(imagePanLabel, 7, 0);

    imageZoomLineEdit = new QLineEdit(page3D);
    imageZoomLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(imageZoomLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processImageZoomText()));
    layout3D->addWidget(imageZoomLineEdit, 8, 1, 1, 2);
    QLabel *imageZoomLabel = new QLabel(tr("Image zoom"), page3D);
    imageZoomLabel->setBuddy(imageZoomLineEdit);
    layout3D->addWidget(imageZoomLabel, 8, 0);

    // portion for modifying the eye angle.
    eyeAngleLineEdit = new QNarrowLineEdit(page3D);
    connect(eyeAngleLineEdit, SIGNAL(returnPressed()), this,
            SLOT(processEyeAngleText()));
    QLabel *eyeAngleLabel = new QLabel(tr("Eye Angle (stereo)"), page3D);
    eyeAngleLabel->setBuddy(eyeAngleLineEdit);
    eyeAngleSlider = new QSlider(page3D);
    eyeAngleSlider->setOrientation(Qt::Horizontal);
    eyeAngleSlider->setMinimum(0);
    eyeAngleSlider->setMaximum(80);
    eyeAngleSlider->setPageStep(10);
    eyeAngleSlider->setValue(40);
    connect(eyeAngleSlider, SIGNAL(valueChanged(int)), this,
            SLOT(eyeAngleSliderChanged(int)));
    layout3D->addWidget(eyeAngleLabel, 9, 0);
    layout3D->addWidget(eyeAngleLineEdit, 9, 1);
    layout3D->addWidget(eyeAngleSlider, 9, 2);

    // Create the check boxes
    perspectiveToggle = new QCheckBox(tr("Perspective"), page3D);
    connect(perspectiveToggle, SIGNAL(toggled(bool)),
            this, SLOT(perspectiveToggled(bool)));
    layout3D->addWidget(perspectiveToggle, 10, 1);

    // Add alignment options
    alignComboBox = new QComboBox(page3D);
    alignComboBox->addItem("");
    alignComboBox->addItem("-X");
    alignComboBox->addItem("+X");
    alignComboBox->addItem("-Y");
    alignComboBox->addItem("+Y");
    alignComboBox->addItem("-Z");
    alignComboBox->addItem("+Z");
    connect(alignComboBox, SIGNAL(activated(int)),
            this, SLOT(viewButtonClicked(int)));
    layout3D->addWidget(alignComboBox, 11, 1);
    QLabel *alignLabel = new QLabel(tr("Align to axis"), page3D);
    alignLabel->setBuddy(alignComboBox);
    layout3D->addWidget(alignLabel, 11, 0);

    //
    // Add the controls for the axis array view.
    //
    pageAxisArray = new QWidget(central);
    QVBoxLayout *pageAxisArrayLayout = new QVBoxLayout(pageAxisArray);
    pageAxisArrayLayout->setSpacing(5);
    pageAxisArrayLayout->setMargin(10);
    tabs->addTab(pageAxisArray, tr("AxisArray view"));

    QGridLayout *layoutAxisArray = new QGridLayout(0);
    pageAxisArrayLayout->addLayout(layoutAxisArray);
    layoutAxisArray->setSpacing(VIEW_WINDOW_SPACING);

    viewportAxisArrayLineEdit = new QLineEdit(pageAxisArray);
    connect(viewportAxisArrayLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportAxisArrayText()));
    layoutAxisArray->addWidget(viewportAxisArrayLineEdit, 0, 1);
    QLabel *viewportAxisArrayLabel = new QLabel(tr("Viewport"), pageAxisArray);
    viewportAxisArrayLabel->setBuddy(viewportAxisArrayLineEdit);
    layoutAxisArray->addWidget(viewportAxisArrayLabel, 0, 0);

    domainAxisArrayLineEdit = new QLineEdit(pageAxisArray);
    connect(domainAxisArrayLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDomainAxisArrayText()));
    layoutAxisArray->addWidget(domainAxisArrayLineEdit, 1, 1);
    QLabel *domainAxisArrayLabel = new QLabel(tr("Domain"), pageAxisArray);
    domainAxisArrayLabel->setBuddy(domainAxisArrayLineEdit);
    layoutAxisArray->addWidget(domainAxisArrayLabel, 1, 0);

    rangeAxisArrayLineEdit = new QLineEdit(pageAxisArray);
    connect(rangeAxisArrayLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRangeAxisArrayText()));
    layoutAxisArray->addWidget(rangeAxisArrayLineEdit, 2, 1);
    QLabel *rangeAxisArrayLabel = new QLabel(tr("Range"), pageAxisArray);
    rangeAxisArrayLabel->setBuddy(rangeAxisArrayLineEdit);
    layoutAxisArray->addWidget(rangeAxisArrayLabel, 2, 0);
    pageAxisArrayLayout->addStretch(10);

    //
    // The advanced view options.
    //
    pageAdvanced = new QWidget(central);
    QVBoxLayout *pageAdvancedLayout = new QVBoxLayout(pageAdvanced);
    pageAdvancedLayout->setSpacing(5);
    pageAdvancedLayout->setMargin(10);
    tabs->addTab(pageAdvanced, tr("Advanced"));

    QGridLayout *advLayout = new QGridLayout(0);
    pageAdvancedLayout->addLayout(advLayout);
    advLayout->setSpacing(VIEW_WINDOW_SPACING);
    advLayout->setColumnStretch(1, 10);

    extentComboBox = new QComboBox(pageAdvanced);
    extentComboBox->addItem(tr("Original spatial extents"));
    extentComboBox->addItem(tr("Actual spatial extents"));
    connect(extentComboBox, SIGNAL(activated(int)),
            this, SLOT(extentTypeChanged(int)));
    advLayout->addWidget(extentComboBox, 0, 1, 1, 2);
    QLabel *l = new QLabel(tr("View based on"), pageAdvanced);
    l->setBuddy(extentComboBox);
    advLayout->addWidget(l, 0, 0);

    lockedViewToggle = new QCheckBox(tr("Locked view"), pageAdvanced);
    connect(lockedViewToggle, SIGNAL(toggled(bool)),
            this, SLOT(lockedViewChecked(bool)));
    advLayout->addWidget(lockedViewToggle, 1, 0);

    QPushButton *resetViewButton = new QPushButton(tr("Reset view"), pageAdvanced);
    connect(resetViewButton, SIGNAL(clicked()),
            this, SLOT(resetView()));
    advLayout->addWidget(resetViewButton, 2, 0);

    QPushButton *recenterButton = new QPushButton(tr("Recenter view"), pageAdvanced);
    connect(recenterButton, SIGNAL(clicked()),
            this, SLOT(recenterView()));
    advLayout->addWidget(recenterButton, 2, 1);

    QPushButton *undoButton = new QPushButton(tr("Undo view"), pageAdvanced);
    connect(undoButton, SIGNAL(clicked()),
            this, SLOT(undoView()));
    advLayout->addWidget(undoButton, 2, 2);

    copyViewFromCameraToggle = new QCheckBox(tr("Copy view from camera"),
                                             pageAdvanced);
    connect(copyViewFromCameraToggle, SIGNAL(toggled(bool)),
            this, SLOT(copyViewFromCameraChecked(bool)));
    advLayout->addWidget(copyViewFromCameraToggle, 3, 0, 1, 3);

    makeViewKeyframeButton = new QPushButton(tr("Make camera keyframe from view"),
                                             pageAdvanced);
    connect(makeViewKeyframeButton, SIGNAL(clicked()),
            this, SLOT(makeViewKeyframe()));
    advLayout->addWidget(makeViewKeyframeButton, 4, 0, 1, 3);

    centerToggle = new QCheckBox(tr("User defined center of rotation"),
                                 pageAdvanced);
    connect(centerToggle, SIGNAL(toggled(bool)),
            this, SLOT(centerChecked(bool)));
    advLayout->addWidget(centerToggle, 5, 0, 1, 3);

    centerLineEdit = new QLineEdit(pageAdvanced);
    centerLineEdit->setMinimumWidth(MIN_LINEEDIT_WIDTH);
    connect(centerLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCenterText()));
    advLayout->addWidget(centerLineEdit, 6, 1, 1, 2);
    QLabel *centerLabel = new QLabel(tr("Center"), pageAdvanced);
    centerLabel->setBuddy(centerLineEdit);
    advLayout->addWidget(centerLabel, 6, 0);
    pageAdvancedLayout->addStretch(10);

    //
    // Add the command line.
    //
    QGridLayout *gLayout = new QGridLayout(0);
    topLayout->addLayout(gLayout);
    commandLineEdit = new QLineEdit(central);
    connect(commandLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processCommandText()));
    gLayout->addWidget(commandLineEdit, 0, 1);
    QLabel *commandLabel = new QLabel(tr("Commands"), central);
    commandLabel->setBuddy(commandLineEdit);
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
QvisViewWindow::ConnectAxisArrayAttributes(ViewAxisArrayAttributes *v)
{
    viewAxisArray = v;
    viewAxisArray->Attach(this);
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
//   Jeremy Meredith, Mon Feb  4 13:46:48 EST 2008
//   Added support for axis-array views.
//
// ****************************************************************************

void
QvisViewWindow::UpdateWindow(bool doAll)
{
    // Update the appropriate widgets.
    if(SelectedSubject() == viewAxisArray || doAll)
        UpdateAxisArray(doAll);
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
//   Brad Whitlock, Mon Dec 17 10:47:02 PST 2007
//   Made it use ids.
//
//   Jeremy Meredith, Mon Feb  4 13:47:04 EST 2008
//   Renamed a couple widgets to avoid namespace collisions.
//
//   Brad Whitlock, Thu Jun 19 09:48:03 PDT 2008
//   Use DoublesToQString.
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
        case ViewCurveAttributes::ID_domainCoords:
            temp = DoublesToQString(viewCurve->GetDomainCoords(), 2);
            domainCurveLineEdit->setText(temp);
            break;
        case ViewCurveAttributes::ID_rangeCoords:
            temp = DoublesToQString(viewCurve->GetRangeCoords(), 2);
            rangeCurveLineEdit->setText(temp);
            break;
        case ViewCurveAttributes::ID_viewportCoords:
           temp = DoublesToQString(viewCurve->GetViewportCoords(), 4);
            viewportCurveLineEdit->setText(temp);
            break;
        case ViewCurveAttributes::ID_domainScale:
            domainScaleMode->blockSignals(true);
            domainScaleMode->button(viewCurve->GetDomainScale())->setChecked(true);
            domainScaleMode->blockSignals(false);
          break;
        case ViewCurveAttributes::ID_rangeScale:
            rangeScaleMode->blockSignals(true);
            rangeScaleMode->button(viewCurve->GetRangeScale())->setChecked(true);
            rangeScaleMode->blockSignals(false);
          break;
        }
    }
}

// ****************************************************************************
// Method: QvisViewWindow::UpdateAxisArray
//
// Purpose: 
//   Update the portion of the window for axis array views.
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2008
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 09:46:44 PDT 2008
//   Use DoublesToQString.
//
// ****************************************************************************

void
QvisViewWindow::UpdateAxisArray(bool doAll)
{
    if(viewAxisArray == 0)
        return;

    QString temp;

    for(int i = 0; i < viewAxisArray->NumAttributes(); ++i)
    {
        if(!viewAxisArray->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case ViewAxisArrayAttributes::ID_domainCoords:
            temp = DoublesToQString(viewAxisArray->GetDomainCoords(), 2);
            domainAxisArrayLineEdit->setText(temp);
            break;
        case ViewAxisArrayAttributes::ID_rangeCoords:
            temp = DoublesToQString(viewAxisArray->GetRangeCoords(), 2);
            rangeAxisArrayLineEdit->setText(temp);
            break;
        case ViewAxisArrayAttributes::ID_viewportCoords:
            temp = DoublesToQString(viewAxisArray->GetViewportCoords(), 4);
            viewportAxisArrayLineEdit->setText(temp);
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
//
//   Kathleen Bonnell, Wed May  9 11:15:13 PDT 2007 
//   I added radio buttons for 2d log scaling.
//
//   Brad Whitlock, Mon Dec 17 10:48:04 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jun 19 09:42:18 PDT 2008
//   Use DoublesToQString.
//
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
        case View2DAttributes::ID_windowCoords:
            temp = DoublesToQString(view2d->GetWindowCoords(), 4);
            windowLineEdit->setText(temp);
            break;
        case View2DAttributes::ID_viewportCoords:
            temp = DoublesToQString(view2d->GetViewportCoords(), 4);
            viewportLineEdit->setText(temp);
            break;
        case View2DAttributes::ID_fullFrameActivationMode:
            View2DAttributes::TriStateMode itmp;
            itmp = view2d->GetFullFrameActivationMode();
            fullFrameActivationMode->blockSignals(true);
            if (itmp == View2DAttributes::On)
                fullFrameActivationMode->button(1)->setChecked(true);
            else if (itmp == View2DAttributes::Off)
                fullFrameActivationMode->button(2)->setChecked(true);
            else
                fullFrameActivationMode->button(0)->setChecked(true);
            fullFrameActivationMode->blockSignals(false);
            break;
        case View2DAttributes::ID_fullFrameAutoThreshold:
            break;
        case View2DAttributes::ID_xScale:
            xScaleMode->blockSignals(true);
            xScaleMode->button(view2d->GetXScale())->setChecked(true);
            xScaleMode->blockSignals(false);
            break;
        case View2DAttributes::ID_yScale:
            yScaleMode->blockSignals(true);
            yScaleMode->button(view2d->GetYScale())->setChecked(true);
            yScaleMode->blockSignals(false);
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
//
//   Brad Whitlock, Mon Dec 17 10:48:15 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jun 19 09:41:01 PDT 2008
//   Use DoublesToQString.
//
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
        case View3DAttributes::ID_viewNormal:
            temp = DoublesToQString(view3d->GetViewNormal(), 3);
            normalLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_focus:
            temp = DoublesToQString(view3d->GetFocus(), 3);
            focusLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_viewUp:
            temp = DoublesToQString(view3d->GetViewUp(), 3);
            upvectorLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_viewAngle:
            temp.setNum(view3d->GetViewAngle());
            viewAngleLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_parallelScale:
            temp.setNum(view3d->GetParallelScale());
            parallelScaleLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_nearPlane:
            temp.setNum(view3d->GetNearPlane());
            nearLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_farPlane:
            temp.setNum(view3d->GetFarPlane());
            farLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_imagePan:
            temp = DoublesToQString(view3d->GetImagePan(), 2);
            imagePanLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_imageZoom:
            temp.setNum(view3d->GetImageZoom());
            imageZoomLineEdit->setText(temp);
            break;
        case View3DAttributes::ID_perspective:
            perspectiveToggle->blockSignals(true);
            perspectiveToggle->setChecked(view3d->GetPerspective());
            perspectiveToggle->blockSignals(false);
            break;
        case View3DAttributes::ID_eyeAngle:
            temp.setNum(view3d->GetEyeAngle());
            eyeAngleLineEdit->setText(temp);
            UpdateEyeAngleSliderFromAtts();
            break;
        case View3DAttributes::ID_centerOfRotationSet:
            centerToggle->blockSignals(true);
            centerToggle->setChecked(view3d->GetCenterOfRotationSet());
            centerToggle->blockSignals(false);
            break;
        case View3DAttributes::ID_centerOfRotation:
            temp = DoublesToQString(view3d->GetCenterOfRotation(), 3);
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
//
//   Hank Childs, Mon Jun 11 21:51:55 PDT 2007
//   If there is a command in the command line edit, process it.
//
//   Brad Whitlock, Mon Dec 17 11:11:11 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Wed Jun 18 14:12:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisViewWindow::UpdateGlobal(bool doAll)
{
    if(windowInfo == 0)
        return;

    QString temp;
    temp = commandLineEdit->displayText().trimmed();
    if(!temp.isEmpty())
        processCommandText();

    for(int i = 0; i < windowInfo->NumAttributes(); ++i)
    {
        if(!windowInfo->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case WindowInformation::ID_lockView:
            lockedViewToggle->blockSignals(true);
            lockedViewToggle->setChecked(windowInfo->GetLockView());
            lockedViewToggle->blockSignals(false);
            break;
        case WindowInformation::ID_viewExtentsType:
            extentComboBox->blockSignals(true);
            extentComboBox->setCurrentIndex(windowInfo->GetViewExtentsType());
            extentComboBox->blockSignals(false);
            break;
        case WindowInformation::ID_cameraViewMode:
            copyViewFromCameraToggle->blockSignals(true);
            copyViewFromCameraToggle->setChecked(
                                              windowInfo->GetCameraViewMode());
            copyViewFromCameraToggle->blockSignals(false);
            break;
        case WindowInformation::ID_winMode:
            if (!activeTabSetBySlot)
            {
                tabs->blockSignals(true);
                switch(windowInfo->GetWinMode())
                {
                    case 0: activeTab = 1; break;
                    case 1: activeTab = 2; break;
                    case 2: activeTab = 0; break;
                    default: break;
                }
                tabs->setCurrentIndex(activeTab);
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
//   Jeremy Meredith, Mon Feb  4 13:44:33 EST 2008
//   Added support for axis-array views.
//
// ****************************************************************************

void
QvisViewWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        bool doCurve     = (viewCurve->NumAttributesSelected() > 0);
        bool do2d        = (view2d->NumAttributesSelected() > 0);
        bool do3d        = (view3d->NumAttributesSelected() > 0);
        bool doAxisArray = (viewAxisArray->NumAttributesSelected() > 0);

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
        // Tell the viewer to set the AxisArray view attributes.
        if(doAxisArray || ignore)
        {
            viewAxisArray->Notify();
            GetViewerMethods()->SetViewAxisArray();
        }
    }
    else
    {
        // Send the new state to the viewer.
        viewCurve->Notify();
        view2d->Notify();
        view3d->Notify();
        viewAxisArray->Notify();
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
//   Brad Whitlock, Wed Jun 18 14:16:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisViewWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(std::string(windowTitle().toStdString()));

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
//   Brad Whitlock, Wed Jun 18 14:17:00 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisViewWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(windowTitle().toStdString()));
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
//  Method:  QvisViewWindow::GetCurrentValuesAxisArray
//
//  Purpose:
//    Get the current values for the axis array text fields.
//
//  Arguments:
//    which_widget   index of the widget, or -1 for all
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  4, 2008
//
//  Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun 18 15:25:26 PDT 2008
//   Rewrote with utility methods.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValuesAxisArray(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the viewport values.
    if(which_widget == ViewAxisArrayAttributes::ID_viewportCoords || doAll)
    {
        double v[4];
        if(LineEditGetDoubles(viewportAxisArrayLineEdit, v, 4))
            viewAxisArray->SetViewportCoords(v);
        else
        {
            ResettingError(tr("viewport"),
                         DoublesToQString(viewAxisArray->GetViewportCoords(), 4));
            viewAxisArray->SetViewportCoords(viewAxisArray->GetViewportCoords());
        }
    }

    // Do the domain values.
    if(which_widget == ViewAxisArrayAttributes::ID_domainCoords || doAll)
    {
        double v[2];
        if(LineEditGetDoubles(domainAxisArrayLineEdit, v, 2))
            viewAxisArray->SetDomainCoords(v);
        else
        {
            ResettingError(tr("domain"),
                         DoublesToQString(viewAxisArray->GetDomainCoords(), 2));
            viewAxisArray->SetDomainCoords(viewAxisArray->GetDomainCoords());
        }
    }

    // Do the range values.
    if(which_widget == ViewAxisArrayAttributes::ID_rangeCoords || doAll)
    {
        double v[2];
        if(LineEditGetDoubles(rangeAxisArrayLineEdit, v, 2))
            viewAxisArray->SetRangeCoords(v);
        else
        {
            ResettingError(tr("range"),
                         DoublesToQString(viewAxisArray->GetRangeCoords(), 2));
            viewAxisArray->SetRangeCoords(viewAxisArray->GetRangeCoords());
        }
    }
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
//   Jeremy Meredith, Mon Feb  4 13:47:04 EST 2008
//   Renamed a couple widgets to avoid namespace collisions.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun 18 15:28:28 PDT 2008
//   Rewrote using utility methods.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValuesCurve(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the viewport values.
    if(which_widget == ViewCurveAttributes::ID_viewportCoords || doAll)
    {
        double v[4];
        if(LineEditGetDoubles(viewportCurveLineEdit, v, 4))
            viewCurve->SetViewportCoords(v);
        else
        {
            ResettingError(tr("viewport"),
                         DoublesToQString(viewCurve->GetViewportCoords(), 4));
            viewCurve->SetViewportCoords(viewCurve->GetViewportCoords());
        }
    }

    // Do the domain values.
    if(which_widget == ViewCurveAttributes::ID_domainCoords || doAll)
    {
        double v[2];
        if(LineEditGetDoubles(domainCurveLineEdit, v, 2))
            viewCurve->SetDomainCoords(v);
        else
        {
            ResettingError(tr("domain"),
                         DoublesToQString(viewCurve->GetDomainCoords(), 2));
            viewCurve->SetDomainCoords(viewCurve->GetDomainCoords());
        }
    }

    // Do the range values.
    if(which_widget == ViewCurveAttributes::ID_rangeCoords || doAll)
    {
        double v[2];
        if(LineEditGetDoubles(rangeCurveLineEdit, v, 2))
            viewCurve->SetRangeCoords(v);
        else
        {
            ResettingError(tr("range"),
                         DoublesToQString(viewCurve->GetRangeCoords(), 2));
            viewCurve->SetRangeCoords(viewCurve->GetRangeCoords());
        }
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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun 18 15:28:28 PDT 2008
//   Rewrote using utility methods.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValues2d(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the viewport values.
    if(which_widget == View2DAttributes::ID_viewportCoords || doAll)
    {
        double v[4];
        if(LineEditGetDoubles(viewportLineEdit, v, 4))
            view2d->SetViewportCoords(v);
        else
        {
            ResettingError(tr("viewport"),
                         DoublesToQString(view2d->GetViewportCoords(), 4));
            view2d->SetViewportCoords(view2d->GetViewportCoords());
        }
    }

    // Do the window values.
    if(which_widget == View2DAttributes::ID_windowCoords || doAll)
    {
        double v[4];
        if(LineEditGetDoubles(windowLineEdit, v, 4))
            view2d->SetWindowCoords(v);
        else
        {
            ResettingError(tr("window"),
                         DoublesToQString(view2d->GetWindowCoords(), 4));
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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun 18 15:28:28 PDT 2008
//   Rewrote using utility methods.
//
//   Hank Childs, Mon Dec 22 09:22:38 PST 2008
//   Fix problem where up vector and view angle were accidentally merged.
//
// ****************************************************************************

void
QvisViewWindow::GetCurrentValues3d(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the normal values.
    if(which_widget == View3DAttributes::ID_viewNormal || doAll)
    {
        double v[3];
        if(LineEditGetDoubles(normalLineEdit, v, 3))
            view3d->SetViewNormal(v);
        else
        {
            ResettingError(tr("normal"),
                           DoublesToQString(view3d->GetViewNormal(), 3));
            view3d->SetViewNormal(view3d->GetViewNormal());
        }
    }

    // Do the focus values.
    if(which_widget == View3DAttributes::ID_focus || doAll)
    {
        double v[3];
        if(LineEditGetDoubles(focusLineEdit, v, 3))
            view3d->SetFocus(v);
        else
        {
            ResettingError(tr("focus"),
                           DoublesToQString(view3d->GetFocus(), 3));
            view3d->SetFocus(view3d->GetFocus());
        }
    }

    // Do the up vector values.
    if(which_widget == View3DAttributes::ID_viewUp || doAll)
    {
        double v[3];
        if(LineEditGetDoubles(upvectorLineEdit, v, 3))
            view3d->SetViewUp(v);
        else
        {
            ResettingError(tr("up vector"),
                           DoublesToQString(view3d->GetViewUp(), 3));
            view3d->SetViewUp(view3d->GetViewUp());
        }
    }

    // Do the view angle values.
    if(which_widget == View3DAttributes::ID_viewAngle || doAll)
    {
        double v;
        if(LineEditGetDouble(viewAngleLineEdit, v))
            view3d->SetViewAngle(v);
        else
        {
            ResettingError(tr("viewangle"),
                           DoubleToQString(view3d->GetViewAngle()));
            view3d->SetViewAngle(view3d->GetViewAngle());
        }
    }

    // Do the parallel scale value.
    if(which_widget == View3DAttributes::ID_parallelScale || doAll)
    {
        double v;
        if(LineEditGetDouble(parallelScaleLineEdit, v))
            view3d->SetParallelScale(v);
        else
        {
            ResettingError(tr("parallel scale"),
                           DoubleToQString(view3d->GetParallelScale()));
            view3d->SetParallelScale(view3d->GetParallelScale());
        }
    }

    // Do the near value.
    if(which_widget == View3DAttributes::ID_nearPlane || doAll)
    {
        double v;
        if(LineEditGetDouble(nearLineEdit, v))
            view3d->SetNearPlane(v);
        else
        {
            ResettingError(tr("near clipping"),
                           DoubleToQString(view3d->GetNearPlane()));
            view3d->SetNearPlane(view3d->GetNearPlane());
        }
    }

    // Do the far value.
    if(which_widget == View3DAttributes::ID_farPlane || doAll)
    {
        double v;
        if(LineEditGetDouble(farLineEdit, v))
            view3d->SetFarPlane(v);
        else
        {
            ResettingError(tr("far clipping"),
                           DoubleToQString(view3d->GetFarPlane()));
            view3d->SetFarPlane(view3d->GetFarPlane());
        }
    }

    // Do the image pan value.
    if(which_widget == View3DAttributes::ID_imagePan || doAll)
    {
        double v[2];
        if(LineEditGetDoubles(imagePanLineEdit, v, 2))
            view3d->SetImagePan(v);
        else
        {
            ResettingError(tr("image pan"),
                           DoublesToQString(view3d->GetImagePan(), 2));
            view3d->SetImagePan(view3d->GetImagePan());
        }
    }

    // Do the image zoom value.
    if(which_widget == View3DAttributes::ID_imageZoom || doAll)
    {
        double v;
        if(LineEditGetDouble(imageZoomLineEdit, v))
            view3d->SetImageZoom(v);
        else
        {
            ResettingError(tr("image zoom"),
                           DoubleToQString(view3d->GetImageZoom()));
            view3d->SetImageZoom(view3d->GetImageZoom());
        }
    }

    // Do the eye angle value.
    if(which_widget == View3DAttributes::ID_eyeAngle || doAll)
    {
        double v;
        if(LineEditGetDouble(eyeAngleLineEdit, v))
        {
            view3d->SetEyeAngle(v);
            UpdateEyeAngleSliderFromAtts();
        }
        else
        {
            ResettingError(tr("eye angle"),
                           DoubleToQString(view3d->GetEyeAngle()));
            view3d->SetEyeAngle(view3d->GetEyeAngle());
        }
    }

    // Do the center of rotation values.
    if(which_widget == View3DAttributes::ID_centerOfRotation || doAll)
    {
        double v[3];
        if(LineEditGetDoubles(centerLineEdit, v, 3))
            view3d->SetCenterOfRotation(v);
        else
        {
            ResettingError(tr("center of rotation"),
                           DoublesToQString(view3d->GetCenterOfRotation(), 3));
            view3d->SetCenterOfRotation(view3d->GetCenterOfRotation());
        }
    }
}

void
QvisViewWindow::GetCurrentValues(int which_widget)
{
    GetCurrentValuesAxisArray(which_widget);
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
    view3d->RotateAxis(axis, angle);
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

// ****************************************************************************
// Method: QvisViewWindow::show
//
// Purpose: 
//   Qt slot that is called when the window needs to be shown.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 14:08:27 PDT 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 18 14:08:30 PDT 2008
//   Qt 4.
//
// ****************************************************************************

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
    tabs->setCurrentIndex(activeTab);
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
 
    temp = commandLineEdit->displayText().trimmed();
    if(!temp.isEmpty())
    {
        ParseViewCommands(temp.toStdString().c_str()); 
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
//   Cyrus Harrison, Thu Dec 18 09:35:25 PST 2008
//   Changed input argument to tab index instead of tab name for Qt4.
//   
// ****************************************************************************

void
QvisViewWindow::tabSelected(int index)
{
    activeTab = index;
    activeTabSetBySlot = true;
}

//
// Slots for axis array Widgets.
//

void
QvisViewWindow::processViewportAxisArrayText()
{
    GetCurrentValuesAxisArray(ViewAxisArrayAttributes::ID_viewportCoords);
    Apply();    
}

void
QvisViewWindow::processDomainAxisArrayText()
{
    GetCurrentValuesAxisArray(ViewAxisArrayAttributes::ID_domainCoords);
    Apply();    
}

void
QvisViewWindow::processRangeAxisArrayText()
{
    GetCurrentValuesAxisArray(ViewAxisArrayAttributes::ID_rangeCoords);
    Apply();    
}

//
// Slots for curve Widgets.
//

void
QvisViewWindow::processViewportCurveText()
{
    GetCurrentValuesCurve(ViewCurveAttributes::ID_viewportCoords);
    Apply();    
}

void
QvisViewWindow::processDomainText()
{
    GetCurrentValuesCurve(ViewCurveAttributes::ID_domainCoords);
    Apply();    
}

void
QvisViewWindow::processRangeText()
{
    GetCurrentValuesCurve(ViewCurveAttributes::ID_rangeCoords);
    Apply();    
}

//
// Slots for 2d Widgets.
//

void
QvisViewWindow::processViewportText()
{
    GetCurrentValues2d(View2DAttributes::ID_viewportCoords);
    Apply();    
}

void
QvisViewWindow::processWindowText()
{
    GetCurrentValues2d(View2DAttributes::ID_windowCoords);
    Apply();    
}

//
// Slots for 3d Widgets.
//

void
QvisViewWindow::processNormalText()
{
    GetCurrentValues3d(View3DAttributes::ID_viewNormal);
    Apply();    
}

void
QvisViewWindow::processFocusText()
{
    GetCurrentValues3d(View3DAttributes::ID_focus);
    Apply();    
}

void
QvisViewWindow::processUpVectorText()
{
    GetCurrentValues3d(View3DAttributes::ID_viewUp);
    Apply();    
}

void
QvisViewWindow::processViewAngleText()
{
    GetCurrentValues3d(View3DAttributes::ID_viewAngle);
    Apply();    
}

void
QvisViewWindow::processParallelScaleText()
{
    GetCurrentValues3d(View3DAttributes::ID_parallelScale);
    Apply();    
}

void
QvisViewWindow::processNearText()
{
    GetCurrentValues3d(View3DAttributes::ID_nearPlane);
    Apply();    
}

void
QvisViewWindow::processFarText()
{
    GetCurrentValues3d(View3DAttributes::ID_farPlane);
    Apply();    
}

void
QvisViewWindow::processImagePanText()
{
    GetCurrentValues3d(View3DAttributes::ID_imagePan);
    Apply();    
}

void
QvisViewWindow::processImageZoomText()
{
    GetCurrentValues3d(View3DAttributes::ID_imageZoom);
    Apply();    
}

void
QvisViewWindow::processEyeAngleText()
{
    GetCurrentValues3d(View3DAttributes::ID_eyeAngle);
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
//   Brad Whitlock, Thu Jun 19 09:37:19 PDT 2008
//   Use DoublesToQString.
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
        alignComboBox->setCurrentIndex(0);
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
    QString temp(DoublesToQString(view3d->GetViewNormal(), 3));
    normalLineEdit->setText(temp);
    temp = DoublesToQString(view3d->GetViewUp(), 3);
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
//   Brad Whitlock, Wed Jun 18 16:21:35 PDT 2008
//   Use id.
//
// ****************************************************************************

void
QvisViewWindow::processCenterText()
{
    GetCurrentValues3d(View3DAttributes::ID_centerOfRotation);
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


// ****************************************************************************
// Method: QvisViewWindow::xScaleModeChanged
//
// Purpose: Qt slot function to handle changes in x scale
//
// Programmer: Kathleen Bonnell 
// Creation:   April 2, 2007 
//
// ****************************************************************************
void
QvisViewWindow::xScaleModeChanged(int val)
{
    if (val != view2d->GetXScale())
    {
        view2d->SetXScale(val);
    }
}


// ****************************************************************************
// Method: QvisViewWindow::yScaleModeChanged
//
// Purpose: Qt slot function to handle changes in y scale
//
// Programmer: Kathleen Bonnell 
// Creation:   April 2, 2007 
//
// ****************************************************************************
void
QvisViewWindow::yScaleModeChanged(int val)
{
    if (val != view2d->GetYScale())
    {
        view2d->SetYScale(val);
    }
}
