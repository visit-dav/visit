/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <QPushButton>
#include <QTabWidget>
#include <QToolTip>
#include <QButtonGroup>
#include <QRadioButton>
#include <QvisColorTableButton.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
#include <QvisOpacitySlider.h>
#include <QListWidget>
#include <QFileDialog>
#include <stdio.h>
#include <iostream>

static void
TurnOn(QWidget *w0, QWidget *w1=NULL);
static void
TurnOff(QWidget *w0, QWidget *w1=NULL);

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
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Mon Jul 12 15:34:29 EDT 2010
//   Rename Exterior to Boundary.
//
//   Hank Childs, Wed Sep 29 20:22:36 PDT 2010
//   Add label and check box for whether we should restrict the maximum
//   time step length.
//
//   Hank Childs, Fri Oct  1 21:13:56 PDT 2010
//   Add size type for absTol.
//
//   Hank Childs, Oct  8 23:30:27 PDT 2010
//   Set up controls for multiple termination criteria.
//
//   Dave Pugmire, Tue Oct 19 13:52:00 EDT 2010
//   Add a delete all points button for the point list seed option.
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
    mainLayout->setMargin(5);

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
    sourceLayout->addWidget(sourceType, 0, 1, 1, 2);



    // Create the source geometry subgroup
    QGroupBox *geometryGroup = new QGroupBox(sourceGroup);
    sourceLayout->addWidget(geometryGroup, 1, 0, 4, 3);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
    geometryLayout->setRowStretch(5,10);

    int gRow = 0;

    // Create the widgets that specify a point source.
    pointSource = new QLineEdit(sourceGroup);
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(tr("Location"), sourceGroup);
    pointSourceLabel->setBuddy(pointSource);
    geometryLayout->addWidget(pointSourceLabel, gRow, 0);
    geometryLayout->addWidget(pointSource, gRow, 1);
    ++gRow;

    //Point list.
    pointList = new QListWidget(sourceGroup);
    geometryLayout->addWidget(pointList, gRow, 0);
    connect(pointList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(pointListDoubleClicked(QListWidgetItem*)));
    connect(pointList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(pointListClicked(QListWidgetItem*)));
    connect(pointList, SIGNAL(currentTextChanged(const QString&)), this, SLOT(textChanged(QString)));

    pointListReadPoints = new QPushButton(tr("Read Text File"), sourceGroup);
    geometryLayout->addWidget(pointListReadPoints, gRow, 1);
    connect(pointListReadPoints, SIGNAL(clicked()), this, SLOT(readPoints()));

    gRow++;

    pointListAddPoint = new QPushButton(tr("Add Point"), sourceGroup);
    pointListDelPoint = new QPushButton(tr("Delete Point"), sourceGroup);
    pointListDelAllPoints = new QPushButton(tr("Delete All Points"), sourceGroup);
    connect(pointListAddPoint, SIGNAL(clicked()), this, SLOT(addPoint()));
    connect(pointListDelPoint, SIGNAL(clicked()), this, SLOT(deletePoint()));
    connect(pointListDelAllPoints, SIGNAL(clicked()), this, SLOT(deletePoints()));
    geometryLayout->addWidget(pointListAddPoint, gRow, 0);
    geometryLayout->addWidget(pointListDelPoint, gRow, 1);
    geometryLayout->addWidget(pointListDelAllPoints, gRow, 2);
    gRow++;

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

    // Create the widgets that specify a sphere source.
    sphereOrigin = new QLineEdit(sourceGroup);
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(tr("Origin"), sourceGroup);
    sphereOriginLabel->setBuddy(sphereOrigin);
    geometryLayout->addWidget(sphereOriginLabel,gRow,0);
    geometryLayout->addWidget(sphereOrigin, gRow,1);
    ++gRow;

    radius = new QLineEdit(sourceGroup);
    connect(radius, SIGNAL(returnPressed()), this, SLOT(radiusProcessText()));
    radiusLabel = new QLabel(tr("Radius"), sourceGroup);
    radiusLabel->setBuddy(radius);
    geometryLayout->addWidget(radiusLabel,gRow,0);
    geometryLayout->addWidget(radius, gRow,1);
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
    gRow++;

    //Sampling options.
    samplingGroup = new QGroupBox(sourceGroup);
    samplingGroup->setTitle(tr("Sampling"));
    sourceLayout->addWidget(samplingGroup, gRow, 0, 1, 5);
    QGridLayout *samplingLayout = new QGridLayout(samplingGroup);
    samplingLayout->setMargin(5);
    samplingLayout->setSpacing(10);
    samplingLayout->setRowStretch(5,10);
    gRow++;
    
    int sRow = 0;
    fillLabel = new QLabel(tr("Sampling along:"), samplingGroup);
    samplingLayout->addWidget(fillLabel, sRow, 0);
    fillButtonGroup = new QButtonGroup(samplingGroup);
    fillButtons[0] = new QRadioButton(tr("Boundary"), samplingGroup);
    fillButtons[1] = new QRadioButton(tr("Interior"), samplingGroup);
    fillButtons[0]->setChecked(true);
    fillButtonGroup->addButton(fillButtons[0], 0);
    fillButtonGroup->addButton(fillButtons[1], 1);
    samplingLayout->addWidget(fillButtons[0], sRow, 1);
    samplingLayout->addWidget(fillButtons[1], sRow, 2);
    connect(fillButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(fillChanged(int)));
    sRow++;
    
    randomSamples = new QCheckBox(tr("Random"), samplingGroup);
    connect(randomSamples, SIGNAL(toggled(bool)),this, SLOT(randomSamplesChanged(bool)));
    samplingLayout->addWidget(randomSamples, sRow, 0);

    numberOfRandomSamplesLabel = new QLabel(tr("Number of Samples"), samplingGroup);
    samplingLayout->addWidget(numberOfRandomSamplesLabel, sRow, 1);
    numberOfRandomSamples = new QSpinBox(samplingGroup);
    numberOfRandomSamples->setMinimum(1);
    numberOfRandomSamples->setMaximum(100000000);
    connect(numberOfRandomSamples, SIGNAL(valueChanged(int)), this, SLOT(numberOfRandomSamplesChanged(int)));
    samplingLayout->addWidget(numberOfRandomSamples, sRow, 2);

    randomSeedLabel = new QLabel(tr("Seed"), samplingGroup);
    samplingLayout->addWidget(randomSeedLabel, sRow, 3);
    randomSeed = new QSpinBox(samplingGroup);
    randomSeed->setMinimum(0);
    randomSeed->setMaximum(100000000);
    connect(randomSeed, SIGNAL(valueChanged(int)), this, SLOT(randomSeedChanged(int)));
    samplingLayout->addWidget(randomSeed, sRow, 4);
    randomSeedLabel->setBuddy(randomSeed);

    sRow++;    

    sampleDensityLabel[0] = new QLabel(tr("Sample density 0"), samplingGroup);
    sampleDensityLabel[1] = new QLabel(tr("Sample density 1"), samplingGroup);
    sampleDensityLabel[2] = new QLabel(tr("Sample density 2"), samplingGroup);
    sampleDensity[0] = new QSpinBox(samplingGroup);
    sampleDensity[1] = new QSpinBox(samplingGroup);
    sampleDensity[2] = new QSpinBox(samplingGroup);
    sampleDensity[0]->setMinimum(1);
    sampleDensity[0]->setMaximum(10000000);
    sampleDensity[1]->setMinimum(1);
    sampleDensity[1]->setMaximum(10000000);
    sampleDensity[2]->setMinimum(1);
    sampleDensity[1]->setMaximum(10000000);
    connect(sampleDensity[0], SIGNAL(valueChanged(int)), this, SLOT(sampleDensity0Changed(int)));
    connect(sampleDensity[1], SIGNAL(valueChanged(int)), this, SLOT(sampleDensity1Changed(int)));
    connect(sampleDensity[2], SIGNAL(valueChanged(int)), this, SLOT(sampleDensity2Changed(int)));
    samplingLayout->addWidget(sampleDensityLabel[0], sRow, 0);
    samplingLayout->addWidget(sampleDensity[0], sRow, 1);
    samplingLayout->addWidget(sampleDensityLabel[1], sRow, 2);
    samplingLayout->addWidget(sampleDensity[1], sRow, 3);
    samplingLayout->addWidget(sampleDensityLabel[2], sRow, 4);
    samplingLayout->addWidget(sampleDensity[2], sRow, 5);
    sRow++;

    sampleDistanceLabel[0] = new QLabel(tr("Sample distance 0"), samplingGroup);
    sampleDistanceLabel[1] = new QLabel(tr("Sample distance 1"), samplingGroup);
    sampleDistanceLabel[2] = new QLabel(tr("Sample distance 2"), samplingGroup);
    sampleDistance[0] = new QLineEdit(samplingGroup);
    sampleDistance[1] = new QLineEdit(samplingGroup);
    sampleDistance[2] = new QLineEdit(samplingGroup);
    connect(sampleDistance[0], SIGNAL(returnPressed()), this, SLOT(sampleDistance0ProcessText()));
    connect(sampleDistance[1], SIGNAL(returnPressed()), this, SLOT(sampleDistance1ProcessText()));
    connect(sampleDistance[2], SIGNAL(returnPressed()), this, SLOT(sampleDistance2ProcessText()));
    samplingLayout->addWidget(sampleDistanceLabel[0], sRow, 0);
    samplingLayout->addWidget(sampleDistance[0], sRow, 1);
    samplingLayout->addWidget(sampleDistanceLabel[1], sRow, 2);
    samplingLayout->addWidget(sampleDistance[1], sRow, 3);
    samplingLayout->addWidget(sampleDistanceLabel[2], sRow, 4);
    samplingLayout->addWidget(sampleDistance[2], sRow, 5);
    sRow++;

    // Create the termination group box.
    QGroupBox *terminationGroup = new QGroupBox(central);
    terminationGroup->setTitle(tr("Termination"));
    mainLayout->addWidget(terminationGroup, 6, 0, 2, 2);
