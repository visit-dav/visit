// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisRenderingWindow.h>
#include <float.h>
#include <limits.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>
#include <QTabWidget>

#include <RenderingAttributes.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>
#include <QvisOpacitySlider.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: QvisRenderingWindow::QvisRenderingWindow
//
// Purpose:
//   Constructor for the QvisRenderingWindow class.
//
// Arguments:
//   caption : The window caption.
//   shortName : The name shown in the notepad.
//   n         : The notepad area.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:46:40 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshold to scalrenActivationMode
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Initialize dlMode.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Brad Whitlock, Wed Apr  9 11:52:02 PDT 2008
//   QString for caption, shortName.
//
//   Jeremy Meredith, Fri Apr 30 15:04:34 EDT 2010
//   Added an automatic start/end setting capability for depth cueing.
//
//    Dave Pugmire, Tue Aug 24 11:32:12 EDT 2010
//    Add compact domain options.
//
// ****************************************************************************

QvisRenderingWindow::QvisRenderingWindow(const QString &caption,
    const QString &shortName, QvisNotepadArea *n) :
    QvisPostableWindowSimpleObserver(caption, shortName, n, ApplyButton)
{
    renderAtts = 0;
    windowInfo = 0;

    objectRepresentation = 0;
    stereoType = 0;
    scalrenActivationMode = 0;
    scalrenCompressMode = 0;
    compactDomainsActivationMode = 0;
}

// ****************************************************************************
// Method: QvisRenderingWindow::~QvisRenderingWindow
//
// Purpose:
//   This is the destructor for the QvisRenderingWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:47:29 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshold to scalrenActivationMode
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Delete dlMode.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
// ****************************************************************************

QvisRenderingWindow::~QvisRenderingWindow()
{
    if(renderAtts)
        renderAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}


