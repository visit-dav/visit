// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisLimitCycleWindow.h"

#include <LimitCycleAttributes.h>
#include <ViewerProxy.h>

#include <QTabWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QvisVariableButton.h>
#include <QListWidget>

#include <Plot.h>
#include <PlotList.h>
#include <PlotInfoAttributes.h>


static void
TurnOn(QWidget *w0, QWidget *w1=NULL);
static void
TurnOff(QWidget *w0, QWidget *w1=NULL);

// ****************************************************************************
// Method: QvisLimitCycleWindow::QvisLimitCycleWindow
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

QvisLimitCycleWindow::QvisLimitCycleWindow(const int type,
                                                 LimitCycleAttributes *subj,
                                                 const QString &caption,
                                                 const QString &shortName,
                                                 QvisNotepadArea *notepad)
  : QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisLimitCycleWindow::~QvisLimitCycleWindow
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

QvisLimitCycleWindow::~QvisLimitCycleWindow()
{
}


// ****************************************************************************
// Method: QvisLimitCycleWindow::CreateWindowContents
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
//   Add integral curve direction option.
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
//   Changed label text for integral curve algorithms.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add new coloring methods, length, time and ID.
//
//   Sean Ahern, Wed Sep  3 16:19:27 EDT 2008
//   Tweaked the layout to make it a bit more consistent.
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
//   Add custom renderer and lots of appearance options to the integral curves plots.
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
//   Hank Childs, Fri Oct 22 09:22:18 PDT 2010
//   Rename Adams-Bashforth's "Maximum step length" to "Step length" since
//   it always takes the same size step.
//
//   Hank Childs, Sat Oct 30 11:25:04 PDT 2010
//   Initialize sample density.  Otherwise, atts set from the CLI gets overset,
//   because we set the density based on the value of the text field, and we
//   get a signal that causes us to use the default value before we ever set
//   the field with the correct value.
//
//   Hank Childs, Sun Dec  5 09:52:44 PST 2010
//   Add support for disabling warnings for stiffness and critical points.
//   Also add description of tolerances.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
//   Kathleen Biagas, Fri Nov  8 10:05:16 PST 2019
//   Add source tab for source and field widgets to reduce window height.
//
// ****************************************************************************

void
QvisLimitCycleWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget();
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // SourceField tab
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
// Method: QvisLimitCycleWindow::CreateSourceTab
//
// Purpose:
//   Populates the source tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Kathleen Biagas, Fri Nov  8 09:03:19 PST 2019
//   Content moved from CreateIntegration tab, to reduce window height.
//
// ****************************************************************************

void
QvisLimitCycleWindow::CreateSourceTab(QWidget *pageSource)
{
    QVBoxLayout *sLayout = new QVBoxLayout(pageSource);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    sLayout->addLayout(mainLayout);
    sLayout->addStretch(1);

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
    sourceType->addItem(tr("Line"));
//    sourceType->addItem(tr("Plane"));
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
    samplingTypeLabel = new QLabel(tr("Sampling type:"), samplingGroup);
    samplingLayout->addWidget(samplingTypeLabel, sRow, 0);
    samplingTypeButtonGroup = new QButtonGroup(samplingGroup);
    samplingTypeButtons[0] = new QRadioButton(tr("Uniform"), samplingGroup);
//    samplingTypeButtons[1] = new QRadioButton(tr("Random"), samplingGroup);
    samplingTypeButtons[0]->setChecked(true);
    samplingTypeButtonGroup->addButton(samplingTypeButtons[0], 0);
//    samplingTypeButtonGroup->addButton(samplingTypeButtons[1], 1);
    samplingLayout->addWidget(samplingTypeButtons[0], sRow, 1);
//    samplingLayout->addWidget(samplingTypeButtons[1], sRow, 2);
    connect(samplingTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(samplingTypeChanged(int)));

    fillLabel = new QLabel(tr("Sampling along:"), samplingGroup);
    samplingLayout->addWidget(fillLabel, sRow, 3);
    fillButtonGroup = new QButtonGroup(samplingGroup);
    fillButtons[0] = new QRadioButton(tr("Boundary"), samplingGroup);
    fillButtons[1] = new QRadioButton(tr("Interior"), samplingGroup);
    fillButtons[0]->setChecked(true);
    fillButtonGroup->addButton(fillButtons[0], 0);
    fillButtonGroup->addButton(fillButtons[1], 1);
    samplingLayout->addWidget(fillButtons[0], sRow, 4);
    samplingLayout->addWidget(fillButtons[1], sRow, 5);
    connect(fillButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(fillChanged(int)));

    sRow++;

    numberOfRandomSamplesLabel = new QLabel(tr("Number of random samples"), samplingGroup);
    samplingLayout->addWidget(numberOfRandomSamplesLabel, sRow, 0, 1, 2);
    numberOfRandomSamples = new QSpinBox(samplingGroup);
    numberOfRandomSamples->setKeyboardTracking(false);
    numberOfRandomSamples->setMinimum(1);
    numberOfRandomSamples->setMaximum(100000000);
    connect(numberOfRandomSamples, SIGNAL(valueChanged(int)), this, SLOT(numberOfRandomSamplesChanged(int)));
    samplingLayout->addWidget(numberOfRandomSamples, sRow, 2);

    randomSeedLabel = new QLabel(tr("Random number seed"), samplingGroup);
    samplingLayout->addWidget(randomSeedLabel, sRow, 3, 1, 2);
    randomSeed = new QSpinBox(samplingGroup);
    randomSeed->setKeyboardTracking(false);
    randomSeed->setMinimum(0);
    randomSeed->setMaximum(100000000);
    connect(randomSeed, SIGNAL(valueChanged(int)), this, SLOT(randomSeedChanged(int)));
    samplingLayout->addWidget(randomSeed, sRow, 5);
    randomSeedLabel->setBuddy(randomSeed);

    sRow++;

    sampleDensityLabel[0] = new QLabel(tr("Sample density 0"), samplingGroup);
    sampleDensityLabel[1] = new QLabel(tr("Sample density 1"), samplingGroup);
    sampleDensity[0] = new QSpinBox(samplingGroup);
    sampleDensity[1] = new QSpinBox(samplingGroup);
    sampleDensity[0]->setKeyboardTracking(false);
    sampleDensity[0]->setMinimum(1);
    sampleDensity[0]->setMaximum(10000000);
    sampleDensity[0]->setValue(atts->GetSampleDensity0());
    sampleDensity[1]->setKeyboardTracking(false);
    sampleDensity[1]->setMinimum(1);
    sampleDensity[1]->setMaximum(10000000);
    sampleDensity[1]->setValue(atts->GetSampleDensity1());
    connect(sampleDensity[0], SIGNAL(valueChanged(int)), this, SLOT(sampleDensity0Changed(int)));
    connect(sampleDensity[1], SIGNAL(valueChanged(int)), this, SLOT(sampleDensity1Changed(int)));
    samplingLayout->addWidget(sampleDensityLabel[0], sRow, 0);
    samplingLayout->addWidget(sampleDensity[0], sRow, 1);
    samplingLayout->addWidget(sampleDensityLabel[1], sRow, 2);
    samplingLayout->addWidget(sampleDensity[1], sRow, 3);
    sRow++;

    sampleDistanceLabel[0] = new QLabel(tr("Sample distance 0"), samplingGroup);
    sampleDistanceLabel[1] = new QLabel(tr("Sample distance 1"), samplingGroup);
    sampleDistance[0] = new QLineEdit(samplingGroup);
    sampleDistance[1] = new QLineEdit(samplingGroup);
    connect(sampleDistance[0], SIGNAL(returnPressed()), this, SLOT(sampleDistance0ProcessText()));
    connect(sampleDistance[1], SIGNAL(returnPressed()), this, SLOT(sampleDistance1ProcessText()));
    samplingLayout->addWidget(sampleDistanceLabel[0], sRow, 0);
    samplingLayout->addWidget(sampleDistance[0], sRow, 1);
    samplingLayout->addWidget(sampleDistanceLabel[1], sRow, 2);
    samplingLayout->addWidget(sampleDistance[1], sRow, 3);
    sRow++;

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
    fieldType->addItem(tr("Nektar++"));
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
    // forceNodal = new QCheckBox(tr("Force node centering"), fieldGroup);
    // connect(forceNodal, SIGNAL(toggled(bool)), this, SLOT(forceNodalChanged(bool)));
    // fieldLayout->addWidget(forceNodal, 2, 0);
}


