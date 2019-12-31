// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisIntegralCurveWindow.h"

#include <IntegralCurveAttributes.h>
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
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>

#include <SelectionList.h>
#include <SelectionProperties.h>

#include <Plot.h>
#include <PlotList.h>
#include <PlotInfoAttributes.h>


static void
TurnOn(QWidget *w0, QWidget *w1=NULL);
static void
TurnOff(QWidget *w0, QWidget *w1=NULL);

// ****************************************************************************
// Method: QvisIntegralCurveWindow::QvisIntegralCurveWindow
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

QvisIntegralCurveWindow::QvisIntegralCurveWindow(const int type,
                                                 IntegralCurveAttributes *subj,
                                                 const QString &caption,
                                                 const QString &shortName,
                                                 QvisNotepadArea *notepad)
  : QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
    selectionList = GetViewerState()->GetSelectionList();
}


// ****************************************************************************
// Method: QvisIntegralCurveWindow::~QvisIntegralCurveWindow
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

QvisIntegralCurveWindow::~QvisIntegralCurveWindow()
{
}


// ****************************************************************************
// Method: QvisIntegralCurveWindow::CreateWindowContents
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
//   Kathleen Biagas, Fri Nov  8 09:14:27 PST 2019
//   Added Source tab to reduce window height.
//
// ****************************************************************************

void
QvisIntegralCurveWindow::CreateWindowContents()
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
// Method: QvisIntegralCurveWindow::CreateIntegrationTab
//
// Purpose:
//   Populates the integration tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Kathleen Biagas, Fri Nov  8 09:16:34 PST 2019
//   Some widgets moved from Integration tab to reduce window height.
//
// ****************************************************************************