// ****************************************************************************
// Method: QvisRenderingWindow::CreateBasicPage
//
// Purpose:
//   Creates the widgets on the basic page.
//
// Arguments:
//
// Returns:    The basic page widget.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 12:10:45 PDT 2008
//
// Modifications:
//   Eric Brugger, Tue Oct 25 12:32:40 PDT 2011
//   Add a multi resolution display capability for AMR data.
//
//   Burlen Loring, Wed Aug 12 15:57:34 PDT 2015
//   Added options for depth peeling
//
//   Burlen Loring, Sun Sep  6 14:07:04 PDT 2015
//   Added option for odered compositing
//
//   Kevin Griffin, Tue Sep 18 09:20:03 PDT 2018
//   Added a label and tooltip indicating that a VisIt restart is required
//   when changing anti-aliasing.
//
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateBasicPage()
{
    int row = 0;
    QWidget *basicOptions = new QWidget(central);
    QGridLayout *basicLayout = new QGridLayout(basicOptions);
    basicLayout->setSpacing(5);
    basicLayout->setContentsMargins(10,10,10,10);

    // Create the antialiasing widgets.
    antialiasingToggle = new QCheckBox(tr("Antialiasing"), basicOptions);
    connect(antialiasingToggle, SIGNAL(toggled(bool)),
            this, SLOT(antialiasingToggled(bool)));
    basicLayout->addWidget(antialiasingToggle, row, 0);
    restartLabel = new QLabel(tr(" (Requires restart)"), basicOptions);
    restartLabel->setToolTip(tr("When changing antialiasing, a VisIt restart is required\n"
                                "before the changes can take effect."));
    restartLabel->setEnabled(false);
    basicLayout->addWidget(restartLabel, row, 1, 1, 2);
    row++;

    // create the order compositing widgets
    compositeLabel = new QLabel(tr("Compositer Settings"), basicOptions);
    basicLayout->addWidget(compositeLabel, row, 0, 1, 3);
    row++;

    orderedComposite = new QCheckBox(tr("Ordered Compositing"), basicOptions);
    orderedComposite->setCheckState(Qt::Checked);
    orderedComposite->setToolTip(
        tr("Enable ordered compositing. For block stuctured domain\n"
           "decomposition ordered compositing eliminates the need\n"
           "for a global parallel camera order geometry sort when\n"
           "rendering translucent geometry. When combined with depth\n"
           "peeling all geometry sorting is eliminated\n"));
    connect(orderedComposite, SIGNAL(toggled(bool)),
            this, SLOT(updateOrderedComposite()));
    basicLayout->addWidget(orderedComposite, row, 2);
    row++;

    // create the depth and alpha compositing widgets
    depthCompositeThreadsLabel = new QLabel(tr("Depth Compositer Threads"), basicOptions);
    depthCompositeThreadsLabel->setToolTip(
        tr("Sets the number of threads that process communication streams\n"
           "during depth compositing.\n"));
    basicLayout->addWidget(depthCompositeThreadsLabel, row, 2);
    depthCompositeThreads = new QLineEdit("2", basicOptions);
    QIntValidator *iv0 = new QIntValidator(0,8);
    depthCompositeThreads->setValidator(iv0);
    connect(depthCompositeThreads, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthCompositeThreads(void)));
    basicLayout->addWidget(depthCompositeThreads, row, 3);
    row++;

    depthCompositeBlockingLabel = new QLabel(tr("Depth Compositer Blocking"), basicOptions);
    depthCompositeBlockingLabel->setToolTip(
        tr("Sets the block size used for streaming communication\n"
           "during depth compositing. Images are split into blocks\n"
           "of this size and streamed out. Incomning streams are\n"
           "processed in the background using compositing threads\n"));
    basicLayout->addWidget(depthCompositeBlockingLabel, row, 2);
    depthCompositeBlocking = new QLineEdit("65536", basicOptions);
    QIntValidator *iv1 = new QIntValidator(4096,0x3fffffff);
    depthCompositeBlocking->setValidator(iv1);
    connect(depthCompositeBlocking, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthCompositeBlocking(void)));
    basicLayout->addWidget(depthCompositeBlocking, row, 3);
    row++;

    alphaCompositeThreadsLabel = new QLabel(tr("Alpha Compositer Threads"), basicOptions);
    alphaCompositeThreadsLabel->setToolTip(
        tr("Sets the number of threads that process communication streams\n"
           "during alpha compositing.\n"));
    basicLayout->addWidget(alphaCompositeThreadsLabel, row, 2);
    alphaCompositeThreads = new QLineEdit("2", basicOptions);
    QIntValidator *iv2 = new QIntValidator(0,8);
    alphaCompositeThreads->setValidator(iv2);
    connect(alphaCompositeThreads, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateAlphaCompositeThreads(void)));
    basicLayout->addWidget(alphaCompositeThreads, row, 3);
    row++;

    alphaCompositeBlockingLabel = new QLabel(tr("Alpha Compositer Blocking"), basicOptions);
    alphaCompositeBlockingLabel->setToolTip(
        tr("Sets the block size used for streaming communication\n"
           "during alpha compositing. Images are split into blocks\n"
           "of this size and streamed out. Incomning streams are\n"
           "processed in the background using compositing threads\n"));
    basicLayout->addWidget(alphaCompositeBlockingLabel, row, 2);
    alphaCompositeBlocking = new QLineEdit("65536", basicOptions);
    QIntValidator *iv3 = new QIntValidator(4096,0x3fffffff);
    alphaCompositeBlocking->setValidator(iv3);
    connect(alphaCompositeBlocking, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateAlphaCompositeBlocking(void)));
    basicLayout->addWidget(alphaCompositeBlocking, row, 3);
    row++;

    // Create the depthPeeling widgets.
    depthPeeling = new QCheckBox(tr("Depth Peeling"), basicOptions);
    depthPeeling->setCheckState(Qt::Unchecked);
    depthPeeling->setToolTip(
        tr("Enable depth peeling for order independent rendering of\n"
           "transparent geometry. When not using depth peeling a camera\n"
           "order sort is used. If you have a GPU this is usualy a win\n"
           "with OSMesa it will depend on the version and build options\n"
           "with VisIt's current Mesa 7.10 it is *very* slow.\n"));
    connect(depthPeeling, SIGNAL(toggled(bool)),
            this, SLOT(updateDepthPeeling(void)));
    basicLayout->addWidget(depthPeeling, row, 0, 1, 3);
    row++;

    occlusionRatioLabel = new QLabel(tr("Occlusion ratio"), basicOptions);
    occlusionRatioLabel->setToolTip(
        tr("When greater than zero early terminations is enabled and\n"
           "the algorithm will stop doing peels when fewer than this\n"
           "fraction of pixels changed in the last peel. Thus one sacrifices\n"
           "accuracy for speed. When set to zero the maximum number of peels\n"
           "will be made which, when enough peels are requested, ensures a\n"
           "correct result."));
    occlusionRatioLabel->setEnabled(false);
    basicLayout->addWidget(occlusionRatioLabel, row, 2);
    occlusionRatio = new QLineEdit("0.01", basicOptions);
    QDoubleValidator *dv0 = new QDoubleValidator(0.0, 0.5, 4, 0);
    occlusionRatio->setValidator(dv0);
    occlusionRatio->setEnabled(false);
    connect(occlusionRatio, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthPeeling(void)));
    basicLayout->addWidget(occlusionRatio, row, 3);
    row++;

    numberOfPeelsLabel = new QLabel(tr("Max number of Peels"), basicOptions);
    numberOfPeelsLabel->setToolTip(
        tr("Sets the maximum number of peels to use. Each peel renders the\n"
           "next nearest surface for a given fragment. You may need to\n"
           "increase the number of peels for very complex scenes."));
    numberOfPeelsLabel->setEnabled(false);
    basicLayout->addWidget(numberOfPeelsLabel, row, 2);
    numberOfPeels = new QLineEdit("32", basicOptions);
    QIntValidator *iv4 = new QIntValidator(1,1000);
    numberOfPeels->setValidator(iv4);
    numberOfPeels->setEnabled(false);
    connect(numberOfPeels, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthPeeling(void)));
    basicLayout->addWidget(numberOfPeels, row, 3);
    row++;

    connect(depthPeeling, SIGNAL(toggled(bool)),
            occlusionRatioLabel, SLOT(setEnabled(bool)));
    connect(depthPeeling, SIGNAL(toggled(bool)),
            occlusionRatio, SLOT(setEnabled(bool)));

    connect(depthPeeling, SIGNAL(toggled(bool)),
            numberOfPeelsLabel, SLOT(setEnabled(bool)));
    connect(depthPeeling, SIGNAL(toggled(bool)),
            numberOfPeels, SLOT(setEnabled(bool)));

    // Create the multi resolution widgets.
    multiresolutionModeToggle = new QCheckBox(tr("Multi resolution for 2d AMR data"), basicOptions);
    connect(multiresolutionModeToggle, SIGNAL(toggled(bool)),
            this, SLOT(multiresolutionModeToggled(bool)));
    basicLayout->addWidget(multiresolutionModeToggle, row, 0, 1, 3);
    row++;

    multiresolutionSmallestCellLabel = new QLabel(tr("Smallest cell"), basicOptions);
    basicLayout->addWidget(multiresolutionSmallestCellLabel, row, 1);
    multiresolutionSmallestCellLineEdit = new QLineEdit(basicOptions);
    connect(multiresolutionSmallestCellLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMultiresolutionSmallestCellText()));
    connect(multiresolutionSmallestCellLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processMultiresolutionSmallestCellText(const QString &)));
    basicLayout->addWidget(multiresolutionSmallestCellLineEdit, row, 2, 1, 2);
    row++;

    // Create the surface rep widgets.
    QLabel *drawObjLabel = new QLabel(tr("Draw objects as"), basicOptions);
    basicLayout->addWidget(drawObjLabel, row, 0, 1, 3);
    objectRepresentation = new QButtonGroup(basicOptions);
    connect(objectRepresentation, SIGNAL(buttonClicked(int)),
            this, SLOT(objectRepresentationChanged(int)));
    row++;

    QRadioButton *surfaces = new QRadioButton(tr("Surfaces"), basicOptions);
    objectRepresentation->addButton(surfaces, 0);
    basicLayout->addWidget(surfaces, row, 1);
    QRadioButton *wires = new QRadioButton(tr("Wireframe"), basicOptions);
    objectRepresentation->addButton(wires, 1);
    basicLayout->addWidget(wires, row, 2);
    QRadioButton *points = new QRadioButton(tr("Points"), basicOptions);
    objectRepresentation->addButton(points, 2);
    basicLayout->addWidget(points, row, 3);
    row++;

    // Create the stereo widgets.
    stereoToggle = new QCheckBox(tr("Stereo"), basicOptions);
    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    basicLayout->addWidget(stereoToggle, row, 0, 1, 3);
    row++;

    stereoType = new QButtonGroup(basicOptions);
    connect(stereoType, SIGNAL(buttonClicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton(tr("Red/Blue"), basicOptions);
    stereoType->addButton(redblue, 0);
    basicLayout->addWidget(redblue, row, 1);
    interlace = new QRadioButton(tr("Interlace"), basicOptions);
    stereoType->addButton(interlace, 1);
    basicLayout->addWidget(interlace, row, 2);
    row++;
    crystalEyes = new QRadioButton(tr("Crystal Eyes"), basicOptions);
    stereoType->addButton(crystalEyes, 2);
    basicLayout->addWidget(crystalEyes, row, 1);
    redgreen = new QRadioButton(tr("Red/Green"), basicOptions);
    stereoType->addButton(redgreen, 3);
    basicLayout->addWidget(redgreen, row, 2);
    row++;

    // Create the specular lighting options
    specularToggle = new QCheckBox(tr("Specular lighting"), basicOptions);
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    basicLayout->addWidget(specularToggle, row, 0, 1, 3);
    row++;

    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, basicOptions);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularStrengthChanged(int, const void*)));
    specularStrengthLabel = new QLabel(tr("Strength"), basicOptions);
    specularStrengthLabel->setBuddy(specularStrengthSlider);
    basicLayout->addWidget(specularStrengthLabel, row, 1);
    basicLayout->addWidget(specularStrengthSlider, row, 2, 1, 2);
    row++;

    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, basicOptions);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularPowerChanged(int, const void*)));
    specularPowerLabel = new QLabel(tr("Sharpness"), basicOptions);
    specularPowerLabel->setBuddy(specularPowerSlider);
    basicLayout->addWidget(specularPowerLabel, row,1);
    basicLayout->addWidget(specularPowerSlider, row, 2, 1, 2);
    row++;

    return basicOptions;
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateAdvancedPage
//
// Purpose:
//   Creates the advanced page widgets
//
// Arguments:
//
// Returns:
//
// Note:       Moved from CreateWindowContents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 13:18:25 PDT 2008
//
// Modifications:
//   Alok Hota, Mon 23 Apr 07:12:51 PM EDT 2018
//   Added OSPRay rendering toggle and associated parameters
//
//   Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//   Added OSPRay option default values
//
//   Kathleen Biagas, Wed Aug 17, 2022
//   Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//   (bracketed by #elif defined(HAVE_OSPRAY).
//
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateAdvancedPage()
{
    int row = 0;

    QWidget *advancedOptions = new QWidget(central);
    QGridLayout *advLayout = new QGridLayout(advancedOptions);
    advLayout->setSpacing(5);
    advLayout->setContentsMargins(10,10,10,10);

    // Create the scalable rendering widgets.
    QLabel *scalrenLabel = new QLabel(tr("Use scalable rendering"), advancedOptions);
    advLayout->addWidget(scalrenLabel, row, 0, 1, 3);
    scalrenActivationMode = new QButtonGroup(advancedOptions);
    connect(scalrenActivationMode, SIGNAL(buttonClicked(int)),
            this, SLOT(scalrenActivationModeChanged(int)));
    row++;

    scalrenAuto = new QRadioButton(tr("Auto"), advancedOptions);
    scalrenActivationMode->addButton(scalrenAuto, 0);
    advLayout->addWidget(scalrenAuto, row, 1);
    scalrenAlways = new QRadioButton(tr("Always"), advancedOptions);
    scalrenActivationMode->addButton(scalrenAlways, 1);
    advLayout->addWidget(scalrenAlways, row, 2);
    scalrenNever = new QRadioButton(tr("Never"), advancedOptions);
    scalrenActivationMode->addButton(scalrenNever, 2);
    advLayout->addWidget(scalrenNever, row, 3);
    row++;

    // Create the polygon count spin box for scalable rendering threshold
    scalrenGeometryLabel =  new QLabel(tr("When polygon count exceeds"), advancedOptions);
    advLayout->addWidget(scalrenGeometryLabel, row, 1, 1, 2);
    scalrenAutoThreshold = new QSpinBox(advancedOptions);
    scalrenAutoThreshold->setKeyboardTracking(false);
    scalrenAutoThreshold->setMinimum(0);
    scalrenAutoThreshold->setMaximum(10000);
    scalrenAutoThreshold->setValue(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    scalrenAutoThresholdChanged(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    connect(scalrenAutoThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(scalrenAutoThresholdChanged(int)));
    advLayout->addWidget(scalrenAutoThreshold, row, 3);
    row++;

    // Create the compress mode widgets.
    scalrenCompressLabel = new QLabel(tr("Compress images (geometry too) from engine"),
                                      advancedOptions);
    advLayout->addWidget(scalrenCompressLabel, row, 0, 1, 3);
    scalrenCompressMode = new QButtonGroup(advancedOptions);
    connect(scalrenCompressMode, SIGNAL(buttonClicked(int)),
            this, SLOT(scalrenCompressModeChanged(int)));
    row++;

    QRadioButton *cmp_auto = new QRadioButton(tr("Auto"), advancedOptions);
    cmp_auto->setEnabled(0); // disabled until implemented
    scalrenCompressMode->addButton(cmp_auto, 0);
    advLayout->addWidget(cmp_auto, row, 1);
    QRadioButton *cmp_always = new QRadioButton(tr("Always"), advancedOptions);
    scalrenCompressMode->addButton(cmp_always, 1);
    advLayout->addWidget(cmp_always, row, 2);
    QRadioButton *cmp_never = new QRadioButton(tr("Never"), advancedOptions);
    scalrenCompressMode->addButton(cmp_never, 2);
    advLayout->addWidget(cmp_never, row, 3);
    row++;

    // Create the compact domains widgets.
    QLabel *compactDomainsLabel = new QLabel(tr("Compact domains on engine"), advancedOptions);
    advLayout->addWidget(compactDomainsLabel, row, 0, 1, 3);
    compactDomainsActivationMode = new QButtonGroup(advancedOptions);
    connect(compactDomainsActivationMode, SIGNAL(buttonClicked(int)),
            this, SLOT(compactDomainsActivationModeChanged(int)));
    row++;

    compactDomainsAuto = new QRadioButton(tr("Auto"), advancedOptions);
    compactDomainsActivationMode->addButton(compactDomainsAuto, 0);
    advLayout->addWidget(compactDomainsAuto, row, 1);
    compactDomainsAlways = new QRadioButton(tr("Always"), advancedOptions);
    compactDomainsActivationMode->addButton(compactDomainsAlways, 1);
    advLayout->addWidget(compactDomainsAlways, row, 2);
    compactDomainsNever = new QRadioButton(tr("Never"), advancedOptions);
    compactDomainsActivationMode->addButton(compactDomainsNever, 2);
    advLayout->addWidget(compactDomainsNever, row, 3);
    row++;

    // Create the polygon count spin box for scalable rendering threshold
    compactDomainsGeometryLabel =  new QLabel(tr("When domains per process exceeds"), advancedOptions);
    advLayout->addWidget(compactDomainsGeometryLabel, row, 1, 1, 2);
    compactDomainsAutoThreshold = new QSpinBox(advancedOptions);
    compactDomainsAutoThreshold->setKeyboardTracking(false);
    compactDomainsAutoThreshold->setMinimum(0);
    compactDomainsAutoThreshold->setMaximum(100000);
    compactDomainsAutoThreshold->setValue(RenderingAttributes::DEFAULT_COMPACT_DOMAINS_AUTO_THRESHOLD);
    compactDomainsAutoThresholdChanged(RenderingAttributes::DEFAULT_COMPACT_DOMAINS_AUTO_THRESHOLD);
    connect(compactDomainsAutoThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(compactDomainsAutoThresholdChanged(int)));
    advLayout->addWidget(compactDomainsAutoThreshold, row, 3);
    row++;

    // Create the shadow lighting options
    shadowToggle = new QCheckBox(tr("Shadows (scalable rendering only)"), advancedOptions);
    connect(shadowToggle, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    advLayout->addWidget(shadowToggle, row, 0, 1, 3);
    row++;

    shadowStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, advancedOptions);
    shadowStrengthSlider->setTickInterval(25);
    connect(shadowStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(shadowStrengthChanged(int, const void*)));
    shadowStrengthLabel = new QLabel(tr("Strength"), advancedOptions);
    shadowStrengthLabel->setBuddy(shadowStrengthSlider);
    advLayout->addWidget(shadowStrengthLabel, row,1);
    advLayout->addWidget(shadowStrengthSlider, row, 2, 1, 2);
    row++;

    // Create the depth cueing options
    depthCueingToggle = new QCheckBox(tr("Depth Cueing (scalable rendering only)"), advancedOptions);
    connect(depthCueingToggle, SIGNAL(toggled(bool)),
            this, SLOT(depthCueingToggled(bool)));
    advLayout->addWidget(depthCueingToggle, row, 0, 1, 3);
    row++;

    depthCueingAutoToggle = new QCheckBox(tr("Cue automatically along camera depth"), advancedOptions);
    connect(depthCueingAutoToggle, SIGNAL(toggled(bool)),
            this, SLOT(depthCueingAutoToggled(bool)));
    advLayout->addWidget(depthCueingAutoToggle, row, 1, 1, 3);
    row++;

    depthCueingStartLabel = new QLabel(tr("Manual start point"));
    advLayout->addWidget(depthCueingStartLabel, row, 1);
    depthCueingStartEdit = new QLineEdit(advancedOptions);
    advLayout->addWidget(depthCueingStartEdit, row, 2, 1, 2);
    connect(depthCueingStartEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingStartChanged()));
    row++;

    depthCueingEndLabel = new QLabel(tr("Manual end point"));
    advLayout->addWidget(depthCueingEndLabel, row, 1);
    depthCueingEndEdit = new QLineEdit(advancedOptions);
    advLayout->addWidget(depthCueingEndEdit, row, 2, 1, 2);
    connect(depthCueingEndEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingEndChanged()));
    row++;

    // Create color texturing options.
    colorTexturingToggle = new QCheckBox(tr("Apply color using textures"), advancedOptions);
    connect(colorTexturingToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorTexturingToggled(bool)));
    advLayout->addWidget(colorTexturingToggle, row, 0, 1, 3);
    row++;