// ****************************************************************************
// Method: QvisLimitCycleWindow::CreateIntegrationTab
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
//   Kathleen Biagas, Fri Nov  8 09:04:47 PST 2019
//   Source and field widgets moved to Source tab, to reduce window height.
//
// ****************************************************************************

void
QvisLimitCycleWindow::CreateIntegrationTab(QWidget *pageIntegration)
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
    directionType->addItem(tr("Both"));
    directionType->addItem(tr("Forward Directionless"));
    directionType->addItem(tr("Backward Directionless"));
    directionType->addItem(tr("Both Directionless"));
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

    // Create the step length text field.
    limitMaxTimeStep = new QCheckBox(tr("Limit maximum time step"), integrationGroup);
    connect(limitMaxTimeStep, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeStepChanged(bool)));
    integrationLayout->addWidget(limitMaxTimeStep, 3, 0);

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
    integrationLayout->addWidget(terminationGroup, 12, 0, 2, 2);
//    integrationLayout->setStretchFactor(terminationGroup, 100);
    QGridLayout *terminationLayout = new QGridLayout(terminationGroup);
    terminationLayout->setMargin(5);
    terminationLayout->setSpacing(10);

    QLabel *maxStepsLabel = new QLabel(tr("Maximum number of steps"), terminationGroup);
    terminationLayout->addWidget(maxStepsLabel, 0,0);
    maxSteps = new QLineEdit(central);
    connect(maxSteps, SIGNAL(returnPressed()),
            this, SLOT(maxStepsProcessText()));
    terminationLayout->addWidget(maxSteps, 0,1);

    limitMaxTime = new QCheckBox(tr("Limit maximum time elapsed for particles"), terminationGroup);
    connect(limitMaxTime, SIGNAL(toggled(bool)), this, SLOT(limitMaxTimeChanged(bool)));
    terminationLayout->addWidget(limitMaxTime, 1,0);
    maxTime = new QLineEdit(central);
    connect(maxTime, SIGNAL(returnPressed()), this, SLOT(maxTimeProcessText()));
    terminationLayout->addWidget(maxTime, 1,1);

    limitMaxDistance = new QCheckBox(tr("Limit maximum distance traveled by particles"), terminationGroup);
    connect(limitMaxDistance, SIGNAL(toggled(bool)), this, SLOT(limitMaxDistanceChanged(bool)));
    terminationLayout->addWidget(limitMaxDistance, 2,0);
    maxDistance = new QLineEdit(central);
    connect(maxDistance, SIGNAL(returnPressed()), this, SLOT(maxDistanceProcessText()));
    terminationLayout->addWidget(maxDistance, 2,1);
}

// ****************************************************************************
// Method: QvisLimitCycleWindow::CreateAppearanceTab
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
//   Dave Pugmire, Mon Feb 21 08:17:42 EST 2011
//   Add color by correlation distance.
//
// ****************************************************************************

