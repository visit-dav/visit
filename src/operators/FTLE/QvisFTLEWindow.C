/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include "QvisFTLEWindow.h"

#include <FTLEAttributes.h>
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
#include <QListWidget>
#include <QFileDialog>
#include <SelectionProperties.h>

#include <stdio.h>

#include <string>
#include <vector>

static void
TurnOn(QWidget *w0, QWidget *w1=NULL);
static void
TurnOff(QWidget *w0, QWidget *w1=NULL);

// ****************************************************************************
// Method: QvisFTLEWindow::QvisFTLEWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
// ****************************************************************************

QvisFTLEWindow::QvisFTLEWindow(const int type,
                               FTLEAttributes *subj,
                               const QString &caption,
                               const QString &shortName,
                               QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    plotType = type;
    FTLEAtts = subj;
}


// ****************************************************************************
// Method: QvisFTLEWindow::~QvisFTLEWindow
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

QvisFTLEWindow::~QvisFTLEWindow()
{
}


// ****************************************************************************
// Method: QvisFTLEWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFTLEWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // Integration tab
    // ----------------------------------------------------------------------
    QWidget *integrationTab = new QWidget(central);
    propertyTabs->addTab(integrationTab, tr("Integration"));
    CreateIntegrationTab(integrationTab);

    // ----------------------------------------------------------------------
    // Appearance tab
    // ----------------------------------------------------------------------
    QWidget *appearanceTab = new QWidget(central);
    propertyTabs->addTab(appearanceTab, tr("Appearance"));
    CreateAppearanceTab(appearanceTab);

    // ----------------------------------------------------------------------
    // Advanced tab
    // ----------------------------------------------------------------------
    QWidget *advancedTab = new QWidget(central);
    propertyTabs->addTab(advancedTab, tr("Advanced"));
    CreateAdvancedTab(advancedTab);
}