void
QvisIntegralCurveWindow::CreateIntegrationTab(QWidget *pageIntegration)
{
    QGridLayout *mainLayout = new QGridLayout(pageIntegration);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    // Create the source group box.
    QGroupBox *sourceGroup = new QGroupBox(central);
    sourceGroup->setTitle(tr("Source"));
    mainLayout->addWidget(sourceGroup, 0, 0, 4, 2);
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
    sourceType->addItem(tr("Selection"));
    sourceType->addItem(tr("Field Data"));
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    sourceLayout->addWidget(sourceType, 0, 1, 1, 2);

    // Create the source geometry subgroup
    QGroupBox *geometryGroup = new QGroupBox(sourceGroup);
    sourceLayout->addWidget(geometryGroup, 1, 0, 1, 4);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
    geometryLayout->setRowStretch(5,10);

    // Create the widgets that specify a point source.
    int gRow = 0;
    pointSource = new QLineEdit(sourceGroup);
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(tr("Location"), sourceGroup);
    pointSourceLabel->setBuddy(pointSource);
    geometryLayout->addWidget(pointSourceLabel, gRow, 0);
    geometryLayout->addWidget(pointSource, gRow, 1);

    //Point list.
    gRow = 0;
    pointList = new QListWidget(sourceGroup);
    geometryLayout->addWidget(pointList, gRow, 0, 2, 1);
    connect(pointList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(pointListDoubleClicked(QListWidgetItem*)));
    connect(pointList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(pointListClicked(QListWidgetItem*)));
    connect(pointList, SIGNAL(currentTextChanged(const QString&)), this, SLOT(textChanged(QString)));

    pointListAddPoint = new QPushButton(tr("Add Point"), sourceGroup);
    pointListDelPoint = new QPushButton(tr("Delete Point"), sourceGroup);
    pointListDelAllPoints = new QPushButton(tr("Delete All Points"), sourceGroup);
    pointListReadPoints = new QPushButton(tr("Read Text File"), sourceGroup);

    connect(pointListAddPoint, SIGNAL(clicked()), this, SLOT(addPoint()));
    connect(pointListDelPoint, SIGNAL(clicked()), this, SLOT(deletePoint()));
    connect(pointListDelAllPoints, SIGNAL(clicked()), this, SLOT(deletePoints()));
    connect(pointListReadPoints, SIGNAL(clicked()), this, SLOT(readPoints()));

    geometryLayout->addWidget(pointListAddPoint,     gRow,   2);
    geometryLayout->addWidget(pointListReadPoints,   gRow++, 3);
    geometryLayout->addWidget(pointListDelPoint,     gRow,   2);
    geometryLayout->addWidget(pointListDelAllPoints, gRow++, 3);

    // Create the widgets that specify a line source.
    gRow = 0;
    lineStart = new QLineEdit(sourceGroup);
    connect(lineStart, SIGNAL(returnPressed()),
            this, SLOT(lineStartProcessText()));
    lineStartLabel = new QLabel(tr("Start"), sourceGroup);
    lineStartLabel->setBuddy(lineStart);
    geometryLayout->addWidget(lineStartLabel, gRow, 0);
    geometryLayout->addWidget(lineStart, gRow, 1);

    lineEnd = new QLineEdit(sourceGroup);
    connect(lineEnd, SIGNAL(returnPressed()),
            this, SLOT(lineEndProcessText()));
    lineEndLabel = new QLabel(tr("End"), sourceGroup);
    lineEndLabel->setBuddy(lineEnd);
    geometryLayout->addWidget(lineEndLabel, gRow, 2);
    geometryLayout->addWidget(lineEnd, gRow++, 3);

    // Create the widgets that specify a plane source.
    gRow = 0;
    planeOrigin = new QLineEdit(sourceGroup);
    connect(planeOrigin, SIGNAL(returnPressed()),
            this, SLOT(planeOriginProcessText()));
    planeOriginLabel = new QLabel(tr("Origin"), sourceGroup);
    planeOriginLabel->setBuddy(planeOrigin);
    geometryLayout->addWidget(planeOriginLabel,gRow,0);
    geometryLayout->addWidget(planeOrigin, gRow++,1);

    planeNormal = new QLineEdit(sourceGroup);
    connect(planeNormal, SIGNAL(returnPressed()),
            this, SLOT(planeNormalProcessText()));
    planeNormalLabel = new QLabel(tr("Normal"), sourceGroup);
    planeNormalLabel->setBuddy(planeNormal);
    geometryLayout->addWidget(planeNormalLabel,gRow,0);
    geometryLayout->addWidget(planeNormal, gRow,1);

    planeUpAxis = new QLineEdit(sourceGroup);
    connect(planeUpAxis, SIGNAL(returnPressed()),
            this, SLOT(planeUpAxisProcessText()));
    planeUpAxisLabel = new QLabel(tr("Up axis"), sourceGroup);
    planeUpAxisLabel->setBuddy(planeUpAxis);
    geometryLayout->addWidget(planeUpAxisLabel,gRow,2);
    geometryLayout->addWidget(planeUpAxis, gRow++,3);

    // Create the widgets that specify a sphere source.
    gRow = 0;
    sphereOrigin = new QLineEdit(sourceGroup);
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(tr("Origin"), sourceGroup);
    sphereOriginLabel->setBuddy(sphereOrigin);
    geometryLayout->addWidget(sphereOriginLabel,gRow,0);
    geometryLayout->addWidget(sphereOrigin, gRow,1);

    radius = new QLineEdit(sourceGroup);
    connect(radius, SIGNAL(returnPressed()), this, SLOT(radiusProcessText()));
    radiusLabel = new QLabel(tr("Radius"), sourceGroup);
    radiusLabel->setBuddy(radius);
    geometryLayout->addWidget(radiusLabel,gRow,2);
    geometryLayout->addWidget(radius, gRow++,3);

    // Create the widgets that specify a box source
    gRow = 0;
    useWholeBox = new QCheckBox(tr("Whole data set"), sourceGroup);
    connect(useWholeBox, SIGNAL(toggled(bool)),
            this, SLOT(useWholeBoxChanged(bool)));
    geometryLayout->addWidget(useWholeBox, gRow++, 0);

    boxExtents[0] = new QLineEdit(sourceGroup);
    connect(boxExtents[0], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[0] = new QLabel(tr("X Extents"), sourceGroup);
    boxExtentsLabel[0]->setBuddy(boxExtents[0]);
    geometryLayout->addWidget(boxExtentsLabel[0], gRow, 0);
    geometryLayout->addWidget(boxExtents[0], gRow, 1);

    boxExtents[1] = new QLineEdit(sourceGroup);
    connect(boxExtents[1], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[1] = new QLabel(tr("Y Extents"), sourceGroup);
    boxExtentsLabel[1]->setBuddy(boxExtents[1]);
    geometryLayout->addWidget(boxExtentsLabel[1], gRow, 2);
    geometryLayout->addWidget(boxExtents[1], gRow, 3);

    boxExtents[2] = new QLineEdit(sourceGroup);
    connect(boxExtents[2], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[2] = new QLabel(tr("Z Extents"), sourceGroup);
    boxExtentsLabel[2]->setBuddy(boxExtents[2]);
    geometryLayout->addWidget(boxExtentsLabel[2], gRow, 4);
    geometryLayout->addWidget(boxExtents[2], gRow++, 5);

    // FieldData Point list.
    gRow = 0;
    selectionsLabel = new QLabel(tr("Selection"), sourceGroup);
    selections = new QComboBox(sourceGroup);

    connect(selections, SIGNAL(activated(int)),
            this, SLOT(selectionsChanged(int)));

    geometryLayout->addWidget(selectionsLabel, gRow, 0);
    geometryLayout->addWidget(selections, gRow++, 1);

    // FieldData Point list.
    gRow = 0;
    fieldData = new QListWidget(sourceGroup);
    geometryLayout->addWidget(fieldData, gRow, 0, 2, 1);

    fieldDataCopyPoints = new QPushButton(tr("Copy to point list"), sourceGroup);
    connect(fieldDataCopyPoints, SIGNAL(clicked()), this, SLOT(copyPoints()));
    geometryLayout->addWidget(fieldDataCopyPoints, gRow++, 1);

    //Sampling options.
    samplingGroup = new QGroupBox(sourceGroup);
    samplingGroup->setTitle(tr("Sampling"));
    sourceLayout->addWidget(samplingGroup, 2, 0, 1, 6);

    QGridLayout *samplingLayout = new QGridLayout(samplingGroup);
    samplingLayout->setMargin(5);
    samplingLayout->setSpacing(10);
    samplingLayout->setRowStretch(5,10);

    int sRow = 0;
    samplingTypeLabel = new QLabel(tr("Sampling type:"), samplingGroup);
    samplingLayout->addWidget(samplingTypeLabel, sRow, 0);
    samplingTypeButtonGroup = new QButtonGroup(samplingGroup);
    samplingTypeButtons[0] = new QRadioButton(tr("Uniform"), samplingGroup);
    samplingTypeButtons[1] = new QRadioButton(tr("Random"), samplingGroup);
    samplingTypeButtons[0]->setChecked(true);
    samplingTypeButtonGroup->addButton(samplingTypeButtons[0], 0);
    samplingTypeButtonGroup->addButton(samplingTypeButtons[1], 1);
    samplingLayout->addWidget(samplingTypeButtons[0], sRow, 1);
    samplingLayout->addWidget(samplingTypeButtons[1], sRow, 2);
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
    sampleDensityLabel[2] = new QLabel(tr("Sample density 2"), samplingGroup);
    sampleDensity[0] = new QSpinBox(samplingGroup);
    sampleDensity[1] = new QSpinBox(samplingGroup);
    sampleDensity[2] = new QSpinBox(samplingGroup);
    sampleDensity[0]->setKeyboardTracking(false);
    sampleDensity[0]->setMinimum(1);
    sampleDensity[0]->setMaximum(10000000);
    sampleDensity[0]->setValue(atts->GetSampleDensity0());
    sampleDensity[1]->setKeyboardTracking(false);
    sampleDensity[1]->setMinimum(1);
    sampleDensity[1]->setMaximum(10000000);
    sampleDensity[1]->setValue(atts->GetSampleDensity1());
    sampleDensity[2]->setKeyboardTracking(false);
    sampleDensity[2]->setMinimum(1);
    sampleDensity[2]->setMaximum(10000000);
    sampleDensity[2]->setValue(atts->GetSampleDensity2());
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
// Method: QvisIntegralCurveWindow::CreateAppearanceTab
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
QvisIntegralCurveWindow::CreateAppearanceTab(QWidget *pageAppearance)
{
    QGridLayout *mainLayout = new QGridLayout(pageAppearance);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(10);

    // Create the data group
    QGroupBox *dataGroup = new QGroupBox(pageAppearance);
    dataGroup->setTitle(tr("Data"));
    mainLayout->addWidget(dataGroup, 0, 0);

    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    dataLayout->setSpacing(10);
    dataLayout->setColumnStretch(2,10);

    // Create the data value.
    dataLayout->addWidget(new QLabel(tr("Data value"), dataGroup), 0, 0);

    dataValueComboBox = new QComboBox(dataGroup);
    dataValueComboBox->addItem(tr("Solid"), IntegralCurveAttributes::Solid);
    dataValueComboBox->addItem(tr("Seed point ID"), IntegralCurveAttributes::SeedPointID);
    dataValueComboBox->addItem(tr("Speed"), IntegralCurveAttributes::Speed);
    dataValueComboBox->addItem(tr("Vorticity magnitude"), IntegralCurveAttributes::Vorticity);
    dataValueComboBox->addItem(tr("Arc length"), IntegralCurveAttributes::ArcLength);
    dataValueComboBox->addItem(tr("Absolute time"), IntegralCurveAttributes::TimeAbsolute);
    dataValueComboBox->addItem(tr("Relative time"), IntegralCurveAttributes::TimeRelative);
    dataValueComboBox->addItem(tr("Ave. dist. from seed"), IntegralCurveAttributes::AverageDistanceFromSeed);
    dataValueComboBox->addItem(tr("Correlation distance"), IntegralCurveAttributes::CorrelationDistance);
    dataValueComboBox->addItem(tr("Difference"), IntegralCurveAttributes::Difference);
    dataValueComboBox->addItem(tr("Variable"), IntegralCurveAttributes::Variable);
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

    // Create the cleanup group
    QGroupBox *cleanupGrp = new QGroupBox(pageAppearance);
    cleanupGrp->setTitle(tr("Cleanup the integral curve"));
    mainLayout->addWidget(cleanupGrp, 3, 0);

    QGridLayout *cleanupLayout = new QGridLayout(cleanupGrp);
    cleanupLayout->setMargin(5);
    cleanupLayout->setSpacing(10);

    // Create the cleanup value.
    cleanupLayout->addWidget(new QLabel(tr("Point cleanup"), cleanupGrp), 0, 0);

    cleanupMethodComboBox = new QComboBox(cleanupGrp);
    cleanupMethodComboBox->addItem(tr("Keep all points"));
    cleanupMethodComboBox->addItem(tr("Merge points"));
    cleanupMethodComboBox->addItem(tr("Delete points before"));
    cleanupMethodComboBox->addItem(tr("Delete points after "));
    connect(cleanupMethodComboBox, SIGNAL(activated(int)), this, SLOT(cleanupMethodChanged(int)));
    cleanupLayout->addWidget(cleanupMethodComboBox, 0, 1);

    cleanupThresholdLabel = new QLabel(tr("Velocity threshold"), cleanupGrp);
    cleanupLayout->addWidget(cleanupThresholdLabel, 0, 2);

    cleanupThreshold = new QLineEdit(cleanupGrp);
    connect(cleanupThreshold, SIGNAL(returnPressed()), this, SLOT(cleanupThresholdProcessText()));
    cleanupLayout->addWidget(cleanupThreshold, 0, 3);

    // Create the crop group
    QGroupBox *cropGrp = new QGroupBox(pageAppearance);
    cropGrp->setTitle(tr("Crop the integral curve (for animations)"));
    mainLayout->addWidget(cropGrp, 4, 0);

    QGridLayout *cropLayout = new QGridLayout(cropGrp);
    cropLayout->setMargin(5);
    cropLayout->setSpacing(10);

    // Create the crop value.
    cropLayout->addWidget(new QLabel(tr("Crop value"), cropGrp), 0, 0);

    cropValueComboBox = new QComboBox(cropGrp);
    cropValueComboBox->addItem(tr("Distance"));
    cropValueComboBox->addItem(tr("Time"));
    cropValueComboBox->addItem(tr("Step numbers"));
    connect(cropValueComboBox, SIGNAL(activated(int)), this, SLOT(cropValueChanged(int)));
    cropLayout->addWidget(cropValueComboBox, 0, 1);


    cropBeginFlag = new QCheckBox(tr("From"), cropGrp);
    connect(cropBeginFlag, SIGNAL(toggled(bool)), this, SLOT(cropBeginFlagChanged(bool)));
    cropLayout->addWidget(cropBeginFlag, 1, 0);

    cropBegin = new QLineEdit(cropGrp);
    connect(cropBegin, SIGNAL(returnPressed()), this, SLOT(cropBeginProcessText()));
    cropLayout->addWidget(cropBegin, 1, 1);


    cropEndFlag = new QCheckBox(tr("To"), cropGrp);
    connect(cropEndFlag, SIGNAL(toggled(bool)), this, SLOT(cropEndFlagChanged(bool)));
    cropLayout->addWidget(cropEndFlag, 1, 2);

    cropEnd = new QLineEdit(cropGrp);
    connect(cropEnd, SIGNAL(returnPressed()), this, SLOT(cropEndProcessText()));
    cropLayout->addWidget(cropEnd, 1, 3);

    // Streamlines/Pathline Group.
    QGroupBox *icGrp = new QGroupBox(pageAppearance);
    icGrp->setTitle(tr("Streamlines vs Pathlines"));
    mainLayout->addWidget(icGrp, 5, 0);

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
    pathlineOptionsGrp->setTitle(tr("Pathline Options"));
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
// Method: QvisIntegralCurveWindow::CreateAdvancedTab
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
QvisIntegralCurveWindow::CreateAdvancedTab(QWidget *pageAdvanced)
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
// Method: QvisIntegralCurveWindow::UpdateWindow
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
QvisIntegralCurveWindow::UpdateWindow(bool doAll)
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

    // Look for a SelectedSubject that matches one of the plots. Note
    // the doAll condition which will probably never result in anthing
    // because nothing will have been executed yet. And the data is
    // only there after execution.
    for( int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
    {
      const Plot &p = GetViewerState()->GetPlotList()->GetPlots(i);

      PlotInfoAttributes *info =
        GetViewerState()->GetPlotInformation(p.GetPlotType());

      if( doAll || SelectedSubject() == info )
      {
        MapNode *node = info->GetData().GetEntry("ListOfPoints");

        if( node )
        {
          MapNode &ptsNode = *node;
          int nValues = ptsNode["listofpoints_size"].AsInt();

          if( nValues )
          {
            const doubleVector &points =
              ptsNode["listofpoints_coordinates"].AsDoubleVector();

            // Update the GUI
            fieldData->clear();

            for (int i = 0; i < nValues; i+= 3)
            {
              char tmp[256];
              sprintf(tmp, "%lf %lf %lf", points[i], points[i+1], points[i+2]);

              QString str = tmp;
              QListWidgetItem *item = new QListWidgetItem(str, fieldData);
//            item->setFlags(item->flags() | Qt::ItemIsEditable);
              fieldData->setCurrentItem(item);
            }

            // Update the attributes.
            atts->SetFieldData( points );

            if(!doAll)
              return;
           else
             break;
          }
        }
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
        case IntegralCurveAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.
            UpdateSourceAttributes();

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(atts->GetSourceType());
            sourceType->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_maxStepLength:
            maxStepLength->setText(DoubleToQString(atts->GetMaxStepLength()));
            break;
        case IntegralCurveAttributes::ID_limitMaximumTimestep:
            limitMaxTimeStep->blockSignals(true);
            limitMaxTimeStep->setChecked(atts->GetLimitMaximumTimestep());
            limitMaxTimeStep->blockSignals(false);
            maxTimeStep->blockSignals(true);
            if (atts->GetIntegrationType() ==
                IntegralCurveAttributes::DormandPrince )
            {
                maxTimeStep->setEnabled(atts->GetLimitMaximumTimestep());
            }
            else
            {
                maxTimeStep->setEnabled(false);
            }
            maxTimeStep->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_maxTimeStep:
            maxTimeStep->setText(DoubleToQString(atts->GetMaxTimeStep()));
            break;
        case IntegralCurveAttributes::ID_maxSteps:
            maxSteps->setText(IntToQString(atts->GetMaxSteps()));
            break;
        case IntegralCurveAttributes::ID_terminateByDistance:
            limitMaxDistance->blockSignals(true);
            limitMaxDistance->setChecked(atts->GetTerminateByDistance());
            limitMaxDistance->blockSignals(false);
            maxDistance->setEnabled(atts->GetTerminateByDistance());
            break;
        case IntegralCurveAttributes::ID_termDistance:
            maxDistance->setText(DoubleToQString(atts->GetTermDistance()));
            break;
        case IntegralCurveAttributes::ID_terminateByTime:
            limitMaxTime->blockSignals(true);
            limitMaxTime->setChecked(atts->GetTerminateByTime());
            limitMaxTime->blockSignals(false);
            maxTime->setEnabled(atts->GetTerminateByTime());
            break;
        case IntegralCurveAttributes::ID_termTime:
            maxTime->setText(DoubleToQString(atts->GetTermTime()));
            break;
        case IntegralCurveAttributes::ID_velocitySource:
            velocitySource->setText(DoublesToQString(atts->GetVelocitySource(),3));
            break;
        case IntegralCurveAttributes::ID_pointSource:
            pointSource->setText(DoublesToQString(atts->GetPointSource(),3));
            break;
        case IntegralCurveAttributes::ID_lineStart:
            lineStart->setText(DoublesToQString(atts->GetLineStart(),3));
            break;
        case IntegralCurveAttributes::ID_lineEnd:
            lineEnd->setText(DoublesToQString(atts->GetLineEnd(),3));
            break;
        case IntegralCurveAttributes::ID_planeOrigin:
            planeOrigin->setText(DoublesToQString(atts->GetPlaneOrigin(),3));
            break;
        case IntegralCurveAttributes::ID_planeNormal:
            planeNormal->setText(DoublesToQString(atts->GetPlaneNormal(),3));
            break;
        case IntegralCurveAttributes::ID_planeUpAxis:
            planeUpAxis->setText(DoublesToQString(atts->GetPlaneUpAxis(),3));
            break;
        case IntegralCurveAttributes::ID_radius:
            radius->setText(DoubleToQString(atts->GetRadius()));
            break;
        case IntegralCurveAttributes::ID_sphereOrigin:
            sphereOrigin->setText(DoublesToQString(atts->GetSphereOrigin(),3));
            break;
        case IntegralCurveAttributes::ID_pointList:
            {
                std::vector<double> points = atts->GetPointList();

                pointList->clear();
                for (size_t i = 0; i < points.size(); i+= 3)
                {
                    char tmp[256];
                    sprintf(tmp, "%lf %lf %lf",
                            points[i], points[i+1], points[i+2]);
                    QString str = tmp;
                    QListWidgetItem *item = new QListWidgetItem(str, pointList);
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    pointList->setCurrentItem(item);
                }

                break;
            }

        case IntegralCurveAttributes::ID_boxExtents:
            boxExtents[0]->setText(DoublesToQString(atts->GetBoxExtents(),2));
            boxExtents[1]->setText(DoublesToQString(atts->GetBoxExtents()+2,2));
            boxExtents[2]->setText(DoublesToQString(atts->GetBoxExtents()+4,2));
            break;
        case IntegralCurveAttributes::ID_useWholeBox:
            useWholeBox->blockSignals(true);
            useWholeBox->setChecked(atts->GetUseWholeBox());
            if (atts->GetUseWholeBox())
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

        case IntegralCurveAttributes::ID_sampleDensity0:
            sampleDensity[0]->blockSignals(true);
            sampleDensity[0]->setValue(atts->GetSampleDensity0());
            sampleDensity[0]->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_sampleDensity1:
            sampleDensity[1]->blockSignals(true);
            sampleDensity[1]->setValue(atts->GetSampleDensity1());
            sampleDensity[1]->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_sampleDensity2:
            sampleDensity[2]->blockSignals(true);
            sampleDensity[2]->setValue(atts->GetSampleDensity2());
            sampleDensity[2]->blockSignals(false);
            break;

        case IntegralCurveAttributes::ID_sampleDistance0:
            sampleDistance[0]->setText(DoubleToQString(atts->GetSampleDistance0()));
            break;
        case IntegralCurveAttributes::ID_sampleDistance1:
            sampleDistance[1]->setText(DoubleToQString(atts->GetSampleDistance1()));
            break;
        case IntegralCurveAttributes::ID_sampleDistance2:
            sampleDistance[2]->setText(DoubleToQString(atts->GetSampleDistance2()));
            break;

        case IntegralCurveAttributes::ID_dataVariable:
            dataVariable->blockSignals(true);
            dataVariable->setText(atts->GetDataVariable().c_str());
            dataVariable->blockSignals(false);
          break;

        case IntegralCurveAttributes::ID_dataValue:
            dataValueComboBox->blockSignals(true);
            dataValueComboBox->setCurrentIndex(int(atts->GetDataValue()));
            dataValueComboBox->blockSignals(false);

            if (atts->GetDataValue() == IntegralCurveAttributes::Variable)
            {
                dataVariable->setEnabled(true);
                dataVariable->show();
            }
            else
            {
                dataVariable->setEnabled(false);
                dataVariable->hide();
            }
            if (atts->GetDataValue() == IntegralCurveAttributes::CorrelationDistance)
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

        case IntegralCurveAttributes::ID_cleanupMethod:
            cleanupMethodComboBox->blockSignals(true);
            cleanupMethodComboBox->setCurrentIndex(int(atts->GetCleanupMethod()));
            cleanupMethodComboBox->blockSignals(false);

            cleanupThresholdLabel->setEnabled(atts->GetCleanupMethod() !=
                                              IntegralCurveAttributes::NoCleanup);
            cleanupThreshold->setEnabled(atts->GetCleanupMethod() !=
                                              IntegralCurveAttributes::NoCleanup);
            if( atts->GetCleanupMethod() == IntegralCurveAttributes::Merge )
              cleanupThresholdLabel->setText(tr("Spatial threshold"));
            else
              cleanupThresholdLabel->setText(tr("Velocity threshold"));

            break;

        case IntegralCurveAttributes::ID_cleanupThreshold:
            cleanupThreshold->setText(DoubleToQString(atts->GetCleanupThreshold()));
            break;

        case IntegralCurveAttributes::ID_cropBeginFlag:
            cropBeginFlag->blockSignals(true);
            cropBeginFlag->setChecked(atts->GetCropBeginFlag());
            cropBeginFlag->blockSignals(false);
            cropBegin->setEnabled( atts->GetCropBeginFlag() );
            break;

        case IntegralCurveAttributes::ID_cropBegin:
            cropBegin->setText(DoubleToQString(atts->GetCropBegin()));
            break;

        case IntegralCurveAttributes::ID_cropEndFlag:
            cropEndFlag->blockSignals(true);
            cropEndFlag->setChecked(atts->GetCropEndFlag());
            cropEndFlag->blockSignals(false);
            cropEnd->setEnabled( atts->GetCropEndFlag() );
            break;

        case IntegralCurveAttributes::ID_cropEnd:
            cropEnd->setText(DoubleToQString(atts->GetCropEnd()));
            break;

        case IntegralCurveAttributes::ID_cropValue:
            cropValueComboBox->blockSignals(true);
            cropValueComboBox->setCurrentIndex(int(atts->GetCropValue()));
            cropValueComboBox->blockSignals(false);
            break;

        case IntegralCurveAttributes::ID_integrationDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(atts->GetIntegrationDirection()) );
            directionType->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_relTol:
            relTol->setText(DoubleToQString(atts->GetRelTol()));
            break;
        case IntegralCurveAttributes::ID_absTolSizeType:
            absTolSizeType->blockSignals(true);
            absTolSizeType->setCurrentIndex((int) atts->GetAbsTolSizeType());
            absTolSizeType->blockSignals(false);
            if (atts->GetAbsTolSizeType() == IntegralCurveAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            if (atts->GetAbsTolSizeType() == IntegralCurveAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case IntegralCurveAttributes::ID_absTolBBox:
            if (atts->GetAbsTolSizeType() == IntegralCurveAttributes::FractionOfBBox)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolBBox()));
            }
            break;
        case IntegralCurveAttributes::ID_absTolAbsolute:
            if (atts->GetAbsTolSizeType() == IntegralCurveAttributes::Absolute)
            {
                absTol->setText(DoubleToQString(atts->GetAbsTolAbsolute()));
            }
            break;
        case IntegralCurveAttributes::ID_fieldType:
            // Update lots of widget visibility and enabled states.
            UpdateFieldAttributes();

            fieldType->blockSignals(true);
            fieldType->setCurrentIndex(atts->GetFieldType());
            fieldType->blockSignals(false);

            integrationType->blockSignals(true);
            if (atts->GetFieldType() == IntegralCurveAttributes::M3DC12DField)
            {
              atts->SetIntegrationType(IntegralCurveAttributes::M3DC12DIntegrator);
              integrationType->setCurrentIndex(IntegralCurveAttributes::M3DC12DIntegrator);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetFieldType() == IntegralCurveAttributes::NIMRODField)
            {
              atts->SetIntegrationType(IntegralCurveAttributes::AdamsBashforth);
              integrationType->setCurrentIndex(IntegralCurveAttributes::AdamsBashforth);
              UpdateIntegrationAttributes();
            }
            else if (atts->GetIntegrationType() == IntegralCurveAttributes::M3DC12DIntegrator)
            {
              atts->SetIntegrationType(IntegralCurveAttributes::DormandPrince);
              integrationType->setCurrentIndex(IntegralCurveAttributes::DormandPrince);
              UpdateIntegrationAttributes();
            }
            integrationType->blockSignals(false);

            break;
        case IntegralCurveAttributes::ID_fieldConstant:
            fieldConstant->setText(DoubleToQString(atts->GetFieldConstant()));
            break;
        case IntegralCurveAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(atts->GetIntegrationType());
            integrationType->blockSignals(false);

            fieldType->blockSignals(true);
            if (atts->GetIntegrationType() == IntegralCurveAttributes::M3DC12DIntegrator)
            {
              atts->SetFieldType(IntegralCurveAttributes::M3DC12DField);
              fieldType->setCurrentIndex(IntegralCurveAttributes::M3DC12DField);
              UpdateFieldAttributes();
            }
            else if (atts->GetFieldType() == IntegralCurveAttributes::M3DC12DField)
            {
              atts->SetFieldType(IntegralCurveAttributes::Default);
              fieldType->setCurrentIndex(IntegralCurveAttributes::Default);
              UpdateFieldAttributes();
            }
            fieldType->blockSignals(false);

            break;
        case IntegralCurveAttributes::ID_parallelizationAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();
            parallelAlgo->blockSignals(true);
            parallelAlgo->setCurrentIndex(atts->GetParallelizationAlgorithmType());
            parallelAlgo->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_maxProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(atts->GetMaxProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(atts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(atts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_pathlines:
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
        case IntegralCurveAttributes::ID_pathlinesOverrideStartingTimeFlag:
            pathlineOverrideStartingTimeFlag->blockSignals(true);
            pathlineOverrideStartingTimeFlag->setChecked(atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTime->setEnabled(atts->GetPathlines() && atts->GetPathlinesOverrideStartingTimeFlag());
            pathlineOverrideStartingTimeFlag->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_pathlinesOverrideStartingTime:
            pathlineOverrideStartingTime->setText(DoubleToQString(atts->GetPathlinesOverrideStartingTime()));
            break;
        case IntegralCurveAttributes::ID_pathlinesPeriod:
            pathlinePeriod->setText(DoubleToQString(atts->GetPathlinesPeriod()));
            break;
        case IntegralCurveAttributes::ID_pathlinesCMFE:
            pathlineCMFEButtonGroup->blockSignals(true);
            pathlineCMFEButtonGroup->button(atts->GetPathlinesCMFE())->setChecked(true);
            pathlineCMFEButtonGroup->blockSignals(false);
            break;
        case IntegralCurveAttributes::ID_fillInterior:
            fillButtonGroup->blockSignals(true);
            fillButtonGroup->button(atts->GetFillInterior()?1:0)->setChecked(true);
            UpdateSourceAttributes();
            fillButtonGroup->blockSignals(false);
            break;

            case IntegralCurveAttributes::ID_randomSamples:
              samplingTypeButtonGroup->blockSignals(true);
              samplingTypeButtonGroup->button(atts->GetRandomSamples()?1:0)->setChecked(true);
              UpdateSourceAttributes();
              samplingTypeButtonGroup->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_randomSeed:
              randomSeed->blockSignals(true);
              randomSeed->setValue(atts->GetRandomSeed());
              randomSeed->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_numberOfRandomSamples:
              numberOfRandomSamples->blockSignals(true);
              numberOfRandomSamples->setValue(atts->GetNumberOfRandomSamples());
              numberOfRandomSamples->blockSignals(false);
              break;

            // case IntegralCurveAttributes::ID_forceNodeCenteredData:
            //   forceNodal->blockSignals(true);
            //   forceNodal->setChecked(atts->GetForceNodeCenteredData());
            //   forceNodal->blockSignals(false);
            //   break;

            case IntegralCurveAttributes::ID_issueAdvectionWarnings:
              issueWarningForAdvection->blockSignals(true);
              issueWarningForAdvection->setChecked(atts->GetIssueAdvectionWarnings());
              issueWarningForAdvection->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_issueBoundaryWarnings:
              issueWarningForBoundary->blockSignals(true);
              issueWarningForBoundary->setChecked(atts->GetIssueBoundaryWarnings());
              issueWarningForBoundary->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_issueTerminationWarnings:
              issueWarningForMaxSteps->blockSignals(true);
              issueWarningForMaxSteps->setChecked(atts->GetIssueTerminationWarnings());
              issueWarningForMaxSteps->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_issueCriticalPointsWarnings:
              issueWarningForCriticalPoints->blockSignals(true);
              issueWarningForCriticalPoints->setChecked(atts->GetIssueCriticalPointsWarnings());
              criticalPointThreshold->setEnabled(atts->GetIssueCriticalPointsWarnings());
              criticalPointThresholdLabel->setEnabled(atts->GetIssueCriticalPointsWarnings());
              issueWarningForCriticalPoints->blockSignals(false);
              break;

            case IntegralCurveAttributes::ID_issueStepsizeWarnings:
              issueWarningForStepsize->blockSignals(true);
              issueWarningForStepsize->setChecked(atts->GetIssueStepsizeWarnings());
              issueWarningForStepsize->blockSignals(false);
              break;
            case IntegralCurveAttributes::ID_issueStiffnessWarnings:
              issueWarningForStiffness->blockSignals(true);
              issueWarningForStiffness->setChecked(atts->GetIssueStiffnessWarnings());
              issueWarningForStiffness->blockSignals(false);
              break;
            case IntegralCurveAttributes::ID_criticalPointThreshold:
              criticalPointThreshold->setText(DoubleToQString(atts->GetCriticalPointThreshold()));
              break;

          case IntegralCurveAttributes::ID_correlationDistanceAngTol:
            correlationDistanceAngTolEdit->blockSignals(true);
            correlationDistanceAngTolEdit->setText(DoubleToQString(atts->GetCorrelationDistanceAngTol()));
            correlationDistanceAngTolEdit->blockSignals(false);
            break;

          case IntegralCurveAttributes::ID_correlationDistanceMinDistAbsolute:
            if (atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::Absolute)
            {
                correlationDistanceMinDistEdit->blockSignals(true);
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
                correlationDistanceMinDistEdit->blockSignals(false);
            }
            break;
          case IntegralCurveAttributes::ID_correlationDistanceMinDistBBox:
            if (atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::FractionOfBBox)
            {
                correlationDistanceMinDistEdit->blockSignals(true);
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
                correlationDistanceMinDistEdit->blockSignals(false);
            }
            break;

          case IntegralCurveAttributes::ID_correlationDistanceMinDistType:
            correlationDistanceMinDistType->blockSignals(true);
            correlationDistanceMinDistType->setCurrentIndex((int) atts->GetCorrelationDistanceMinDistType());
            correlationDistanceMinDistType->blockSignals(false);
            if (atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::FractionOfBBox)
            {
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistBBox()));
            }
            if (atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::Absolute)
            {
                correlationDistanceMinDistEdit->setText(DoubleToQString(atts->GetCorrelationDistanceMinDistAbsolute()));
            }
            break;

          case IntegralCurveAttributes::ID_selection:
            selections->blockSignals(true);
            std::string nm = atts->GetSelection();
            selections->setCurrentIndex(0);
            if (nm != "")
            {
                for (int i = 0; i < selectionList->GetNumSelections(); i++)
                {
                    const SelectionProperties &sel = selectionList->GetSelections(i);
                    if (sel.GetName() == nm)
                    {
                        selections->setCurrentIndex(i);
                        break;
                    }
                }
            }
            selections->blockSignals(false);
            break;
        }
    }
}

void
QvisIntegralCurveWindow::TurnOffSourceAttributes()
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
    TurnOff(pointListDelAllPoints);
    TurnOff(pointListAddPoint);
    TurnOff(pointListReadPoints);

    TurnOff(fieldData);
    TurnOff(fieldDataCopyPoints);

    TurnOff(fillLabel);
    TurnOff(fillButtons[0]);
    TurnOff(fillButtons[1]);

    TurnOff(samplingTypeLabel);
    TurnOff(samplingTypeButtons[0]);
    TurnOff(samplingTypeButtons[1]);

    TurnOff(numberOfRandomSamples, numberOfRandomSamplesLabel);
    TurnOff(randomSeed, randomSeedLabel);

    for (int i = 0; i < 3; i++)
    {
        TurnOff(sampleDensity[i], sampleDensityLabel[i]);
        TurnOff(sampleDistance[i], sampleDistanceLabel[i]);
    }

    TurnOff(selectionsLabel);
    TurnOff(selections);
    TurnOff(samplingGroup);

}

// ****************************************************************************
// Method: QvisIntegralCurveWindow::UpdateSourceAttributes
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
QvisIntegralCurveWindow::UpdateSourceAttributes()
{
    TurnOffSourceAttributes();

    bool showSampling = false, enableFill = false;

    if (atts->GetSourceType() == IntegralCurveAttributes::SpecifiedPoint)
        TurnOn(pointSource, pointSourceLabel);
    else if (atts->GetSourceType() == IntegralCurveAttributes::PointList)
    {
        TurnOn(pointList);
        TurnOn(pointListDelPoint);
        TurnOn(pointListDelAllPoints);
        TurnOn(pointListAddPoint);
        TurnOn(pointListReadPoints);
    }
    else if (atts->GetSourceType() == IntegralCurveAttributes::SpecifiedLine)
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
    else if (atts->GetSourceType() == IntegralCurveAttributes::SpecifiedPlane)
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
    else if (atts->GetSourceType() == IntegralCurveAttributes::Circle)
    {
        TurnOn(planeOrigin, planeOriginLabel);
        TurnOn(planeNormal, planeNormalLabel);
        TurnOn(planeUpAxis, planeUpAxisLabel);
        TurnOn(radius, radiusLabel);

        showSampling = true;
        enableFill = true;

        if (atts->GetRandomSamples())
        {
          TurnOn(randomSeed, randomSeedLabel);
          TurnOn(numberOfRandomSamples, numberOfRandomSamplesLabel);
        }
        else
        {
            if (atts->GetFillInterior())
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
    }
    else if (atts->GetSourceType() == IntegralCurveAttributes::SpecifiedSphere)
    {
        TurnOn(sphereOrigin, sphereOriginLabel);
        TurnOn(radius, radiusLabel);

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
            sampleDensityLabel[0]->setText(tr("Samples in Latitude:"));
            sampleDensityLabel[1]->setText(tr("Samples in Longitude:"));

            if (atts->GetFillInterior())
            {
                TurnOn(sampleDensity[2], sampleDensityLabel[2]);
                sampleDensityLabel[2]->setText(tr("Samples in R:"));
            }
        }
     }
    else if (atts->GetSourceType() == IntegralCurveAttributes::SpecifiedBox)
    {
        TurnOn(useWholeBox);
        for (int i = 0; i < 3; i++)
            TurnOn(boxExtents[i], boxExtentsLabel[i]);

        if (atts->GetUseWholeBox())
        {
            for (int i = 0; i < 3; i++)
            {
                boxExtents[i]->setEnabled(false);
                boxExtentsLabel[i]->setEnabled(false);
            }
        }

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
            TurnOn(sampleDensity[2], sampleDensityLabel[2]);
            sampleDensityLabel[0]->setText(tr("Samples in X:"));
            sampleDensityLabel[1]->setText(tr("Samples in Y:"));
            sampleDensityLabel[2]->setText(tr("Samples in Z:"));
            for (int i = 0; i < 3; i++)
                sampleDensity[i]->setMinimum(1);
        }
    }
    else if (atts->GetSourceType() == IntegralCurveAttributes::Selection)
    {
        TurnOn(selectionsLabel);
        TurnOn(selections);
        showSampling = true;
        enableFill = false;

        if (atts->GetRandomSamples())
        {
          TurnOn(randomSeed, randomSeedLabel);
          TurnOn(numberOfRandomSamples, numberOfRandomSamplesLabel);
        }
        else
        {
            TurnOn(sampleDensity[0], sampleDensityLabel[0]);
            sampleDensityLabel[0]->setText(tr("Sampling stride:"));
            sampleDensity[0]->setMinimum(1);
        }

    }
    else if (atts->GetSourceType() == IntegralCurveAttributes::FieldData)
    {
        TurnOn(fieldData);
        TurnOn(fieldDataCopyPoints);
    }

    if (showSampling)
    {
        TurnOn(samplingGroup);

        TurnOn(samplingTypeLabel);
        TurnOn(samplingTypeButtons[0]);
        TurnOn(samplingTypeButtons[1]);

        if (enableFill)
        {
            TurnOn(fillLabel);
            TurnOn(fillButtons[0]);
            TurnOn(fillButtons[1]);
        }
    }
}


// ****************************************************************************
// Method: QvisIntegralCurveWindow::UpdateFieldAttributes
//
// Purpose:
//   Updates the widgets for the various field types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// ****************************************************************************

void
QvisIntegralCurveWindow::UpdateFieldAttributes()
{
    switch( atts->GetFieldType() )
    {
    case IntegralCurveAttributes::M3DC12DField:
      if( atts->GetIntegrationType() ==
          IntegralCurveAttributes::M3DC12DIntegrator )
        TurnOn(fieldConstant, fieldConstantLabel);
      else
        TurnOff(fieldConstant, fieldConstantLabel);

      TurnOff(velocitySource, velocitySourceLabel);

      break;

    case IntegralCurveAttributes::FlashField:
      TurnOn(fieldConstant, fieldConstantLabel);
      TurnOn(velocitySource, velocitySourceLabel);
      break;

    case IntegralCurveAttributes::NIMRODField:
    default:
      TurnOff(fieldConstant, fieldConstantLabel);
      TurnOff(velocitySource, velocitySourceLabel);

      break;
    }
}


// ****************************************************************************
// Method: QvisIntegralCurveWindow::UpdateIntegrationAttributes
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
QvisIntegralCurveWindow::UpdateIntegrationAttributes()
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
    case IntegralCurveAttributes::Euler:
    case IntegralCurveAttributes::Leapfrog:
    case IntegralCurveAttributes::RK4:
        maxStepLength->show();
        maxStepLengthLabel->show();
      break;

    case IntegralCurveAttributes::DormandPrince:
        limitMaxTimeStep->show();
        maxTimeStep->show();
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
        absTolSizeType->show();
        break;

    case IntegralCurveAttributes::AdamsBashforth:
    case IntegralCurveAttributes::M3DC12DIntegrator:
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
// Method: QvisIntegralCurveWindow::UpdateAlgorithmAttributes
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
QvisIntegralCurveWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = (atts->GetParallelizationAlgorithmType() ==
                            IntegralCurveAttributes::LoadOnDemand);
    bool useStaticDomains = (atts->GetParallelizationAlgorithmType() ==
                             IntegralCurveAttributes::ParallelStaticDomains);
    bool useMasterSlave = (atts->GetParallelizationAlgorithmType() ==
                           IntegralCurveAttributes::MasterSlave);

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
// Method: QvisIntegralCurveWindow::GetCurrentValues
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
//   Kathleen Biagas, Thu Jun  9 11:15:04 PDT 2016
//   Ensure values from spin boxes are retrieved.
//
// ****************************************************************************

void
QvisIntegralCurveWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg;

    // Do fieldConstant
    if(which_widget == IntegralCurveAttributes::ID_fieldConstant || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_maxStepLength || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_maxTimeStep || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_maxSteps || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_termTime || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_termDistance || doAll)
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
    // Do velocity threshold
    if(which_widget == IntegralCurveAttributes::ID_cleanupThreshold || doAll)
    {
        double val;
        if(LineEditGetDouble(cleanupThreshold, val))
            atts->SetCleanupThreshold(val);
        else
        {
            ResettingError(tr("velocity threshold"),
                DoubleToQString(atts->GetCleanupThreshold()));
            atts->SetCleanupThreshold(atts->GetCleanupThreshold());
        }
    }

    // Do crop begin
    if(which_widget == IntegralCurveAttributes::ID_cropBegin || doAll)
    {
        double val;
        if(LineEditGetDouble(cropBegin, val))
            atts->SetCropBegin(val);
        else
        {
            ResettingError(tr("crop begin"),
                DoubleToQString(atts->GetCropBegin()));
            atts->SetCropBegin(atts->GetCropBegin());
        }
    }
    // Do crop end
    if(which_widget == IntegralCurveAttributes::ID_cropEnd || doAll)
    {
        double val;
        if(LineEditGetDouble(cropEnd, val))
            atts->SetCropEnd(val);
        else
        {
            ResettingError(tr("crop end"),
                DoubleToQString(atts->GetCropEnd()));
            atts->SetCropEnd(atts->GetCropEnd());
        }
    }

    if(which_widget == IntegralCurveAttributes::ID_pathlinesOverrideStartingTime || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_pathlinesPeriod || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_relTol || doAll)
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
    if ((which_widget == IntegralCurveAttributes::ID_absTolBBox || doAll)
        && atts->GetAbsTolSizeType() == IntegralCurveAttributes::FractionOfBBox)
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
    if ((which_widget == IntegralCurveAttributes::ID_absTolAbsolute || doAll)
        && atts->GetAbsTolSizeType() == IntegralCurveAttributes::Absolute)
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
    if(which_widget == IntegralCurveAttributes::ID_velocitySource || doAll)
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

    // Do pointSource
    if(which_widget == IntegralCurveAttributes::ID_pointSource || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(pointSource, val, 3))
            atts->SetPointSource(val);
        else
        {
            ResettingError(tr("point source"),
                DoublesToQString(atts->GetPointSource(), 3));
            atts->SetPointSource(atts->GetPointSource());
        }
    }

    // Do lineStart
    if(which_widget == IntegralCurveAttributes::ID_lineStart || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_lineEnd || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_planeOrigin || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_planeNormal || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_planeUpAxis || doAll)
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

    // Do radius
    if(which_widget == IntegralCurveAttributes::ID_radius || doAll)
    {
        double val;
        if(LineEditGetDouble(radius, val))
            atts->SetRadius(val);
        else
        {
            ResettingError(tr("plane radius"),
                DoubleToQString(atts->GetRadius()));
            atts->SetRadius(atts->GetRadius());
        }
    }

    // Do sampleDistance 0
    if(which_widget == IntegralCurveAttributes::ID_sampleDistance0 || doAll)
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
    if(which_widget == IntegralCurveAttributes::ID_sampleDistance1 || doAll)
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
    // Do sampleDistance 2
    if(which_widget == IntegralCurveAttributes::ID_sampleDistance2 || doAll)
    {
        double val;
        if(LineEditGetDouble(sampleDistance[2], val))
            atts->SetSampleDistance2(val);
        else
        {
            ResettingError(tr("Sample distance 1"),
                DoubleToQString(atts->GetSampleDistance2()));
            atts->SetSampleDistance2(atts->GetSampleDistance2());
        }
    }

    // Do sphereOrigin
    if(which_widget == IntegralCurveAttributes::ID_sphereOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(sphereOrigin, val, 3))
            atts->SetSphereOrigin(val);
        else
        {
            ResettingError(tr("sphere origin"),
                DoublesToQString(atts->GetSphereOrigin(), 3));
            atts->SetSphereOrigin(atts->GetSphereOrigin());
        }
    }

    // Do boxExtents
    if(which_widget == IntegralCurveAttributes::ID_boxExtents || doAll)
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
            atts->SelectBoxExtents();
        }
        else
            atts->SetBoxExtents(d);
    }

    // Do pointList
    if(which_widget == IntegralCurveAttributes::ID_pointList || doAll)
    {
        std::vector<double> points;
        double x,y,z;
        for (int i = 0; i < pointList->count(); i++)
        {
            QListWidgetItem *item = pointList->item(i);
            if (item)
            {
                std::string str = item->text().toLatin1().data();
                sscanf(str.c_str(), "%lf %lf %lf", &x, &y, &z);
                points.push_back(x);
                points.push_back(y);
                points.push_back(z);
            }
        }
        atts->SetPointList(points);
    }

    // maxIntegralCurveProcessCount
    if (which_widget == IntegralCurveAttributes::ID_maxProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            atts->SetMaxProcessCount(val);
    }

    // workGroupSize
    if (which_widget == IntegralCurveAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            atts->SetWorkGroupSize(val);
    }

    // criticalPointThreshold
    if(which_widget == IntegralCurveAttributes::ID_criticalPointThreshold || doAll)
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

    if (which_widget == IntegralCurveAttributes::ID_correlationDistanceAngTol || doAll)
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
    if ((which_widget == IntegralCurveAttributes::ID_correlationDistanceMinDistAbsolute || doAll)
        && atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::Absolute)
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
    if ((which_widget == IntegralCurveAttributes::ID_correlationDistanceMinDistBBox || doAll)
        && atts->GetCorrelationDistanceMinDistType() == IntegralCurveAttributes::FractionOfBBox)
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


    if (which_widget == IntegralCurveAttributes::ID_sampleDensity0 || doAll)
    {
        if (sampleDensity[0]->value() != atts->GetSampleDensity0())
            atts->SetSampleDensity0(sampleDensity[0]->value());
    }
    if (which_widget == IntegralCurveAttributes::ID_sampleDensity1 || doAll)
    {
        if (sampleDensity[1]->value() != atts->GetSampleDensity1())
            atts->SetSampleDensity1(sampleDensity[1]->value());
    }
    if (which_widget == IntegralCurveAttributes::ID_sampleDensity2 || doAll)
    {
        if (sampleDensity[2]->value() != atts->GetSampleDensity2())
            atts->SetSampleDensity2(sampleDensity[2]->value());
    }
    if (which_widget == IntegralCurveAttributes::ID_numberOfRandomSamples || doAll)
    {
        if (numberOfRandomSamples->value() != atts->GetNumberOfRandomSamples())
            atts->SetNumberOfRandomSamples(numberOfRandomSamples->value());
    }
    if (which_widget == IntegralCurveAttributes::ID_randomSeed || doAll)
    {
        if (randomSeed->value() != atts->GetRandomSeed())
            atts->SetRandomSeed(randomSeed->value());
    }

}