void
QvisLimitCycleWindow::CreateAppearanceTab(QWidget *pageAppearance)
{
    QGridLayout *mainLayout = new QGridLayout(pageAppearance);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);


    // Create the cycle group
    QGroupBox *cycleGroup = new QGroupBox(pageAppearance);
    cycleGroup->setTitle(tr("Limit cycle search"));
    mainLayout->addWidget(cycleGroup, 0, 0);

    QGridLayout *cycleLayout = new QGridLayout(cycleGroup);
    cycleLayout->setSpacing(10);
    cycleLayout->setColumnStretch(2,10);

    QLabel *cycleToleranceLabel = new QLabel(tr("Cycle tolerance"), cycleGroup);
    cycleLayout->addWidget(cycleToleranceLabel, 0,0);

    cycleTolerance = new QLineEdit(central);
    connect(cycleTolerance, SIGNAL(returnPressed()),
            this, SLOT(cycleToleranceProcessText()));
    cycleLayout->addWidget(cycleTolerance, 0,1);


    QLabel *maxIterationsLabel = new QLabel(tr("Maximum iterations"), cycleGroup);
    cycleLayout->addWidget(maxIterationsLabel, 1,0);
    maxIterations = new QLineEdit(central);
    connect(maxIterations, SIGNAL(returnPressed()),
            this, SLOT(maxIterationsProcessText()));
    cycleLayout->addWidget(maxIterations, 1,1);


    showPartialResults = new QCheckBox(tr("Show partial results (limit cycle may not be present)"), cycleGroup);
    connect(showPartialResults, SIGNAL(toggled(bool)), this, SLOT(showPartialResultsChanged(bool)));
    cycleLayout->addWidget(showPartialResults, 2,0, 1,2);


    showReturnDistances = new QCheckBox(tr("Show the signed return distances for the first iteration"), cycleGroup);
    connect(showReturnDistances, SIGNAL(toggled(bool)), this, SLOT(showReturnDistancesChanged(bool)));
    cycleLayout->addWidget(showReturnDistances, 3,0, 1,2);

    // Create the data group
    QGroupBox *dataGroup = new QGroupBox(pageAppearance);
    dataGroup->setTitle(tr("Data"));
    mainLayout->addWidget(dataGroup, 2, 0);

    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    dataLayout->setSpacing(10);
    dataLayout->setColumnStretch(2,10);

    // Create the data value.
    dataLayout->addWidget(new QLabel(tr("Data value"), dataGroup), 0, 0);

    dataValueComboBox = new QComboBox(dataGroup);
    dataValueComboBox->addItem(tr("Solid"), LimitCycleAttributes::Solid);
    dataValueComboBox->addItem(tr("Seed point ID"), LimitCycleAttributes::SeedPointID);
    dataValueComboBox->addItem(tr("Speed"), LimitCycleAttributes::Speed);
    dataValueComboBox->addItem(tr("Vorticity magnitude"), LimitCycleAttributes::Vorticity);
    dataValueComboBox->addItem(tr("Arc length"), LimitCycleAttributes::ArcLength);
    dataValueComboBox->addItem(tr("Absolute time"), LimitCycleAttributes::TimeAbsolute);
    dataValueComboBox->addItem(tr("Relative time"), LimitCycleAttributes::TimeRelative);
    dataValueComboBox->addItem(tr("Ave. dist. from seed"), LimitCycleAttributes::AverageDistanceFromSeed);
    dataValueComboBox->addItem(tr("Correlation distance"), LimitCycleAttributes::CorrelationDistance);
    dataValueComboBox->addItem(tr("Difference"), LimitCycleAttributes::Difference);
    dataValueComboBox->addItem(tr("Variable"), LimitCycleAttributes::Variable);
    connect(dataValueComboBox, SIGNAL(activated(int)),
            this, SLOT(dataValueChanged(int)));
    dataLayout->addWidget(dataValueComboBox, 0, 1);

    dataVariable = new QvisVariableButton(false, true, true,
                                         QvisVariableButton::Scalars,
                                         dataGroup);
    dataLayout->addWidget(dataVariable, 0, 2);
    connect(dataVariable, SIGNAL(activated(const QString &)),
            this, SLOT(dataVariableChanged(const QString&)));

    //Correlation distance widgets.
    correlationDistanceAngTolLabel = new QLabel(tr("Angular tolerance (degrees)"), dataGroup);
    correlationDistanceMinDistLabel = new QLabel(tr("Minimum measuring distance"), dataGroup);
    correlationDistanceAngTolEdit = new QLineEdit(dataGroup);
    correlationDistanceMinDistEdit = new QLineEdit(dataGroup);
    correlationDistanceMinDistType = new QComboBox(dataGroup);
    correlationDistanceMinDistType->addItem(tr("Absolute"), 0);
    correlationDistanceMinDistType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(correlationDistanceMinDistType, SIGNAL(activated(int)), this, SLOT(correlationDistanceMinDistTypeChanged(int)));

    dataLayout->addWidget(correlationDistanceAngTolLabel, 1, 0);
    dataLayout->addWidget(correlationDistanceAngTolEdit, 1, 1);
    dataLayout->addWidget(correlationDistanceMinDistLabel, 2, 0);
    dataLayout->addWidget(correlationDistanceMinDistEdit, 2, 1);
    dataLayout->addWidget(correlationDistanceMinDistType, 2, 2);
    connect(correlationDistanceAngTolEdit, SIGNAL(returnPressed()),
            this, SLOT(processCorrelationDistanceAngTolEditText()));
    connect(correlationDistanceMinDistEdit, SIGNAL(returnPressed()),
            this, SLOT(processCorrelationDistanceMinDistEditText()));

    // Streamlines/Pathline Group.
    QGroupBox *icGrp = new QGroupBox(pageAppearance);
    icGrp->setTitle(tr("Streamlines vs Pathlines"));
    mainLayout->addWidget(icGrp, 6, 0);

    QGridLayout *icGrpLayout = new QGridLayout(icGrp);
    icGrpLayout->setSpacing(10);
    icGrpLayout->setColumnStretch(1,10);

    icButtonGroup = new QButtonGroup(icGrp);
    QRadioButton *streamlineButton = new QRadioButton(tr("Streamline\n    Compute trajectories in an (instantaneous) snapshot of the vector field.\n    Uses and loads vector data from only the current time slice."), icGrp);
    QRadioButton *pathlineButton = new QRadioButton(tr("Pathline    \n    Compute trajectories in the time-varying vector field.\n    Uses and loads vector data from all relevant time slices"), icGrp);
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

    pathlineOverrideStartingTimeFlag = new QCheckBox(tr("Override starting time"), pathlineOptionsGrp);
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
// Method: QvisLimitCycleWindow::CreateAdvancedTab
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
QvisLimitCycleWindow::CreateAdvancedTab(QWidget *pageAdvanced)
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

    issueWarningForMaxSteps = new QCheckBox(central);
    connect(issueWarningForMaxSteps, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForMaxStepsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForMaxSteps, 0, 0);
    QLabel *maxStepsLabel = new QLabel(tr("Issue warning when the maximum number of steps is reached."), warningsGrp);
    warningsGLayout->addWidget(maxStepsLabel, 0, 1, 1, 2);

    issueWarningForStepsize = new QCheckBox(central);
    connect(issueWarningForStepsize, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForStepsizeChanged(bool)));
    warningsGLayout->addWidget(issueWarningForStepsize, 1, 0);
    QLabel *stepsizeLabel = new QLabel(tr("Issue warning when a step size underflow is detected."), warningsGrp);
    warningsGLayout->addWidget(stepsizeLabel, 1, 1, 1, 2);

    issueWarningForStiffness = new QCheckBox(central);
    connect(issueWarningForStiffness, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForStiffnessChanged(bool)));
    warningsGLayout->addWidget(issueWarningForStiffness, 2, 0);
    QLabel *stiffnessLabel = new QLabel(tr("Issue warning when a stiffness condition is detected."), warningsGrp);
    warningsGLayout->addWidget(stiffnessLabel, 2, 1, 1, 2);
    QLabel *stiffnessDescLabel1 = new QLabel(tr("(Stiffness refers to one vector component being so much "), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel1, 3, 1, 1, 2);
    QLabel *stiffnessDescLabel2 = new QLabel(tr("larger than another that tolerances can't be met.)"), warningsGrp);
    warningsGLayout->addWidget(stiffnessDescLabel2, 4, 1, 1, 2);

    issueWarningForCriticalPoints = new QCheckBox(central);
    connect(issueWarningForCriticalPoints, SIGNAL(toggled(bool)),
            this, SLOT(issueWarningForCriticalPointsChanged(bool)));
    warningsGLayout->addWidget(issueWarningForCriticalPoints, 5, 0);
    QLabel *critPointLabel = new QLabel(tr("Issue warning when a curve doesn't terminate at a critical point."), warningsGrp);
    warningsGLayout->addWidget(critPointLabel, 5, 1, 1, 2);
    QLabel *critPointDescLabel = new QLabel(tr("(I.e. the curve circles around the critical point without stopping.)"), warningsGrp);
    warningsGLayout->addWidget(critPointDescLabel, 6, 1, 1, 2);
    criticalPointThresholdLabel = new QLabel(tr("Speed cutoff for critical points"), warningsGrp);
    criticalPointThresholdLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    warningsGLayout->addWidget(criticalPointThresholdLabel, 7, 1);
    criticalPointThreshold = new QLineEdit(warningsGrp);
    criticalPointThreshold->setAlignment(Qt::AlignLeft);
    connect(criticalPointThreshold, SIGNAL(returnPressed()),
            this, SLOT(criticalPointThresholdProcessText()));
    warningsGLayout->addWidget(criticalPointThreshold, 7, 2);
}

