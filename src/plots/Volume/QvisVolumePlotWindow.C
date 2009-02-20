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
#include <QSlider>
#include <QTabWidget>

#include <math.h>
#include <stdio.h>

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

#ifdef HAVE_LIBSLIVR 
    // Create the transfer function dimension button
    QWidget *tfWidget = new QWidget(central);
    topLayout->addWidget(tfWidget);
    QHBoxLayout *tfLayout = new QHBoxLayout(tfWidget);
    tfLayout->setMargin(0);
    tfLayout->addWidget(new QLabel(tr("Transfer function"), tfWidget));
    transferFunctionGroup = new QButtonGroup(this);
    connect(transferFunctionGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(transferDimChanged(int)));
    oneDimButton = new QRadioButton(tr("1D"), tfWidget);
    transferFunctionGroup->addButton(oneDimButton, 0);
    tfLayout->addWidget(oneDimButton);
    twoDimButton = new QRadioButton(tr("2D"), tfWidget);
    transferFunctionGroup->addButton(twoDimButton, 1);
    tfLayout->addWidget(twoDimButton);
    tfLayout->addStretch(10);

    // Create a tab widget so we can put the transfer functions on their
    // own tabs.
    QTabWidget *tfTabs = new QTabWidget(central);
    topLayout->addWidget(tfTabs, 100);

    // Create the transfer function pages.
    tfParent1D = Create1DTransferFunctionGroup(maxWidth);
    tfParent2D = Create2DTransferFunctionGroup();

    tfTabs->addTab(tfParent1D, tr("1D transfer function"));
    tfTabs->addTab(tfParent2D, tr("2D transfer function"));
#else
    // We don't have SLIVR so only create the 1D transfer function widgets
    // and parent them directly in the window.
    QWidget *tf1 = Create1DTransferFunctionGroup(maxWidth);
    topLayout->addWidget(tf1, 100);
#endif

    CreateOptions(maxWidth);

    // Create the color selection widget.
    colorSelect = new QvisColorSelectionWidget(0);
    connect(colorSelect, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedColor(const QColor &)));
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
// ****************************************************************************

