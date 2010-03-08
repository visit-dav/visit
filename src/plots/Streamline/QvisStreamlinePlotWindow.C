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

#include "QvisStreamlinePlotWindow.h"

#include <StreamlineAttributes.h>
#include <ViewerProxy.h>
#include <DataNode.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolTip>
#include <QvisColorTableButton.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
#include <QvisOpacitySlider.h>
#include <stdio.h>


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::QvisStreamlinePlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisStreamlinePlotWindow::QvisStreamlinePlotWindow(const int type,
                         StreamlineAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    streamAtts = subj;
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::~QvisStreamlinePlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisStreamlinePlotWindow::~QvisStreamlinePlotWindow()
{
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 13:05:04 PST 2004
//   I added support for coloring by vorticity.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
//   Brad Whitlock, Wed Apr 23 11:46:59 PDT 2008
//   Added tr()'s
//
//   Dave Pugmire, Mon Aug 4 2:49:38 EDT 2008
//   Added termination, algorithm and integration options.
//
//   Brad Whitlock, Wed Aug  6 10:20:49 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 12:56:11 EST 2008
//   Changed label text for streamline algorithms.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add new coloring methods, length, time and ID.
//
//   Sean Ahern, Wed Sep  3 16:19:27 EDT 2008
//   Mucked with the layout to make it a bit more consistent.
//
//   Dave Pugmire, Tue Oct 7 08:17:22 EDT 2008
//   Changed 'Termination Criteria' to 'Termination Criterion'
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added number of steps as a termination criterion.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Add pathline GUI.
//    
//   Hank Childs, Sat May  2 22:14:38 PDT 2009
//   Add support for point lists.
//
//   Dave Pugmire, Wed Jun 10 16:26:25 EDT 2009
//   Add color by variable.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // Streamline tab
    // ----------------------------------------------------------------------
    QWidget *streamlineTab = new QWidget(central);
    propertyTabs->addTab(streamlineTab, tr("Streamlines"));
    
    QGridLayout *mainLayout = new QGridLayout(streamlineTab);

    // Create the source group box.
    QGroupBox *sourceGroup = new QGroupBox(central);
    sourceGroup->setTitle(tr("Source"));
    mainLayout->addWidget(sourceGroup, 0, 0, 5, 2);
//    mainLayout->setStretchFactor(sourceGroup, 100);
    QGridLayout *sourceLayout = new QGridLayout(sourceGroup);
    sourceLayout->setMargin(5);
    sourceLayout->setSpacing(10);


    // Create the source type combo box.
    sourceLayout->addWidget(new QLabel(tr("Source type"), sourceGroup), 0, 0);
    sourceType = new QComboBox(sourceGroup);
    sourceType->addItem(tr("Single Point"));
    sourceType->addItem(tr("Point List"));
    sourceType->addItem(tr("Line"));
    sourceType->addItem(tr("Circle"));
    sourceType->addItem(tr("Plane"));
    sourceType->addItem(tr("Sphere"));
    sourceType->addItem(tr("Box"));
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    sourceLayout->addWidget(sourceType, 0, 1);



    // Create the source geometry subgroup
    QGroupBox *geometryGroup = new QGroupBox(sourceGroup);
    sourceLayout->addWidget(geometryGroup, 1, 0, 4, 2);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
    geometryLayout->setRowStretch(5,10);


    //
    // Create the widget that lets the user set the point density.
    //
    int gRow = 0;
    pointDensityLabel = new QLabel(tr("Point density"), sourceGroup);
    geometryLayout->addWidget(pointDensityLabel, gRow, 0);
    pointDensity = new QSpinBox(sourceGroup);
    pointDensity->setMinimum(1);
    pointDensity->setMaximum(1000);
    connect(pointDensity, SIGNAL(valueChanged(int)), this, SLOT(pointDensityChanged(int)));
    geometryLayout->addWidget(pointDensity, gRow, 1);
    ++gRow;

    // Create the widgets that specify a point source.
    pointSource = new QLineEdit(sourceGroup);
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(tr("Location"), sourceGroup);
    pointSourceLabel->setBuddy(pointSource);
    geometryLayout->addWidget(pointSourceLabel, gRow, 0);
    geometryLayout->addWidget(pointSource, gRow, 1);
    ++gRow;


    // Create the widgets that specify a point list.
    pointList = new QLineEdit(sourceGroup);
    connect(pointList, SIGNAL(returnPressed()),
            this, SLOT(pointListProcessText()));
    pointListLabel = new QLabel(tr("Locations"), sourceGroup);
    pointListInfo = new QLabel(tr("Format as \"X1 Y1 Z1 X2 Y2 Z2 ...\".  For 2D, use 0 for Z."), sourceGroup);
    pointListLabel->setBuddy(pointList);
    geometryLayout->addWidget(pointListLabel, gRow, 0);
    geometryLayout->addWidget(pointList, gRow, 1);
    ++gRow;
    geometryLayout->addWidget(pointListInfo, gRow, 0, 1, 2);
    ++gRow;


    // Create the widgets that specify a line source.
    lineStart = new QLineEdit(sourceGroup);
    connect(lineStart, SIGNAL(returnPressed()),
            this, SLOT(lineStartProcessText()));
    lineStartLabel = new QLabel(tr("Start"), sourceGroup);
    lineStartLabel->setBuddy(lineStart);
    geometryLayout->addWidget(lineStartLabel, gRow, 0);
    geometryLayout->addWidget(lineStart, gRow, 1);
    ++gRow;
    lineEnd = new QLineEdit(sourceGroup);
    connect(lineEnd, SIGNAL(returnPressed()),
            this, SLOT(lineEndProcessText()));
    lineEndLabel = new QLabel(tr("End"), sourceGroup);
    lineEndLabel->setBuddy(lineEnd);
    geometryLayout->addWidget(lineEndLabel, gRow, 0);
    geometryLayout->addWidget(lineEnd, gRow, 1);
    ++gRow;


    // Create the widgets that specify a plane source.
    planeOrigin = new QLineEdit(sourceGroup);
    connect(planeOrigin, SIGNAL(returnPressed()),
            this, SLOT(planeOriginProcessText()));
    planeOriginLabel = new QLabel(tr("Origin"), sourceGroup);
    planeOriginLabel->setBuddy(planeOrigin);
    geometryLayout->addWidget(planeOriginLabel,gRow,0);
    geometryLayout->addWidget(planeOrigin, gRow,1);
    ++gRow;

    planeNormal = new QLineEdit(sourceGroup);
    connect(planeNormal, SIGNAL(returnPressed()),
            this, SLOT(planeNormalProcessText()));
    planeNormalLabel = new QLabel(tr("Normal"), sourceGroup);
    planeNormalLabel->setBuddy(planeNormal);
    geometryLayout->addWidget(planeNormalLabel,gRow,0);
    geometryLayout->addWidget(planeNormal, gRow,1);
    ++gRow;

    planeUpAxis = new QLineEdit(sourceGroup);
    connect(planeUpAxis, SIGNAL(returnPressed()),
            this, SLOT(planeUpAxisProcessText()));
    planeUpAxisLabel = new QLabel(tr("Up axis"), sourceGroup);
    planeUpAxisLabel->setBuddy(planeUpAxis);
    geometryLayout->addWidget(planeUpAxisLabel,gRow,0);
    geometryLayout->addWidget(planeUpAxis, gRow,1);
    ++gRow;

    planeRadius = new QLineEdit(sourceGroup);
    connect(planeRadius, SIGNAL(returnPressed()),
            this, SLOT(planeRadiusProcessText()));
    planeRadiusLabel = new QLabel(tr("Radius"), sourceGroup);
    planeRadiusLabel->setBuddy(planeRadius);
    geometryLayout->addWidget(planeRadiusLabel,gRow,0);
    geometryLayout->addWidget(planeRadius, gRow,1);
    ++gRow;

    // Create the widgets that specify a sphere source.
    sphereOrigin = new QLineEdit(sourceGroup);
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(tr("Origin"), sourceGroup);
    sphereOriginLabel->setBuddy(sphereOrigin);
    geometryLayout->addWidget(sphereOriginLabel,gRow,0);
    geometryLayout->addWidget(sphereOrigin, gRow,1);
    ++gRow;

    sphereRadius = new QLineEdit(sourceGroup);
    connect(sphereRadius, SIGNAL(returnPressed()),
            this, SLOT(sphereRadiusProcessText()));
    sphereRadiusLabel = new QLabel(tr("Radius"), sourceGroup);
    sphereRadiusLabel->setBuddy(sphereRadius);
    geometryLayout->addWidget(sphereRadiusLabel,gRow,0);
    geometryLayout->addWidget(sphereRadius, gRow,1);
    ++gRow;

    // Create the widgets that specify a box source
    useWholeBox = new QCheckBox(tr("Whole data set"), sourceGroup);
    connect(useWholeBox, SIGNAL(toggled(bool)),
            this, SLOT(useWholeBoxChanged(bool)));
    geometryLayout->addWidget(useWholeBox, gRow, 0);
    ++gRow;

    boxExtents[0] = new QLineEdit(sourceGroup);
    connect(boxExtents[0], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[0] = new QLabel(tr("X Extents"), sourceGroup);
    boxExtentsLabel[0]->setBuddy(boxExtents[0]);
    geometryLayout->addWidget(boxExtentsLabel[0], gRow, 0);
    geometryLayout->addWidget(boxExtents[0], gRow, 1);
    ++gRow;

    boxExtents[1] = new QLineEdit(sourceGroup);
    connect(boxExtents[1], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[1] = new QLabel(tr("Y Extents"), sourceGroup);
    boxExtentsLabel[1]->setBuddy(boxExtents[1]);
    geometryLayout->addWidget(boxExtentsLabel[1], gRow, 0);
    geometryLayout->addWidget(boxExtents[1], gRow, 1);
    ++gRow;

    boxExtents[2] = new QLineEdit(sourceGroup);
    connect(boxExtents[2], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[2] = new QLabel(tr("Z Extents"), sourceGroup);
    boxExtentsLabel[2]->setBuddy(boxExtents[2]);
    geometryLayout->addWidget(boxExtentsLabel[2], gRow, 0);
    geometryLayout->addWidget(boxExtents[2], gRow, 1);


    // Create the termination group box.
    QGroupBox *terminationGroup = new QGroupBox(central);
    terminationGroup->setTitle(tr("Termination"));
    mainLayout->addWidget(terminationGroup, 6, 0, 2, 2);
//    mainLayout->setStretchFactor(terminationGroup, 100);
    QGridLayout *terminationLayout = new QGridLayout(terminationGroup);
    terminationLayout->setMargin(5);
    terminationLayout->setSpacing(10);


    // Create the maximum time text field.
    termType = new QComboBox(central);
    termType->addItem(tr("Distance"));
    termType->addItem(tr("Time"));
    termType->addItem(tr("Number of Steps"));
    connect(termType, SIGNAL(activated(int)),
            this, SLOT(termTypeChanged(int)));
    terminationLayout->addWidget(termType, 1,0);

    termination = new QLineEdit(central);
    connect(termination, SIGNAL(returnPressed()),
            this, SLOT(terminationProcessText()));
    terminationLayout->addWidget(termination, 1,1);

    //Create the direction of integration.
    terminationLayout->addWidget(new QLabel(tr("Streamline direction"), central),3,0);
    directionType = new QComboBox(central);
    directionType->addItem(tr("Forward"));
    directionType->addItem(tr("Backward"));
    directionType->addItem(tr("Both"));
    connect(directionType, SIGNAL(activated(int)),
            this, SLOT(directionTypeChanged(int)));
    terminationLayout->addWidget(directionType, 3,1);

    /*pathlineFlag = new QCheckBox(tr("Pathlines"), central);
    connect(pathlineFlag, SIGNAL(toggled(bool)),
            this, SLOT(pathlineFlagChanged(bool)));
    terminationLayout->addWidget(pathlineFlag, 4,0);
    */


    // Create the integration group box.
    QGroupBox *integrationGroup = new QGroupBox(central);
    integrationGroup->setTitle(tr("Integration"));
    mainLayout->addWidget(integrationGroup, 8, 0, 4, 2);
//    mainLayout->setStretchFactor(integrationGroup, 100);
    QGridLayout *integrationLayout = new QGridLayout(integrationGroup);
    integrationLayout->setMargin(5);
    integrationLayout->setSpacing(10);

    integrationLayout->addWidget( new QLabel(tr("Integrator"), integrationGroup), 0,0);
    integrationType = new QComboBox(integrationGroup);
    integrationType->addItem(tr("Dormand-Prince (Runge-Kutta)"));
    integrationType->addItem(tr("Adams-Bashforth (Multi-step)"));
    connect(integrationType, SIGNAL(activated(int)),
            this, SLOT(integrationTypeChanged(int)));
    integrationLayout->addWidget(integrationType, 0,1);
    
    // Create the step length text field.
    maxStepLengthLabel = new QLabel(tr("Maximum step length"), integrationGroup);
    maxStepLength = new QLineEdit(integrationGroup);
    connect(maxStepLength, SIGNAL(returnPressed()),
            this, SLOT(maxStepLengthProcessText()));
    integrationLayout->addWidget(maxStepLengthLabel, 1,0);
    integrationLayout->addWidget(maxStepLength, 1,1);

    // Create the relative tolerance text field.
    relTolLabel = new QLabel(tr("Relative tolerance"), integrationGroup);
    relTol = new QLineEdit(integrationGroup);
    connect(relTol, SIGNAL(returnPressed()),
            this, SLOT(relTolProcessText()));
    integrationLayout->addWidget(relTolLabel, 2,0);
    integrationLayout->addWidget(relTol, 2, 1);

    // Create the absolute tolerance text field.
    absTolLabel = new QLabel(tr("Absolute tolerance"), integrationGroup);
    absTol = new QLineEdit(integrationGroup);
    connect(absTol, SIGNAL(returnPressed()),
            this, SLOT(absTolProcessText()));
    integrationLayout->addWidget(absTolLabel, 3,0);
    integrationLayout->addWidget(absTol, 3, 1);

    // ----------------------------------------------------------------------
    // Appearance tab
    // ----------------------------------------------------------------------
    QWidget *appearanceTab = new QWidget(central);
    propertyTabs->addTab(appearanceTab, tr("Appearance"));
    CreateAppearanceTab(appearanceTab);

    // ----------------------------------------------------------------------
    // Parallel tab
    // ----------------------------------------------------------------------
    QWidget *parallelTab = new QWidget(central);
    propertyTabs->addTab(parallelTab, tr("Parallel"));
    CreateAdvancedTab(parallelTab);
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::CreateAppearanceTab
//
// Purpose: 
//   Populates the appearance tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add ramp opacity and draw geom head.
//
//   Dave Pugmire, Tue Feb 16 09:08:32 EST 2010
//   Add display head geom as cone.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateAppearanceTab(QWidget *pageAppearance)
{
    QGridLayout *appearanceLayout = new QGridLayout(pageAppearance);
    appearanceLayout->setMargin(5);
    appearanceLayout->setSpacing(10);

    // Create the data group
    QGroupBox *dataGroup = new QGroupBox(pageAppearance);
    dataGroup->setTitle(tr("Data"));
    appearanceLayout->addWidget(dataGroup);

    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    dataLayout->setSpacing(10);
    dataLayout->setColumnStretch(2,10);

    // Create the data value.
    dataLayout->addWidget(new QLabel(tr("Data Value"), dataGroup), 0, 0);

    dataValueComboBox = new QComboBox(dataGroup);
    dataValueComboBox->addItem(tr("Solid"),0);
    dataValueComboBox->addItem(tr("Speed"),1);
    dataValueComboBox->addItem(tr("Vorticity magnitude"),2);
    dataValueComboBox->addItem(tr("Arc length"),3);
    dataValueComboBox->addItem(tr("Time"),4);
    dataValueComboBox->addItem(tr("Seed point ID"),5);
    dataValueComboBox->addItem(tr("Variable"),6);
    connect(dataValueComboBox, SIGNAL(activated(int)),
            this, SLOT(coloringMethodChanged(int)));

    dataLayout->addWidget(dataValueComboBox, 0, 1);

    dataLayout->addWidget(new QLabel(tr("   "), central), 0, 2);


    // Create the limits group box.
    QGroupBox *limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 1, 3);

    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

    limitsLayout->addWidget(new QLabel(tr("Limits"), central), 0, 0);

    legendMinToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(legendMinToggle, 0, 1);
    connect(legendMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendMinToggled(bool)));
    legendMinEdit = new QLineEdit(central);
    connect(legendMinEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText()));
    limitsLayout->addWidget(legendMinEdit, 0, 2);

    legendMaxToggle = new QCheckBox(tr("Maximum"), central);
    connect(legendMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendMaxToggled(bool)));
    limitsLayout->addWidget(legendMaxToggle, 0, 3);

    legendMaxEdit = new QLineEdit(central);
    connect(legendMaxEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText()));
    limitsLayout->addWidget(legendMaxEdit, 0, 4);


    // Create the display group
    QGroupBox *displayGrp = new QGroupBox(pageAppearance);
    displayGrp->setTitle(tr("Display"));
    appearanceLayout->addWidget(displayGrp);

    QGridLayout *dLayout = new QGridLayout(displayGrp);
    dLayout->setMargin(5);
    dLayout->setSpacing(10);
    dLayout->setColumnStretch(2,10);


    // Create the display subgroup
    QWidget *drawGroup = new QWidget(displayGrp);
    dLayout->addWidget(drawGroup, 0, 0, 2, 5);

    QGridLayout *drawLayout = new QGridLayout(drawGroup);
    drawLayout->setMargin(5);
    drawLayout->setSpacing(10);

    // Create widgets that help determine the appearance of the streamlines.
    displayMethod = new QComboBox(displayGrp);
    displayMethod->addItem(tr("Lines"), 0);
    displayMethod->addItem(tr("Tubes"), 1);
    displayMethod->addItem(tr("Ribbons"), 2);
    connect(displayMethod, SIGNAL(activated(int)), this, SLOT(displayMethodChanged(int)));
    drawLayout->addWidget(new QLabel(tr("Draw as"), displayGrp), 0, 0);
    drawLayout->addWidget(displayMethod, 0, 1);

    //--lines
    lineWidth = new QvisLineWidthWidget(0, displayGrp);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(tr("Width"), displayGrp);
    lineWidthDummy = new QLabel(tr("  "), displayGrp);
    lineWidthLabel->setBuddy(lineWidth);
    lineWidthDummy->setBuddy(lineWidth);
    drawLayout->addWidget(lineWidthLabel, 1, 0, Qt::AlignRight);
    drawLayout->addWidget(lineWidth, 1, 1);
    drawLayout->addWidget(lineWidthDummy, 1, 2);
    drawLayout->addWidget(lineWidthDummy, 1, 3);

    //--tube/ribbon
    tubeRadius = new QLineEdit(displayGrp);
    ribbonWidth = new QLineEdit(displayGrp);
    connect(tubeRadius, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusProcessText()));
    connect(ribbonWidth, SIGNAL(returnPressed()),
            this, SLOT(ribbonWidthProcessText()));
    radiusLabel = new QLabel(tr("Radius"), displayGrp);
    radiusLabel->setBuddy(tubeRadius);
    radiusLabel->setToolTip(tr("Radius used for tubes and ribbons."));
    drawLayout->addWidget(radiusLabel, 1, 0, Qt::AlignRight);
    drawLayout->addWidget(tubeRadius, 1, 1);
    drawLayout->addWidget(ribbonWidth, 1, 1);

    tubeDisplayDensity = new QSpinBox(displayGrp);
    tubeDisplayDensity->setMinimum(2);
    tubeDisplayDensity->setMaximum(100);
    tubeDisplayDensityLabel = new QLabel(tr("Display density"), displayGrp);
    connect(tubeDisplayDensity, SIGNAL(valueChanged(int)), this, SLOT(tubeDisplayDensityChanged(int)));
    drawLayout->addWidget(tubeDisplayDensityLabel, 1, 2);
    drawLayout->addWidget(tubeDisplayDensity, 1, 3);


    // Splitter
    QFrame *splitter = new QFrame(displayGrp);
    splitter->setFrameStyle(QFrame::HLine + QFrame::Raised);
    dLayout->addWidget(splitter, 2, 0, 1, 5);



    // Create the show subgroup
    QWidget *showGroup = new QWidget(displayGrp);
    dLayout->addWidget(showGroup, 3, 0, 4, 5);

    QGridLayout *showLayout = new QGridLayout(showGroup);
    showLayout->setMargin(5);
    showLayout->setSpacing(10);

    //show seeds
    showSeeds = new QCheckBox(tr("Show seeds"), displayGrp);
    connect(showSeeds, SIGNAL(toggled(bool)),
            this, SLOT(showSeedsChanged(bool)));
    showLayout->addWidget(showSeeds, 0, 0);

    seedRadius = new QLineEdit(displayGrp);
    connect(seedRadius, SIGNAL(returnPressed()), this, SLOT(seedRadiusProcessText()));
    seedRadiusLabel = new QLabel(tr("Radius"), displayGrp);
    seedRadiusLabel->setBuddy(seedRadius);
    seedRadiusLabel->setToolTip(tr("Radius for seed point display."));
    seedRadiusLabel->setBuddy(seedRadius);
    showLayout->addWidget(seedRadiusLabel, 0, 1, Qt::AlignRight);
    showLayout->addWidget(seedRadius, 0, 2);

    //show heads
    showHeads = new QCheckBox(tr("Show heads"), displayGrp);
    connect(showHeads, SIGNAL(toggled(bool)),this, SLOT(showHeadsChanged(bool)));
    showLayout->addWidget(showHeads, 1, 0);
    
    headDisplayTypeLabel = new QLabel(tr("Display as"), displayGrp);
    headDisplayType = new QComboBox(displayGrp);
    headDisplayType->addItem(tr("Sphere"), 0);
    headDisplayType->addItem(tr("Cone"), 1);
    connect(headDisplayType, SIGNAL(activated(int)), this, SLOT(headDisplayTypeChanged(int)));
    showLayout->addWidget(headDisplayTypeLabel, 1, 1, Qt::AlignRight);
    showLayout->addWidget(headDisplayType, 1, 2);


    headRadius = new QLineEdit(displayGrp);
    connect(headRadius, SIGNAL(returnPressed()), this, SLOT(headRadiusProcessText()));
    headRadiusLabel = new QLabel(tr("Radius"), displayGrp);
    headRadiusLabel->setBuddy(headRadius);
    headRadiusLabel->setToolTip(tr("Radius for head point display."));
    headHeight = new QLineEdit(displayGrp);
    connect(headHeight, SIGNAL(returnPressed()), this, SLOT(headHeightProcessText()));
    headHeightLabel = new QLabel(tr("Height"), displayGrp);
    headHeightLabel->setBuddy(headHeight);
    headHeightLabel->setToolTip(tr("Height for head point display."));
    showLayout->addWidget(headRadiusLabel, 2, 1, Qt::AlignRight);
    showLayout->addWidget(headRadius, 2, 2);
    showLayout->addWidget(headHeightLabel, 2, 3);
    showLayout->addWidget(headHeight, 2, 4);


    geomDisplayQuality = new QComboBox(displayGrp);
    geomDisplayQuality->addItem(tr("Low"), 0);
    geomDisplayQuality->addItem(tr("Medium"), 1);
    geomDisplayQuality->addItem(tr("High"), 2);
    geomDisplayQuality->addItem(tr("Super"), 3);
    geomDisplayQualityLabel = new QLabel(tr("Display quality"), displayGrp);
    connect(geomDisplayQuality, SIGNAL(activated(int)), this, SLOT(geomDisplayQualityChanged(int)));
    showLayout->addWidget(geomDisplayQualityLabel, 3, 0);
    showLayout->addWidget(geomDisplayQuality, 3, 1);


    // Splitter
    splitter = new QFrame(displayGrp);
    splitter->setFrameStyle(QFrame::HLine + QFrame::Raised);
    dLayout->addWidget(splitter, 7, 0, 1, 5);


    QWidget *ddGroup = new QWidget(displayGrp);
    dLayout->addWidget(ddGroup, 8, 0, 1, 5);

    QHBoxLayout *ddLayout = new QHBoxLayout(ddGroup);
    ddLayout->setMargin(5);
    ddLayout->setSpacing(10);


    displayLabel = new QLabel(tr("Display"), displayGrp);
    displayBeginToggle = new QCheckBox(tr("Begin"), displayGrp);
    displayEndToggle = new QCheckBox(tr("End"), displayGrp);
    connect(displayBeginToggle, SIGNAL(toggled(bool)), this, SLOT(displayBeginToggled(bool)));
    connect(displayEndToggle, SIGNAL(toggled(bool)), this, SLOT(displayEndToggled(bool)));

    displayBeginEdit = new QLineEdit(displayGrp);
    displayEndEdit = new QLineEdit(displayGrp);
    connect(displayBeginEdit, SIGNAL(returnPressed()), this, SLOT(processDisplayBeginText()));
    connect(displayEndEdit, SIGNAL(returnPressed()), this, SLOT(processDisplayEndText()));

    ddLayout->addWidget(displayLabel, 0);
    ddLayout->addWidget(displayBeginToggle, 1);
    ddLayout->addWidget(displayBeginEdit, 2);
    ddLayout->addWidget(displayEndToggle, 3, Qt::AlignRight);
    ddLayout->addWidget(displayEndEdit, 4);



    QGroupBox *colorGrp = new QGroupBox(pageAppearance);
    colorGrp->setTitle(tr("Color"));
    appearanceLayout->addWidget(colorGrp);

    QGridLayout *cLayout = new QGridLayout(colorGrp);
    cLayout->setMargin(5);
    cLayout->setSpacing(10);
    cLayout->setColumnStretch(1,10);
    int cRow = 0;


    //--table
    colorTableName = new QvisColorTableButton(colorGrp);
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    colorTableNameLabel = new QLabel(tr("Color table"), colorGrp);
    colorTableNameLabel->setBuddy(colorTableName);
    cLayout->addWidget(colorTableNameLabel,cRow,0);
    cLayout->addWidget(colorTableName, cRow,1, Qt::AlignLeft);
    cRow++;

    //--single
    singleColor = new QvisColorButton(colorGrp);
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));
    singleColorLabel = new QLabel(tr("Single color"), colorGrp);
    singleColorLabel->setBuddy(singleColor);
    cLayout->addWidget(singleColorLabel,cRow,0);
    cLayout->addWidget(singleColor, cRow,1, Qt::AlignLeft);
    cRow++;


    
    // Create the opacity widgets.
    opacityType = new QComboBox(colorGrp);
    opacityType->addItem(tr("None"),0);
    opacityType->addItem(tr("Constant"),1);
    opacityType->addItem(tr("Ramp"),2);
    opacityType->addItem(tr("Variable Range"),3);
    connect(opacityType, SIGNAL(activated(int)),
            this, SLOT(opacityTypeChanged(int)));
    cLayout->addWidget(new QLabel(tr("Opacity"), colorGrp), cRow,0);
    cLayout->addWidget(opacityType, cRow, 1);

    opacityVarLabel = new QLabel(tr("Variable"), colorGrp);
    opacityVar = new QvisVariableButton(false, true, true,
                                        QvisVariableButton::Scalars, colorGrp);
    cLayout->addWidget(opacityVarLabel,cRow,2);
    cLayout->addWidget(opacityVar,cRow,3);
    connect(opacityVar, SIGNAL(activated(const QString &)),
            this, SLOT(opacityVariableChanged(const QString&)));

    cRow++;
    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, colorGrp);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    cLayout->addWidget(opacitySlider, cRow, 1, 1,3);
    cRow++;

    opacityMinToggle = new QCheckBox(tr("Opacity Min"), displayGrp);
    opacityMaxToggle = new QCheckBox(tr("Opacity Max"), displayGrp);
    connect(opacityMinToggle, SIGNAL(toggled(bool)), this, SLOT(opacityMinToggled(bool)));
    connect(opacityMaxToggle, SIGNAL(toggled(bool)), this, SLOT(opacityMaxToggled(bool)));

    opacityVarMin = new QLineEdit(colorGrp);
    opacityVarMax = new QLineEdit(colorGrp);

    connect(opacityVarMin, SIGNAL(returnPressed()), this, SLOT(processOpacityVarMin()));
    connect(opacityVarMax, SIGNAL(returnPressed()), this, SLOT(processOpacityVarMax()));
    cLayout->addWidget(opacityMinToggle, cRow, 1);
    cLayout->addWidget(opacityVarMin, cRow, 2);
    cLayout->addWidget(opacityMaxToggle, cRow, 3);
    cLayout->addWidget(opacityVarMax, cRow, 4);
    cRow++;

    varLabel = new QLabel(tr("Variable"), colorGrp);
    var = new QvisVariableButton(false, true, true, QvisVariableButton::Scalars,
                                 colorGrp);
    cLayout->addWidget(varLabel,cRow,2);
    cLayout->addWidget(var,cRow,3);
    connect(var, SIGNAL(activated(const QString &)),
            this, SLOT(coloringVariableChanged(const QString&)));
    cRow++;


    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    appearanceLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setMargin(5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendFlag = new QCheckBox(tr("Legend"), central);
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    miscLayout->addWidget(legendFlag, 0, 0);

    // Create the lighting toggle
    lightingFlag = new QCheckBox(tr("Lighting"), central);
    connect(lightingFlag, SIGNAL(toggled(bool)),
            this, SLOT(lightingFlagChanged(bool)));
    miscLayout->addWidget(lightingFlag, 0, 1);
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::CreateAdvancedTab
//
// Purpose: 
//   Populates the advanced tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateAdvancedTab(QWidget *pageAdvanced)
{
    int row = 0;
    QGridLayout *advGLayout = new QGridLayout(pageAdvanced);
    advGLayout->setMargin(5);
    advGLayout->setSpacing(5);

    QGroupBox *algoGrp = new QGroupBox(pageAdvanced);
    algoGrp->setTitle(tr("Parallel streamline options"));
    advGLayout->addWidget(algoGrp, 0, 0, 1, 4);

    // Algorithm group.
    QGridLayout *algoGLayout = new QGridLayout(algoGrp);
    algoGLayout->setSpacing(10);
    algoGLayout->setColumnStretch(1,10);

    slAlgoLabel = new QLabel(tr("Parallelize across"), algoGrp);
    slAlgo = new QComboBox(algoGrp);
    slAlgo->addItem(tr("Streamlines"));
    slAlgo->addItem(tr("Domains"));
    slAlgo->addItem(tr("Streamlines and Domains"));
    connect(slAlgo, SIGNAL(activated(int)),
            this, SLOT(streamlineAlgorithmChanged(int)));
    algoGLayout->addWidget( slAlgoLabel, 1,0);
    algoGLayout->addWidget( slAlgo, 1,1);
    
    maxSLCountLabel = new QLabel(tr("Communication threshold"), algoGrp);
    maxSLCount = new QSpinBox(algoGrp);
    maxSLCount->setMinimum(1);
    maxSLCount->setMaximum(100000);
    connect(maxSLCount, SIGNAL(valueChanged(int)), 
            this, SLOT(maxSLCountChanged(int)));
    algoGLayout->addWidget( maxSLCountLabel, 2,0);
    algoGLayout->addWidget( maxSLCount,2,1);

    maxDomainCacheLabel = new QLabel(tr("Domain cache size"), algoGrp);
    maxDomainCache = new QSpinBox(algoGrp);
    maxDomainCache->setMinimum(1);
    maxDomainCache->setMaximum(100000);
    connect(maxDomainCache, SIGNAL(valueChanged(int)),
            this, SLOT(maxDomainCacheChanged(int)));
    algoGLayout->addWidget( maxDomainCacheLabel, 3,0);
    algoGLayout->addWidget( maxDomainCache, 3,1);

    workGroupSizeLabel = new QLabel(tr("Work group size"), algoGrp);
    workGroupSize = new QSpinBox(algoGrp);
    workGroupSize->setMinimum(2);
    workGroupSize->setMaximum(1000000);
    connect(workGroupSize, SIGNAL(valueChanged(int)),
            this, SLOT(workGroupSizeChanged(int)));
    algoGLayout->addWidget( workGroupSizeLabel, 4,0);
    algoGLayout->addWidget( workGroupSize, 4,1);
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::ProcessOldVersions
//
// Purpose: 
//   Allows modification of the data node before it is used.
//
// Arguments:
//   parentNode    : The node that's a parent node of the node that is used
//                   by this window.
//   configVersion : The version of the config file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 3 11:24:24 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Aug  6 10:38:36 PDT 2008
//   Compensate for 2.0.0 version changes.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::ProcessOldVersions(DataNode *parentNode,
    const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("Streamline plot attributes");
    if(searchNode == 0)
        return;

    // Remove height/width information if the config file is older than
    // 1.4.1 since this window changed size quite a bit in version 1.4.1.
    if (StreamlineAttributes::VersionLessThan(configVersion, "1.4.1") ||
        StreamlineAttributes::VersionLessThan(configVersion, "2.0.0"))
    {
        searchNode->RemoveNode("width");
        searchNode->RemoveNode("height");
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Brad Whitlock, Wed Dec 22 13:10:59 PST 2004
//   I added support for coloring by vorticity and for showing ribbons.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added support for useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
//   Brad Whitlock, Wed Aug  6 10:39:12 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EDT 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Fri Aug 8 16:27:03 EDT 2008
//   Set the termType combo box.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateWindow(bool doAll)
{
    QString       temp;
    const double *dptr;
    QColor        tempcolor;

    for(int i = 0; i < streamAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!streamAtts->IsSelected(i))
            {
                continue;
            }
        }
        switch(i)
        {
        case StreamlineAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.
            UpdateSourceAttributes();

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(streamAtts->GetSourceType());
            sourceType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxStepLength:
            temp.setNum(streamAtts->GetMaxStepLength());
            maxStepLength->setText(temp);
            break;
        case StreamlineAttributes::ID_termination:
            temp.setNum(streamAtts->GetTermination());
            termination->setText(temp);
            break;
        case StreamlineAttributes::ID_pointSource:
            pointSource->setText(DoublesToQString(streamAtts->GetPointSource(),3));
            break;
        case StreamlineAttributes::ID_lineStart:
            lineStart->setText(DoublesToQString(streamAtts->GetLineStart(),3));
            break;
        case StreamlineAttributes::ID_lineEnd:
            lineEnd->setText(DoublesToQString(streamAtts->GetLineEnd(),3));
            break;
        case StreamlineAttributes::ID_planeOrigin:
            planeOrigin->setText(DoublesToQString(streamAtts->GetPlaneOrigin(),3));
            break;
        case StreamlineAttributes::ID_planeNormal:
            planeNormal->setText(DoublesToQString(streamAtts->GetPlaneNormal(),3));
            break;
        case StreamlineAttributes::ID_planeUpAxis:
            planeUpAxis->setText(DoublesToQString(streamAtts->GetPlaneUpAxis(),3));
            break;
        case StreamlineAttributes::ID_planeRadius:
            temp.setNum(streamAtts->GetPlaneRadius());
            planeRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_sphereOrigin:
            sphereOrigin->setText(DoublesToQString(streamAtts->GetSphereOrigin(),3));
            break;
        case StreamlineAttributes::ID_sphereRadius:
            temp.setNum(streamAtts->GetSphereRadius());
            sphereRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_pointList:
            pointList->setText(DoublesToQString(streamAtts->GetPointList()));
            break;
        case StreamlineAttributes::ID_boxExtents:
            boxExtents[0]->setText(DoublesToQString(streamAtts->GetBoxExtents(),2));
            boxExtents[1]->setText(DoublesToQString(streamAtts->GetBoxExtents()+2,2));
            boxExtents[2]->setText(DoublesToQString(streamAtts->GetBoxExtents()+4,2));
            break;
        case StreamlineAttributes::ID_useWholeBox:
            useWholeBox->blockSignals(true);
            useWholeBox->setChecked(streamAtts->GetUseWholeBox());
            if (streamAtts->GetUseWholeBox())
            {
                boxExtents[0]->setEnabled(false);
                boxExtents[1]->setEnabled(false);
                boxExtents[2]->setEnabled(false);
                boxExtentsLabel[0]->setEnabled(false);
                boxExtentsLabel[1]->setEnabled(false);
                boxExtentsLabel[2]->setEnabled(false);
            }
            else
            {
                boxExtents[0]->setEnabled(true);
                boxExtents[1]->setEnabled(true);
                boxExtents[2]->setEnabled(true);
                boxExtentsLabel[0]->setEnabled(true);
                boxExtentsLabel[1]->setEnabled(true);
                boxExtentsLabel[2]->setEnabled(true);
            }
            useWholeBox->blockSignals(false);
            break;
        case StreamlineAttributes::ID_pointDensity:
            pointDensity->blockSignals(true);
            pointDensity->setValue(streamAtts->GetPointDensity());
            pointDensity->blockSignals(false);
            break;

          case StreamlineAttributes::ID_tubeDisplayDensity:
            tubeDisplayDensity->blockSignals(true);
            tubeDisplayDensity->setValue(streamAtts->GetTubeDisplayDensity());
            tubeDisplayDensity->blockSignals(false);
            break;

          case StreamlineAttributes::ID_headDisplayType:
            headDisplayType->blockSignals(true);
            headDisplayType->setCurrentIndex(streamAtts->GetHeadDisplayType());
            headDisplayType->blockSignals(false);

            if (streamAtts->GetShowHeads() && streamAtts->GetHeadDisplayType() == StreamlineAttributes::Cone)
            {
                headHeight->setEnabled(true);
                headHeightLabel->setEnabled(true);
                headHeight->show();
                headHeightLabel->show();
            }
            else
            {
                headHeight->setEnabled(false);
                headHeightLabel->setEnabled(false);
                headHeight->hide();
                headHeightLabel->hide();
            }
            break;

          case StreamlineAttributes::ID_geomDisplayQuality:
            geomDisplayQuality->blockSignals(true);
            geomDisplayQuality->setCurrentIndex(streamAtts->GetGeomDisplayQuality());
            geomDisplayQuality->blockSignals(false);
            break;

        case StreamlineAttributes::ID_displayMethod:
            { // new scope
            if (streamAtts->GetDisplayMethod() == StreamlineAttributes::Lines)
            {
                lineWidth->show();
                lineWidthLabel->show();
                lineWidthDummy->show();

                tubeRadius->hide();
                ribbonWidth->hide();
                radiusLabel->hide();
                tubeDisplayDensityLabel->hide();
                tubeDisplayDensity->hide();
            }
            else
            {
                lineWidth->hide();
                lineWidthLabel->hide();
                lineWidthDummy->hide();
                if (streamAtts->GetDisplayMethod() == StreamlineAttributes::Tubes)
                {
                    tubeRadius->show();
                    ribbonWidth->hide();
                }
                if (streamAtts->GetDisplayMethod() == StreamlineAttributes::Ribbons)
                {
                    ribbonWidth->show();
                    tubeRadius->hide();
                }
                
                radiusLabel->show();
                tubeDisplayDensityLabel->show();
                tubeDisplayDensity->show();
                geomDisplayQualityLabel->show();
                geomDisplayQuality->show();
            }

            displayMethod->blockSignals(true);
            displayMethod->setCurrentIndex(int(streamAtts->GetDisplayMethod()));
            displayMethod->blockSignals(false);
            }
            break;
            
        case StreamlineAttributes::ID_coloringVariable:
            temp = streamAtts->GetColoringVariable().c_str();
            var->setText(temp);
          break;
          
        case StreamlineAttributes::ID_showSeeds:
            seedRadius->setEnabled(streamAtts->GetShowSeeds());
            seedRadiusLabel->setEnabled(streamAtts->GetShowSeeds());
            geomDisplayQuality->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());
            geomDisplayQualityLabel->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());

            showSeeds->blockSignals(true);
            showSeeds->setChecked(streamAtts->GetShowSeeds());
            showSeeds->blockSignals(false);
            break;

        case StreamlineAttributes::ID_showHeads:
            headRadius->setEnabled(streamAtts->GetShowHeads());
            headRadiusLabel->setEnabled(streamAtts->GetShowHeads());
            headDisplayType->setEnabled(streamAtts->GetShowHeads());
            headHeight->setEnabled(streamAtts->GetShowHeads() && streamAtts->GetHeadDisplayType() == StreamlineAttributes::Cone);
            headHeightLabel->setEnabled(streamAtts->GetShowHeads() && streamAtts->GetHeadDisplayType() == StreamlineAttributes::Cone);
            geomDisplayQuality->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());
            geomDisplayQualityLabel->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());

            showHeads->blockSignals(true);
            showHeads->setChecked(streamAtts->GetShowHeads());
            showHeads->blockSignals(false);
            break;
            
        case StreamlineAttributes::ID_seedDisplayRadius:
            temp.setNum(streamAtts->GetSeedDisplayRadius());
            seedRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_headDisplayRadius:
            temp.setNum(streamAtts->GetHeadDisplayRadius());
            headRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_headDisplayHeight:
            temp.setNum(streamAtts->GetHeadDisplayHeight());
            headHeight->setText(temp);
            break;

        case StreamlineAttributes::ID_tubeRadius:
            temp.setNum(streamAtts->GetTubeRadius());
            tubeRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_ribbonWidth:
            temp.setNum(streamAtts->GetRibbonWidth());
            ribbonWidth->setText(temp);
            break;
        case StreamlineAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(streamAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case StreamlineAttributes::ID_coloringMethod:
            {// New scope
            bool needCT = streamAtts->GetColoringMethod() != StreamlineAttributes::Solid;
            colorTableName->setEnabled(needCT);
            colorTableNameLabel->setEnabled(needCT);
            if (needCT)
            {
                singleColor->hide();
                singleColorLabel->hide();
                colorTableName->show();
                colorTableNameLabel->show();
            }
            else
            {
                singleColor->show();
                singleColorLabel->show();
                colorTableName->hide();
                colorTableNameLabel->hide();
            }

            legendMaxToggle->setEnabled(needCT);
            legendMinToggle->setEnabled(needCT);
            legendMaxEdit->setEnabled((needCT ? streamAtts->GetLegendMaxFlag() : false));
            legendMinEdit->setEnabled((needCT ? streamAtts->GetLegendMinFlag() : false));

            dataValueComboBox->blockSignals(true);
            dataValueComboBox->setCurrentIndex(int(streamAtts->GetColoringMethod()));
            dataValueComboBox->blockSignals(false);

            if (streamAtts->GetColoringMethod() == StreamlineAttributes::ColorByVariable)
            {
                varLabel->setEnabled(true);
                var->setEnabled(true);
                varLabel->show();
                var->show();
            }
            else
            {
                varLabel->setEnabled(false);
                var->setEnabled(false);
                varLabel->hide();
                var->hide();
            }
            
            }
            break;
        case StreamlineAttributes::ID_colorTableName:
            colorTableName->setColorTable(streamAtts->GetColorTableName().c_str());
            break;
        case StreamlineAttributes::ID_singleColor:
            tempcolor = QColor(streamAtts->GetSingleColor().Red(),
                               streamAtts->GetSingleColor().Green(),
                               streamAtts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case StreamlineAttributes::ID_legendFlag:
            legendFlag->blockSignals(true);
            legendFlag->setChecked(streamAtts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
        case StreamlineAttributes::ID_lightingFlag:
            lightingFlag->blockSignals(true);
            lightingFlag->setChecked(streamAtts->GetLightingFlag());
            lightingFlag->blockSignals(false);
            break;
        case StreamlineAttributes::ID_streamlineDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(streamAtts->GetStreamlineDirection()) );
            directionType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_relTol:
            temp.setNum(streamAtts->GetRelTol());
            relTol->setText(temp);
            break;
        case StreamlineAttributes::ID_absTol:
            temp.setNum(streamAtts->GetAbsTol());
            absTol->setText(temp);
            break;
        case StreamlineAttributes::ID_terminationType:
            UpdateTerminationType();
            termType->blockSignals(true);
            termType->setCurrentIndex( int(streamAtts->GetTerminationType()) );
            termType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(streamAtts->GetIntegrationType());
            integrationType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_streamlineAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();

            slAlgo->blockSignals(true);
            slAlgo->setCurrentIndex(streamAtts->GetStreamlineAlgorithmType());
            slAlgo->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxStreamlineProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(streamAtts->GetMaxStreamlineProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(streamAtts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case StreamlineAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(streamAtts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case StreamlineAttributes::ID_pathlines:
          /*
            pathlineFlag->blockSignals(true);
            pathlineFlag->setChecked(streamAtts->GetPathlines());
            pathlineFlag->blockSignals(false);
          */
            break;
            
          case StreamlineAttributes::ID_legendMinFlag:
            legendMinToggle->blockSignals(true);
            legendMinToggle->setChecked(streamAtts->GetLegendMinFlag());
            
            legendMinEdit->setEnabled(streamAtts->GetLegendMinFlag());
            legendMinToggle->blockSignals(false);
            break;
          case StreamlineAttributes::ID_legendMaxFlag:
            legendMaxToggle->blockSignals(true);
            legendMaxToggle->setChecked(streamAtts->GetLegendMaxFlag());

            legendMaxEdit->setEnabled(streamAtts->GetLegendMaxFlag());
            legendMaxToggle->blockSignals(false);
            break;

          case StreamlineAttributes::ID_legendMin:
            temp.setNum(streamAtts->GetLegendMin());
            legendMinEdit->setText(temp);
            break;
          case StreamlineAttributes::ID_legendMax:
            temp.setNum(streamAtts->GetLegendMax());
            legendMaxEdit->setText(temp);
            break;

          case StreamlineAttributes::ID_displayBeginFlag:
            displayBeginToggle->blockSignals(true);
            displayBeginToggle->setChecked(streamAtts->GetDisplayBeginFlag());

            displayBeginEdit->setEnabled(streamAtts->GetDisplayBeginFlag());
            displayBeginToggle->blockSignals(false);
            break;

          case StreamlineAttributes::ID_displayEndFlag:
            displayEndToggle->blockSignals(true);
            displayEndToggle->setChecked(streamAtts->GetDisplayEndFlag());

            displayEndEdit->setEnabled(streamAtts->GetDisplayEndFlag());
            displayEndToggle->blockSignals(false);
            break;

          case StreamlineAttributes::ID_displayBegin:
            displayBeginEdit->setEnabled(streamAtts->GetDisplayBeginFlag());
            temp.setNum(streamAtts->GetDisplayBegin());
            displayBeginEdit->setText(temp);
            break;
          case StreamlineAttributes::ID_displayEnd:
            displayEndEdit->setEnabled(streamAtts->GetDisplayEndFlag());
            temp.setNum(streamAtts->GetDisplayEnd());
            displayEndEdit->setText(temp);
            break;

          case StreamlineAttributes::ID_opacityVariable:
            temp = streamAtts->GetOpacityVariable().c_str();
            opacityVar->setText(temp);
            break;
            
          case StreamlineAttributes::ID_opacity:
            opacitySlider->blockSignals(true);
            opacitySlider->setValue(int(streamAtts->GetOpacity() * 255.0));
            opacitySlider->blockSignals(false);
            break;

          case StreamlineAttributes::ID_opacityVarMin:
            temp.setNum(streamAtts->GetOpacityVarMin());
            opacityVarMin->setText(temp);
            break;

          case StreamlineAttributes::ID_opacityVarMax:
            temp.setNum(streamAtts->GetOpacityVarMax());
            opacityVarMax->setText(temp);
            break;
            
          case StreamlineAttributes::ID_opacityType:
            if (streamAtts->GetOpacityType() == StreamlineAttributes::None)
            {
                opacitySlider->hide();
                opacityVar->hide();
                opacityVarLabel->hide();
                opacityMinToggle->hide();
                opacityMaxToggle->hide();
                opacityVarMin->hide();
                opacityVarMax->hide();
            }
            else if (streamAtts->GetOpacityType() == StreamlineAttributes::Constant ||
                     streamAtts->GetOpacityType() == StreamlineAttributes::Ramp)
            {
                opacitySlider->show();
                opacityVar->hide();
                opacityVarLabel->hide();
                opacityMinToggle->hide();
                opacityMaxToggle->hide();
                opacityVarMin->hide();
                opacityVarMax->hide();
            }
            else if (streamAtts->GetOpacityType() == StreamlineAttributes::VariableRange)
            {
                opacitySlider->show();
                opacityVar->show();
                opacityVarLabel->show();
                opacityMinToggle->show();
                opacityMaxToggle->show();
                opacityVarMin->show();
                opacityVarMax->show();
            }
            
            opacityType->blockSignals(true);
            opacityType->setCurrentIndex(int(streamAtts->GetOpacityType()));
            opacityType->blockSignals(false);
            break;
            
            case StreamlineAttributes::ID_opacityVarMinFlag:
              opacityMinToggle->blockSignals(true);
              opacityMinToggle->setChecked(streamAtts->GetOpacityVarMinFlag());
              
              opacityVarMin->setEnabled(streamAtts->GetOpacityVarMinFlag());
              opacityMinToggle->blockSignals(false);
              break;

            case StreamlineAttributes::ID_opacityVarMaxFlag:
              opacityMaxToggle->blockSignals(true);
              opacityMaxToggle->setChecked(streamAtts->GetOpacityVarMaxFlag());
              
              opacityVarMax->setEnabled(streamAtts->GetOpacityVarMaxFlag());
              opacityMaxToggle->blockSignals(false);
              break;
        }
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateSourceAttributes
//
// Purpose: 
//   Updates the widgets for the various stream source types.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 17:22:05 PST 2002
//
// Modifications:
//   
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Hide/show useWholeBox.  Also enable/disable pointDensity if source type
//   is point.
//
//   Hank Childs, Sat May  2 22:05:56 PDT 2009
//   Add support for point lists.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateSourceAttributes()
{
    bool usePoint =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPoint;
    bool usePointList =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPointList;
    bool useLine  =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedLine;
    bool useCircle =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedCircle;
    bool usePlane =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPlane;
    bool useSphere =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedSphere;
    bool useBox =
      streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedBox;
    //
    // Update the point widgets.
    //
    pointSource->setEnabled(usePoint);
    pointSourceLabel->setEnabled(usePoint);
    if(usePoint)
    {
        pointSource->show();
        pointSourceLabel->show();
    }
    else
    {
        pointSource->hide();
        pointSourceLabel->hide();
    }

    //
    // Update the line widgets.
    //
    lineStart->setEnabled(useLine);
    lineEnd->setEnabled(useLine);
    lineStartLabel->setEnabled(useLine);
    lineEndLabel->setEnabled(useLine);
    if(useLine)
    {
        lineStart->show();
        lineEnd->show();
        lineStartLabel->show();
        lineEndLabel->show();
    }
    else
    {
        lineStart->hide();
        lineEnd->hide();
        lineStartLabel->hide();
        lineEndLabel->hide();
    }

    //
    // Update the plane widgets.
    //
    planeOrigin->setEnabled(usePlane || useCircle);
    planeNormal->setEnabled(usePlane || useCircle);
    planeUpAxis->setEnabled(usePlane || useCircle);
    planeRadius->setEnabled(usePlane || useCircle);
    planeOriginLabel->setEnabled(usePlane || useCircle);
    planeNormalLabel->setEnabled(usePlane || useCircle);
    planeUpAxisLabel->setEnabled(usePlane || useCircle);
    planeRadiusLabel->setEnabled(usePlane || useCircle);
    if(usePlane || useCircle)
    {
        planeOrigin->show();
        planeNormal->show();
        planeUpAxis->show();
        planeRadius->show();
        planeOriginLabel->show();
        planeNormalLabel->show();
        planeUpAxisLabel->show();
        planeRadiusLabel->show();
    }
    else
    {
        planeOrigin->hide();
        planeNormal->hide();
        planeUpAxis->hide();
        planeRadius->hide();
        planeOriginLabel->hide();
        planeNormalLabel->hide();
        planeUpAxisLabel->hide();
        planeRadiusLabel->hide();
    }

    //
    // Update the sphere widgets.
    //
    sphereOrigin->setEnabled(useSphere);
    sphereRadius->setEnabled(useSphere);
    sphereOriginLabel->setEnabled(useSphere);
    sphereRadiusLabel->setEnabled(useSphere);
    if(useSphere)
    {
        sphereOrigin->show();
        sphereRadius->show();
        sphereOriginLabel->show();
        sphereRadiusLabel->show();
    }
    else
    {
        sphereOrigin->hide();
        sphereRadius->hide();
        sphereOriginLabel->hide();
        sphereRadiusLabel->hide();
    }

    //
    // Update the box widgets
    //
    if(useBox)
    {
        useWholeBox->show();
    }
    else
        useWholeBox->hide();
    for(int i = 0; i < 3; ++i)
    {
        boxExtents[i]->setEnabled(useBox);
        boxExtentsLabel[i]->setEnabled(useBox);
        if(useBox)
        {
            boxExtents[i]->show();
            boxExtentsLabel[i]->show();
            if (streamAtts->GetUseWholeBox())
            {
                boxExtents[i]->setEnabled(false);
                boxExtentsLabel[i]->setEnabled(false);
            }
        }
        else
        {
            boxExtents[i]->hide();
            boxExtentsLabel[i]->hide();
        }
    }

    //
    // Update the point list widgets.
    //
    pointList->setEnabled(usePointList);
    pointListLabel->setEnabled(usePointList);
    pointListInfo->setEnabled(usePointList);
    if(usePointList)
    {
        pointList->show();
        pointListLabel->show();
        pointListInfo->show();
    }
    else
    {
        pointList->hide();
        pointListLabel->hide();
        pointListInfo->hide();
    }

    if (usePoint || usePointList)
    {
        pointDensity->setEnabled(false);
        pointDensityLabel->hide();
        pointDensity->hide();
    }
    else
    {
        pointDensity->setEnabled(true);
        pointDensityLabel->show();
        pointDensity->show();
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateIntegrationAttributes
//
// Purpose: 
//   Updates the widgets for the various integration types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// Modifications:
//
//   Dave Pugmire, Fri Aug 8 16:27:03 EDT 2008
//   Change the step label text based on the integration method.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateIntegrationAttributes()
{
    bool useDormandPrince = streamAtts->GetIntegrationType() == StreamlineAttributes::DormandPrince;
    bool useAdamsBashforth = streamAtts->GetIntegrationType() == StreamlineAttributes::AdamsBashforth;

    //Turn off everything.
    maxStepLength->hide();
    maxStepLengthLabel->hide();
    relTol->hide();
    relTolLabel->hide();
    absTol->hide();
    absTolLabel->hide();

    if ( useDormandPrince )
    {
        maxStepLength->show();
        maxStepLengthLabel->show();
        maxStepLengthLabel->setText(tr("Maximum step length"));
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
    }
    else if ( useAdamsBashforth )
    {
        maxStepLength->show();
        maxStepLengthLabel->show();
        maxStepLengthLabel->setText(tr("Step length"));
        absTol->show();
        absTolLabel->show();
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateTerminationType
//
// Purpose: 
//   Updates the widgets for the various termination types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Feb 19 12:35:38 EST 2008
//
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateTerminationType()
{
  /*
    if (streamAtts->GetTerminationType() == StreamlineAttributes::Time ||
        streamAtts->GetTerminationType() == StreamlineAttributes::Distance)
      {
        //termination->SetPrecision(5);
      }
    else
      {
      streamAtts->SetTermination( (int)streamAtts->GetTermination());
      }
  */
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateAlgorithmAttributes
//
// Purpose: 
//   Updates the widgets for the various integration types.
//
// Programmer: Dave Pugmire
// Creation:   Fri Aug 1 16:41:38 EDT 2008
//
//
// Modifications:
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::LoadOnDemand;
    bool useStaticDomains = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::ParallelStaticDomains;
    bool useMasterSlave = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::MasterSlave;
    
    //Turn off everything.
    maxDomainCacheLabel->hide();
    maxDomainCache->hide();
    maxSLCountLabel->hide();
    maxSLCount->hide();
    workGroupSizeLabel->hide();
    workGroupSize->hide();

    if ( useLoadOnDemand )
    {
        maxDomainCacheLabel->show();
        maxDomainCache->show();
    }
    else if ( useStaticDomains )
    {
        maxSLCountLabel->show();
        maxSLCount->show();
    }
    else if (useMasterSlave)
    {
        maxDomainCacheLabel->show();
        maxDomainCache->show();
        maxSLCountLabel->show();
        maxSLCount->show();
        workGroupSizeLabel->show();
        workGroupSize->show();
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 14:47:44 PST 2004
//   Changed tubeRadius to radius.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added support for useWholeBox and changed numbers.  Also fix a bug where
//   the point density is not getting updated correctly if you don't click
//   return.
//
//   Brad Whitlock, Wed Apr 23 11:49:55 PDT 2008
//   Support for internationalization.
//
//   Dave Pugmire, Mon Aug 4 2:49:38 EDT 2008
//   Added termination, algorithm and integration options.
//
//   Brad Whitlock, Wed Aug  6 15:34:13 PDT 2008
//   Use new methods.
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do stepLength
    if(which_widget == StreamlineAttributes::ID_maxStepLength || doAll)
    {
        double val;
        if(LineEditGetDouble(maxStepLength, val))
            streamAtts->SetMaxStepLength(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(streamAtts->GetMaxStepLength()));
            streamAtts->SetMaxStepLength(streamAtts->GetMaxStepLength());
        }
    }

    // Do termination
    if(which_widget == StreamlineAttributes::ID_termination || doAll)
    {
        double val;
        if(LineEditGetDouble(termination, val))
            streamAtts->SetTermination(val);
        else
        {
            ResettingError(tr("termination"),
                DoubleToQString(streamAtts->GetTermination()));
            streamAtts->SetTermination(streamAtts->GetTermination());
        }
    }

    // Do relTol
    if(which_widget == StreamlineAttributes::ID_relTol || doAll)
    {
        double val;
        if(LineEditGetDouble(relTol, val))
            streamAtts->SetRelTol(val);
        else
        {
            ResettingError(tr("relative tolerance"),
                DoubleToQString(streamAtts->GetRelTol()));
            streamAtts->SetRelTol(streamAtts->GetRelTol());
        }
    }

    // Do absTol
    if(which_widget == StreamlineAttributes::ID_absTol || doAll)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            streamAtts->SetAbsTol(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(streamAtts->GetAbsTol()));
            streamAtts->SetAbsTol(streamAtts->GetAbsTol());
        }
    }

    // Do pointSource
    if(which_widget == StreamlineAttributes::ID_pointSource || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(pointSource, val, 3))
            streamAtts->SetPointSource(val);
        else
        {
            ResettingError(tr("point source"),
                DoublesToQString(streamAtts->GetPointSource(), 3));
            streamAtts->SetPointSource(streamAtts->GetPointSource());
        }
    }

    // Do lineStart
    if(which_widget == StreamlineAttributes::ID_lineStart || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineStart, val, 3))
            streamAtts->SetLineStart(val);
        else
        {
            ResettingError(tr("line start"),
                DoublesToQString(streamAtts->GetLineStart(), 3));
            streamAtts->SetLineStart(streamAtts->GetLineStart());
        }
    }

    // Do lineEnd
    if(which_widget == StreamlineAttributes::ID_lineEnd || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineEnd, val, 3))
            streamAtts->SetLineEnd(val);
        else
        {
            ResettingError(tr("line end"),
                DoublesToQString(streamAtts->GetLineEnd(), 3));
            streamAtts->SetLineEnd(streamAtts->GetLineEnd());
        }
    }

    // Do planeOrigin
    if(which_widget == StreamlineAttributes::ID_planeOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeOrigin, val, 3))
            streamAtts->SetPlaneOrigin(val);
        else
        {
            ResettingError(tr("plane origin"),
                DoublesToQString(streamAtts->GetPlaneOrigin(), 3));
            streamAtts->SetPlaneOrigin(streamAtts->GetPlaneOrigin());
        }
    }

    // Do planeNormal
    if(which_widget == StreamlineAttributes::ID_planeNormal || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeNormal, val, 3))
            streamAtts->SetPlaneNormal(val);
        else
        {
            ResettingError(tr("plane normal"),
                DoublesToQString(streamAtts->GetPlaneNormal(), 3));
            streamAtts->SetPlaneNormal(streamAtts->GetPlaneNormal());
        }
    }

    // Do planeUpAxis
    if(which_widget == StreamlineAttributes::ID_planeUpAxis || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeUpAxis, val, 3))
            streamAtts->SetPlaneUpAxis(val);
        else
        {
            ResettingError(tr("plane up axis"),
                DoublesToQString(streamAtts->GetPlaneUpAxis(), 3));
            streamAtts->SetPlaneUpAxis(streamAtts->GetPlaneUpAxis());
        }
    }

    // Do planeRadius
    if(which_widget == StreamlineAttributes::ID_planeRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(planeRadius, val))
            streamAtts->SetPlaneRadius(val);
        else
        {
            ResettingError(tr("plane radius"),
                DoubleToQString(streamAtts->GetPlaneRadius()));
            streamAtts->SetPlaneRadius(streamAtts->GetPlaneRadius());
        }
    }

    // Do sphereOrigin
    if(which_widget == StreamlineAttributes::ID_sphereOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(sphereOrigin, val, 3))
            streamAtts->SetSphereOrigin(val);
        else
        {
            ResettingError(tr("sphere origin"),
                DoublesToQString(streamAtts->GetSphereOrigin(), 3));
            streamAtts->SetSphereOrigin(streamAtts->GetSphereOrigin());
        }
    }

    // Do sphereRadius
    if(which_widget == StreamlineAttributes::ID_sphereRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(sphereRadius, val))
            streamAtts->SetSphereRadius(val);
        else
        {
            ResettingError(tr("sphere radius"),
                DoubleToQString(streamAtts->GetSphereRadius()));
            streamAtts->SetSphereRadius(streamAtts->GetSphereRadius());
        }
    }

    // Do boxExtents
    if(which_widget == StreamlineAttributes::ID_boxExtents || doAll)
    {
        double d[6];
        bool allOkay = true;
        for(int i = 0; i < 3; ++i)
        {
            if(!LineEditGetDoubles(boxExtents[i], &d[i*2], 2))
                allOkay = false;
        }

        if(!allOkay)
        {
            Message(tr("The box extents contained errors so the previous "
                       "values will be used."));
            streamAtts->SelectBoxExtents();
        }
        else
            streamAtts->SetBoxExtents(d);
    }

    // Do pointList
    if(which_widget == StreamlineAttributes::ID_pointList || doAll)
    {
        double d[3];
        bool allOkay = true;
        std::vector<double> pointListV;
        if(!LineEditGetDoubles(pointList, pointListV))
            allOkay = false;
        bool multipleOf3Error = false;
        if ((pointListV.size() % 3) != 0)
        {
            allOkay = false;
            multipleOf3Error = true;
        }

        if(!allOkay)
        {
            if (multipleOf3Error)
                Message(tr("The number of values in the point list was not a multiple" 
                           " of three, so the previous values will be used."));
            else
                Message(tr("The point list contained errors so the previous "
                           "values will be used."));
            streamAtts->SelectPointList();
        }
        else
            streamAtts->SetPointList(pointListV);
    }

    // pointDensity
    if (which_widget == StreamlineAttributes::ID_pointDensity|| doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = pointDensity->value();
        if (val >= 1)
            streamAtts->SetPointDensity(val);
    }

    // Do radius
    if(which_widget == StreamlineAttributes::ID_tubeRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(tubeRadius, val))
            streamAtts->SetTubeRadius(val);
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(streamAtts->GetTubeRadius()));
            streamAtts->SetTubeRadius(streamAtts->GetTubeRadius());
        }
    }

    // Do radius
    if(which_widget == StreamlineAttributes::ID_ribbonWidth || doAll)
    {
        double val;
        if(LineEditGetDouble(ribbonWidth, val))
            streamAtts->SetRibbonWidth(val);
        else
        {
            ResettingError(tr("ribbon width"),
                DoubleToQString(streamAtts->GetRibbonWidth()));
            streamAtts->SetRibbonWidth(streamAtts->GetRibbonWidth());
        }
    }

    // maxStreamlineProcessCount
    if (which_widget == StreamlineAttributes::ID_maxStreamlineProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            streamAtts->SetMaxStreamlineProcessCount(val);
    }

    // workGroupSize
    if (which_widget == StreamlineAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            streamAtts->SetWorkGroupSize(val);
    }
    
    // Legend min
    if(which_widget == StreamlineAttributes::ID_legendMin || doAll)
    {
        double val;
        if(LineEditGetDouble(legendMinEdit, val))
            streamAtts->SetLegendMin(val);
        else
        {
            ResettingError(tr("Legend Min"),
                DoubleToQString(streamAtts->GetLegendMin()));
            streamAtts->SetLegendMin(streamAtts->GetLegendMin());
        }
    }
    // Legend max
    if(which_widget == StreamlineAttributes::ID_legendMax || doAll)
    {
        double val;
        if(LineEditGetDouble(legendMaxEdit, val))
            streamAtts->SetLegendMax(val);
        else
        {
            ResettingError(tr("Legend Max"),
                DoubleToQString(streamAtts->GetLegendMax()));
            streamAtts->SetLegendMax(streamAtts->GetLegendMax());
        }
    }
    // Display begin
    if(which_widget == StreamlineAttributes::ID_displayBegin || doAll)
    {
        double val;
        if (LineEditGetDouble(displayBeginEdit, val))
            streamAtts->SetDisplayBegin(val);
        else
        {
            ResettingError(tr("Display Begin"),
                           DoubleToQString(streamAtts->GetDisplayBegin()));
            streamAtts->SetDisplayBegin(streamAtts->GetDisplayBegin());
        }
    }
    // Display end
    if(which_widget == StreamlineAttributes::ID_displayEnd || doAll)
    {
        double val;
        if (LineEditGetDouble(displayEndEdit, val))
            streamAtts->SetDisplayEnd(val);
        else
        {
            ResettingError(tr("Display End"),
                           DoubleToQString(streamAtts->GetDisplayEnd()));
            streamAtts->SetDisplayEnd(streamAtts->GetDisplayEnd());
        }
    }

    // tubeDisplayDensity
    if (which_widget == StreamlineAttributes::ID_tubeDisplayDensity|| doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = tubeDisplayDensity->value();
        streamAtts->SetTubeDisplayDensity(val);
    }

    // seedRadius
    if(which_widget == StreamlineAttributes::ID_seedDisplayRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(seedRadius, val))
            streamAtts->SetSeedDisplayRadius(val);
        else
        {
            ResettingError(tr("Seed radius"),
                DoubleToQString(streamAtts->GetSeedDisplayRadius()));
            streamAtts->SetSeedDisplayRadius(streamAtts->GetSeedDisplayRadius());
        }
    }
    // headRadius
    if(which_widget == StreamlineAttributes::ID_headDisplayRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(headRadius, val))
            streamAtts->SetHeadDisplayRadius(val);
        else
        {
            ResettingError(tr("Head radius"),
                DoubleToQString(streamAtts->GetHeadDisplayRadius()));
            streamAtts->SetHeadDisplayRadius(streamAtts->GetHeadDisplayRadius());
        }
    }
    // headHeight
    if(which_widget == StreamlineAttributes::ID_headDisplayHeight || doAll)
    {
        double val;
        if(LineEditGetDouble(headHeight, val))
            streamAtts->SetHeadDisplayHeight(val);
        else
        {
            cout<<"val= "<<val<<endl;
            ResettingError(tr("Head height"),
                DoubleToQString(streamAtts->GetHeadDisplayHeight()));
            streamAtts->SetHeadDisplayHeight(streamAtts->GetHeadDisplayHeight());
        }
    }
    
    // opacityMin
    if(which_widget == StreamlineAttributes::ID_opacityVarMin || doAll)
    {
        double val;
        if(LineEditGetDouble(opacityVarMin, val))
            streamAtts->SetOpacityVarMin(val);
        else
        {
            ResettingError(tr("Opacity Min"),
                DoubleToQString(streamAtts->GetOpacityVarMin()));
            streamAtts->SetOpacityVarMin(streamAtts->GetOpacityVarMin());
        }
    }

    // opacityMax
    if(which_widget == StreamlineAttributes::ID_opacityVarMax || doAll)
    {
        double val;
        if(LineEditGetDouble(opacityVarMax, val))
            streamAtts->SetOpacityVarMax(val);
        else
        {
            ResettingError(tr("Opacity Max"),
                DoubleToQString(streamAtts->GetOpacityVarMax()));
            streamAtts->SetOpacityVarMax(streamAtts->GetOpacityVarMax());
        }
    }
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        streamAtts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        streamAtts->Notify();
}


