#include <QvisVolumePlotWindow.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qslider.h>
#include <math.h>
#include <stdio.h>

#include <QvisOpacitySlider.h>
#include <QvisSpectrumBar.h>
#include <QvisColorSelectionWidget.h>
#include <QvisGaussianOpacityBar.h>
#include <QvisScribbleOpacityBar.h>
#include <VolumeAttributes.h>
#include <ColorControlPoint.h>
#include <GaussianControlPoint.h>
#include <ViewerProxy.h>
#include <ImproperUseException.h>

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
// ****************************************************************************

QvisVolumePlotWindow::QvisVolumePlotWindow(const int type,
    VolumeAttributes *volumeAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(volumeAtts_, caption, shortName, notepad)
{
    plotType    = type;
    volumeAtts = volumeAtts_;

    colorCycle = 1;

    // Initialize parentless widgets.
    modeButtonGroup = 0;
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
// ****************************************************************************

QvisVolumePlotWindow::~QvisVolumePlotWindow()
{
    volumeAtts = 0;

    // Delete parentless widgets.
    delete modeButtonGroup;
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
//   Jeremy Meredith, Tue Nov 13 11:41:51 PST 2001
//   Added a resample target and slider, and an opacity variable.
//
//   Brad Whitlock, Thu Feb 14 14:25:13 PST 2002
//   Modified window layout.
//
//   Brad Whitlock, Mon Mar 4 15:52:59 PST 2002
//   Connected a signal to the scribble opacity widget.
//
//   Jeremy Meredith, Thu Oct  2 13:10:53 PDT 2003
//   Added settings for the renderer type, the gradient method, and
//   the number of 3D textured slices.
//
//   Jeremy Meredith, Fri Mar 19 15:04:16 PST 2004
//   I added a new callback for when the resample target slider
//   is released.
//
//   Hank Childs, Mon Nov 22 09:28:33 PST 2004
//   Replace "Software" button with "Ray Trace" toggle.
//
// ****************************************************************************

void
QvisVolumePlotWindow::CreateWindowContents()
{
    // Figure out the max width that we want to allow for some simple
    // line edit widgets.
    int maxWidth = fontMetrics().width("1.0000000000");

    // Add the group box that will contain the color-related widgets.
    colorWidgetGroup = new QGroupBox(central, "colorWidgetGroup");
    colorWidgetGroup->setTitle("Color");
    topLayout->addWidget(colorWidgetGroup);
    QVBoxLayout *innerColorLayout = new QVBoxLayout(colorWidgetGroup);
    innerColorLayout->setMargin(5);
    innerColorLayout->addSpacing(15);

    // Create the spectrum bar.
    spectrumBar = new QvisSpectrumBar(colorWidgetGroup, "spectrumBar");
    connect(spectrumBar, SIGNAL(controlPointMoved(int,float)),
            this, SLOT(controlPointMoved(int,float)));

    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *seLayout = new QHBoxLayout(innerColorLayout);
    seLayout->setSpacing(5);
    seLayout->addSpacing(5);
    addPointButton = new QPushButton("+", colorWidgetGroup, "addPointButton");
    connect(addPointButton, SIGNAL(clicked()), this, SLOT(addControlPoint()));
    seLayout->addWidget(addPointButton);

    rmPointButton = new QPushButton("-", colorWidgetGroup, "rmPointButton");
    connect(rmPointButton, SIGNAL(clicked()),
            this, SLOT(removeControlPoint()));
    seLayout->addWidget(rmPointButton);

    alignPointButton = new QPushButton("Align", colorWidgetGroup, "alignPointButton");
    connect(alignPointButton, SIGNAL(clicked()),
            this, SLOT(alignControlPoints()));
    seLayout->addWidget(alignPointButton);
    seLayout->addSpacing(5);
    seLayout->addStretch(10);

    smoothCheckBox = new QCheckBox("Smooth", colorWidgetGroup, "smoothCheckbox");
    smoothCheckBox->setChecked(true);
    connect(smoothCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(smoothToggled(bool)));
    seLayout->addWidget(smoothCheckBox);

    equalCheckBox = new QCheckBox("Equal", colorWidgetGroup, "equalCheckbox");
    connect(equalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(equalSpacingToggled(bool)));
    seLayout->addWidget(equalCheckBox);
    topLayout->addSpacing(5);

    // Add the spectrum bar to the window.
    connect(spectrumBar, SIGNAL(selectColor(int)),
            this, SLOT(popupColorSelect(int)));
    spectrumBar->resize(spectrumBar->width(), 60);
    innerColorLayout->addWidget(spectrumBar, 100);
    innerColorLayout->addSpacing(10);

    // Create the color min widgets.
    QHBoxLayout *colorMinMaxLayout = new QHBoxLayout(innerColorLayout);
    colorMinMaxLayout->setSpacing(5);
    colorMinMaxLayout->setMargin(5);
    colorMinToggle = new QCheckBox("Min", colorWidgetGroup,
                                   "colorMinToggle");
    connect(colorMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMinToggled(bool)));
    colorMinMaxLayout->addWidget(colorMinToggle);
    colorMin = new QLineEdit(colorWidgetGroup, "colorMin");
    colorMin->setMaximumWidth(maxWidth);
    colorMin->setEnabled(volumeAtts->GetUseColorVarMin());
    connect(colorMin, SIGNAL(returnPressed()),
            this, SLOT(colorMinProcessText()));
    colorMinMaxLayout->addWidget(colorMin);

    // Create the color max widgets.
    colorMaxToggle = new QCheckBox("Max", colorWidgetGroup,
                                   "colorMaxToggle");
    connect(colorMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorMaxToggled(bool)));
    colorMinMaxLayout->addWidget(colorMaxToggle);

    colorMax = new QLineEdit(colorWidgetGroup, "colorMax");
    colorMax->setMaximumWidth(maxWidth);
    colorMax->setEnabled(volumeAtts->GetUseColorVarMax());
    connect(colorMax, SIGNAL(returnPressed()),
            this, SLOT(colorMaxProcessText()));
    colorMinMaxLayout->addWidget(colorMax);

    // Add the group box that will contain the color-related widgets.
    opacityWidgetGroup = new QGroupBox(central, "opacityWidgetGroup");
    opacityWidgetGroup->setTitle("Opacity");
    topLayout->addWidget(opacityWidgetGroup, 100);
    QVBoxLayout *innerOpacityLayout = new QVBoxLayout(opacityWidgetGroup);
    innerOpacityLayout->setMargin(10);
    innerOpacityLayout->addSpacing(10);

    // Create the buttons that control what mode the opacity widget it in.
    QHBoxLayout *opLayout = new QHBoxLayout(innerOpacityLayout);
    opLayout->addStretch(100);
    opLayout->setSpacing(5);
    QLabel *interactionModeLabel = new QLabel("Interaction mode",
        opacityWidgetGroup, "interactionModeLabel");
    opLayout->addWidget(interactionModeLabel);
    
    // Create the interaction mode button group.
    modeButtonGroup = new QButtonGroup(0, "modeButtonGroup");
    connect(modeButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(interactionModeChanged(int)));
    QRadioButton *rb= new QRadioButton("Freeform", opacityWidgetGroup);
    modeButtonGroup->insert(rb, 0);
    opLayout->addWidget(rb, 5, 0);
    rb = new QRadioButton("Gaussian", opacityWidgetGroup);
    rb->setChecked(true);
    modeButtonGroup->insert(rb, 1);
    opLayout->addWidget(rb, 6, 0);

    // Create the gaussian opacity editor widget.
    alphaWidget = new QvisGaussianOpacityBar(opacityWidgetGroup, "alphaWidget");
    alphaWidget->setMinimumHeight(80);
    connect(alphaWidget, SIGNAL(mouseReleased()),
            this, SLOT(alphaValuesChanged()));
    innerOpacityLayout->addWidget(alphaWidget, 100);

    // Create the scribble opacity editor widget.
    scribbleAlphaWidget = new QvisScribbleOpacityBar(opacityWidgetGroup, "scribbleWidget");
    scribbleAlphaWidget->setMinimumHeight(80);
    scribbleAlphaWidget->hide();
    connect(scribbleAlphaWidget, SIGNAL(opacitiesChanged()),
            this, SLOT(alphaValuesChanged()));
    connect(scribbleAlphaWidget, SIGNAL(mouseReleased()),
            this, SLOT(alphaValuesChanged()));
    innerOpacityLayout->addWidget(scribbleAlphaWidget, 100);

    // Create some style pixmaps
    QPixmap blackPixmap(black_xpm);
    QPixmap rampPixmap(ramp_xpm);
    QPixmap whitePixmap(white_xpm);

    QHBoxLayout *abLayout = new QHBoxLayout(innerOpacityLayout);
    abLayout->setSpacing(5);
    abLayout->addStretch(10);
    zeroButton = new QPushButton(opacityWidgetGroup, "zeroButton");
    zeroButton->setPixmap(blackPixmap);
    connect(zeroButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTotallyZero()));
    abLayout->addWidget(zeroButton);

    rampButton = new QPushButton(opacityWidgetGroup, "rampButton");
    rampButton->setPixmap(rampPixmap);
    connect(rampButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeLinearRamp()));
    abLayout->addWidget(rampButton);

    oneButton = new QPushButton(opacityWidgetGroup, "oneButton");
    oneButton->setPixmap(whitePixmap);
    connect(oneButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(makeTotallyOne()));
    abLayout->addWidget(oneButton);

    smoothButton = new QPushButton("Smooth", opacityWidgetGroup, "smoothButton");
    connect(smoothButton, SIGNAL(clicked()), scribbleAlphaWidget, SLOT(smoothCurve()));
    abLayout->addWidget(smoothButton);
    abLayout->addStretch(10);
    innerOpacityLayout->addSpacing(10);

    // Create the opacity attenuation widgets.
    QGridLayout *attLayout = new QGridLayout(innerOpacityLayout, 2, 2);
    attLayout->setSpacing(5);
    attenuationSlider = new QvisOpacitySlider(0, 255, 10, 255, opacityWidgetGroup, "attenuationSlider");
    attenuationSlider->setGradientColor(QColor(0,0,0));
    QLabel *attenuationLabel = new QLabel(attenuationSlider, "Attenuation", 
                                          opacityWidgetGroup, "attenuationSliderLabel");
    connect(attenuationSlider, SIGNAL(valueChanged(int)),
            this, SLOT(attenuationChanged(int)));
    attLayout->addWidget(attenuationLabel, 0, 0);
    attLayout->addWidget(attenuationSlider, 0, 1);

    // Create the opacity variable
    opacityVariable = new QLineEdit(opacityWidgetGroup, "opacityVariable");
    connect(opacityVariable, SIGNAL(returnPressed()),
            this, SLOT(opacityVariableProcessText()));
    QLabel *opacityVarLabel = new QLabel(opacityVariable, "Opacity variable",
        opacityWidgetGroup, "opacityVarLabel");
    attLayout->addWidget(opacityVarLabel, 1, 0);
    attLayout->addWidget(opacityVariable, 1, 1);
    innerOpacityLayout->addSpacing(10);

    // Create the opacity min widgets.
    QHBoxLayout *opacityMinMaxLayout = new QHBoxLayout(innerOpacityLayout);
    opacityMinMaxLayout->setSpacing(5);
    opacityMinToggle = new QCheckBox("Min", opacityWidgetGroup,
                                   "opacityMinToggle");
    connect(opacityMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMinToggled(bool)));
    opacityMinMaxLayout->addWidget(opacityMinToggle);
    opacityMin = new QLineEdit(opacityWidgetGroup, "opacityMin");
    opacityMin->setMaximumWidth(maxWidth);
    opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
    connect(opacityMin, SIGNAL(returnPressed()),
            this, SLOT(opacityMinProcessText()));
    opacityMinMaxLayout->addWidget(opacityMin);

    // Create the opacity max widgets.
    opacityMaxToggle = new QCheckBox("Max", opacityWidgetGroup,
                                     "opacityMaxToggle");
    connect(opacityMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(opacityMaxToggled(bool)));
    opacityMinMaxLayout->addWidget(opacityMaxToggle);

    opacityMax = new QLineEdit(opacityWidgetGroup, "opacityMax");
    opacityMax->setMaximumWidth(maxWidth);
    opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
    connect(opacityMax, SIGNAL(returnPressed()),
            this, SLOT(opacityMaxProcessText()));
    opacityMinMaxLayout->addWidget(opacityMax);

    // Create the resample target value
    QGridLayout *resampleAndOpacityLayout = new QGridLayout(topLayout, 4,3, 10, "resampleAndOpacityLayout");
    resampleTarget = new QLineEdit(central, "resampleTarget");
    connect(resampleTarget, SIGNAL(returnPressed()),
            this, SLOT(resampleTargetProcessText()));
    QLabel *resampleLabel = new QLabel(resampleTarget, "Number of samples",
                                       central, "resampleLabel");
    resampleTargetSlider = new QSlider(0,80,10,34, Qt::Horizontal,central,"resampleTargetSlider");
    connect(resampleTargetSlider, SIGNAL(valueChanged(int)),
            this, SLOT(resampleTargetSliderChanged(int)));
    connect(resampleTargetSlider, SIGNAL(sliderReleased()),
            this, SLOT(resampleTargetSliderReleased()));
    resampleAndOpacityLayout->addWidget(resampleLabel, 0, 0);
    resampleAndOpacityLayout->addWidget(resampleTarget, 0, 1);
    resampleAndOpacityLayout->addWidget(resampleTargetSlider, 0, 2);

    // Create the number of 3D slices.
    num3DSlices = new QLineEdit(central, "num3DSlices");
    connect(num3DSlices, SIGNAL(returnPressed()), this,
            SLOT(num3DSlicesProcessText()));
    QLabel *num3DSlicesLabel = new QLabel(num3DSlices, "Number of slices",
                                            central, "num3DSlicesLabel");
    resampleAndOpacityLayout->addWidget(num3DSlicesLabel, 1, 0);
    resampleAndOpacityLayout->addWidget(num3DSlices, 1, 1);

    // Create the number of samples per ray.
    samplesPerRay = new QLineEdit(central, "samplesPerRay");
    connect(samplesPerRay, SIGNAL(returnPressed()), this,
            SLOT(samplesPerRayProcessText()));
    QLabel *samplesPerRayLabel = new QLabel(samplesPerRay, "Samples per ray",
                                            central, "samplesPerRayLabel");
    resampleAndOpacityLayout->addWidget(samplesPerRayLabel, 2, 0);
    resampleAndOpacityLayout->addWidget(samplesPerRay, 2, 1);

    // Create the rendering method radio buttons.
    QHBoxLayout *rendererLayout = new QHBoxLayout(topLayout);
    rendererLayout->addWidget(new QLabel("Rendering method", central));
    rendererButtonGroup = new QButtonGroup(0, "rendererButtonGroup");
    connect(rendererButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(rendererTypeChanged(int)));
    rb = new QRadioButton("Splatting", central);
    rendererButtonGroup->insert(rb, 0);
    rendererLayout->addWidget(rb);
    rb = new QRadioButton("3D Texturing", central);
    rendererButtonGroup->insert(rb, 1);
    rendererLayout->addWidget(rb);

    // Create the gradient method radio buttons.
    QHBoxLayout *gradientLayout = new QHBoxLayout(topLayout);
    gradientLayout->addWidget(new QLabel("Gradient method", central));
    gradientButtonGroup = new QButtonGroup(0, "gradientButtonGroup");
    connect(gradientButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(gradientTypeChanged(int)));
    rb = new QRadioButton("Centered diff", central);
    gradientButtonGroup->insert(rb, 0);
    gradientLayout->addWidget(rb);
    rb = new QRadioButton("Sobel", central);
    gradientButtonGroup->insert(rb, 1);
    gradientLayout->addWidget(rb);

    // Create the legend toggle.
    QHBoxLayout *toggleLayout = new QHBoxLayout(topLayout);
    toggleLayout->setSpacing(10);
    legendToggle = new QCheckBox("Legend", central, "legendToggle");
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    toggleLayout->addWidget(legendToggle);

    // Create the lighting toggle.
    lightingToggle = new QCheckBox("Lighting", central, "lightingToggle");
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    toggleLayout->addWidget(lightingToggle);

    // Create the smooth data toggle.
    smoothDataToggle = new QCheckBox("Smooth Data", central, "smoothToggle");
    connect(smoothDataToggle, SIGNAL(toggled(bool)),
            this, SLOT(smoothDataToggled(bool)));
    toggleLayout->addWidget(smoothDataToggle);

    // Create the software toggle.
    QHBoxLayout *softwareButtonLayout = new QHBoxLayout(topLayout);
    softwareToggle = new QCheckBox("Ray Trace", central, "softwareToggle");
    connect(softwareToggle, SIGNAL(toggled(bool)),
            this, SLOT(softwareToggled(bool)));
    softwareButtonLayout->addWidget(softwareToggle);
    softwareButtonLayout->addStretch(10);

    // Create the color selection widget.
    colorSelect = new QvisColorSelectionWidget(this, "colorSelect", WType_Popup);
    connect(colorSelect, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedColor(const QColor &)));
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
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
        case 0: // legendFlag
            legendToggle->blockSignals(true);
            legendToggle->setChecked(volumeAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case 1: // lightingFlag
            lightingToggle->blockSignals(true);
            lightingToggle->setChecked(volumeAtts->GetLightingFlag());
            lightingToggle->blockSignals(false);
            break;
        case 2: // colorControlPoints
            UpdateColorControlPoints();
            break;
        case 3: // opacityAttenuation
            attenuationSlider->blockSignals(true);
            attenuationSlider->setValue(volumeAtts->GetOpacityAttenuation() * 255);
            attenuationSlider->blockSignals(false);
            break;
        case 4: // freeformFlag
            modeButtonGroup->blockSignals(true);
            modeButtonGroup->setButton(volumeAtts->GetFreeformFlag()?0:1);
            modeButtonGroup->blockSignals(false);

            if(volumeAtts->GetFreeformFlag())
            {
                alphaWidget->hide();
                scribbleAlphaWidget->show();
            }
            else
            {
                scribbleAlphaWidget->hide();
                alphaWidget->show();
            }
            break;
        case 5: // opacityControlPoints
            UpdateGaussianControlPoints();
            break;
        case 6: // resampleTarget
            temp.sprintf("%d", volumeAtts->GetResampleTarget());
            resampleTarget->setText(temp);
            SetResampleTargetSliderFromAtts();
            break;
        case 7: // opacityVariable
            temp = volumeAtts->GetOpacityVariable().c_str();
            opacityVariable->setText(temp);
            break;
        case 8: // freeformOpacity
            UpdateFreeform();
            break;
        case 9: // doSoftware
            softwareToggle->blockSignals(true);
            softwareToggle->setChecked(volumeAtts->GetDoSoftware());
            softwareToggle->blockSignals(false);
            break;
        case 10: // useColorVarMin
            colorMinToggle->blockSignals(true);
            colorMinToggle->setChecked(volumeAtts->GetUseColorVarMin());
            colorMinToggle->blockSignals(false);
            colorMin->setEnabled(volumeAtts->GetUseColorVarMin());
            break;
        case 11: // colorVarMin
            temp.setNum(volumeAtts->GetColorVarMin());
            colorMin->setText(temp);
            break;
        case 12: // useColorVarMax
            colorMaxToggle->blockSignals(true);
            colorMaxToggle->setChecked(volumeAtts->GetUseColorVarMax());
            colorMaxToggle->blockSignals(false);
            colorMax->setEnabled(volumeAtts->GetUseColorVarMax());
            break;
        case 13: // colorVarMax
            temp.setNum(volumeAtts->GetColorVarMax());
            colorMax->setText(temp);
            break;
        case 14: // useOpacityVarMin
            opacityMinToggle->blockSignals(true);
            opacityMinToggle->setChecked(volumeAtts->GetUseOpacityVarMin());
            opacityMinToggle->blockSignals(false);
            opacityMin->setEnabled(volumeAtts->GetUseOpacityVarMin());
            break;
        case 15: // opacityVarMin
            temp.setNum(volumeAtts->GetOpacityVarMin());
            opacityMin->setText(temp);
            break;
        case 16: // useOpacityVarMax
            opacityMaxToggle->blockSignals(true);
            opacityMaxToggle->setChecked(volumeAtts->GetUseOpacityVarMax());
            opacityMaxToggle->blockSignals(false);
            opacityMax->setEnabled(volumeAtts->GetUseOpacityVarMax());
            break;
        case 17: // opacityVarMax
            temp.setNum(volumeAtts->GetOpacityVarMax());
            opacityMax->setText(temp);
            break;
        case 18: // smoothData
            smoothDataToggle->blockSignals(true);
            smoothDataToggle->setChecked(volumeAtts->GetSmoothData());
            smoothDataToggle->blockSignals(false);
        case 19: // samplesPerRay
            temp.sprintf("%d", volumeAtts->GetSamplesPerRay());
            samplesPerRay->setText(temp);
            break;
        case 20: // rendererType
            if (volumeAtts->GetRendererType() == VolumeAttributes::Splatting)
            {
                rendererButtonGroup->setButton(0);
                num3DSlices->setEnabled(false);
            }
            else
            {
                rendererButtonGroup->setButton(1);
                num3DSlices->setEnabled(true);
            }
            break;
        case 21: // gradientType
            if (volumeAtts->GetGradientType() == VolumeAttributes::CenteredDifferences)
                gradientButtonGroup->setButton(0);
            else
                gradientButtonGroup->setButton(1);
            break;
        case 22:
            temp.sprintf("%d", volumeAtts->GetNum3DSlices());
            num3DSlices->setText(temp);
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

    if(spectrumBar->numControlPoints() == cpts.GetNumColorControlPoints())
    {
        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }
    }
    else if(spectrumBar->numControlPoints() < cpts.GetNumColorControlPoints())
    {
        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }

        // We need to add control points.
        for(i = spectrumBar->numControlPoints(); i < cpts.GetNumColorControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2]);
            spectrumBar->addControlPoint(ctmp, cpts[i].GetPosition());
        }
    }
    else
    {
        // We need to remove control points.
        for(i = spectrumBar->numControlPoints()-1; i >= cpts.GetNumColorControlPoints(); --i)
            spectrumBar->removeControlPoint();

        // Set the control points' colors and positions
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor ctmp((int)cpts[i].GetColors()[0],
                        (int)cpts[i].GetColors()[1],
                        (int)cpts[i].GetColors()[2]);
            spectrumBar->setControlPointColor(i, ctmp);
            spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
        }
    }

    spectrumBar->blockSignals(false);
    spectrumBar->setSuppressUpdates(false);
    spectrumBar->update();
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
    int n=volumeAtts->GetOpacityControlPoints().GetNumGaussianControlPoints();
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
// ****************************************************************************

