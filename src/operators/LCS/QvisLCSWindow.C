// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisLCSWindow.h"

#include <LCSAttributes.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <SelectionProperties.h>


static void
TurnOn(QWidget *w0, QWidget *w1=NULL);
static void
TurnOff(QWidget *w0, QWidget *w1=NULL);

// ****************************************************************************
// Method: QvisLCSWindow::QvisLCSWindow
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

QvisLCSWindow::QvisLCSWindow(const int type,
                               LCSAttributes *subj,
                               const QString &caption,
                               const QString &shortName,
                               QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisLCSWindow::~QvisLCSWindow
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

QvisLCSWindow::~QvisLCSWindow()
{
}


// ****************************************************************************
// Method: QvisLCSWindow::CreateWindowContents
//
// Purpose:
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Kathleen Biagas, Fri Nov  8 08:08:13 PST 2019
//   Added sourceTab to hold Source and Field widgets.
//
// ****************************************************************************

void
QvisLCSWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // Source/Field tab
    // ----------------------------------------------------------------------
    QWidget *sourceTab = new QWidget();
    propertyTabs->addTab(sourceTab, tr("Source"));
    CreateSourceTab(sourceTab);

    // ----------------------------------------------------------------------
    // Integration tab
    // ----------------------------------------------------------------------
    QWidget *integrationTab = new QWidget();
    propertyTabs->addTab(integrationTab, tr("Integration"));
    CreateIntegrationTab(integrationTab);

    // ----------------------------------------------------------------------
    // Appearance tab
    // ----------------------------------------------------------------------
    QWidget *appearanceTab = new QWidget();
    propertyTabs->addTab(appearanceTab, tr("Appearance"));
    CreateAppearanceTab(appearanceTab);

    // ----------------------------------------------------------------------
    // Advanced tab
    // ----------------------------------------------------------------------
    QWidget *advancedTab = new QWidget();
    propertyTabs->addTab(advancedTab, tr("Advanced"));
    CreateAdvancedTab(advancedTab);
}


// ****************************************************************************
// Method: QvisLCSWindow::CreateSourceTab
//
// Purpose:
//   Populates the source tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//    Kathleen Biagas, Fri Nov  8 08:09:13 PST 2019
//    Content moved from Integration tab.
//
// ****************************************************************************

void
QvisLCSWindow::CreateSourceTab(QWidget *pageSource)
{
    QVBoxLayout *sLayout = new QVBoxLayout(pageSource);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    sLayout->addLayout(mainLayout);
    sLayout->addStretch(1);

    // Create the source group box.
    QGroupBox *sourceGroup = new QGroupBox();
    sourceGroup->setTitle(tr("Source"));
    mainLayout->addWidget(sourceGroup, 0, 0, 5, 2);
    QGridLayout *sourceLayout = new QGridLayout(sourceGroup);
    sourceLayout->setMargin(5);
    sourceLayout->setSpacing(10);

    // Create the source type combo box.
    sourceLayout->addWidget(new QLabel(tr("Source type")), 0, 0);
    sourceType = new QComboBox();
    sourceType->addItem(tr("Native Mesh"));
    sourceType->addItem(tr("Regular Grid"));
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    sourceLayout->addWidget(sourceType, 0, 1, 1, 2);

    // Create the source geometry subgroup
    QGroupBox *geometryGroup = new QGroupBox();
    sourceLayout->addWidget(geometryGroup, 1, 0, 4, 3);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
    geometryLayout->setRowStretch(5,10);

    // Regular grid
    QGroupBox *regularGridBox = new QGroupBox();
    regularGridBox->setTitle(tr("Regular Grid"));
    geometryLayout->addWidget(regularGridBox);

    // Resolution
    QGridLayout *rgridLayout = new QGridLayout(regularGridBox);
    ResolutionLabel = new QLabel(tr("Resolution"));
    rgridLayout->addWidget(ResolutionLabel,0,0);

    Resolution = new QLineEdit();
    connect(Resolution, SIGNAL(returnPressed()),
            this, SLOT(ResolutionProcessText()));
    rgridLayout->addWidget(Resolution, 0, 1, 1, 2);

    // Start extents
    rgridLayout->addWidget(new QLabel(tr("Data start extent")), 1, 0);

    UseDataSetStart = new QButtonGroup();
    QRadioButton* rb = new QRadioButton(tr("Full"));
    UseDataSetStart->addButton(rb, 0);
    rgridLayout->addWidget(rb, 1,1);

    rb = new QRadioButton(tr("Subset"));
    UseDataSetStart->addButton(rb, 1);
    rgridLayout->addWidget(rb, 1,2);
    connect(UseDataSetStart, SIGNAL(buttonClicked(int)), this,
            SLOT(UseDataSetStartChanged(int)));

    StartPosition = new QLineEdit();
    connect(StartPosition, SIGNAL(returnPressed()),
            this, SLOT(StartPositionProcessText()));
    rgridLayout->addWidget(StartPosition, 1,3);


    // End extents
    rgridLayout->addWidget(new QLabel(tr("Data end extent")), 2, 0);

    UseDataSetEnd = new QButtonGroup();
    rb = new QRadioButton(tr("Full"));
    UseDataSetEnd->addButton(rb, 0);
    rgridLayout->addWidget(rb, 2,1);

    rb = new QRadioButton(tr("Subset"));
    UseDataSetEnd->addButton(rb, 1);
    rgridLayout->addWidget(rb, 2,2);
    connect(UseDataSetEnd, SIGNAL(buttonClicked(int)), this,
            SLOT(UseDataSetEndChanged(int)));

    EndPosition = new QLineEdit();
    connect(EndPosition, SIGNAL(returnPressed()),
            this, SLOT(EndPositionProcessText()));
    rgridLayout->addWidget(EndPosition, 2,3);


    // Create the auxiliary grid group box.
    QGroupBox *auxiliaryGridGroup = new QGroupBox();
    auxiliaryGridGroup->setTitle(tr("Auxiliary Grid"));
    sourceLayout->addWidget(auxiliaryGridGroup, 5, 0, 1, 4);

    QGridLayout *auxiliaryGridLayout = new QGridLayout(auxiliaryGridGroup);
    auxiliaryGridLayout->setMargin(5);
    auxiliaryGridLayout->setSpacing(10);

    // Auxiliary grid label and combo box
    auxiliaryGridLayout->addWidget( new QLabel(tr("Auxiliary Grid")), 0,0);

    auxiliaryGrid = new QComboBox();
    auxiliaryGrid->addItem(tr("None"));
    auxiliaryGrid->addItem(tr("2D"));
    auxiliaryGrid->addItem(tr("3D"));
    connect(auxiliaryGrid, SIGNAL(activated(int)),
            this, SLOT(auxiliaryGridChanged(int)));
    auxiliaryGridLayout->addWidget(auxiliaryGrid, 0,1);

    // Create the auxiliary grid spacing
    auxiliaryGridSpacingLabel = new QLabel(tr("Spacing"));
    auxiliaryGridSpacing = new QLineEdit();
    connect(auxiliaryGridSpacing, SIGNAL(returnPressed()), this,
            SLOT(auxiliaryGridSpacingProcessText()));
    auxiliaryGridLayout->addWidget(auxiliaryGridSpacingLabel, 0,2);
    auxiliaryGridLayout->addWidget(auxiliaryGridSpacing, 0,3);


    // Create the field group box.
    QGroupBox *fieldGroup = new QGroupBox();
    fieldGroup->setTitle(tr("Field"));
    mainLayout->addWidget(fieldGroup, 6, 0, 1, 1);
//    mainLayout->setStretchFactor(fieldGroup, 100);
    QGridLayout *fieldLayout = new QGridLayout(fieldGroup);
    fieldLayout->setMargin(5);
    fieldLayout->setSpacing(10);


    fieldLayout->addWidget( new QLabel(tr("Field")), 0,0);
    fieldType = new QComboBox();
    fieldType->addItem(tr("Default"));
    fieldType->addItem(tr("Flash"));
    fieldType->addItem(tr("M3D-C1 2D"));
    fieldType->addItem(tr("M3D-C1 3D"));
    fieldType->addItem(tr("Nek5000"));
    fieldType->addItem(tr("Nektar++"));
    fieldType->addItem(tr("NIMROD"));
    connect(fieldType, SIGNAL(activated(int)),
            this, SLOT(fieldTypeChanged(int)));
    fieldLayout->addWidget(fieldType, 0,1);


    // Create the field constant text field.
    fieldConstantLabel = new QLabel(tr("Constant"));
    fieldConstant = new QLineEdit();
    connect(fieldConstant, SIGNAL(returnPressed()), this,
            SLOT(fieldConstantProcessText()));
    fieldLayout->addWidget(fieldConstantLabel, 0,2);
    fieldLayout->addWidget(fieldConstant, 0,3);

    // Create the widgets that specify a velocity source.
    velocitySource = new QLineEdit();
    connect(velocitySource, SIGNAL(returnPressed()),
            this, SLOT(velocitySourceProcessText()));
    velocitySourceLabel = new QLabel(tr("Velocity"));
    velocitySourceLabel->setBuddy(velocitySource);
    fieldLayout->addWidget(velocitySourceLabel, 1, 2);
    fieldLayout->addWidget(velocitySource, 1, 3);

    // Create the node centering
    // forceNodal = new QCheckBox(tr("Force node centering"));
    // connect(forceNodal, SIGNAL(toggled(bool)), this, SLOT(forceNodalChanged(bool)));
    // fieldLayout->addWidget(forceNodal, 2, 0);
}