void
QvisIntegralCurveWindow::sourceTypeChanged(int val)
{
    if(val != atts->GetSourceType())
    {
        atts->SetSourceType(IntegralCurveAttributes::SourceType(val));
        Apply();
    }
}

void
QvisIntegralCurveWindow::selectionsChanged(int val)
{
    const SelectionProperties &sel = selectionList->GetSelections(val);
    std::string nm = sel.GetName();

    if(nm != atts->GetSelection())
    {
        atts->SetSelection(nm);
        Apply();
    }
}

void
QvisIntegralCurveWindow::directionTypeChanged(int val)
 {
    if(val != atts->GetIntegrationDirection())
    {
        atts->SetIntegrationDirection(IntegralCurveAttributes::IntegrationDirection(val));
        Apply();
    }
}

void
QvisIntegralCurveWindow::fieldTypeChanged(int val)
 {
    if(val != atts->GetFieldType())
    {
        atts->SetFieldType(IntegralCurveAttributes::FieldType(val));
        Apply();
    }
}

void
QvisIntegralCurveWindow::fieldConstantProccessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_fieldConstant);
    Apply();
}

void
QvisIntegralCurveWindow::integrationTypeChanged(int val)
{
    if(val != atts->GetIntegrationType())
    {
        atts->SetIntegrationType(IntegralCurveAttributes::IntegrationType(val));
        Apply();
    }
}