void
QvisVolumePlotWindow::UpdateFreeform()
{
    const unsigned char *opacity = volumeAtts->GetFreeformOpacity();

    // Convert the state object's opacities into floats that can be used by
    // the alpha widget.
    float f[256];
    for(int i = 0; i < 256; ++i)
        f[i] = float(opacity[i]) / 255.;

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
        cpts.ClearColorControlPoints();
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor c(spectrumBar->controlPointColor(i));
            float  pos = spectrumBar->controlPointPosition(i);

            ColorControlPoint pt;
            unsigned char ptColors[4];
            ptColors[0] = (unsigned char)c.red();
            ptColors[1] = (unsigned char)c.green();
            ptColors[2] = (unsigned char)c.blue();
            ptColors[3] = 0;
            pt.SetColors(ptColors);
            pt.SetPosition(pos);
            cpts.AddColorControlPoint(pt);
        }
        cpts.SetEqualSpacingFlag(spectrumBar->equalSpacing());
        cpts.SetSmoothingFlag(spectrumBar->smoothing());
        volumeAtts->SetColorControlPoints(cpts);
    }

    // Get the alpha values from the opacity bar and put them into
    // the state object's rgbaColors array.
    if(which_widget == 1 || doAll)
    {
        // Store the opacity
        if(!volumeAtts->GetFreeformFlag())
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
                cpts.AddGaussianControlPoint(gcpt);
            }
            volumeAtts->SetOpacityControlPoints(cpts);
        }
        else
        {
            // Store the freeform opacities.
            float *alphas = scribbleAlphaWidget->getRawOpacities(256);
            unsigned char a[256];
            for(i = 0; i < 256; ++i)
                a[i] = (unsigned char)(alphas[i] * 255.);
            volumeAtts->SetFreeformOpacity(a);
            delete [] alphas;
        }
    }

    // Get the value of the resample target
    if(which_widget == 2 || doAll)
    {
        temp = resampleTarget->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            volumeAtts->SetResampleTarget(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of resampleTarget was invalid. "
                "Resetting to the last good value of %d.",
                volumeAtts->GetResampleTarget());
            Message(msg);
            volumeAtts->SetResampleTarget(volumeAtts->GetResampleTarget());
        }
    }

    // Get the value of the opacity variable
    if(which_widget == 3 || doAll)
    {
        temp = opacityVariable->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            volumeAtts->SetOpacityVariable(temp.latin1());

            // If we're using the default opacity variable, do not use
            // the opacity min, max.
            if(volumeAtts->GetOpacityVariable() == "default")
            {
                volumeAtts->SetUseOpacityVarMin(false);
                volumeAtts->SetUseOpacityVarMax(false);
            }
        }

        if(!okay)
        {
            msg.sprintf("The value of the opacity variable was invalid. "
                "Resetting to the last good value of %s.",
                volumeAtts->GetOpacityVariable().c_str());
            Message(msg);
            volumeAtts->SetOpacityVariable(volumeAtts->GetOpacityVariable());
        }
    }

    // Get the value of the minimum for the color variable.
    if(which_widget == 4 || doAll)
    {
        temp = colorMin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetColorVarMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The minimum value for the color variable was invalid."
                "Resetting to the last good value of %g.",
                volumeAtts->GetColorVarMin());
            Message(msg);
        }
    }

    // Get the value of the maximum for the color variable.
    if(which_widget == 5 || doAll)
    {
        temp = colorMax->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetColorVarMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The maximum value for the color variable was invalid."
                "Resetting to the last good value of %g.",
                volumeAtts->GetColorVarMax());
            Message(msg);
        }
    }

    // Get the value of the minimum for the opacity variable.
    if(which_widget == 6 || doAll)
    {
        temp = opacityMin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetOpacityVarMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The minimum value for the opacity var was invalid."
                "Resetting to the last good value of %g.",
                volumeAtts->GetOpacityVarMin());
            Message(msg);
        }
    }

    // Get the value of the maximum for the opacity variable.
    if(which_widget == 7 || doAll)
    {
        temp = opacityMax->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetOpacityVarMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The maximum value for the opacity var was invalid."
                "Resetting to the last good value of %g.",
                volumeAtts->GetOpacityVarMax());
            Message(msg);
        }
    }

    // Get the number of samples per ray.
    if(which_widget == 8 || doAll)
    {
        temp = samplesPerRay->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetSamplesPerRay(val);
        }

        if(!okay)
        {
            msg.sprintf("The number of samples per ray was invalid."
                "Resetting to the last good value of %i.",
                volumeAtts->GetSamplesPerRay());
            Message(msg);
        }
    }

    // Get the number of slices for 3D texturing.
    if(which_widget == 9 || doAll)
    {
        temp = num3DSlices->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            volumeAtts->SetNum3DSlices(val);
        }

        if(!okay)
        {
            msg.sprintf("The number of 3d slices was invalid."
                "Resetting to the last good value of %i.",
                volumeAtts->GetNum3DSlices());
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
//   
// ****************************************************************************

void
QvisVolumePlotWindow::Apply(bool ignore)
{
    QString temp = opacityVariable->displayText();
    if (temp == "default")
    {
        if (volumeAtts->GetUseColorVarMin() || volumeAtts->GetUseColorVarMax())
        {
            if (!volumeAtts->GetUseOpacityVarMin() &&
                !volumeAtts->GetUseOpacityVarMax())
            {
                Message("The range set for indexing colors will also be "
                        "used when indexing opacities");
            }
        }
    }
                
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        volumeAtts->Notify();

        // Tell the viewer to set the volume plot attributes.
        viewer->SetPlotOptions(plotType);

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
//
// ****************************************************************************

void
QvisVolumePlotWindow::SetResampleTargetSliderFromAtts()
{
    float val = volumeAtts->GetResampleTarget();
    float exp = log(val)/log(10.);
    int   pos = int((exp-3.)*80./4.);
    pos = QMIN(QMAX(0, pos), 80);
    resampleTargetSlider->blockSignals(true);
    resampleTargetSlider->setValue(pos);
    resampleTargetSlider->blockSignals(false);
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
    viewer->SetDefaultPlotOptions(plotType);
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
    viewer->ResetPlotOptions(plotType);
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
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 14:06:35 PST 2001
//
// Modifications:
//   Hank Childs, Wed Dec 12 10:05:48 PST 2001
//   Removed unused variable buttonMiddle.
//
// ****************************************************************************

void
QvisVolumePlotWindow::popupColorSelect(int index)
{
    // Set the popup's initial color.
    colorSelect->blockSignals(true);
    colorSelect->setSelectedColor(spectrumBar->controlPointColor(index));
    colorSelect->blockSignals(false);

    // Figure out a good place to popup the menu.
    int menuW = colorSelect->sizeHint().width();
    int menuH = colorSelect->sizeHint().height();
    int menuX = QCursor::pos().x();
    int menuY = QCursor::pos().y() - (menuH >> 1);

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
// ****************************************************************************

void
QvisVolumePlotWindow::interactionModeChanged(int index)
{
    volumeAtts->SetFreeformFlag(index == 0);
    if(volumeAtts->GetFreeformFlag())
    {
        CopyGaussianOpacitiesToFreeForm();
        UpdateFreeform();
    }
    Apply();
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
// Method: QvisVolumePlotWindow::softwareToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the toggle for the software
//   is clicked.
//
// Programmer: Hank Childs
// Creation:   November 22, 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVolumePlotWindow::softwareToggled(bool)
{
    volumeAtts->SetDoSoftware(!volumeAtts->GetDoSoftware());
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
    GetCurrentValues(4);
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
    GetCurrentValues(5);
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
    GetCurrentValues(6);
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
    GetCurrentValues(7);
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
    GetCurrentValues(8);
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
    GetCurrentValues(2);
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
// Method:  QvisVolumePlotWindow::opacityVariableProcessText
//
// Purpose:
//   Qt slot function, called when opacityVariable is changed.
//
// Programmer:  Jeremy Meredith
// Creation:    November 13, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisVolumePlotWindow::opacityVariableProcessText()
{
    GetCurrentValues(3);
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
    GetCurrentValues(9);
    Apply();
}