//    mainLayout->setStretchFactor(terminationGroup, 100);
    QGridLayout *terminationLayout = new QGridLayout(terminationGroup);
    terminationLayout->setMargin(5);
    terminationLayout->setSpacing(10);

    QLabel *maxStepsLabel = new QLabel(tr("Maximum number of steps"), terminationGroup);
    terminationLayout->addWidget(maxStepsLabel, 0,1);
    maxSteps = new QLineEdit(central);
    connect(maxSteps, SIGNAL(returnPressed()),
            this, SLOT(maxStepsProcessText()));
    terminationLayout->addWidget(maxSteps, 0,2);

    limitMaxDistance = new QCheckBox(central);
    connect(limitMaxDistance, SIGNAL(toggled(bool)), this, SLOT(limitMaxDistanceChanged(bool)));
    terminationLayout->addWidget(limitMaxDistance, 1,0);
    QLabel *limitMaxDistanceLabel = new QLabel(tr("Limit maximum distance traveled by particles"), terminationGroup);
    terminationLayout->addWidget(limitMaxDistanceLabel, 1,1);
    maxDistance = new QLineEdit(central);
    connect(maxDistance, SIGNAL(returnPressed()), this, SLOT(maxDistanceProcessText()));
    terminationLayout->addWidget(maxDistance, 1,2);

    limitMaxTime = new QCheckBox(central);
    connect(limitMaxTime, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeChanged(bool)));
    terminationLayout->addWidget(limitMaxTime, 2,0);
    QLabel *limitMaxTimeLabel = new QLabel(tr("Limit maximum time elapsed for particles"), terminationGroup);
    terminationLayout->addWidget(limitMaxTimeLabel, 2,1);
    maxTime = new QLineEdit(central);
    connect(maxTime, SIGNAL(returnPressed()), this, SLOT(maxTimeProcessText()));
    terminationLayout->addWidget(maxTime, 2,2);

    //Create the direction of integration.
    terminationLayout->addWidget(new QLabel(tr("Streamline direction"), central),3,1, Qt::AlignRight);
    directionType = new QComboBox(central);
    directionType->addItem(tr("Forward"));
    directionType->addItem(tr("Backward"));
    directionType->addItem(tr("Both"));
    connect(directionType, SIGNAL(activated(int)),
            this, SLOT(directionTypeChanged(int)));
    terminationLayout->addWidget(directionType, 3,2);

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
    integrationType->addItem(tr("M3D-C1 Integrator"));
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

    limitMaxTimeStepLabel = new QLabel(tr("Limit maximum time step"), integrationGroup);
    limitMaxTimeStep = new QCheckBox(integrationGroup);
    connect(limitMaxTimeStep, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeStepChanged(bool)));
    integrationLayout->addWidget(limitMaxTimeStepLabel, 2,0);
    integrationLayout->addWidget(limitMaxTimeStep, 2, 1);
    
    // Create the step length text field.
    maxTimeStepLabel = new QLabel(tr("Maximum time step"), integrationGroup);
    maxTimeStep = new QLineEdit(integrationGroup);
    connect(maxTimeStep, SIGNAL(returnPressed()),
            this, SLOT(maxTimeStepProcessText()));
    integrationLayout->addWidget(maxTimeStepLabel, 3,0);
    integrationLayout->addWidget(maxTimeStep, 3,1);

    // Create the relative tolerance text field.
    relTolLabel = new QLabel(tr("Relative tolerance"), integrationGroup);
    relTol = new QLineEdit(integrationGroup);
    connect(relTol, SIGNAL(returnPressed()),
            this, SLOT(relTolProcessText()));
    integrationLayout->addWidget(relTolLabel, 4,0);
    integrationLayout->addWidget(relTol, 4, 1);

    // Create the absolute tolerance text field.
    absTolLabel = new QLabel(tr("Absolute tolerance"), integrationGroup);
    absTol = new QLineEdit(integrationGroup);
    connect(absTol, SIGNAL(returnPressed()), this, SLOT(absTolProcessText()));
    integrationLayout->addWidget(absTolLabel, 5,0);
    integrationLayout->addWidget(absTol, 5, 1);

    absTolSizeType = new QComboBox(integrationGroup);
    absTolSizeType->addItem(tr("Absolute"), 0);
    absTolSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(absTolSizeType, SIGNAL(activated(int)), this, SLOT(absTolSizeTypeChanged(int)));
    integrationLayout->addWidget(absTolSizeType, 5, 2);

    forceNodalLabel = new QLabel(tr("Force node centering"), integrationGroup);
    forceNodal = new QCheckBox(integrationGroup);
    connect(forceNodal, SIGNAL(toggled(bool)), this, SLOT(forceNodalChanged(bool)));
    integrationLayout->addWidget(forceNodalLabel, 6,0);
    integrationLayout->addWidget(forceNodal, 6, 1);

    // ----------------------------------------------------------------------
    // Appearance tab
    // ----------------------------------------------------------------------
    QWidget *appearanceTab = new QWidget(central);
    propertyTabs->addTab(appearanceTab, tr("Appearance"));
    CreateAppearanceTab(appearanceTab);

    // ----------------------------------------------------------------------
    // Parallel tab
    // ----------------------------------------------------------------------
    QWidget *advancedTab = new QWidget(central);
    propertyTabs->addTab(advancedTab, tr("Advanced"));
    CreateAdvancedTab(advancedTab);
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
//   Dave Pugmire, Tue Apr  6 08:15:33 EDT 2010
//   Rearrange the color by variable widgets.
//
//   Hank Childs, Wed Sep 29 19:12:39 PDT 2010
//   Rename None to FullyOpaque.
//
//   Hank Childs, Oct  8 23:30:27 PDT 2010
//   Set up controls for multiple termination criteria.
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


    coloringVar = new QvisVariableButton(false, true, true,
                                         QvisVariableButton::Scalars,
                                         dataGroup);
    dataLayout->addWidget(coloringVar, 0, 2);
    connect(coloringVar, SIGNAL(activated(const QString &)),
            this, SLOT(coloringVariableChanged(const QString&)));

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
    drawLayout->addWidget(new QLabel(tr("  "), displayGrp), 0, 2, 1, 2);

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
    geomRadiusLabel = new QLabel(tr("Radius"), displayGrp);
    geomRadiusLabel->setBuddy(tubeRadius);
    geomRadiusLabel->setToolTip(tr("Radius used for tubes and ribbons."));
    drawLayout->addWidget(geomRadiusLabel, 1, 0, Qt::AlignRight);
    drawLayout->addWidget(tubeRadius, 1, 1);
    drawLayout->addWidget(ribbonWidth, 1, 1);

    tubeSizeType = new QComboBox(displayGrp);
    tubeSizeType->addItem(tr("Absolute"), 0);
    tubeSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(tubeSizeType, SIGNAL(activated(int)), this, SLOT(tubeSizeTypeChanged(int)));
    drawLayout->addWidget(tubeSizeType, 1, 2);

    ribbonSizeType = new QComboBox(displayGrp);
    ribbonSizeType->addItem(tr("Absolute"), 0);
    ribbonSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(ribbonSizeType, SIGNAL(activated(int)), this, SLOT(ribbonSizeTypeChanged(int)));
    drawLayout->addWidget(ribbonSizeType, 1, 2);

    tubeDisplayDensity = new QSpinBox(displayGrp);
    tubeDisplayDensity->setMinimum(2);
    tubeDisplayDensity->setMaximum(100);
    tubeDisplayDensityLabel = new QLabel(tr("Display density"), displayGrp);
    connect(tubeDisplayDensity, SIGNAL(valueChanged(int)), this, SLOT(tubeDisplayDensityChanged(int)));
    drawLayout->addWidget(tubeDisplayDensityLabel, 0, 2);
    drawLayout->addWidget(tubeDisplayDensity, 0, 3);


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

    seedSizeType = new QComboBox(displayGrp);
    seedSizeType->addItem(tr("Absolute"), 0);
    seedSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(seedSizeType, SIGNAL(activated(int)), this, SLOT(seedSizeTypeChanged(int)));
    showLayout->addWidget(seedSizeType, 0, 3);

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
    connect(headHeight, SIGNAL(returnPressed()), this, SLOT(headHeightRatioProcessText()));
    headHeightLabel = new QLabel(tr("Height:Radius Ratio"), displayGrp);
    headHeightLabel->setBuddy(headHeight);
    headHeightLabel->setToolTip(tr("Height for head point display."));
    showLayout->addWidget(headRadiusLabel, 2, 1, Qt::AlignRight);
    showLayout->addWidget(headRadius, 2, 2);
    showLayout->addWidget(headHeightLabel, 3, 1);
    showLayout->addWidget(headHeight, 3, 2);

    headSizeType = new QComboBox(displayGrp);
    headSizeType->addItem(tr("Absolute"), 0);
    headSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(headSizeType, SIGNAL(activated(int)), this, SLOT(headSizeTypeChanged(int)));
    showLayout->addWidget(headSizeType, 2, 3);

    geomDisplayQuality = new QComboBox(displayGrp);
    geomDisplayQuality->addItem(tr("Low"), 0);
    geomDisplayQuality->addItem(tr("Medium"), 1);
    geomDisplayQuality->addItem(tr("High"), 2);
    geomDisplayQuality->addItem(tr("Super"), 3);
    geomDisplayQualityLabel = new QLabel(tr("Display quality"), displayGrp);
    connect(geomDisplayQuality, SIGNAL(activated(int)), this, SLOT(geomDisplayQualityChanged(int)));
    showLayout->addWidget(geomDisplayQualityLabel, 4, 0);
    showLayout->addWidget(geomDisplayQuality, 4, 1);

    // Create the display group
    QGroupBox *cropGrp = new QGroupBox(pageAppearance);
    cropGrp->setTitle(tr("Crop away portion of streamlines (for animations)"));
    appearanceLayout->addWidget(cropGrp);

    QGridLayout *cropLayout = new QGridLayout(cropGrp);
    cropLayout->setMargin(5);
    cropLayout->setSpacing(10);

    displayBeginToggle = new QCheckBox(tr("Retain from"), cropGrp);
    displayEndToggle = new QCheckBox(tr("To"), cropGrp);
    connect(displayBeginToggle, SIGNAL(toggled(bool)), this, SLOT(displayBeginToggled(bool)));
    connect(displayEndToggle, SIGNAL(toggled(bool)), this, SLOT(displayEndToggled(bool)));

    displayBeginEdit = new QLineEdit(cropGrp);
    displayEndEdit = new QLineEdit(cropGrp);
    connect(displayBeginEdit, SIGNAL(returnPressed()), this, SLOT(processDisplayBeginText()));
    connect(displayEndEdit, SIGNAL(returnPressed()), this, SLOT(processDisplayEndText()));

    displayReferenceType = new QComboBox(cropGrp);
    displayReferenceType->addItem(tr("Distance"));
    displayReferenceType->addItem(tr("Time"));
    displayReferenceType->addItem(tr("Step numbers"));
    connect(displayReferenceType, SIGNAL(activated(int)), this, SLOT(displayReferenceTypeChanged(int)));
    QLabel *drtl = new QLabel(tr("Units are in"), cropGrp);

    cropLayout->addWidget(displayBeginToggle, 0, 0);
    cropLayout->addWidget(displayBeginEdit, 0, 1);
    cropLayout->addWidget(displayEndToggle, 0, 2);
    cropLayout->addWidget(displayEndEdit, 0, 3);
    cropLayout->addWidget(drtl, 1, 0);
    cropLayout->addWidget(displayReferenceType, 1, 1);

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
    opacityType->addItem(tr("Fully Opaque"),0);
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
//   Hank Childs, Wed Sep 29 19:25:06 PDT 2010
//   Add option for having VisIt select the best algorithm.
//
//   Hank Childs, Oct  8 23:30:27 PDT 2010
//   Set up controls for multiple termination criteria.
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
    slAlgo->addItem(tr("Have VisIt select the best algorithm"));
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

    // Warnings group.
    QGroupBox *warningsGrp = new QGroupBox(pageAdvanced);
    warningsGrp->setTitle(tr("Warnings"));
    advGLayout->addWidget(warningsGrp, 1, 0, 1, 4);

    QGridLayout *warningsGLayout = new QGridLayout(warningsGrp);
    warningsGLayout->setSpacing(10);
    warningsGLayout->setColumnStretch(1,10);

    issueWarningForMaxSteps = new QCheckBox(central);
    connect(issueWarningForMaxSteps, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForMaxStepsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForMaxSteps, 0, 0);
    QLabel *warningsLabel = new QLabel(tr("Issue warnings when the maximum number of steps is reached"), warningsGrp);
    warningsGLayout->addWidget(warningsLabel, 0, 1, 1, 4);
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
//   Hank Childs, Wed Sep 29 19:12:39 PDT 2010
//   Rename None to FullyOpaque.
//
//   Hank Childs, Wed Sep 29 20:22:36 PDT 2010
//   Add maxTimeStep.
//
//   Hank Childs, Thu Sep 30 12:07:14 PDT 2010
//   Support widgets for scaling based on fraction of the bounding box.
//
//   Hank Childs, Oct  8 23:30:27 PDT 2010
//   Set up controls for multiple termination criteria.
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
        case StreamlineAttributes::ID_limitMaximumTimestep:
            limitMaxTimeStep->blockSignals(true);
            limitMaxTimeStep->setChecked(streamAtts->GetLimitMaximumTimestep());
            limitMaxTimeStep->blockSignals(false);
            maxTimeStep->blockSignals(true);
            maxTimeStepLabel->blockSignals(true);
            if (streamAtts->GetIntegrationType() == StreamlineAttributes::DormandPrince
                && !streamAtts->GetLimitMaximumTimestep())
            {
                maxTimeStep->setEnabled(false);
                maxTimeStepLabel->setEnabled(false);
            }
            else
            {
                maxTimeStep->setEnabled(true);
                maxTimeStepLabel->setEnabled(true);
            }
            maxTimeStep->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxTimeStep:
            temp.setNum(streamAtts->GetMaxTimeStep());
            maxTimeStep->setText(temp);
            break;
        case StreamlineAttributes::ID_maxSteps:
            temp.setNum(streamAtts->GetMaxSteps());
            maxSteps->setText(temp);
            break;
        case StreamlineAttributes::ID_terminateByDistance:
            limitMaxDistance->blockSignals(true);
            limitMaxDistance->setChecked(streamAtts->GetTerminateByDistance());
            limitMaxDistance->blockSignals(false);
            maxDistance->setEnabled(streamAtts->GetTerminateByDistance());
            break;
        case StreamlineAttributes::ID_termDistance:
            temp.setNum(streamAtts->GetTermDistance());
            maxDistance->setText(temp);
            break;
        case StreamlineAttributes::ID_terminateByTime:
            limitMaxTime->blockSignals(true);
            limitMaxTime->setChecked(streamAtts->GetTerminateByTime());
            limitMaxTime->blockSignals(false);
            maxTime->setEnabled(streamAtts->GetTerminateByTime());
            break;
        case StreamlineAttributes::ID_termTime:
            temp.setNum(streamAtts->GetTermTime());
            maxTime->setText(temp);
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
        case StreamlineAttributes::ID_radius:
            temp.setNum(streamAtts->GetRadius());
            radius->setText(temp);
            break;
        case StreamlineAttributes::ID_sphereOrigin:
            sphereOrigin->setText(DoublesToQString(streamAtts->GetSphereOrigin(),3));
            break;
        case StreamlineAttributes::ID_pointList:
            {
                std::vector<double> points = streamAtts->GetPointList();

                pointList->clear();
                for (int i = 0; i < points.size(); i+= 3)
                {
                    char tmp[256];
                    sprintf(tmp, "%lf %lf %lf", points[i], points[i+1], points[i+2]);
                    QString str = tmp;
                    QListWidgetItem *item = new QListWidgetItem(str, pointList);
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    pointList->setCurrentItem(item);
                }

                break;
            }
            
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

        case StreamlineAttributes::ID_sampleDensity0:
            sampleDensity[0]->blockSignals(true);
            sampleDensity[0]->setValue(streamAtts->GetSampleDensity0());
            sampleDensity[0]->blockSignals(false);
            break;
        case StreamlineAttributes::ID_sampleDensity1:
            sampleDensity[1]->blockSignals(true);
            sampleDensity[1]->setValue(streamAtts->GetSampleDensity1());
            sampleDensity[1]->blockSignals(false);
            break;
        case StreamlineAttributes::ID_sampleDensity2:
            sampleDensity[2]->blockSignals(true);
            sampleDensity[2]->setValue(streamAtts->GetSampleDensity2());
            sampleDensity[2]->blockSignals(false);
            break;

        case StreamlineAttributes::ID_sampleDistance0:
            temp.setNum(streamAtts->GetSampleDistance0());
            sampleDistance[0]->setText(temp);
            break;
        case StreamlineAttributes::ID_sampleDistance1:
            temp.setNum(streamAtts->GetSampleDistance1());
            sampleDistance[1]->setText(temp);
            break;
        case StreamlineAttributes::ID_sampleDistance2:
            temp.setNum(streamAtts->GetSampleDistance2());
            sampleDistance[2]->setText(temp);
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
                tubeSizeType->hide();
                ribbonSizeType->hide();
                geomRadiusLabel->hide();
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
                    tubeSizeType->show();
                    ribbonSizeType->hide();
                }
                if (streamAtts->GetDisplayMethod() == StreamlineAttributes::Ribbons)
                {
                    ribbonWidth->show();
                    tubeRadius->hide();
                    tubeSizeType->hide();
                    ribbonSizeType->show();
                }
                
                geomRadiusLabel->show();
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
            coloringVar->setText(temp);
          break;
          
        case StreamlineAttributes::ID_showSeeds:
            seedRadius->setEnabled(streamAtts->GetShowSeeds());
            seedRadiusLabel->setEnabled(streamAtts->GetShowSeeds());
            seedSizeType->setEnabled(streamAtts->GetShowSeeds());
            geomDisplayQuality->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());
            geomDisplayQualityLabel->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());

            showSeeds->blockSignals(true);
            showSeeds->setChecked(streamAtts->GetShowSeeds());
            showSeeds->blockSignals(false);
            break;

        case StreamlineAttributes::ID_showHeads:
            headRadius->setEnabled(streamAtts->GetShowHeads());
            headRadiusLabel->setEnabled(streamAtts->GetShowHeads());
            headSizeType->setEnabled(streamAtts->GetShowHeads());
            headDisplayType->setEnabled(streamAtts->GetShowHeads());
            headHeight->setEnabled(streamAtts->GetShowHeads() && streamAtts->GetHeadDisplayType() == StreamlineAttributes::Cone);
            headHeightLabel->setEnabled(streamAtts->GetShowHeads() && streamAtts->GetHeadDisplayType() == StreamlineAttributes::Cone);
            geomDisplayQuality->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());
            geomDisplayQualityLabel->setEnabled(streamAtts->GetShowSeeds()||streamAtts->GetShowHeads());

            showHeads->blockSignals(true);
            showHeads->setChecked(streamAtts->GetShowHeads());
            showHeads->blockSignals(false);
            break;
            
        case StreamlineAttributes::ID_seedRadiusSizeType:
            seedSizeType->blockSignals(true);
            seedSizeType->setCurrentIndex((int) streamAtts->GetSeedRadiusSizeType());
            seedSizeType->blockSignals(false);
            if (streamAtts->GetSeedRadiusSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetSeedRadiusAbsolute());
                seedRadius->setText(temp);
            }
            else
            {
                temp.setNum(streamAtts->GetSeedRadiusBBox());
                seedRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_seedRadiusAbsolute:
            if (streamAtts->GetSeedRadiusSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetSeedRadiusAbsolute());
                seedRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_seedRadiusBBox:
            if (streamAtts->GetSeedRadiusSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetSeedRadiusBBox());
                seedRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_headRadiusSizeType:
            headSizeType->blockSignals(true);
            headSizeType->setCurrentIndex((int) streamAtts->GetHeadRadiusSizeType());
            headSizeType->blockSignals(false);
            if (streamAtts->GetHeadRadiusSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetHeadRadiusAbsolute());
                headRadius->setText(temp);
            }
            else
            {
                temp.setNum(streamAtts->GetHeadRadiusBBox());
                headRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_headRadiusAbsolute:
            if (streamAtts->GetHeadRadiusSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetHeadRadiusAbsolute());
                headRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_headRadiusBBox:
            if (streamAtts->GetHeadRadiusSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetHeadRadiusBBox());
                headRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_headHeightRatio:
            temp.setNum(streamAtts->GetHeadHeightRatio());
            headHeight->setText(temp);
            break;

        case StreamlineAttributes::ID_tubeSizeType:
            tubeSizeType->blockSignals(true);
            tubeSizeType->setCurrentIndex((int) streamAtts->GetTubeSizeType());
            tubeSizeType->blockSignals(false);
            if (streamAtts->GetTubeSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetTubeRadiusAbsolute());
                tubeRadius->setText(temp);
            }
            else
            {
                temp.setNum(streamAtts->GetTubeRadiusBBox());
                tubeRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_tubeRadiusAbsolute:
            if (streamAtts->GetTubeSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetTubeRadiusAbsolute());
                tubeRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_tubeRadiusBBox:
            if (streamAtts->GetTubeSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetTubeRadiusBBox());
                tubeRadius->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_ribbonWidthSizeType:
            ribbonSizeType->blockSignals(true);
            ribbonSizeType->setCurrentIndex((int) streamAtts->GetRibbonWidthSizeType());
            ribbonSizeType->blockSignals(false);
            if (streamAtts->GetRibbonWidthSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetRibbonWidthAbsolute());
                ribbonWidth->setText(temp);
            }
            else
            {
                temp.setNum(streamAtts->GetRibbonWidthBBox());
                ribbonWidth->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_ribbonWidthAbsolute:
            if (streamAtts->GetRibbonWidthSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetRibbonWidthAbsolute());
                ribbonWidth->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_ribbonWidthBBox:
            if (streamAtts->GetRibbonWidthSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetRibbonWidthBBox());
                ribbonWidth->setText(temp);
            }
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
                coloringVar->setEnabled(true);
                coloringVar->show();
            }
            else
            {
                coloringVar->setEnabled(false);
                coloringVar->hide();
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
        case StreamlineAttributes::ID_absTolSizeType:
            absTolSizeType->blockSignals(true);
            absTolSizeType->setCurrentIndex((int) streamAtts->GetAbsTolSizeType());
            absTolSizeType->blockSignals(false);
            if (streamAtts->GetAbsTolSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetAbsTolBBox());
                absTol->setText(temp);
            }
            if (streamAtts->GetAbsTolSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetAbsTolAbsolute());
                absTol->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_absTolBBox:
            if (streamAtts->GetAbsTolSizeType() == StreamlineAttributes::FractionOfBBox)
            {
                temp.setNum(streamAtts->GetAbsTolBBox());
                absTol->setText(temp);
            }
            break;
        case StreamlineAttributes::ID_absTolAbsolute:
            if (streamAtts->GetAbsTolSizeType() == StreamlineAttributes::Absolute)
            {
                temp.setNum(streamAtts->GetAbsTolAbsolute());
                absTol->setText(temp);
            }
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
          case StreamlineAttributes::ID_referenceTypeForDisplay:
            displayReferenceType->blockSignals(true);
            displayReferenceType->setCurrentIndex((int) streamAtts->GetReferenceTypeForDisplay());
            displayReferenceType->blockSignals(false);
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
            if (streamAtts->GetOpacityType() == StreamlineAttributes::FullyOpaque)
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

            case StreamlineAttributes::ID_randomSamples:
              randomSamples->blockSignals(true);
              randomSamples->setChecked(streamAtts->GetRandomSamples());
              UpdateSourceAttributes();
              randomSamples->blockSignals(false);
              break;

            case StreamlineAttributes::ID_fillInterior:
              fillButtonGroup->blockSignals(true);
              fillButtonGroup->button(streamAtts->GetFillInterior()?1:0)->setChecked(true);
              UpdateSourceAttributes();
              fillButtonGroup->blockSignals(false);
              break;

            case StreamlineAttributes::ID_randomSeed:
              randomSeed->blockSignals(true);
              randomSeed->setValue(streamAtts->GetRandomSeed());
              randomSeed->blockSignals(false);
              break;
              
            case StreamlineAttributes::ID_numberOfRandomSamples:
              numberOfRandomSamples->blockSignals(true);
              numberOfRandomSamples->setValue(streamAtts->GetNumberOfRandomSamples());
              numberOfRandomSamples->blockSignals(false);
              break;

            case StreamlineAttributes::ID_forceNodeCenteredData:
              forceNodal->blockSignals(true);
              forceNodal->setChecked(streamAtts->GetForceNodeCenteredData());
              forceNodal->blockSignals(false);
              break;

            case StreamlineAttributes::ID_issueTerminationWarnings:
              issueWarningForMaxSteps->blockSignals(true);
              issueWarningForMaxSteps->setChecked(streamAtts->GetIssueTerminationWarnings());
              issueWarningForMaxSteps->blockSignals(false);
              break;
        }
    }
}

