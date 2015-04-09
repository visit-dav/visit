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
    
#include <visit-config.h>
#include <QvisVolumePlotWindow.h>
#ifdef HAVE_LIBSLIVR
#   include <QvisCMap2Widget.h>
#else
// We need to have a pointer to a QvisCMap2Widget object in the QvisVolumeWindow
// object all the time, even when we don't use it. That makes the object size
// consistent in the moc data and the real object. When we don't want the real
// widget, use this dummy class.
class QvisCMap2Widget
{
public:
    QvisCMap2Widget() : a(0) { }
    ~QvisCMap2Widget() { }
    int a;
};
#endif
#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QDesktopWidget>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTabWidget>

#include <math.h>
#include <stdio.h>
#ifdef _WIN32
// Windows #defines its own min/max, since we are using
// std::min, std::max, undefine the windows versions here.
#undef min
#undef max
#endif

#include <QvisOpacitySlider.h>
#include <QvisSpectrumBar.h>
#include <QvisColorSelectionWidget.h>
#include <QvisGaussianOpacityBar.h>
#include <QvisScribbleOpacityBar.h>
#include <QvisVariableButton.h>
#include <QvisColorTableButton.h>
#include <QNarrowLineEdit.h>
#include <TransferFunctionWidget.h>

#include <VolumeAttributes.h>
#include <VolumeRLEFunctions.h>
#include <ColorControlPoint.h>
#include <GaussianControlPoint.h>
#include <ViewerProxy.h>
#include <ImproperUseException.h>

#include <DataNode.h>
#include <InstallationFunctions.h>

#include <ColorTableAttributes.h>

#include <PlotInfoAttributes.h>

#define MAX_RENDERER_SAMPLE_VALUE 20.f
#ifdef HAVE_LIBSLIVR
#   define SLIVR_ONLY(stmt) stmt
#else
#   define SLIVR_ONLY(stmt) /* nothing */
#endif

// XPM data for pixmaps.
static const char * black_xpm[] = {
"40 20 1 1",
"+    c #000000",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++"};

static const char * ramp_xpm[] = {
"40 20 2 1",
".    c #000000",
"+    c #FFFFFF",
"......................................++",
"....................................++++",
"..................................++++++",
"................................++++++++",
"..............................++++++++++",
"............................++++++++++++",
"..........................++++++++++++++",
"........................++++++++++++++++",
"......................++++++++++++++++++",
"....................++++++++++++++++++++",
"..................++++++++++++++++++++++",
"................++++++++++++++++++++++++",
"..............++++++++++++++++++++++++++",
"............++++++++++++++++++++++++++++",
"..........++++++++++++++++++++++++++++++",
"........++++++++++++++++++++++++++++++++",
"......++++++++++++++++++++++++++++++++++",
"....++++++++++++++++++++++++++++++++++++",
"..++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++"};

static const char * inverse_ramp_xpm[] = {
"40 20 2 1",
".    c #000000",
"+    c #FFFFFF",
"........................................",
"++......................................",
"++++....................................",
"++++++..................................",
"++++++++................................",
"++++++++++..............................",
"++++++++++++............................",
"++++++++++++++..........................",
"++++++++++++++++........................",
"++++++++++++++++++......................",
"++++++++++++++++++++....................",
"++++++++++++++++++++++..................",
"++++++++++++++++++++++++................",
"++++++++++++++++++++++++++..............",
"++++++++++++++++++++++++++++............",
"++++++++++++++++++++++++++++++..........",
"++++++++++++++++++++++++++++++++........",
"++++++++++++++++++++++++++++++++++......",
"++++++++++++++++++++++++++++++++++++....",
"++++++++++++++++++++++++++++++++++++++.."};


static const char * tents_xpm[] = {
"40 20 2 1",
"+    c #000000",
".    c #FFFFFF",
"........................................",
"........................................",
".......++..............++..............+",
".......++..............++..............+",
"......++++............++++............++",
"......++++............++++............++",
".....++++++..........++++++..........+++",
".....++++++..........++++++..........+++",
"....++++++++........++++++++........++++",
"....++++++++........++++++++........++++",
"...++++++++++......++++++++++......+++++",
"...++++++++++......++++++++++......+++++",
"..++++++++++++....++++++++++++....++++++",
"..++++++++++++....++++++++++++....++++++",
".++++++++++++++..++++++++++++++..+++++++",
".++++++++++++++..++++++++++++++..+++++++",
".++++++++++++++..++++++++++++++..+++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++"};

static const char * white_xpm[] = {
"40 20 1 1",
"+    c #FFFFFF",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++++++++++"};

// ****************************************************************************
// Method: QvisVolumePlotWindow::QvisVolumePlotWindow
//
// Purpose: 
//   This is the constructor for the QvisVolumePlotWindow class.
//
// Arguments:
//   
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:58:08 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:50:54 PST 2002
//   Initialized parentless widgets.
//
//   Kathleen Bonnell, Thu Mar  3 11:01:22 PST 2005 
//   Initialized scaling.
//
//   Gunther Weber, Fri Apr  6 16:33:19 PDT 2007
//   Initialize showColorInAlphaWidget.
//
// ****************************************************************************

QvisVolumePlotWindow::QvisVolumePlotWindow(const int type,
    VolumeAttributes *volumeAtts_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(volumeAtts_, caption, shortName, notepad)
{
    plotType    = type;
    volumeAtts = volumeAtts_;

    colorCycle = 1;
    showColorsInAlphaWidget = true;

    // Initialize parentless widgets.
    modeButtonGroup = 0;
    colorSelect = 0;

    // Watch the plot info atts too.
    GetViewerState()->GetPlotInformation(plotType)->Attach(this);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::~QvisVolumePlotWindow
//
// Purpose: 
//   Destructor for the QvisVolumePlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:59:14 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:01:20 PST 2002
//   Added deletion of parentless widgets.
//
//   Kathleen Bonnell, Thu Mar  3 11:01:22 PST 2005 
//   Delete scaling.
//
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Made sure a button groups have parents, so we don't need to explicitly
//   delete.
//
// ****************************************************************************

QvisVolumePlotWindow::~QvisVolumePlotWindow()
{
    volumeAtts = 0;

    GetViewerState()->GetPlotInformation(plotType)->Detach(this);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 27 12:04:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 15:02:05 PDT 2008
//   Moved code into helper methods.
//
//   Brad Whitlock, Thu Dec 18 15:16:00 PST 2008
//   I moved the transfer function radio button creation to here and I 
//   reorganized the code so we can create 1D/2D transfer function pages.
//
// ****************************************************************************

void
QvisVolumePlotWindow::CreateWindowContents()
{
    // Figure out the max width that we want to allow for some simple
    // line edit widgets.
    int maxWidth = fontMetrics().width("1.0000000000");

    // Create a tab widget so we can put the transfer functions on their
    // own tabs.
    tfTabs = new QTabWidget(central);
    topLayout->addWidget(tfTabs, 100);

    tfRendererOptions = CreateRendererOptionsGroup(maxWidth);
    tfTabs->addTab(tfRendererOptions, tr("Renderer Options"));

    tfParent1D = Create1DTransferFunctionGroup(maxWidth);
    tfTabs->addTab(tfParent1D, tr("1D transfer function"));

#ifdef HAVE_LIBSLIVR 
    // Create the transfer function pages.
    tfParent2D = Create2DTransferFunctionGroup();
    tfTabs->addTab(tfParent2D, tr("2D transfer function"));
#endif

    // Create the color selection widget.
    colorSelect = new QvisColorSelectionWidget(0);
    connect(colorSelect, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedColor(const QColor &)));

    UpdateSamplingGroup();
}

// ****************************************************************************
// Method: QvisVolumePlot::CreateMatLightGroup
//
// Purpose: 
//   Creates the different Light Shading options for SLIVR
//
// Programmer: Pascal Grosset
// Creation:   Tue Apr 10
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::CreateMatLightGroup(QWidget *parent, QLayout *pLayout, int maxWidth)
{
    // Add the group box that will contain the options.
    lightMaterialPropGroup = new QGroupBox(parent);
    lightMaterialPropGroup->setTitle(tr("Lighting and Material Properties"));
    pLayout->addWidget(lightMaterialPropGroup);

    QVBoxLayout *lightMaterialPropLayout = new QVBoxLayout(lightMaterialPropGroup);

    // Create the lighting toggle
    lightingToggle = new QCheckBox(tr("Lighting"), lightMaterialPropGroup);
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));

    // Material properties
    materialProperties=new QWidget(lightMaterialPropGroup);
    QGridLayout *materialPropertiesLayout = new QGridLayout(materialProperties);

    matKa = new QDoubleSpinBox(materialProperties);
    matKa->setMinimum(0.0);
    matKa->setMaximum(1.0);
    matKa->setDecimals(2);
    matKa->setSingleStep(0.05);
    matKa->setValue(0.4);
    
    matKd = new QDoubleSpinBox(materialProperties);
    matKd->setMinimum(0.0);
    matKd->setMaximum(1.0);
    matKd->setDecimals(2);
    matKd->setSingleStep(0.05);
    matKd->setValue(0.75);
    
    matKs = new QDoubleSpinBox(materialProperties);
    matKs->setMinimum(0.0);
    matKs->setMaximum(1.0);
    matKs->setDecimals(2);
    matKs->setSingleStep(0.05);
    matKs->setValue(0.0);
    
    matN = new QDoubleSpinBox(materialProperties);
    matN->setMinimum(0.0);
    matN->setMaximum(100.0);
    matN->setDecimals(1);
    matN->setSingleStep(1);
    matN->setValue(15);
    
   
    Ka = new QLabel(tr("Ambient:"), materialProperties);
    Kd = new QLabel(tr("Diffuse:"), materialProperties);
    Ks = new QLabel(tr("Specular:"), materialProperties);
    specPow = new QLabel(tr("Shininess:"), materialProperties);
    Ka->setBuddy(matKa);
    Kd->setBuddy(matKd);
    Ks->setBuddy(matKs);
    specPow->setBuddy(matN);

    
    materialPropertiesLayout->addWidget(Ka,     0,  0,  1,1, Qt::AlignRight);
    materialPropertiesLayout->addWidget(matKa,  0,  1,  1,1, Qt::AlignLeft);

    materialPropertiesLayout->addWidget(Kd,     0,  3,  1,1, Qt::AlignRight);
    materialPropertiesLayout->addWidget(matKd,  0,  4,  1,1, Qt::AlignLeft);

    materialPropertiesLayout->addWidget(Ks,     0,  6,  1,1, Qt::AlignRight);
    materialPropertiesLayout->addWidget(matKs,  0,  7,  1,1, Qt::AlignLeft);

    materialPropertiesLayout->addWidget(specPow,0,  10, 1,1, Qt::AlignRight);
    materialPropertiesLayout->addWidget(matN,   0,  11, 1,1, Qt::AlignLeft);
    
    lightMaterialPropLayout->addWidget(lightingToggle);
    lightMaterialPropLayout->addWidget(materialProperties);

    connect(matKa, SIGNAL(valueChanged(double)), this, SLOT(setMaterialKa(double))); 
    connect(matKd, SIGNAL(valueChanged(double)), this, SLOT(setMaterialKd(double))); 
    connect(matKs, SIGNAL(valueChanged(double)), this, SLOT(setMaterialKs(double))); 
    connect(matN , SIGNAL(valueChanged(double)), this, SLOT(setMaterialN(double) ));
}

// ****************************************************************************
// Method: QvisVolumePlot::Create1DTransferFunctionGroup
//
// Purpose: 
//   Creates the widgets associated with the 1D transfer functions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 15:23:41 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisVolumePlotWindow::Create1DTransferFunctionGroup(int maxWidth)
{
    QWidget *parent = new QWidget(central);
    QVBoxLayout *pLayout = new QVBoxLayout(parent);
    pLayout->setMargin(5);

    CreateColorGroup(parent, pLayout, maxWidth);
    pLayout->addSpacing(10);
    CreateOpacityGroup(parent, pLayout, maxWidth);

    return parent;
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::CreateColorGroup
//
// Purpose: 
//   Creates the widgets in the color group.
//
// Note:       Adapted from CreateWindowContents when porting to Qt 4.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 14:58:34 PDT 2008
//
// Modifications:
//   Tom Fogal, Fri Sep 19 11:00:47 MDT 2008
//   Only connect 2D TF widgets when SLIVR is available.
//
//   Brad Whitlock, Thu Dec 18 15:17:38 PST 2008
//   I removed 2D transfer functions stuff since this method is for 1D
//   transfer functions. I also passed in a new parent for widgets. Finally,
//   I changed the widget layout.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of
//   set via freeform/gaussian editor.
//
//   Brad Whitlock, Fri Apr 27 15:58:26 PDT 2012
//   I added different smoothing methods.
//
// ****************************************************************************

void
QvisVolumePlotWindow::CreateColorGroup(QWidget *parent, QVBoxLayout *pLayout,
    int maxWidth)
{
    // Add the group box that will contain the color-related widgets.
    colorWidgetGroup = new QGroupBox(parent);
    colorWidgetGroup->setTitle(tr("Color"));
    pLayout->addWidget(colorWidgetGroup);
    pLayout->setStretchFactor(colorWidgetGroup,1000);
    QVBoxLayout *innerColorLayout = new QVBoxLayout(colorWidgetGroup);
    innerColorLayout->setMargin(5);
    innerColorLayout->setSpacing(0);

    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *seLayout = new QHBoxLayout(0);
    seLayout->setMargin(5);
    seLayout->setSpacing(5);
    innerColorLayout->addLayout(seLayout);

    seLayout->addWidget( new QLabel(tr("Color table"), central));

    colorTableButton = new QvisColorTableButton(colorWidgetGroup);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    seLayout->addWidget(colorTableButton);

    seLayout->addSpacing(5);
    seLayout->addStretch(10);

    addPointButton = new QPushButton(tr("+"), colorWidgetGroup);
    connect(addPointButton, SIGNAL(clicked()), this, SLOT(addControlPoint()));
    seLayout->addWidget(addPointButton);

    rmPointButton = new QPushButton(tr("-"), colorWidgetGroup);
    connect(rmPointButton, SIGNAL(clicked()),
            this, SLOT(removeControlPoint()));
    seLayout->addWidget(rmPointButton);

    alignPointButton = new QPushButton(tr("Align"), colorWidgetGroup);
    connect(alignPointButton, SIGNAL(clicked()),
            this, SLOT(alignControlPoints()));
    seLayout->addWidget(alignPointButton);

    seLayout->addSpacing(5);
    seLayout->addStretch(10);

    QLabel *smoothLabel = new QLabel(tr("Smoothing"), colorWidgetGroup);
    seLayout->addWidget(smoothLabel);
    smoothingMethod = new QComboBox(colorWidgetGroup);
    smoothingMethod->addItem(tr("None"));
    smoothingMethod->addItem(tr("Linear"));
    smoothingMethod->addItem(tr("Cubic Spline"));
    connect(smoothingMethod, SIGNAL(activated(int)),
            this, SLOT(smoothingMethodChanged(int)));
    seLayout->addWidget(smoothingMethod);

    equalCheckBox = new QCheckBox(tr("Equal"), colorWidgetGroup);
    connect(equalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(equalSpacingToggled(bool)));
    seLayout->addWidget(equalCheckBox);

    // Add the spectrum bar to the window.
    spectrumBar = new QvisSpectrumBar(colorWidgetGroup);
    connect(spectrumBar, SIGNAL(controlPointMoved(int,float)),
            this, SLOT(controlPointMoved(int,float)));
    connect(spectrumBar, SIGNAL(selectColor(int, const QPoint &)),
            this, SLOT(popupColorSelect(int, const QPoint &)));
    spectrumBar->resize(spectrumBar->width(), 60);
    innerColorLayout->addWidget(spectrumBar, 100);


    //
    // Create the scale radio buttons
    //
    QGroupBox * dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    innerColorLayout->addWidget(dataGroup);

    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    dataLayout->addWidget( new QLabel(tr("Scale"), central), 0, 0);
    
    // Create the radio buttons
    scalingButtons = new QButtonGroup(central);

    QRadioButton * rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    scalingButtons->addButton(rb, 0);
    dataLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Log"), central);
    scalingButtons->addButton(rb, 1);
    dataLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("Skew"), central);
    scalingButtons->addButton(rb, 2);
    dataLayout->addWidget(rb, 0, 3);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(scalingButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(scaleClicked(int)));

    // Create the skew factor line edit    
    skewLineEdit = new QLineEdit(central);
    dataLayout->addWidget(skewLineEdit, 0, 4);
    connect(skewLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSkewText()));

    //
    // Create the Limits stuff
    //
    QGroupBox * limitsGroup = new QGroupBox(central);
    dataLayout->addWidget(limitsGroup, 1, 0, 1, 5);

    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

    // ARS NOTE: the limits should be in here to match the pseducolor plot
    // gui.  Everything is in place on the GUI side it needs to be
    // implemented on teh plot side.

//     limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

//     limitsSelect = new QComboBox(central);
//     limitsSelect->addItem(tr("Use Original Data"));
//     limitsSelect->addItem(tr("Use Current Plot"));
//     connect(limitsSelect, SIGNAL(activated(int)),
//             this, SLOT(limitsSelectChanged(int))); 
//     limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    colorMinToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(colorMinToggle, 0, 0);
    connect(colorMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMinToggled(bool)));
    colorMin = new QLineEdit(central);
    limitsLayout->addWidget(colorMin, 0, 1);
    connect(colorMin, SIGNAL(returnPressed()),
            this, SLOT(colorMinProcessText())); 

    // Create the max toggle and line edit
    colorMaxToggle = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(colorMaxToggle, 0, 2);
    connect(colorMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMaxToggled(bool)));
    colorMax = new QLineEdit(central);
    limitsLayout->addWidget(colorMax, 0, 3);
    connect(colorMax, SIGNAL(returnPressed()),
            this, SLOT(colorMaxProcessText())); 
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::CreateOpacityGroup
//
// Purpose: 
//   Creates the widgets in the opacity group.
//
// Note:       Adapted from CreateWindowContents when porting to Qt 4.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 14:58:34 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Dec 18 15:17:38 PST 2008
//   I passed in a new parent for widgets. I also changed the layout of the
//   widgets to save a line.
//   
// ****************************************************************************