// ****************************************************************************
// Method: QvisLCSWindow::IntegrationTab
//
// Purpose:
//   Populates the integration tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//    Kathleen Biagas, Fri Nov  8 08:09:41 PST 2019
//    Moved Source and Field widgets to Source tab.
//
// ****************************************************************************

void
QvisLCSWindow::CreateIntegrationTab(QWidget *pageIntegration)
{
    QGridLayout *mainLayout = new QGridLayout(pageIntegration);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

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

    // For FTLE or FLLE base the termintion on the time or distance,
    // respectively. While using the max steps as a back up. This is
    // opposite of streamlines/pathlines which optionally use the
    // termination.
    terminationTypeButtonGroup = new QButtonGroup(terminationGroup);

    // Create the operation of integration.
    terminationLayout->addWidget(new QLabel(tr("Operation type"),
                                            central), 0, 0);

    operationType = new QComboBox(central);
    operationType->addItem(tr("Integration time"));
    operationType->addItem(tr("Arc length"));
    operationType->addItem(tr("Average distance from seed"));
    operationType->addItem(tr("Eigen Value"));
    operationType->addItem(tr("Eigen Vector"));
    operationType->addItem(tr("Lyapunov Exponent"));
    connect(operationType, SIGNAL(activated(int)),
            this, SLOT(operationTypeChanged(int)));
    terminationLayout->addWidget(operationType, 0, 1);

    // Create the cauchyGreenTensor of integration.
    cauchyGreenTensorLabel = new QLabel(tr("Tensor"), central);
    cauchyGreenTensorLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    terminationLayout->addWidget(cauchyGreenTensorLabel, 0, 2);

    cauchyGreenTensor = new QComboBox(central);
    cauchyGreenTensor->addItem(tr("Left Cauchy Green"));
    cauchyGreenTensor->addItem(tr("Right Cauchy Green"));
    connect(cauchyGreenTensor, SIGNAL(activated(int)),
            this, SLOT(cauchyGreenTensorChanged(int)));
    terminationLayout->addWidget(cauchyGreenTensor, 0, 3);

    // Create the eigenComponent of integration.
    eigenComponentLabel = new QLabel(tr("Eigen value"), central);
    terminationLayout->addWidget(eigenComponentLabel, 1, 0);

    eigenComponent = new QComboBox(central);
    eigenComponent->addItem(tr("Smallest"));
    eigenComponent->addItem(tr("Intermediate (3D only)"));
    eigenComponent->addItem(tr("Largest"));
    eigenComponent->addItem(tr("Shear Pos."));
    eigenComponent->addItem(tr("Shear Neg."));
    eigenComponent->addItem(tr("Shear Pos. linear combination"));
    eigenComponent->addItem(tr("Shear Neg. linear combination"));
    eigenComponent->setMaxCount(7);
    connect(eigenComponent, SIGNAL(activated(int)),
            this, SLOT(eigenComponentChanged(int)));
    terminationLayout->addWidget(eigenComponent, 1, 1);

    // Create the operator of integrator.
    operatorType = new QComboBox(central);
    operatorType->addItem(tr("Base value"));
    operatorType->addItem(tr("Gradient"));
    connect(operatorType, SIGNAL(activated(int)),
            this, SLOT(operatorTypeChanged(int)));
    terminationLayout->addWidget(operatorType, 0, 2);

    clampLogValues = new QCheckBox(tr("Clamp exponent values"), central);
    connect(clampLogValues, SIGNAL(toggled(bool)), this, SLOT(clampLogValuesChanged(bool)));
    terminationLayout->addWidget(clampLogValues, 1, 3);

    // Create the eigen weight text field.
//    eigenWeightLabel = new QLabel(tr("Weight"), central);
//    eigenWeightLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    eigenWeightEdit = new QLineEdit(terminationGroup);
    connect(eigenWeightEdit, SIGNAL(returnPressed()), this, SLOT(eigenWeightEditProcessText()));

    eigenWeightSlider = new QSlider(Qt::Horizontal, central);
    eigenWeightSlider->setRange(-100,+100);
    eigenWeightSlider->setSingleStep(10);
    eigenWeightSlider->setValue(0);

    connect(eigenWeightSlider, SIGNAL(valueChanged(int)),
            this, SLOT(eigenWeightSliderChanged(int)));
    connect(eigenWeightSlider, SIGNAL(sliderPressed()),
            this, SLOT(eigenWeightSliderPressed()));
    connect(eigenWeightSlider, SIGNAL(sliderReleased()),
            this, SLOT(eigenWeightSliderReleased()));

//    terminationLayout->addWidget(eigenWeightLabel,  2, 3);
    terminationLayout->addWidget(eigenWeightEdit,   1, 2);
    terminationLayout->addWidget(eigenWeightSlider, 1, 3);


    // Radio button termination type
    QRadioButton *rb = new QRadioButton(tr("Limit maximum advection time i.e. FTLE"), terminationGroup);
    terminationTypeButtonGroup->addButton(rb, 0);
    terminationLayout->addWidget(rb, 2, 0, 1, 2);

    rb->setChecked(true);

    rb = new QRadioButton(tr("Limit maximum advection distance i.e. FLLE"), terminationGroup);
    terminationTypeButtonGroup->addButton(rb, 1);
    terminationLayout->addWidget(rb, 3, 0, 1, 2);

    rb = new QRadioButton(tr("Limit maximum size i.e. FSLE"), terminationGroup);
    terminationTypeButtonGroup->addButton(rb, 2);
    terminationLayout->addWidget(rb, 4, 0, 1, 2);

    connect(terminationTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(terminationTypeButtonGroupChanged(int)));

    // Check box termination type
    limitMaxTime = new QCheckBox(tr("Limit maximum advection time"), terminationGroup);
    connect(limitMaxTime, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeChanged(bool)));
    terminationLayout->addWidget(limitMaxTime, 2, 0, 1, 2);
    limitMaxTime->hide();

    limitMaxDistance = new QCheckBox(tr("Limit maximum advection distance"), terminationGroup);
    connect(limitMaxDistance, SIGNAL(toggled(bool)), this, SLOT(limitMaxDistanceChanged(bool)));
    terminationLayout->addWidget(limitMaxDistance, 3, 0, 1, 2);
    limitMaxDistance->hide();

    // Termination values
    maxTime = new QLineEdit(central);
    connect(maxTime, SIGNAL(returnPressed()), this, SLOT(maxTimeProcessText()));
    terminationLayout->addWidget(maxTime, 2, 2);

    maxDistance = new QLineEdit(central);
    connect(maxDistance, SIGNAL(returnPressed()), this, SLOT(maxDistanceProcessText()));
    terminationLayout->addWidget(maxDistance, 3, 2);

    maxSize = new QLineEdit(central);
    connect(maxSize, SIGNAL(returnPressed()), this, SLOT(maxSizeProcessText()));
    terminationLayout->addWidget(maxSize, 4, 2);

    // Max steps override
    QLabel *maxStepsLabel = new QLabel(tr("Maximum number of steps"), terminationGroup);
    terminationLayout->addWidget(maxStepsLabel, 5, 0, 1, 2);
    maxSteps = new QLineEdit(central);
    connect(maxSteps, SIGNAL(returnPressed()),
            this, SLOT(maxStepsProcessText()));
    terminationLayout->addWidget(maxSteps, 5, 2);
}