void
QvisStreamlinePlotWindow::TurnOffSourceAttributes()
{
    TurnOff(pointSource, pointSourceLabel);

    TurnOff(lineStart, lineStartLabel);
    TurnOff(lineEnd, lineEndLabel);

    TurnOff(planeOrigin, planeOriginLabel);
    TurnOff(planeNormal, planeNormalLabel);
    TurnOff(planeUpAxis, planeUpAxisLabel);

    TurnOff(sphereOrigin, sphereOriginLabel);

    TurnOff(radius, radiusLabel);

    TurnOff(useWholeBox);
    for (int i = 0; i < 3; i++)
        TurnOff(boxExtents[i], boxExtentsLabel[i]);

    TurnOff(pointList);
    TurnOff(pointListDelPoint);
    TurnOff(pointListAddPoint);
    TurnOff(pointListReadPoints);

    TurnOff(randomSamples);
    TurnOff(numberOfRandomSamples, numberOfRandomSamplesLabel);
    TurnOff(randomSeed, randomSeedLabel);
    TurnOff(fillLabel);
    TurnOff(fillButtons[0]);
    TurnOff(fillButtons[1]);
    for (int i = 0; i < 3; i++)
    {
        TurnOff(sampleDensity[i], sampleDensityLabel[i]);
        TurnOff(sampleDistance[i], sampleDistanceLabel[i]);
    }
    TurnOff(samplingGroup);
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
    TurnOffSourceAttributes();

    bool showSampling = false, enableRandom = false, enableFill = false;
    
    if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPoint)
        TurnOn(pointSource, pointSourceLabel);
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedLine)
    {
        TurnOn(lineStart, lineStartLabel);
        TurnOn(lineEnd, lineEndLabel);
        if (! streamAtts->GetRandomSamples())
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            sampleDensityLabel[0]->setText(tr("Samples along line:"));
            sampleDensity[0]->setMinimum(1);
        }

        enableRandom = true;
        showSampling = true;
    }
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPlane)
    {
        TurnOn(planeOrigin, planeOriginLabel);
        TurnOn(planeNormal, planeNormalLabel);
        TurnOn(planeUpAxis, planeUpAxisLabel);
        TurnOn(sampleDistance[0], sampleDistanceLabel[0]);
        TurnOn(sampleDistance[1], sampleDistanceLabel[1]);
        sampleDistanceLabel[0]->setText(tr("Distance in X:"));
        sampleDistanceLabel[1]->setText(tr("Distance in Y:"));

        if (! streamAtts->GetRandomSamples())
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            TurnOn(sampleDensity[1], sampleDensityLabel[1]);
            sampleDensityLabel[0]->setText(tr("Samples in X:"));
            sampleDensityLabel[1]->setText(tr("Samples in Y:"));
            for (int i = 0; i < 2; i++)
                sampleDensity[i]->setMinimum(2);
        }
        enableRandom = true;
        enableFill = true;
        showSampling = true;
    }
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedCircle)
    {
        TurnOn(planeOrigin, planeOriginLabel);
        TurnOn(planeNormal, planeNormalLabel);
        TurnOn(planeUpAxis, planeUpAxisLabel);
        TurnOn(radius, radiusLabel);
        if (! streamAtts->GetRandomSamples())
        {
            if (streamAtts->GetFillInterior())
            {
                TurnOn(sampleDensity[0], sampleDensityLabel[0]);
                TurnOn(sampleDensity[1], sampleDensityLabel[1]);
                sampleDensityLabel[0]->setText(tr("Samples in Theta:"));
                sampleDensityLabel[1]->setText(tr("Samples in R:"));
            }
            else
            {
                TurnOn(sampleDensity[0], sampleDensityLabel[0]);
                sampleDensityLabel[0]->setText(tr("Samples in Theta:"));
            }
        }
        enableRandom = true;
        enableFill = true;
        showSampling = true;
    }
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedSphere)
    {
        TurnOn(sphereOrigin, sphereOriginLabel);
        TurnOn(radius, radiusLabel);
        
        if (! streamAtts->GetRandomSamples())
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            TurnOn(sampleDensity[1], sampleDensityLabel[1]);
            sampleDensityLabel[0]->setText(tr("Samples in Latitude:"));
            sampleDensityLabel[1]->setText(tr("Samples in Longitude:"));

            if (streamAtts->GetFillInterior())
            {
                TurnOn(sampleDensity[2], sampleDensityLabel[2]);
                sampleDensityLabel[2]->setText(tr("Samples in R:"));
            }
        }

        enableRandom = true;
        enableFill = true;
        showSampling = true;
    }
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedBox)
    {
        TurnOn(useWholeBox);
        for (int i = 0; i < 3; i++)
            TurnOn(boxExtents[i], boxExtentsLabel[i]);

        if (streamAtts->GetUseWholeBox())
        {
            for (int i = 0; i < 3; i++)
            {
                boxExtents[i]->setEnabled(false);
                boxExtentsLabel[i]->setEnabled(false);
            }
        }

        if (! streamAtts->GetRandomSamples())
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            TurnOn(sampleDensity[1], sampleDensityLabel[1]);
            TurnOn(sampleDensity[2], sampleDensityLabel[2]);
            sampleDensityLabel[0]->setText(tr("Samples in X:"));
            sampleDensityLabel[1]->setText(tr("Samples in Y:"));
            sampleDensityLabel[2]->setText(tr("Samples in Z:"));
            for (int i = 0; i < 3; i++)
                sampleDensity[i]->setMinimum(2);
        }

        enableRandom = true;
        enableFill = true;
        showSampling = true;
    }
    else if (streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPointList)
    {
        TurnOn(pointList);
        TurnOn(pointListDelPoint);
        TurnOn(pointListAddPoint);
        TurnOn(pointListReadPoints);
    }

    if (enableRandom)
    {
        TurnOn(randomSamples);
        TurnOn(randomSeed, randomSeedLabel);
        TurnOn(numberOfRandomSamples, numberOfRandomSamplesLabel);
        if (! streamAtts->GetRandomSamples())
        {
            randomSeed->setEnabled(false);
            randomSeedLabel->setEnabled(false);
            numberOfRandomSamples->setEnabled(false);
            numberOfRandomSamplesLabel->setEnabled(false);
        }
    }
    
    if (enableFill)
    {
        TurnOn(fillLabel);
        TurnOn(fillButtons[0]);
        TurnOn(fillButtons[1]);
    }
    if (showSampling)
        TurnOn(samplingGroup);
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
//   Hank Childs, Wed Sep 29 20:22:36 PDT 2010
//   Add support for limiting the maximum time step.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateIntegrationAttributes()
{
    //Turn off everything.
    maxStepLength->hide();
    maxStepLengthLabel->hide();
    limitMaxTimeStepLabel->hide();
    limitMaxTimeStep->hide();
    maxTimeStep->hide();
    maxTimeStepLabel->hide();
    relTol->hide();
    relTolLabel->hide();
    absTol->hide();
    absTolLabel->hide();

    switch( streamAtts->GetIntegrationType() )
    {
    case StreamlineAttributes::DormandPrince:
        limitMaxTimeStepLabel->show();
        limitMaxTimeStep->show();
        maxTimeStep->show();
        maxTimeStepLabel->show();
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
        break;

    case StreamlineAttributes::AdamsBashforth:
    case StreamlineAttributes::M3DC1Integrator:
        maxStepLength->show();
        maxStepLengthLabel->show();
        absTol->show();
        absTolLabel->show();
        break;
    }
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
//   Hank Childs, Wed Sep 29 20:44:18 PDT 2010
//   Add support for the max time step.
//
//   Hank Childs, Thu Sep 30 01:48:49 PDT 2010
//   Support widgets for fraction bbox vs absolute sizes.
//
//   Hank Childs, Oct  8 23:30:27 PDT 2010
//   Set up controls for multiple termination criteria.
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

    // Do max time step
    if(which_widget == StreamlineAttributes::ID_maxTimeStep || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTimeStep, val))
            streamAtts->SetMaxTimeStep(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(streamAtts->GetMaxTimeStep()));
            streamAtts->SetMaxTimeStep(streamAtts->GetMaxTimeStep());
        }
    }

    // Do termination
    if(which_widget == StreamlineAttributes::ID_maxSteps || doAll)
    {
        int val;
        if(LineEditGetInt(maxSteps, val))
            streamAtts->SetMaxSteps(val);
        else
        {
            ResettingError(tr("maxsteps"),
                IntToQString(streamAtts->GetMaxSteps()));
            streamAtts->SetMaxSteps(streamAtts->GetMaxSteps());
        }
    }
    if(which_widget == StreamlineAttributes::ID_termTime || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTime, val))
            streamAtts->SetTermTime(val);
        else
        {
            ResettingError(tr("maxtime"),
                DoubleToQString(streamAtts->GetTermTime()));
            streamAtts->SetTermTime(streamAtts->GetTermTime());
        }
    }
    if(which_widget == StreamlineAttributes::ID_termDistance || doAll)
    {
        double val;
        if(LineEditGetDouble(maxDistance, val))
            streamAtts->SetTermDistance(val);
        else
        {
            ResettingError(tr("maxdistance"),
                DoubleToQString(streamAtts->GetTermDistance()));
            streamAtts->SetTermDistance(streamAtts->GetTermDistance());
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
    if ((which_widget == StreamlineAttributes::ID_absTolBBox || doAll)
        && streamAtts->GetAbsTolSizeType() == StreamlineAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            streamAtts->SetAbsTolBBox(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(streamAtts->GetAbsTolBBox()));
                streamAtts->SetAbsTolBBox(streamAtts->GetAbsTolBBox());
        }
    }
    if ((which_widget == StreamlineAttributes::ID_absTolAbsolute || doAll)
        && streamAtts->GetAbsTolSizeType() == StreamlineAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            streamAtts->SetAbsTolAbsolute(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(streamAtts->GetAbsTolAbsolute()));
                streamAtts->SetAbsTolAbsolute(streamAtts->GetAbsTolAbsolute());
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

    // Do radius
    if(which_widget == StreamlineAttributes::ID_radius || doAll)
    {
        double val;
        if(LineEditGetDouble(radius, val))
            streamAtts->SetRadius(val);
        else
        {
            ResettingError(tr("plane radius"),
                DoubleToQString(streamAtts->GetRadius()));
            streamAtts->SetRadius(streamAtts->GetRadius());
        }
    }

    // Do sampleDistance 0
    if(which_widget == StreamlineAttributes::ID_sampleDistance0 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[0], val))
            streamAtts->SetSampleDistance0(val);
        else
        {
            ResettingError(tr("Sample distance 0"),
                DoubleToQString(streamAtts->GetSampleDistance0()));
            streamAtts->SetSampleDistance0(streamAtts->GetSampleDistance0());
        }
    }
    // Do sampleDistance 1
    if(which_widget == StreamlineAttributes::ID_sampleDistance1 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[1], val))
            streamAtts->SetSampleDistance1(val);
        else
        {
            ResettingError(tr("Sample distance 1"),
                DoubleToQString(streamAtts->GetSampleDistance1()));
            streamAtts->SetSampleDistance1(streamAtts->GetSampleDistance1());
        }
    }
    // Do sampleDistance 2
    if(which_widget == StreamlineAttributes::ID_sampleDistance2 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[2], val))
            streamAtts->SetSampleDistance2(val);
        else
        {
            ResettingError(tr("Sample distance 1"),
                DoubleToQString(streamAtts->GetSampleDistance2()));
            streamAtts->SetSampleDistance2(streamAtts->GetSampleDistance2());
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
        std::vector<double> points;
        double x,y,z;
        for (int i = 0; i < pointList->count(); i++)
        {
            QListWidgetItem *item = pointList->item(i);
            if (item)
            {
                string str = item->text().toLatin1().data();
                sscanf(str.c_str(), "%lf %lf %lf", &x, &y, &z);
                points.push_back(x);
                points.push_back(y);
                points.push_back(z);
            }
        }
        streamAtts->SetPointList(points);
    }

    // Do radius
    if ((which_widget == StreamlineAttributes::ID_tubeRadiusAbsolute || doAll)
        && streamAtts->GetTubeSizeType() == StreamlineAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(tubeRadius, val))
            streamAtts->SetTubeRadiusAbsolute(val);
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(streamAtts->GetTubeRadiusAbsolute()));
            streamAtts->SetTubeRadiusAbsolute(streamAtts->GetTubeRadiusAbsolute());
        }
    }
    if ((which_widget == StreamlineAttributes::ID_tubeRadiusBBox || doAll)
        && streamAtts->GetTubeSizeType() == StreamlineAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(tubeRadius, val))
            streamAtts->SetTubeRadiusBBox(val);
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(streamAtts->GetTubeRadiusBBox()));
            streamAtts->SetTubeRadiusBBox(streamAtts->GetTubeRadiusBBox());
        }
    }

    // Do radius
    if ((which_widget == StreamlineAttributes::ID_ribbonWidthAbsolute || doAll)
        && streamAtts->GetRibbonWidthSizeType() == StreamlineAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(ribbonWidth, val))
            streamAtts->SetRibbonWidthAbsolute(val);
        else
        {
            ResettingError(tr("ribbon width"),
                DoubleToQString(streamAtts->GetRibbonWidthAbsolute()));
            streamAtts->SetRibbonWidthAbsolute(streamAtts->GetRibbonWidthAbsolute());
        }
    }
    if ((which_widget == StreamlineAttributes::ID_ribbonWidthBBox || doAll)
        && streamAtts->GetRibbonWidthSizeType() == StreamlineAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(ribbonWidth, val))
            streamAtts->SetRibbonWidthBBox(val);
        else
        {
            ResettingError(tr("ribbon width"),
                DoubleToQString(streamAtts->GetRibbonWidthBBox()));
            streamAtts->SetRibbonWidthBBox(streamAtts->GetRibbonWidthBBox());
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
    if ((which_widget == StreamlineAttributes::ID_seedRadiusAbsolute || doAll)
        && streamAtts->GetSeedRadiusSizeType() == StreamlineAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(seedRadius, val))
            streamAtts->SetSeedRadiusAbsolute(val);
        else
        {
            ResettingError(tr("Seed radius"),
                DoubleToQString(streamAtts->GetSeedRadiusAbsolute()));
            streamAtts->SetSeedRadiusAbsolute(streamAtts->GetSeedRadiusAbsolute());
        }
    }
    if ((which_widget == StreamlineAttributes::ID_seedRadiusBBox || doAll)
        && streamAtts->GetSeedRadiusSizeType() == StreamlineAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(seedRadius, val))
            streamAtts->SetSeedRadiusBBox(val);
        else
        {
            ResettingError(tr("Seed radius"),
                DoubleToQString(streamAtts->GetSeedRadiusBBox()));
            streamAtts->SetSeedRadiusBBox(streamAtts->GetSeedRadiusBBox());
        }
    }

    // headRadius
    if ((which_widget == StreamlineAttributes::ID_headRadiusAbsolute || doAll)
        && streamAtts->GetHeadRadiusSizeType() == StreamlineAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(headRadius, val))
            streamAtts->SetHeadRadiusAbsolute(val);
        else
        {
            ResettingError(tr("Head radius"),
                DoubleToQString(streamAtts->GetHeadRadiusAbsolute()));
            streamAtts->SetHeadRadiusAbsolute(streamAtts->GetHeadRadiusAbsolute());
        }
    }
    if ((which_widget == StreamlineAttributes::ID_headRadiusBBox || doAll)
        && streamAtts->GetHeadRadiusSizeType() == StreamlineAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(headRadius, val))
            streamAtts->SetHeadRadiusBBox(val);
        else
        {
            ResettingError(tr("Head radius"),
                DoubleToQString(streamAtts->GetHeadRadiusBBox()));
            streamAtts->SetHeadRadiusBBox(streamAtts->GetHeadRadiusBBox());
        }
    }

    // headHeight
    if(which_widget == StreamlineAttributes::ID_headHeightRatio || doAll)
    {
        double val;
        if(LineEditGetDouble(headHeight, val))
            streamAtts->SetHeadHeightRatio(val);
        else
        {
            ResettingError(tr("Head height"),
                DoubleToQString(streamAtts->GetHeadHeightRatio()));
            streamAtts->SetHeadHeightRatio(streamAtts->GetHeadHeightRatio());
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
QvisStreamlinePlotWindow::maxTimeStepProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_maxTimeStep);
    Apply();
}