// ****************************************************************************
// Method: QvisFTLEWindow::IntegrationTab
//
// Purpose: 
//   Populates the integration tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisFTLEWindow::CreateIntegrationTab(QWidget *pageIntegration)
{
    QGridLayout *mainLayout = new QGridLayout(pageIntegration);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

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
    sourceType->addItem(tr("Native Mesh"));
    sourceType->addItem(tr("Regular Grid"));
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

    // Regular grid
    QGroupBox *regularGridBox = new QGroupBox(central);
    regularGridBox->setTitle(tr("Regular Grid"));
    geometryLayout->addWidget(regularGridBox);

    // Resolution
    QGridLayout *rgridLayout = new QGridLayout(regularGridBox);
    ResolutionLabel = new QLabel(tr("Resolution"), central);
    rgridLayout->addWidget(ResolutionLabel,0,0);

    Resolution = new QLineEdit(central);
    connect(Resolution, SIGNAL(returnPressed()),
            this, SLOT(ResolutionProcessText()));
    rgridLayout->addWidget(Resolution, 0, 1, 1, 2);

    // Start extents
    rgridLayout->addWidget(new QLabel(tr("Data start extent"), central), 1, 0);

    UseDataSetStart = new QButtonGroup(central);
    QRadioButton* rb = new QRadioButton(tr("Full"), central);
    UseDataSetStart->addButton(rb, 0);
    rgridLayout->addWidget(rb, 1,1);

    rb = new QRadioButton(tr("Subset"), central);
    UseDataSetStart->addButton(rb, 1);
    rgridLayout->addWidget(rb, 1,2);
    connect(UseDataSetStart, SIGNAL(buttonClicked(int)), this,
            SLOT(UseDataSetStartChanged(int)));

    StartPosition = new QLineEdit(central);
    connect(StartPosition, SIGNAL(returnPressed()),
            this, SLOT(StartPositionProcessText()));
    rgridLayout->addWidget(StartPosition, 1,3);


    // End extents
    rgridLayout->addWidget(new QLabel(tr("Data end extent"), central), 2, 0);

    UseDataSetEnd = new QButtonGroup(central);
    rb = new QRadioButton(tr("Full"), central);
    UseDataSetEnd->addButton(rb, 0);
    rgridLayout->addWidget(rb, 2,1);

    rb = new QRadioButton(tr("Subset"), central);
    UseDataSetEnd->addButton(rb, 1);
    rgridLayout->addWidget(rb, 2,2);
    connect(UseDataSetEnd, SIGNAL(buttonClicked(int)), this,
            SLOT(UseDataSetEndChanged(int)));

    EndPosition = new QLineEdit(central);
    connect(EndPosition, SIGNAL(returnPressed()),
            this, SLOT(EndPositionProcessText()));
    rgridLayout->addWidget(EndPosition, 2,3);

    // Create the field group box.
    QGroupBox *fieldGroup = new QGroupBox(central);
    fieldGroup->setTitle(tr("Field"));
    mainLayout->addWidget(fieldGroup, 6, 0, 1, 1);
//    mainLayout->setStretchFactor(fieldGroup, 100);
    QGridLayout *fieldLayout = new QGridLayout(fieldGroup);
    fieldLayout->setMargin(5);
    fieldLayout->setSpacing(10);


    fieldLayout->addWidget( new QLabel(tr("Field"), fieldGroup), 0,0);
    fieldType = new QComboBox(fieldGroup);
    fieldType->addItem(tr("Default"));
    fieldType->addItem(tr("Flash"));
    fieldType->addItem(tr("M3D-C1 2D"));
    fieldType->addItem(tr("M3D-C1 3D"));
    fieldType->addItem(tr("Nek5000"));
    fieldType->addItem(tr("NIMROD"));
    connect(fieldType, SIGNAL(activated(int)),
            this, SLOT(fieldTypeChanged(int)));
    fieldLayout->addWidget(fieldType, 0,1);
    

    // Create the field constant text field.
    fieldConstantLabel = new QLabel(tr("Constant"), fieldGroup);
    fieldConstant = new QLineEdit(fieldGroup);
    connect(fieldConstant, SIGNAL(returnPressed()), this,
            SLOT(fieldConstantProccessText()));
    fieldLayout->addWidget(fieldConstantLabel, 0,2);
    fieldLayout->addWidget(fieldConstant, 0,3);

    // Create the widgets that specify a velocity source.
    velocitySource = new QLineEdit(fieldGroup);
    connect(velocitySource, SIGNAL(returnPressed()),
            this, SLOT(velocitySourceProcessText()));
    velocitySourceLabel = new QLabel(tr("Velocity"), fieldGroup);
    velocitySourceLabel->setBuddy(velocitySource);
    fieldLayout->addWidget(velocitySourceLabel, 1, 2);
    fieldLayout->addWidget(velocitySource, 1, 3);

    // Create the node centering
    forceNodal = new QCheckBox(tr("Force node centering"), fieldGroup);
    connect(forceNodal, SIGNAL(toggled(bool)), this, SLOT(forceNodalChanged(bool)));
    fieldLayout->addWidget(forceNodal, 2, 0);

    // Create the integration group box.
    QGroupBox *integrationGroup = new QGroupBox(central);
    integrationGroup->setTitle(tr("Integration"));
    mainLayout->addWidget(integrationGroup, 7, 0, 4, 2);
//    mainLayout->setStretchFactor(integrationGroup, 100);
    QGridLayout *integrationLayout = new QGridLayout(integrationGroup);
    integrationLayout->setMargin(5);
    integrationLayout->setSpacing(10);


    // Create the direction of integration.
    integrationLayout->addWidget(new QLabel(tr("Integration direction"),
                                            central), 0, 0);
    directionType = new QComboBox(central);
    directionType->addItem(tr("Forward"));
    directionType->addItem(tr("Backward"));
    connect(directionType, SIGNAL(activated(int)),
            this, SLOT(directionTypeChanged(int)));
    integrationLayout->addWidget(directionType, 0, 1);

    // Create the type of integration.
    integrationLayout->addWidget( new QLabel(tr("Integrator"), integrationGroup), 1,0);
    integrationType = new QComboBox(integrationGroup);
    integrationType->addItem(tr("Forward Euler (Single-step)"));
    integrationType->addItem(tr("Leapfrog (Single-step)"));
    integrationType->addItem(tr("Dormand-Prince (Runge-Kutta)"));
    integrationType->addItem(tr("Adams-Bashforth (Multi-step)"));
    integrationType->addItem(tr("Runge-Kutta 4 (Single-step)"));
    integrationType->addItem(tr("M3D-C1 2D Integrator (M3D-C1 2D fields only)"));
    connect(integrationType, SIGNAL(activated(int)),
            this, SLOT(integrationTypeChanged(int)));
    integrationLayout->addWidget(integrationType, 1,1);
    
    // Create the step length text field.
    maxStepLengthLabel = new QLabel(tr("Step length"), integrationGroup);
    maxStepLength = new QLineEdit(integrationGroup);
    connect(maxStepLength, SIGNAL(returnPressed()),
            this, SLOT(maxStepLengthProcessText()));
    integrationLayout->addWidget(maxStepLengthLabel, 2,0);
    integrationLayout->addWidget(maxStepLength, 2,1);

    limitMaxTimeStep = new QCheckBox(tr("Limit maximum time step"), integrationGroup);
    connect(limitMaxTimeStep, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeStepChanged(bool)));
    integrationLayout->addWidget(limitMaxTimeStep, 3, 0);
    
    // Create the step length text field.
    maxTimeStep = new QLineEdit(integrationGroup);
    connect(maxTimeStep, SIGNAL(returnPressed()),
            this, SLOT(maxTimeStepProcessText()));
    integrationLayout->addWidget(maxTimeStep, 3,1);

    QGroupBox *toleranceGroup = new QGroupBox(central);
    toleranceGroup->setTitle(tr("Tolerances: max error for step < max(abstol, reltol*velocity_i) for each component i"));
    integrationLayout->addWidget(toleranceGroup, 4, 0, 2, 3);
    QGridLayout *toleranceLayout = new QGridLayout(toleranceGroup);
    toleranceLayout->setMargin(5);
    toleranceLayout->setSpacing(10);

    // Create the relative tolerance text field.
    relTolLabel = new QLabel(tr("Relative tolerance"), toleranceGroup);
    relTol = new QLineEdit(toleranceGroup);
    connect(relTol, SIGNAL(returnPressed()),
            this, SLOT(relTolProcessText()));
    toleranceLayout->addWidget(relTolLabel, 0, 0);
    toleranceLayout->addWidget(relTol, 0, 1);

    // Create the absolute tolerance text field.
    absTolLabel = new QLabel(tr("Absolute tolerance"), toleranceGroup);
    absTol = new QLineEdit(toleranceGroup);
    connect(absTol, SIGNAL(returnPressed()), this, SLOT(absTolProcessText()));
    toleranceLayout->addWidget(absTolLabel, 1, 0);
    toleranceLayout->addWidget(absTol, 1, 1);

    absTolSizeType = new QComboBox(toleranceGroup);
    absTolSizeType->addItem(tr("Absolute"), 0);
    absTolSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(absTolSizeType, SIGNAL(activated(int)), this, SLOT(absTolSizeTypeChanged(int)));
    toleranceLayout->addWidget(absTolSizeType, 1, 2);

    // Create the termination group box.
    QGroupBox *terminationGroup = new QGroupBox(central);
    terminationGroup->setTitle(tr("Termination"));
    mainLayout->addWidget(terminationGroup, 12, 0, 2, 2);