// ****************************************************************************
// Method: QvisLCSWindow::CreateAppearanceTab
//
// Purpose:
//   Populates the appearance tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
// ****************************************************************************

void
QvisLCSWindow::CreateAppearanceTab(QWidget *pageAppearance)
{
    QGridLayout *mainLayout = new QGridLayout(pageAppearance);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    // Seed generation group.
    QGroupBox *seedGroup = new QGroupBox(pageAppearance);
    seedGroup->setTitle(tr("Seed generation"));
    mainLayout->addWidget(seedGroup, 0, 0);

    QGridLayout *seedGroupLayout = new QGridLayout(seedGroup);
    seedGroupLayout->setSpacing(10);
    seedGroupLayout->setColumnStretch(1,10);

    QLabel *thresholdLimitLabel = new QLabel(tr("Threshold limit"), seedGroup);
    thresholdLimitLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    seedGroupLayout->addWidget(thresholdLimitLabel, 0, 0);
    thresholdLimit = new QLineEdit(seedGroup);
    connect(thresholdLimit, SIGNAL(returnPressed()),
            this, SLOT(thresholdLimitProcessText()));
    seedGroupLayout->addWidget(thresholdLimit, 0, 1);


    QLabel *radialLimitLabel = new QLabel(tr("Radial limit"), seedGroup);
    radialLimitLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    seedGroupLayout->addWidget(radialLimitLabel, 1, 0);
    radialLimit = new QLineEdit(seedGroup);
    connect(radialLimit, SIGNAL(returnPressed()),
            this, SLOT(radialLimitProcessText()));
    seedGroupLayout->addWidget(radialLimit, 1, 1);


    QLabel *boundaryLimitLabel = new QLabel(tr("Boundary limit"), seedGroup);
    boundaryLimitLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    seedGroupLayout->addWidget(boundaryLimitLabel, 2, 0);
    boundaryLimit = new QLineEdit(seedGroup);
    connect(boundaryLimit, SIGNAL(returnPressed()),
            this, SLOT(boundaryLimitProcessText()));
    seedGroupLayout->addWidget(boundaryLimit, 2, 1);

    QLabel *seedLimitLabel = new QLabel(tr("Maximum number of seeds"), seedGroup);
    seedLimitLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    seedGroupLayout->addWidget( seedLimitLabel, 3, 0);
    seedLimit = new QSpinBox(seedGroup);
    seedLimit->setKeyboardTracking(false);
    seedLimit->setMinimum(1);
    seedLimit->setMaximum(100);
    connect(seedLimit, SIGNAL(valueChanged(int)),
            this, SLOT(seedLimitChanged(int)));
    seedGroupLayout->addWidget( seedLimit, 3, 1);

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
    pathlineOptionsGrp->setTitle(tr("Pathline Options"));
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

    QLabel *pathlinePeriodLabel = new QLabel(tr("Period"), pathlineOptionsGrp);
    pathlinePeriodLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    pathlineOptionsGrpLayout->addWidget(pathlinePeriodLabel, 1, 3);
    pathlinePeriod = new QLineEdit(pathlineOptionsGrp);
    connect(pathlinePeriod, SIGNAL(returnPressed()),
            this, SLOT(pathlinePeriodProcessText()));
    pathlineOptionsGrpLayout->addWidget(pathlinePeriod, 1, 4);

    QGroupBox *cmfeOptionsGrp = new QGroupBox(pathlineOptionsGrp);
    cmfeOptionsGrp->setTitle(tr("How to perform interpolation over time"));
    pathlineOptionsGrpLayout->addWidget(cmfeOptionsGrp, 2, 0, 2, 5);

    QGridLayout *cmfeOptionsGrpLayout = new QGridLayout(cmfeOptionsGrp);
    cmfeOptionsGrpLayout->setSpacing(10);
    cmfeOptionsGrpLayout->setColumnStretch(1,10);

    pathlineCMFEButtonGroup = new QButtonGroup(cmfeOptionsGrp);
    QRadioButton *connButton = new QRadioButton(tr("Mesh is static over time (fast, but special purpose)"), cmfeOptionsGrp);
    QRadioButton *posButton = new QRadioButton(tr("Mesh changes over time (slow, but robust)"), cmfeOptionsGrp);
    posButton->setChecked(true);
    pathlineCMFEButtonGroup->addButton(connButton, 0);
    pathlineCMFEButtonGroup->addButton(posButton, 1);
    cmfeOptionsGrpLayout->addWidget(connButton, 2, 0, 1, 5);
    cmfeOptionsGrpLayout->addWidget(posButton, 3, 0, 1, 5);
    connect(pathlineCMFEButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(pathlineCMFEButtonGroupChanged(int)));

}

// ****************************************************************************
// Method: QvisLCSWindow::CreateAdvancedTab
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
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
// ****************************************************************************

void
QvisLCSWindow::CreateAdvancedTab(QWidget *pageAdvanced)
{
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
    maxSLCount->setKeyboardTracking(false);
    maxSLCount->setMinimum(1);
    maxSLCount->setMaximum(100000);
    connect(maxSLCount, SIGNAL(valueChanged(int)),
            this, SLOT(maxSLCountChanged(int)));
    algoGLayout->addWidget( maxSLCountLabel, 2,0);
    algoGLayout->addWidget( maxSLCount,2,1);

    maxDomainCacheLabel = new QLabel(tr("Domain cache size"), algoGrp);
    maxDomainCache = new QSpinBox(algoGrp);
    maxDomainCache->setKeyboardTracking(false);
    maxDomainCache->setMinimum(1);
    maxDomainCache->setMaximum(100000);
    connect(maxDomainCache, SIGNAL(valueChanged(int)),
            this, SLOT(maxDomainCacheChanged(int)));
    algoGLayout->addWidget( maxDomainCacheLabel, 3,0);
    algoGLayout->addWidget( maxDomainCache, 3,1);

    workGroupSizeLabel = new QLabel(tr("Work group size"), algoGrp);
    workGroupSize = new QSpinBox(algoGrp);
    workGroupSize->setKeyboardTracking(false);
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

    issueWarningForAdvection = new QCheckBox(central);
    connect(issueWarningForAdvection, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForAdvectionChanged(bool)));
    warningsGLayout->addWidget(issueWarningForAdvection, 0, 0);
    QLabel *advectionLabel = new QLabel(tr("Issue warning if the advection limit is not reached."), warningsGrp);
    warningsGLayout->addWidget(advectionLabel, 0, 1, 1, 2);

    issueWarningForBoundary = new QCheckBox(central);
    connect(issueWarningForBoundary, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForBoundaryChanged(bool)));
    warningsGLayout->addWidget(issueWarningForBoundary, 1, 0);
    QLabel *boundaryLabel = new QLabel(tr("Issue warning if the spatial boundary is reached."), warningsGrp);
    warningsGLayout->addWidget(boundaryLabel, 1, 1, 1, 2);

    issueWarningForMaxSteps = new QCheckBox(central);
    connect(issueWarningForMaxSteps, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForMaxStepsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForMaxSteps, 2, 0);
    QLabel *maxStepsLabel = new QLabel(tr("Issue warning when the maximum number of steps is reached."), warningsGrp);
    warningsGLayout->addWidget(maxStepsLabel, 2, 1, 1, 2);

    issueWarningForStepsize = new QCheckBox(central);
    connect(issueWarningForStepsize, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForStepsizeChanged(bool)));
    warningsGLayout->addWidget(issueWarningForStepsize, 3, 0);
    QLabel *stepsizeLabel = new QLabel(tr("Issue warning when a step size underflow is detected."), warningsGrp);
    warningsGLayout->addWidget(stepsizeLabel, 3, 1, 1, 2);

    issueWarningForStiffness = new QCheckBox(central);
    connect(issueWarningForStiffness, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForStiffnessChanged(bool)));
    warningsGLayout->addWidget(issueWarningForStiffness, 4, 0);
    QLabel *stiffnessLabel = new QLabel(tr("Issue warning when a stiffness condition is detected."), warningsGrp);
    warningsGLayout->addWidget(stiffnessLabel, 4, 1, 1, 2);
    QLabel *stiffnessDescLabel1 = new QLabel(tr("(Stiffness refers to one vector component being so much "), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel1, 5, 1, 1, 2);
    QLabel *stiffnessDescLabel2 = new QLabel(tr("larger than another that tolerances can't be met.)"), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel2, 6, 1, 1, 2);

    issueWarningForCriticalPoints = new QCheckBox(central);
    connect(issueWarningForCriticalPoints, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForCriticalPointsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForCriticalPoints, 7, 0);
    QLabel *critPointLabel = new QLabel(tr("Issue warning when a curve doesn't terminate at a critical point."), warningsGrp);
    warningsGLayout->addWidget(critPointLabel, 7, 1, 1, 2);
    QLabel *critPointDescLabel = new QLabel(tr("(I.e. the curve circles around the critical point without stopping.)"), warningsGrp);
    warningsGLayout->addWidget(critPointDescLabel, 8, 1, 1, 2);
    criticalPointThresholdLabel = new QLabel(tr("Speed cutoff for critical points"), warningsGrp);
    criticalPointThresholdLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    warningsGLayout->addWidget(criticalPointThresholdLabel, 9, 1);
    criticalPointThreshold = new QLineEdit(warningsGrp);
    criticalPointThreshold->setAlignment(Qt::AlignLeft);
    connect(criticalPointThreshold, SIGNAL(returnPressed()),
            this, SLOT(criticalPointThresholdProcessText()));
    warningsGLayout->addWidget(criticalPointThreshold, 9, 2);
}