// ****************************************************************************
// Method: QvisLimitCycleWindow::UpdateWindow
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
//   Add integral curve direction option.
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
//   Add custom renderer and lots of appearance options to the integral curves plots.
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
//   Hank Childs, Sun Dec  5 09:52:44 PST 2010
//   Add support for disabling warnings for stiffness and critical points.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisLimitCycleWindow::UpdateWindow(bool doAll)
{
    // Attach and detach this operator to the plot information.
    if( SelectedSubject() == GetViewerState()->GetOperatorAttributes(plotType) )
    {
      // Detach from plot information objects (maybe keep a list of
      // ones this window observes)
      for( int i = 0; i < GetViewerState()->GetNumStateObjects(); ++i)
      {
        PlotInfoAttributes *info = GetViewerState()->GetPlotInformation(i);

        if( info )
          info->Detach(this);
      }

      // Attach to the selected plot types
      for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
      {
        const Plot &p = GetViewerState()->GetPlotList()->GetPlots(i);
        GetViewerState()->GetPlotInformation(p.GetPlotType())->Attach(this);
      }
    }

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
        case LimitCycleAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.
            UpdateSourceAttributes();

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(atts->GetSourceType());
            sourceType->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_maxStepLength:
            maxStepLength->setText(DoubleToQString(atts->GetMaxStepLength()));
            break;
        case LimitCycleAttributes::ID_limitMaximumTimestep:
            limitMaxTimeStep->blockSignals(true);
            limitMaxTimeStep->setChecked(atts->GetLimitMaximumTimestep());
            limitMaxTimeStep->blockSignals(false);
            maxTimeStep->blockSignals(true);
            if (atts->GetIntegrationType() ==
                LimitCycleAttributes::DormandPrince )
            {
                maxTimeStep->setEnabled(atts->GetLimitMaximumTimestep());
            }
            else
            {
                maxTimeStep->setEnabled(false);
            }
            maxTimeStep->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_maxTimeStep:
            maxTimeStep->setText(DoubleToQString(atts->GetMaxTimeStep()));
            break;
        case LimitCycleAttributes::ID_maxSteps:
            maxSteps->setText(IntToQString(atts->GetMaxSteps()));
            break;
        case LimitCycleAttributes::ID_terminateByDistance:
            limitMaxDistance->blockSignals(true);
            limitMaxDistance->setChecked(atts->GetTerminateByDistance());
            limitMaxDistance->blockSignals(false);
            maxDistance->setEnabled(atts->GetTerminateByDistance());
            break;
        case LimitCycleAttributes::ID_termDistance:
            maxDistance->setText(DoubleToQString(atts->GetTermDistance()));
            break;
        case LimitCycleAttributes::ID_terminateByTime:
            limitMaxTime->blockSignals(true);
            limitMaxTime->setChecked(atts->GetTerminateByTime());
            limitMaxTime->blockSignals(false);
            maxTime->setEnabled(atts->GetTerminateByTime());
            break;
        case LimitCycleAttributes::ID_termTime:
            maxTime->setText(DoubleToQString(atts->GetTermTime()));
            break;
        case LimitCycleAttributes::ID_velocitySource:
            velocitySource->setText(DoublesToQString(atts->GetVelocitySource(),3));
            break;
        case LimitCycleAttributes::ID_lineStart:
            lineStart->setText(DoublesToQString(atts->GetLineStart(),3));
            break;
        case LimitCycleAttributes::ID_lineEnd:
            lineEnd->setText(DoublesToQString(atts->GetLineEnd(),3));
            break;
        case LimitCycleAttributes::ID_planeOrigin:
            planeOrigin->setText(DoublesToQString(atts->GetPlaneOrigin(),3));
            break;
        case LimitCycleAttributes::ID_planeNormal:
            planeNormal->setText(DoublesToQString(atts->GetPlaneNormal(),3));
            break;
        case LimitCycleAttributes::ID_planeUpAxis:
            planeUpAxis->setText(DoublesToQString(atts->GetPlaneUpAxis(),3));
            break;

        case LimitCycleAttributes::ID_sampleDensity0:
            sampleDensity[0]->blockSignals(true);
            sampleDensity[0]->setValue(atts->GetSampleDensity0());
            sampleDensity[0]->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_sampleDensity1:
            sampleDensity[1]->blockSignals(true);
            sampleDensity[1]->setValue(atts->GetSampleDensity1());
            sampleDensity[1]->blockSignals(false);
            break;

        case LimitCycleAttributes::ID_sampleDistance0:
            sampleDistance[0]->setText(DoubleToQString(atts->GetSampleDistance0()));
            break;
        case LimitCycleAttributes::ID_sampleDistance1:
            sampleDistance[1]->setText(DoubleToQString(atts->GetSampleDistance1()));
            break;

        case LimitCycleAttributes::ID_cycleTolerance:
            cycleTolerance->setText(DoubleToQString(atts->GetCycleTolerance()));
            break;
        case LimitCycleAttributes::ID_maxIterations:
            maxIterations->setText(IntToQString(atts->GetMaxIterations()));
            break;
        case LimitCycleAttributes::ID_showPartialResults:
            showPartialResults->blockSignals(true);
            showPartialResults->setChecked(atts->GetShowPartialResults());
            showPartialResults->blockSignals(false);
        case LimitCycleAttributes::ID_showReturnDistances:
            showReturnDistances->blockSignals(true);
            showReturnDistances->setChecked(atts->GetShowReturnDistances());
            showReturnDistances->blockSignals(false);

        case LimitCycleAttributes::ID_dataVariable:
            dataVariable->blockSignals(true);
            dataVariable->setText(atts->GetDataVariable().c_str());
            dataVariable->blockSignals(false);
          break;

        case LimitCycleAttributes::ID_dataValue:
            dataValueComboBox->blockSignals(true);
            dataValueComboBox->setCurrentIndex(int(atts->GetDataValue()));
            dataValueComboBox->blockSignals(false);

            if (atts->GetDataValue() == LimitCycleAttributes::Variable)
            {
                dataVariable->setEnabled(true);
                dataVariable->show();
            }
            else
            {
                dataVariable->setEnabled(false);
                dataVariable->hide();
            }
            if (atts->GetDataValue() == LimitCycleAttributes::CorrelationDistance)
            {
                TurnOn(correlationDistanceAngTolLabel);
                TurnOn(correlationDistanceMinDistLabel);
                TurnOn(correlationDistanceAngTolEdit);
                TurnOn(correlationDistanceMinDistEdit);
                TurnOn(correlationDistanceMinDistType);
            }
            else
            {
                TurnOff(correlationDistanceAngTolLabel);
                TurnOff(correlationDistanceMinDistLabel);
                TurnOff(correlationDistanceAngTolEdit);
                TurnOff(correlationDistanceMinDistEdit);
                TurnOff(correlationDistanceMinDistType);
            }
            break;

        case LimitCycleAttributes::ID_integrationDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(atts->GetIntegrationDirection()) );
            directionType->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_relTol:
            relTol->setText(DoubleToQString(atts->GetRelTol()));
            break;
        case LimitCycleAttributes::ID_absTolSizeType:
            absTolSizeType->blockSignals(true);
            absTolSizeType->setCurrentIndex((int) atts->GetAbsTolSizeType());
            absTolSizeType->blockSignals(false);
            if (atts->GetAbsTolSizeType() == LimitCycleAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            if (atts->GetAbsTolSizeType() == LimitCycleAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case LimitCycleAttributes::ID_absTolBBox:
            if (atts->GetAbsTolSizeType() == LimitCycleAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            break;
        case LimitCycleAttributes::ID_absTolAbsolute:
            if (atts->GetAbsTolSizeType() == LimitCycleAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case LimitCycleAttributes::ID_fieldType:
            // Update lots of widget visibility and enabled states.
            UpdateFieldAttributes();

            fieldType->blockSignals(true);
            fieldType->setCurrentIndex(atts->GetFieldType());
            fieldType->blockSignals(false);

            integrationType->blockSignals(true);
            if (atts->GetFieldType() == LimitCycleAttributes::M3DC12DField)
            {
              atts->SetIntegrationType(LimitCycleAttributes::M3DC12DIntegrator);
              integrationType->setCurrentIndex(LimitCycleAttributes::M3DC12DIntegrator);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetFieldType() == LimitCycleAttributes::NIMRODField)
            {
              atts->SetIntegrationType(LimitCycleAttributes::AdamsBashforth);
              integrationType->setCurrentIndex(LimitCycleAttributes::AdamsBashforth);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetIntegrationType() == LimitCycleAttributes::M3DC12DIntegrator)
            {
              atts->SetIntegrationType(LimitCycleAttributes::DormandPrince);
              integrationType->setCurrentIndex(LimitCycleAttributes::DormandPrince);
              UpdateIntegrationAttributes();
            }
            integrationType->blockSignals(false);

            break;
        case LimitCycleAttributes::ID_fieldConstant:
            fieldConstant->setText(DoubleToQString(atts->GetFieldConstant()));
            break;
        case LimitCycleAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(atts->GetIntegrationType());
            integrationType->blockSignals(false);

            fieldType->blockSignals(true);
            if (atts->GetIntegrationType() == LimitCycleAttributes::M3DC12DIntegrator)
            {
              atts->SetFieldType(LimitCycleAttributes::M3DC12DField);
              fieldType->setCurrentIndex(LimitCycleAttributes::M3DC12DField);
              UpdateFieldAttributes();
            }
            else if (atts->GetFieldType() == LimitCycleAttributes::M3DC12DField)
            {
              atts->SetFieldType(LimitCycleAttributes::Default);
              fieldType->setCurrentIndex(LimitCycleAttributes::Default);
              UpdateFieldAttributes();
            }
            fieldType->blockSignals(false);

            break;
        case LimitCycleAttributes::ID_parallelizationAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();
            parallelAlgo->blockSignals(true);
            parallelAlgo->setCurrentIndex(atts->GetParallelizationAlgorithmType());
            parallelAlgo->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_maxProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(atts->GetMaxProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(atts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(atts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_pathlines:
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
        case LimitCycleAttributes::ID_pathlinesOverrideStartingTimeFlag:
            pathlineOverrideStartingTimeFlag->blockSignals(true);
            pathlineOverrideStartingTimeFlag->setChecked(atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTime->setEnabled(atts->GetPathlines() && atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTimeFlag->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_pathlinesOverrideStartingTime:
            pathlineOverrideStartingTime->setText(DoubleToQString(atts->GetPathlinesOverrideStartingTime()));
            break;
        case LimitCycleAttributes::ID_pathlinesPeriod:
            pathlinePeriod->setText(DoubleToQString(atts->GetPathlinesPeriod()));
            break;
        case LimitCycleAttributes::ID_pathlinesCMFE:
            pathlineCMFEButtonGroup->blockSignals(true);
            pathlineCMFEButtonGroup->button(atts->GetPathlinesCMFE())->setChecked(true);
            pathlineCMFEButtonGroup->blockSignals(false);
            break;
        case LimitCycleAttributes::ID_fillInterior:
            fillButtonGroup->blockSignals(true);
            fillButtonGroup->button(atts->GetFillInterior()?1:0)->setChecked(true);
            UpdateSourceAttributes();
            fillButtonGroup->blockSignals(false);
            break;

            case LimitCycleAttributes::ID_randomSamples:
              samplingTypeButtonGroup->blockSignals(true);
              samplingTypeButtonGroup->button(atts->GetRandomSamples()?1:0)->setChecked(true);
              UpdateSourceAttributes();
              samplingTypeButtonGroup->blockSignals(false);
              break;

            case LimitCycleAttributes::ID_randomSeed:
              randomSeed->blockSignals(true);
              randomSeed->setValue(atts->GetRandomSeed());
              randomSeed->blockSignals(false);
              break;

            case LimitCycleAttributes::ID_numberOfRandomSamples:
              numberOfRandomSamples->blockSignals(true);
              numberOfRandomSamples->setValue(atts->GetNumberOfRandomSamples());
              numberOfRandomSamples->blockSignals(false);
              break;

            // case LimitCycleAttributes::ID_forceNodeCenteredData:
            //   forceNodal->blockSignals(true);
            //   forceNodal->setChecked(atts->GetForceNodeCenteredData());
            //   forceNodal->blockSignals(false);
            //   break;

            case LimitCycleAttributes::ID_issueTerminationWarnings:
              issueWarningForMaxSteps->blockSignals(true);
              issueWarningForMaxSteps->setChecked(atts->GetIssueTerminationWarnings());
              issueWarningForMaxSteps->blockSignals(false);
              break;

            case LimitCycleAttributes::ID_issueCriticalPointsWarnings:
              issueWarningForCriticalPoints->blockSignals(true);
              issueWarningForCriticalPoints->setChecked(atts->GetIssueCriticalPointsWarnings());
              criticalPointThreshold->setEnabled(atts->GetIssueCriticalPointsWarnings());
              criticalPointThresholdLabel->setEnabled(atts->GetIssueCriticalPointsWarnings());
              issueWarningForCriticalPoints->blockSignals(false);
              break;

            case LimitCycleAttributes::ID_issueStepsizeWarnings:
              issueWarningForStepsize->blockSignals(true);
              issueWarningForStepsize->setChecked(atts->GetIssueStepsizeWarnings());
              issueWarningForStepsize->blockSignals(false);
              break;
            case LimitCycleAttributes::ID_issueStiffnessWarnings:
              issueWarningForStiffness->blockSignals(true);
              issueWarningForStiffness->setChecked(atts->GetIssueStiffnessWarnings());
              issueWarningForStiffness->blockSignals(false);
              break;
            case LimitCycleAttributes::ID_criticalPointThreshold:
              criticalPointThreshold->setText(DoubleToQString(atts->GetCriticalPointThreshold()));
              break;

          case LimitCycleAttributes::ID_correlationDistanceAngTol:
            correlationDistanceAngTolEdit->blockSignals(true);
            correlationDistanceAngTolEdit->setText(DoubleToQString(atts->GetCorrelationDistanceAngTol()));
            correlationDistanceAngTolEdit->blockSignals(false);
            break;

          case LimitCycleAttributes::ID_correlationDistanceMinDistAbsolute:
            if (atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::Absolute)
            {
                correlationDistanceMinDistEdit->blockSignals(true);
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
                correlationDistanceMinDistEdit->blockSignals(false);
            }
            break;
          case LimitCycleAttributes::ID_correlationDistanceMinDistBBox:
            if (atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::FractionOfBBox)
            {
                correlationDistanceMinDistEdit->blockSignals(true);
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
                correlationDistanceMinDistEdit->blockSignals(false);
            }
            break;

          case LimitCycleAttributes::ID_correlationDistanceMinDistType:
            correlationDistanceMinDistType->blockSignals(true);
            correlationDistanceMinDistType->setCurrentIndex((int) atts->GetCorrelationDistanceMinDistType());
            correlationDistanceMinDistType->blockSignals(false);
            if (atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::FractionOfBBox)
            {
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
            }
            if (atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::Absolute)
            {
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
            }
            break;
        }
    }
}

void
QvisLimitCycleWindow::TurnOffSourceAttributes()
{
    TurnOff(lineStart, lineStartLabel);
    TurnOff(lineEnd, lineEndLabel);

    TurnOff(planeOrigin, planeOriginLabel);
    TurnOff(planeNormal, planeNormalLabel);
    TurnOff(planeUpAxis, planeUpAxisLabel);

    TurnOff(fillLabel);
    TurnOff(fillButtons[0]);
    TurnOff(fillButtons[1]);

    TurnOff(samplingTypeLabel);
    TurnOff(samplingTypeButtons[0]);
//    TurnOff(samplingTypeButtons[1]);

    TurnOff(numberOfRandomSamples, numberOfRandomSamplesLabel);
    TurnOff(randomSeed, randomSeedLabel);

    for (int i = 0; i < 2; i++)
    {
        TurnOff(sampleDensity[i], sampleDensityLabel[i]);
        TurnOff(sampleDistance[i], sampleDistanceLabel[i]);
    }
}

// ****************************************************************************
// Method: QvisLimitCycleWindow::UpdateSourceAttributes
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
//   Dave Pugmire, Wed Nov 10 09:21:17 EST 2010
//   Allow box sampling min to be 1.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
// ****************************************************************************

void
QvisLimitCycleWindow::UpdateSourceAttributes()
{
    TurnOffSourceAttributes();

    bool showSampling = false, enableFill = false;

    if (atts->GetSourceType() == LimitCycleAttributes::SpecifiedLine)
    {
        TurnOn(lineStart, lineStartLabel);
        TurnOn(lineEnd, lineEndLabel);

        showSampling = true;

        if (atts->GetRandomSamples())
        {
          TurnOn(randomSeed, randomSeedLabel);
          TurnOn(numberOfRandomSamples, numberOfRandomSamplesLabel);
        }
        else
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            sampleDensityLabel[0]->setText(tr("Samples along line:"));
            sampleDensity[0]->setMinimum(1);
        }
    }
    else if (atts->GetSourceType() == LimitCycleAttributes::SpecifiedPlane)
    {
        TurnOn(planeOrigin, planeOriginLabel);
        TurnOn(planeNormal, planeNormalLabel);
        TurnOn(planeUpAxis, planeUpAxisLabel);
        TurnOn(sampleDistance[0], sampleDistanceLabel[0]);
        TurnOn(sampleDistance[1], sampleDistanceLabel[1]);
        sampleDistanceLabel[0]->setText(tr("Distance in X:"));
        sampleDistanceLabel[1]->setText(tr("Distance in Y:"));

        showSampling = true;
        enableFill = true;

        if (atts->GetRandomSamples())
        {
          TurnOn(randomSeed, randomSeedLabel);
          TurnOn(numberOfRandomSamples, numberOfRandomSamplesLabel);
        }
        else
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            TurnOn(sampleDensity[1], sampleDensityLabel[1]);
            sampleDensityLabel[0]->setText(tr("Samples in X:"));
            sampleDensityLabel[1]->setText(tr("Samples in Y:"));
            for (int i = 0; i < 2; i++)
                sampleDensity[i]->setMinimum(2);
        }
    }

    if (showSampling)
    {
        TurnOn(samplingGroup);

        TurnOn(samplingTypeLabel);
        TurnOn(samplingTypeButtons[0]);
//        TurnOn(samplingTypeButtons[1]);

        if (enableFill)
        {
            TurnOn(fillLabel);
            TurnOn(fillButtons[0]);
            TurnOn(fillButtons[1]);
        }
    }
}


// ****************************************************************************
// Method: QvisLimitCycleWindow::UpdateFieldAttributes
//
// Purpose:
//   Updates the widgets for the various field types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// ****************************************************************************

void
QvisLimitCycleWindow::UpdateFieldAttributes()
{
    switch( atts->GetFieldType() )
    {
    case LimitCycleAttributes::M3DC12DField:
      if( atts->GetIntegrationType() ==
          LimitCycleAttributes::M3DC12DIntegrator )
        TurnOn(fieldConstant, fieldConstantLabel);
      else
        TurnOff(fieldConstant, fieldConstantLabel);

      TurnOff(velocitySource, velocitySourceLabel);

      break;

    case LimitCycleAttributes::FlashField:
      TurnOn(fieldConstant, fieldConstantLabel);
      TurnOn(velocitySource, velocitySourceLabel);
      break;

    case LimitCycleAttributes::NIMRODField:
    default:
      TurnOff(fieldConstant, fieldConstantLabel);
      TurnOff(velocitySource, velocitySourceLabel);

      break;
    }
}


// ****************************************************************************
// Method: QvisLimitCycleWindow::UpdateIntegrationAttributes
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
QvisLimitCycleWindow::UpdateIntegrationAttributes()
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
    case LimitCycleAttributes::Euler:
    case LimitCycleAttributes::Leapfrog:
    case LimitCycleAttributes::RK4:
        maxStepLength->show();
        maxStepLengthLabel->show();
      break;

    case LimitCycleAttributes::DormandPrince:
        limitMaxTimeStep->show();
        maxTimeStep->show();
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
        absTolSizeType->show();
        break;

    case LimitCycleAttributes::AdamsBashforth:
    case LimitCycleAttributes::M3DC12DIntegrator:
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
// Method: QvisLimitCycleWindow::UpdateAlgorithmAttributes
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
QvisLimitCycleWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = (atts->GetParallelizationAlgorithmType() ==
                            LimitCycleAttributes::LoadOnDemand);
    bool useStaticDomains = (atts->GetParallelizationAlgorithmType() ==
                             LimitCycleAttributes::ParallelStaticDomains);
    bool useMasterSlave = (atts->GetParallelizationAlgorithmType() ==
                           LimitCycleAttributes::MasterSlave);

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
// Method: QvisLimitCycleWindow::GetCurrentValues
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
//   Add custom renderer and lots of appearance options to the integral curves plots.
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
//   Hank Childs, Sun Dec  5 09:52:44 PST 2010
//   Add support for disabling warnings for stiffness and critical points.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
//   Kathleen Biagas, Thu Jun  9 11:44:32 PDT 2016
//   Ensure values from spin boxes are retrieved.
//
// ****************************************************************************

void
QvisLimitCycleWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg;

    // Do fieldConstant
    if(which_widget == LimitCycleAttributes::ID_fieldConstant || doAll)
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
    if(which_widget == LimitCycleAttributes::ID_maxStepLength || doAll)
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
    if(which_widget == LimitCycleAttributes::ID_maxTimeStep || doAll)
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
    if(which_widget == LimitCycleAttributes::ID_maxSteps || doAll)
    {
        int val;
        if(LineEditGetInt(maxSteps, val))
            atts->SetMaxSteps(val);
        else
        {
            ResettingError(tr("maxsteps"),
                IntToQString(atts->GetMaxSteps()));
            atts->SetMaxSteps(atts->GetMaxSteps());
        }
    }
    if(which_widget == LimitCycleAttributes::ID_termTime || doAll)
    {
        double val;
        if(LineEditGetDouble(maxTime, val))
            atts->SetTermTime(val);
        else
        {
            ResettingError(tr("maxtime"),
                DoubleToQString(atts->GetTermTime()));
            atts->SetTermTime(atts->GetTermTime());
        }
    }
    if(which_widget == LimitCycleAttributes::ID_termDistance || doAll)
    {
        double val;
        if(LineEditGetDouble(maxDistance, val))
            atts->SetTermDistance(val);
        else
        {
            ResettingError(tr("maxdistance"),
                DoubleToQString(atts->GetTermDistance()));
            atts->SetTermDistance(atts->GetTermDistance());
        }
    }
    if(which_widget == LimitCycleAttributes::ID_pathlinesOverrideStartingTime || doAll)
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
    if(which_widget == LimitCycleAttributes::ID_pathlinesPeriod || doAll)
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
    if(which_widget == LimitCycleAttributes::ID_relTol || doAll)
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
    if ((which_widget == LimitCycleAttributes::ID_absTolBBox || doAll)
        && atts->GetAbsTolSizeType() == LimitCycleAttributes::FractionOfBBox)
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
    if ((which_widget == LimitCycleAttributes::ID_absTolAbsolute || doAll)
        && atts->GetAbsTolSizeType() == LimitCycleAttributes::Absolute)
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
    if(which_widget == LimitCycleAttributes::ID_velocitySource || doAll)
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

    // Do lineStart
    if(which_widget == LimitCycleAttributes::ID_lineStart || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineStart, val, 3))
            atts->SetLineStart(val);
        else
        {
            ResettingError(tr("line start"),
                DoublesToQString(atts->GetLineStart(), 3));
            atts->SetLineStart(atts->GetLineStart());
        }
    }

    // Do lineEnd
    if(which_widget == LimitCycleAttributes::ID_lineEnd || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineEnd, val, 3))
            atts->SetLineEnd(val);
        else
        {
            ResettingError(tr("line end"),
                DoublesToQString(atts->GetLineEnd(), 3));
            atts->SetLineEnd(atts->GetLineEnd());
        }
    }

    // Do planeOrigin
    if(which_widget == LimitCycleAttributes::ID_planeOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeOrigin, val, 3))
            atts->SetPlaneOrigin(val);
        else
        {
            ResettingError(tr("plane origin"),
                DoublesToQString(atts->GetPlaneOrigin(), 3));
            atts->SetPlaneOrigin(atts->GetPlaneOrigin());
        }
    }

    // Do planeNormal
    if(which_widget == LimitCycleAttributes::ID_planeNormal || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeNormal, val, 3))
            atts->SetPlaneNormal(val);
        else
        {
            ResettingError(tr("plane normal"),
                DoublesToQString(atts->GetPlaneNormal(), 3));
            atts->SetPlaneNormal(atts->GetPlaneNormal());
        }
    }

    // Do planeUpAxis
    if(which_widget == LimitCycleAttributes::ID_planeUpAxis || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeUpAxis, val, 3))
            atts->SetPlaneUpAxis(val);
        else
        {
            ResettingError(tr("plane up axis"),
                DoublesToQString(atts->GetPlaneUpAxis(), 3));
            atts->SetPlaneUpAxis(atts->GetPlaneUpAxis());
        }
    }

    // Do sampleDistance 0
    if(which_widget == LimitCycleAttributes::ID_sampleDistance0 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[0], val))
            atts->SetSampleDistance0(val);
        else
        {
            ResettingError(tr("Sample distance 0"),
                DoubleToQString(atts->GetSampleDistance0()));
            atts->SetSampleDistance0(atts->GetSampleDistance0());
        }
    }
    // Do sampleDistance 1
    if(which_widget == LimitCycleAttributes::ID_sampleDistance1 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[1], val))
            atts->SetSampleDistance1(val);
        else
        {
            ResettingError(tr("Sample distance 1"),
                DoubleToQString(atts->GetSampleDistance1()));
            atts->SetSampleDistance1(atts->GetSampleDistance1());
        }
    }

    // Do termination
    if(which_widget == LimitCycleAttributes::ID_maxIterations || doAll)
    {
        int val;
        if(LineEditGetInt(maxIterations, val))
            atts->SetMaxIterations(val);
        else
        {
            ResettingError(tr("maximum iterations"),
                IntToQString(atts->GetMaxIterations()));
            atts->SetMaxIterations(atts->GetMaxIterations());
        }
    }
    if(which_widget == LimitCycleAttributes::ID_cycleTolerance || doAll)
    {
        double val;
        if(LineEditGetDouble(cycleTolerance, val))
            atts->SetCycleTolerance(val);
        else
        {
            ResettingError(tr("cycle tolerance"),
                DoubleToQString(atts->GetCycleTolerance()));
            atts->SetCycleTolerance(atts->GetCycleTolerance());
        }
    }

    // maxProcessCount
    if (which_widget == LimitCycleAttributes::ID_maxProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            atts->SetMaxProcessCount(val);
    }

    // workGroupSize
    if (which_widget == LimitCycleAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            atts->SetWorkGroupSize(val);
    }

    // maxDomainCache
    if (which_widget == LimitCycleAttributes::ID_maxDomainCacheSize || doAll)
    {
        if (maxDomainCache->value() != atts->GetMaxDomainCacheSize())
            atts->SetMaxDomainCacheSize(maxDomainCache->value());
    }

    // numberOfRandomSamples
    if (which_widget == LimitCycleAttributes::ID_numberOfRandomSamples || doAll)
    {
        if (numberOfRandomSamples->value() != atts->GetNumberOfRandomSamples())
            atts->SetNumberOfRandomSamples(maxDomainCache->value());
    }

    // randomSeed
    if (which_widget == LimitCycleAttributes::ID_randomSeed || doAll)
    {
        if (randomSeed->value() != atts->GetRandomSeed())
            atts->SetRandomSeed(randomSeed->value());
    }

    // sampleDensity0
    if (which_widget == LimitCycleAttributes::ID_sampleDensity0 || doAll)
    {
        if (sampleDensity[0]->value() != atts->GetSampleDensity0())
            atts->SetSampleDensity0(sampleDensity[0]->value());
    }

    // sampleDensity1
    if (which_widget == LimitCycleAttributes::ID_sampleDensity1 || doAll)
    {
        if (sampleDensity[1]->value() != atts->GetSampleDensity1())
            atts->SetSampleDensity1(sampleDensity[1]->value());
    }

    // criticalPointThreshold
    if(which_widget == LimitCycleAttributes::ID_criticalPointThreshold || doAll)
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

    if (which_widget == LimitCycleAttributes::ID_correlationDistanceAngTol || doAll)
    {
        double val;
        bool res = LineEditGetDouble(correlationDistanceAngTolEdit, val);
        if (res)
        {
            if (val >= 0.0)
                atts->SetCorrelationDistanceAngTol(val);
            else
            {
                ResettingError(tr("Correlation distance angular tolerance must be >= 0.0"),
                               DoubleToQString(atts->GetCorrelationDistanceAngTol()));
                atts->SetCorrelationDistanceAngTol(atts->GetCorrelationDistanceAngTol());
            }
        }
        else
        {
            ResettingError(tr("Correlation distance angular tolerance"),
                DoubleToQString(atts->GetCorrelationDistanceAngTol()));
            atts->SetCorrelationDistanceAngTol(atts->GetCorrelationDistanceAngTol());
        }
    }
    if ((which_widget == LimitCycleAttributes::ID_correlationDistanceMinDistAbsolute || doAll)
        && atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::Absolute)
    {
        double val;
        bool res = LineEditGetDouble(correlationDistanceMinDistEdit, val);
        if (res)
        {
            if (val >= 0.0)
                atts->SetCorrelationDistanceMinDistAbsolute(val);
            else
            {
                ResettingError(tr("Correlation distance minimum distance must be >= 0.0"),
                               DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
                atts->SetCorrelationDistanceMinDistAbsolute(atts->GetCorrelationDistanceMinDistAbsolute());
            }
        }
        else
        {
            ResettingError(tr("Correlation distance minimum distnace"),
                DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
            atts->SetCorrelationDistanceMinDistAbsolute(atts->GetCorrelationDistanceMinDistAbsolute());
        }
    }
    if ((which_widget == LimitCycleAttributes::ID_correlationDistanceMinDistBBox || doAll)
        && atts->GetCorrelationDistanceMinDistType() == LimitCycleAttributes::FractionOfBBox)
    {
        double val;
        bool res = LineEditGetDouble(correlationDistanceMinDistEdit, val);
        if (res)
        {
            if (val >= 0.0)
                atts->SetCorrelationDistanceMinDistBBox(val);
            else
            {
                ResettingError(tr("Correlation distance minimum distance must be >= 0.0"),
                               DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
                atts->SetCorrelationDistanceMinDistBBox(atts->GetCorrelationDistanceMinDistBBox());
            }
        }
        else
        {
            ResettingError(tr("Correlation distance minimum distnace"),
                DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
            atts->SetCorrelationDistanceMinDistBBox(atts->GetCorrelationDistanceMinDistBBox());
        }
    }
}

void
QvisLimitCycleWindow::sourceTypeChanged(int val)
{
    if(val != atts->GetSourceType())
    {
        atts->SetSourceType(LimitCycleAttributes::SourceType(val));
        Apply();
    }
}

void
QvisLimitCycleWindow::directionTypeChanged(int val)
 {
    if(val != atts->GetIntegrationDirection())
    {
        atts->SetIntegrationDirection(LimitCycleAttributes::IntegrationDirection(val));
        Apply();
    }
}

void
QvisLimitCycleWindow::fieldTypeChanged(int val)
 {
    if(val != atts->GetFieldType())
    {
        atts->SetFieldType(LimitCycleAttributes::FieldType(val));
        Apply();
    }
}

void
QvisLimitCycleWindow::fieldConstantProccessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_fieldConstant);
    Apply();
}

void
QvisLimitCycleWindow::integrationTypeChanged(int val)
{
    if(val != atts->GetIntegrationType())
    {
        atts->SetIntegrationType(LimitCycleAttributes::IntegrationType(val));
        Apply();
    }
}

void
QvisLimitCycleWindow::parallelAlgorithmChanged(int val)
{
    if(val != atts->GetParallelizationAlgorithmType())
    {
        atts->SetParallelizationAlgorithmType(LimitCycleAttributes::ParallelizationAlgorithmType(val));
        Apply();
    }
}

void
QvisLimitCycleWindow::maxStepLengthProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_maxStepLength);
    Apply();
}