void
QvisIntegralCurveWindow::parallelAlgorithmChanged(int val)
{
    if(val != atts->GetParallelizationAlgorithmType())
    {
        atts->SetParallelizationAlgorithmType(IntegralCurveAttributes::ParallelizationAlgorithmType(val));
        Apply();
    }
}

void
QvisIntegralCurveWindow::maxStepLengthProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_maxStepLength);
    Apply();
}

void
QvisIntegralCurveWindow::maxTimeStepProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_maxTimeStep);
    Apply();
}

void
QvisIntegralCurveWindow::maxStepsProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_maxSteps);
    Apply();
}

void
QvisIntegralCurveWindow::limitMaxTimeChanged(bool val)
{
    if(val != atts->GetTerminateByTime())
    {
        atts->SetTerminateByTime(val);
        Apply();
    }
}

void
QvisIntegralCurveWindow::limitMaxDistanceChanged(bool val)
{
    if(val != atts->GetTerminateByDistance())
    {
        atts->SetTerminateByDistance(val);
        Apply();
    }
}

void
QvisIntegralCurveWindow::maxTimeProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_termTime);
    Apply();
}

void
QvisIntegralCurveWindow::maxDistanceProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_termDistance);
    Apply();
}

void
QvisIntegralCurveWindow::relTolProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_relTol);
    Apply();
}