// ****************************************************************************
// Method: QvisLCSWindow::UpdateWindow
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
QvisLCSWindow::UpdateWindow(bool doAll)
{
    bool bval;
    int ival;

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
        case LCSAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(atts->GetSourceType());
            sourceType->blockSignals(false);

            if (atts->GetSourceType() == LCSAttributes::RegularGrid)
            {
                Resolution->setEnabled(true);
                ResolutionLabel->setEnabled(true);
                UseDataSetStart->button(0)->setEnabled(true);
                UseDataSetStart->button(1)->setEnabled(true);
                UseDataSetEnd->button(0)->setEnabled(true);
                UseDataSetEnd->button(1)->setEnabled(true);
                StartPosition->setEnabled(atts->GetUseDataSetStart() ==
                                          LCSAttributes::Subset);
                EndPosition->setEnabled(atts->GetUseDataSetEnd() ==
                                        LCSAttributes::Subset);
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

        case LCSAttributes::ID_Resolution:
            Resolution->setText(IntsToQString(atts->GetResolution(),3));
            break;
        case LCSAttributes::ID_UseDataSetStart:
            UseDataSetStart->blockSignals(true);
            UseDataSetStart->button(atts->GetUseDataSetStart())->setChecked(true);
            UseDataSetStart->blockSignals(false);

            StartPosition->setEnabled(atts->GetUseDataSetStart() ==
                                      LCSAttributes::Subset);

            break;
        case LCSAttributes::ID_StartPosition:
            StartPosition->setText(DoublesToQString(atts->GetStartPosition(), 3));
            break;
        case LCSAttributes::ID_UseDataSetEnd:
            UseDataSetEnd->blockSignals(true);
            UseDataSetEnd->button(atts->GetUseDataSetEnd())->setChecked(true);
            UseDataSetEnd->blockSignals(false);

            EndPosition->setEnabled(atts->GetUseDataSetEnd() ==
                                    LCSAttributes::Subset);
            break;
        case LCSAttributes::ID_EndPosition:
            EndPosition->setText(DoublesToQString(atts->GetEndPosition(), 3));
            break;
        case LCSAttributes::ID_maxStepLength:
            maxStepLength->setText(DoubleToQString(atts->GetMaxStepLength()));
            break;
        case LCSAttributes::ID_limitMaximumTimestep:
            limitMaxTimeStep->blockSignals(true);
            limitMaxTimeStep->setChecked(atts->GetLimitMaximumTimestep());
            limitMaxTimeStep->blockSignals(false);
            maxTimeStep->blockSignals(true);
            if (atts->GetIntegrationType() ==
                LCSAttributes::DormandPrince )
            {
                maxTimeStep->setEnabled(atts->GetLimitMaximumTimestep());
            }
            else
            {
                maxTimeStep->setEnabled(false);
            }
            maxTimeStep->blockSignals(false);
            break;
        case LCSAttributes::ID_maxTimeStep:
            maxTimeStep->setText(DoubleToQString(atts->GetMaxTimeStep()));
            break;
        case LCSAttributes::ID_maxSteps:
            maxSteps->setText(IntToQString(atts->GetMaxSteps()));
            break;

        case LCSAttributes::ID_terminateByDistance:
            limitMaxDistance->blockSignals(true);
            limitMaxDistance->setChecked(atts->GetTerminateByDistance());
            limitMaxDistance->blockSignals(false);
            maxDistance->setEnabled(atts->GetTerminateByDistance());
            break;

        case LCSAttributes::ID_terminateByTime:
            limitMaxTime->blockSignals(true);
            limitMaxTime->setChecked(atts->GetTerminateByTime());
            limitMaxTime->blockSignals(false);
            maxTime->setEnabled(atts->GetTerminateByTime());
            break;

        case LCSAttributes::ID_operationType:
            operationType->blockSignals(true);
            operationType->setCurrentIndex(int(atts->GetOperationType()) );
            operationType->blockSignals(false);

            if( atts->GetOperationType() == LCSAttributes::Lyapunov )
            {
              terminationTypeButtonGroup->blockSignals(true);
              terminationTypeButtonGroup->button(0)->show();
              terminationTypeButtonGroup->button(1)->show();
              terminationTypeButtonGroup->button(2)->show();
              cauchyGreenTensorLabel->show();
              cauchyGreenTensor->show();
              eigenComponent->removeItem(6);
              eigenComponent->removeItem(5);
              eigenComponent->removeItem(4);
              eigenComponent->removeItem(3);
              eigenComponentLabel->show();
              eigenComponent->show();
              maxSize->show();
              limitMaxDistance->hide();
              limitMaxTime->hide();

              atts->SetTerminateByTime(     atts->GetTerminationType() == LCSAttributes::Time);
              atts->SetTerminateByDistance( atts->GetTerminationType() == LCSAttributes::Distance);
              atts->SetTerminateBySize(     atts->GetTerminationType() == LCSAttributes::Size);

//              eigenWeightLabel->hide();
              eigenWeightEdit->hide();
              eigenWeightSlider->hide();
              operatorType->hide();
              clampLogValues->show();

              terminationTypeButtonGroup->blockSignals(false);
            }
            else
            {
              terminationTypeButtonGroup->button(0)->hide();
              terminationTypeButtonGroup->button(1)->hide();
              terminationTypeButtonGroup->button(2)->hide();

              if( atts->GetOperationType() == LCSAttributes::EigenVector )
              {
                if( eigenComponent->itemText(3).isNull() ||
                    eigenComponent->itemText(3).isEmpty() )
                {
                  eigenComponent->addItem(tr("Shear Pos."));
                  eigenComponent->addItem(tr("Shear Neg."));
                  eigenComponent->addItem(tr("Lambda Shear Pos."));
                  eigenComponent->addItem(tr("Lambda Shear Neg."));
                }

                bval = (atts->GetEigenComponent() ==
                        LCSAttributes::PosLambdaShearVector ||
                        atts->GetEigenComponent() ==
                        LCSAttributes::NegLambdaShearVector);

                if( bval )
                {
//                  eigenWeightLabel->show();
                    eigenWeightEdit->show();
                    eigenWeightSlider->show();
                }
                else
                {
//                  eigenWeightLabel->hide();
                    eigenWeightEdit->hide();
                    eigenWeightSlider->hide();
                }

//              eigenWeightLabel->setEnabled( bval );
                eigenWeightEdit->setEnabled( bval );
                eigenWeightSlider->setEnabled( bval );
              }
              else
              {
                if( !(eigenComponent->itemText(3).isNull()) &&
                    !(eigenComponent->itemText(3).isEmpty()) )
                  {
                    eigenComponent->removeItem(6);
                    eigenComponent->removeItem(5);
                    eigenComponent->removeItem(4);
                    eigenComponent->removeItem(3);
                  }
//                eigenWeightLabel->hide();
                eigenWeightEdit->hide();
                eigenWeightSlider->hide();
              }

              if( atts->GetOperationType() == LCSAttributes::EigenValue ||
                  atts->GetOperationType() == LCSAttributes::EigenVector )
              {
                cauchyGreenTensorLabel->show();
                cauchyGreenTensor->show();
                eigenComponentLabel->show();
                eigenComponent->show();
                operatorType->hide();
              }
              else
              {
                cauchyGreenTensorLabel->hide();
                cauchyGreenTensor->hide();
                eigenComponentLabel->hide();
                eigenComponent->hide();
                operatorType->show();
              }

              maxSize->hide();
              limitMaxDistance->show();
              limitMaxTime->show();
              if( atts->GetOperatorType() == LCSAttributes::BaseValue)
                clampLogValues->hide();
              else
                clampLogValues->show();
            }
            break;

        case LCSAttributes::ID_cauchyGreenTensor:
            cauchyGreenTensor->blockSignals(true);
            cauchyGreenTensor->setCurrentIndex(int(atts->GetCauchyGreenTensor()) );
            cauchyGreenTensor->blockSignals(false);
            break;

        case LCSAttributes::ID_eigenComponent:
            eigenComponent->blockSignals(true);
            eigenComponent->setCurrentIndex(int(atts->GetEigenComponent()) );
            eigenComponent->blockSignals(false);

            bval = (atts->GetEigenComponent() ==
                    LCSAttributes::PosLambdaShearVector ||
                    atts->GetEigenComponent() ==
                    LCSAttributes::NegLambdaShearVector);

            if( bval )
            {
//            eigenWeightLabel->show();
              eigenWeightEdit->show();
              eigenWeightSlider->show();
            }
            else
            {
//            eigenWeightLabel->hide();
              eigenWeightEdit->hide();
              eigenWeightSlider->hide();
            }

//          eigenWeightLabel->setEnabled( bval );
            eigenWeightEdit->setEnabled( bval );
            eigenWeightSlider->setEnabled( bval );
            break;

        case LCSAttributes::ID_operatorType:
            operatorType->blockSignals(true);
            operatorType->setCurrentIndex(int(atts->GetOperatorType()) );
            operatorType->blockSignals(false);

            if( atts->GetOperationType() == LCSAttributes::Lyapunov ||
                atts->GetOperatorType() == LCSAttributes::Gradient)
              clampLogValues->show();
            else
              clampLogValues->hide();
            break;

        case LCSAttributes::ID_eigenWeight:
            eigenWeightEdit->setText(DoubleToQString(atts->GetEigenWeight()));

            ival =
              int(qMin(qMax(-100.0,1000.0*(atts->GetEigenWeight()-1.0)),100.0));

            eigenWeightSlider->blockSignals(true);
            eigenWeightSlider->setValue(ival);
            eigenWeightSlider->blockSignals(false);
            break;

        case LCSAttributes::ID_clampLogValues:
            clampLogValues->blockSignals(true);
            clampLogValues->setChecked(atts->GetClampLogValues());
            clampLogValues->blockSignals(false);
            break;

        case LCSAttributes::ID_terminationType:
            terminationTypeButtonGroup->blockSignals(true);
            terminationTypeButtonGroup->button(atts->GetTerminationType())->setChecked(true);
            terminationTypeButtonGroup->blockSignals(false);

            maxTime->setEnabled(atts->GetTerminationType() == LCSAttributes::Time);
            maxDistance->setEnabled(atts->GetTerminationType() == LCSAttributes::Distance);
            maxSize->setEnabled(atts->GetTerminationType() == LCSAttributes::Size);
            break;
        case LCSAttributes::ID_termSize:
            maxSize->setText(DoubleToQString(atts->GetTermSize()));
            break;
        case LCSAttributes::ID_termDistance:
            maxDistance->setText(DoubleToQString(atts->GetTermDistance()));
            break;
        case LCSAttributes::ID_termTime:
            maxTime->setText(DoubleToQString(atts->GetTermTime()));
            break;
        case LCSAttributes::ID_velocitySource:
            velocitySource->setText(DoublesToQString(atts->GetVelocitySource(),3));
            break;

        case LCSAttributes::ID_integrationDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(atts->GetIntegrationDirection()) );
            directionType->blockSignals(false);
            break;
        case LCSAttributes::ID_relTol:
            relTol->setText(DoubleToQString(atts->GetRelTol()));
            break;
        case LCSAttributes::ID_absTolSizeType:
            absTolSizeType->blockSignals(true);
            absTolSizeType->setCurrentIndex((int) atts->GetAbsTolSizeType());
            absTolSizeType->blockSignals(false);
            if (atts->GetAbsTolSizeType() == LCSAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            if (atts->GetAbsTolSizeType() == LCSAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case LCSAttributes::ID_absTolBBox:
            if (atts->GetAbsTolSizeType() == LCSAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            break;
        case LCSAttributes::ID_absTolAbsolute:
            if (atts->GetAbsTolSizeType() == LCSAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case LCSAttributes::ID_auxiliaryGrid:
            auxiliaryGrid->blockSignals(true);
            auxiliaryGrid->setCurrentIndex(atts->GetAuxiliaryGrid());
            auxiliaryGrid->blockSignals(false);

            auxiliaryGridSpacingLabel->setEnabled(!(atts->GetAuxiliaryGrid() ==
                                                    LCSAttributes::None));
            auxiliaryGridSpacing->setEnabled(!(atts->GetAuxiliaryGrid() ==
                                               LCSAttributes::None));
            break;

        case LCSAttributes::ID_auxiliaryGridSpacing:
            auxiliaryGridSpacing->setText(DoubleToQString(atts->GetAuxiliaryGridSpacing()));
            break;

        case LCSAttributes::ID_fieldType:
            // Update lots of widget visibility and enabled states.
            UpdateFieldAttributes();

            fieldType->blockSignals(true);
            fieldType->setCurrentIndex(atts->GetFieldType());
            fieldType->blockSignals(false);

            integrationType->blockSignals(true);
            if (atts->GetFieldType() == LCSAttributes::M3DC12DField)
            {
              atts->SetIntegrationType(LCSAttributes::M3DC12DIntegrator);
              integrationType->setCurrentIndex(LCSAttributes::M3DC12DIntegrator);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetFieldType() == LCSAttributes::NIMRODField)
            {
              atts->SetIntegrationType(LCSAttributes::AdamsBashforth);
              integrationType->setCurrentIndex(LCSAttributes::AdamsBashforth);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetIntegrationType() == LCSAttributes::M3DC12DIntegrator)
            {
              atts->SetIntegrationType(LCSAttributes::DormandPrince);
              integrationType->setCurrentIndex(LCSAttributes::DormandPrince);
              UpdateIntegrationAttributes();
            }
            integrationType->blockSignals(false);

            break;
        case LCSAttributes::ID_fieldConstant:
            fieldConstant->setText(DoubleToQString(atts->GetFieldConstant()));
            break;
        case LCSAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(atts->GetIntegrationType());
            integrationType->blockSignals(false);

            fieldType->blockSignals(true);
            if (atts->GetIntegrationType() == LCSAttributes::M3DC12DIntegrator)
            {
              atts->SetFieldType(LCSAttributes::M3DC12DField);
              fieldType->setCurrentIndex(LCSAttributes::M3DC12DField);
              UpdateFieldAttributes();
            }
            else if (atts->GetFieldType() == LCSAttributes::M3DC12DField)
            {
              atts->SetFieldType(LCSAttributes::Default);
              fieldType->setCurrentIndex(LCSAttributes::Default);
              UpdateFieldAttributes();
            }
            fieldType->blockSignals(false);

            break;

        case LCSAttributes::ID_thresholdLimit:
            thresholdLimit->setText(DoubleToQString(atts->GetThresholdLimit()));
            break;
        case LCSAttributes::ID_radialLimit:
            radialLimit->setText(DoubleToQString(atts->GetRadialLimit()));
            break;
        case LCSAttributes::ID_boundaryLimit:
            boundaryLimit->setText(DoubleToQString(atts->GetBoundaryLimit()));
            break;
        case LCSAttributes::ID_seedLimit:
            seedLimit->blockSignals(true);
            seedLimit->setValue(atts->GetSeedLimit());
            seedLimit->blockSignals(false);
            break;

        case LCSAttributes::ID_parallelizationAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();
            parallelAlgo->blockSignals(true);
            parallelAlgo->setCurrentIndex(atts->GetParallelizationAlgorithmType());
            parallelAlgo->blockSignals(false);
            break;

        case LCSAttributes::ID_maxProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(atts->GetMaxProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case LCSAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(atts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case LCSAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(atts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case LCSAttributes::ID_pathlines:
            icButtonGroup->blockSignals(true);
            icButtonGroup->button(atts->GetPathlines()?1:0)->setChecked(true);
            pathlineOverrideStartingTimeFlag->setEnabled(atts->GetPathlines());
            if( pathlineOverrideStartingTimeFlag->isChecked() && ! icButtonGroup->button(1)->isChecked() )
                pathlineOverrideStartingTimeFlag->setChecked(false);
            pathlineOverrideStartingTime->setEnabled(atts->GetPathlines() && atts->GetPathlinesOverrideStartingTimeFlag());
            pathlinePeriod->setEnabled(atts->GetPathlines());
            pathlineCMFEButtonGroup->button(0)->setEnabled(atts->GetPathlines());
            pathlineCMFEButtonGroup->button(1)->setEnabled(atts->GetPathlines());
            icButtonGroup->blockSignals(false);
            break;
        case LCSAttributes::ID_pathlinesOverrideStartingTimeFlag:
            pathlineOverrideStartingTimeFlag->blockSignals(true);
            pathlineOverrideStartingTimeFlag->setChecked(atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTime->setEnabled(atts->GetPathlines() && atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTimeFlag->blockSignals(false);
            break;
        case LCSAttributes::ID_pathlinesOverrideStartingTime:
            pathlineOverrideStartingTime->setText(DoubleToQString(atts->GetPathlinesOverrideStartingTime()));
            break;
        case LCSAttributes::ID_pathlinesPeriod:
            pathlinePeriod->setText(DoubleToQString(atts->GetPathlinesPeriod()));
            break;
        case LCSAttributes::ID_pathlinesCMFE:
            pathlineCMFEButtonGroup->blockSignals(true);
            pathlineCMFEButtonGroup->button(atts->GetPathlinesCMFE())->setChecked(true);
            pathlineCMFEButtonGroup->blockSignals(false);
            break;

        // case LCSAttributes::ID_forceNodeCenteredData:
        //     forceNodal->blockSignals(true);
        //     forceNodal->setChecked(atts->GetForceNodeCenteredData());
        //     forceNodal->blockSignals(false);
        //     break;

        case LCSAttributes::ID_issueAdvectionWarnings:
            issueWarningForAdvection->blockSignals(true);
            issueWarningForAdvection->setChecked(atts->GetIssueAdvectionWarnings());
            issueWarningForAdvection->blockSignals(false);
            break;

        case LCSAttributes::ID_issueBoundaryWarnings:
            issueWarningForBoundary->blockSignals(true);
            issueWarningForBoundary->setChecked(atts->GetIssueBoundaryWarnings());
            issueWarningForBoundary->blockSignals(false);
            break;

        case LCSAttributes::ID_issueTerminationWarnings:
            issueWarningForMaxSteps->blockSignals(true);
            issueWarningForMaxSteps->setChecked(atts->GetIssueTerminationWarnings());
            issueWarningForMaxSteps->blockSignals(false);
            break;

        case LCSAttributes::ID_issueCriticalPointsWarnings:
            issueWarningForCriticalPoints->blockSignals(true);
            issueWarningForCriticalPoints->setChecked(atts->GetIssueCriticalPointsWarnings());
            criticalPointThreshold->setEnabled(atts->GetIssueCriticalPointsWarnings());
            criticalPointThresholdLabel->setEnabled(atts->GetIssueCriticalPointsWarnings());
            issueWarningForCriticalPoints->blockSignals(false);
            break;

        case LCSAttributes::ID_issueStepsizeWarnings:
            issueWarningForStepsize->blockSignals(true);
            issueWarningForStepsize->setChecked(atts->GetIssueStepsizeWarnings());
            issueWarningForStepsize->blockSignals(false);
            break;

        case LCSAttributes::ID_issueStiffnessWarnings:
            issueWarningForStiffness->blockSignals(true);
            issueWarningForStiffness->setChecked(atts->GetIssueStiffnessWarnings());
            issueWarningForStiffness->blockSignals(false);
            break;

        case LCSAttributes::ID_criticalPointThreshold:
            criticalPointThreshold->setText(DoubleToQString(atts->GetCriticalPointThreshold()));
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisLCSWindow::UpdateFieldAttributes
//
// Purpose:
//   Updates the widgets for the various field types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// ****************************************************************************

void
QvisLCSWindow::UpdateFieldAttributes()
{
    switch( atts->GetFieldType() )
    {
    case LCSAttributes::M3DC12DField:
      if( atts->GetIntegrationType() ==
          LCSAttributes::M3DC12DIntegrator )
        TurnOn(fieldConstant, fieldConstantLabel);
      else
        TurnOff(fieldConstant, fieldConstantLabel);

      TurnOff(velocitySource, velocitySourceLabel);

      break;

    case LCSAttributes::FlashField:
      TurnOn(fieldConstant, fieldConstantLabel);
      TurnOn(velocitySource, velocitySourceLabel);
      break;

    case LCSAttributes::NIMRODField:
    default:
      TurnOff(fieldConstant, fieldConstantLabel);
      TurnOff(velocitySource, velocitySourceLabel);

      break;
    }
}


// ****************************************************************************
// Method: QvisLCSWindow::UpdateIntegrationAttributes
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
QvisLCSWindow::UpdateIntegrationAttributes()
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

    switch( atts->GetIntegrationType() )
    {
    case LCSAttributes::Euler:
    case LCSAttributes::Leapfrog:
    case LCSAttributes::RK4:
        maxStepLength->show();
        maxStepLengthLabel->show();
      break;

    case LCSAttributes::DormandPrince:
        limitMaxTimeStep->show();
        maxTimeStep->show();
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
        absTolSizeType->show();
        break;

    case LCSAttributes::AdamsBashforth:
    case LCSAttributes::M3DC12DIntegrator:
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
// Method: QvisLCSWindow::UpdateAlgorithmAttributes
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
QvisLCSWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = (atts->GetParallelizationAlgorithmType() ==
                            LCSAttributes::LoadOnDemand);
    bool useStaticDomains = (atts->GetParallelizationAlgorithmType() ==
                             LCSAttributes::ParallelStaticDomains);
    bool useMasterSlave = (atts->GetParallelizationAlgorithmType() ==
                           LCSAttributes::MasterSlave);

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
// Method: QvisLCSWindow::GetCurrentValues
//
// Purpose:
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Ensure values are retrieved from spin boxes.
//
// ****************************************************************************

void
QvisLCSWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg;

    // Do resolution
    if(which_widget == LCSAttributes::ID_Resolution || doAll)
    {
        int val[3];
        if(LineEditGetInts(Resolution, val, 3))
            atts->SetResolution(val);
        else
        {
            ResettingError(tr("Resolution"),
                IntsToQString(atts->GetResolution(), 3));
            atts->SetResolution(atts->GetResolution());
        }
    }

    // Do Start Position
    if(which_widget == LCSAttributes::ID_StartPosition || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(StartPosition, val, 3))
            atts->SetStartPosition(val);
        else
        {
            ResettingError(tr("StartPosition"),
                DoublesToQString(atts->GetStartPosition(), 3));
            atts->SetStartPosition(atts->GetStartPosition());
        }
    }

    // Do End Position
    if(which_widget == LCSAttributes::ID_EndPosition || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(EndPosition, val, 3))
            atts->SetEndPosition(val);
        else
        {
            ResettingError(tr("EndPosition"),
                DoublesToQString(atts->GetEndPosition(), 3));
            atts->SetEndPosition(atts->GetEndPosition());
        }
    }


    // Do auxiliaryGridSpacing
    if(which_widget == LCSAttributes::ID_auxiliaryGridSpacing || doAll)
    {
        double val;
        if(LineEditGetDouble(auxiliaryGridSpacing, val))
            atts->SetAuxiliaryGridSpacing(val);
        else
        {
            ResettingError(tr("auxiliary grid spacing"),
                DoubleToQString(atts->GetAuxiliaryGridSpacing()));
            atts->SetAuxiliaryGridSpacing(atts->GetAuxiliaryGridSpacing());
        }
    }

    // Do fieldConstant
    if(which_widget == LCSAttributes::ID_fieldConstant || doAll)
    {
        double val;
        if(LineEditGetDouble(fieldConstant, val))
            atts->SetFieldConstant(val);
        else
        {
            ResettingError(tr("field constant"),
                DoubleToQString(atts->GetFieldConstant()));
            atts->SetFieldConstant(atts->GetFieldConstant());
        }
    }

    // Do stepLength
    if(which_widget == LCSAttributes::ID_maxStepLength || doAll)
    {
        double val;
        if(LineEditGetDouble(maxStepLength, val))
            atts->SetMaxStepLength(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(atts->GetMaxStepLength()));
            atts->SetMaxStepLength(atts->GetMaxStepLength());
        }
    }

    // Do max time step
    if(which_widget == LCSAttributes::ID_maxTimeStep || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTimeStep, val))
            atts->SetMaxTimeStep(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(atts->GetMaxTimeStep()));
            atts->SetMaxTimeStep(atts->GetMaxTimeStep());
        }
    }

    // Do termination
    if(which_widget == LCSAttributes::ID_eigenWeight || doAll)
    {
        double val;
        if(LineEditGetDouble(eigenWeightEdit, val) && 0.9 <= val && val <= 1.1)
            atts->SetEigenWeight(val);
        else
        {
            ResettingError(tr("eigen weight"),
                DoubleToQString(atts->GetEigenWeight()));
            atts->SetEigenWeight(atts->GetEigenWeight());
        }
    }

    if(which_widget == LCSAttributes::ID_maxSteps || doAll)
    {
        int val;
        if(LineEditGetInt(maxSteps, val))
            atts->SetMaxSteps(val);
        else
        {
            ResettingError(tr("max steps"),
                IntToQString(atts->GetMaxSteps()));
            atts->SetMaxSteps(atts->GetMaxSteps());
        }
    }
    if(which_widget == LCSAttributes::ID_termTime || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTime, val))
            atts->SetTermTime(val);
        else
        {
            ResettingError(tr("max time"),
                DoubleToQString(atts->GetTermTime()));
            atts->SetTermTime(atts->GetTermTime());
        }
    }
    if(which_widget == LCSAttributes::ID_termDistance || doAll)
    {
        double val;
        if(LineEditGetDouble(maxDistance, val))
            atts->SetTermDistance(val);
        else
        {
            ResettingError(tr("max distance"),
                DoubleToQString(atts->GetTermDistance()));
            atts->SetTermDistance(atts->GetTermDistance());
        }
    }
    if(which_widget == LCSAttributes::ID_termSize || doAll)
    {
        double val;
        if(LineEditGetDouble(maxSize, val))
            atts->SetTermSize(val);
        else
        {
            ResettingError(tr("max size"),
                DoubleToQString(atts->GetTermSize()));
            atts->SetTermSize(atts->GetTermSize());
        }
    }
    if(which_widget == LCSAttributes::ID_pathlinesOverrideStartingTime || doAll)
    {
        double val;
        if(LineEditGetDouble(pathlineOverrideStartingTime, val))
            atts->SetPathlinesOverrideStartingTime(val);
        else
        {
            ResettingError(tr("Pathlines Override Starting Time"),
                DoubleToQString(atts->GetPathlinesOverrideStartingTime()));
            atts->SetPathlinesOverrideStartingTime(atts->GetPathlinesOverrideStartingTime());
        }
    }
    if(which_widget == LCSAttributes::ID_pathlinesPeriod || doAll)
    {
        double val;
        if(LineEditGetDouble(pathlinePeriod, val))
            atts->SetPathlinesPeriod(val);
        else
        {
            ResettingError(tr("Pathlines Period"),
                DoubleToQString(atts->GetPathlinesPeriod()));
            atts->SetPathlinesPeriod(atts->GetPathlinesPeriod());
        }
    }


    // Do relTol
    if(which_widget == LCSAttributes::ID_relTol || doAll)
    {
        double val;
        if(LineEditGetDouble(relTol, val))
            atts->SetRelTol(val);
        else
        {
            ResettingError(tr("relative tolerance"),
                DoubleToQString(atts->GetRelTol()));
            atts->SetRelTol(atts->GetRelTol());
        }
    }

    // Do absTol
    if ((which_widget == LCSAttributes::ID_absTolBBox || doAll)
        && atts->GetAbsTolSizeType() == LCSAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            atts->SetAbsTolBBox(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(atts->GetAbsTolBBox()));
                atts->SetAbsTolBBox(atts->GetAbsTolBBox());
        }
    }
    if ((which_widget == LCSAttributes::ID_absTolAbsolute || doAll)
        && atts->GetAbsTolSizeType() == LCSAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            atts->SetAbsTolAbsolute(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(atts->GetAbsTolAbsolute()));
                atts->SetAbsTolAbsolute(atts->GetAbsTolAbsolute());
        }
    }

    // Do velocitySource
    if(which_widget == LCSAttributes::ID_velocitySource || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(velocitySource, val, 3))
            atts->SetVelocitySource(val);
        else
        {
            ResettingError(tr("velocity source"),
                DoublesToQString(atts->GetVelocitySource(), 3));
            atts->SetVelocitySource(atts->GetVelocitySource());
        }
    }


    if(which_widget == LCSAttributes::ID_thresholdLimit || doAll)
    {
        double val;
        if(LineEditGetDouble(thresholdLimit, val))
            atts->SetThresholdLimit(val);
        else
        {
            ResettingError(tr("Threshold Limit"),
                DoubleToQString(atts->GetThresholdLimit()));
            atts->SetThresholdLimit(atts->GetThresholdLimit());
        }
    }

    if(which_widget == LCSAttributes::ID_radialLimit || doAll)
    {
        double val;
        if(LineEditGetDouble(radialLimit, val))
            atts->SetRadialLimit(val);
        else
        {
            ResettingError(tr("Radial Limit"),
                DoubleToQString(atts->GetRadialLimit()));
            atts->SetRadialLimit(atts->GetRadialLimit());
        }
    }

    if(which_widget == LCSAttributes::ID_boundaryLimit || doAll)
    {
        double val;
        if(LineEditGetDouble(boundaryLimit, val))
            atts->SetBoundaryLimit(val);
        else
        {
            ResettingError(tr("Boundary Limit"),
                DoubleToQString(atts->GetBoundaryLimit()));
            atts->SetBoundaryLimit(atts->GetBoundaryLimit());
        }
    }

    // seedLimit
    if (which_widget == LCSAttributes::ID_seedLimit || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = seedLimit->value();
        if (val >= 0)
            atts->SetSeedLimit(val);
    }


    // maxProcessCount
    if (which_widget == LCSAttributes::ID_maxProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            atts->SetMaxProcessCount(val);
    }

    // workGroupSize
    if (which_widget == LCSAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            atts->SetWorkGroupSize(val);
    }

    // criticalPointThreshold
    if(which_widget == LCSAttributes::ID_criticalPointThreshold || doAll)
    {
        double val;
        if(LineEditGetDouble(criticalPointThreshold, val))
            atts->SetCriticalPointThreshold(val);
        else
        {
            ResettingError(tr("Speed cutoff for critical points"),
                DoubleToQString(atts->GetCriticalPointThreshold()));
            atts->SetCriticalPointThreshold(atts->GetCriticalPointThreshold());
        }
    }
    // maxDomainCache
    if (which_widget == LCSAttributes::ID_maxDomainCacheSize || doAll)
    {
        if (maxDomainCache->value() != atts->GetMaxDomainCacheSize())
            atts->SetMaxDomainCacheSize(maxDomainCache->value());
    }
}