//    mainLayout->setStretchFactor(terminationGroup, 100);
    QGridLayout *terminationLayout = new QGridLayout(terminationGroup);
    terminationLayout->setMargin(5);
    terminationLayout->setSpacing(10);

    // For FTLE or FDLE base the termintion on the time or distance,
    // respectively. While using the max steps as a back up. This is
    // opposite of streamlines/pathlines which optionally use the
    // termination.
    limitButtonGroup = new QButtonGroup(terminationGroup);
    rb = new QRadioButton(tr("Limit maximum time i.e. FTLE"), terminationGroup);
    limitButtonGroup->addButton(rb, 0);
    terminationLayout->addWidget(rb, 0,0);

    rb->setChecked(true);

    rb = new QRadioButton(tr("Limit maximum distance i.e. FDLE"), terminationGroup);
    limitButtonGroup->addButton(rb, 1);
    terminationLayout->addWidget(rb, 1,0);

    rb = new QRadioButton(tr("Limit maximum size i.e. FSLE"), terminationGroup);
    limitButtonGroup->addButton(rb, 2);
    terminationLayout->addWidget(rb, 2,0);


    connect(limitButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(limitButtonGroupChanged(int)));

    // limitMaxTime = new QCheckBox(tr("Limit maximum time elapsed for particles"), terminationGroup);
    // connect(limitMaxTime, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeChanged(bool)));
    // terminationLayout->addWidget(limitMaxTime, 0,0);
    maxTime = new QLineEdit(central);
    connect(maxTime, SIGNAL(returnPressed()), this, SLOT(maxTimeProcessText()));
    terminationLayout->addWidget(maxTime, 0,1);

    // limitMaxDistance = new QCheckBox(tr("Limit maximum distance traveled by particles"), terminationGroup);
    // connect(limitMaxDistance, SIGNAL(toggled(bool)), this, SLOT(limitMaxDistanceChanged(bool)));
    // terminationLayout->addWidget(limitMaxDistance, 1,0);

    maxDistance = new QLineEdit(central);
    connect(maxDistance, SIGNAL(returnPressed()), this, SLOT(maxDistanceProcessText()));
    terminationLayout->addWidget(maxDistance, 1,1);


    maxSize = new QLineEdit(central);
    connect(maxSize, SIGNAL(returnPressed()), this, SLOT(maxSizeProcessText()));
    terminationLayout->addWidget(maxSize, 2,1);


    QLabel *maxStepsLabel = new QLabel(tr("Maximum number of steps"), terminationGroup);
    terminationLayout->addWidget(maxStepsLabel, 3,0);
    maxSteps = new QLineEdit(central);
    connect(maxSteps, SIGNAL(returnPressed()),
            this, SLOT(maxStepsProcessText()));
    terminationLayout->addWidget(maxSteps, 3,1);
}


// ****************************************************************************
// Method: QvisFTLEWindow::CreateAdvancedTab
//
// Purpose: 
//   Populates the appearance tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisFTLEWindow::CreateAppearanceTab(QWidget *pageAppearance)
{
    QGridLayout *mainLayout = new QGridLayout(pageAppearance);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    // Streamlines/Pathline Group.
    QGroupBox *icGrp = new QGroupBox(pageAppearance);
    icGrp->setTitle(tr("Streamlines vs Pathlines"));
    mainLayout->addWidget(icGrp, 1, 0);

    QGridLayout *icGrpLayout = new QGridLayout(icGrp);
    icGrpLayout->setSpacing(10);
    icGrpLayout->setColumnStretch(1,10);

    icButtonGroup = new QButtonGroup(icGrp);
    QRadioButton *streamlineButton = new QRadioButton(tr("Streamline\n    Compute trajectories in an (instantaneous) snapshot of the vector field.\n    Uses and loads vector data from only the current time slice."), icGrp);
    QRadioButton *pathlineButton = new QRadioButton(tr("Pathline    \n    Compute trajectories in the time-varying vector field.\n    Uses and loads vector data from all relevant time slices."), icGrp);
    streamlineButton->setChecked(true);
    icButtonGroup->addButton(streamlineButton, 0);
    icButtonGroup->addButton(pathlineButton, 1);
    icGrpLayout->addWidget(streamlineButton, 1, 0);
    icGrpLayout->addWidget(pathlineButton, 2, 0);
    connect(icButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(icButtonGroupChanged(int)));

    // Pathline Options
    QGroupBox *pathlineOptionsGrp = new QGroupBox(icGrp);
    pathlineOptionsGrp->setTitle(tr("Pathlines Options"));
    icGrpLayout->addWidget(pathlineOptionsGrp, 3, 0);

    QGridLayout *pathlineOptionsGrpLayout = new QGridLayout(pathlineOptionsGrp);
    pathlineOptionsGrpLayout->setSpacing(10);
    pathlineOptionsGrpLayout->setColumnStretch(1,10);

    pathlineOverrideStartingTimeFlag = new QCheckBox(tr("Override Starting Time"), pathlineOptionsGrp);
    connect(pathlineOverrideStartingTimeFlag, SIGNAL(toggled(bool)),
            this, SLOT(pathlineOverrideStartingTimeFlagChanged(bool)));
    pathlineOptionsGrpLayout->addWidget(pathlineOverrideStartingTimeFlag, 1, 0);

    QLabel *pathlineOverrideStartingTimeLabel = new QLabel(tr("Time"), pathlineOptionsGrp);
    pathlineOverrideStartingTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    pathlineOptionsGrpLayout->addWidget(pathlineOverrideStartingTimeLabel, 1, 1);
    pathlineOverrideStartingTime = new QLineEdit(pathlineOptionsGrp);
    connect(pathlineOverrideStartingTime, SIGNAL(returnPressed()),
            this, SLOT(pathlineOverrideStartingTimeProcessText()));
    pathlineOptionsGrpLayout->addWidget(pathlineOverrideStartingTime, 1, 2);

    QGroupBox *cmfeOptionsGrp = new QGroupBox(pathlineOptionsGrp);
    cmfeOptionsGrp->setTitle(tr("How to perform interpolation over time"));
    pathlineOptionsGrpLayout->addWidget(cmfeOptionsGrp, 2, 0);

    QGridLayout *cmfeOptionsGrpLayout = new QGridLayout(cmfeOptionsGrp);
    cmfeOptionsGrpLayout->setSpacing(10);
    cmfeOptionsGrpLayout->setColumnStretch(1,10);

    pathlineCMFEButtonGroup = new QButtonGroup(cmfeOptionsGrp);
    QRadioButton *connButton = new QRadioButton(tr("Mesh is static over time (fast, but special purpose)"), cmfeOptionsGrp);
    QRadioButton *posButton = new QRadioButton(tr("Mesh changes over time (slow, but robust)"), cmfeOptionsGrp);
    posButton->setChecked(true);
    pathlineCMFEButtonGroup->addButton(connButton, 0);
    pathlineCMFEButtonGroup->addButton(posButton, 1);
    cmfeOptionsGrpLayout->addWidget(connButton, 2, 0);
    cmfeOptionsGrpLayout->addWidget(posButton, 3, 0);
    connect(pathlineCMFEButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(pathlineCMFEButtonGroupChanged(int)));

}

// ****************************************************************************
// Method: QvisFTLEWindow::CreateAdvancedTab
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
//   Hank Childs, Sun Dec  5 05:31:57 PST 2010
//   Add additional warning controls.
//
// ****************************************************************************