void
QvisIntegralCurveWindow::lineStartProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_lineStart);
    Apply();
}

void
QvisIntegralCurveWindow::lineEndProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_lineEnd);
    Apply();
}

void
QvisIntegralCurveWindow::pointListProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_pointList);
    Apply();
}

void
QvisIntegralCurveWindow::planeOriginProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_planeOrigin);
    Apply();
}

void
QvisIntegralCurveWindow::planeNormalProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_planeNormal);
    Apply();
}

void
QvisIntegralCurveWindow::planeUpAxisProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_planeUpAxis);
    Apply();
}

void
QvisIntegralCurveWindow::radiusProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_radius);
    Apply();
}

void
QvisIntegralCurveWindow::sampleDistance0ProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_sampleDistance0);
    Apply();
}

void
QvisIntegralCurveWindow::sampleDistance1ProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_sampleDistance1);
    Apply();
}

void
QvisIntegralCurveWindow::sampleDistance2ProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_sampleDistance2);
    Apply();
}

void
QvisIntegralCurveWindow::sphereOriginProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_sphereOrigin);
    Apply();
}


void
QvisIntegralCurveWindow::sampleDensity0Changed(int val)
{
    atts->SetSampleDensity0(val);
    Apply();
}

void
QvisIntegralCurveWindow::sampleDensity1Changed(int val)
{
    atts->SetSampleDensity1(val);
    Apply();
}