void
QvisLCSWindow::sourceTypeChanged(int val)
{
    if(val != atts->GetSourceType())
    {
        atts->SetSourceType(LCSAttributes::SourceType(val));
        Apply();
    }
}


void
QvisLCSWindow::ResolutionProcessText()
{
    GetCurrentValues(LCSAttributes::ID_Resolution);
    Apply();
}


void
QvisLCSWindow::UseDataSetStartChanged(int val)
{
    atts->SetUseDataSetStart(LCSAttributes::Extents(val));
    Apply();
}


void
QvisLCSWindow::StartPositionProcessText()
{
    GetCurrentValues(LCSAttributes::ID_StartPosition);
    Apply();
}


void
QvisLCSWindow::UseDataSetEndChanged(int val)
{
  atts->SetUseDataSetEnd(LCSAttributes::Extents(val));
    Apply();
}


void
QvisLCSWindow::EndPositionProcessText()
{
    GetCurrentValues(LCSAttributes::ID_EndPosition);
    Apply();
}


void
QvisLCSWindow::directionTypeChanged(int val)
 {
    if(val != atts->GetIntegrationDirection())
    {
        atts->SetIntegrationDirection(LCSAttributes::IntegrationDirection(val));
        Apply();
    }
}

void
QvisLCSWindow::fieldTypeChanged(int val)
 {
    if(val != atts->GetFieldType())
    {
        atts->SetFieldType(LCSAttributes::FieldType(val));
        Apply();
    }
}