void
QvisVolumePlotWindow::CreateOpacityGroup(QWidget *parent, QVBoxLayout *pLayout,
    int maxWidth)
{
    //
    // Add the group box that will contain the opacity-related widgets.
    //
    opacityWidgetGroup = new QGroupBox(parent);
    opacityWidgetGroup->setTitle(tr("Opacity"));
    pLayout->addWidget(opacityWidgetGroup);
    pLayout->setStretchFactor(opacityWidgetGroup,1000);
    QVBoxLayout *innerOpacityLayout = new QVBoxLayout(opacityWidgetGroup);
    innerOpacityLayout->setMargin(5);
    innerOpacityLayout->setSpacing(0);

    // Create the buttons that control what mode the opacity widget it in.
    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *opLayout = new QHBoxLayout(0);
    opLayout->setMargin(5);
    opLayout->setSpacing(5);
    innerOpacityLayout->addLayout(opLayout);

    // Create the interaction mode label.
    QLabel *interactionModeLabel = new QLabel(tr("Interaction mode"),
        opacityWidgetGroup);
    opLayout->addWidget(interactionModeLabel);
    
    // Create the interaction mode button group.
    modeButtonGroup = new QButtonGroup(opacityWidgetGroup);
    connect(modeButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(interactionModeChanged(int)));

    QRadioButton *rb= new QRadioButton(tr("Freeform"), opacityWidgetGroup);
    modeButtonGroup->addButton(rb, 0);
    opLayout->addWidget(rb);
    rb->setChecked(true);

    rb = new QRadioButton(tr("Gaussian"), opacityWidgetGroup);
    modeButtonGroup->addButton(rb, 1);
    opLayout->addWidget(rb);

    rb = new QRadioButton(tr("From Color Table"), opacityWidgetGroup);
    modeButtonGroup->addButton(rb, 2);
    opLayout->addWidget(rb);

    opLayout->addSpacing(5);
    opLayout->addStretch(10);

    showColorsInAlphaWidgetToggle =
      new QCheckBox(tr("Show colors"), opacityWidgetGroup);
    showColorsInAlphaWidgetToggle->setChecked(showColorsInAlphaWidget);
    connect(showColorsInAlphaWidgetToggle, SIGNAL(toggled(bool)),
            this, SLOT(showColorsInAlphaWidgetToggled(bool)));
    opLayout->addWidget(showColorsInAlphaWidgetToggle);


    // Create the gaussian opacity editor widget.
    QVBoxLayout *barLayout = new QVBoxLayout(0);
    barLayout->setMargin(0);
    barLayout->setSpacing(0);
    innerOpacityLayout->addLayout(barLayout);
    innerOpacityLayout->setStretchFactor(barLayout, 100);

    alphaWidget = new QvisGaussianOpacityBar(opacityWidgetGroup);
    alphaWidget->setMinimumHeight(64);
    connect(alphaWidget, SIGNAL(mouseReleased()),
            this, SLOT(alphaValuesChanged()));
    barLayout->addWidget(alphaWidget, 100);

    // Create the scribble opacity editor widget.
    scribbleAlphaWidget = new QvisScribbleOpacityBar(opacityWidgetGroup);
    scribbleAlphaWidget->setMinimumHeight(64);
    scribbleAlphaWidget->hide();
    connect(scribbleAlphaWidget, SIGNAL(opacitiesChanged()),
            this, SLOT(alphaValuesChanged()));
    connect(scribbleAlphaWidget, SIGNAL(mouseReleased()),
            this, SLOT(alphaValuesChanged()));
    barLayout->addWidget(scribbleAlphaWidget, 100);

    // Create some style pixmaps
    QPixmap blackIcon(black_xpm);
    QPixmap rampIcon(ramp_xpm);
    QPixmap inverseRampIcon(inverse_ramp_xpm);
    QPixmap tentIcon(tents_xpm); 
    QPixmap whiteIcon(white_xpm);

    QHBoxLayout *abLayout = new QHBoxLayout(0);
    abLayout->setMargin(5);
    abLayout->setSpacing(5);
    innerOpacityLayout->addLayout(abLayout);

    zeroButton = new QPushButton(opacityWidgetGroup);
    zeroButton->setIcon(QIcon(blackIcon));
    connect(zeroButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTotallyZero()));
    abLayout->addWidget(zeroButton);

    rampButton = new QPushButton(opacityWidgetGroup);
    rampButton->setIcon(QIcon(rampIcon));
    connect(rampButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeLinearRamp()));
    abLayout->addWidget(rampButton);

    inverseRampButton = new QPushButton(opacityWidgetGroup);
    inverseRampButton->setIcon(QIcon(inverseRampIcon));
    connect(inverseRampButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeInverseLinearRamp()));
    abLayout->addWidget(inverseRampButton);

    tentButton = new QPushButton(opacityWidgetGroup);
    tentButton->setIcon(QIcon(tentIcon));
    connect(tentButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTent()));
    abLayout->addWidget(tentButton);

    oneButton = new QPushButton(opacityWidgetGroup);
    oneButton->setIcon(QIcon(whiteIcon));
    connect(oneButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTotallyOne()));
    abLayout->addWidget(oneButton);

    smoothButton = new QPushButton(tr("Smooth"), opacityWidgetGroup);
    connect(smoothButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(smoothCurve()));
    abLayout->addWidget(smoothButton);


    // Create the opacity attenuation widgets.
    attenuationSlider =
      new QvisOpacitySlider(0, 255, 10, 255, opacityWidgetGroup);
    attenuationSlider->setGradientColor(QColor(0,0,0));
    QLabel *attenuationLabel =
      new QLabel(tr("Attenuation"), opacityWidgetGroup);
    attenuationLabel->setBuddy(attenuationSlider);
    connect(attenuationSlider, SIGNAL(valueChanged(int)),
            this, SLOT(attenuationChanged(int)));
    abLayout->addWidget(attenuationLabel);
    abLayout->addWidget(attenuationSlider);


    //
    // Create the Opacity Variable stuff
    //
    QHBoxLayout *ovLayout = new QHBoxLayout(0);
    ovLayout->setMargin(5);
    ovLayout->setSpacing(10);
    innerOpacityLayout->addLayout(ovLayout);


    QGroupBox *opacityVariableGroup = new QGroupBox(central);
    opacityVariableGroup->setTitle(tr("Opacity variable"));
    ovLayout->addWidget(opacityVariableGroup);

    QGridLayout *opacityVariableLayout = new QGridLayout(opacityVariableGroup);
    opacityVariableLayout->setMargin(5);
    opacityVariableLayout->setSpacing(10);


    // Create the opacity variable
    opacityVariable = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars, opacityWidgetGroup);
    connect(opacityVariable, SIGNAL(activated(const QString &)),
            this, SLOT(opacityVariableChanged(const QString &)));
    QLabel *opacityVarLabel = new QLabel(tr("Variable"), opacityWidgetGroup);
    opacityVarLabel->setBuddy(opacityVariable);
    opacityVariableLayout->addWidget(opacityVarLabel, 0, 0);
    opacityVariableLayout->addWidget(opacityVariable, 0, 1);
    opacityVariableLayout->setColumnStretch(3, 10);

    // Create the opacity min widgets.
    opacityMinToggle = new QCheckBox(tr("Minimum"), opacityWidgetGroup);
    connect(opacityMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMinToggled(bool)));
    opacityVariableLayout->addWidget(opacityMinToggle, 0, 2);
    opacityMin = new QLineEdit(opacityWidgetGroup);
    opacityMin->setMaximumWidth(maxWidth);
    opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
    connect(opacityMin, SIGNAL(returnPressed()),
            this, SLOT(opacityMinProcessText()));
    opacityVariableLayout->addWidget(opacityMin, 0, 3);

    // Create the opacity max widgets.
    opacityMaxToggle = new QCheckBox(tr("Maximum"), opacityWidgetGroup);
    connect(opacityMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMaxToggled(bool)));
    opacityVariableLayout->addWidget(opacityMaxToggle, 0, 4);

    opacityMax = new QLineEdit(opacityWidgetGroup);
    opacityMax->setMaximumWidth(maxWidth);
    opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
    connect(opacityMax, SIGNAL(returnPressed()),
            this, SLOT(opacityMaxProcessText()));
    opacityVariableLayout->addWidget(opacityMax, 0, 5);


}