void
QvisFTLEWindow::CreateAdvancedTab(QWidget *pageAdvanced)
{
    int row = 0;
    QGridLayout *mainLayout = new QGridLayout(pageAdvanced);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(5);

    QGroupBox *algoGrp = new QGroupBox(pageAdvanced);
    algoGrp->setTitle(tr("Parallel integration options"));
    mainLayout->addWidget(algoGrp, 0, 0);

    // Algorithm group.
    QGridLayout *algoGLayout = new QGridLayout(algoGrp);
    algoGLayout->setSpacing(10);
    algoGLayout->setColumnStretch(1,10);

    parallelAlgoLabel = new QLabel(tr("Parallelization"), algoGrp);
    parallelAlgo = new QComboBox(algoGrp);
    parallelAlgo->addItem(tr("Parallelize over curves"));
    parallelAlgo->addItem(tr("Parallelize over domains"));
    parallelAlgo->addItem(tr("Parallelize over curves and domains"));
    parallelAlgo->addItem(tr("Have VisIt select the best algorithm"));
    connect(parallelAlgo, SIGNAL(activated(int)),
            this, SLOT(parallelAlgorithmChanged(int)));
    algoGLayout->addWidget( parallelAlgoLabel, 1,0);
    algoGLayout->addWidget( parallelAlgo, 1,1);
    
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
    mainLayout->addWidget(warningsGrp, 1, 0);

    QGridLayout *warningsGLayout = new QGridLayout(warningsGrp);
    warningsGLayout->setSpacing(10);
    warningsGLayout->setColumnStretch(1,10);

    issueWarningForMaxSteps = new QCheckBox(central);
    connect(issueWarningForMaxSteps, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForMaxStepsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForMaxSteps, 0, 0);
    QLabel *maxStepsLabel = new QLabel(tr("Issue warning when the maximum number of steps is reached."), warningsGrp);
    warningsGLayout->addWidget(maxStepsLabel, 0, 1, 1, 2);

    issueWarningForStiffness = new QCheckBox(central);
    connect(issueWarningForStiffness, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForStiffnessChanged(bool)));
    warningsGLayout->addWidget(issueWarningForStiffness, 1, 0);
    QLabel *stiffnessLabel = new QLabel(tr("Issue warning when stiffness is detected."), warningsGrp);
    warningsGLayout->addWidget(stiffnessLabel, 1, 1, 1, 2);
    QLabel *stiffnessDescLabel1 = new QLabel(tr("(Stiffness refers to one vector component being so much "), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel1, 2, 1, 1, 2);
    QLabel *stiffnessDescLabel2 = new QLabel(tr("larger than another that tolerances can't be met.)"), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel2, 3, 1, 1, 2);
    
    issueWarningForCriticalPoints = new QCheckBox(central);
    connect(issueWarningForCriticalPoints, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForCriticalPointsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForCriticalPoints, 4, 0);
    QLabel *critPointLabel = new QLabel(tr("Issue warning when a curve doesn't terminate at a critical point."), warningsGrp);
    warningsGLayout->addWidget(critPointLabel, 4, 1, 1, 2);
    QLabel *critPointDescLabel = new QLabel(tr("(I.e. the curve circles around the critical point without stopping.)"), warningsGrp);
    warningsGLayout->addWidget(critPointDescLabel, 5, 1, 1, 2);
    criticalPointThresholdLabel = new QLabel(tr("Speed cutoff for critical points"), warningsGrp);
    criticalPointThresholdLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    warningsGLayout->addWidget(criticalPointThresholdLabel, 6, 1);
    criticalPointThreshold = new QLineEdit(warningsGrp);
    criticalPointThreshold->setAlignment(Qt::AlignLeft);
    connect(criticalPointThreshold, SIGNAL(returnPressed()),
            this, SLOT(criticalPointThresholdProcessText()));
    warningsGLayout->addWidget(criticalPointThreshold, 6, 2);
}