void
QvisLimitCycleWindow::maxTimeStepProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_maxTimeStep);
    Apply();
}

void
QvisLimitCycleWindow::maxStepsProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_maxSteps);
    Apply();
}

void
QvisLimitCycleWindow::limitMaxTimeChanged(bool val)
{
    if(val != atts->GetTerminateByTime())
    {
        atts->SetTerminateByTime(val);
        Apply();
    }
}

void
QvisLimitCycleWindow::limitMaxDistanceChanged(bool val)
{
    if(val != atts->GetTerminateByDistance())
    {
        atts->SetTerminateByDistance(val);
        Apply();
    }
}

void
QvisLimitCycleWindow::maxTimeProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_termTime);
    Apply();
}

void
QvisLimitCycleWindow::maxDistanceProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_termDistance);
    Apply();
}

void
QvisLimitCycleWindow::relTolProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_relTol);
    Apply();
}

void
QvisLimitCycleWindow::lineStartProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_lineStart);
    Apply();
}

void
QvisLimitCycleWindow::lineEndProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_lineEnd);
    Apply();
}

void
QvisLimitCycleWindow::planeOriginProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_planeOrigin);
    Apply();
}

void
QvisLimitCycleWindow::planeNormalProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_planeNormal);
    Apply();
}