//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    streamAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisStreamlinePlotWindow::sourceTypeChanged(int val)
{
    if(val != streamAtts->GetSourceType())
    {
        streamAtts->SetSourceType(StreamlineAttributes::SourceType(val));
        Apply();
    }
}

void
QvisStreamlinePlotWindow::directionTypeChanged(int val)
 {
    if(val != streamAtts->GetStreamlineDirection())
    {
        streamAtts->SetStreamlineDirection(StreamlineAttributes::IntegrationDirection(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::termTypeChanged(int val)
 {
    if(val != streamAtts->GetTerminationType())
    {
        streamAtts->SetTerminationType(StreamlineAttributes::TerminationType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::integrationTypeChanged(int val)
 {
    if(val != streamAtts->GetIntegrationType())
    {
        streamAtts->SetIntegrationType(StreamlineAttributes::IntegrationType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::streamlineAlgorithmChanged(int val)
{
    if(val != streamAtts->GetStreamlineAlgorithmType())
    {
        streamAtts->SetStreamlineAlgorithmType(StreamlineAttributes::StreamlineAlgorithmType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::maxStepLengthProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_maxStepLength);
    Apply();
}

void
QvisStreamlinePlotWindow::terminationProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_termination);
    Apply();
}

void
QvisStreamlinePlotWindow::relTolProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_relTol);
    Apply();
}

void
QvisStreamlinePlotWindow::lineStartProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_lineStart);
    Apply();
}

void
QvisStreamlinePlotWindow::lineEndProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_lineEnd);
    Apply();
}

void
QvisStreamlinePlotWindow::pointListProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_pointList);
    Apply();
}

void
QvisStreamlinePlotWindow::planeOriginProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeOrigin);
    Apply();
}