// ****************************************************************************
// Method: QvisFTLEWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisFTLEWindow::UpdateWindow(bool doAll)
{
    for(int i = 0; i < FTLEAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!FTLEAtts->IsSelected(i))
            {
                continue;
            }
        }
        switch(i)
        {
        case FTLEAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(FTLEAtts->GetSourceType());
            sourceType->blockSignals(false);

            if (FTLEAtts->GetSourceType() == FTLEAttributes::RegularGrid)
            {
                Resolution->setEnabled(true);
                ResolutionLabel->setEnabled(true);
                UseDataSetStart->button(0)->setEnabled(true);
                UseDataSetStart->button(1)->setEnabled(true);
                UseDataSetEnd->button(0)->setEnabled(true);
                UseDataSetEnd->button(1)->setEnabled(true);
                StartPosition->setEnabled(FTLEAtts->GetUseDataSetStart() ==
                                          FTLEAttributes::Subset);
                EndPosition->setEnabled(FTLEAtts->GetUseDataSetEnd() ==
                                        FTLEAttributes::Subset);
            }
            else
            {
                Resolution->setEnabled(false);
                ResolutionLabel->setEnabled(false);
                UseDataSetStart->button(0)->setEnabled(false);
                UseDataSetStart->button(1)->setEnabled(false);
                UseDataSetEnd->button(0)->setEnabled(false);
                UseDataSetEnd->button(1)->setEnabled(false);
                StartPosition->setEnabled(false);
                EndPosition->setEnabled(false);
            }

            break;

        case FTLEAttributes::ID_Resolution:
            Resolution->setText(IntsToQString(FTLEAtts->GetResolution(),3));
            break;
        case FTLEAttributes::ID_UseDataSetStart:
            UseDataSetStart->blockSignals(true);
            UseDataSetStart->button(FTLEAtts->GetUseDataSetStart())->setChecked(true);
            UseDataSetStart->blockSignals(false);

            StartPosition->setEnabled(FTLEAtts->GetUseDataSetStart() ==
                                      FTLEAttributes::Subset);

            break;
        case FTLEAttributes::ID_StartPosition:
            StartPosition->setText(DoublesToQString(FTLEAtts->GetStartPosition(), 3));
            break;
        case FTLEAttributes::ID_UseDataSetEnd:
            UseDataSetEnd->blockSignals(true);
            UseDataSetEnd->button(FTLEAtts->GetUseDataSetEnd())->setChecked(true);
            UseDataSetEnd->blockSignals(false);

            EndPosition->setEnabled(FTLEAtts->GetUseDataSetEnd() ==
                                    FTLEAttributes::Subset);
            break;
        case FTLEAttributes::ID_EndPosition:
            EndPosition->setText(DoublesToQString(FTLEAtts->GetEndPosition(), 3));
            break;
        case FTLEAttributes::ID_maxStepLength:
            maxStepLength->setText(DoubleToQString(FTLEAtts->GetMaxStepLength()));
            break;
        case FTLEAttributes::ID_limitMaximumTimestep:
            limitMaxTimeStep->blockSignals(true);
            limitMaxTimeStep->setChecked(FTLEAtts->GetLimitMaximumTimestep());
            limitMaxTimeStep->blockSignals(false);
            maxTimeStep->blockSignals(true);
            if (FTLEAtts->GetIntegrationType() ==
                FTLEAttributes::DormandPrince )
            {
                maxTimeStep->setEnabled(FTLEAtts->GetLimitMaximumTimestep());
            }
            else
            {
                maxTimeStep->setEnabled(false);
            }
            maxTimeStep->blockSignals(false);
            break;
        case FTLEAttributes::ID_maxTimeStep:
            maxTimeStep->setText(DoubleToQString(FTLEAtts->GetMaxTimeStep()));
            break;
        case FTLEAttributes::ID_maxSteps:
            maxSteps->setText(IntToQSTring(FTLEAtts->GetMaxSteps()));
            break;
        case FTLEAttributes::ID_terminationType:
            limitButtonGroup->blockSignals(true);
            limitButtonGroup->button(FTLEAtts->GetTerminationType())->setChecked(true);
            limitButtonGroup->blockSignals(false);

            maxTime->setEnabled(FTLEAtts->GetTerminationType()==0);
            maxDistance->setEnabled(FTLEAtts->GetTerminationType()==1);
            maxSize->setEnabled(FTLEAtts->GetTerminationType()==2);
            break;
        case FTLEAttributes::ID_termSize:
            maxSize->setText(DoubleToQString(FTLEAtts->GetTermSize()));
            break;
        case FTLEAttributes::ID_termDistance:
            maxDistance->setText(DoubleToQString(FTLEAtts->GetTermDistance()));
            break;
        case FTLEAttributes::ID_termTime:
            maxTime->setText(DoubleToQString(FTLEAtts->GetTermTime()));
            break;
        case FTLEAttributes::ID_velocitySource:
            velocitySource->setText(DoublesToQString(FTLEAtts->GetVelocitySource(),3));
            break;

        case FTLEAttributes::ID_integrationDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(FTLEAtts->GetIntegrationDirection()) );
            directionType->blockSignals(false);
            break;
        case FTLEAttributes::ID_relTol:
            relTol->setText(DoubleToQString(FTLEAtts->GetRelTol()));
            break;
        case FTLEAttributes::ID_absTolSizeType:
            absTolSizeType->blockSignals(true);
            absTolSizeType->setCurrentIndex((int) FTLEAtts->GetAbsTolSizeType());
            absTolSizeType->blockSignals(false);
            if (FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(FTLEAtts->GetAbsTolBBox()));
            }
            if (FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(FTLEAtts->GetAbsTolAbsolute()));
            }
            break;
        case FTLEAttributes::ID_absTolBBox:
            if (FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(FTLEAtts->GetAbsTolBBox()));
            }
            break;
        case FTLEAttributes::ID_absTolAbsolute:
            if (FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(FTLEAtts->GetAbsTolAbsolute()));
            }
            break;
        case FTLEAttributes::ID_fieldType:
            // Update lots of widget visibility and enabled states.
            UpdateFieldAttributes();

            fieldType->blockSignals(true);
            fieldType->setCurrentIndex(FTLEAtts->GetFieldType());
            fieldType->blockSignals(false);

            integrationType->blockSignals(true);
            if (FTLEAtts->GetFieldType() == FTLEAttributes::M3DC12DField)
            {
              FTLEAtts->SetIntegrationType(FTLEAttributes::M3DC12DIntegrator);
              integrationType->setCurrentIndex(FTLEAttributes::M3DC12DIntegrator);
              UpdateIntegrationAttributes();
            }
            else if (FTLEAtts->GetFieldType() == FTLEAttributes::NIMRODField)
            {
              FTLEAtts->SetIntegrationType(FTLEAttributes::AdamsBashforth);
              integrationType->setCurrentIndex(FTLEAttributes::AdamsBashforth);
              UpdateIntegrationAttributes();
            }
            else if (FTLEAtts->GetIntegrationType() == FTLEAttributes::M3DC12DIntegrator) 
            {
              FTLEAtts->SetIntegrationType(FTLEAttributes::DormandPrince);
              integrationType->setCurrentIndex(FTLEAttributes::DormandPrince);
              UpdateIntegrationAttributes();
            }
            integrationType->blockSignals(false);

            break;
        case FTLEAttributes::ID_fieldConstant:
            fieldConstant->setText(DoubleToQString(FTLEAtts->GetFieldConstant()));
            break;
        case FTLEAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(FTLEAtts->GetIntegrationType());
            integrationType->blockSignals(false);

            fieldType->blockSignals(true);
            if (FTLEAtts->GetIntegrationType() == FTLEAttributes::M3DC12DIntegrator)
            {
              FTLEAtts->SetFieldType(FTLEAttributes::M3DC12DField);
              fieldType->setCurrentIndex(FTLEAttributes::M3DC12DField);
              UpdateFieldAttributes();
            }
            else if (FTLEAtts->GetFieldType() == FTLEAttributes::M3DC12DField)
            {
              FTLEAtts->SetFieldType(FTLEAttributes::Default);
              fieldType->setCurrentIndex(FTLEAttributes::Default);
              UpdateFieldAttributes();
            }
            fieldType->blockSignals(false);

            break;
        case FTLEAttributes::ID_parallelizationAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();
            parallelAlgo->blockSignals(true);
            parallelAlgo->setCurrentIndex(FTLEAtts->GetParallelizationAlgorithmType());
            parallelAlgo->blockSignals(false);
            break;

        case FTLEAttributes::ID_maxProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(FTLEAtts->GetMaxProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case FTLEAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(FTLEAtts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case FTLEAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(FTLEAtts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case FTLEAttributes::ID_pathlines:
            icButtonGroup->blockSignals(true);
            icButtonGroup->button(FTLEAtts->GetPathlines()?1:0)->setChecked(true);
            pathlineOverrideStartingTimeFlag->setEnabled(FTLEAtts->GetPathlines());
            if( pathlineOverrideStartingTimeFlag->isChecked() && ! icButtonGroup->button(1)->isChecked() )
                pathlineOverrideStartingTimeFlag->setChecked(false);
            pathlineOverrideStartingTime->setEnabled(FTLEAtts->GetPathlines() && FTLEAtts->GetPathlinesOverrideStartingTimeFlag());
            pathlineCMFEButtonGroup->button(0)->setEnabled(FTLEAtts->GetPathlines());
            pathlineCMFEButtonGroup->button(1)->setEnabled(FTLEAtts->GetPathlines());
            icButtonGroup->blockSignals(false);
            break;
        case FTLEAttributes::ID_pathlinesOverrideStartingTimeFlag:
            pathlineOverrideStartingTimeFlag->blockSignals(true);
            pathlineOverrideStartingTimeFlag->setChecked(FTLEAtts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTime->setEnabled(FTLEAtts->GetPathlines() && FTLEAtts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTimeFlag->blockSignals(false);
            break;
        case FTLEAttributes::ID_pathlinesOverrideStartingTime:
            pathlineOverrideStartingTime->setText(DoubleToQString(FTLEAtts->GetPathlinesOverrideStartingTime()));
            break;
        case FTLEAttributes::ID_pathlinesCMFE:
            pathlineCMFEButtonGroup->blockSignals(true);
            pathlineCMFEButtonGroup->button(FTLEAtts->GetPathlinesCMFE())->setChecked(true);
            pathlineCMFEButtonGroup->blockSignals(false);
            break;

        case FTLEAttributes::ID_forceNodeCenteredData:
            forceNodal->blockSignals(true);
            forceNodal->setChecked(FTLEAtts->GetForceNodeCenteredData());
            forceNodal->blockSignals(false);
            break;

        case FTLEAttributes::ID_issueTerminationWarnings:
            issueWarningForMaxSteps->blockSignals(true);
            issueWarningForMaxSteps->setChecked(FTLEAtts->GetIssueTerminationWarnings());
            issueWarningForMaxSteps->blockSignals(false);
            break;
            
        case FTLEAttributes::ID_issueCriticalPointsWarnings:
            issueWarningForCriticalPoints->blockSignals(true);
            issueWarningForCriticalPoints->setChecked(FTLEAtts->GetIssueCriticalPointsWarnings());
            criticalPointThreshold->setEnabled(FTLEAtts->GetIssueCriticalPointsWarnings());
            criticalPointThresholdLabel->setEnabled(FTLEAtts->GetIssueCriticalPointsWarnings());
            issueWarningForCriticalPoints->blockSignals(false);
            break;

        case FTLEAttributes::ID_issueStiffnessWarnings:
            issueWarningForStiffness->blockSignals(true);
            issueWarningForStiffness->setChecked(FTLEAtts->GetIssueStiffnessWarnings());
            issueWarningForStiffness->blockSignals(false);
            break;
        case FTLEAttributes::ID_criticalPointThreshold:
            criticalPointThreshold->setText(DoubleToQString(FTLEAtts->GetCriticalPointThreshold()));
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisFTLEWindow::UpdateFieldAttributes
//
// Purpose: 
//   Updates the widgets for the various field types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// ****************************************************************************

void
QvisFTLEWindow::UpdateFieldAttributes()
{
    switch( FTLEAtts->GetFieldType() )
    {
    case FTLEAttributes::M3DC12DField:
      if( FTLEAtts->GetIntegrationType() ==
          FTLEAttributes::M3DC12DIntegrator ) 
        TurnOn(fieldConstant, fieldConstantLabel);
      else
        TurnOff(fieldConstant, fieldConstantLabel);

      TurnOff(velocitySource, velocitySourceLabel);

      break;

    case FTLEAttributes::FlashField:
      TurnOn(fieldConstant, fieldConstantLabel);
      TurnOn(velocitySource, velocitySourceLabel);
      break;

    case FTLEAttributes::NIMRODField:
    default:
      TurnOff(fieldConstant, fieldConstantLabel);
      TurnOff(velocitySource, velocitySourceLabel);

      break;
    }
}


// ****************************************************************************
// Method: QvisFTLEWindow::UpdateIntegrationAttributes
//
// Purpose: 
//   Updates the widgets for the various integration types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisFTLEWindow::UpdateIntegrationAttributes()
{
    //Turn off everything.
    maxStepLength->hide();
    maxStepLengthLabel->hide();
    limitMaxTimeStep->hide();
    maxTimeStep->hide();
    relTol->hide();
    relTolLabel->hide();
    absTol->hide();
    absTolLabel->hide();
    absTolSizeType->hide();

    switch( FTLEAtts->GetIntegrationType() )
    {
    case FTLEAttributes::Euler:
    case FTLEAttributes::Leapfrog:
    case FTLEAttributes::RK4:
        maxStepLength->show();
        maxStepLengthLabel->show();
      break;

    case FTLEAttributes::DormandPrince:
        limitMaxTimeStep->show();
        maxTimeStep->show();
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
        absTolSizeType->show();
        break;

    case FTLEAttributes::AdamsBashforth:
    case FTLEAttributes::M3DC12DIntegrator:
        maxStepLength->show();
        maxStepLengthLabel->show();
        absTol->show();
        absTolLabel->show();
        absTolSizeType->show();
        break;

    default:
        break;
    }
}


// ****************************************************************************
// Method: QvisFTLEWindow::UpdateAlgorithmAttributes
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
// ****************************************************************************

void
QvisFTLEWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = (FTLEAtts->GetParallelizationAlgorithmType() ==
                            FTLEAttributes::LoadOnDemand);
    bool useStaticDomains = (FTLEAtts->GetParallelizationAlgorithmType() ==
                             FTLEAttributes::ParallelStaticDomains);
    bool useMasterSlave = (FTLEAtts->GetParallelizationAlgorithmType() ==
                           FTLEAttributes::MasterSlave);
    
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
// Method: QvisFTLEWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisFTLEWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do resolution
    if(which_widget == FTLEAttributes::ID_Resolution || doAll)
    {
        int val[3];
        if(LineEditGetInts(Resolution, val, 3))
            FTLEAtts->SetResolution(val);
        else
        {
            ResettingError(tr("Resolution"),
                IntsToQString(FTLEAtts->GetResolution(), 3));
            FTLEAtts->SetResolution(FTLEAtts->GetResolution());
        }
    }

    // Do Start Position
    if(which_widget == FTLEAttributes::ID_StartPosition || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(StartPosition, val, 3))
            FTLEAtts->SetStartPosition(val);
        else
        {
            ResettingError(tr("StartPosition"),
                DoublesToQString(FTLEAtts->GetStartPosition(), 3));
            FTLEAtts->SetStartPosition(FTLEAtts->GetStartPosition());
        }
    }

    // Do End Position
    if(which_widget == FTLEAttributes::ID_EndPosition || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(EndPosition, val, 3))
            FTLEAtts->SetEndPosition(val);
        else
        {
            ResettingError(tr("EndPosition"),
                DoublesToQString(FTLEAtts->GetEndPosition(), 3));
            FTLEAtts->SetEndPosition(FTLEAtts->GetEndPosition());
        }
    }


    // Do fieldConstant
    if(which_widget == FTLEAttributes::ID_fieldConstant || doAll)
    {
        double val;
        if(LineEditGetDouble(fieldConstant, val))
            FTLEAtts->SetFieldConstant(val);
        else
        {
            ResettingError(tr("field constant"),
                DoubleToQString(FTLEAtts->GetFieldConstant()));
            FTLEAtts->SetFieldConstant(FTLEAtts->GetFieldConstant());
        }
    }

    // Do stepLength
    if(which_widget == FTLEAttributes::ID_maxStepLength || doAll)
    {
        double val;
        if(LineEditGetDouble(maxStepLength, val))
            FTLEAtts->SetMaxStepLength(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(FTLEAtts->GetMaxStepLength()));
            FTLEAtts->SetMaxStepLength(FTLEAtts->GetMaxStepLength());
        }
    }

    // Do max time step
    if(which_widget == FTLEAttributes::ID_maxTimeStep || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTimeStep, val))
            FTLEAtts->SetMaxTimeStep(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(FTLEAtts->GetMaxTimeStep()));
            FTLEAtts->SetMaxTimeStep(FTLEAtts->GetMaxTimeStep());
        }
    }

    // Do termination
    if(which_widget == FTLEAttributes::ID_maxSteps || doAll)
    {
        int val;
        if(LineEditGetInt(maxSteps, val))
            FTLEAtts->SetMaxSteps(val);
        else
        {
            ResettingError(tr("maxsteps"),
                IntToQString(FTLEAtts->GetMaxSteps()));
            FTLEAtts->SetMaxSteps(FTLEAtts->GetMaxSteps());
        }
    }
    if(which_widget == FTLEAttributes::ID_termTime || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTime, val))
            FTLEAtts->SetTermTime(val);
        else
        {
            ResettingError(tr("maxtime"),
                DoubleToQString(FTLEAtts->GetTermTime()));
            FTLEAtts->SetTermTime(FTLEAtts->GetTermTime());
        }
    }
    if(which_widget == FTLEAttributes::ID_termDistance || doAll)
    {
        double val;
        if(LineEditGetDouble(maxDistance, val))
            FTLEAtts->SetTermDistance(val);
        else
        {
            ResettingError(tr("maxdistance"),
                DoubleToQString(FTLEAtts->GetTermDistance()));
            FTLEAtts->SetTermDistance(FTLEAtts->GetTermDistance());
        }
    }
    if(which_widget == FTLEAttributes::ID_termSize || doAll)
    {
        double val;
        if(LineEditGetDouble(maxSize, val))
            FTLEAtts->SetTermSize(val);
        else
        {
            ResettingError(tr("maxsize"),
                DoubleToQString(FTLEAtts->GetTermSize()));
            FTLEAtts->SetTermSize(FTLEAtts->GetTermSize());
        }
    }
    if(which_widget == FTLEAttributes::ID_pathlinesOverrideStartingTime || doAll)
    {
        double val;
        if(LineEditGetDouble(pathlineOverrideStartingTime, val))
            FTLEAtts->SetPathlinesOverrideStartingTime(val);
        else
        {
            ResettingError(tr("Pathlines Override Starting Time"),
                DoubleToQString(FTLEAtts->GetPathlinesOverrideStartingTime()));
            FTLEAtts->SetPathlinesOverrideStartingTime(FTLEAtts->GetPathlinesOverrideStartingTime());
        }
    }


    // Do relTol
    if(which_widget == FTLEAttributes::ID_relTol || doAll)
    {
        double val;
        if(LineEditGetDouble(relTol, val))
            FTLEAtts->SetRelTol(val);
        else
        {
            ResettingError(tr("relative tolerance"),
                DoubleToQString(FTLEAtts->GetRelTol()));
            FTLEAtts->SetRelTol(FTLEAtts->GetRelTol());
        }
    }

    // Do absTol
    if ((which_widget == FTLEAttributes::ID_absTolBBox || doAll)
        && FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            FTLEAtts->SetAbsTolBBox(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(FTLEAtts->GetAbsTolBBox()));
                FTLEAtts->SetAbsTolBBox(FTLEAtts->GetAbsTolBBox());
        }
    }
    if ((which_widget == FTLEAttributes::ID_absTolAbsolute || doAll)
        && FTLEAtts->GetAbsTolSizeType() == FTLEAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            FTLEAtts->SetAbsTolAbsolute(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(FTLEAtts->GetAbsTolAbsolute()));
                FTLEAtts->SetAbsTolAbsolute(FTLEAtts->GetAbsTolAbsolute());
        }
    }

    // Do velocitySource
    if(which_widget == FTLEAttributes::ID_velocitySource || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(velocitySource, val, 3))
            FTLEAtts->SetVelocitySource(val);
        else
        {
            ResettingError(tr("velocity source"),
                DoublesToQString(FTLEAtts->GetVelocitySource(), 3));
            FTLEAtts->SetVelocitySource(FTLEAtts->GetVelocitySource());
        }
    }

    if(which_widget == FTLEAttributes::ID_pathlinesOverrideStartingTime || doAll)
    {
        double val;
        if(LineEditGetDouble(pathlineOverrideStartingTime, val))
            FTLEAtts->SetPathlinesOverrideStartingTime(val);
        else
        {
            ResettingError(tr("Pathlines Override Starting Time"),
                DoubleToQString(FTLEAtts->GetPathlinesOverrideStartingTime()));
            FTLEAtts->SetPathlinesOverrideStartingTime(FTLEAtts->GetPathlinesOverrideStartingTime());
        }
    }

    // maxProcessCount
    if (which_widget == FTLEAttributes::ID_maxProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            FTLEAtts->SetMaxProcessCount(val);
    }

    // workGroupSize
    if (which_widget == FTLEAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            FTLEAtts->SetWorkGroupSize(val);
    }
    
    // criticalPointThreshold
    if(which_widget == FTLEAttributes::ID_criticalPointThreshold || doAll)
    {
        double val;
        if(LineEditGetDouble(criticalPointThreshold, val))
            FTLEAtts->SetCriticalPointThreshold(val);
        else
        {
            ResettingError(tr("Speed cutoff for critical points"),
                DoubleToQString(FTLEAtts->GetCriticalPointThreshold()));
            FTLEAtts->SetCriticalPointThreshold(FTLEAtts->GetCriticalPointThreshold());
        }
    }
}