#ifdef VISIT_OSPRAY
    // Create the OSPRay rendering toggle
    osprayRenderingToggle = new QCheckBox(tr("OSPRay rendering"),
            advancedOptions);
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            this, SLOT(osprayRenderingToggled(bool)));
    advLayout->addWidget(osprayRenderingToggle, row, 0, 1, 3);
    row++;

    ospraySPPLabel = new QLabel(tr("Samples per pixel"), advancedOptions);
    ospraySPPLabel->setEnabled(false);
    advLayout->addWidget(ospraySPPLabel, row, 1, 1, 2);
    ospraySPP = new QSpinBox(advancedOptions);
    ospraySPP->setMinimum(1);
    ospraySPP->setEnabled(false);
    connect(ospraySPP, SIGNAL(valueChanged(int)),
            this, SLOT(ospraySPPChanged(int)));
    advLayout->addWidget(ospraySPP, row, 3);
    row++;
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            ospraySPPLabel, SLOT(setEnabled(bool)));
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            ospraySPP, SLOT(setEnabled(bool)));

    osprayAOLabel = new QLabel(tr("Ambient occlusion samples"), advancedOptions);
    osprayAOLabel->setEnabled(false);
    advLayout->addWidget(osprayAOLabel, row, 1, 1, 2);
    osprayAO = new QSpinBox(advancedOptions);
    osprayAO->setMinimum(0);
    osprayAO->setEnabled(false);
    connect(osprayAO, SIGNAL(valueChanged(int)),
            this, SLOT(osprayAOChanged(int)));
    advLayout->addWidget(osprayAO, row, 3);
    row++;
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            osprayAOLabel, SLOT(setEnabled(bool)));
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            osprayAO, SLOT(setEnabled(bool)));

    osprayShadowsToggle = new QCheckBox(tr("Shadows"), advancedOptions);
    osprayShadowsToggle->setEnabled(false);
    connect(osprayShadowsToggle, SIGNAL(toggled(bool)),
            this, SLOT(osprayShadowsToggled(bool)));
    advLayout->addWidget(osprayShadowsToggle, row, 1, 1, 2);
    connect(osprayRenderingToggle, SIGNAL(toggled(bool)),
            osprayShadowsToggle, SLOT(setEnabled(bool)));
    row++;