void
QvisStreamlinePlotWindow::planeNormalProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeNormal);
    Apply();
}

void
QvisStreamlinePlotWindow::planeUpAxisProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeUpAxis);
    Apply();
}

void
QvisStreamlinePlotWindow::planeRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereOriginProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sphereOrigin);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sphereRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::pointDensityChanged(int val)
{
    streamAtts->SetPointDensity(val);
    Apply();
}

void
QvisStreamlinePlotWindow::maxSLCountChanged(int val)
{
    streamAtts->SetMaxStreamlineProcessCount(val);
    Apply();
}

void
QvisStreamlinePlotWindow::maxDomainCacheChanged(int val)
{
    streamAtts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisStreamlinePlotWindow::workGroupSizeChanged(int val)
{
    streamAtts->SetWorkGroupSize(val);
    Apply();
}

void
QvisStreamlinePlotWindow::displayMethodChanged(int val)
{
    streamAtts->SetDisplayMethod((StreamlineAttributes::DisplayMethod)val);
    Apply();
}

void
QvisStreamlinePlotWindow::geomDisplayQualityChanged(int val)
{
    streamAtts->SetGeomDisplayQuality((StreamlineAttributes::DisplayQuality)val);
    Apply();
}

void
QvisStreamlinePlotWindow::opacityTypeChanged(int val)
{
    streamAtts->SetOpacityType((StreamlineAttributes::OpacityType)val);
    Apply();
}

void
QvisStreamlinePlotWindow::opacityVariableChanged(const QString &var)
{
    streamAtts->SetOpacityVariable(var.toStdString());
    Apply();
}

void
QvisStreamlinePlotWindow::opacityChanged(int opacity, const void*)
{
    streamAtts->SetOpacity((double)opacity/255.);
    Apply();
}

void
QvisStreamlinePlotWindow::showSeedsChanged(bool val)
{
    streamAtts->SetShowSeeds(val);
    Apply();
}

void
QvisStreamlinePlotWindow::showHeadsChanged(bool val)
{
    streamAtts->SetShowHeads(val);
    Apply();
}

void
QvisStreamlinePlotWindow::tubeRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_tubeRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::ribbonWidthProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_ribbonWidth);
    Apply();
}