void
QvisFTLEWindow::sourceTypeChanged(int val)
{
    if(val != FTLEAtts->GetSourceType())
    {
        FTLEAtts->SetSourceType(FTLEAttributes::SourceType(val));
        Apply();
    }
}


void
QvisFTLEWindow::ResolutionProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_Resolution);
    Apply();
}


void
QvisFTLEWindow::UseDataSetStartChanged(int val)
{
    FTLEAtts->SetUseDataSetStart(FTLEAttributes::Extents(val));
    Apply();
}


void
QvisFTLEWindow::StartPositionProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_StartPosition);
    Apply();
}


void
QvisFTLEWindow::UseDataSetEndChanged(int val)
{
  FTLEAtts->SetUseDataSetEnd(FTLEAttributes::Extents(val));
    Apply();
}


void
QvisFTLEWindow::EndPositionProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_EndPosition);
    Apply();
}


void
QvisFTLEWindow::directionTypeChanged(int val)
 {
    if(val != FTLEAtts->GetIntegrationDirection())
    {
        FTLEAtts->SetIntegrationDirection(FTLEAttributes::IntegrationDirection(val));
        Apply();
    }
}   

void
QvisFTLEWindow::fieldTypeChanged(int val)
 {
    if(val != FTLEAtts->GetFieldType())
    {
        FTLEAtts->SetFieldType(FTLEAttributes::FieldType(val));
        Apply();
    }
}   