void
QvisLimitCycleWindow::planeUpAxisProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_planeUpAxis);
    Apply();
}

void
QvisLimitCycleWindow::sampleDistance0ProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_sampleDistance0);
    Apply();
}

void
QvisLimitCycleWindow::sampleDistance1ProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_sampleDistance1);
    Apply();
}

void
QvisLimitCycleWindow::sampleDensity0Changed(int val)
{
    atts->SetSampleDensity0(val);
    Apply();
}

void
QvisLimitCycleWindow::sampleDensity1Changed(int val)
{
    atts->SetSampleDensity1(val);
    Apply();
}

void
QvisLimitCycleWindow::maxSLCountChanged(int val)
{
    atts->SetMaxProcessCount(val);
    Apply();
}

void
QvisLimitCycleWindow::maxDomainCacheChanged(int val)
{
    atts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisLimitCycleWindow::workGroupSizeChanged(int val)
{
    atts->SetWorkGroupSize(val);
    Apply();
}

void
QvisLimitCycleWindow::cycleToleranceProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_cycleTolerance);
    Apply();
}

void
QvisLimitCycleWindow::maxIterationsProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_maxIterations);
    Apply();
}

void
QvisLimitCycleWindow::showPartialResultsChanged(bool val)
{
    atts->SetShowPartialResults(val);
    Apply();
}