void
QvisStreamlinePlotWindow::seedRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_seedDisplayRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::headRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_headDisplayRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::headHeightProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_headDisplayHeight);
    Apply();
}

void
QvisStreamlinePlotWindow::useWholeBoxChanged(bool val)
{
    streamAtts->SetUseWholeBox(val);
    Apply();
}


void
QvisStreamlinePlotWindow::boxExtentsProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_boxExtents);
    Apply();
}

void
QvisStreamlinePlotWindow::lineWidthChanged(int style)
{
    streamAtts->SetLineWidth(style);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::coloringMethodChanged(int val)
{
    streamAtts->SetColoringMethod((StreamlineAttributes::ColoringMethod)val);
    Apply();
}

void
QvisStreamlinePlotWindow::coloringVariableChanged(const QString &var)
{
    streamAtts->SetColoringVariable(var.toStdString());
    Apply();
}

void
QvisStreamlinePlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    streamAtts->SetColorTableName(ctName.toStdString());
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    streamAtts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::legendFlagChanged(bool val)
{
    streamAtts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::lightingFlagChanged(bool val)
{
    streamAtts->SetLightingFlag(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::pathlineFlagChanged(bool val)
{
    streamAtts->SetPathlines(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::absTolProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_absTol);
    Apply();
}

void
QvisStreamlinePlotWindow::pointSourceProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_pointSource);
    Apply();
}

void
QvisStreamlinePlotWindow::legendMaxToggled(bool val)
{
    streamAtts->SetLegendMaxFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::legendMinToggled(bool val)
{
    streamAtts->SetLegendMinFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::displayBeginToggled(bool val)
{
    streamAtts->SetDisplayBeginFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::displayEndToggled(bool val)
{
    streamAtts->SetDisplayEndFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::processMinLimitText()
{
    GetCurrentValues(StreamlineAttributes::ID_legendMin);
    Apply();
}

void
QvisStreamlinePlotWindow::processMaxLimitText()
{
    GetCurrentValues(StreamlineAttributes::ID_legendMax);
    Apply();
}

void
QvisStreamlinePlotWindow::opacityMinToggled(bool val)
{
    streamAtts->SetOpacityVarMinFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::opacityMaxToggled(bool val)
{
    streamAtts->SetOpacityVarMaxFlag(val);
    Apply();
}

void
QvisStreamlinePlotWindow::processOpacityVarMin()
{
    GetCurrentValues(StreamlineAttributes::ID_opacityVarMin);
    Apply();
}

void
QvisStreamlinePlotWindow::processOpacityVarMax()
{
    GetCurrentValues(StreamlineAttributes::ID_opacityVarMax);
    Apply();
}

void
QvisStreamlinePlotWindow::processDisplayBeginText()
{
    GetCurrentValues(StreamlineAttributes::ID_displayBegin);
    Apply();
}

void
QvisStreamlinePlotWindow::processDisplayEndText()
{
    GetCurrentValues(StreamlineAttributes::ID_displayEnd);
    Apply();
}

void
QvisStreamlinePlotWindow::seedDisplayQualityChanged(int val)
{
    streamAtts->SetGeomDisplayQuality(StreamlineAttributes::DisplayQuality(val));
    Apply();
}

void
QvisStreamlinePlotWindow::headDisplayTypeChanged(int val)
{
    streamAtts->SetHeadDisplayType(StreamlineAttributes::GeomDisplayType(val));
    Apply();
}

void
QvisStreamlinePlotWindow::tubeDisplayDensityChanged(int val)
{
    streamAtts->SetTubeDisplayDensity(val);
    Apply();
}