void
QvisStreamlinePlotWindow::maxStepsProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_maxSteps);
    Apply();
}

void
QvisStreamlinePlotWindow::limitMaxTimeChanged(bool val)
{
    if(val != streamAtts->GetTerminateByTime())
    {
        streamAtts->SetTerminateByTime(val);
        Apply();
    }
}

void
QvisStreamlinePlotWindow::limitMaxDistanceChanged(bool val)
{
    if(val != streamAtts->GetTerminateByDistance())
    {
        streamAtts->SetTerminateByDistance(val);
        Apply();
    }
}

void
QvisStreamlinePlotWindow::maxTimeProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_termTime);
    Apply();
}

void
QvisStreamlinePlotWindow::maxDistanceProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_termDistance);
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
QvisStreamlinePlotWindow::radiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_radius);
    Apply();
}

void
QvisStreamlinePlotWindow::sampleDistance0ProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sampleDistance0);
    Apply();
}

void
QvisStreamlinePlotWindow::sampleDistance1ProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sampleDistance1);
    Apply();
}

void
QvisStreamlinePlotWindow::sampleDistance2ProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sampleDistance2);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereOriginProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sphereOrigin);
    Apply();
}


void
QvisStreamlinePlotWindow::sampleDensity0Changed(int val)
{
    streamAtts->SetSampleDensity0(val);
    Apply();
}