void
QvisLimitCycleWindow::showReturnDistancesChanged(bool val)
{
    atts->SetShowReturnDistances(val);
    Apply();
}

void
QvisLimitCycleWindow::dataValueChanged(int val)
{
    atts->SetDataValue((LimitCycleAttributes::DataValue)val);
    Apply();
}

void
QvisLimitCycleWindow::dataVariableChanged(const QString &var)
{
    atts->SetDataVariable(var.toStdString());
    Apply();
}

void
QvisLimitCycleWindow::absTolProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_absTolBBox);
    GetCurrentValues(LimitCycleAttributes::ID_absTolAbsolute);
    Apply();
}

void
QvisLimitCycleWindow::absTolSizeTypeChanged(int val)
{
    atts->SetAbsTolSizeType((LimitCycleAttributes::SizeType) val);
    Apply();
}

void
QvisLimitCycleWindow::limitMaxTimeStepChanged(bool val)
{
    atts->SetLimitMaximumTimestep(val);
    Apply();
}

// void
// QvisLimitCycleWindow::forceNodalChanged(bool val)
// {
//     atts->SetForceNodeCenteredData(val);
//     Apply();
// }

void
QvisLimitCycleWindow::velocitySourceProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_velocitySource);
    Apply();
}

void
QvisLimitCycleWindow::fillChanged(int index)
{
    atts->SetFillInterior(index == 1);
    Apply();
}