void
QvisFTLEWindow::fieldConstantProccessText()
{
    GetCurrentValues(FTLEAttributes::ID_fieldConstant);
    Apply();
}

void
QvisFTLEWindow::integrationTypeChanged(int val)
{
    if(val != FTLEAtts->GetIntegrationType())
    {
        FTLEAtts->SetIntegrationType(FTLEAttributes::IntegrationType(val));
        Apply();
    }
}   

void
QvisFTLEWindow::parallelAlgorithmChanged(int val)
{
    if(val != FTLEAtts->GetParallelizationAlgorithmType())
    {
        FTLEAtts->SetParallelizationAlgorithmType(FTLEAttributes::ParallelizationAlgorithmType(val));
        Apply();
    }
}   

void
QvisFTLEWindow::maxStepLengthProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_maxStepLength);
    Apply();
}

void
QvisFTLEWindow::maxTimeStepProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_maxTimeStep);
    Apply();
}

void
QvisFTLEWindow::maxStepsProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_maxSteps);
    Apply();
}

void
QvisFTLEWindow::limitButtonGroupChanged(int index)
{
    FTLEAtts->SetTerminationType( (FTLEAttributes::TerminationType) index );
    FTLEAtts->SetTerminateByTime( index == 0);
    FTLEAtts->SetTerminateByDistance( index == 1);
    FTLEAtts->SetTerminateBySize( index == 2);
    Apply();
}