void
QvisIntegralCurveWindow::sampleDensity2Changed(int val)
{
    atts->SetSampleDensity2(val);
    Apply();
}

void
QvisIntegralCurveWindow::maxSLCountChanged(int val)
{
    atts->SetMaxProcessCount(val);
    Apply();
}

void
QvisIntegralCurveWindow::maxDomainCacheChanged(int val)
{
    atts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisIntegralCurveWindow::workGroupSizeChanged(int val)
{
    atts->SetWorkGroupSize(val);
    Apply();
}

void
QvisIntegralCurveWindow::cleanupMethodChanged(int val)
{
    atts->SetCleanupMethod((IntegralCurveAttributes::CleanupMethod)val);
    Apply();
}

void
QvisIntegralCurveWindow::cleanupThresholdProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_cleanupThreshold);
    Apply();
}

void
QvisIntegralCurveWindow::cropBeginFlagChanged(bool val)
{
    atts->SetCropBeginFlag(val);
    Apply();
}

void
QvisIntegralCurveWindow::cropBeginProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_cropBegin);
    Apply();
}

void
QvisIntegralCurveWindow::cropEndFlagChanged(bool val)
{
    atts->SetCropEndFlag(val);
    Apply();
}

void
QvisIntegralCurveWindow::cropEndProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_cropEnd);
    Apply();
}

