// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisRenderingWindow.h>
#include <float.h>
#include <limits.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QTabWidget>

#include <RenderingAttributes.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>
#include <QvisOpacitySlider.h>

#ifdef HAVE_ANARI
    #include <AnariRenderingWidget.h>
#endif

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
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
//   Kathleen Biagas, Wed May 14, 2025
//   Remove 'Requires restart' label from antialiasingToggle.
//
//   Kathleen Biagas, Mon Jul 28, 2025
//   Change antialiasingToggle checkbox to antialiasingMode buttonGroup
//   to hold the different Antialiasing modes.
//
//   Kathleen Biagas, Thu Aug 14, 2025
//   Removed void* arg from SIGNAL and SLOT for QvisOpacitySlider as the arg
//   isn't needed for these instances.
//   Use QGroupBox and QFormLayout to better organize the page.
//   Add widgets for msaaSamples and fxaa options.
//
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateBasicPage()
{
    int row = 0;
    QWidget *basicOptions = new QWidget(central);
    QVBoxLayout *basicLayout = new QVBoxLayout(basicOptions);
    basicLayout->setContentsMargins(10,10,10,10);

    //
    // Create the antialiasing widgets.
    //
    QGroupBox *aaGroup = new QGroupBox(tr("Antialiasing"));
    aaGroup->setCheckable(false);
    basicLayout->addWidget(aaGroup);

    QGridLayout *aaLayout = new QGridLayout();
    aaLayout->setContentsMargins(10,10,10,10);
    aaGroup->setLayout(aaLayout);

    int aaRow= 0;
    QLabel *aaLabel = new QLabel(tr("MSAA will be disabled if Depth Peeling also selected."), basicOptions);
    aaLayout->addWidget(aaLabel, aaRow, 0, 1, 3);
    aaRow++;

    antialiasingMode = new QButtonGroup(basicOptions);
    connect(antialiasingMode, SIGNAL(idClicked(int)),
            this, SLOT(antialiasingChanged(int)));

    QRadioButton *aaNone = new QRadioButton(tr("None"), basicOptions);
    antialiasingMode->addButton(aaNone, 0);
    aaLayout->addWidget(aaNone, aaRow, 0);
    QRadioButton *aaMSAA = new QRadioButton(tr("MSAA"), basicOptions);
    antialiasingMode->addButton(aaMSAA, 1);
    aaLayout->addWidget(aaMSAA, aaRow, 1);
    QRadioButton *aaFXAA = new QRadioButton(tr("FXAA"), basicOptions);
    antialiasingMode->addButton(aaFXAA, 2);
    aaLayout->addWidget(aaFXAA, aaRow, 2);
    aaRow++;

    // MSAA options

    msaaSamplesLabel = new QLabel(tr("Number of MSAA samples"));
    msaaSamplesLabel->setEnabled(false);
    aaLayout->addWidget(msaaSamplesLabel, aaRow, 0);

    msaaSamples = new QSpinBox();
    msaaSamples->setKeyboardTracking(false);
    msaaSamples->setMinimum(2);
    msaaSamples->setMaximum(8);
    msaaSamples->setValue(4);
    msaaSamples->setSingleStep(2);
    msaaSamples->setEnabled(false);
    aaLayout->addWidget(msaaSamples, aaRow, 1);
    connect(msaaSamples, SIGNAL(valueChanged(int)),
            this, SLOT(msaaSamplesChanged(int)));
    aaRow++;

    // FXAA options
    QFormLayout *fxaaLeftLayout = new QFormLayout();
    aaLayout->addLayout(fxaaLeftLayout, aaRow,0, 1, 2);
    QFormLayout *fxaaRightLayout = new QFormLayout();
    aaLayout->addLayout(fxaaRightLayout, aaRow, 2, 1, 2);

    // RelativeConstrastThreshold default values and custom value widget
    QDoubleValidator *dvfxaa = new QDoubleValidator(0.f,1.f, 5);

    fxaaRCTLabel = new QLabel(tr("Relative contrast threshold"));
    fxaaRCTLabel->setToolTip(
        tr("Threshold for applying FXAA to a pixel, relative to the maximum luminosity of its 4 immediate neighbors\n"
           "The luminosity of the current pixel and it's NSWE neighbors is computed. The maximum luminosity and luminosity range (contrast) of all 5 pixels is found. If the contrast is less than RelativeContrastThreshold * maxLum, the pixel is not considered aliased and will not be affected by FXAA.\n"));
    fxaaRCT = new QComboBox();
    fxaaRCT->addItem("Too little", 0.3333);
    fxaaRCT->addItem("Low quality", 0.25);
    fxaaRCT->addItem("High quality", 0.125);
    fxaaRCT->addItem("Overkill", 0.0625);
    fxaaRCT->addItem("Custom", 0.125);
    fxaaRCT->setCurrentIndex(2);
    connect(fxaaRCT, SIGNAL(currentIndexChanged(int)),
            this, SLOT(fxaaRCTChanged(int)));
    fxaaLeftLayout->addRow(fxaaRCTLabel, fxaaRCT);

    fxaaRCTCustomLabel = new QLabel(tr("Custom RCT value"));
    fxaaRCTCustom = new QLineEdit("0.0625");
    fxaaRCTCustom->setValidator(dvfxaa);
    connect(fxaaRCTCustom, SIGNAL(editingFinished()),
            this, SLOT(fxaaRCTCustomChanged()));
    fxaaLeftLayout->addRow(fxaaRCTCustomLabel, fxaaRCTCustom);

    // HardConstrastThreshold default values and custom value widgets
    fxaaHCTLabel = new QLabel(tr("Hard contrast threshold"));
    fxaaHCTLabel->setToolTip(
        tr("Similar to RelativeContrastThreshold, but not scaled by the maximum luminosity.\n"
           "If the contrast of the current pixel and it's 4 immediate NSWE neighbors is less than HardContrastThreshold, the pixel is not considered aliased and will not be affected by FXAA.\n"));
    fxaaHCT = new QComboBox();
    fxaaHCT->addItem("VisibleLimit", 0.03125);
    fxaaHCT->addItem("HigherQuality", 0.0625);
    fxaaHCT->addItem("UpperLimit", 0.08333);
    fxaaHCT->addItem("Custom", 0.0625);
    fxaaHCT->setCurrentIndex(1);
    connect(fxaaHCT, SIGNAL(currentIndexChanged(int)),
            this, SLOT(fxaaHCTChanged(int)));
    fxaaRightLayout->addRow(fxaaHCTLabel, fxaaHCT);

    fxaaHCTCustomLabel = new QLabel(tr("Custom HCT value"));
    fxaaHCTCustom = new QLineEdit("0.0625");
    fxaaHCTCustom->setValidator(dvfxaa);
    connect(fxaaHCTCustom, SIGNAL(editingFinished()),
            this, SLOT(fxaaHCTCustomChanged()));
    fxaaRightLayout->addRow(fxaaHCTCustomLabel, fxaaHCTCustom);

    // SubpixelBlendLimit default values and custom value widgets
    fxaaSBLLabel = new QLabel(tr("Subpixel blend limit"));
    fxaaSBLLabel->setToolTip(
        tr("Subpixel aliasing is corrected by applying a lowpass filter\n"
           "to the current pixel. This is implemented by blending an\n"
           "average of the 3x3 neighborhood around the pixel into the\n"
           "final result. The amount of blending is determined by\n"
           "comparing the detected amount of subpixel aliasing to the\n"
           "total contrasting of the CNSWE pixels:\n"
           "SubpixelBlending = abs(lumC - lumAveNSWE) / (lumMaxCNSWE - lumMinCNSWE)\n"
           "This parameter sets an upper limit to the amount of subpixel\n"
           "blending to prevent the image from simply getting blurred.\n"));
    fxaaSBL = new QComboBox();
    fxaaSBL->addItem("Low", 0.5);
    fxaaSBL->addItem("Medium", 0.75);
    fxaaSBL->addItem("High", 0.875);
    fxaaSBL->addItem("Maximum", 1.0);
    fxaaSBL->addItem("Custom", 0.75);
    fxaaSBL->setCurrentIndex(1);
    connect(fxaaSBL, SIGNAL(currentIndexChanged(int)),
            this, SLOT(fxaaSBLChanged(int)));
    fxaaLeftLayout->addRow(fxaaSBLLabel, fxaaSBL);

    fxaaSBLCustomLabel = new QLabel(tr("Custom SBL value"));
    fxaaSBLCustom = new QLineEdit("0.75");
    fxaaSBLCustom->setValidator(dvfxaa);
    connect(fxaaSBLCustom, SIGNAL(editingFinished()),
            this, SLOT(fxaaSBLCustomChanged()));
    fxaaLeftLayout->addRow(fxaaSBLCustomLabel, fxaaSBLCustom);

    // SubpixelContrastThreshold default values and custom value widgets
    fxaaSCTLabel = new QLabel(tr("Subpixel contrast threshold"));
    fxaaSCTLabel->setToolTip(
        tr("Minimum amount of subpixel aliasing required for subpixel\n"
           "antialiasing to be applied.\n"
           "If SubpixelBlending is less than this threshold,\n"
           "no lowpass blending will occur.\n"));
    fxaaSCT = new QComboBox();
    fxaaSCT->addItem("Low", 0.5);
    fxaaSCT->addItem("Medium", 0.75);
    fxaaSCT->addItem("High", 0.875);
    fxaaSCT->addItem("Maximum", 1.0);
    fxaaSCT->addItem("Custom", 0.75);
    fxaaSCT->setCurrentIndex(1);
    connect(fxaaSCT, SIGNAL(currentIndexChanged(int)),
            this, SLOT(fxaaSCTChanged(int)));
    fxaaRightLayout->addRow(fxaaSCTLabel, fxaaSCT);

    fxaaSCTCustomLabel = new QLabel(tr("Custom SCT value"));
    fxaaSCTCustom = new QLineEdit("0.75");
    fxaaSCTCustom->setValidator(dvfxaa);
    connect(fxaaSCTCustom, SIGNAL(editingFinished()),
            this, SLOT(fxaaSCTCustomChanged()));
    fxaaRightLayout->addRow(fxaaSCTCustomLabel, fxaaSCTCustom);

    // UseHighQualityEndpoint default values and custom value widgets
    fxaaHQE = new QCheckBox(tr("Use high quality endpoints"));
    fxaaHQE->setCheckState(Qt::Checked);
    fxaaHQE->setToolTip(
        tr("Use an improved edge endpoint detection algorithm.\n"
            "If true, a modified edge endpoint detection algorithm is used\n"
            "that requires more texture lookups, but will properly detect\n"
            "aliased single-pixel lines.\n"
            "If false, the edge endpoint algorithm proposed by NVIDIA will\n"
            "be used. This algorithm is faster (fewer lookups), but will \n"
            "fail to detect endpoints of single pixel edge steps.\n"));
    connect(fxaaHQE, SIGNAL(toggled(bool)),
            this, SLOT(fxaaHQEToggled(bool)));
    fxaaLeftLayout->addRow(fxaaHQE);

    // EndpointSearchIterations default values and custom value widgets
    fxaaESILabel = new QLabel(tr("Endpoint search iterations"));
    fxaaESILabel->setToolTip(
        tr("Set the number of iterations for the endpoint search algorithm.\n"
           "Increasing this value will increase runtime, but also properly\n"
           "detect longer edges. The current implementation steps one pixel\n"
           "in both the positive and negative directions per iteration.\n"
           "The default value is 12, which will resolve endpoints of\n"
           "edges < 25 pixels long (2 * 12 + 1).\n"));

    QIntValidator *ivfxaa = new QIntValidator(0,10000);
    fxaaESI = new QLineEdit("12");
    fxaaESI->setValidator(ivfxaa);
    connect(fxaaESI, SIGNAL(editingFinished()),
            this, SLOT(fxaaESIChanged()));
    fxaaRightLayout->addRow(fxaaESILabel, fxaaESI);

    //
    // Create the depthPeeling widgets.
    //
    depthPeeling = new QGroupBox(tr("Depth Peeling"));
    depthPeeling->setCheckable(true);
    depthPeeling->setChecked(false);
    depthPeeling->setToolTip(
        tr("Enable depth peeling for order independent rendering of\n"
           "transparent geometry. When not using depth peeling a camera\n"
           "order sort is used. If you have a GPU this is usualy a win\n"
           "with OSMesa it will depend on the version and build options\n"
           "with VisIt's current Mesa 7.10 it is *very* slow.\n"));
    connect(depthPeeling, SIGNAL(toggled(bool)),
            this, SLOT(updateDepthPeeling(void)));
    basicLayout->addWidget(depthPeeling);

    QFormLayout *dpLayout = new QFormLayout();
    dpLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    depthPeeling->setLayout(dpLayout);

    QLabel *occlusionRatioLabel = new QLabel(tr("Occlusion ratio"));
    occlusionRatioLabel->setToolTip(
        tr("When greater than zero early terminations is enabled and\n"
           "the algorithm will stop doing peels when fewer than this\n"
           "fraction of pixels changed in the last peel. Thus one sacrifices\n"
           "accuracy for speed. When set to zero the maximum number of peels\n"
           "will be made which, when enough peels are requested, ensures a\n"
           "correct result."));
    occlusionRatio = new QLineEdit("0.01");
    QDoubleValidator *dv0 = new QDoubleValidator(0.0, 0.5, 4, 0);
    occlusionRatio->setValidator(dv0);
    connect(occlusionRatio, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthPeeling(void)));

    dpLayout->addRow(occlusionRatioLabel, occlusionRatio);

    QLabel *numberOfPeelsLabel = new QLabel(tr("Max number of Peels"));
    numberOfPeelsLabel->setToolTip(
        tr("Sets the maximum number of peels to use. Each peel renders the\n"
           "next nearest surface for a given fragment. You may need to\n"
           "increase the number of peels for very complex scenes."));
    numberOfPeels = new QLineEdit("32");
    QIntValidator *iv4 = new QIntValidator(1,1000);
    numberOfPeels->setValidator(iv4);
    connect(numberOfPeels, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthPeeling(void)));
    dpLayout->addRow(numberOfPeelsLabel, numberOfPeels);

    //
    // create the order compositing widgets
    //
    QGroupBox *compositeSettings = new QGroupBox(tr("Compositer Settings"));
    compositeSettings->setCheckable(false);
    basicLayout->addWidget(compositeSettings);
    QFormLayout *compositeLayout = new QFormLayout();
    compositeLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    compositeSettings->setLayout(compositeLayout);

    orderedComposite = new QCheckBox(tr("Ordered Compositing"));

    orderedComposite->setCheckState(Qt::Checked);
    orderedComposite->setToolTip(
        tr("Enable ordered compositing. For block stuctured domain\n"
           "decomposition ordered compositing eliminates the need\n"
           "for a global parallel camera order geometry sort when\n"
           "rendering translucent geometry. When combined with depth\n"
           "peeling all geometry sorting is eliminated\n"));
    connect(orderedComposite, SIGNAL(toggled(bool)),
            this, SLOT(updateOrderedComposite()));
    compositeLayout->addRow(orderedComposite);

    // create the depth and alpha compositing widgets
    QLabel *depthCompositeThreadsLabel = new QLabel(tr("Depth Compositer Threads"));
    depthCompositeThreadsLabel->setToolTip(
        tr("Sets the number of threads that process communication streams\n"
           "during depth compositing.\n"));
    depthCompositeThreads = new QLineEdit("2");
    QIntValidator *iv0 = new QIntValidator(0,8);
    depthCompositeThreads->setValidator(iv0);
    connect(depthCompositeThreads, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthCompositeThreads(void)));

    compositeLayout->addRow(depthCompositeThreadsLabel, depthCompositeThreads);

    QLabel *depthCompositeBlockingLabel = new QLabel(tr("Depth Compositer Blocking"));
    depthCompositeBlockingLabel->setToolTip(
        tr("Sets the block size used for streaming communication\n"
           "during depth compositing. Images are split into blocks\n"
           "of this size and streamed out. Incomning streams are\n"
           "processed in the background using compositing threads\n"));
    depthCompositeBlocking = new QLineEdit("65536");
    QIntValidator *iv1 = new QIntValidator(4096,0x3fffffff);
    depthCompositeBlocking->setValidator(iv1);
    connect(depthCompositeBlocking, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateDepthCompositeBlocking(void)));

    compositeLayout->addRow(depthCompositeBlockingLabel, depthCompositeBlocking);

    QLabel *alphaCompositeThreadsLabel = new QLabel(tr("Alpha Compositer Threads"));
    alphaCompositeThreadsLabel->setToolTip(
        tr("Sets the number of threads that process communication streams\n"
           "during alpha compositing.\n"));
    alphaCompositeThreads = new QLineEdit("2");
    QIntValidator *iv2 = new QIntValidator(0,8);
    alphaCompositeThreads->setValidator(iv2);
    connect(alphaCompositeThreads, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateAlphaCompositeThreads(void)));

    compositeLayout->addRow(alphaCompositeThreadsLabel, alphaCompositeThreads);

    QLabel *alphaCompositeBlockingLabel = new QLabel(tr("Alpha Compositer Blocking"));
    alphaCompositeBlockingLabel->setToolTip(
        tr("Sets the block size used for streaming communication\n"
           "during alpha compositing. Images are split into blocks\n"
           "of this size and streamed out. Incomning streams are\n"
           "processed in the background using compositing threads\n"));
    alphaCompositeBlocking = new QLineEdit("65536" );
    QIntValidator *iv3 = new QIntValidator(4096,0x3fffffff);
    alphaCompositeBlocking->setValidator(iv3);
    connect(alphaCompositeBlocking, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateAlphaCompositeBlocking(void)));

    compositeLayout->addRow(alphaCompositeBlockingLabel, alphaCompositeBlocking);

    //
    // Create the multi resolution widgets.
    //
    multiresolutionModeToggle = new QGroupBox(tr("Multi resolution for 2d AMR data"));
    multiresolutionModeToggle->setCheckable(true);
    multiresolutionModeToggle->setChecked(false);
    connect(multiresolutionModeToggle, SIGNAL(toggled(bool)),
            this, SLOT(multiresolutionModeToggled(bool)));
    basicLayout->addWidget(multiresolutionModeToggle);

    QFormLayout *mrLayout = new QFormLayout();
    mrLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    multiresolutionModeToggle->setLayout(mrLayout);

    QLabel *multiresolutionSmallestCellLabel = new QLabel(tr("Smallest cell"));
    multiresolutionSmallestCellLineEdit = new QLineEdit();
    connect(multiresolutionSmallestCellLineEdit, SIGNAL(editingFinished()),
            this, SLOT(processMultiresolutionSmallestCellText()));

    mrLayout->addRow(multiresolutionSmallestCellLabel, multiresolutionSmallestCellLineEdit);

    //
    // Create the surface rep widgets.
    //
    QGroupBox *drawObj = new QGroupBox(tr("Draw objects as"));
    drawObj->setCheckable(false);
    basicLayout->addWidget(drawObj);

    QGridLayout *objLayout = new QGridLayout();
    objLayout->setContentsMargins(10,10,10,10);
    drawObj->setLayout(objLayout);

    objectRepresentation = new QButtonGroup();
    connect(objectRepresentation, SIGNAL(idClicked(int)),
            this, SLOT(objectRepresentationChanged(int)));

    QRadioButton *surfaces = new QRadioButton(tr("Surfaces"));
    objectRepresentation->addButton(surfaces, 0);
    objLayout->addWidget(surfaces, 0, 0);
    QRadioButton *wires = new QRadioButton(tr("Wireframe"));
    objectRepresentation->addButton(wires, 1);
    objLayout->addWidget(wires, 0, 1);
    QRadioButton *points = new QRadioButton(tr("Points"));
    objectRepresentation->addButton(points, 2);
    objLayout->addWidget(points, 0, 2);
    objLayout->setSpacing(0);

    //
    // Create the stereo widgets.
    //
    stereoToggle = new QGroupBox(tr("Stereo"));
    stereoToggle->setCheckable(true);
    stereoToggle->setChecked(false);

    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    basicLayout->addWidget(stereoToggle);

    QGridLayout *stereoLayout = new QGridLayout();
    stereoLayout->setContentsMargins(10,10,10,10);
    stereoToggle->setLayout(stereoLayout);

    stereoType = new QButtonGroup(stereoToggle);
    connect(stereoType, SIGNAL(idClicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton(tr("Red/Blue"));
    stereoType->addButton(redblue, 0);
    stereoLayout->addWidget(redblue, 0, 0);
    interlace = new QRadioButton(tr("Interlace"));
    stereoType->addButton(interlace, 1);
    stereoLayout->addWidget(interlace, 0, 1);
    crystalEyes = new QRadioButton(tr("Crystal Eyes"));
    stereoType->addButton(crystalEyes, 2);
    stereoLayout->addWidget(crystalEyes, 1,0);
    redgreen = new QRadioButton(tr("Red/Green"));
    stereoType->addButton(redgreen, 3);
    stereoLayout->addWidget(redgreen, 1,1);
    stereoLayout->setSpacing(0);

    //
    // Create the specular lighting options
    //
    specularToggle = new QGroupBox(tr("Specular lighting"));
    specularToggle->setCheckable(true);
    specularToggle->setChecked(false);
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    basicLayout->addWidget(specularToggle);

    QFormLayout *specLayout = new QFormLayout();
    specularToggle->setLayout(specLayout);

    QLabel *specularStrengthLabel = new QLabel(tr("Strength"));
    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, specularToggle);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int)),
            this, SLOT(specularStrengthChanged(int)));

    specLayout->addRow(specularStrengthLabel, specularStrengthSlider);

    QLabel *specularPowerLabel = new QLabel(tr("Sharpness"));
    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, specularToggle);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int)),
            this, SLOT(specularPowerChanged(int)));
    specLayout->addRow(specularPowerLabel, specularPowerSlider);

    basicLayout->setSpacing(0);
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
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
//   Kathleen Biagas, Thu Aug 14, 2025
//   Removed void* arg from SIGNAL and SLOT for QvisOpacitySlider as the arg
//   isn't needed for these instances.
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
    connect(scalrenActivationMode, SIGNAL(idClicked(int)),
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
    connect(scalrenCompressMode, SIGNAL(idClicked(int)),
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
    connect(compactDomainsActivationMode, SIGNAL(idClicked(int)),
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
    connect(shadowStrengthSlider, SIGNAL(valueChanged(int)),
            this, SLOT(shadowStrengthChanged(int)));
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

#ifdef HAVE_ANARI
    // Divider
    QFrame *separator = new QFrame(advancedOptions);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(2);
    separator->setLineWidth(1);
    advLayout->addWidget(separator, row, 0, 2, 4); // row, col, rowspan, colspan
    row += 2;

    anariRenderingWidget = new AnariRenderingWidget(this, renderAtts, advancedOptions);
    int anariRowCount = anariRenderingWidget->GetRowCount();
    advLayout->addWidget(anariRenderingWidget, row, 0, anariRowCount, 4);
    row += anariRowCount;
#endif

#if defined(HAVE_OSPRAY)
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
//   Kevin Griffin, Wed Mar 05 2025 11:59:26 AM CST
//   ANARI Integration
//
//   Kathleen Biagas, Monday July 28, 2025
//   Update handling of antialiasing.
//
//   Kathleen Biagas, Thu Aug 14, 2025
//   Add handling of msaaSamples and fxaaOptions.
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
            itmp = (int)renderAtts->GetAntialiasing();
            antialiasingMode->blockSignals(true);
            antialiasingMode->button(itmp)->setChecked(true);
            UpdateAAControls(itmp);
            antialiasingMode->blockSignals(false);
            break;
        case RenderingAttributes::ID_MSAASamples:
            msaaSamples->blockSignals(true);
            msaaSamples->setValue(renderAtts->GetMSAASamples());
            msaaSamples->blockSignals(false);
            break;
        case RenderingAttributes::ID_FXAAOpt:
        {
            fxaaRCTLabel->blockSignals(true);
            fxaaRCT->blockSignals(true);
            fxaaRCTCustomLabel->blockSignals(true);
            fxaaRCTCustom->blockSignals(true);
            fxaaHCTLabel->blockSignals(true);
            fxaaHCT->blockSignals(true);
            fxaaHCTCustomLabel->blockSignals(true);
            fxaaHCTCustom->blockSignals(true);
            fxaaSBLLabel->blockSignals(true);
            fxaaSBL->blockSignals(true);
            fxaaSBLCustomLabel->blockSignals(true);
            fxaaSBLCustom->blockSignals(true);
            fxaaSCTLabel->blockSignals(true);
            fxaaSCT->blockSignals(true);
            fxaaSCTCustomLabel->blockSignals(true);
            fxaaSCTCustom->blockSignals(true);
            fxaaHQE->blockSignals(true);  
            fxaaESILabel->blockSignals(true);
            fxaaESI->blockSignals(true);  

            FXAAOptions &fxaaOpt = renderAtts->GetFXAAOpt();

            fxaaRCT->setCurrentIndex((int)fxaaOpt.GetRelativeContrastThreshold());
            tmp = FloatToQString(fxaaOpt.GetCustomRCT(),5);
            fxaaRCTCustom->setText(tmp);

            fxaaHCT->setCurrentIndex((int)fxaaOpt.GetHardContrastThreshold());
            tmp = FloatToQString(fxaaOpt.GetCustomHCT(),5);
            fxaaHCTCustom->setText(tmp);

            fxaaSBL->setCurrentIndex((int)fxaaOpt.GetSubpixelBlendLimit());
            tmp = FloatToQString(fxaaOpt.GetCustomSBL(),5);
            fxaaSBLCustom->setText(tmp);

            fxaaSCT->setCurrentIndex((int)fxaaOpt.GetSubpixelContrastThreshold());
            tmp = FloatToQString(fxaaOpt.GetCustomSCT(),5);
            fxaaSCTCustom->setText(tmp);

            fxaaHQE->setChecked(fxaaOpt.GetUseHighQualityEndpoints());

            tmp = IntToQString(fxaaOpt.GetEndpointSearchIterations());
            fxaaESI->setText(tmp);

            fxaaRCTLabel->blockSignals(false);
            fxaaRCT->blockSignals(false);
            fxaaRCTCustomLabel->blockSignals(false);
            fxaaRCTCustom->blockSignals(false);
            fxaaHCTLabel->blockSignals(false);
            fxaaHCT->blockSignals(false);
            fxaaHCTCustomLabel->blockSignals(false);
            fxaaHCTCustom->blockSignals(false);
            fxaaSBLLabel->blockSignals(false);
            fxaaSBL->blockSignals(false);
            fxaaSBLCustomLabel->blockSignals(false);
            fxaaSBLCustom->blockSignals(false);
            fxaaSCTLabel->blockSignals(false);
            fxaaSCT->blockSignals(false);
            fxaaSCTCustomLabel->blockSignals(false);
            fxaaSCTCustom->blockSignals(false);
            fxaaHQE->blockSignals(false);  
            fxaaESILabel->blockSignals(false);
            fxaaESI->blockSignals(false);  
        }
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
            depthPeeling->blockSignals(true);
            depthPeeling->setChecked(renderAtts->GetDepthPeeling());
            depthPeeling->blockSignals(false);
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
#if defined(HAVE_OSPRAY)
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
#ifdef HAVE_ANARI
        case RenderingAttributes::ID_anariRendering:
            anariRenderingWidget->SetChecked(renderAtts->GetAnariRendering());
            break;
        case RenderingAttributes::ID_anariLibrary:
            anariRenderingWidget->UpdateLibraryName(renderAtts->GetAnariLibrary());
            break;
        case RenderingAttributes::ID_anariLibrarySubtype:
            anariRenderingWidget->UpdateLibrarySubtypes(renderAtts->GetAnariLibrarySubtype());
            break;
        case RenderingAttributes::ID_anariRendererSubtype:
            anariRenderingWidget->UpdateRendererSubtypes(renderAtts->GetAnariRendererSubtype());
            break;
        case RenderingAttributes::ID_anariRendererParameters:
            anariRenderingWidget->UpdateRendererParameters(renderAtts->GetAnariRendererParameters());
            break;
        case RenderingAttributes::ID_anariUSDParameters:
            anariRenderingWidget->UpdateUSDParameters(renderAtts->GetAnariUSDParameters());
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
//    Kathleen Biagas, Thu Aug 14, 2025
//    Removed setting of widgets whose enablement is controlled by their
//    containing QGroupBox.
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateWindowSensitivity()
{
    bool scalableAuto =
        renderAtts->GetScalableActivationMode() == RenderingAttributes::Auto;
    bool compactAuto =
        renderAtts->GetCompactDomainsActivationMode() == RenderingAttributes::Auto;
    bool shadowOn = renderAtts->GetDoShadowing();
    bool depthCueingOn = renderAtts->GetDoDepthCueing();
    bool depthCueingAuto = renderAtts->GetDepthCueingAutomatic();

    scalrenAutoThreshold->setEnabled(scalableAuto);
    compactDomainsAutoThreshold->setEnabled(compactAuto);
    shadowStrengthSlider->setEnabled(shadowOn);
    shadowStrengthLabel->setEnabled(shadowOn);

    depthCueingAutoToggle->setEnabled(depthCueingOn);
    depthCueingStartEdit->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingStartLabel->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingEndEdit->setEnabled(depthCueingOn && !depthCueingAuto);
    depthCueingEndLabel->setEnabled(depthCueingOn && !depthCueingAuto);
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
// Method: QvisRenderingWindow::UpdateAAControls
//
// Purpose:
//   Updates enabled state of AA widgets based on mode.
//
// Arguments:
//   mode : The new AA mode.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateAAControls(int mode)
{
    // Set enabled state based on mode
    msaaSamplesLabel->blockSignals(true);
    msaaSamples->blockSignals(true);

    fxaaRCTLabel->blockSignals(true);
    fxaaRCT->blockSignals(true);
    fxaaRCTCustomLabel->blockSignals(true);
    fxaaRCTCustom->blockSignals(true);

    fxaaHCTLabel->blockSignals(true);
    fxaaHCT->blockSignals(true);
    fxaaHCTCustomLabel->blockSignals(true);
    fxaaHCTCustom->blockSignals(true);

    fxaaSBLLabel->blockSignals(true);
    fxaaSBL->blockSignals(true);
    fxaaSBLCustomLabel->blockSignals(true);
    fxaaSBLCustom->blockSignals(true);

    fxaaSCTLabel->blockSignals(true);
    fxaaSCT->blockSignals(true);
    fxaaSCTCustomLabel->blockSignals(true);
    fxaaSCTCustom->blockSignals(true);

    fxaaHQE->blockSignals(true);
    fxaaESI->blockSignals(true);

    msaaSamplesLabel->setEnabled(mode == 1);
    msaaSamples->setEnabled(mode == 1);

    fxaaRCTLabel->setEnabled(mode == 2);
    fxaaRCT->setEnabled(mode == 2);
    fxaaRCTCustomLabel->setEnabled(mode == 2 && fxaaRCT->currentIndex() == 4);
    fxaaRCTCustom->setEnabled(mode == 2 && fxaaRCT->currentIndex() == 4);

    fxaaHCTLabel->setEnabled(mode == 2);
    fxaaHCT->setEnabled(mode == 2);
    fxaaHCTCustomLabel->setEnabled(mode == 2 && fxaaHCT->currentIndex() == 3);
    fxaaHCTCustom->setEnabled(mode == 2 && fxaaHCT->currentIndex() == 3);

    fxaaSBLLabel->setEnabled(mode == 2);
    fxaaSBL->setEnabled(mode == 2);
    fxaaSBLCustomLabel->setEnabled(mode == 2 && fxaaSBL->currentIndex() == 4);
    fxaaSBLCustom->setEnabled(mode == 2 && fxaaSBL->currentIndex() == 4);

    fxaaSCTLabel->setEnabled(mode == 2);
    fxaaSCT->setEnabled(mode == 2);
    fxaaSCTCustomLabel->setEnabled(mode == 2 && fxaaSCT->currentIndex() == 5);
    fxaaSCTCustom->setEnabled(mode == 2 && fxaaSCT->currentIndex() == 5);

    fxaaHQE->setEnabled(mode == 2);
    fxaaESILabel->setEnabled(mode == 2);
    fxaaESI->setEnabled(mode == 2);

    msaaSamplesLabel->blockSignals(false);
    msaaSamples->blockSignals(false);

    fxaaRCTLabel->blockSignals(false);
    fxaaRCT->blockSignals(false);
    fxaaRCTCustomLabel->blockSignals(false);
    fxaaRCTCustom->blockSignals(false);

    fxaaHCTLabel->blockSignals(false);
    fxaaHCT->blockSignals(false);
    fxaaHCTCustomLabel->blockSignals(false);
    fxaaHCTCustom->blockSignals(false);

    fxaaSBLLabel->blockSignals(false);
    fxaaSBL->blockSignals(false);
    fxaaSBLCustomLabel->blockSignals(false);
    fxaaSBLCustom->blockSignals(false);

    fxaaSCTLabel->blockSignals(false);
    fxaaSCT->blockSignals(false);
    fxaaSCTCustomLabel->blockSignals(false);
    fxaaSCTCustom->blockSignals(false);
    fxaaHQE->blockSignals(false);
    fxaaESI->blockSignals(false);
}


// ****************************************************************************
// Method: QvisRenderingWindow::antialiasingChanged
//
// Purpose:
//   Slot function called when an antialiasing radio button is clicked.
//
// Arguments:
//   val : The new AA value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:52:07 PST 2002
//
// Modifications:
//   Kathleen Biagas, Monday July 28, 2025
//   Changed from antialiasToggled to antialiasingChanged.
//
//   Kathleen Biagas, Thu Aug 14, 2025
//   Added call to UpdateAAControls.
//
// ****************************************************************************

void
QvisRenderingWindow::antialiasingChanged(int val)
{
    renderAtts->SetAntialiasing(RenderingAttributes::AAMode(val));
    UpdateAAControls(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::msaaSamplesChanged
//
// Purpose:
//   Slot function called when an msaaSamples Spinbox value is changed.
//
// Arguments:
//   val : The new msaaSamples value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::msaaSamplesChanged(int val)
{
    renderAtts->SetMSAASamples(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaRCTChanged
//
// Purpose:
//   Slot function called when an fxaaRCTChanged ComboBox value is changed.
//
// Arguments:
//   index : The new fxaaRCT value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaRCTChanged(int index)
{
    renderAtts->GetFXAAOpt().SetRelativeContrastThreshold(FXAAOptions::RCT(index));
    UpdateAAControls(antialiasingMode->checkedId());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::fxaaRCTCustomChanged
//
// Purpose:
//   Slot function called when an fxaaRCTCustom value is changed.
//
// Arguments:
//   index : The new fxaaRCTCustom value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaRCTCustomChanged()
{
    renderAtts->GetFXAAOpt().SetCustomRCT(fxaaRCTCustom->text().toFloat());
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaHCTChanged
//
// Purpose:
//   Slot function called when an fxaaHCTChanged ComboBox value is changed.
//
// Arguments:
//   index : The new fxaaHCT value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaHCTChanged(int index)
{
    renderAtts->GetFXAAOpt().SetHardContrastThreshold(FXAAOptions::HCT(index));
    UpdateAAControls(antialiasingMode->checkedId());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::fxaaHCTCustomChanged
//
// Purpose:
//   Slot function called when an fxaaHCTCustom value is changed.
//
// Arguments:
//   index : The new fxaaHCTCustom value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaHCTCustomChanged()
{
    renderAtts->GetFXAAOpt().SetCustomHCT(fxaaHCTCustom->text().toFloat());
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaSBLChanged
//
// Purpose:
//   Slot function called when an fxaaSBLChanged ComboBox value is changed.
//
// Arguments:
//   index : The new fxaaSBL value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaSBLChanged(int index)
{
    renderAtts->GetFXAAOpt().SetSubpixelBlendLimit(FXAAOptions::SBL(index));
    UpdateAAControls(antialiasingMode->checkedId());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::fxaaSBLCustomChanged
//
// Purpose:
//   Slot function called when an fxaaSBLCustom value is changed.
//
// Arguments:
//   index : The new fxaaSBLCustom value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaSBLCustomChanged()
{
    renderAtts->GetFXAAOpt().SetCustomSBL(fxaaSBLCustom->text().toFloat());
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaSCTChanged
//
// Purpose:
//   Slot function called when an fxaaSCTChanged ComboBox value is changed.
//
// Arguments:
//   index : The new fxaaSCT value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaSCTChanged(int index)
{
    renderAtts->GetFXAAOpt().SetSubpixelContrastThreshold(FXAAOptions::SCT(index));
    UpdateAAControls(antialiasingMode->checkedId());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::fxaaSCTCustomChanged
//
// Purpose:
//   Slot function called when an fxaaSCTCustom value is changed.
//
// Arguments:
//   index : The new fxaaSCTCustom value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaSCTCustomChanged()
{
    renderAtts->GetFXAAOpt().SetCustomSCT(fxaaSCTCustom->text().toFloat());
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaHQEToggled
//
// Purpose:
//   Slot function called when fxaaHQE is toggled.
//
// Arguments:
//   val : The new fxaaHQE toggled state.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaHQEToggled(bool val)
{
    renderAtts->GetFXAAOpt().SetUseHighQualityEndpoints(val);
    Apply();
}


// ****************************************************************************
// Method: QvisRenderingWindow::fxaaESIChanged
//
// Purpose:
//   Slot function called when an fxaaESI value is changed.
//
// Arguments:
//   index : The new fxaaESI value.
//
// Programmer: Kathleen Biagas
// Creation:   August 14, 2025
//
// Modifications:
//
// ****************************************************************************

void
QvisRenderingWindow::fxaaESIChanged()
{
    renderAtts->GetFXAAOpt().SetEndpointSearchIterations(fxaaESI->text().toInt());
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
//   Kathleen Biagas, Tue Aug 19, 2025
//   Turn of MSAA mode if depth peeling being enabled.
//   Set enabled state of MSAA button dependent on depthPeeling state.
// 
// ****************************************************************************

void
QvisRenderingWindow::updateDepthPeeling()
{
    bool doUpdate = false;
    // has depthPeeling toggle changed
    if (depthPeeling->isChecked() && !renderAtts->GetDepthPeeling())
    {
        if (renderAtts->GetAntialiasing() == RenderingAttributes::MSAA)
        {
            Warning(tr("MSAA is incompatible with Depth Peeling. Antialiasing will be set to None. You may want to try FXAA."));
            renderAtts->SetAntialiasing(RenderingAttributes::None);
            doUpdate = true;
        }
    }
    // enable/disable MSAA button based on state of depthPeeling.
    antialiasingMode->button(1)->setEnabled(!depthPeeling->isChecked());
    renderAtts->SetDepthPeeling(depthPeeling->isChecked());
    renderAtts->SetOcclusionRatio(occlusionRatio->text().toDouble());
    renderAtts->SetNumberOfPeels(numberOfPeels->text().toInt());
    SetUpdate(doUpdate);
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
//  Modifications:
//    Kathleen Biagas, Monday Aug 11, 2025
//    Removed void* argument as it was unecessary.
//
// ****************************************************************************

void
QvisRenderingWindow::shadowStrengthChanged(int val)
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
//  Modifications:
//    Kathleen Biagas, Monday Aug 11, 2025
//    Removed void* argument as it was unecessary.
//
// ****************************************************************************

void
QvisRenderingWindow::specularStrengthChanged(int val)
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
//  Modifications:
//    Kathleen Biagas, Monday Aug 11, 2025
//    Removed void* argument as it was unecessary.
//
// ****************************************************************************

void
QvisRenderingWindow::specularPowerChanged(int val)
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