void
QvisLCSWindow::fieldConstantProcessText()
{
    GetCurrentValues(LCSAttributes::ID_fieldConstant);
    Apply();
}

void
QvisLCSWindow::auxiliaryGridChanged(int val)
 {
    if(val != atts->GetAuxiliaryGrid())
    {
        atts->SetAuxiliaryGrid(LCSAttributes::AuxiliaryGrid(val));
        Apply();
    }
}

void
QvisLCSWindow::auxiliaryGridSpacingProcessText()
{
    GetCurrentValues(LCSAttributes::ID_auxiliaryGridSpacing);
    Apply();
}

void
QvisLCSWindow::integrationTypeChanged(int val)
{
    if(val != atts->GetIntegrationType())
    {
        atts->SetIntegrationType(LCSAttributes::IntegrationType(val));
        Apply();
    }
}

void
QvisLCSWindow::parallelAlgorithmChanged(int val)
{
    if(val != atts->GetParallelizationAlgorithmType())
    {
        atts->SetParallelizationAlgorithmType(LCSAttributes::ParallelizationAlgorithmType(val));
        Apply();
    }
}

void
QvisLCSWindow::maxStepLengthProcessText()
{
    GetCurrentValues(LCSAttributes::ID_maxStepLength);
    Apply();
}