void
QvisIntegralCurveWindow::cropValueChanged(int val)
{
    atts->SetCropValue((IntegralCurveAttributes::CropValue)val);
    Apply();
}

void
QvisIntegralCurveWindow::useWholeBoxChanged(bool val)
{
    atts->SetUseWholeBox(val);
    Apply();
}

void
QvisIntegralCurveWindow::boxExtentsProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_boxExtents);
    Apply();
}

void
QvisIntegralCurveWindow::dataValueChanged(int val)
{
    atts->SetDataValue((IntegralCurveAttributes::DataValue)val);
    Apply();
}

void
QvisIntegralCurveWindow::dataVariableChanged(const QString &var)
{
    atts->SetDataVariable(var.toStdString());
    Apply();
}

void
QvisIntegralCurveWindow::absTolProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_absTolBBox);
    GetCurrentValues(IntegralCurveAttributes::ID_absTolAbsolute);
    Apply();
}

void
QvisIntegralCurveWindow::absTolSizeTypeChanged(int val)
{
    atts->SetAbsTolSizeType((IntegralCurveAttributes::SizeType) val);
    Apply();
}

void
QvisIntegralCurveWindow::limitMaxTimeStepChanged(bool val)
{
    atts->SetLimitMaximumTimestep(val);
    Apply();
}