void
QvisStreamlinePlotWindow::sampleDensity1Changed(int val)
{
    streamAtts->SetSampleDensity1(val);
    Apply();
}

void
QvisStreamlinePlotWindow::sampleDensity2Changed(int val)
{
    streamAtts->SetSampleDensity2(val);
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
    GetCurrentValues(StreamlineAttributes::ID_tubeRadiusAbsolute);
    GetCurrentValues(StreamlineAttributes::ID_tubeRadiusBBox);
    Apply();
}

void
QvisStreamlinePlotWindow::ribbonWidthProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_ribbonWidthAbsolute);
    GetCurrentValues(StreamlineAttributes::ID_ribbonWidthBBox);
    Apply();
}

void
QvisStreamlinePlotWindow::seedRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_seedRadiusAbsolute);
    GetCurrentValues(StreamlineAttributes::ID_seedRadiusBBox);
    Apply();
}

void
QvisStreamlinePlotWindow::headRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_headRadiusAbsolute);
    GetCurrentValues(StreamlineAttributes::ID_headRadiusBBox);
    Apply();
}

void
QvisStreamlinePlotWindow::headHeightRatioProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_headHeightRatio);
    Apply();
}

void 
QvisStreamlinePlotWindow::headSizeTypeChanged(int v)
{
    streamAtts->SetHeadRadiusSizeType((StreamlineAttributes::SizeType) v);
    Apply();
}