void
QvisLCSWindow::maxTimeStepProcessText()
{
    GetCurrentValues(LCSAttributes::ID_maxTimeStep);
    Apply();
}

void
QvisLCSWindow::maxStepsProcessText()
{
    GetCurrentValues(LCSAttributes::ID_maxSteps);
    Apply();
}

void
QvisLCSWindow::limitMaxTimeChanged(bool val)
{
    if(val != atts->GetTerminateByTime())
    {
        atts->SetTerminateByTime(val);
        Apply();
    }
}

void
QvisLCSWindow::limitMaxDistanceChanged(bool val)
{
    if(val != atts->GetTerminateByDistance())
    {
        atts->SetTerminateByDistance(val);
        Apply();
    }
}

void
QvisLCSWindow::operationTypeChanged(int val)
 {
    if(val != atts->GetOperationType())
    {
        atts->SetOperationType(LCSAttributes::OperationType(val));
        Apply();
    }
}

void
QvisLCSWindow::cauchyGreenTensorChanged(int val)
 {
    if(val != atts->GetCauchyGreenTensor())
    {
        atts->SetCauchyGreenTensor(LCSAttributes::CauchyGreenTensor(val));
        Apply();
    }
}

void
QvisLCSWindow::eigenComponentChanged(int val)
 {
    if(val != atts->GetEigenComponent())
    {
        atts->SetEigenComponent(LCSAttributes::EigenComponent(val));
        Apply();
    }
}