void
QvisVolumePlotWindow::CreateColorGroup(QWidget *parent, QVBoxLayout *pLayout,
    int maxWidth)
{
    // Add the group box that will contain the color-related widgets.
    colorWidgetGroup = new QGroupBox(parent);
    colorWidgetGroup->setTitle(tr("Color"));
    pLayout->addWidget(colorWidgetGroup);
    QVBoxLayout *innerColorLayout = new QVBoxLayout(colorWidgetGroup);
    innerColorLayout->setMargin(5);

    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *seLayout = new QHBoxLayout(0);
    seLayout->setMargin(0);
    innerColorLayout->addLayout(seLayout);
    seLayout->setSpacing(5);
    seLayout->addSpacing(5);
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

    colorTableButton = new QvisColorTableButton(colorWidgetGroup);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    seLayout->addWidget(colorTableButton);

    seLayout->addSpacing(5);
    seLayout->addStretch(20);

    smoothCheckBox = new QCheckBox(tr("Smooth"), colorWidgetGroup);
    smoothCheckBox->setChecked(true);
    connect(smoothCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(smoothToggled(bool)));
    seLayout->addWidget(smoothCheckBox);

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

    // Create the color min widgets.
    QGridLayout *colorScaleLayout = new QGridLayout(0);
    colorScaleLayout->setMargin(0);
    innerColorLayout->addLayout(colorScaleLayout);
    colorScaleLayout->setSpacing(5);
    colorMinToggle = new QCheckBox(tr("Minimum"), colorWidgetGroup);
    connect(colorMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMinToggled(bool)));
    colorScaleLayout->addWidget(colorMinToggle, 1, 0);
    colorMin = new QLineEdit(colorWidgetGroup);
    colorMin->setMaximumWidth(maxWidth);
    colorMin->setEnabled(volumeAtts->GetUseColorVarMin());
    connect(colorMin, SIGNAL(returnPressed()),
            this, SLOT(colorMinProcessText()));
    colorScaleLayout->addWidget(colorMin, 1, 1);

    // Create the color max widgets.
    colorMaxToggle = new QCheckBox(tr("Maximum"), colorWidgetGroup);
    connect(colorMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMaxToggled(bool)));
    colorScaleLayout->addWidget(colorMaxToggle, 0, 0);

    colorMax = new QLineEdit(colorWidgetGroup);
    colorMax->setMaximumWidth(maxWidth);
    colorMax->setEnabled(volumeAtts->GetUseColorVarMax());
    connect(colorMax, SIGNAL(returnPressed()),
            this, SLOT(colorMaxProcessText()));
    colorScaleLayout->addWidget(colorMax, 0, 1);

    //
    // Create the scale radio buttons
    //
    QLabel *scaleLabel = new QLabel(tr("Scale"), colorWidgetGroup);
    colorScaleLayout->addWidget(scaleLabel, 0, 2);

    // Create the scaling button group 
    scaling = new QComboBox(colorWidgetGroup);
    scaling->addItem(tr("Linear"));
    scaling->addItem(tr("Log10"));
    scaling->addItem(tr("Skew"));
    connect(scaling, SIGNAL(activated(int)),
            this, SLOT(scaleClicked(int)));
    colorScaleLayout->addWidget(scaling, 0, 3);

    // Create the skew factor line edit    
    skewLineEdit = new QNarrowLineEdit(colorWidgetGroup);
    skewLineEdit->setMaximumWidth(maxWidth);
    connect(skewLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSkewText())); 
    skewLabel = new QLabel(tr("Skew factor"), colorWidgetGroup);
    skewLabel->setBuddy(skewLineEdit);
    colorScaleLayout->addWidget(skewLabel, 1, 2);
    colorScaleLayout->addWidget(skewLineEdit, 1, 3);
    colorScaleLayout->setColumnStretch(4, 10);
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

    // Create the buttons that control what mode the opacity widget it in.
    QHBoxLayout *opLayout = new QHBoxLayout(0);
    opLayout->setMargin(0);
    innerOpacityLayout->addLayout(opLayout);
    showColorsInAlphaWidgetToggle = new QCheckBox(tr("Show colors"), opacityWidgetGroup);
    showColorsInAlphaWidgetToggle->setChecked(showColorsInAlphaWidget);
    connect(showColorsInAlphaWidgetToggle, SIGNAL(toggled(bool)),
            this, SLOT(showColorsInAlphaWidgetToggled(bool)));
    opLayout->addWidget(showColorsInAlphaWidgetToggle);
    opLayout->addSpacing(10);
    opLayout->addStretch(100);
    opLayout->setSpacing(5);
    QLabel *interactionModeLabel = new QLabel(tr("Interaction mode"),
        opacityWidgetGroup);
    opLayout->addWidget(interactionModeLabel);
    
    // Create the interaction mode button group.
    modeButtonGroup = new QButtonGroup(opacityWidgetGroup);
    connect(modeButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(interactionModeChanged(int)));
    QRadioButton *rb= new QRadioButton(tr("Freeform"), opacityWidgetGroup);
    modeButtonGroup->addButton(rb, 0);
    opLayout->addWidget(rb, 5, 0);
    rb = new QRadioButton(tr("Gaussian"), opacityWidgetGroup);
    rb->setChecked(true);
    modeButtonGroup->addButton(rb, 1);
    opLayout->addWidget(rb, 6, 0);
    rb = new QRadioButton(tr("From Color Table"), opacityWidgetGroup);
    rb->setChecked(true);
    modeButtonGroup->addButton(rb, 2);
    opLayout->addWidget(rb, 7, 0);

    // Create the gaussian opacity editor widget.
    QVBoxLayout *barLayout = new QVBoxLayout(0);
    barLayout->setMargin(0);
    barLayout->setSpacing(0);
    innerOpacityLayout->addLayout(barLayout);
    innerOpacityLayout->setStretchFactor(barLayout, 100);
    alphaWidget = new QvisGaussianOpacityBar(opacityWidgetGroup);
    alphaWidget->setMinimumHeight(80);
    connect(alphaWidget, SIGNAL(mouseReleased()),
            this, SLOT(alphaValuesChanged()));
    barLayout->addWidget(alphaWidget, 100);

    // Create the scribble opacity editor widget.
    scribbleAlphaWidget = new QvisScribbleOpacityBar(opacityWidgetGroup);
    scribbleAlphaWidget->setMinimumHeight(80);
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
    QPixmap whiteIcon(white_xpm);

    QHBoxLayout *abLayout = new QHBoxLayout(0);
    abLayout->setMargin(0);
    innerOpacityLayout->addLayout(abLayout);
    abLayout->setSpacing(5);
    abLayout->addStretch(10);
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

    oneButton = new QPushButton(opacityWidgetGroup);
    oneButton->setIcon(QIcon(whiteIcon));
    connect(oneButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTotallyOne()));
    abLayout->addWidget(oneButton);

    smoothButton = new QPushButton(tr("Smooth"), opacityWidgetGroup);
    connect(smoothButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(smoothCurve()));
    abLayout->addWidget(smoothButton);
    abLayout->addStretch(10);

    // Create the opacity attenuation widgets.
    QGridLayout *attLayout = new QGridLayout(0);
    innerOpacityLayout->addLayout(attLayout);
    attLayout->setMargin(0);
    attLayout->setSpacing(5);
    attenuationSlider = new QvisOpacitySlider(0, 255, 10, 255, opacityWidgetGroup);
    attenuationSlider->setGradientColor(QColor(0,0,0));
    QLabel *attenuationLabel = new QLabel(tr("Attenuation"), opacityWidgetGroup);
    attenuationLabel->setBuddy(attenuationSlider);
    connect(attenuationSlider, SIGNAL(valueChanged(int)),
            this, SLOT(attenuationChanged(int)));
    attLayout->addWidget(attenuationLabel, 0, 2);
    attLayout->addWidget(attenuationSlider, 0, 3);

    // Create the opacity variable
    opacityVariable = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars, opacityWidgetGroup);
    connect(opacityVariable, SIGNAL(activated(const QString &)),
            this, SLOT(opacityVariableChanged(const QString &)));
    QLabel *opacityVarLabel = new QLabel(tr("Opacity variable"), opacityWidgetGroup);
    opacityVarLabel->setBuddy(opacityVariable);
    attLayout->addWidget(opacityVarLabel, 1, 2);
    attLayout->addWidget(opacityVariable, 1, 3);
    attLayout->setColumnStretch(3, 10);

    // Create the opacity min widgets.
    opacityMinToggle = new QCheckBox(tr("Minimum"), opacityWidgetGroup);
    connect(opacityMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMinToggled(bool)));
    attLayout->addWidget(opacityMinToggle,1,0);
    opacityMin = new QLineEdit(opacityWidgetGroup);
    opacityMin->setMaximumWidth(maxWidth);
    opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
    connect(opacityMin, SIGNAL(returnPressed()),
            this, SLOT(opacityMinProcessText()));
    attLayout->addWidget(opacityMin,1,1);

    // Create the opacity max widgets.
    opacityMaxToggle = new QCheckBox(tr("Maximum"), opacityWidgetGroup);
    connect(opacityMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMaxToggled(bool)));
    attLayout->addWidget(opacityMaxToggle, 0, 0);

    opacityMax = new QLineEdit(opacityWidgetGroup);
    opacityMax->setMaximumWidth(maxWidth);
    opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
    connect(opacityMax, SIGNAL(returnPressed()),
            this, SLOT(opacityMaxProcessText()));
    attLayout->addWidget(opacityMax, 0, 1);
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
// Method: QvisVolumePlotWindow::CreateOptions
//
// Purpose: 
//   Creates the widgets in the options group.
//
// Note:       Adapted from CreateWindowContents when porting to Qt 4.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 14:58:34 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::CreateOptions(int maxWidth)
{
    // Create the rendering method radio buttons.
    int row = 0;
    QGridLayout *rendererOptionsLayout = new QGridLayout(0);
    rendererOptionsLayout->setMargin(0);
    rendererOptionsLayout->setSpacing(5);
    topLayout->addLayout(rendererOptionsLayout);
    rendererTypesComboBox = new QComboBox(central);
    rendererTypesComboBox->addItem(tr("Splatting"));
    rendererTypesComboBox->addItem(tr("3D Texturing"));
    rendererTypesComboBox->addItem(tr("Ray casting: compositing"));
    rendererTypesComboBox->addItem(tr("Ray casting: integration (grey scale)"));
    rendererTypesComboBox->addItem(tr("SCI, University of Utah (Tuvok)"));
#ifdef HAVE_LIBSLIVR
    rendererTypesComboBox->addItem(tr("SCI, University of Utah (SLIVR)"));
#endif
    connect(rendererTypesComboBox, SIGNAL(activated(int)),
            this, SLOT(rendererTypeChanged(int)));
    rendererOptionsLayout->addWidget(new QLabel(tr("Rendering method"), central),
        row, 0);
    rendererOptionsLayout->addWidget(rendererTypesComboBox, row,1, 1, 2);
    ++row;

    // Create the resample target value
    resampleTarget = new QLineEdit(central);
    connect(resampleTarget, SIGNAL(returnPressed()),
            this, SLOT(resampleTargetProcessText()));
    resampleTargetLabel = new QLabel(tr("Number of samples"), central);
    resampleTargetLabel->setBuddy(resampleTarget);
    resampleTargetSlider = new QSlider(central);
    resampleTargetSlider->setMinimum(0);
    resampleTargetSlider->setMaximum(80);
    resampleTargetSlider->setPageStep(10);
    resampleTargetSlider->setOrientation(Qt::Horizontal);
    connect(resampleTargetSlider, SIGNAL(valueChanged(int)),
            this, SLOT(resampleTargetSliderChanged(int)));
    connect(resampleTargetSlider, SIGNAL(sliderReleased()),
            this, SLOT(resampleTargetSliderReleased()));
    rendererOptionsLayout->addWidget(resampleTargetLabel, row, 0);
    rendererOptionsLayout->addWidget(resampleTarget, row, 1);
    rendererOptionsLayout->addWidget(resampleTargetSlider, row, 2); 
    ++row;

    // Create the number of 3D slices.
    num3DSlices = new QLineEdit(central);
    connect(num3DSlices, SIGNAL(returnPressed()), this,
            SLOT(num3DSlicesProcessText()));
    num3DSlicesLabel = new QLabel(tr("Number of slices"), central);
    num3DSlicesLabel->setBuddy(num3DSlices);
    rendererOptionsLayout->addWidget(num3DSlicesLabel, row, 0);
    rendererOptionsLayout->addWidget(num3DSlices, row, 1);
    ++row;

    // Create the number of samples per ray.
    samplesPerRay = new QLineEdit(central);
    connect(samplesPerRay, SIGNAL(returnPressed()), this,
            SLOT(samplesPerRayProcessText()));
    samplesPerRayLabel = new QLabel(tr("Samples per ray"), central);
    samplesPerRayLabel->setBuddy(samplesPerRay);
    rendererOptionsLayout->addWidget(samplesPerRayLabel, row, 0);
    rendererOptionsLayout->addWidget(samplesPerRay, row, 1);
    ++row;

#ifdef HAVE_LIBSLIVR
    rendererSamples = new QLineEdit(central);
    connect(rendererSamples, SIGNAL(returnPressed()),
            this, SLOT(rendererSamplesProcessText()));
    rendererSamplesLabel = new QLabel(tr("Sampling rate"), central);
    rendererSamplesLabel->setBuddy(rendererSamples);
    rendererSamplesSlider = new QSlider(central);
    rendererSamplesSlider->setMinimum(0);
    rendererSamplesSlider->setMaximum(1000);
    rendererSamplesSlider->setOrientation(Qt::Horizontal);
    rendererSamplesSlider->setPageStep(25);
    rendererSamplesSlider->setTickInterval(150);
    connect(rendererSamplesSlider, SIGNAL(valueChanged(int)),
            this, SLOT(rendererSamplesSliderChanged(int)));
    connect(rendererSamplesSlider, SIGNAL(sliderReleased()),
            this, SLOT(rendererSamplesSliderReleased()));
    rendererOptionsLayout->addWidget(rendererSamplesLabel, row, 0);
    rendererOptionsLayout->addWidget(rendererSamples, row, 1);
    rendererOptionsLayout->addWidget(rendererSamplesSlider, row, 2);
    ++row;
#else
    rendererSamples = 0;
    rendererSamplesSlider = 0;
    rendererSamplesLabel = 0;
#endif

    // Create the gradient method radio buttons.
    rendererOptionsLayout->addWidget(new QLabel(tr("Gradient method")),row,0);
    gradientButtonGroup = new QButtonGroup(central);
    connect(gradientButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(gradientTypeChanged(int)));
    centeredDiffButton = new QRadioButton(tr("Centered diff"),central);
    gradientButtonGroup->addButton(centeredDiffButton, 0);
    rendererOptionsLayout->addWidget(centeredDiffButton,row,1);
    sobelButton = new QRadioButton(tr("Sobel"),central);
    gradientButtonGroup->addButton(sobelButton, 1);
    rendererOptionsLayout->addWidget(sobelButton,row,2);
    ++row;

    // Create the sampling method buttons.
    rendererOptionsLayout->addWidget(new QLabel(tr("Sampling method")),row,0);
    samplingButtonGroup = new QButtonGroup(central);
    connect(samplingButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(samplingTypeChanged(int)));
    rasterizationButton = new QRadioButton(tr("Rasterization"),central);
    samplingButtonGroup->addButton(rasterizationButton, 0);
    rendererOptionsLayout->addWidget(rasterizationButton,row,1);
    kernelButton = new QRadioButton(tr("Kernel Based"),central);
    samplingButtonGroup->addButton(kernelButton, 1);
    rendererOptionsLayout->addWidget(kernelButton,row,2);
    ++row;

    // Create the legend toggle.
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    rendererOptionsLayout->addWidget(legendToggle,row,0);

    // Create the lighting toggle.
    lightingToggle = new QCheckBox(tr("Lighting"), central);
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    rendererOptionsLayout->addWidget(lightingToggle,row,1);

    // Create the smooth data toggle.
    smoothDataToggle = new QCheckBox(tr("Smooth Data"), central);
    connect(smoothDataToggle, SIGNAL(toggled(bool)),
            this, SLOT(smoothDataToggled(bool)));
    rendererOptionsLayout->addWidget(smoothDataToggle,row,2);
    ++row;
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
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateHistogram()
{
    PlotInfoAttributes *info = GetViewerState()->GetPlotInformation(plotType);
    bool invalid = true;

    MapNode *hNode = info->GetData().GetEntry("VolumeHistogram");
    if(hNode != 0)
    {
        MapNode &vhist = *hNode;
        int hist_size = vhist["histogram_size"].AsInt();

        const floatVector &hist = vhist["histogram_1d"].AsFloatVector();
        alphaWidget->setHistogramTexture(&hist[0], hist_size);
        scribbleAlphaWidget->setHistogramTexture(&hist[0], hist_size);

#ifdef HAVE_LIBSLIVR
        const unsignedCharVector &hist2 = vhist["histogram_2d"].AsUnsignedCharVector();
        unsignedCharVector decompressed;
        VolumeRLEDecompress(hist2, decompressed);
        transferFunc2D->setHistogramTexture(&decompressed[0], hist_size);
#endif
        invalid = false;
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
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // If the plot info atts changed then update the histogram.
    if(doAll || SelectedSubject() == GetViewerState()->GetPlotInformation(plotType))
    {
        UpdateHistogram();
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
        case VolumeAttributes::ID_lightingFlag:
            lightingToggle->blockSignals(true);
            lightingToggle->setChecked(volumeAtts->GetLightingFlag());
            lightingToggle->blockSignals(false);
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
            temp.sprintf("%d", volumeAtts->GetResampleTarget());
            resampleTarget->setText(temp);
            SetResampleTargetSliderFromAtts();
            break;
        case VolumeAttributes::ID_opacityVariable:
            temp = volumeAtts->GetOpacityVariable().c_str();
            opacityVariable->setText(temp);
            break;
        case VolumeAttributes::ID_freeformOpacity:
            UpdateFreeform();
            break;
        case VolumeAttributes::ID_useColorVarMin:
            colorMinToggle->blockSignals(true);
            colorMinToggle->setChecked(volumeAtts->GetUseColorVarMin());
            colorMinToggle->blockSignals(false);
            colorMin->setEnabled(volumeAtts->GetUseColorVarMin());
            break;
        case VolumeAttributes::ID_colorVarMin:
            temp.setNum(volumeAtts->GetColorVarMin());
            colorMin->setText(temp);
            break;
        case VolumeAttributes::ID_useColorVarMax:
            colorMaxToggle->blockSignals(true);
            colorMaxToggle->setChecked(volumeAtts->GetUseColorVarMax());
            colorMaxToggle->blockSignals(false);
            colorMax->setEnabled(volumeAtts->GetUseColorVarMax());
            break;
        case VolumeAttributes::ID_colorVarMax:
            temp.setNum(volumeAtts->GetColorVarMax());
            colorMax->setText(temp);
            break;
        case VolumeAttributes::ID_useOpacityVarMin:
            opacityMinToggle->blockSignals(true);
            opacityMinToggle->setChecked(volumeAtts->GetUseOpacityVarMin());
            opacityMinToggle->blockSignals(false);
            opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
            break;
        case VolumeAttributes::ID_opacityVarMin:
            temp.setNum(volumeAtts->GetOpacityVarMin());
            opacityMin->setText(temp);
            break;
        case VolumeAttributes::ID_useOpacityVarMax:
            opacityMaxToggle->blockSignals(true);
            opacityMaxToggle->setChecked(volumeAtts->GetUseOpacityVarMax());
            opacityMaxToggle->blockSignals(false);
            opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
            break;
        case VolumeAttributes::ID_opacityVarMax:
            temp.setNum(volumeAtts->GetOpacityVarMax());
            opacityMax->setText(temp);
            break;
        case VolumeAttributes::ID_smoothData:
            smoothDataToggle->blockSignals(true);
            smoothDataToggle->setChecked(volumeAtts->GetSmoothData());
            smoothDataToggle->blockSignals(false);
        case VolumeAttributes::ID_samplesPerRay:
            temp.sprintf("%d", volumeAtts->GetSamplesPerRay());
            samplesPerRay->setText(temp);
            break;
        case VolumeAttributes::ID_rendererType:
            rendererTypesComboBox->blockSignals(true);
            if (volumeAtts->GetRendererType() == VolumeAttributes::Splatting)
            {
                colorWidgetGroup->show();
                opacityWidgetGroup->show();
                colorWidgetGroup->setEnabled(true);
                opacityWidgetGroup->setEnabled(true);
                lightingToggle->setEnabled(true);
                centeredDiffButton->setEnabled(true);
                sobelButton->setEnabled(true);
                rendererTypesComboBox->setCurrentIndex(0);
                num3DSlicesLabel->setEnabled(false);
                num3DSlices->setEnabled(false);
                resampleTargetLabel->setEnabled(true);
                resampleTarget->setEnabled(true);
                resampleTargetSlider->setEnabled(true);
                samplesPerRayLabel->setEnabled(false);
                samplesPerRay->setEnabled(false);
                rasterizationButton->setEnabled(false);
                kernelButton->setEnabled(false);
#ifdef HAVE_LIBSLIVR
                rendererSamplesLabel->setEnabled(false);
                rendererSamplesSlider->setEnabled(false);
                rendererSamples->setEnabled(false);
                
#endif
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::Texture3D)
            {
                colorWidgetGroup->show();
                opacityWidgetGroup->show();
                colorWidgetGroup->setEnabled(true);
                opacityWidgetGroup->setEnabled(true);
                lightingToggle->setEnabled(true);
                centeredDiffButton->setEnabled(true);
                sobelButton->setEnabled(true);
                rendererTypesComboBox->setCurrentIndex(1);
                num3DSlicesLabel->setEnabled(true);
                num3DSlices->setEnabled(true);
                resampleTargetLabel->setEnabled(true);
                resampleTarget->setEnabled(true);
                resampleTargetSlider->setEnabled(true);
                samplesPerRayLabel->setEnabled(false);
                samplesPerRay->setEnabled(false);
                rasterizationButton->setEnabled(false);
                kernelButton->setEnabled(false);
#ifdef HAVE_LIBSLIVR
                rendererSamplesLabel->setEnabled(false);
                rendererSamplesSlider->setEnabled(false);
                rendererSamples->setEnabled(false);
#endif
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::RayCasting)
            {
                colorWidgetGroup->show();
                opacityWidgetGroup->show();
                colorWidgetGroup->setEnabled(true);
                opacityWidgetGroup->setEnabled(true);
                lightingToggle->setEnabled(true);
                centeredDiffButton->setEnabled(true);
                sobelButton->setEnabled(true);
                rendererTypesComboBox->setCurrentIndex(2);
                num3DSlicesLabel->setEnabled(false);
                num3DSlices->setEnabled(false);
                resampleTargetLabel->setEnabled(false);
                resampleTarget->setEnabled(false);
                resampleTargetSlider->setEnabled(false);
                samplesPerRayLabel->setEnabled(true);
                samplesPerRay->setEnabled(true);
                rasterizationButton->setEnabled(true);
                kernelButton->setEnabled(true);
#ifdef HAVE_LIBSLIVR
                rendererSamplesLabel->setEnabled(false);
                rendererSamplesSlider->setEnabled(false);
                rendererSamples->setEnabled(false);
#endif
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::RayCastingIntegration)
            {
                colorWidgetGroup->show();
                opacityWidgetGroup->show();
                colorWidgetGroup->setEnabled(false);
                opacityWidgetGroup->setEnabled(false);
                lightingToggle->setEnabled(false);
                centeredDiffButton->setEnabled(false);
                sobelButton->setEnabled(false);
                rendererTypesComboBox->setCurrentIndex(3);
                num3DSlicesLabel->setEnabled(false);
                num3DSlices->setEnabled(false);
                resampleTargetLabel->setEnabled(false);
                resampleTarget->setEnabled(false);
                resampleTargetSlider->setEnabled(false);
                samplesPerRayLabel->setEnabled(true);
                samplesPerRay->setEnabled(true);
                rasterizationButton->setEnabled(true);
                kernelButton->setEnabled(true);
#ifdef HAVE_LIBSLIVR
                rendererSamplesLabel->setEnabled(false);
                rendererSamplesSlider->setEnabled(false);
                rendererSamples->setEnabled(false);
#endif
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::Tuvok)
            {
                colorWidgetGroup->show();
                opacityWidgetGroup->show();
                colorWidgetGroup->setEnabled(true);
                opacityWidgetGroup->setEnabled(true);
                lightingToggle->setEnabled(true);
                centeredDiffButton->setEnabled(true);
                sobelButton->setEnabled(true);
                rendererTypesComboBox->setCurrentIndex(4);
                num3DSlicesLabel->setEnabled(true);
                num3DSlices->setEnabled(true);
                resampleTargetLabel->setEnabled(true);
                resampleTarget->setEnabled(true);
                resampleTargetSlider->setEnabled(true);
                samplesPerRayLabel->setEnabled(false);
                samplesPerRay->setEnabled(false);
                rasterizationButton->setEnabled(false);
                kernelButton->setEnabled(false);
#ifdef HAVE_LIBSLIVR
                rendererSamplesLabel->setEnabled(false);
                rendererSamplesSlider->setEnabled(false);
                rendererSamples->setEnabled(false);
#endif
            }
            else if (volumeAtts->GetRendererType() == VolumeAttributes::SLIVR)
            {
                lightingToggle->setEnabled(true);
                centeredDiffButton->setEnabled(true);
                sobelButton->setEnabled(true);
#ifdef HAVE_LIBSLIVR
                rendererTypesComboBox->setCurrentIndex(5);
                num3DSlicesLabel->setEnabled(false);
                num3DSlices->setEnabled(false);
                rendererSamplesLabel->setEnabled(true);
                rendererSamplesSlider->setEnabled(true);
                rendererSamples->setEnabled(true);
#else
                // Revert to 3D texturing
                rendererTypesComboBox->setCurrentIndex(1);
                num3DSlicesLabel->setEnabled(true);
                num3DSlices->setEnabled(true);
#endif
                resampleTargetLabel->setEnabled(true);
                resampleTarget->setEnabled(true);
                resampleTargetSlider->setEnabled(true);
                samplesPerRayLabel->setEnabled(false);
                samplesPerRay->setEnabled(false);
                rasterizationButton->setEnabled(false);
                kernelButton->setEnabled(false);
            }
#ifdef HAVE_LIBSLIVR
            // Just for now, disable the opacity variable if we are using the
            // SLIVR renderer -- until I figure out how to do color and opacity
            // using separate variables.
            opacityVariable->setEnabled(volumeAtts->GetRendererType() != VolumeAttributes::SLIVR);
            // Disable the 1D/2D transfer function buttons if the renderer is not SLIVR.
            oneDimButton->setEnabled(volumeAtts->GetRendererType() == VolumeAttributes::SLIVR);
            twoDimButton->setEnabled(volumeAtts->GetRendererType() == VolumeAttributes::SLIVR);
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
            temp.sprintf("%d", volumeAtts->GetNum3DSlices());
            num3DSlices->setText(temp);
            break;
        case VolumeAttributes::ID_scaling:
            scaling->setCurrentIndex(volumeAtts->GetScaling());
            skewLineEdit->setEnabled(volumeAtts->GetScaling() ==
                VolumeAttributes::Skew);
            skewLabel->setEnabled(volumeAtts->GetScaling() ==
                VolumeAttributes::Skew);
            break;
        case VolumeAttributes::ID_skewFactor:
            temp.setNum(volumeAtts->GetSkewFactor());
            skewLineEdit->setText(temp);
            break;
        case VolumeAttributes::ID_sampling:
            samplingButtonGroup->blockSignals(true);
            if (volumeAtts->GetSampling() == VolumeAttributes::Rasterization)
                samplingButtonGroup->button(0)->setChecked(true);
            else
                samplingButtonGroup->button(1)->setChecked(true);
            samplingButtonGroup->blockSignals(false);
            break;
        case VolumeAttributes::ID_rendererSamples:
#ifdef HAVE_LIBSLIVR
            temp.sprintf("%1.3f", volumeAtts->GetRendererSamples());
            rendererSamples->setText(temp);
            SetRendererSamplesSliderFromAtts();
#endif
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

            // Enable transfer function widgets based on the dimensionality of
            // the transfer function.
            tfParent1D->setEnabled(volumeAtts->GetTransferFunctionDim()==1);
            tfParent2D->setEnabled(volumeAtts->GetTransferFunctionDim()==2);
#endif
            break;
        }
    }

    // Update the opacity min, max toggle buttons.
    bool notDefaultVar = (volumeAtts->GetOpacityVariable() != "default");
    opacityMinToggle->setEnabled(notDefaultVar);
    opacityMaxToggle->setEnabled(notDefaultVar);
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
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateColorControlPoints()
{
    const ColorControlPointList &cpts = volumeAtts->GetColorControlPoints();
    int   i;

    // Set the smoothing checkbox's checked state.
    smoothCheckBox->blockSignals(true);
    smoothCheckBox->setChecked(volumeAtts->GetSmoothingFlag());
    smoothCheckBox->blockSignals(false);

    // Set the equal checkbox's checked state.
    equalCheckBox->blockSignals(true);
    equalCheckBox->setChecked(volumeAtts->GetEqualSpacingFlag());
    equalCheckBox->blockSignals(false);

    spectrumBar->setSuppressUpdates(true);
    spectrumBar->blockSignals(true);

    // Set the spectrum bar's equal and smoothing flags.
    spectrumBar->setSmoothing(volumeAtts->GetSmoothingFlag());
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
// ****************************************************************************

void
QvisVolumePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
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
        cpts.SetSmoothingFlag(spectrumBar->smoothing());
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
        int val;
        if(LineEditGetInt(resampleTarget, val))
            volumeAtts->SetResampleTarget(val);
        else
        {
            ResettingError("resample target", IntToQString(
                volumeAtts->GetResampleTarget()));
            volumeAtts->SetResampleTarget(volumeAtts->GetResampleTarget());
        }
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
        int val;
        if(LineEditGetInt(samplesPerRay, val))
            volumeAtts->SetSamplesPerRay(val);
        else
        {
            msg = tr("The number of samples per ray was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetSamplesPerRay());
            Message(msg);
        }
    }

    // Get the number of slices for 3D texturing.
    if(which_widget == VolumeAttributes::ID_num3DSlices || doAll)
    {
        int val;
        if(LineEditGetInt(num3DSlices, val))
            volumeAtts->SetNum3DSlices(val);
        else
        {
            msg = tr("The number of 3d slices was invalid."
                     "Resetting to the last good value of %1.").
                  arg(volumeAtts->GetNum3DSlices());
            Message(msg);
        }
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
#ifdef HAVE_LIBSLIVR
    if(which_widget == VolumeAttributes::ID_rendererSamples || doAll)
    {
        float val;
        bool okay = false;
        if(LineEditGetFloat(rendererSamples, val))
        {
            if(val >= 1.f && val <= MAX_RENDERER_SAMPLE_VALUE)
            {
                okay = true;
                volumeAtts->SetRendererSamples(val);
            }
        }

        if(!okay)
        {
            QString value; value.sprintf("%1.23f",MAX_RENDERER_SAMPLE_VALUE);
            msg = tr("The value of renderer samples was invalid. Valid values are in the range [1.,%1]. "
                     "Resetting to the last good value of %2.").
                  arg(value).arg(volumeAtts->GetRendererSamples());
            Message(msg);
            volumeAtts->SetRendererSamples(volumeAtts->GetRendererSamples());
        }
    }
#endif
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
// Method:  QvisVolumePlotWindow::SetResampleTargetSliderFromAtts
//
// Purpose:
//   Find an appropriate position for the resample slider, and set it.
//
// Programmer:  Jeremy Meredith
// Creation:    November 13, 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 14:16:29 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVolumePlotWindow::SetResampleTargetSliderFromAtts()
{
    float val = volumeAtts->GetResampleTarget();
    float exp = log(val)/log(10.);
    int   pos = int((exp-3.f)*80.f/4.f);
    pos = qMin(qMax(0, pos), 80);
    resampleTargetSlider->blockSignals(true);
    resampleTargetSlider->setValue(pos);
    resampleTargetSlider->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::SetRendererSamplesSliderFromAtts
//
// Purpose: 
//   Sets the renderer samples from the atts.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 11 16:03:27 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::SetRendererSamplesSliderFromAtts()
{
    float t = (volumeAtts->GetRendererSamples() - 1.f) / (MAX_RENDERER_SAMPLE_VALUE - 1.f);
    if(t < 0.f) t = 0.f;
    if(t > 1.f) t = 1.f;
    int pos = int(t * 1000.f);
    rendererSamplesSlider->blockSignals(true);
    rendererSamplesSlider->setValue(pos);
    rendererSamplesSlider->blockSignals(false);
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
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVolumePlotWindow::colorMinToggled
//
// Purpose: 
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
// Method: QvisVolumePlotWindow::smoothToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's smooth
//   toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 15:38:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::smoothToggled(bool val)
{
    GetCurrentValues(0);
    volumeAtts->SetSmoothingFlag(val);
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
QvisVolumePlotWindow::samplesPerRayProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_samplesPerRay);
    SetUpdate(false);
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
QvisVolumePlotWindow::resampleTargetProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_resampleTarget);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::resampleTargetSliderChanged
//
// Purpose:
//   Qt slot function, called when resampleTargetSlider is changed.
//
// Arguments:
//   val        the new slider position (0..80)
//
// Programmer:  Jeremy Meredith
// Creation:    November 13, 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:55:56 PST 2002
//   Added a little code to set the resample value into the line edit. We
//   normally don't set widget state outside the UpdateWindow method but
//   this is an exception. This fixes auto update.
//
//   Jeremy Meredith, Fri Mar 19 15:02:54 PST 2004
//   I removed the call to Apply from this function, and put it in a new
//   callback for when the slider is released.  We don't want to notify
//   the viewer while it is being changed, although we do want to display
//   the target value.
//
// ****************************************************************************

void
QvisVolumePlotWindow::resampleTargetSliderChanged(int val)
{
    float exp = float(val)*4./80. + 3;
    float nf  = pow(10.f, exp);
    float rnd = pow(10.f, int(log(nf)/log(10.) - 1.));
    int   ni  = int(rnd)*int(nf/rnd + .5);
    volumeAtts->SetResampleTarget(ni);

    // Set the value in the line edit.
    QString tmp;
    tmp.sprintf("%d", ni);
    resampleTarget->setText(tmp);
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::resampleTargetSliderReleased
//
// Purpose:
//   Qt slot function, called when resampleTargetSlider is released.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    March 19, 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::resampleTargetSliderReleased()
{
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
        volumeAtts->SetRendererType(VolumeAttributes::Tuvok);
        break;
      case 5:
#ifdef HAVE_LIBSLIVR
        volumeAtts->SetRendererType(VolumeAttributes::SLIVR);
#else
        Warning("SLIVR is not available. VisIt will revert to 3D texturing.");
        volumeAtts->SetRendererType(VolumeAttributes::Texture3D);
#endif
        break;
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
QvisVolumePlotWindow::num3DSlicesProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_num3DSlices);
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
QvisVolumePlotWindow::rendererSamplesProcessText()
{
    GetCurrentValues(VolumeAttributes::ID_rendererSamples);
    Apply();
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::rendererSamplesSliderChanged
//
// Purpose:
//   Qt slot function, called when rendererSamplesSlider is changed.
//
// Arguments:
//   val        the new slider position (0..1000)
//
// Programmer:  Brad Whitlock
// Creation:    Fri Jan 11 16:07:29 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::rendererSamplesSliderChanged(int val)
{
    float t = float(val) / 1000.f;
    float value = (1.f-t)*1 + t*MAX_RENDERER_SAMPLE_VALUE;
    volumeAtts->SetRendererSamples(value);

    // Set the value in the line edit.
    QString tmp;
    tmp.sprintf("%1.3f", value);
    rendererSamples->setText(tmp);
}

// ****************************************************************************
// Method:  QvisVolumePlotWindow::rendererSamplesSliderReleased
//
// Purpose:
//   Qt slot function, called when rendererSamplesSlider is released.
//
// Arguments:
//   none
//
// Programmer:  Brad Whitlock
// Creation:    Fri Jan 11 16:07:29 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::rendererSamplesSliderReleased()
{
    SetUpdate(false);
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