#elif defined(HAVE_OSPRAY)
    QFrame *line = new QFrame(advancedOptions);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setFixedHeight(2);
    line->setLineWidth(1);
    advLayout->addWidget(line, row, 0, 2, 4);
    row += 2;

    // Create the OSPRay group box.
    osprayGroup = new QGroupBox(tr("OSPRay Rendering"), advancedOptions);
    osprayGroup->setCheckable(true);
    osprayGroup->setChecked(false);
    connect(osprayGroup, SIGNAL(toggled(bool)),
            this, SLOT(osprayRenderingToggled(bool)));
    advLayout->addWidget(osprayGroup, row, 0, 3, 4);
    row++;

    QGridLayout *osprayLayout = new QGridLayout(osprayGroup);
    osprayLayout->setContentsMargins(5,5,5,5);
    osprayLayout->setSpacing(10);

    int orow  = 0;
    ospraySPPLabel = new QLabel(tr("Samples per pixel"), advancedOptions);
    ospraySPPLabel->setEnabled(false);
    osprayLayout->addWidget(ospraySPPLabel, orow, 0, 1, 2);
    ospraySPP = new QSpinBox(advancedOptions);
    ospraySPP->setMinimum(1);
    ospraySPP->setEnabled(false);
    connect(ospraySPP, SIGNAL(valueChanged(int)),
            this, SLOT(ospraySPPChanged(int)));
    osprayLayout->addWidget(ospraySPP, orow, 2);
    orow++;
    connect(osprayGroup, SIGNAL(toggled(bool)),
            ospraySPPLabel, SLOT(setEnabled(bool)));
    connect(osprayGroup, SIGNAL(toggled(bool)),
            ospraySPP, SLOT(setEnabled(bool)));

    osprayAOLabel = new QLabel(tr("Ambient occlusion samples"), advancedOptions);
    osprayAOLabel->setEnabled(false);
    osprayLayout->addWidget(osprayAOLabel, orow, 0, 1, 2);
    osprayAO = new QSpinBox(advancedOptions);
    osprayAO->setMinimum(0);
    osprayAO->setEnabled(false);
    connect(osprayAO, SIGNAL(valueChanged(int)),
            this, SLOT(osprayAOChanged(int)));
    osprayLayout->addWidget(osprayAO, orow, 2);
    orow++;
    connect(osprayGroup, SIGNAL(toggled(bool)),
            osprayAOLabel, SLOT(setEnabled(bool)));
    connect(osprayGroup, SIGNAL(toggled(bool)),
            osprayAO, SLOT(setEnabled(bool)));

    osprayShadowsToggle = new QCheckBox(tr("Shadows"), advancedOptions);
    osprayShadowsToggle->setEnabled(false);
    connect(osprayShadowsToggle, SIGNAL(toggled(bool)),
            this, SLOT(osprayShadowsToggled(bool)));
    osprayLayout->addWidget(osprayShadowsToggle, orow, 0, 1, 2);
    orow++;
    connect(osprayGroup, SIGNAL(toggled(bool)),
            osprayShadowsToggle, SLOT(setEnabled(bool)));
#endif

    return advancedOptions;
}


// ****************************************************************************
// Method: QvisRenderingWindow::CreateInformationPage
//
// Purpose:
//   Creates the information page.
//
// Arguments:
//
// Returns:    The information page.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 13:18:51 PDT 2008
//
// Modifications:
//
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateInformationPage()
{
    QWidget *info = new QWidget(central);
    QVBoxLayout *vLayout = new QVBoxLayout(info);
    vLayout->addSpacing(10);
    vLayout->setContentsMargins(10,10,10,10);

    renderNotifyToggle = new QCheckBox(tr("Query after each render"), info);
    connect(renderNotifyToggle, SIGNAL(toggled(bool)),
            this, SLOT(renderNotifyToggled(bool)));
    vLayout->addWidget(renderNotifyToggle);
    vLayout->addSpacing(5);

    QGridLayout *iLayout = new QGridLayout(0);
    iLayout->setSpacing(5);
    vLayout->addLayout(iLayout);
    QLabel *scalrenLabel2 = new QLabel(tr("Using Scalable Rendering: "), info);
    iLayout->addWidget(scalrenLabel2, 0, 0);
    scalrenUsingLabel = new QLabel(tr("N/A"), info);
    iLayout->addWidget(scalrenUsingLabel, 0, 1, 1, 3);
    vLayout->addSpacing(5);

    fpsLabel = new QLabel(tr("Frames per second:"), info);
    iLayout->addWidget(fpsLabel, 1, 0);
    fpsMinLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsMinLabel, 1, 1);
    fpsAvgLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsAvgLabel, 1, 2);
    fpsMaxLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsMaxLabel, 1, 3);

    QLabel *ntri = new QLabel(tr("Approximate polygon count:"), info);
    iLayout->addWidget(ntri, 2, 0);
    approxNumPrimitives = new QLabel("0.", info);
    iLayout->addWidget(approxNumPrimitives, 2, 1);
    vLayout->addSpacing(5);

    QGridLayout *eLayout = new QGridLayout(0);
    eLayout->setSpacing(5);
    vLayout->addLayout(eLayout);
    QLabel *xExtentsLabel = new QLabel(tr("X Extents:"), info);
    eLayout->addWidget(xExtentsLabel, 0, 0);
    extents[0] = new QLabel("0.", info);
    eLayout->addWidget(extents[0], 0, 1);
    extents[1] = new QLabel("0.", info);
    eLayout->addWidget(extents[1], 0, 2);

    QLabel *yExtentsLabel = new QLabel(tr("Y Extents:"), info);
    eLayout->addWidget(yExtentsLabel, 1, 0);
    extents[2] = new QLabel("0.", info);
    eLayout->addWidget(extents[2], 1, 1);
    extents[3] = new QLabel("0.", info);
    eLayout->addWidget(extents[3], 1, 2);

    QLabel *zExtentsLabel = new QLabel(tr("Z Extents:"), info);
    eLayout->addWidget(zExtentsLabel, 2, 0);
    extents[4] = new QLabel("0.", info);
    eLayout->addWidget(extents[4], 2, 1);
    extents[5] = new QLabel("0.", info);
    eLayout->addWidget(extents[5], 2, 2);
    vLayout->addStretch(1000);

    return info;
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateWindowContents
//
// Purpose:
//   This method creates all of the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:47:57 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 11:57:46 PDT 2008
//   Moved code to helper functions.
//
// ****************************************************************************