// void
// QvisIntegralCurveWindow::forceNodalChanged(bool val)
// {
//     atts->SetForceNodeCenteredData(val);
//     Apply();
// }

void
QvisIntegralCurveWindow::velocitySourceProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_velocitySource);
    Apply();
}

void
QvisIntegralCurveWindow::pointSourceProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_pointSource);
    Apply();
}

void
QvisIntegralCurveWindow::fillChanged(int index)
{
    atts->SetFillInterior(index == 1);
    Apply();
}

void
QvisIntegralCurveWindow::samplingTypeChanged(int index)
{
    atts->SetRandomSamples(index == 1);
    Apply();
}

void
QvisIntegralCurveWindow::randomSeedChanged(int val)
{
    atts->SetRandomSeed(val);
    Apply();
}

void
QvisIntegralCurveWindow::numberOfRandomSamplesChanged(int val)
{
    atts->SetNumberOfRandomSamples(val);
    Apply();
}

void
QvisIntegralCurveWindow::pointListDoubleClicked(QListWidgetItem *item)
{
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void
QvisIntegralCurveWindow::pointListClicked(QListWidgetItem *item)
{
}

void
QvisIntegralCurveWindow::textChanged(const QString &currentText)
{
}

void
QvisIntegralCurveWindow::addPoint()
{
    QListWidgetItem *item = new QListWidgetItem("0 0 0", pointList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    pointList->setCurrentItem(item);
}

void
QvisIntegralCurveWindow::deletePoint()
{
    if (!pointList->selectedItems().empty())
    {
        qDeleteAll(pointList->selectedItems());
    }
}

void
QvisIntegralCurveWindow::deletePoints()
{
    pointList->clear();
}

void
QvisIntegralCurveWindow::readPoints()
{
    QString res = QFileDialog::getOpenFileName(NULL, tr("Open text file"), ".");
    std::string filename = res.toLatin1().data();

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


void
QvisIntegralCurveWindow::copyPoints()
{
    pointList->clear();

    std::vector<double> points;
    for (int i = 0; i < fieldData->count(); i++)
    {
        QListWidgetItem *item = fieldData->item(i);

        if (item)
        {
            std::string str = item->text().toLatin1().data();
            item = new QListWidgetItem(str.c_str(), pointList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            pointList->setCurrentItem(item);
        }
    }
}


void
QvisIntegralCurveWindow::correlationDistanceMinDistTypeChanged(int v)
{
    atts->SetCorrelationDistanceMinDistType((IntegralCurveAttributes::SizeType) v);
    Apply();
}

void
QvisIntegralCurveWindow::processCorrelationDistanceAngTolEditText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_correlationDistanceAngTol);
    Apply();
}

void
QvisIntegralCurveWindow::processCorrelationDistanceMinDistEditText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_correlationDistanceMinDistAbsolute);
    GetCurrentValues(IntegralCurveAttributes::ID_correlationDistanceMinDistBBox);
    Apply();
}

void
QvisIntegralCurveWindow::icButtonGroupChanged(int val)
{
    switch( val )
    {
        case 0: // IntegralCurve
            atts->SetPathlines(false);
            break;
        case 1: // Pathline
            atts->SetPathlines(true);
            break;
    }
    Apply();
}

void
QvisIntegralCurveWindow::pathlineOverrideStartingTimeFlagChanged(bool val)
{
    atts->SetPathlinesOverrideStartingTimeFlag(val);
    Apply();
}

void
QvisIntegralCurveWindow::pathlineOverrideStartingTimeProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_pathlinesOverrideStartingTime);
    Apply();
}

void
QvisIntegralCurveWindow::pathlinePeriodProcessText()
{
    GetCurrentValues(IntegralCurveAttributes::ID_pathlinesPeriod);
    Apply();
}

void
QvisIntegralCurveWindow::pathlineCMFEButtonGroupChanged(int val)
{
    atts->SetPathlinesCMFE((IntegralCurveAttributes::PathlinesCMFE)val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForAdvectionChanged(bool val)
{
    atts->SetIssueAdvectionWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForBoundaryChanged(bool val)
{
    atts->SetIssueBoundaryWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForMaxStepsChanged(bool val)
{
    atts->SetIssueTerminationWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForStepsizeChanged(bool val)
{
    atts->SetIssueStepsizeWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForStiffnessChanged(bool val)
{
    atts->SetIssueStiffnessWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::issueWarningForCriticalPointsChanged(bool val)
{
    atts->SetIssueCriticalPointsWarnings(val);
    Apply();
}

void
QvisIntegralCurveWindow::criticalPointThresholdProcessText(void)
{
    GetCurrentValues(IntegralCurveAttributes::ID_criticalPointThreshold);
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