// ****************************************************************************
// Method: QvisVolumePlot::Create2DTransferFunctionGroup
//
// Purpose: 
//   Create the widgets that we need for 2D transfer functions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 15:20:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisVolumePlotWindow::Create2DTransferFunctionGroup()
{
    QWidget *parent = 0;

#ifdef HAVE_LIBSLIVR
    parent = new QWidget(central);
    QVBoxLayout *pLayout = new QVBoxLayout(parent);
    pLayout->setMargin(0);

    // Add the 2D transfer function widget (SLIVR)
    transferFunc2D = new QvisCMap2Widget(parent);
    connect(transferFunc2D, SIGNAL(widgetListChanged()),
            this, SLOT(updateTransferFunc2D()));
    connect(transferFunc2D, SIGNAL(widgetChanged(WidgetID)),
            this, SLOT(updateTransferFunc2D(WidgetID))); 
    pLayout->addWidget(transferFunc2D);
#endif

    return parent;
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::CreateRendererOptionsGroup
//
// Purpose: 
//   Creates the widgets in the renderer options group.
//
// Note:       Adapted from CreateWindowContents when porting to Qt 4.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 14:58:34 PDT 2008
//
// Modifications:
//   Jeremy Meredith, Mon Jan  4 17:12:16 EST 2010
//   Added ability to reduce amount of lighting for low-gradient-mag areas.
//   
//   Jeremy Meredith, Tue Jan  5 14:25:17 EST 2010
//   Added more settings for low-gradient-mag area lighting reduction: more
//   curve shape power, and an optional max-grad-mag-value clamp useful both
//   as an extra tweak and for making animations not have erratic lighting.
//
//   Hank Childs, Thu Oct 28 19:03:00 PDT 2010
//   Increase the maximum number of samples per ray.
//
//   Allen Harvey, Brad Whitlock, Tue Jan 31 16:32:54 PST 2012
//   Add support for no resampling.
//
// ****************************************************************************

void QvisVolumePlotWindow::CreateSamplingGroups(QWidget *parent, QLayout *pLayout)
{
    //resample group (shared by splatting, texture3d, tuvok, and slivr)
    {
        resampleGroup = new QGroupBox(parent);
        resampleGroup->setFlat(true);
        resampleGroup->setContentsMargins(0,0,0,0);
        QHBoxLayout *resampleLayout     = new QHBoxLayout(                                  resampleGroup);
        resampleTargetWidget            = new QWidget();
        QHBoxLayout *resampleTargetLayout= new QHBoxLayout(                                 resampleTargetWidget);
        resampleTargetLayout->setMargin(0);
        resampleTargetLabel             = new QLabel(tr("Number of samples"),               resampleGroup);
        resampleToggle                  = new QCheckBox(tr("Sample data onto regular grid"),resampleGroup);
        resampleTarget                  = new QSpinBox(                                     resampleGroup);
        resampleTarget->setMinimum(1000);
        resampleTarget->setMaximum(100000000);
        resampleTarget->setSingleStep(10000);
        resampleTargetLabel->setBuddy(resampleTarget);
        connect(resampleToggle, SIGNAL(toggled(bool)),      this, SLOT(resampleToggled(bool)));
        connect(resampleTarget, SIGNAL(valueChanged(int)),  this, SLOT(resampleTargetChanged(int)));
        resampleLayout->addWidget(resampleToggle);
        resampleLayout->addStretch(QSizePolicy::Maximum);
        resampleLayout->addWidget(resampleTargetWidget);
        resampleTargetLayout->addWidget(resampleTargetLabel,Qt::AlignRight);
        resampleTargetLayout->addWidget(resampleTarget,Qt::AlignLeft);
        resampleGroup->setVisible(false);
    }

    //splatting group
    {
        splattingGroup = new QGroupBox(parent);
        splattingGroup->setTitle(tr("Splatting Options"));
        splattingGroupLayout                = new QVBoxLayout(                                                      splattingGroup);
        splattingOptions                    = new QWidget(                                                          splattingGroup);
        QHBoxLayout        *splattingLayout = new QHBoxLayout(                                                      splattingOptions);
        QLabel             *compactVarLabel = new QLabel(tr("Compact support variable"),                            splattingOptions);
        QvisVariableButton *compactVariable = new QvisVariableButton(true, true, true, QvisVariableButton::Scalars, splattingOptions);
        compactVarLabel->setBuddy(compactVariable);
        connect(compactVariable, SIGNAL(activated(const QString &)), this, SLOT(compactVariableChanged(const QString &)));
        splattingLayout->addWidget(compactVarLabel);
        splattingLayout->addWidget(compactVariable);
        splattingLayout->addStretch(QSizePolicy::Maximum);
        splattingGroupLayout->addWidget(splattingOptions);
        pLayout->addWidget(splattingGroup);
    }

    //texture3d group
    {
        texture3dGroup = new QGroupBox(parent);
        texture3dGroup->setTitle(tr("3D Texturing Options"));
        texture3dGroupLayout            = new QVBoxLayout(                  texture3dGroup);
        texture3dOptions                = new QWidget(                      texture3dGroup);
        QHBoxLayout *texture3dLayout    = new QHBoxLayout(                  texture3dOptions);
        QLabel      *num3DSlicesLabel   = new QLabel(tr("Number of slices"),texture3dOptions);
        num3DSlices                     = new QSpinBox(                     texture3dOptions);
        num3DSlices->setMinimum(1);
        num3DSlices->setMaximum(1000);
        num3DSlices->setSingleStep(10);
        num3DSlicesLabel->setBuddy(num3DSlices);
        connect(num3DSlices, SIGNAL(valueChanged(int)), this, SLOT(num3DSlicesChanged(int)));
        texture3dLayout->addWidget(num3DSlicesLabel);
        texture3dLayout->addWidget(num3DSlices);
        texture3dLayout->addStretch(QSizePolicy::Maximum);
        texture3dGroupLayout->addWidget(texture3dOptions);
        pLayout->addWidget(texture3dGroup);
    }

    //raycasting group
    {
        raycastingGroup = new QGroupBox(parent);
        raycastingGroup->setTitle(tr("Ray Casting Options"));
        QGridLayout *raycastingLayout = new QGridLayout(raycastingGroup);
        // raycastingLayoutV->setSpacing(0);
        // raycastingLayoutV->setMargin(0);
        //6 x 2 layout: 
        // (0,0) Sampling method  (0,1) Rasterization   (0,2) Kernel Based       (0,4) Trilinear
        //                        (0,1) Samples per ray (0,2) sprSpinBox         (0,4) Sampling rate  (0,5) srSpinBox
        samplesPerRayWidget         = new QWidget(                          raycastingGroup);
        QHBoxLayout *sprLayout      = new QHBoxLayout(                      samplesPerRayWidget);
        samplesPerRayLabel          = new QLabel(tr("Samples per ray"),     samplesPerRayWidget);
        samplesPerRay               = new QSpinBox(                         samplesPerRayWidget);
        rendererSamplesWidget       = new QWidget(                          raycastingGroup);
        QHBoxLayout *rsLayout       = new QHBoxLayout(                      rendererSamplesWidget);
        rendererSamplesLabel        = new QLabel(tr("Sampling rate"),       rendererSamplesWidget);
        rendererSamples             = new QDoubleSpinBox(                   rendererSamplesWidget);
        samplingMethodWidget        = new QWidget(                          raycastingGroup);
        samplingMethodLabel         = new QLabel(tr("Sampling method"),     samplingMethodWidget);
        samplingButtonGroup         = new QButtonGroup(                     samplingMethodWidget);
        rasterizationButton         = new QRadioButton(tr("Rasterization"), samplingMethodWidget);
        kernelButton                = new QRadioButton(tr("Kernel Based"),  samplingMethodWidget);
        trilinearButton             = new QRadioButton(tr("Trilinear"),     samplingMethodWidget);
        samplingButtonGroup->addButton(rasterizationButton, 0);
        samplingButtonGroup->addButton(kernelButton, 1);
        samplingButtonGroup->addButton(trilinearButton, 2);
        samplesPerRay->setMinimum(1);
        samplesPerRay->setMaximum(25000);
        samplesPerRay->setSingleStep(50);
        samplesPerRayLabel->setBuddy(samplesPerRay);
        rendererSamples->setMinimum(1);
        rendererSamples->setMaximum(20);
        rendererSamples->setSingleStep(.1);
        rendererSamplesLabel->setBuddy(rendererSamples);
        connect(rendererSamples,     SIGNAL(valueChanged(double)),  this, SLOT(rendererSamplesChanged(double)));
        connect(samplesPerRay,       SIGNAL(valueChanged(int)),     this, SLOT(samplesPerRayChanged(int)));
        connect(samplingButtonGroup, SIGNAL(buttonClicked(int)),    this, SLOT(samplingTypeChanged(int)));
        rsLayout->addWidget(rendererSamplesLabel);
        rsLayout->addWidget(rendererSamples,Qt::AlignLeft);
        rsLayout->addStretch(QSizePolicy::Maximum);
        sprLayout->addWidget(samplesPerRayLabel);
        sprLayout->addWidget(samplesPerRay,Qt::AlignLeft);
        sprLayout->addStretch(QSizePolicy::Maximum);
        raycastingLayout->addWidget(samplingMethodLabel,0,0);
        raycastingLayout->addWidget(rasterizationButton,0,1);
        raycastingLayout->addWidget(kernelButton,0,2,1,2);
        raycastingLayout->addWidget(trilinearButton,0,4);
        raycastingLayout->addWidget(samplesPerRayWidget,1,1,1,2);
        raycastingLayout->addWidget(rendererSamplesWidget,1,4,1,2);
        pLayout->addWidget(raycastingGroup);
    }
    
    //slivr group
    {
        slivrGroup = new QGroupBox(parent);
        slivrGroup->setTitle(tr("SLIVR Options"));
        slivrGroupLayout             = new QVBoxLayout(                  slivrGroup);
        slivrOptions                 = new QWidget(                      slivrGroup);
        QHBoxLayout *slivrLayout     = new QHBoxLayout(                  slivrOptions);
        rendererSamplesSLIVRLabel    = new QLabel(tr("Sampling rate"),   slivrOptions);
        rendererSamplesSLIVR         = new QDoubleSpinBox(               slivrOptions);
        rendererSamplesSLIVR->setMinimum(1);
        rendererSamplesSLIVR->setMaximum(20);
        rendererSamplesSLIVR->setSingleStep(.1);
        rendererSamplesSLIVRLabel->setBuddy(rendererSamplesSLIVR);
        connect(rendererSamplesSLIVR, SIGNAL(valueChanged(double)), this, SLOT(rendererSamplesChanged(double)));
        slivrLayout->addWidget(rendererSamplesSLIVRLabel);
        slivrLayout->addWidget(rendererSamplesSLIVR);
        slivrLayout->addStretch(QSizePolicy::Maximum);
        slivrGroupLayout->addWidget(slivrOptions);
        pLayout->addWidget(slivrGroup);
    }

    //tuvok group
    {
        tuvokGroup = new QGroupBox(parent);
        tuvokGroup->setTitle(tr("Tuvok Options"));
        tuvokGroupLayout                = new QVBoxLayout(                  tuvokGroup);
        tuvokOptions                    = new QWidget(                      tuvokGroup);
        QHBoxLayout *tuvokLayout        = new QHBoxLayout(                  tuvokOptions);
        QLabel      *num3DSlicesLabel   = new QLabel(tr("Number of slices"),tuvokOptions);
        QSpinBox    *num3DSlices        = new QSpinBox(                     tuvokOptions);
        num3DSlices->setMinimum(1);
        num3DSlices->setMaximum(1000);
        num3DSlices->setSingleStep(10);
        num3DSlicesLabel->setBuddy(num3DSlices);
        connect(num3DSlices, SIGNAL(valueChanged(int)), this, SLOT(num3DSlicesChanged(int)));
        tuvokLayout->addWidget(num3DSlicesLabel);
        tuvokLayout->addWidget(num3DSlices);
        tuvokLayout->addStretch(QSizePolicy::Maximum);
        tuvokGroupLayout->addWidget(tuvokOptions);
        pLayout->addWidget(tuvokGroup);
    }
}

void QvisVolumePlotWindow::EnableSamplingMethods(bool enable)
{
    samplingMethodLabel->setEnabled(enable);
    rasterizationButton->setEnabled(enable);
    kernelButton->setEnabled(enable);
    trilinearButton->setEnabled(enable);
}

void QvisVolumePlotWindow::UpdateLowGradientGroup(bool enable)
{
    lowGradientGroup->setEnabled(enable);
    if (!enable) return;
    lowGradientLightingReductionLabel->setEnabled(enable);
    lowGradientLightingReductionCombo->setEnabled(enable);
    enable &= (volumeAtts->GetLowGradientLightingReduction() != VolumeAttributes::Off);
    lowGradientClampToggle->setEnabled(enable);
    enable &= volumeAtts->GetLowGradientLightingClampFlag();
    lowGradientClamp->setEnabled(enable);
}

void QvisVolumePlotWindow::EnableSplattingGroup()
{
    //resampleGroup is shared between several renderers' options
    splattingGroupLayout->addWidget(resampleGroup);
    resampleGroup->setVisible(true);
    resampleGroup->setEnabled(true);
    splattingGroup->setVisible(true);
    splattingOptions->setEnabled(!volumeAtts->GetResampleFlag());
}

void QvisVolumePlotWindow::EnableTexture3dGroup()
{
    //resampleGroup is shared between several renderers' options
    texture3dGroupLayout->addWidget(resampleGroup);
    resampleGroup->setVisible(true);
    resampleGroup->setEnabled(true);
    texture3dGroup->setVisible(true);
    texture3dOptions->setEnabled(true);
}

void QvisVolumePlotWindow::EnableTuvokGroup()
{
    //resampleGroup is shared between several renderers' options
    tuvokGroupLayout->addWidget(resampleGroup);
    resampleGroup->setVisible(true);
    resampleGroup->setEnabled(true);
    tuvokGroup->setVisible(true);
    tuvokOptions->setEnabled(true);
}

void QvisVolumePlotWindow::EnableSLIVRGroup()
{
    //resampleGroup is shared between several renderers' options
    slivrGroupLayout->addWidget(resampleGroup);
    resampleGroup->setVisible(true);
    resampleGroup->setEnabled(true);
    slivrGroup->setVisible(true);
    slivrOptions->setEnabled(!volumeAtts->GetResampleFlag());
}

void QvisVolumePlotWindow::UpdateSamplingGroup()
{
    //hide all groups
    resampleGroup->setVisible(false);
    splattingGroup->setVisible(false);
    texture3dGroup->setVisible(false);
    tuvokGroup->setVisible(false);
    raycastingGroup->setVisible(false);
    slivrGroup->setVisible(false);

    //disable 2d transfer function tab (only enabled for SLIVR)
    tfTabs->setTabEnabled(1, true);
    tfTabs->setTabEnabled(2, false);

    //lighting and material properties group, enabled for all but RayCastingIntegration
    lightMaterialPropGroup->setEnabled(true);
    lightingToggle->setEnabled(true);

    //disable material properties (only enabled with lighting for SLIVR, RayCasting w/ Trilinear Sampling)
    materialProperties->setEnabled(false);

    //enable/disable resampleTarget
    resampleTargetWidget->setEnabled(volumeAtts->GetResampleFlag());

    //smooth data
    smoothDataToggle->setEnabled(true);

    //color widget and opacity groups
    colorWidgetGroup->setEnabled(true);
    opacityWidgetGroup->setEnabled(true);

    //gradient methods
    centeredDiffButton->setEnabled(true);
    sobelButton->setEnabled(true);
 
    //add sampling options to layout based on current settings
    VolumeAttributes::Renderer renderer_type=volumeAtts->GetRendererType();
    switch (renderer_type)
    {
    case VolumeAttributes::Splatting:
        EnableSplattingGroup();
        UpdateLowGradientGroup(true);
        break;

    case VolumeAttributes::Texture3D:
        EnableTexture3dGroup();
        UpdateLowGradientGroup(true);
        break;

    case VolumeAttributes::Tuvok:
        EnableTuvokGroup();
        UpdateLowGradientGroup(false);
        break;

    case VolumeAttributes::RayCasting:
        resampleGroup->setEnabled(false);
        raycastingGroup->setVisible(true);
        UpdateLowGradientGroup(true);
        materialProperties->setEnabled(volumeAtts->GetSampling()==VolumeAttributes::Trilinear && volumeAtts->GetLightingFlag());
        EnableSamplingMethods(true);
        samplesPerRayWidget->setEnabled(volumeAtts->GetSampling()!=VolumeAttributes::Trilinear);
        rendererSamplesWidget->setEnabled(volumeAtts->GetSampling()==VolumeAttributes::Trilinear);
        break;

    case VolumeAttributes::RayCastingIntegration:
        resampleGroup->setEnabled(false);
        raycastingGroup->setVisible(true);
        UpdateLowGradientGroup(false);
        lightMaterialPropGroup->setEnabled(false);
        colorWidgetGroup->setEnabled(false);
        opacityWidgetGroup->setEnabled(false);
        centeredDiffButton->setEnabled(false);
        sobelButton->setEnabled(false);
        EnableSamplingMethods(false);
        samplesPerRayWidget->setEnabled(true);
        rendererSamplesWidget->setEnabled(false);
        rendererSamples->setEnabled(true);
        rendererSamplesLabel->setEnabled(true);
        break;

    case VolumeAttributes::SLIVR:
        EnableSLIVRGroup();
        UpdateLowGradientGroup(false);
        materialProperties->setEnabled(volumeAtts->GetLightingFlag());
        // Enable selected transfer function widget.
        tfTabs->setTabEnabled(1, volumeAtts->GetTransferFunctionDim()==1);
        tfTabs->setTabEnabled(2, volumeAtts->GetTransferFunctionDim()==2);
        break;

    case VolumeAttributes::RayCastingSLIVR:
        EnableSLIVRGroup();
        resampleGroup->setEnabled(false);
        raycastingGroup->setVisible(false);
        UpdateLowGradientGroup(false);
        materialProperties->setEnabled(volumeAtts->GetRendererType()==VolumeAttributes::RayCastingSLIVR && volumeAtts->GetLightingFlag());
        EnableSamplingMethods(true);
        samplesPerRayWidget->setEnabled(volumeAtts->GetRendererType()!=VolumeAttributes::RayCastingSLIVR);
        rendererSamplesWidget->setEnabled(volumeAtts->GetRendererType()==VolumeAttributes::RayCastingSLIVR);
        break;

    default:
        EXCEPTION1(ImproperUseException, "No such renderer type.");
    }        
}

QWidget *
QvisVolumePlotWindow::CreateRendererOptionsGroup(int maxWidth)
{
    QWidget *parent = new QWidget(central);
    parent->setStyleSheet(QString::fromUtf8("QSpinBox:disabled, QDoubleSpinBox:disabled, QLineEdit:disabled, QLabel:disabled"
                                            "{ color: gray }"
                                            "QGroupBox"
                                            "{ font-size: 10px; color: black; }"
                                            "QGroupBox:disabled"
                                            "{ font-size: 10px; color: gray; }"
                                            "QGroupBox::title"
                                            "{ subcontrol-origin: padding; subcontrol-position: top left; padding: 2 5px; }"
                                            ));

    // Create the rendering method radio buttons.
    this->rendererOptionsLayout = new QVBoxLayout(parent);
    rendererOptionsLayout->setMargin(5);
    rendererOptionsLayout->setSpacing(5);

    //
    // Create the render stuff
    //
    QGroupBox *renderGroup = new QGroupBox(parent);
    renderGroup->setTitle(tr("Rendering Method"));
    rendererOptionsLayout->addWidget(renderGroup);

    QHBoxLayout *renderLayout = new QHBoxLayout(renderGroup);
    rendererTypesComboBox = new QComboBox(renderGroup);
    rendererTypesComboBox->addItem(tr("Splatting"));
    rendererTypesComboBox->addItem(tr("3D Texturing"));
    rendererTypesComboBox->addItem(tr("Ray casting: compositing"));
    rendererTypesComboBox->addItem(tr("Ray casting: integration (grey scale)"));
#ifdef USE_TUVOK
    rendererTypesComboBox->addItem(tr("Tuvok"));
#endif
#ifdef HAVE_LIBSLIVR
    rendererTypesComboBox->addItem(tr("SLIVR"));
    rendererTypesComboBox->addItem(tr("Ray casting: SLIVR"));
#endif
    connect(rendererTypesComboBox, SIGNAL(activated(int)),
            this, SLOT(rendererTypeChanged(int)));

    renderLayout->addWidget(rendererTypesComboBox);
    renderLayout->addStretch(QSizePolicy::Maximum);

#ifdef HAVE_LIBSLIVR
    // Create the transfer function dimension button
    tfWidget = new QWidget(renderGroup);
    renderLayout->addWidget(tfWidget);

    QHBoxLayout *tfLayout = new QHBoxLayout(tfWidget);
    tfLayout->setMargin(0);

    QLabel *tfLabel=new QLabel(tr("Transfer function"), tfWidget);
    tfLabel->setBuddy(tfWidget);
    tfLayout->addWidget(tfLabel);
    transferFunctionGroup = new QButtonGroup(tfWidget);
    connect(transferFunctionGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(transferDimChanged(int)));
    oneDimButton = new QRadioButton(tr("1D"), tfWidget);
    transferFunctionGroup->addButton(oneDimButton, 0);
    tfLayout->addWidget(oneDimButton);
    twoDimButton = new QRadioButton(tr("2D"), tfWidget);
    transferFunctionGroup->addButton(twoDimButton, 1);
    tfLayout->addWidget(twoDimButton);
#endif

    //
    //Create sampling groups
    //
    CreateSamplingGroups(parent,rendererOptionsLayout);

    //
    // Create the methods stuff
    //
    QGroupBox *methodsGroup = new QGroupBox(parent);
    methodsGroup->setTitle(tr("Methods"));
    rendererOptionsLayout->addWidget(methodsGroup);

    // Create the gradient method radio buttons.
    QHBoxLayout *methodsLayout = new QHBoxLayout(methodsGroup);
    methodsLayout->addWidget(new QLabel(tr("Gradient method")));
    gradientButtonGroup = new QButtonGroup(methodsGroup);
    connect(gradientButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(gradientTypeChanged(int)));
    centeredDiffButton = new QRadioButton(tr("Centered differences"),methodsGroup);
    gradientButtonGroup->addButton(centeredDiffButton, 0);
    methodsLayout->addWidget(centeredDiffButton);
    sobelButton = new QRadioButton(tr("Sobel"),methodsGroup);
    gradientButtonGroup->addButton(sobelButton, 1);
    methodsLayout->addWidget(sobelButton);

    //
    // Create Material Properties group
    //
    CreateMatLightGroup(parent, rendererOptionsLayout, maxWidth);

    //
    // Create the low gradient lighting reduction.
    //
    lowGradientGroup = new QGroupBox(parent);
    lowGradientGroup->setTitle(tr("Low gradient lighting"));
    rendererOptionsLayout->addWidget(lowGradientGroup);

    QHBoxLayout *lowGradientLayout = new QHBoxLayout(lowGradientGroup);
 
    // Create the low gradient lighting reduction combo.
    lowGradientLightingReductionLabel =
        new QLabel(tr("Reduction factor"),lowGradientGroup);
    lowGradientLayout->addWidget(lowGradientLightingReductionLabel);

    lowGradientLightingReductionCombo = new QComboBox(lowGradientGroup);
    lowGradientLightingReductionCombo->addItem("Off");
    lowGradientLightingReductionCombo->addItem("Lowest");
    lowGradientLightingReductionCombo->addItem("Lower");
    lowGradientLightingReductionCombo->addItem("Low");
    lowGradientLightingReductionCombo->addItem("Medium");
    lowGradientLightingReductionCombo->addItem("High");
    lowGradientLightingReductionCombo->addItem("Higher");
    lowGradientLightingReductionCombo->addItem("Highest");
    connect(lowGradientLightingReductionCombo, SIGNAL(activated(int)),
            this, SLOT(lowGradientLightingReductionChanged(int)));
    lowGradientLayout->addWidget(lowGradientLightingReductionCombo);
    lowGradientLayout->addStretch(QSizePolicy::Maximum);

    lowGradientClampToggle =
        new QCheckBox(tr("Reduction maximum value"),lowGradientGroup);
    connect(lowGradientClampToggle, SIGNAL(toggled(bool)),
            this, SLOT(lowGradientClampToggled(bool)));
    lowGradientLayout->addWidget(lowGradientClampToggle);

    lowGradientClamp = new QLineEdit(lowGradientGroup);
    lowGradientClamp->setEnabled(volumeAtts->GetUseColorVarMax());
    connect(lowGradientClamp, SIGNAL(returnPressed()),
            this, SLOT(lowGradientClampProcessText()));
    lowGradientLayout->addWidget(lowGradientClamp,Qt::AlignLeft);
    lowGradientLayout->addStretch(QSizePolicy::Maximum);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(parent);
    miscGroup->setTitle(tr("Misc"));
    rendererOptionsLayout->addWidget(miscGroup);

    QHBoxLayout *miscLayout = new QHBoxLayout(miscGroup);
    //miscLayout->setMargin(5);
    //miscLayout->setSpacing(10);

    // Create the smooth data toggle.
    smoothDataToggle = new QCheckBox(tr("Smooth Data"), miscGroup);
    connect(smoothDataToggle, SIGNAL(toggled(bool)),
            this, SLOT(smoothDataToggled(bool)));
    miscLayout->addWidget(smoothDataToggle);

    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Show Legend"), miscGroup);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle);

    return parent;
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::UpdateHistogram
//
// Purpose: 
//   This method takes the histogram data and updates the widgets that can
//   display it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 16:42:34 PST 2008
//
// Modifications:
//
//   Tom Fogal, Tue Mar 10 17:22:11 MST 2009
//   Don't try to pass empty histograms to the widgets.
//   
//   Hank Childs, Fri May 21 12:05:03 PDT 2010
//   Add argument for need2D.
//
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateHistogram(bool need2D)
{
    PlotInfoAttributes *info = GetViewerState()->GetPlotInformation(plotType);
    bool invalid = true;

    MapNode *hNode = info->GetData().GetEntry("VolumeHistogram");
    if(hNode != 0)
    {
        MapNode &vhist = *hNode;
        int hist_size = vhist["histogram_size"].AsInt();

        const floatVector &hist = vhist["histogram_1d"].AsFloatVector();
        if(!hist.empty())
        {
            alphaWidget->setHistogramTexture(&hist[0], hist_size);
            scribbleAlphaWidget->setHistogramTexture(&hist[0], hist_size);
        }

#ifdef HAVE_LIBSLIVR
        const unsignedCharVector &hist2 = vhist["histogram_2d"].AsUnsignedCharVector();
        if(!hist2.empty())
        {
            unsignedCharVector decompressed;
            VolumeRLEDecompress(hist2, decompressed);
            transferFunc2D->setHistogramTexture(&decompressed[0], hist_size);
        }
#endif
        // Ugh.  Sorry this looks so gross.
        if(!hist.empty() SLIVR_ONLY(&& (!need2D || !hist2.empty())))
        {
            invalid = false;
        }
    }

    if(invalid)
    {
        // We don't have histogram data, get rid of histograms in the
        // widgets.
#ifdef HAVE_LIBSLIVR
        transferFunc2D->setHistogramTexture(0, 0);
#endif
        alphaWidget->setHistogramTexture(0, 0);
        scribbleAlphaWidget->setHistogramTexture(0, 0);
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 27 12:07:55 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 09:22:57 PDT 2001
//   Modified to account for changes in the state object.
//
//   Jeremy Meredith, Tue Nov 13 11:42:53 PST 2001
//   Added resample target and opacity var.  Made freeformOpacity index 8.
//
//   Hank Childs, Wed Dec 12 09:30:52 PST 2001
//   Added support for min/max of color/opacity vars.
//
//   Hank Childs, Fri Feb  8 19:25:19 PST 2002
//   Added support for smooth data and samples per ray.
//
//   Brad Whitlock, Thu Feb 14 09:31:51 PDT 2002
//   Fixed a bug in which the min/max toggles were not set properly.
//
//   Jeremy Meredith, Thu Oct  2 13:11:02 PDT 2003
//   Added settings for the renderer type, the gradient method, and
//   the number of 3D textured slices.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Hank Childs, Mon Nov 22 09:28:33 PST 2004
//   Account for ray trace toggle.
//
//   Brad Whitlock, Wed Dec 15 09:26:05 PDT 2004
//   I moved raytrace support into the rendering mode. I also made it be
//   a combo box.
//
//   Kathleen Bonnell, Thu Mar  3 11:01:22 PST 2005 
//   Added support for scaling and skew factor.
//
//   Hank Childs, Sun Jan  8 08:19:39 PST 2006
//   Add support for kernel based sampling.
//
//   Hank Childs, Mon Sep 11 10:34:32 PDT 2006
//   I added the RayCastingIntegration option.
//
//   Gunther H. Weber, Fri Apr  6 11:44:12 PDT 2007
//   Disable lighting toggle for RayCasting
//
//   Brad Whitlock, Thu Jan 10 14:35:10 PST 2008
//   Added support for SLIVR, use the case ids too.
//
//   Brad Whitlock, Fri Jul 18 13:59:58 PDT 2008
//   Qt 4.
//
//   Hank Childs, Sun Aug 31 10:53:59 PDT 2008
//   Allow lighting toggle to remain active with ray casting.
//
//   Josh Stratton, Mon Sep  8 10:12:56 MDT 2008
//   Added toggle for 1D/2D SLIVR transfer functions
//
//   Tom Fogal, Fri Sep 19 11:05:44 MDT 2008
//   Use SLIVR_ONLY to conditionally enable 2D transfer function widgets.
//
//   Josh Stratton, Mon Dec 15 13:01:07 MST 2008
//   Added support for Tuvok rendering mode.
//
//   Brad Whitlock, Tue Dec 16 11:52:01 PST 2008
//   I added code to update the histogram. I also renamed a field.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of
//   set via freeform/gaussian editor.
//
//   Jeremy Meredith, Mon Jan  4 17:12:16 EST 2010
//   Added ability to reduce amount of lighting for low-gradient-mag areas.
//   Applies only to the software volume renderer for the moment.
//
//   Jeremy Meredith, Tue Jan  5 14:25:17 EST 2010
//   Added more settings for low-gradient-mag area lighting reduction: more
//   curve shape power, and an optional max-grad-mag-value clamp useful both
//   as an extra tweak and for making animations not have erratic lighting.
//
//   Jeremy Meredith, Tue Jan  5 15:51:26 EST 2010
//   The low-gradient-mag lighting reduction now also applies to the 
//   3D texturing and splatting volume renderers.
//
//   Hank Childs, Fri May 21 12:05:03 PDT 2010
//   Add argument to UpdateHistogram.
//
//   Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//   Make resampling optional.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateWindow(bool doAll)
{
    double *mat;

    // If the plot info atts changed then update the histogram.
    if(doAll || SelectedSubject() == GetViewerState()->GetPlotInformation(plotType))
    {
        bool need2D = false;
        if (volumeAtts->GetRendererType() == VolumeAttributes::SLIVR)
            need2D = true;
        UpdateHistogram(need2D);
        if(!doAll)
            return;
    }

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < volumeAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!volumeAtts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case VolumeAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(volumeAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case VolumeAttributes::ID_resampleFlag:        
            resampleToggle->blockSignals(true);
            resampleToggle->setChecked(volumeAtts->GetResampleFlag());
            resampleToggle->blockSignals(false);
            resampleTarget->setEnabled(volumeAtts->GetResampleFlag());
            resampleTargetLabel->setEnabled(volumeAtts->GetResampleFlag());
            break;
        case VolumeAttributes::ID_lightingFlag:
            lightingToggle->blockSignals(true);
            lightingToggle->setChecked(volumeAtts->GetLightingFlag());
            lightingToggle->blockSignals(false);
            break;
        case VolumeAttributes::ID_lowGradientLightingReduction:
            lowGradientLightingReductionCombo->blockSignals(true);
            lowGradientLightingReductionCombo->setCurrentIndex(
                           (int)volumeAtts->GetLowGradientLightingReduction());
            lowGradientLightingReductionCombo->blockSignals(false);
            break;
        case VolumeAttributes::ID_lowGradientLightingClampFlag:
            lowGradientClampToggle->blockSignals(true);
            lowGradientClampToggle->setChecked(
                           volumeAtts->GetLowGradientLightingClampFlag());
            lowGradientClampToggle->blockSignals(false);
            break;
        case VolumeAttributes::ID_lowGradientLightingClampValue:
            lowGradientClamp->blockSignals(true);
            lowGradientClamp->setText(DoubleToQString(volumeAtts->GetLowGradientLightingClampValue()));
            lowGradientClamp->blockSignals(false);
            break;
        case VolumeAttributes::ID_colorControlPoints:
            UpdateColorControlPoints();
            UpdateFreeform();
            break;
        case VolumeAttributes::ID_opacityAttenuation:
            attenuationSlider->blockSignals(true);
            attenuationSlider->setValue(int(volumeAtts->GetOpacityAttenuation() * 255));
            attenuationSlider->blockSignals(false);
            break;
        case VolumeAttributes::ID_opacityMode:
            modeButtonGroup->blockSignals(true);
            modeButtonGroup->button(int(volumeAtts->GetOpacityMode()))->setChecked(true);
            modeButtonGroup->blockSignals(false);

            if(volumeAtts->GetOpacityMode() == VolumeAttributes::FreeformMode)
            {
                alphaWidget->hide();
                scribbleAlphaWidget->show();
                scribbleAlphaWidget->setEnabled(true);
            }
            else if(volumeAtts->GetOpacityMode() == VolumeAttributes::GaussianMode)
            {
                scribbleAlphaWidget->hide();
                alphaWidget->show();
            }
            else // from color table
            {
                alphaWidget->hide();
                scribbleAlphaWidget->show();
                scribbleAlphaWidget->setEnabled(false);
            }
            break;
        case VolumeAttributes::ID_opacityControlPoints:
            UpdateGaussianControlPoints();
            break;
        case VolumeAttributes::ID_resampleTarget:
            resampleTarget->blockSignals(true);
            resampleTarget->setValue(volumeAtts->GetResampleTarget());
            resampleTarget->blockSignals(false);
            break;
        case VolumeAttributes::ID_opacityVariable:
            opacityVariable->setText(volumeAtts->GetOpacityVariable().c_str());
            break;
        case VolumeAttributes::ID_freeformOpacity:
            UpdateFreeform();
            break;
//         case VolumeAttributes::ID_limitsMode:
//             limitsSelect->blockSignals(true);
//             limitsSelect->setCurrentIndex(volumeAtts->GetLimitsMode());
//             limitsSelect->blockSignals(false);
//             break;
        case VolumeAttributes::ID_useColorVarMin:
            colorMinToggle->blockSignals(true);
            colorMinToggle->setChecked(volumeAtts->GetUseColorVarMin());
            colorMinToggle->blockSignals(false);
            colorMin->setEnabled(volumeAtts->GetUseColorVarMin());
            break;
        case VolumeAttributes::ID_colorVarMin:
            colorMin->setText(FloatToQString(volumeAtts->GetColorVarMin()));
            break;
        case VolumeAttributes::ID_useColorVarMax:
            colorMaxToggle->blockSignals(true);
            colorMaxToggle->setChecked(volumeAtts->GetUseColorVarMax());
            colorMaxToggle->blockSignals(false);
            colorMax->setEnabled(volumeAtts->GetUseColorVarMax());
            break;
        case VolumeAttributes::ID_colorVarMax:
            colorMax->setText(FloatToQString(volumeAtts->GetColorVarMax()));
            break;
        case VolumeAttributes::ID_useOpacityVarMin:
            opacityMinToggle->blockSignals(true);
            opacityMinToggle->setChecked(volumeAtts->GetUseOpacityVarMin());
            opacityMinToggle->blockSignals(false);
            opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
            break;
        case VolumeAttributes::ID_opacityVarMin:
            opacityMin->setText(FloatToQString(volumeAtts->GetOpacityVarMin()));
            break;
        case VolumeAttributes::ID_useOpacityVarMax:
            opacityMaxToggle->blockSignals(true);
            opacityMaxToggle->setChecked(volumeAtts->GetUseOpacityVarMax());
            opacityMaxToggle->blockSignals(false);
            opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
            break;
        case VolumeAttributes::ID_opacityVarMax:
            opacityMax->setText(FloatToQString(volumeAtts->GetOpacityVarMax()));
            break;
        case VolumeAttributes::ID_smoothData:
            smoothDataToggle->blockSignals(true);
            smoothDataToggle->setChecked(volumeAtts->GetSmoothData());
            smoothDataToggle->blockSignals(false);
        case VolumeAttributes::ID_samplesPerRay:
            samplesPerRay->blockSignals(true);
            samplesPerRay->setValue(volumeAtts->GetSamplesPerRay());
            samplesPerRay->blockSignals(false);
            break;
        case VolumeAttributes::ID_rendererType:
            rendererTypesComboBox->blockSignals(true);
            if (volumeAtts->GetRendererType() == VolumeAttributes::Splatting)
            {
                rendererTypesComboBox->setCurrentIndex(0);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::Texture3D)
            {
                rendererTypesComboBox->setCurrentIndex(1);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::RayCasting)
            {
                rendererTypesComboBox->setCurrentIndex(2);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::RayCastingIntegration)
            {
                rendererTypesComboBox->setCurrentIndex(3);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::Tuvok)
            {
                int idx=std::max(1,rendererTypesComboBox->findText("Tuvok"));
                rendererTypesComboBox->setCurrentIndex(idx);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::SLIVR)
            {
                int idx=std::max(1,rendererTypesComboBox->findText("SLIVR"));
                rendererTypesComboBox->setCurrentIndex(idx);
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::RayCastingSLIVR)
            {
                int idx=std::max(1,rendererTypesComboBox->findText("Ray casting: SLIVR"));
                rendererTypesComboBox->setCurrentIndex(idx);
            }


            // Just for now, disable the opacity variable if we are using the
            // SLIVR renderer -- until I figure out how to do color and opacity
            // using separate variables.
            opacityVariable->setEnabled(volumeAtts->GetRendererType() != VolumeAttributes::SLIVR);

#ifdef HAVE_LIBSLIVR
            // Disable the 1D/2D transfer function buttons if the renderer is not SLIVR.
            tfWidget->setEnabled(volumeAtts->GetRendererType() == VolumeAttributes::SLIVR);
            // oneDimButton->setEnabled(volumeAtts->GetRendererType() == VolumeAttributes::SLIVR);
            // twoDimButton->setEnabled(volumeAtts->GetRendererType() == VolumeAttributes::SLIVR);
#endif
            rendererTypesComboBox->blockSignals(false);
            break;
        case VolumeAttributes::ID_gradientType:
            gradientButtonGroup->blockSignals(true);
            if (volumeAtts->GetGradientType() == VolumeAttributes::CenteredDifferences)
                gradientButtonGroup->button(0)->setChecked(true);
            else
                gradientButtonGroup->button(1)->setChecked(true);
            gradientButtonGroup->blockSignals(false);
            break;
        case VolumeAttributes::ID_num3DSlices:
            num3DSlices->blockSignals(true);
            num3DSlices->setValue(volumeAtts->GetNum3DSlices());
            num3DSlices->blockSignals(false);
            break;
        case VolumeAttributes::ID_scaling:
            scalingButtons->blockSignals(true);
            scalingButtons->button(volumeAtts->GetScaling())->setChecked(true);
            scalingButtons->blockSignals(false);

            skewLineEdit->setEnabled( volumeAtts->GetScaling() ==
                                      VolumeAttributes::Skew);
            break;
        case VolumeAttributes::ID_skewFactor:
            skewLineEdit->setText(DoubleToQString(volumeAtts->GetSkewFactor()));
            break;
        case VolumeAttributes::ID_sampling:
            samplingButtonGroup->blockSignals(true);
            if (volumeAtts->GetSampling() == VolumeAttributes::Rasterization)
                samplingButtonGroup->button(0)->setChecked(true);
            else if (volumeAtts->GetSampling() == VolumeAttributes::KernelBased)
                samplingButtonGroup->button(1)->setChecked(true);
            else
                samplingButtonGroup->button(2)->setChecked(true);
            samplingButtonGroup->blockSignals(false);
            break;
        case VolumeAttributes::ID_rendererSamples:
            rendererSamples->blockSignals(true);
            rendererSamples->setValue(volumeAtts->GetRendererSamples());
            rendererSamples->blockSignals(false);
            rendererSamplesSLIVR->blockSignals(true);
            rendererSamplesSLIVR->setValue(volumeAtts->GetRendererSamples());
            rendererSamplesSLIVR->blockSignals(false);
            break;
        case VolumeAttributes::ID_transferFunction2DWidgets:
#ifdef HAVE_LIBSLIVR
            Update2DTransferFunction();
#endif
            break;
        case VolumeAttributes::ID_transferFunctionDim:
#ifdef HAVE_LIBSLIVR
            transferFunctionGroup->blockSignals(true);
            if(volumeAtts->GetTransferFunctionDim() == 1 ||
               volumeAtts->GetTransferFunctionDim() == 2)
                transferFunctionGroup->button(volumeAtts->GetTransferFunctionDim()-1)->setChecked(true);
            transferFunctionGroup->blockSignals(false);
#endif
            break;

        case VolumeAttributes::ID_materialProperties:
            matKa->blockSignals(true);
            matKd->blockSignals(true);
            matKs->blockSignals(true);
            matN->blockSignals(true);

            mat = volumeAtts->GetMaterialProperties();
            matKa->setValue(mat[0]);
            matKd->setValue(mat[1]);
            matKs->setValue(mat[2]);
            matN->setValue(mat[3]);

            matKa->blockSignals(false);
            matKd->blockSignals(false);
            matKs->blockSignals(false);
            matN->blockSignals(false);
            break;
        }
    }

    // Update the opacity min, max toggle buttons.
    bool notDefaultVar = (volumeAtts->GetOpacityVariable() != "default");
    opacityMinToggle->setEnabled(notDefaultVar);
    opacityMaxToggle->setEnabled(notDefaultVar);

    //Update the sampling group
    UpdateSamplingGroup();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::UpdateColorControlPoints
//
// Purpose: 
//   This method is called when the color control points must be updated.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 27 23:35:25 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 09:21:28 PDT 2001
//   Moved the code to set the equalSpacing and smoothing check boxes to here.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of
//   set via freeform/gaussian editor.  Make sure to pass the color
//   table's alphas to the spectrum bar.
//
//   Brad Whitlock, Fri Apr 27 16:01:54 PDT 2012
//   Add more smoothing types.
//
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateColorControlPoints()
{
    const ColorControlPointList &cpts = volumeAtts->GetColorControlPoints();
    int   i;

    // Set the smoothing checkbox's checked state.
    smoothingMethod->blockSignals(true);
    smoothingMethod->setCurrentIndex((int)volumeAtts->GetSmoothing());
    smoothingMethod->blockSignals(false);

    // Set the equal checkbox's checked state.
    equalCheckBox->blockSignals(true);
    equalCheckBox->setChecked(volumeAtts->GetEqualSpacingFlag());
    equalCheckBox->blockSignals(false);

    spectrumBar->setSuppressUpdates(true);
    spectrumBar->blockSignals(true);

    // Set the spectrum bar's equal and smoothing flags.
    switch(volumeAtts->GetSmoothing())
    {
    case ColorControlPointList::None:
        spectrumBar->setSmoothing(QvisSpectrumBar::None);
        break;
    default:
    case ColorControlPointList::Linear:
        spectrumBar->setSmoothing(QvisSpectrumBar::Linear);
        break;
    case ColorControlPointList::CubicSpline:
        spectrumBar->setSmoothing(QvisSpectrumBar::CubicSpline);
        break;
    }
    spectrumBar->setEqualSpacing(volumeAtts->GetEqualSpacingFlag());

    if(spectrumBar->numControlPoints() == cpts.GetNumControlPoints())
    {
        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2],
                        (int)cpts[i].GetColors()[3]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }
    }
    else if(spectrumBar->numControlPoints() < cpts.GetNumControlPoints())
    {
        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2],
                        (int)cpts[i].GetColors()[3]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }

        // We need to add control points.
        for(i = spectrumBar->numControlPoints(); i < cpts.GetNumControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2],
                        (int)cpts[i].GetColors()[3]);
            spectrumBar->addControlPoint(ctmp, cpts[i].GetPosition());
        }
    }
    else
    {
        // We need to remove control points.
        for(i = spectrumBar->numControlPoints()-1; i >= cpts.GetNumControlPoints(); --i)
            spectrumBar->removeControlPoint();

        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2],
                        (int)cpts[i].GetColors()[3]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }
    }

    spectrumBar->blockSignals(false);
    spectrumBar->setSuppressUpdates(false);
    spectrumBar->update();

    if (showColorsInAlphaWidget)
    {
        alphaWidget->setBackgroundColorControlPoints(&cpts);
        scribbleAlphaWidget->setBackgroundColorControlPoints(&cpts);
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::UpdateGaussianControlPoints
//
// Purpose: 
//   This method is called when we must update the opacity control points.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 11:36:25 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:27:25 PST 2002
//   Fixed a memory leak.
//
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateGaussianControlPoints()
{
    alphaWidget->blockSignals(true);
    // Create an array that can be fed to the widget in one shot.
    int n=volumeAtts->GetOpacityControlPoints().GetNumControlPoints();
    if(n > 0)
    {
        float *gcpts = new float[n * 5];
        float *fptr = gcpts;

        GaussianControlPointList &cpts = volumeAtts->GetOpacityControlPoints();
        for(int i = 0; i < n; ++i)
        {
            *fptr++ = cpts[i].GetX();
            *fptr++ = cpts[i].GetHeight();
            *fptr++ = cpts[i].GetWidth();
            *fptr++ = cpts[i].GetXBias();
            *fptr++ = cpts[i].GetYBias();
        }

        // Set all of the gaussians into the widget.
        alphaWidget->setAllGaussians(n, gcpts);

        // Delete the gcpts array.
        delete [] gcpts;
    }
    else
    {
        // Set no gaussians into the widget.
        alphaWidget->setAllGaussians(0, NULL);
    }
    alphaWidget->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::UpdateFreeform
//
// Purpose: 
//   Sets the freeform opacity widget's values from the state object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 6 13:29:48 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 10:30:50 PDT 2001
//   Modified to use the new version of the VolumeAttributes.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of
//   set via freeform/gaussian editor.
//
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateFreeform()
{
    float f[256];

    if (volumeAtts->GetOpacityMode() == VolumeAttributes::ColorTableMode)
    {
        unsigned char rgb[256*3];
        unsigned char alpha[256];
        volumeAtts->GetColorControlPoints().GetColors(rgb, 256, alpha);

        // Convert the state object's opacities into floats that can be used by
        // the alpha widget.
        for(int i = 0; i < 256; ++i)
            f[i] = float(alpha[i]) / 255.;
    }
    else
    {
        const unsigned char *opacity = volumeAtts->GetFreeformOpacity();

        // Convert the state object's opacities into floats that can be used by
        // the alpha widget.
        for(int i = 0; i < 256; ++i)
            f[i] = float(opacity[i]) / 255.;
    }

    // Set the alphas into the scribble widget.
    scribbleAlphaWidget->blockSignals(true);
    scribbleAlphaWidget->setRawOpacities(256, f);
    scribbleAlphaWidget->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::CopyGaussianOpacitiesToFreeForm
//
// Purpose: 
//   Copies the gaussian opacities to the freeform opacities.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 7 10:54:31 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::CopyGaussianOpacitiesToFreeForm()
{
    float *rawOpacities = alphaWidget->getRawOpacities(256);
    unsigned char freeform[256];
    for(int i = 0; i < 256; ++i)
        freeform[i] = (unsigned char)(rawOpacities[i] * 255.);
    volumeAtts->SetFreeformOpacity(freeform);
    delete [] rawOpacities;
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::Update2DTransferFunction
//
// Purpose: 
//   This method updates the 2D transfer function widget with the new
//   widget values.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 12 14:00:05 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::Update2DTransferFunction()
{
#ifdef HAVE_LIBSLIVR
    // Get the name of the active widget, if there is one.
    QString selectedName = transferFunc2D->getName(transferFunc2D->getSelectedWidget());

    // Disconnect the signals.
    disconnect(transferFunc2D, SIGNAL(widgetListChanged()),
               this, SLOT(updateTransferFunc2D()));
    disconnect(transferFunc2D, SIGNAL(widgetChanged(WidgetID)),
               this, SLOT(updateTransferFunc2D(WidgetID))); 

    // Clear out the old list of widgets
    transferFunc2D->clear();

    // Add new widgets based on the ones from the atts.
    int i;
    for(i = 0; i < volumeAtts->GetNumTransferFunction2DWidgets(); ++i)
    {
        const TransferFunctionWidget &w = volumeAtts->GetTransferFunction2DWidgets(i);
        QString wName(w.GetName().c_str());
        WidgetID id = QvisCMap2Display::WIDGET_NOT_FOUND;

        if(w.GetType() == TransferFunctionWidget::Rectangle)
        {
            id = transferFunc2D->addRectangleWidget(wName,
                w.GetPosition()[0],
                w.GetPosition()[1],
                w.GetPosition()[2],
                w.GetPosition()[3],
                w.GetPosition()[4]);
        }
        else if(w.GetType() == TransferFunctionWidget::Triangle)
        {
            id = transferFunc2D->addTriangleWidget(wName,
                w.GetPosition()[0],
                w.GetPosition()[1],
                w.GetPosition()[2],
                w.GetPosition()[3],
                w.GetPosition()[4]);
        }
        else if(w.GetType() == TransferFunctionWidget::Paraboloid)
        {
            id = transferFunc2D->addParaboloidWidget(wName,
                w.GetPosition()[0],
                w.GetPosition()[1],
                w.GetPosition()[2],
                w.GetPosition()[3],
                w.GetPosition()[4],
                w.GetPosition()[5],
                w.GetPosition()[6],
                w.GetPosition()[7]);
        }
        else if(w.GetType() == TransferFunctionWidget::Ellipsoid)
        {
            id = transferFunc2D->addEllipsoidWidget(wName,
                w.GetPosition()[0],
                w.GetPosition()[1],
                w.GetPosition()[2],
                w.GetPosition()[3],
                w.GetPosition()[4]);
        }

        int r = (int)(w.GetBaseColor()[0] * 255.f);
        int g = (int)(w.GetBaseColor()[1] * 255.f);
        int b = (int)(w.GetBaseColor()[2] * 255.f);
        transferFunc2D->setColor(id, QColor(r,g,b));
        transferFunc2D->setAlpha(id, w.GetBaseColor()[3]);
    }

    // Reconnect the signals.
    connect(transferFunc2D, SIGNAL(widgetListChanged()),
            this, SLOT(updateTransferFunc2D()));
    connect(transferFunc2D, SIGNAL(widgetChanged(WidgetID)),
            this, SLOT(updateTransferFunc2D(WidgetID))); 

    // See if we can reactivate the previously selected widget
    bool selected = false;
    for(int i = 0; i < transferFunc2D->numWidgets(); ++i)
    {
         WidgetID id = transferFunc2D->getID(i);
         if(transferFunc2D->getName(id) == selectedName)
         {
             transferFunc2D->selectWidget(id);
             selected = true;
             break;
         }
    }
    if(!selected && transferFunc2D->numWidgets() > 0)
        transferFunc2D->selectWidget(transferFunc2D->getID(0));
#endif
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values from certain widgets and stores the values in the
//   state object.
//
// Arguments:
//   which_widget : A number indicating which widget that we should get the
//                  values from.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 27 18:42:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 10:14:34 PDT 2001
//   Modified code to account for changes to the VolumeAttributes.
//
//    Jeremy Meredith, Tue Nov 13 11:43:59 PST 2001
//    Added resample target and opacity var.
//
//   Hank Childs, Tue Dec 11 16:58:54 PST 2001
//   Added color/opacity min/max.
//
//   Hank Childs, Fri Feb  8 19:03:49 PST 2002
//   Added samples per ray.
//
//   Brad Whitlock, Thu Feb 14 12:46:53 PDT 2002
//   Fixed a memory leak and moved a rule about the opacity variable to here.
//
//   Jeremy Meredith, Thu Oct  2 13:11:15 PDT 2003
//   Added settings for the renderer type, the gradient method, and
//   the number of 3D textured slices.
//
//   Brad Whitlock, Thu Dec 9 17:37:31 PST 2004
//   I removed the code to get the opacity variable since it's in a different
//   kind of widget.
//
//   Kathleen Bonnell, Thu Mar  3 11:01:22 PST 2005 
//   Added support for skew factor.
//   
//   Dave Pugmire, Thu Nov  8 15:03:32 EST 2007
//   Check the "okay" flag before setting the skew edit.
//
//   Brad Whitlock, Fri Jan 11 16:23:00 PST 2008
//   Added support for renderer samples, changed to use ID labels.
//
//   Brad Whitlock, Thu Dec 18 13:51:43 PST 2008
//   I added code to update the 1D opacity widgets when we are showing
//   colors in their backgrounds.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of
//   set via freeform/gaussian editor.
//
//   Jeremy Meredith, Tue Jan  5 14:25:17 EST 2010
//   Added more settings for low-gradient-mag area lighting reduction: more
//   curve shape power, and an optional max-grad-mag-value clamp useful both
//   as an extra tweak and for making animations not have erratic lighting.
//
//   Brad Whitlock, Fri Apr 27 16:03:53 PDT 2012
//   Add more smoothing types.
//
// ****************************************************************************

void
QvisVolumePlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;
    int i;

    // Get the rgb colors from the spectrumbar and put them into the state
    // object's rgbaColors array.
    if(which_widget == 0 || doAll)
    {
        // Store the widget's control points.
        ColorControlPointList cpts;
        cpts.ClearControlPoints();
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor c(spectrumBar->controlPointColor(i));
            float  pos = spectrumBar->controlPointPosition(i);

            ColorControlPoint pt;
            unsigned char ptColors[4];
            ptColors[0] = (unsigned char)c.red();
            ptColors[1] = (unsigned char)c.green();
            ptColors[2] = (unsigned char)c.blue();
            ptColors[3] = (unsigned char)c.alpha();;
            pt.SetColors(ptColors);
            pt.SetPosition(pos);
            cpts.AddControlPoints(pt);
        }
        cpts.SetEqualSpacingFlag(spectrumBar->equalSpacing());
        switch(spectrumBar->smoothing())
        {
        case QvisSpectrumBar::None:
            cpts.SetSmoothing(ColorControlPointList::None);
            break;
        default:
        case QvisSpectrumBar::Linear:
            cpts.SetSmoothing(ColorControlPointList::Linear);
            break;
        case QvisSpectrumBar::CubicSpline:
            cpts.SetSmoothing(ColorControlPointList::CubicSpline);
            break;
        }
        volumeAtts->SetColorControlPoints(cpts);

        if (showColorsInAlphaWidget)
        {
            alphaWidget->update();
            scribbleAlphaWidget->update();
        }
    }

    // Get the alpha values from the opacity bar and put them into
    // the state object's rgbaColors array.
    if(which_widget == 1 || doAll)
    {
        // Store the opacity
        if(volumeAtts->GetOpacityMode() == VolumeAttributes::GaussianMode)
        {
            // Store the gaussian control points.
            GaussianControlPointList cpts;
            for(i = 0; i < alphaWidget->getNumberOfGaussians(); ++i)
            {
                float x, h, w, bx, by;
                alphaWidget->getGaussian(i, &x, &h, &w, &bx, &by);

                GaussianControlPoint gcpt;
                gcpt.SetX(x);
                gcpt.SetHeight(h);
                gcpt.SetWidth(w);
                gcpt.SetXBias(bx);
                gcpt.SetYBias(by);
                cpts.AddControlPoints(gcpt);
            }
            volumeAtts->SetOpacityControlPoints(cpts);
        }
        else if (volumeAtts->GetOpacityMode() == VolumeAttributes::FreeformMode)
        {
            // Store the freeform opacities.
            float *alphas = scribbleAlphaWidget->getRawOpacities(256);
            unsigned char a[256];
            for(i = 0; i < 256; ++i)
                a[i] = (unsigned char)(alphas[i] * 255.);
            volumeAtts->SetFreeformOpacity(a);
            delete [] alphas;
        }
        else
        {
            // Nothing to do....
        }
    }

    // Get the value of the resample target
    if(which_widget == VolumeAttributes::ID_resampleTarget || doAll)
    {
        volumeAtts->SetResampleTarget(resampleTarget->value());
    }
    
    // Get the value of the minimum for the color variable.
    if(which_widget == VolumeAttributes::ID_colorVarMin || doAll)
    {
        float val;
        if(LineEditGetFloat(colorMin, val))
            volumeAtts->SetColorVarMin(val);
        else
        {
            msg = tr("The minimum value for the color variable was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetColorVarMin());
            Message(msg);
        }
    }

    // Get the value of the maximum for the color variable.
    if(which_widget == VolumeAttributes::ID_colorVarMax || doAll)
    {
        float val;
        if(LineEditGetFloat(colorMax, val))
            volumeAtts->SetColorVarMax(val);
        else
        {
            msg = tr("The maximum value for the color variable was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetColorVarMax());
            Message(msg);
        }
    }

    // Get the value of the minimum for the opacity variable.
    if(which_widget == VolumeAttributes::ID_opacityVarMin || doAll)
    {
        float val;
        if(LineEditGetFloat(opacityMin, val))
            volumeAtts->SetOpacityVarMin(val);
        else
        {
            msg = tr("The minimum value for the opacity variable was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetOpacityVarMin());
            Message(msg);
        }
    }

    // Get the value of the maximum for the opacity variable.
    if(which_widget == VolumeAttributes::ID_opacityVarMax || doAll)
    {
        float val;
        if(LineEditGetFloat(opacityMax, val))
            volumeAtts->SetOpacityVarMax(val);
        else
        {
            msg = tr("The maximum value for the opacity var was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetOpacityVarMax());
            Message(msg);
        }
    }

    // Get the number of samples per ray.
    if(which_widget == VolumeAttributes::ID_samplesPerRay || doAll)
    {
        volumeAtts->SetSamplesPerRay(samplesPerRay->value());
    }

    // Get the number of slices for 3D texturing.
    if(which_widget == VolumeAttributes::ID_num3DSlices || doAll)
    {
        volumeAtts->SetNum3DSlices(num3DSlices->value());
    }

    // Do the skew factor value
    if(which_widget == VolumeAttributes::ID_skewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(skewLineEdit, val))
            volumeAtts->SetSkewFactor(val);
        else
        {
            msg = tr("The skew factor was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetSkewFactor());
            Message(msg);
            volumeAtts->SetSkewFactor(volumeAtts->GetSkewFactor());
        }
    }

    // Get the value of the renderer samples
    if(which_widget == VolumeAttributes::ID_rendererSamples || doAll)
    {
        volumeAtts->SetRendererSamples(rendererSamples->value());
    }

    if(which_widget == VolumeAttributes::ID_lowGradientLightingClampValue
       || doAll)
    {
        double val;
        if(LineEditGetDouble(lowGradientClamp, val) && val>0)
            volumeAtts->SetLowGradientLightingClampValue(val);
        else
        {
            msg = tr("The value for the low gradient lighting max clamp "
                     "was invalid. Resetting to the last good value of %1.").
                  arg(volumeAtts->GetLowGradientLightingClampValue());
            Message(msg);
        }
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 14:00:18 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:38:26 PST 2004
//   Changed to work with a variable button.
//
//   Brad Whitlock, Wed Apr 23 12:22:03 PDT 2008
//   Added tr().
//
// ****************************************************************************

void
QvisVolumePlotWindow::Apply(bool ignore)
{
    QString temp = opacityVariable->text();
    if (temp == "default")
    {
        if (volumeAtts->GetUseColorVarMin() || volumeAtts->GetUseColorVarMax())
        {
            if (!volumeAtts->GetUseOpacityVarMin() &&
                !volumeAtts->GetUseOpacityVarMax())
            {
                Message(tr("The range set for indexing colors will also be "
                           "used when indexing opacities"));
            }
        }
    }
                
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        volumeAtts->Notify();

        // Tell the viewer to set the volume plot attributes.
        GetViewerMethods()->SetPlotOptions(plotType);

    }
    else
        volumeAtts->Notify();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::ProcessOldVersions
//
// Purpose: 
//   Massage the data node before we use it to SetFromNode.
//
// Arguments:
//   parentNode : The node that contains the window's node.
//   configVersion : The version of the config file.
//
// Returns:    
//
// Note:       We're using this method to remove the width and height values
//             from old config files so the windows don't get resized to the
//             old tall values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 27 16:31:43 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::ProcessOldVersions(DataNode *parentNode, const char *configVersion)
{
    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode != 0)
    {
        if(VersionGreaterThan("2.1.0", configVersion))
        {
            // If configVersion < 2.1.0 then remove the width and height since the
            // window has changed a lot.
            winNode->RemoveNode("width", true);
            winNode->RemoveNode("height", true);
        }
    }
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisVolumePlotWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::makeDefault()
{
    // Tell the viewer to set the default contour plot attributes.
    GetCurrentValues(-1);
    volumeAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::reset()
{
    // Tell the viewer to reset the contour plot attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::addControlPoint
//
// Purpose: 
//   This is a Qt slot function that adds a new color control point to the
//   spectrumbar widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 14:02:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::addControlPoint()
{
    // Table of default control point colors.
    static int colors[] = {
        255, 0, 0,
        255, 255, 0,
        0, 255, 0,
        0, 255, 255,
        0, 0, 255,
        255, 0, 255,
        0, 0, 0,
        255, 255, 255};

    // Add the color control point to the spectrum bar.
    spectrumBar->addControlPoint(QColor(colors[colorCycle*3],
        colors[colorCycle*3+1], colors[colorCycle*3+2]));

    // Move on to the next color.
    colorCycle = (colorCycle < 7) ? (colorCycle + 1) : 0;

    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::removeControlPoint
//
// Purpose: 
//   This is a Qt slot function that removes the control point with the
//   highest rank from the spectrum bar.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 10:03:42 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::removeControlPoint()
{
    // Remove the highest ranked control point.
    spectrumBar->removeControlPoint();
    
     // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();   
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::alignControlPoints
//
// Purpose: 
//   This is a Qt slot function that tells the spectrum bar to align its
//   control points.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 10:04:17 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::alignControlPoints()
{
    // Align the control points.
    spectrumBar->alignControlPoints();

    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::controlPointMoved
//
// Purpose: 
//   This is a Qt slot function that is called when the spectrum bar's control
//   points are moved.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 11:39:32 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::controlPointMoved(int, float)
{
    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::popupColorSelect
//
// Purpose: 
//   This is a Qt slot function that is called when a color control point in
//   the spectrum bar widget wants to have a new color assigned to it.
//
// Arguments:
//   index : The index of the color control point that wants changed.
//   p     : The clicked point.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 14:06:35 PST 2001
//
// Modifications:
//   Hank Childs, Wed Dec 12 10:05:48 PST 2001
//   Removed unused variable buttonMiddle.
//
//   Brad Whitlock, Fri Jul 18 15:39:07 PDT 2008
//   Added argument.
//
// ****************************************************************************

void
QvisVolumePlotWindow::popupColorSelect(int index, const QPoint &p)
{
    // Set the popup's initial color.
    colorSelect->blockSignals(true);
    colorSelect->setSelectedColor(spectrumBar->controlPointColor(index));
    colorSelect->blockSignals(false);

    // Figure out a good place to popup the menu.
    int menuW = colorSelect->sizeHint().width();
    int menuH = colorSelect->sizeHint().height();
    int menuX = p.x();
    int menuY = p.y() - (menuH >> 1);

    // Fix the X dimension.
    if(menuX < 0)
        menuX = 0;
    else if(menuX + menuW > QApplication::desktop()->width())
        menuX -= (menuW + 5);

    // Fix the Y dimension.
    if(menuY < 0)
        menuY = 0;
    else if(menuY + menuH > QApplication::desktop()->height())
        menuY -= ((menuY + menuH) - QApplication::desktop()->height());

    // Show the popup menu.         
    colorSelect->move(menuX, menuY);
    colorSelect->show();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::selectedColor
//
// Purpose: 
//   This is a Qt slot function that is called by the color popup menu when
//   a new color has been selected.
//
// Arguments:
//   color : The new color that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 14:07:31 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::selectedColor(const QColor &color)
{
    // Hide the popup menu.
    colorSelect->hide();

    if(color.isValid())
    {
        // Set the color of the active color control point.
        spectrumBar->setControlPointColor(spectrumBar->activeControlPoint(),
                                          color);

        // Get the current values for the spectrum bar.
        GetCurrentValues(0);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::interactionModeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the interaction mode changes.
//
// Arguments:
//   index : The index of the button that was clicked. 0 means freeform and 1
//           means gaussian.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:35:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 16:12:32 PST 2002
//   Added code to update the freeform widget so GetCurrentValues does not
//   undo the copy that we just did when auto update is on.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Added support for getting alphas from color table instead of set
//   via freeform/gaussian editor.  When we enter this mode, make sure
//   to copy the values from the color table to the freeform display.
//
// ****************************************************************************

void
QvisVolumePlotWindow::interactionModeChanged(int index)
{
    VolumeAttributes::OpacityModes oldmode = volumeAtts->GetOpacityMode();
    VolumeAttributes::OpacityModes newmode =
        (VolumeAttributes::OpacityModes)index;

    volumeAtts->SetOpacityMode(newmode);
    if(oldmode == VolumeAttributes::GaussianMode &&
       newmode == VolumeAttributes::FreeformMode)
    {
        CopyGaussianOpacitiesToFreeForm();
        UpdateFreeform();
    }
    else if (newmode == VolumeAttributes::ColorTableMode)
    {
        UpdateFreeform();
    }
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::showColorsInAlphaWidgetToggled
//
// Purpose: 
//   This is a Qt slot function that toggles whether the color map is shown
//   as background of the opacity widget.
//
// Arguments:
//   show: Flag indicating, whether colors should be shown
//
// Programmer: Gunther H. Weber
// Creation:   Thu Apr  5 16:34:57 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Dec 18 12:00:54 PST 2008
//   Changed some method names.
//
// ****************************************************************************

void
QvisVolumePlotWindow::showColorsInAlphaWidgetToggled(bool show)
{
    showColorsInAlphaWidget = show;
    if (show)
    {
        const ColorControlPointList &cpts = volumeAtts->GetColorControlPoints();
        alphaWidget->setBackgroundColorControlPoints(&cpts);
        scribbleAlphaWidget->setBackgroundColorControlPoints(&cpts);
    }
    else
    {
        alphaWidget->setBackgroundColorControlPoints(0);
        scribbleAlphaWidget->setBackgroundColorControlPoints(0);
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::attenuationChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   opacity attenuation.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:36:38 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 09:52:42 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::attenuationChanged(int opacity)
{
    // Store the new attenuation factor in the state object.
    volumeAtts->SetOpacityAttenuation((float)opacity / 255.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::smoothDataToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the "smooth data" toggle is
//    clicked.
//
// Programmer: Hank Childs
// Creation:   February 8, 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::smoothDataToggled(bool)
{
    volumeAtts->SetSmoothData(!volumeAtts->GetSmoothData());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the legend toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:37:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 09:55:10 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::legendToggled(bool)
{
    volumeAtts->SetLegendFlag(!volumeAtts->GetLegendFlag());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::resampleToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the resample toggle is clicked.
//
// Programmer: Allen Harvey
// Creation:   Sun July 31 20:00:OO  2011
//
// ****************************************************************************

void
QvisVolumePlotWindow::resampleToggled(bool val)
{
    volumeAtts->SetResampleFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::lightingToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the lighting toggle is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:37:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 09:55:30 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::lightingToggled(bool)
{
    volumeAtts->SetLightingFlag(!volumeAtts->GetLightingFlag());
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::gradientMagAffectsLightingToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the toogle for
//   whether or not the gradient magnitude affects lighting is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   January  4, 2010
//
// Modifications:
//   Jeremy Meredith, Tue Jan  5 14:25:17 EST 2010
//   Added more settings for low-gradient-mag area lighting reduction: more
//   curve shape power, and an optional max-grad-mag-value clamp useful both
//   as an extra tweak and for making animations not have erratic lighting.
//
//   Jeremy Meredith, Tue Jan  5 15:51:26 EST 2010
//   The low-gradient-mag lighting reduction now also applies to the 
//   3D texturing and splatting volume renderers.
//
// ****************************************************************************

void
QvisVolumePlotWindow::lowGradientLightingReductionChanged(int val)
{
    volumeAtts->SetLowGradientLightingReduction(
        (VolumeAttributes::LowGradientLightingReduction)val);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::lowGradientClampToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the low gradient
//   lighting clamp flag is toggled.
//   
//
// Programmer: Jeremy Meredith
// Creation:   January  5, 2010
//
// Modifications:
//   Jeremy Meredith, Tue Jan  5 15:51:26 EST 2010
//   The low-gradient-mag lighting reduction now also applies to the 
//   3D texturing and splatting volume renderers.
//   
// ****************************************************************************

void
QvisVolumePlotWindow::lowGradientClampToggled(bool val)
{
    volumeAtts->SetLowGradientLightingClampFlag(val);
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::lowGradientClampProcessText
//
//  Purpose:
//    Qt slot function, called when the low gradient lighting clamp
//    value is changed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  5, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::lowGradientClampProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_lowGradientLightingClampValue);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisVolumePlotWindow::limitsSelectChanged
//
//   This is a Qt slot function that is called when the limits for the data
//   is selected.
//
// Programmer: Allen Sanderson
// Creation:   August 11, 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::limitsSelectChanged(int mode)
{
    // Only do it if it changed.
    if(mode != volumeAtts->GetLimitsMode())
    {
        volumeAtts->SetLimitsMode(VolumeAttributes::LimitsMode(mode));
        Apply();
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::colorMinToggled
//
//   This is a Qt slot function that is called when the extents for the colors
//   toggle is clicked.
//
// Programmer: Hank Childs
// Creation:   December 11, 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::colorMinToggled(bool)
{
    volumeAtts->SetUseColorVarMin(!volumeAtts->GetUseColorVarMin());
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::colorMinProcessText
//
// Purpose:
//   Qt slot function, called when colorMin is changed.
//
// Programmer:  Hank Childs
// Creation:    December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 12:11:59 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::colorMinProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_colorVarMin);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::colorMaxToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the extents for the colors
//   toggle is clicked.
//
// Programmer: Hank Childs
// Creation:   December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:27:57 PDT 2002
//   Removed code to enable the line edit.
//   
// ****************************************************************************

void
QvisVolumePlotWindow::colorMaxToggled(bool)
{
    volumeAtts->SetUseColorVarMax(!volumeAtts->GetUseColorVarMax());
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::colorMaxProcessText
//
//  Purpose:
//    Qt slot function, called when colorMax is changed.
//
//  Programmer:  Hank Childs
//  Creation:    December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:28:42 PDT 2002
//   Prevented update.
//
// ****************************************************************************

void
QvisVolumePlotWindow::colorMaxProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_colorVarMax);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::opacityMinToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the extents for the opacity
//   toggle is clicked.
//
// Programmer: Hank Childs
// Creation:   December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:27:57 PDT 2002
//   Removed code to enable the line edit.
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityMinToggled(bool)
{
    volumeAtts->SetUseOpacityVarMin(!volumeAtts->GetUseOpacityVarMin());
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::opacityMinProcessText
//
//  Purpose:
//    Qt slot function, called when opacityMin is changed.
//
//  Programmer:  Hank Childs
//  Creation:    December 11, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityMinProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_opacityVarMin);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::opacityMaxToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the extents for the opacity
//   toggle is clicked.
//
// Programmer: Hank Childs
// Creation:   December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:27:57 PDT 2002
//   Removed code to enable the line edit.
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityMaxToggled(bool)
{
    volumeAtts->SetUseOpacityVarMax(!volumeAtts->GetUseOpacityVarMax());
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::opacityMaxProcessText
//
// Purpose:
//   Qt slot function, called when opacityMax is changed.
//
// Programmer:  Hank Childs
// Creation:    December 11, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 12:14:40 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityMaxProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_opacityVarMax);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::smoothingMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's smooth
//   combobox is activated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:38:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 27 16:04:53 PDT 2012
//   work with different smoothing types.
//
// ****************************************************************************

void
QvisVolumePlotWindow::smoothingMethodChanged(int val)
{
    GetCurrentValues(0);
    volumeAtts->SetSmoothing(ColorControlPointList::SmoothingMethod(val));
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::equalSpacingToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's equal spacing
//   toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:38:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 10:15:18 PDT 2001
//   Modified to account for changes in the attributes.
//
// ****************************************************************************

void
QvisVolumePlotWindow::equalSpacingToggled(bool val)
{
    GetCurrentValues(0);
    volumeAtts->SetEqualSpacingFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::alphaValuesChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the opacity bar is
//   modified.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:38:58 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 10:15:18 PDT 2001
//   Modified to account for changes in the attributes.
//   
// ****************************************************************************

void
QvisVolumePlotWindow::alphaValuesChanged()
{
    GetCurrentValues(1);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::samplesPerRayProcessText
//
// Purpose:
//   Qt slot function, called when samplesPerRay is changed.
//
// Programmer:  Hank Childs
// Creation:    February 8, 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:01:40 PDT 2002
//   Prevented updates.
//
// ****************************************************************************

void
QvisVolumePlotWindow::samplesPerRayChanged(int val)
{
    volumeAtts->SetSamplesPerRay(val);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::resampleTargetProcessText
//
// Purpose:
//   Qt slot function, called when resampleTarget is changed.
//
// Programmer:  Jeremy Meredith
// Creation:    November 13, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 10:03:10 PDT 2002
//   Removed a function call that upates widgets since that is done in
//   UpdateWindow as a result of the Apply function call.
//
// ****************************************************************************

void
QvisVolumePlotWindow::resampleTargetChanged(int val)
{
    volumeAtts->SetResampleTarget(val);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::opacityVariableChanged
//
// Purpose:
//   Qt slot function, called when opacityVariable is changed.
//
// Programmer:  Jeremy Meredith
// Creation:    November 13, 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:39:00 PST 2004
//   I renamed the method and recoded it.
//
//   Hank Childs, Sun Dec  4 18:30:40 PST 2005
//   Set whether or not the toggles should be active ['6827].
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityVariableChanged(const QString &var)
{
    volumeAtts->SetOpacityVariable(var.toStdString());
    bool notDefaultVar = (volumeAtts->GetOpacityVariable() != "default");
    opacityMinToggle->setEnabled(notDefaultVar);
    opacityMaxToggle->setEnabled(notDefaultVar);
    opacityMin->setEnabled(notDefaultVar && volumeAtts->GetUseOpacityVarMin());
    opacityMax->setEnabled(notDefaultVar && volumeAtts->GetUseOpacityVarMax());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::compactVariableChanged
//
// Purpose:
//   Qt slot function, called when compactVariable is changed.
//
// Programmer:  Allen Harvey
// Creation:    October 30, 2011
//
// ****************************************************************************

void
QvisVolumePlotWindow::compactVariableChanged(const QString &var)
{
    volumeAtts->SetCompactVariable(var.toStdString());
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisVolumePlotWindow::gradientTypeChanged
//
//  Purpose:
//    Update the gradient type based on user input
//
//  Arguments:
//    val        the new gradient type
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
void
QvisVolumePlotWindow::gradientTypeChanged(int val)
{
    switch (val)
    {
      case 0:
        volumeAtts->SetGradientType(VolumeAttributes::CenteredDifferences);
        break;
      case 1:
        volumeAtts->SetGradientType(VolumeAttributes::SobelOperator);
        break;
      default:
        EXCEPTION1(ImproperUseException,
                   "The Volume plot received a signal for a gradient method "
                   "that it didn't understand");
        break;
    }
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::samplingTypeChanged
//
//  Purpose:
//    Update the sampling type based on user input
//
//  Arguments:
//    val        the new sampling type
//
//  Programmer:  Hank Childs
//  Creation:    January 8, 2005
//
// ****************************************************************************
void
QvisVolumePlotWindow::samplingTypeChanged(int val)
{
    switch (val)
    {
      case 0:
        volumeAtts->SetSampling(VolumeAttributes::Rasterization);
        break;
      case 1:
        volumeAtts->SetSampling(VolumeAttributes::KernelBased);
        break;
      case 2:
        volumeAtts->SetSampling(VolumeAttributes::Trilinear);
        break;
      default:
        EXCEPTION1(ImproperUseException,
                   "The Volume plot received a signal for a sampling method "
                   "that it didn't understand");
        break;
    }
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::rendererTypeChanged
//
//  Purpose:
//    Update the renderer type based on user input
//
//  Arguments:
//    val        the new renderer type
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
//  Modifications:
//    Brad Whitlock, Wed Dec 15 09:29:12 PDT 2004
//    I added the RayCasting option.
//
//    Hank Childs, Mon Sep 11 10:34:32 PDT 2006
//    I added the RayCastingIntegration option.
//
//    Brad Whitlock, Thu Jan 10 14:38:21 PST 2008
//    Added SLIVR support.
//
//    Josh Stratton, Mon Dec 15 13:01:07 MST 2008
//    Added Tuvok support.
//
//    Tom Fogal, Thu May 13 09:43:31 MDT 2010
//    Fix case where tuvok is missing.
//
//    Brad Whitlock, Tue Jan 31 16:30:58 PST 2012
//    Force resampling for HW accelerated renderers except for Splatting.
//
// ****************************************************************************

void
QvisVolumePlotWindow::rendererTypeChanged(int val)
{
    switch (val)
    {
      case 0:
        volumeAtts->SetRendererType(VolumeAttributes::Splatting);
        break;
      case 1:
        volumeAtts->SetRendererType(VolumeAttributes::Texture3D);
        break;
      case 2:
        volumeAtts->SetRendererType(VolumeAttributes::RayCasting);
        break;
      case 3:
        volumeAtts->SetRendererType(VolumeAttributes::RayCastingIntegration);
        break;
      case 4:
      case 5:
      case 6:
      {
          if (rendererTypesComboBox->findText("Tuvok") == val)
              volumeAtts->SetRendererType(VolumeAttributes::Tuvok);
          else if (rendererTypesComboBox->findText("SLIVR") == val)
              volumeAtts->SetRendererType(VolumeAttributes::SLIVR);
          else if (rendererTypesComboBox->findText("Ray casting: SLIVR") == val)
              volumeAtts->SetRendererType(VolumeAttributes::RayCastingSLIVR);
          break;
      }
      default:
        EXCEPTION1(ImproperUseException,
                   "The Volume plot received a signal for a renderer "
                   "that it didn't understand");
        break;
    }

    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::num3DSlicesProcessText
//
//  Purpose:
//    Update the number of 3D texturing slices based on user input
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2003
//
// ****************************************************************************
void
QvisVolumePlotWindow::num3DSlicesChanged(int val)
{
    volumeAtts->SetNum3DSlices(val);
    Apply();
}


// ****************************************************************************
//  Method:  QvisVolumePlotWindow::scaleClicked
//
//  Purpose:
//    Update the scaling type. 
//
//  Arguments:
//    scale      The new scaling type.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 3, 2005 
//
// ****************************************************************************

void
QvisVolumePlotWindow::scaleClicked(int scale)
{
    // Only do it if it changed.
    if(scale != volumeAtts->GetScaling())
    {
        volumeAtts->SetScaling(VolumeAttributes::Scaling(scale));
        Apply();
    }
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that imports the desired color table as color
//   component of the transfer function in the volume plot attributes.
//
// Arguments:
//   useDefault : Whether or not to use the default color table.
//   ctName     : The name of the color table to use.
//
//  Programmer:  Gunther H. Weber 
//  Creation:    April 5, 2007
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 14:17:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVolumePlotWindow::colorTableClicked(bool useDefault, const QString &ctName)
{
    ColorTableAttributes *cta = GetViewerState()->GetColorTableAttributes();
    const ColorControlPointList *ccp = cta->GetColorControlPoints(ctName.toStdString());
    if (ccp) volumeAtts->SetColorControlPoints(*ccp);
    UpdateColorControlPoints();
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::processSkewText
//
//  Purpose:
//    Update the skew factor. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 3, 2005 
//
// ****************************************************************************

void
QvisVolumePlotWindow::processSkewText()
{
    GetCurrentValues(VolumeAttributes::ID_skewFactor);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::rendererSamplesProcessText
//
// Purpose:
//   Qt slot function, called when rendererSamples is changed.
//
// Programmer:  Brad Whitlock
// Creation:    Fri Jan 11 16:07:29 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::rendererSamplesChanged(double val)
{
    volumeAtts->SetRendererSamples(val);
    Apply();
}

// ****************************************************************************
//  Method:  QvisVolumePlotWindow::transferDimChanged
//
//  Purpose:
//    Update the UI to handle 1D or 2D transfer functions
//
//  Arguments:
//    val        the dimension of the transfer function
//
//  Programmer:  Josh Stratton
//  Creation:    Fri Sep  5 13:26:47 MDT 2008
//
//  Modifications:
//    Call Apply.
//
// ****************************************************************************
void
QvisVolumePlotWindow::transferDimChanged(int val)
{
    volumeAtts->SetTransferFunctionDim(val + 1);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::updateTransferFunc2D
//
// Purpose:
//   Qt slot function, called when SLIVR transfer function changes
//
// Arguments:
//   none
//
// Programmer:  Josh Stratton
// Creation:    Fri May 23 12:43:38 MDT 2008
//
// Modifications:
//   Tom Fogal, Fri Sep 19 11:02:10 MDT 2008
//   Wrap definition in HAVE_LIBSLIVR.
//
//   Brad Whitlock, Tue Sep 30 09:56:49 PDT 2008
//   Rewrote.
//
//   Brad Whitlock, Mon Jan 12 14:52:00 PST 2009
//   Added code to save the widget name.
//
// ****************************************************************************

void
QvisVolumePlotWindow::updateTransferFunc2D()
{
#ifdef HAVE_LIBSLIVR
    // scrap all current widgets
    volumeAtts->ClearTransferFunction2DWidgets();

    int num_widgets = transferFunc2D->numWidgets();
    for (int i = 0; i < num_widgets; i++)
    {
        WidgetID id = transferFunc2D->getID(i);
        QString qdef = transferFunc2D->getString(id);
        QString shapeType(qdef.left(1));
        QString shapeArgs(qdef.right(qdef.length()-1));

        TransferFunctionWidget widget;
        float position[8];
        bool okay = false;

        if (shapeType == "r")
        {
            float tmp[6];
            // Compensate for an extra zero on the front of the rect string.
            okay = QStringToFloats(shapeArgs, tmp, 6);
            for(int i = 0; i < 5; ++i)
                position[i] = tmp[i+1];
            widget.SetType(TransferFunctionWidget::Rectangle);
        }
        else if (shapeType == "t")
        {
            okay = QStringToFloats(shapeArgs, position, 5);
            widget.SetType(TransferFunctionWidget::Triangle);
        }
        else if (shapeType == "p")
        {
            okay = QStringToFloats(shapeArgs, position, 8);
            widget.SetType(TransferFunctionWidget::Paraboloid);
        }
        else if (shapeType == "e")
        {
            okay = QStringToFloats(shapeArgs, position, 5);
            widget.SetType(TransferFunctionWidget::Ellipsoid);
        }

        if(okay)
        {
            // Save the widget name
            widget.SetName(transferFunc2D->getName(id).toStdString());

            // parse color
            QColor qcolor = transferFunc2D->getColor(id);
            float color[4];
            color[0] = qcolor.red() / 255.0f;
            color[1] = qcolor.green() / 255.0f;
            color[2] = qcolor.blue() / 255.0f;
            color[3] = transferFunc2D->getAlpha(id);
            widget.SetBaseColor(color);
            widget.SetPosition(position);
            volumeAtts->AddTransferFunction2DWidgets(widget);
        }
    }

    SetUpdate(false);
    Apply();
#endif /* HAVE_LIBSLIVR */
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::updateTransferFunc2D
//
// Purpose:
//   Qt slot function, called when SLIVR transfer function changes
//
// Arguments:
//   none
//
// Programmer:  Josh Stratton
// Creation:    Fri May 23 12:43:38 MDT 2008
//
// Modifications:
//
//   Tom Fogal, Fri Sep 19 11:02:10 MDT 2008
//   Wrap definition in HAVE_LIBSLIVR.
//
// ****************************************************************************
void
QvisVolumePlotWindow::updateTransferFunc2D(WidgetID id)
{
#ifdef HAVE_LIBSLIVR
    updateTransferFunc2D();
#endif /* HAVE_LIBSLIVR */
}


// ****************************************************************************
// Method:  QvisVolumePlotWindow::setLight...
//
// Purpose:
//   
//
//  Arguments:
//    val        
//
// Programmer:  Pascal Grosset
// Creation:    Tue Apr 10 2012
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::setMaterialKa(double val){
    double *mat = new double[4];

    mat = volumeAtts->GetMaterialProperties();
    mat[0]=val;
    volumeAtts->SetMaterialProperties(mat);
    SetUpdate(false);

    Apply();
}


void
QvisVolumePlotWindow::setMaterialKd(double val){
    double *mat = new double[4];

    mat = volumeAtts->GetMaterialProperties();
    mat[1]=val;
    volumeAtts->SetMaterialProperties(mat);
    SetUpdate(false);

    Apply();
}


void
QvisVolumePlotWindow::setMaterialKs(double val){
    double *mat = new double[4];
    
    mat = volumeAtts->GetMaterialProperties();
    mat[2]=val;
    volumeAtts->SetMaterialProperties(mat);
    SetUpdate(false);

    Apply();
}


void
QvisVolumePlotWindow::setMaterialN(double val){
    double *mat = new double[4];

    mat = volumeAtts->GetMaterialProperties();
    mat[3]=val;
    volumeAtts->SetMaterialProperties(mat);
    SetUpdate(false);

    Apply();
}