void
QvisLimitCycleWindow::samplingTypeChanged(int index)
{
    atts->SetRandomSamples(index == 1);
    Apply();
}

void
QvisLimitCycleWindow::randomSeedChanged(int val)
{
    atts->SetRandomSeed(val);
    Apply();
}

void
QvisLimitCycleWindow::numberOfRandomSamplesChanged(int val)
{
    atts->SetNumberOfRandomSamples(val);
    Apply();
}

void
QvisLimitCycleWindow::correlationDistanceMinDistTypeChanged(int v)
{
    atts->SetCorrelationDistanceMinDistType((LimitCycleAttributes::SizeType) v);
    Apply();
}

void
QvisLimitCycleWindow::processCorrelationDistanceAngTolEditText()
{
    GetCurrentValues(LimitCycleAttributes::ID_correlationDistanceAngTol);
    Apply();
}

void
QvisLimitCycleWindow::processCorrelationDistanceMinDistEditText()
{
    GetCurrentValues(LimitCycleAttributes::ID_correlationDistanceMinDistAbsolute);
    GetCurrentValues(LimitCycleAttributes::ID_correlationDistanceMinDistBBox);
    Apply();
}

void
QvisLimitCycleWindow::icButtonGroupChanged(int val)
{
    switch( val )
    {
        case 0: // LimitCycle
            atts->SetPathlines(false);
            break;
        case 1: // Pathline
            atts->SetPathlines(true);
            break;
    }
    Apply();
}

void
QvisLimitCycleWindow::pathlineOverrideStartingTimeFlagChanged(bool val)
{
    atts->SetPathlinesOverrideStartingTimeFlag(val);
    Apply();
}

void
QvisLimitCycleWindow::pathlineOverrideStartingTimeProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_pathlinesOverrideStartingTime);
    Apply();
}

void
QvisLimitCycleWindow::pathlinePeriodProcessText()
{
    GetCurrentValues(LimitCycleAttributes::ID_pathlinesPeriod);
    Apply();
}

void
QvisLimitCycleWindow::pathlineCMFEButtonGroupChanged(int val)
{
    atts->SetPathlinesCMFE((LimitCycleAttributes::PathlinesCMFE)val);
    Apply();
}

void
QvisLimitCycleWindow::issueWarningForMaxStepsChanged(bool val)
{
    atts->SetIssueTerminationWarnings(val);
    Apply();
}

void
QvisLimitCycleWindow::issueWarningForStepsizeChanged(bool val)
{
    atts->SetIssueStepsizeWarnings(val);
    Apply();
}

void
QvisLimitCycleWindow::issueWarningForStiffnessChanged(bool val)
{
    atts->SetIssueStiffnessWarnings(val);
    Apply();
}

void
QvisLimitCycleWindow::issueWarningForCriticalPointsChanged(bool val)
{
    atts->SetIssueCriticalPointsWarnings(val);
    Apply();
}

void
QvisLimitCycleWindow::criticalPointThresholdProcessText(void)
{
    GetCurrentValues(LimitCycleAttributes::ID_criticalPointThreshold);
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