void
QvisRenderingWindow::CreateWindowContents()
{
    QTabWidget *topTab = new QTabWidget(central);
    topLayout->addWidget(topTab);

    //
    // Create the basic renderer options page.
    //
    topTab->addTab(CreateBasicPage(), tr("Basic"));

    //
    // Create the advanced renderer options group.
    //
    topTab->addTab(CreateAdvancedPage(), tr("Advanced"));

    //
    // Create the renderer information group.
    //
    topTab->addTab(CreateInformationPage(), tr("Information"));
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateWindow
//
// Purpose:
//   This method updates the window's widgets when its subjects update.
//
// Arguments:
//   doAll : Whether or not to ignore field selection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:48:24 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateWindow(bool doAll)
{
    if(SelectedSubject() == renderAtts || doAll)
        UpdateOptions(doAll);
    if(SelectedSubject() == windowInfo || doAll)
        UpdateInformation(doAll);
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateOptions
//
// Purpose:
//   Updates the widgets that control the rendering options.
//
// Arguments:
//   doAll : Whether or not to ignore selection and update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 11:28:53 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 13:35:22 PST 2002
//   I added code to enable/disable the stereo rendering buttons.
//
//   Kathleen Bonnell, Wed Dec  4 18:42:48 PST 2002
//   Renumber switch cases, to reflect antialisingFrames removed from atts.
//
//   Jeremy Meredith, Fri Nov 14 17:44:35 PST 2003
//   Added specular options.
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshld to scalrenActivationMode
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Add support for multiple display list modes.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Oct 24 07:36:18 PDT 2004
//   Added shadow options.
//
//   Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//   Fixed problem with updating scalable auto threshold
//
//   Kathleen Bonnell, Thu Jun 30 15:29:55 PDT 2005
//   Added redgreen radiobutton.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Brad Whitlock, Mon Sep 18 10:49:51 PDT 2006
//   Added colorTexturingFlag.
//
//   Jeremy Meredith, Wed Aug 29 15:27:16 EDT 2007
//   Added depth cueing.  Simplified and corrected window sensitivity.
//
//   Brad Whitlock, Mon Dec 17 10:25:23 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jun 19 13:26:50 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Apr 30 15:04:34 EDT 2010
//   Added an automatic start/end setting capability for depth cueing.
//
//   Eric Brugger, Tue Oct 25 12:32:40 PDT 2011
//   Add a multi resolution display capability for AMR data.
//
//   Alok Hota, Mon 23 Apr 07:12:51 PM EDT 2018
//   Added OSPRay rendering toggle
//
//   Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//   Modified OSPRay rendering toggle to disable other OSPRay options
//   when it is disabled
//
//   Kathleen Biagas, Wed Aug 17, 2022
//   Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//   (bracketed by #elif defined(HAVE_OSPRAY).
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateOptions(bool doAll)
{
    QString tmp;
    bool enabled;
    int itmp, itmp2;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < renderAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!renderAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case RenderingAttributes::ID_antialiasing:
            antialiasingToggle->blockSignals(true);
            antialiasingToggle->setChecked(renderAtts->GetAntialiasing());
            antialiasingToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_multiresolutionMode:
            multiresolutionModeToggle->blockSignals(true);
            multiresolutionModeToggle->setChecked(renderAtts->GetMultiresolutionMode());
            multiresolutionModeToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_multiresolutionCellSize:
            multiresolutionSmallestCellLineEdit->blockSignals(true);
            tmp = DoubleToQString(renderAtts->GetMultiresolutionCellSize());
            multiresolutionSmallestCellLineEdit->setText(tmp);
            multiresolutionSmallestCellLineEdit->blockSignals(false);
            break;
        case RenderingAttributes::ID_geometryRepresentation:
            itmp = (int)renderAtts->GetGeometryRepresentation();
            objectRepresentation->blockSignals(true);
            objectRepresentation->button(itmp)->setChecked(true);
            objectRepresentation->blockSignals(false);
            break;
        case RenderingAttributes::ID_stereoRendering:
            stereoToggle->blockSignals(true);
            stereoToggle->setChecked(renderAtts->GetStereoRendering());
            stereoToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_stereoType:
            stereoType->blockSignals(true);
            stereoType->button((int)renderAtts->GetStereoType())->setChecked(true);
            stereoType->blockSignals(false);
            break;
        case RenderingAttributes::ID_notifyForEachRender:
            renderNotifyToggle->blockSignals(true);
            renderNotifyToggle->setChecked(renderAtts->GetNotifyForEachRender());
            renderNotifyToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_depthPeeling:
            enabled = renderAtts->GetDepthPeeling();
            depthPeeling->blockSignals(true);
            depthPeeling->setChecked(enabled);
            depthPeeling->blockSignals(false);
            occlusionRatioLabel->setEnabled(enabled);
            occlusionRatio->setEnabled(enabled);
            numberOfPeelsLabel->setEnabled(enabled);
            numberOfPeels->setEnabled(enabled);
            break;
        case RenderingAttributes::ID_occlusionRatio:
            tmp = DoubleToQString(renderAtts->GetOcclusionRatio());
            occlusionRatio->blockSignals(true);
            occlusionRatio->setText(tmp);
            occlusionRatio->blockSignals(false);
            break;
        case RenderingAttributes::ID_numberOfPeels:
            tmp = IntToQString(renderAtts->GetNumberOfPeels());
            numberOfPeels->blockSignals(true);
            numberOfPeels->setText(tmp);
            numberOfPeels->blockSignals(false);
            break;
#ifdef VISIT_OSPRAY
        case RenderingAttributes::ID_osprayRendering:
            osprayRenderingToggle->blockSignals(true);
            osprayRenderingToggle->setChecked(renderAtts->GetOsprayRendering());
            osprayRenderingToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_ospraySPP:
            ospraySPP->blockSignals(true);
            ospraySPP->setValue(int(renderAtts->GetOspraySPP()));
            ospraySPP->blockSignals(false);
            break;
        case RenderingAttributes::ID_osprayAO:
            osprayAO->blockSignals(true);
            osprayAO->setValue(int(renderAtts->GetOsprayAO()));
            osprayAO->blockSignals(false);
            break;
        case RenderingAttributes::ID_osprayShadows:
            osprayShadowsToggle->blockSignals(true);
            osprayShadowsToggle->setChecked(renderAtts->GetOsprayShadows());
            osprayShadowsToggle->blockSignals(false);
            break;
#elif defined(HAVE_OSPRAY)
        case RenderingAttributes::ID_osprayRendering:
            enabled = renderAtts->GetOsprayRendering();
            osprayGroup->blockSignals(true);
            osprayGroup->setChecked(enabled);
            ospraySPPLabel->setEnabled(enabled);
            ospraySPP->setEnabled(enabled);
            osprayAOLabel->setEnabled(enabled);
            osprayAO->setEnabled(enabled);
            osprayShadowsToggle->setEnabled(enabled);
            osprayGroup->blockSignals(false);
            break;
        case RenderingAttributes::ID_ospraySPP:
            ospraySPP->blockSignals(true);
            ospraySPP->setValue(int(renderAtts->GetOspraySPP()));
            ospraySPP->blockSignals(false);
            break;
        case RenderingAttributes::ID_osprayAO:
            osprayAO->blockSignals(true);
            osprayAO->setValue(int(renderAtts->GetOsprayAO()));
            osprayAO->blockSignals(false);
            break;
        case RenderingAttributes::ID_osprayShadows:
            osprayShadowsToggle->blockSignals(true);
            osprayShadowsToggle->setChecked(renderAtts->GetOsprayShadows());
            osprayShadowsToggle->blockSignals(false);
            break;
#endif
        case RenderingAttributes::ID_scalableActivationMode:
            { // new scope
            RenderingAttributes::TriStateMode rtmp;
            rtmp = renderAtts->GetScalableActivationMode();
            scalrenActivationMode->blockSignals(true);
            if (rtmp == RenderingAttributes::Always)
               scalrenActivationMode->button(1)->setChecked(true);
            else if (rtmp == RenderingAttributes::Never)
               scalrenActivationMode->button(2)->setChecked(true);
            else
               scalrenActivationMode->button(0)->setChecked(true);
            scalrenActivationMode->blockSignals(false);
            }
            break;
        case RenderingAttributes::ID_scalableAutoThreshold:
            { // new scope
            QString suffix;
            int step, widgetVal;
            int actualVal = renderAtts->GetScalableAutoThreshold();
            InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);
            scalrenAutoThreshold->blockSignals(true);
            scalrenAutoThreshold->setSuffix(suffix);
            scalrenAutoThreshold->setSingleStep(step);
            scalrenAutoThreshold->setValue(widgetVal);
            scalrenAutoThreshold->blockSignals(false);
            break;
            }
        case RenderingAttributes::ID_compactDomainsActivationMode:
            {
            RenderingAttributes::TriStateMode rtmp;
            rtmp = renderAtts->GetCompactDomainsActivationMode();
            compactDomainsActivationMode->blockSignals(true);
            if (rtmp == RenderingAttributes::Always)
                compactDomainsActivationMode->button(1)->setChecked(true);
            else if (rtmp == RenderingAttributes::Never)
                compactDomainsActivationMode->button(2)->setChecked(true);
            else
                compactDomainsActivationMode->button(0)->setChecked(true);
            compactDomainsActivationMode->blockSignals(false);
            }
          break;
        case RenderingAttributes::ID_compactDomainsAutoThreshold:
            { // new scope
            compactDomainsAutoThreshold->blockSignals(true);
            int val = renderAtts->GetCompactDomainsAutoThreshold();
            compactDomainsAutoThreshold->setValue(val);
            compactDomainsAutoThreshold->blockSignals(false);
            break;
            }

        case RenderingAttributes::ID_specularFlag:
            specularToggle->blockSignals(true);
            specularToggle->setChecked(renderAtts->GetSpecularFlag());
            specularToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularCoeff:
            specularStrengthSlider->blockSignals(true);
            specularStrengthSlider->setValue(int(renderAtts->GetSpecularCoeff()*100.));
            specularStrengthSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularPower:
            specularPowerSlider->blockSignals(true);
            specularPowerSlider->setValue(int(renderAtts->GetSpecularPower()*10.));
            specularPowerSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularColor:
            // Not user-modifiable at this time
            break;
        case RenderingAttributes::ID_doShadowing:
            shadowToggle->blockSignals(true);
            shadowToggle->setChecked(renderAtts->GetDoShadowing());
            shadowToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_shadowStrength:
            shadowStrengthSlider->blockSignals(true);
            shadowStrengthSlider->setValue(int(renderAtts->GetShadowStrength()*100.));
            shadowStrengthSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_doDepthCueing:
            depthCueingToggle->blockSignals(true);
            depthCueingToggle->setChecked(renderAtts->GetDoDepthCueing());
            depthCueingToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_depthCueingAutomatic:
            depthCueingAutoToggle->blockSignals(true);
            depthCueingAutoToggle->setChecked(renderAtts->GetDepthCueingAutomatic());
            depthCueingAutoToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_startCuePoint:
            depthCueingStartEdit->blockSignals(true);
            tmp = DoublesToQString(renderAtts->GetStartCuePoint(), 3);
            depthCueingStartEdit->setText(tmp);
            depthCueingStartEdit->blockSignals(false);
            break;
        case RenderingAttributes::ID_endCuePoint:
            depthCueingEndEdit->blockSignals(true);
            tmp = DoublesToQString(renderAtts->GetEndCuePoint(), 3);
            depthCueingEndEdit->setText(tmp);
            depthCueingEndEdit->blockSignals(false);
            break;
        case RenderingAttributes::ID_compressionActivationMode:
            itmp = (int) renderAtts->GetCompressionActivationMode();
            if (itmp == 2) // Auto for atts's enum type order
                itmp2 = 0; // Order of Auto in window
            else if (itmp == 1) // Always for atts' enum type order
                itmp2 = 1; // Order of Always in window.
            else           // Never for atts' enum type order
                itmp2 = 2; // Order of Never in window.
            scalrenCompressMode->blockSignals(true);
            scalrenCompressMode->button(itmp2)->setChecked(true);
            scalrenCompressMode->blockSignals(false);
            break;
        case RenderingAttributes::ID_colorTexturingFlag:
            colorTexturingToggle->blockSignals(true);
            colorTexturingToggle->setChecked(renderAtts->GetColorTexturingFlag());
            colorTexturingToggle->blockSignals(false);
            break;
        }
    }

    UpdateWindowSensitivity();
}