void
QvisFTLEWindow::maxTimeProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_termTime);
    Apply();
}

void
QvisFTLEWindow::maxDistanceProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_termDistance);
    Apply();
}

void
QvisFTLEWindow::maxSizeProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_termSize);
    Apply();
}

void
QvisFTLEWindow::relTolProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_relTol);
    Apply();
}

void
QvisFTLEWindow::maxSLCountChanged(int val)
{
    FTLEAtts->SetMaxProcessCount(val);
    Apply();
}

void
QvisFTLEWindow::maxDomainCacheChanged(int val)
{
    FTLEAtts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisFTLEWindow::workGroupSizeChanged(int val)
{
    FTLEAtts->SetWorkGroupSize(val);
    Apply();
}

void
QvisFTLEWindow::icButtonGroupChanged(int val)
{
    switch( val )
    {
        case 0: // Streamline
            FTLEAtts->SetPathlines(false);
            break;
        case 1: // Pathline
            FTLEAtts->SetPathlines(true);
            break;
    }
    Apply();
}

void
QvisFTLEWindow::pathlineOverrideStartingTimeFlagChanged(bool val)
{
    FTLEAtts->SetPathlinesOverrideStartingTimeFlag(val);
    Apply();
}

void
QvisFTLEWindow::pathlineOverrideStartingTimeProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_pathlinesOverrideStartingTime);
    Apply();
}

void
QvisFTLEWindow::pathlineCMFEButtonGroupChanged(int val)
{
    FTLEAtts->SetPathlinesCMFE((FTLEAttributes::PathlinesCMFE)val);
    Apply();
}

void
QvisFTLEWindow::absTolProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_absTolBBox);
    GetCurrentValues(FTLEAttributes::ID_absTolAbsolute);
    Apply();
}

void
QvisFTLEWindow::absTolSizeTypeChanged(int val)
{
    FTLEAtts->SetAbsTolSizeType((FTLEAttributes::SizeType) val);
    Apply();
}

void
QvisFTLEWindow::limitMaxTimeStepChanged(bool val)
{
    FTLEAtts->SetLimitMaximumTimestep(val);
    Apply();
}

void
QvisFTLEWindow::forceNodalChanged(bool val)
{
    FTLEAtts->SetForceNodeCenteredData(val);
    Apply();
}

void
QvisFTLEWindow::velocitySourceProcessText()
{
    GetCurrentValues(FTLEAttributes::ID_velocitySource);
    Apply();
}

void
QvisFTLEWindow::issueWarningForMaxStepsChanged(bool val)
{
    FTLEAtts->SetIssueTerminationWarnings(val);
    Apply();
}

void
QvisFTLEWindow::issueWarningForStiffnessChanged(bool val)
{
    FTLEAtts->SetIssueStiffnessWarnings(val);
    Apply();
}

void
QvisFTLEWindow::issueWarningForCriticalPointsChanged(bool val)
{
    FTLEAtts->SetIssueCriticalPointsWarnings(val);
    Apply();
}

void
QvisFTLEWindow::criticalPointThresholdProcessText(void)
{
    GetCurrentValues(FTLEAttributes::ID_criticalPointThreshold);
    Apply();
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