void 
QvisStreamlinePlotWindow::tubeSizeTypeChanged(int v)
{
    streamAtts->SetTubeSizeType((StreamlineAttributes::SizeType) v);
    Apply();
}

void 
QvisStreamlinePlotWindow::seedSizeTypeChanged(int v)
{
    streamAtts->SetSeedRadiusSizeType((StreamlineAttributes::SizeType) v);
    Apply();
}

void 
QvisStreamlinePlotWindow::ribbonSizeTypeChanged(int v)
{
    streamAtts->SetRibbonWidthSizeType((StreamlineAttributes::SizeType) v);
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
    GetCurrentValues(StreamlineAttributes::ID_absTolBBox);
    GetCurrentValues(StreamlineAttributes::ID_absTolAbsolute);
    Apply();
}

void
QvisStreamlinePlotWindow::absTolSizeTypeChanged(int val)
{
    streamAtts->SetAbsTolSizeType((StreamlineAttributes::SizeType) val);
    Apply();
}

void
QvisStreamlinePlotWindow::limitMaxTimeStepChanged(bool val)
{
    streamAtts->SetLimitMaximumTimestep(val);
    Apply();
}

void
QvisStreamlinePlotWindow::forceNodalChanged(bool val)
{
    streamAtts->SetForceNodeCenteredData(val);
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
QvisStreamlinePlotWindow::issueWarningForMaxStepsChanged(bool val)
{
    streamAtts->SetIssueTerminationWarnings(val);
    Apply();
}

void
QvisStreamlinePlotWindow::tubeDisplayDensityChanged(int val)
{
    streamAtts->SetTubeDisplayDensity(val);
    Apply();
}

void
QvisStreamlinePlotWindow::randomSamplesChanged(bool val)
{
    streamAtts->SetRandomSamples(val);
    Apply();
}

void
QvisStreamlinePlotWindow::fillChanged(int index)
{
    streamAtts->SetFillInterior(index == 1);
    Apply();
}

void
QvisStreamlinePlotWindow::randomSeedChanged(int val)
{
    streamAtts->SetRandomSeed(val);
    Apply();
}

void
QvisStreamlinePlotWindow::numberOfRandomSamplesChanged(int val)
{
    streamAtts->SetNumberOfRandomSamples(val);
    Apply();
}

void
QvisStreamlinePlotWindow::displayReferenceTypeChanged(int val)
{
    streamAtts->SetReferenceTypeForDisplay((StreamlineAttributes::ReferenceType) val);
    Apply();
}

void
QvisStreamlinePlotWindow::pointListDoubleClicked(QListWidgetItem *item)
{
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void
QvisStreamlinePlotWindow::pointListClicked(QListWidgetItem *item)
{
}

void
QvisStreamlinePlotWindow::textChanged(const QString &currentText)
{
}

void
QvisStreamlinePlotWindow::addPoint()
{
    QListWidgetItem *item = new QListWidgetItem("0 0 0", pointList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    pointList->setCurrentItem(item);
}

void
QvisStreamlinePlotWindow::deletePoint()
{
    if (!pointList->selectedItems().empty())
    {
        qDeleteAll(pointList->selectedItems());
    }
}

void
QvisStreamlinePlotWindow::deletePoints()
{
    pointList->clear();
}

void
QvisStreamlinePlotWindow::readPoints()
{
    QString res = QFileDialog::getOpenFileName(NULL, tr("Open text file"), ".");
    string filename = res.toLatin1().data();

    if (filename == "")
        return;
    std::ifstream f;
    f.open(filename.c_str());
    while (f.good())
    {
        char tmp[256];
        f.getline(tmp, 256);
        if (strlen(tmp) == 0)
            break;

        float x, y, z;
        int n = sscanf(tmp, "%f %f %f", &x, &y, &z);
        if (n != 3)
            n = sscanf(tmp, "%f, %f, %f", &x, &y, &z);
        if (n == 2)
        {
            z = 0.0;
            n = 3;
        }
        if (n == 3)
        {
            char vals[256];
            sprintf(vals, "%f %f %f", x,y,z);
            QListWidgetItem *item = new QListWidgetItem(vals, pointList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            pointList->setCurrentItem(item);
        }
    }

    f.close();
}

static void
TurnOn(QWidget *w0, QWidget *w1)
{
    if (w0)
    {
        w0->setEnabled(true);
        w0->show();
    }
    if (w1)
    {
        w1->setEnabled(true);
        w1->show();
    }
}

static void
TurnOff(QWidget *w0, QWidget *w1)
{
    if (w0)
    {
        w0->setEnabled(false);
        w0->hide();
    }
    if (w1)
    {
        w1->setEnabled(false);
        w1->hide();
    }
}