// ****************************************************************************
//  Method:  QvisRenderingWindow::UpdateWindowSensitivity
//
//  Purpose:
//    Set the sensitivity of all widgets in this window based on
//    values in the attributes.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
//  Modifications:
//    Brad Whitlock, Thu Jun 19 13:44:11 PDT 2008
//    Qt 4.
//
//    Cyrus Harrison, Mon Jul 28 15:23:05 PDT 2008
//    I added code to enable/disable the scalable auto threshold spin box based
//    on the scalable rendering mode.
//
//    Jeremy Meredith, Fri Apr 30 15:04:34 EDT 2010
//    Added an automatic start/end setting capability for depth cueing.
//
//    Dave Pugmire, Tue Aug 24 11:32:12 EDT 2010
//    Add compact domain options.
//
//    Hank Childs, Wed Oct 12 07:26:39 PDT 2011
//    Don't disable shadowing or depth cueing ... they now manually force
//    SR to always.
//
//    Eric Brugger, Tue Oct 25 12:32:40 PDT 2011
//    Add a multi resolution display capability for AMR data.
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateWindowSensitivity()
{
    bool multiresolutionOn = renderAtts->GetMultiresolutionMode();
    bool scalableAuto =
        renderAtts->GetScalableActivationMode() == RenderingAttributes::Auto;
    bool compactAuto =
        renderAtts->GetCompactDomainsActivationMode() == RenderingAttributes::Auto;
    bool shadowOn = renderAtts->GetDoShadowing();
    bool depthCueingOn = renderAtts->GetDoDepthCueing();
    bool depthCueingAuto = renderAtts->GetDepthCueingAutomatic();
    bool stereoOn = renderAtts->GetStereoRendering();
    bool specularOn = renderAtts->GetSpecularFlag();

    multiresolutionSmallestCellLabel->setEnabled(multiresolutionOn);
    multiresolutionSmallestCellLineEdit->setEnabled(multiresolutionOn);

    scalrenAutoThreshold->setEnabled(scalableAuto);
    compactDomainsAutoThreshold->setEnabled(compactAuto);
    shadowStrengthSlider->setEnabled(shadowOn);
    shadowStrengthLabel->setEnabled(shadowOn);

    depthCueingAutoToggle->setEnabled(depthCueingOn);
    depthCueingStartEdit->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingStartLabel->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingEndEdit->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingEndLabel->setEnabled(depthCueingOn && !depthCueingAuto);

    redblue->setEnabled(stereoOn);
    interlace->setEnabled(stereoOn);
    crystalEyes->setEnabled(stereoOn);
    redgreen->setEnabled(stereoOn);

    specularStrengthSlider->setEnabled(specularOn);
    specularPowerSlider->setEnabled(specularOn);
    specularStrengthLabel->setEnabled(specularOn);
    specularPowerLabel->setEnabled(specularOn);
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateInformation
//
// Purpose:
//   Updates the information labels with statistics from the viewer.
//
// Arguments:
//   doAll : Whether or not to ignore selection and update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 11:29:35 PDT 2002
//
// Modifications:
//   Eric Brugger, Fri Apr 18 11:52:33 PDT 2003
//   I removed auto center view.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Wed Jan  5 16:23:01 PST 2005
//   Fixed problem with case lables not matching Select statements
//
//   Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//   Changed GetNumTriangles to GetNumPrimitives
//
//   Mark C. Miller, Thu Jul 21 20:16:42 PDT 2005
//   Added a break; statement after case label 16
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added seconds per frame for < 1 fps cases
//
//   Brad Whitlock, Mon Dec 17 10:30:33 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Kathleen Biagas, Thu Jan 21, 2021
//   Replace QString.asprintf with QString.arg and QString.setNum
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateInformation(bool doAll)
{
    QString tmp;
    float fps;
    int j;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < windowInfo->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!windowInfo->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case WindowInformation::ID_activeSource:
        case WindowInformation::ID_activeTimeSlider:
        case WindowInformation::ID_timeSliders:
        case WindowInformation::ID_timeSliderCurrentStates:
        case WindowInformation::ID_animationMode:
        case WindowInformation::ID_interactionMode:
        case WindowInformation::ID_boundingBoxNavigate:
        case WindowInformation::ID_spin:
        case WindowInformation::ID_fullFrame:
        case WindowInformation::ID_perspective:
        case WindowInformation::ID_lockView:
        case WindowInformation::ID_lockTools:
        case WindowInformation::ID_lockTime:
        case WindowInformation::ID_viewExtentsType:
        case WindowInformation::ID_viewDimension:
        case WindowInformation::ID_viewKeyframes:
        case WindowInformation::ID_cameraViewMode:
            break;
        case WindowInformation::ID_usingScalableRendering:
            scalrenUsingLabel->setText(windowInfo->GetUsingScalableRendering() ? tr("yes") : tr("no"));
            break;
        case WindowInformation::ID_lastRenderMin:
            // Determine the fps.
            if(windowInfo->GetLastRenderMin() > 0. &&
               windowInfo->GetLastRenderMin() < 1.e05)
                fps = 1. / windowInfo->GetLastRenderMin();
            else
                fps = 0.;
            tmp = QString("%1").arg(fps,0,'g',3);
            fpsMaxLabel->setText(tmp);
            break;
        case WindowInformation::ID_lastRenderAvg:
            // Determine the fps.
            if(windowInfo->GetLastRenderAvg() > 0.)
                fps = 1. / windowInfo->GetLastRenderAvg();
            else
                fps = 0.;
            if (fps > 0.0 && fps < 1.0)
            {
                fps = 1.0 / fps;
                fpsLabel->setText(tr("Seconds per frame:"));
            }
            else
            {
                fpsLabel->setText(tr("Frames per second:"));
            }
            tmp = QString("%1").arg(fps,0,'g',3);
            fpsAvgLabel->setText(tmp);
            break;
        case WindowInformation::ID_lastRenderMax:
            // Determine the fps.
            if(windowInfo->GetLastRenderMax() > 0.)
                fps = 1. / windowInfo->GetLastRenderMax();
            else
                fps = 0.;
            tmp = QString("%1").arg(fps,0,'g',3);
            fpsMinLabel->setText(tmp);
            break;
        case WindowInformation::ID_numPrimitives:
            tmp.setNum(windowInfo->GetNumPrimitives());
            approxNumPrimitives->setText(tmp);
            break;
        case WindowInformation::ID_extents:
            for(j = 0; j < 6; ++j)
            {
                double d = windowInfo->GetExtents()[j];
                if(d == DBL_MAX || d == -DBL_MAX)
                    tmp = tr("not set");
                else
                    tmp.setNum(windowInfo->GetExtents()[j]);
                extents[j]->setText(tmp);
            }
            break;
        case WindowInformation::ID_windowSize:
        case WindowInformation::ID_winMode:
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisRenderingWindow::Apply
//
// Purpose:
//   Tells the viewer to apply the rendering attributes.
//
// Arguments:
//   ignore : A flag that causes us to unconditionally apply the
//            rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:49:19 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        renderAtts->Notify();
        GetViewerMethods()->SetRenderingAttributes();
    }
    else
        renderAtts->Notify();
}

// ****************************************************************************
// Method: QvisRenderingWindow::SubjectRemoved
//
// Purpose:
//   This method makes sure that we don't Detach from the subject if it is
//   destroyed first.
//
// Arguments:
//   TheRemovedSubject : The subject being deleted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:50:04 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == renderAtts)
        renderAtts = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
// Method: QvisRenderingWindow::ConnectRenderingAttributes
//
// Purpose:
//   Makes this window observe the rendering attributes.
//
// Arguments:
//   w : The rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:11 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::ConnectRenderingAttributes(RenderingAttributes *w)
{
    renderAtts = w;
    renderAtts->Attach(this);
}