void
QvisLCSWindow::operatorTypeChanged(int val)
 {
    if(val != atts->GetOperatorType())
    {
        atts->SetOperatorType(LCSAttributes::OperatorType(val));
        Apply();
    }
}

void
QvisLCSWindow::eigenWeightEditProcessText()
{
    GetCurrentValues(LCSAttributes::ID_eigenWeight);
    Apply();
}

void
QvisLCSWindow::eigenWeightSliderChanged(int val)
{
    atts->SetEigenWeight( 1.0 + (double) val/1000.0 );

    eigenWeightEdit->setText(DoubleToQString( (double) 1.0 + val/1000.0 ) );

    if (!sliderDragging)
        Apply();
}

void
QvisLCSWindow::eigenWeightSliderPressed()
{
    sliderDragging = true;
}

void
QvisLCSWindow::eigenWeightSliderReleased()
{
    sliderDragging = false;
    Apply();
}

void
QvisLCSWindow::clampLogValuesChanged(bool val)
{
    atts->SetClampLogValues(val);
    Apply();
}

void
QvisLCSWindow::terminationTypeButtonGroupChanged(int index)
{
    atts->SetTerminationType( (LCSAttributes::TerminationType) index );
    atts->SetTerminateByTime( index == 0);
    atts->SetTerminateByDistance( index == 1);
    atts->SetTerminateBySize( index == 2);
    Apply();
}

void
QvisLCSWindow::maxTimeProcessText()
{
    GetCurrentValues(LCSAttributes::ID_termTime);
    Apply();
}

void
QvisLCSWindow::maxDistanceProcessText()
{
    GetCurrentValues(LCSAttributes::ID_termDistance);
    Apply();
}

void
QvisLCSWindow::maxSizeProcessText()
{
    GetCurrentValues(LCSAttributes::ID_termSize);
    Apply();
}

void
QvisLCSWindow::relTolProcessText()
{
    GetCurrentValues(LCSAttributes::ID_relTol);
    Apply();
}

void
QvisLCSWindow::thresholdLimitProcessText()
{
    GetCurrentValues(LCSAttributes::ID_thresholdLimit);
    Apply();
}

void
QvisLCSWindow::radialLimitProcessText()
{
    GetCurrentValues(LCSAttributes::ID_radialLimit);
    Apply();
}

void
QvisLCSWindow::boundaryLimitProcessText()
{
    GetCurrentValues(LCSAttributes::ID_boundaryLimit);
    Apply();
}

void
QvisLCSWindow::seedLimitChanged(int val)
{
    atts->SetSeedLimit(val);
    Apply();
}


void
QvisLCSWindow::maxSLCountChanged(int val)
{
    atts->SetMaxProcessCount(val);
    Apply();
}

void
QvisLCSWindow::maxDomainCacheChanged(int val)
{
    atts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisLCSWindow::workGroupSizeChanged(int val)
{
    atts->SetWorkGroupSize(val);
    Apply();
}

void
QvisLCSWindow::icButtonGroupChanged(int val)
{
    switch( val )
    {
        case 0: // Streamline
            atts->SetPathlines(false);
            break;
        case 1: // Pathline
            atts->SetPathlines(true);
            break;
    }
    Apply();
}

void
QvisLCSWindow::pathlineOverrideStartingTimeFlagChanged(bool val)
{
    atts->SetPathlinesOverrideStartingTimeFlag(val);
    Apply();
}

void
QvisLCSWindow::pathlineOverrideStartingTimeProcessText()
{
    GetCurrentValues(LCSAttributes::ID_pathlinesOverrideStartingTime);
    Apply();
}

void
QvisLCSWindow::pathlinePeriodProcessText()
{
    GetCurrentValues(LCSAttributes::ID_pathlinesPeriod);
    Apply();
}

void
QvisLCSWindow::pathlineCMFEButtonGroupChanged(int val)
{
    atts->SetPathlinesCMFE((LCSAttributes::PathlinesCMFE)val);
    Apply();
}

void
QvisLCSWindow::absTolProcessText()
{
    GetCurrentValues(LCSAttributes::ID_absTolBBox);
    GetCurrentValues(LCSAttributes::ID_absTolAbsolute);
    Apply();
}

void
QvisLCSWindow::absTolSizeTypeChanged(int val)
{
    atts->SetAbsTolSizeType((LCSAttributes::SizeType) val);
    Apply();
}

void
QvisLCSWindow::limitMaxTimeStepChanged(bool val)
{
    atts->SetLimitMaximumTimestep(val);
    Apply();
}

// void
// QvisLCSWindow::forceNodalChanged(bool val)
// {
//     atts->SetForceNodeCenteredData(val);
//     Apply();
// }

void
QvisLCSWindow::velocitySourceProcessText()
{
    GetCurrentValues(LCSAttributes::ID_velocitySource);
    Apply();
}

void
QvisLCSWindow::issueWarningForAdvectionChanged(bool val)
{
    atts->SetIssueAdvectionWarnings(val);
    Apply();
}

void
QvisLCSWindow::issueWarningForBoundaryChanged(bool val)
{
    atts->SetIssueBoundaryWarnings(val);
    Apply();
}

void
QvisLCSWindow::issueWarningForMaxStepsChanged(bool val)
{
    atts->SetIssueTerminationWarnings(val);
    Apply();
}

void
QvisLCSWindow::issueWarningForStepsizeChanged(bool val)
{
    atts->SetIssueStepsizeWarnings(val);
    Apply();
}

void
QvisLCSWindow::issueWarningForStiffnessChanged(bool val)
{
    atts->SetIssueStiffnessWarnings(val);
    Apply();
}

void
QvisLCSWindow::issueWarningForCriticalPointsChanged(bool val)
{
    atts->SetIssueCriticalPointsWarnings(val);
    Apply();
}

void
QvisLCSWindow::criticalPointThresholdProcessText(void)
{
    GetCurrentValues(LCSAttributes::ID_criticalPointThreshold);
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