// ****************************************************************************
// Method: QvisRenderingWindow::ConnectWindowInformation
//
// Purpose:
//   Makes this window observe the window information.
//
// Arguments:
//   w : The window information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:11 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::ConnectWindowInformation(WindowInformation *w)
{
    windowInfo = w;
    windowInfo->Attach(this);
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisRenderingWindow::apply
//
// Purpose:
//   This Qt slot function is called by clicking the apply button.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:45 PST 2002
//
// Modifications:
//   Jeremy Meredith, Wed Aug 29 15:27:16 EDT 2007
//   Added call to GetCurrentValues.
//
// ****************************************************************************

void
QvisRenderingWindow::apply()
{
    GetCurrentValues();
    Apply(true);
}

// ****************************************************************************
// Method: QvisRenderingWindow::antialiasingToggled
//
// Purpose:
//   This Qt slot function is called when the antialiasing checkbox is clicked.
//
// Arguments:
//   val : The new AA value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:52:07 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::antialiasingToggled(bool val)
{
    renderAtts->SetAntialiasing(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::updateDepthPeeling
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:   Sun Sep  6 08:42:01 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateDepthPeeling()
{
    renderAtts->SetDepthPeeling(depthPeeling->isChecked());
    renderAtts->SetOcclusionRatio(occlusionRatio->text().toDouble());
    renderAtts->SetNumberOfPeels(numberOfPeels->text().toInt());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::updateOrderedComposite
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:   Sun Sep  6 08:42:01 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateOrderedComposite()
{
    renderAtts->SetOrderComposite(orderedComposite->isChecked());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::depthCompositeThreads
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:  Tue Sep 29 11:52:04 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateDepthCompositeThreads()
{
    renderAtts->SetDepthCompositeThreads(depthCompositeThreads->text().toInt());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::alphaCompositeThreads
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:  Tue Sep 29 11:52:04 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateAlphaCompositeThreads()
{
    renderAtts->SetAlphaCompositeThreads(alphaCompositeThreads->text().toInt());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::depthCompositeBlocking
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:  Tue Sep 29 11:52:04 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateDepthCompositeBlocking()
{
    renderAtts->SetDepthCompositeBlocking(depthCompositeBlocking->text().toInt());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::alphaCompositeBlocking
//
// Purpose:
//   Update the rendering attributes
//
// Programmer: Burlen Loring
// Creation:  Tue Sep 29 11:52:04 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::updateAlphaCompositeBlocking()
{
    renderAtts->SetAlphaCompositeBlocking(alphaCompositeBlocking->text().toInt());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::multiresolutionModeToggled
//
// Purpose:
//   This Qt slot function is called when the multiresolution mode checkbox is
//   clicked.
//
// Arguments:
//   val : The new multi resolution value.
//
// Programmer: Eric Brugger
// Creation:   Tue Oct 25 12:32:40 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::multiresolutionModeToggled(bool val)
{
    renderAtts->SetMultiresolutionMode(val);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisRenderingWindow::processMultiresolutionSmallestCellText
//
// Purpose:
//   This Qt slot function is called when the multiresolution smallest
//   cell text is changed.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Tue Oct 25 12:32:40 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::processMultiresolutionSmallestCellText()
{
    double temp = -1.0;
    bool okay = sscanf(multiresolutionSmallestCellLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        renderAtts->SetMultiresolutionCellSize(temp);
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisRenderingWindow::processMultiresolutionSmallestCellText
//
// Purpose:
//   This Qt slot function is called when the multiresolution smallest
//   cell text is changed.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Tue Oct 25 12:32:40 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::processMultiresolutionSmallestCellText(const QString &tols)
{
    double temp = -1.0;
    bool okay = sscanf(multiresolutionSmallestCellLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        renderAtts->SetMultiresolutionCellSize(temp);
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisRenderingWindow::objectRepresentationChanged
//
// Purpose:
//   This Qt slot function is called when we change surface representations.
//
// Arguments:
//   val : The new surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:53:28 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::objectRepresentationChanged(int val)
{
    renderAtts->SetGeometryRepresentation(
        (RenderingAttributes::GeometryRepresentation)val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::stereoToggled
//
// Purpose:
//   This Qt slot function is called when we change turn stereo on/off.
//
// Arguments:
//   val : The new stereo enabled mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:54:52 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::stereoToggled(bool val)
{
    renderAtts->SetStereoRendering(val);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::stereoTypeChanged
//
// Purpose:
//   This Qt slot function is called when the stereo type changes.
//
// Arguments:
//   val : The new stereo type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:55:32 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::stereoTypeChanged(int val)
{
    renderAtts->SetStereoType((RenderingAttributes::StereoTypes)val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::renderNotifyToggled
//
// Purpose:
//   This Qt slot function is called when the "Query after render" toggle
//   button is clicked.
//
// Arguments:
//   val : The new query mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:56:12 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::renderNotifyToggled(bool val)
{
    renderAtts->SetNotifyForEachRender(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalrenActivationModeChanged
//
// Purpose:
//   This Qt slot function is called when the scalable activation mode changes.
//
// Arguments:
//   val : The new scalable activation mode flag.
//         0 = Auto, 1 = Always, 2 = Never
//
// Programmer: Mark C. Miller
// Creation:   Wed Nov 20 17:09:59 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added scalrenAutoThreshold spinbox and geometry label
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Dec  4 18:41:39 PST 2005
//   Automatically update the shadow toggle ['5596].
//
//   Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//   Moved window sensitivity handling to its own function.
//
// ****************************************************************************

void
QvisRenderingWindow::scalrenActivationModeChanged(int val)
{
    if (val == 0)
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Auto);
        scalrenAutoThresholdChanged(scalrenAutoThreshold->value());
    }
    else if (val == 1)
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Always);
    }
    else
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Never);
    }
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisRenderingWindow::InterpretScalableAutoThreshold
//
// Purpose: Determine widget controls given scalable auto threshold
//
// Programmer: Mark C. Miller
// Creation:   January 4, 2005
//
// ****************************************************************************
void
QvisRenderingWindow::InterpretScalableAutoThreshold(int actualVal,
    int *step, QString *suffix, int *widgetVal) const
{
    // compute the step size and suffix for the displayed value in the GUI
    if      (actualVal < 1e3) { *step = 50;  *suffix = "  Polys"; }
    else if (actualVal < 1e4) { *step = 500; *suffix = "  Polys"; }
    else if (actualVal < 1e5) { *step = 5;   *suffix = " KPolys"; }
    else if (actualVal < 1e6) { *step = 50;  *suffix = " KPolys"; }
    else if (actualVal < 1e7) { *step = 500; *suffix = " KPolys"; }
    else if (actualVal < 1e8) { *step = 5;   *suffix = " MPolys"; }
    else if (actualVal < 1e9) { *step = 50;  *suffix = " MPolys"; }
    else                      { *step = 1;   *suffix = " GPolys"; }

    // compute the divisor for the displayed value in the GUI
    int div;
    if      (*suffix == " KPolys") div = (int) 1e3;
    else if (*suffix == " MPolys") div = (int) 1e6;
    else if (*suffix == " GPolys") div = (int) 1e9;
    else                          div = 1;

    *widgetVal = actualVal / div;
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalrenAutoThresholdChanged
//
// Purpose:
//   This Qt slot function is called when the scalable rendering automatic
//   polygon count threshold changes.
//
// Arguments:
//   val : The new polygon count threshold.
//
// Programmer: Mark C. Miller
// Creation:   Wed Apr 21 22:42:57 PDT 2004
//
// Modifications:
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//   Changed to use InterpretScalableAutoThreshold
//
//   Brad Whitlock, Thu Jun 19 13:45:24 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisRenderingWindow::scalrenAutoThresholdChanged(int val)
{
    // determine the actual value from the given value and the current suffix
    int actualVal;
    if (scalrenAutoThreshold->suffix() == " GPolys")
        actualVal = val * (int) 1e9;
    else if (scalrenAutoThreshold->suffix() == " MPolys")
        actualVal = val * (int) 1e6;
    else if (scalrenAutoThreshold->suffix() == " KPolys")
        actualVal = val * (int) 1e3;
    else if (scalrenAutoThreshold->suffix() == "  Polys")
        actualVal = val;
    else
        actualVal = val;

    // deal with stepping backwards
    int step = scalrenAutoThreshold->singleStep();
    if ((val == 500) && (step == 500) && (scalrenAutoThreshold->suffix() == "  Polys"))
        actualVal = 950;
    else if ((val == 5) && (step == 5) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 9500;
    else if ((val == 50) && (step == 50) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 95000;
    else if ((val == 500) && (step == 500) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 950000;
    else if ((val == 5) && (step == 5) && (scalrenAutoThreshold->suffix() == " MPolys"))
        actualVal = 9500000;
    else if ((val == 50) && (step == 50) && (scalrenAutoThreshold->suffix() == " MPolys"))
        actualVal = 95000000;
    else if ((val == 0) && (step == 1) && (scalrenAutoThreshold->suffix() == " GPolys"))
        actualVal = 950000000;

    int widgetVal;
    QString suffix;
    InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);

    // set the new GUI value
    scalrenAutoThreshold->setSingleStep(step);
    scalrenAutoThreshold->setSuffix(suffix);
    scalrenAutoThreshold->setValue(widgetVal);

    renderAtts->SetScalableAutoThreshold(actualVal);

    SetUpdate(false);
    Apply();
}



// ****************************************************************************
// Method:  QvisRenderingWindow::compactDomainsAutoThresholdChanged
//
// Programmer:  Dave Pugmire
// Creation:    August 24, 2010
//
// ****************************************************************************

void
QvisRenderingWindow::compactDomainsAutoThresholdChanged(int val)
{
    renderAtts->SetCompactDomainsAutoThreshold(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::compactDomainsActivationModeChanged
//
// Programmer: Dave Pugmire
// Creation:   August 24, 2010
//
// ****************************************************************************

void
QvisRenderingWindow::compactDomainsActivationModeChanged(int mode)
{
    if (mode == 0)
    {
        renderAtts->SetCompactDomainsActivationMode(RenderingAttributes::Auto);
        compactDomainsAutoThresholdChanged(compactDomainsAutoThreshold->value());
    }
    else if (mode == 1)
        renderAtts->SetCompactDomainsActivationMode(RenderingAttributes::Always);
    else
        renderAtts->SetCompactDomainsActivationMode(RenderingAttributes::Never);

    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalrenCompressModeChanged
//
// Programmer: Mark C. Miller
// Creation:   November 2, 2005
//
// ****************************************************************************

void
QvisRenderingWindow::scalrenCompressModeChanged(int mode)
{
    int itmp = 0;
    if (mode == 0)      // Auto in Window
        itmp = 2;       // Auto for atts' enum type
    else if (mode == 1) // Always in window
        itmp = 1;       // Always for atts' enum type
    else                // Never in window.
        itmp = 0;       // Never for atts' enum type

    renderAtts->SetCompressionActivationMode((RenderingAttributes::TriStateMode)itmp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::shadowToggled
//
//  Purpose:
//    Callback for the shadow toggle button
//
//  Arguments:
//    val        true to enable shadow
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//    Moved window sensitivity handling to its own function.
//
//    Hank Childs, Wed Oct 12 07:26:39 PDT 2011
//    Add new behavior for switching SR to always when shadows are enabled.
//
// ****************************************************************************

void
QvisRenderingWindow::shadowToggled(bool val)
{
    bool doUpdate = false;
    if (val == true)
    {
        if (renderAtts->GetScalableActivationMode() != RenderingAttributes::Always)
        {
            Warning(tr("As shadows only work with VisIt's software rendering mode, software rendering is now being enabled.  If you turn off shadows at a later time, you must manually disable software rendering.  You do this by setting \"Use scalable rendering\" to \"Auto\"."));
            renderAtts->SetScalableActivationMode(RenderingAttributes::Always);
            doUpdate = true;
        }
    }
    renderAtts->SetDoShadowing(val);
    SetUpdate(doUpdate);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::shadowStrengthChanged
//
//  Purpose:
//    Callback for the shadow coefficient slider
//
//  Arguments:
//    val        the new coefficient
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
// ****************************************************************************

void
QvisRenderingWindow::shadowStrengthChanged(int val, const void*)
{
    renderAtts->SetShadowStrength(float(val)/100.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularToggled
//
//  Purpose:
//    Callback for the specular toggle button
//
//  Arguments:
//    val        true to enable specular
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//    Moved window sensitivity handling to its own function.
//
// ****************************************************************************

void
QvisRenderingWindow::specularToggled(bool val)
{
    renderAtts->SetSpecularFlag(val);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularStrengthChanged
//
//  Purpose:
//    Callback for the specular coefficient slider
//
//  Arguments:
//    val        the new coefficient
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
QvisRenderingWindow::specularStrengthChanged(int val, const void*)
{
    renderAtts->SetSpecularCoeff(float(val)/100.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularPowerChanged
//
//  Purpose:
//    Callback for the specular exponent slider.
//
//  Arguments:
//    val        the new value
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
QvisRenderingWindow::specularPowerChanged(int val, const void*)
{
    renderAtts->SetSpecularPower(float(val)/10.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::colorTexturingToggled
//
// Purpose:
//   This is a Qt slot function that is called when the colorTexturing check
//   box is toggled.
//
// Arguments:
//   val : The new on/off value for the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 10:52:30 PDT 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::colorTexturingToggled(bool val)
{
    renderAtts->SetColorTexturingFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingToggled
//
//  Purpose:
//    Triggered when depth cueing is toggled.
//
//  Arguments:
//    val        the new value for the flag
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
//  Modifications:
//
//    Hank Childs, Wed Oct 12 07:26:39 PDT 2011
//    Add new behavior for switching SR to always when shadows are enabled.
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingToggled(bool val)
{
    bool doUpdate = false;
    if (val == true)
    {
        if (renderAtts->GetScalableActivationMode() != RenderingAttributes::Always)
        {
            Warning(tr("As depth cueing only work with VisIt's software rendering mode, software rendering is now being enabled.  If you turn off depth cueing at a later time, you must manually disable software rendering.  You do this by setting \"Use scalable rendering\" to \"Auto\"."));
            renderAtts->SetScalableActivationMode(RenderingAttributes::Always);
            doUpdate = true;
        }
    }
    renderAtts->SetDoDepthCueing(val);
    UpdateWindowSensitivity();
    SetUpdate(doUpdate);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingAutoToggled
//
//  Purpose:
//    Triggered when depth cueing automatic mode is toggled.
//
//  Arguments:
//    val        the new value for the flag
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 30, 2010
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingAutoToggled(bool val)
{
    renderAtts->SetDepthCueingAutomatic(val);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingStartChanged
//
//  Purpose:
//    Triggered when return is pressed in the depth cueing start point widget.
//
//  Arguments:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingStartChanged()
{
    GetCurrentValues();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingEndChanged
//
//  Purpose:
//    Triggered when return is pressed in the depth cueing end point widget.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingEndChanged()
{
    GetCurrentValues();
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisRenderingWindow::GetCurrentValues
//
//  Purpose:
//    Fill the values in the attributes from the ones in widgets which
//    do not update continuously (like line edits).
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Thu Jun 19 13:34:38 PDT 2008
//    Use convenience methods.
//
//    Jeremy Meredith, Fri Apr 30 14:06:35 EDT 2010
//    Fix bug.
//
// ****************************************************************************

void
QvisRenderingWindow::GetCurrentValues()
{
    double v[3];

    if(LineEditGetDoubles(depthCueingStartEdit, v, 3))
        renderAtts->SetStartCuePoint(v);
    else
    {
        ResettingError(tr("start point"),
                       DoublesToQString(renderAtts->GetStartCuePoint(), 3));
        renderAtts->SetStartCuePoint(renderAtts->GetStartCuePoint());
    }

    if(LineEditGetDoubles(depthCueingEndEdit, v, 3))
        renderAtts->SetEndCuePoint(v);
    else
    {
        ResettingError(tr("end point"),
                       DoublesToQString(renderAtts->GetEndCuePoint(), 3));
        renderAtts->SetEndCuePoint(renderAtts->GetEndCuePoint());
    }
}


// ****************************************************************************
// Method: QvisRenderingWindow::osprayRenderingToggled
//
// Purpose:
//    Triggered when ospray rendering is toggled.
//
// Programmer:  Garrett Morrison
// Creation:    Wed May 16 17:42:42 PDT 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::osprayRenderingToggled(bool val)
{
    renderAtts->SetOsprayRendering(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::ospraySPPChanged
//
// Purpose:
//    Triggered when ospray samples per pixel are changed.
//
//  Arguments:
//    val        the new value
//
// Programmer:  Garrett Morrison
// Creation:    Wed May 16 17:42:42 PDT 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::ospraySPPChanged(int val)
{
    renderAtts->SetOspraySPP(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::osprayAOChanged
//
// Purpose:
//    Triggered when ospray ambient occlusion samples are changed.
//
//  Arguments:
//    val        the new value
//
// Programmer:  Garrett Morrison
// Creation:    Wed May 16 17:42:42 PDT 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::osprayAOChanged(int val)
{
    renderAtts->SetOsprayAO(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::osprayShadowsToggled
//
// Purpose:
//    Triggered when ospray shadows are toggled.
//
// Programmer:  Garrett Morrison
// Creation:    Wed May 16 17:42:42 PDT 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::osprayShadowsToggled(bool val)
{
    renderAtts->SetOsprayShadows(val);
    SetUpdate(false);
    Apply();
}
