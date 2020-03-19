// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QWidget>
#include <QLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include <QvisPseudocolorPlotWindow.h>
#include <PseudocolorAttributes.h>
#include <ViewerProxy.h>
#include <QvisOpacitySlider.h>
#include <QvisColorTableWidget.h>
#include <QvisPointControl.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
#include <QvisColorButton.h>

#include <QvisCollapsibleFrame.h>

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::QvisPseudocolorPlotWindow
//
// Purpose:
//   Constructor for the QvisPseudocolorPlotWindow class.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:05:42 PST 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 06:59:33 PST 2001
//    I added the argument type.
//
// ****************************************************************************

QvisPseudocolorPlotWindow::QvisPseudocolorPlotWindow(const int type,
    PseudocolorAttributes *_pcAtts, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_pcAtts, caption, shortName, notepad)
{
    plotType = type;
    pcAtts   = _pcAtts;
    centeringButtons = 0;
    scalingButtons = 0;
    smoothingLevelButtons = 0;
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::~QvisPseudocolorPlotWindow
//
// Purpose:
//   Destructor for the QvisPseudocolorPlotWindow class.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:01 PST 2000
//
// Modifications:
//
// ****************************************************************************

QvisPseudocolorPlotWindow::~QvisPseudocolorPlotWindow()
{
    pcAtts = 0;
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::CreateWindowContents
//
// Purpose:
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:30 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity slider.
//
//   Brad Whitlock, Sat Jun 16 12:43:40 PDT 2001
//   Added the color table button.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//   Added the limits selection combo box.
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002
//   Removed 'Specify' from limitsSelect.
//
//   Jeremy Meredith, Tue Dec 10 10:23:07 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 21:36:38 PDT 2003
//   Added point type options.
//
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004
//   Replace individual point-size related widgets with QvisPointControl
//
//   Brad Whitlock, Wed Jul 20 14:27:00 PST 2005
//   Added pointSizePixelsChanged slot.
//
//   Brad Whitlock, Wed Apr 23 10:16:57 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue May 27 14:52:08 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Oct 29 16:00:48 EDT 2008
//   Swap the min/max in the gui.
//
//   Jeremy Meredith, Wed Nov 26 11:28:24 EST 2008
//   Added line style/width controls.
//
//   Jeremy Meredith, Fri Feb 20 15:14:29 EST 2009
//   Added support for using per-color alpha values from a color table
//   (instead of just a single global opacity for the whole plot).
//   There's a new toggle for this, and it overrides the whole-plot opacity.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:02:39 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
//   Cyrus Harrison, Wed Nov  2 18:43:13 PDT 2016
//   Changed layout to use tabs.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateWindowContents()
{
    // create atts window a with a standard tab style layout
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // Data tab
    // ----------------------------------------------------------------------
    QWidget *dataTab = new QWidget(central);
    propertyTabs->addTab(dataTab, tr("Data"));
    CreateDataTab(dataTab);

    // ----------------------------------------------------------------------
    // Geometry tab
    // ----------------------------------------------------------------------
    QWidget *geometryTab = new QWidget(central);
    propertyTabs->addTab(geometryTab, tr("Geometry"));
    CreateGeometryTab(geometryTab);

    // ----------------------------------------------------------------------
    // Extras tab -- currently not used but setup for future use
    // ----------------------------------------------------------------------
    // QWidget *extrasTab = new QWidget(central);
    // propertyTabs->addTab(extrasTab, tr("Extras"));
    // CreateExtrasTab(extrasTab);

//     This code was used to create the accordion style layout

//     propertyLayout = new QvisCollapsibleLayout(central);
//     propertyLayout = new QvisCollapsibleLayout(central);
//     propertyLayout->setParent( this );
//     topLayout->addWidget(propertyLayout);
//
//     // ----------------------------------------------------------------------
//     // Data tab
//     // ----------------------------------------------------------------------
//     QWidget *dataTab = new QWidget(central);
//     CreateDataTab(dataTab);
//
//     QvisCollapsibleFrame* dataFrame =
//       propertyLayout->addFrame( tr("Data"), dataTab);
//
//     dataFrame->setShow();
//
//     // ----------------------------------------------------------------------
//     // Geometry tab
//     // ----------------------------------------------------------------------
//     QWidget *geometryTab = new QWidget(central);
//     CreateGeometryTab(geometryTab);
//
//     QvisCollapsibleFrame* geometryFrame =
//       propertyLayout->addFrame( tr("Geometry"), geometryTab);
//
//     geometryFrame->setHide();
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::CreateDataTab
//
// Purpose:
//   Populates the data tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//    Kathleen Biagas, Wed Dec 26 13:07:02 PST 2018
//    Add color buttons for values < min and > max.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateDataTab(QWidget *pageData)
{
    QGridLayout *topLayout = new QGridLayout(pageData);
    topLayout->setMargin(5);
    topLayout->setSpacing(10);

    //
    // Create the scale group
    //
    QGroupBox * dataGroup = new QGroupBox(central);
    dataGroup->setTitle(tr("Data"));
    topLayout->addWidget(dataGroup);

    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    dataLayout->setMargin(5);
    dataLayout->setSpacing(10);

    //
    // Create the scale radio buttons
    //
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
    dataLayout->addWidget(limitsGroup, 1, 0, 2, 5);

    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(5);
    limitsLayout->setSpacing(10);

    limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

    limitsSelect = new QComboBox(central);
    limitsSelect->addItem(tr("Use Original Data"));
    limitsSelect->addItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int)));
    limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit, and color for values < min
    minToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(minToggle, 1, 0);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central);
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText()));
    limitsLayout->addWidget(minLineEdit, 1, 1, 1, 1);

    belowMinToggle = new QCheckBox(tr("Color for values < min"), central);
    limitsLayout->addWidget(belowMinToggle, 1, 2, 1, 2, Qt::AlignRight);
    connect(belowMinToggle, SIGNAL(toggled(bool)),
            this, SLOT(belowMinToggled(bool)));

    belowMinColor = new QvisColorButton(central);
    belowMinColor->setButtonColor(QColor(255, 255, 255));
    connect(belowMinColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(belowMinColorChanged(const QColor &)));
    limitsLayout->addWidget(belowMinColor, 1, 4);

    // Create the max toggle and line edit and color for values > max
    maxToggle = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(maxToggle, 2, 0);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central);
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText()));
    limitsLayout->addWidget(maxLineEdit, 2, 1);

    aboveMaxToggle = new QCheckBox(tr("Color for values > max"), central);
    limitsLayout->addWidget(aboveMaxToggle, 2, 2, 1, 2);
    connect(aboveMaxToggle, SIGNAL(toggled(bool)),
            this, SLOT(aboveMaxToggled(bool)));

    aboveMaxColor = new QvisColorButton(central);
    aboveMaxColor->setButtonColor(QColor(255, 255, 255));
    connect(aboveMaxColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(aboveMaxColorChanged(const QColor &)));
    limitsLayout->addWidget(aboveMaxColor, 2, 4);

    //
    // Create the mesh group
    //

    // Create the centering label
    dataLayout->addWidget(new QLabel(tr("Centering"), central), 3, 0);

    // Create the radio buttons
    centeringButtons = new QButtonGroup(central);
    rb = new QRadioButton(tr("Original"), central );
    rb->setChecked(true);
    centeringButtons->addButton(rb, 0);
    dataLayout->addWidget(rb, 3, 1);
    rb = new QRadioButton(tr("Nodal"), central );
    dataLayout->addWidget(rb, 3, 2);
    centeringButtons->addButton(rb, 1);
    rb = new QRadioButton(tr("Zonal"), central );
    dataLayout->addWidget(rb, 3, 3);
    centeringButtons->addButton(rb, 2);
    // Each time a radio button is clicked, call the centeringClicked slot.
    connect(centeringButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(centeringClicked(int)));

    //
    // Create the color stuff
    //
    QGroupBox * colorGroup = new QGroupBox(central);
    colorGroup->setTitle(tr("Color"));
    topLayout->addWidget(colorGroup);

    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setMargin(5);
    colorLayout->setSpacing(10);

    int gRow = 0;

    // Create the color table widgets
    colorLayout->addWidget( new QLabel(tr("Color table"), central), gRow, 0);

    colorTableWidget = new QvisColorTableWidget(central, true);
    connect(colorTableWidget, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    connect(colorTableWidget,
            SIGNAL(invertColorTableToggled(bool)),
            this,
            SLOT(invertColorTableToggled(bool)));
    colorLayout->addWidget(colorTableWidget, gRow, 1, Qt::AlignLeft | Qt::AlignVCenter);

    gRow++;

    // Create the opacity widgets.
    opacityType = new QComboBox(central);
    opacityType->addItem(tr("From color table"),1);
    opacityType->addItem(tr("Fully opaque"),0);
    opacityType->addItem(tr("Constant"),2);
    opacityType->addItem(tr("Ramp"),3);
    // ARS - FIX ME  - FIX ME  - FIX ME  - FIX ME  - FIX ME
    // This functionality was possible with the deprecated Streamline
    // plot but it is currently not possible using the vtkMapper and
    // avtActor.
    // opacityType->addItem(tr("Variable range"),4);

    connect(opacityType, SIGNAL(activated(int)),
            this, SLOT(opacityTypeChanged(int)));
    colorLayout->addWidget(new QLabel(tr("Opacity"), central), gRow,0);
    colorLayout->addWidget(opacityType, gRow, 1);

    opacityVarLabel = new QLabel(tr("Variable"), central);
    opacityVar = new QvisVariableButton(true, true, true,
                                        QvisVariableButton::Scalars, central);
    colorLayout->addWidget(opacityVarLabel,gRow,2, Qt::AlignRight);
    colorLayout->addWidget(opacityVar,gRow,3);
    connect(opacityVar, SIGNAL(activated(const QString &)),
            this, SLOT(opacityVariableChanged(const QString&)));

    gRow++;
    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    colorLayout->addWidget(opacitySlider, gRow, 1, 1,3);
    gRow++;

    opacityMinToggle = new QCheckBox(tr("Opacity Min"), central);
    opacityMaxToggle = new QCheckBox(tr("Opacity Max"), central);
    connect(opacityMinToggle, SIGNAL(toggled(bool)), this, SLOT(opacityMinToggled(bool)));
    connect(opacityMaxToggle, SIGNAL(toggled(bool)), this, SLOT(opacityMaxToggled(bool)));

    opacityVarMin = new QLineEdit(central);
    opacityVarMax = new QLineEdit(central);

    connect(opacityVarMin, SIGNAL(returnPressed()), this, SLOT(processOpacityVarMin()));
    connect(opacityVarMax, SIGNAL(returnPressed()), this, SLOT(processOpacityVarMax()));



    opacityMinMaxGroup = new QGroupBox(central);
    colorLayout->addWidget(opacityMinMaxGroup, gRow, 1, 1, 3);

    QGridLayout *opacityMinMaxLayout = new QGridLayout(opacityMinMaxGroup);
    opacityMinMaxLayout->setMargin(5);
    opacityMinMaxLayout->setSpacing(10);


    opacityMinMaxLayout->addWidget(opacityMinToggle, 0, 1);
    opacityMinMaxLayout->addWidget(opacityVarMin,    0, 2);
    opacityMinMaxLayout->addWidget(opacityMaxToggle, 0, 3);
    opacityMinMaxLayout->addWidget(opacityVarMax,    0, 4);
    gRow++;


    // // Create the radio buttons
    // colorLayout->addWidget(new QLabel(tr("Opacity"), central), 1, 0);

    // opacityButtons = new QButtonGroup(central);

    // rb = new QRadioButton(tr("Set explicitly"), central);
    // rb->setChecked(true);
    // opacityButtons->addButton(rb, 0);
    // colorLayout->addWidget(rb, gRow, 1);
    // rb = new QRadioButton(tr("From color table"), central);
    // opacityButtons->addButton(rb, 1);
    // colorLayout->addWidget(rb, gRow, 2);

    // // Each time a radio button is clicked, call the scale clicked slot.
    // connect(opacityButtons, SIGNAL(buttonClicked(int)),
    //         this, SLOT(setOpaacityClicked(int)));

    // gRow++;

    // //
    // // Create the opacity slider
    // //
    // opacitySliderLabel = new QLabel(tr("Opacity"), central);
    // colorLayout->addWidget(opacitySliderLabel, gRow, 0);

    // opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central);
    // opacitySlider->setTickInterval(64);
    // opacitySlider->setGradientColor(QColor(0, 0, 0));
    // connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
    //         this, SLOT(changedOpacity(int, const void*)));
    // colorLayout->addWidget(opacitySlider, gRow, 1, 1, 3);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setMargin(5);
    miscLayout->setSpacing(10);

    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);

    // Create the lighting toggle
    lightingToggle = new QCheckBox(tr("Lighting"), central);
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    miscLayout->addWidget(lightingToggle, 0, 1);
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::CreateGeometryTab
//
// Purpose:
//   Populates the geometry tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Eric Brugger, Wed Oct 26 09:18:20 PDT 2016
//   I modified the plot to support independently setting the point style
//   for the two end points of lines.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateGeometryTab(QWidget *pageGeometry)
{
    QGridLayout *topLayout = new QGridLayout(pageGeometry);
    topLayout->setMargin(5);
    topLayout->setSpacing(10);

    //
    // Create line related controls.
    //
    QGroupBox * lineGroup = new QGroupBox(central);
    lineGroup->setTitle(tr("Line "));
    topLayout->addWidget(lineGroup);

    QGridLayout *lineLayout = new QGridLayout(lineGroup);
    lineLayout->setMargin(5);
    lineLayout->setSpacing(10);

    //
    // Create the line style/width buttons
    //
    // Create the lineSyle widget.

    // Line
    lineTypeLabel = new QLabel(tr("Line type"), central);
    lineLayout->addWidget(lineTypeLabel, 0, 0, Qt::AlignRight);

    lineType = new QComboBox(central);
    lineType->addItem(tr("Lines"), 0);
    lineType->addItem(tr("Tubes"), 1);
    lineType->addItem(tr("Ribbons"), 2);
    connect(lineType, SIGNAL(activated(int)), this, SLOT(lineTypeChanged(int)));
    lineLayout->addWidget(lineType, 0, 1);

    // ARS - FIX ME  - FIX ME  - FIX ME  - FIX ME  - FIX ME
    // lineTypeLabel->hide();
    // lineType->hide();


    // Create the lineSyle widget.
    lineWidthLabel = new QLabel(tr("Line width"), central);
    lineLayout->addWidget(lineWidthLabel, 1, 0, Qt::AlignRight);

    lineWidth = new QvisLineWidthWidget(0, central);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineLayout->addWidget(lineWidth, 1, 1);


    // Tube/ribbon
    tubeRadiusLabel = new QLabel(tr("Radius"), central);
    tubeRadiusLabel->setToolTip(tr("Radius used for tubes and ribbons."));
    lineLayout->addWidget(tubeRadiusLabel, 1, 0, Qt::AlignRight);

    tubeRadius = new QLineEdit(central);
    lineLayout->addWidget(tubeRadius, 1, 1);
    connect(tubeRadius, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusProcessText()));

    tubeRadiusSizeType = new QComboBox(central);
    tubeRadiusSizeType->addItem(tr("Absolute"), 0);
    tubeRadiusSizeType->addItem(tr("Fraction of Bounding Box"), 1);
    connect(tubeRadiusSizeType, SIGNAL(activated(int)), this, SLOT(tubeRadiusSizeTypeChanged(int)));
    lineLayout->addWidget(tubeRadiusSizeType, 1, 2);

    // Tube variable radius.

    tubeRadiusVarEnabled = new QCheckBox(tr("Variable radius"), central);
    connect(tubeRadiusVarEnabled, SIGNAL(toggled(bool)), this, SLOT(tubeRadiusVarToggled(bool)));
    lineLayout->addWidget(tubeRadiusVarEnabled, 2, 1, Qt::AlignRight);
    tubeRadiusVar = new QvisVariableButton(false, true, true,
                                           QvisVariableButton::Scalars, central);
    connect(tubeRadiusVar, SIGNAL(activated(const QString &)),
            this, SLOT(tubeRadiusVarChanged(const QString&)));
    lineLayout->addWidget(tubeRadiusVar, 2, 2);

    tubeRadiusVarRatioLabel = new QLabel(tr("Max/Min Ratio"), central);
    lineLayout->addWidget(tubeRadiusVarRatioLabel, 2, 3, Qt::AlignRight);
    tubeRadiusVarRatio = new QLineEdit(central);
    connect(tubeRadiusVarRatio, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusVarRatioProcessText()));
    lineLayout->addWidget(tubeRadiusVarRatio, 2, 4);


    tubeResolutionLabel = new QLabel(tr("Resolution"), central);
    lineLayout->addWidget(tubeResolutionLabel, 0, 3, Qt::AlignRight);

    tubeResolution = new QSpinBox(central);
    tubeResolution->setKeyboardTracking(false);
    tubeResolution->setMinimum(3);
    tubeResolution->setMaximum(100);
    lineLayout->addWidget(tubeResolution, 0, 4);
    connect(tubeResolution, SIGNAL(valueChanged(int)), this, SLOT(tubeResolutionChanged(int)));


    // Splitter
    QFrame *splitter = new QFrame(central);
    splitter->setFrameStyle(QFrame::HLine + QFrame::Raised);
    lineLayout->addWidget(splitter, 3, 0, 1, 5);

    // End points
    tailStyleLabel = new QLabel(tr("Tail"), central);
    lineLayout->addWidget(tailStyleLabel, 4, 0, Qt::AlignRight);

    tailStyle = new QComboBox(central);
    tailStyle->addItem(tr("None"), 0);
    tailStyle->addItem(tr("Sphere"), 1);
    tailStyle->addItem(tr("Cone"), 2);
    connect(tailStyle, SIGNAL(activated(int)), this, SLOT(tailStyleChanged(int)));
    lineLayout->addWidget(tailStyle, 4, 1);

    headStyleLabel = new QLabel(tr("Head"), central);
    lineLayout->addWidget(headStyleLabel, 4, 2, Qt::AlignRight);

    headStyle = new QComboBox(central);
    headStyle->addItem(tr("None"), 0);
    headStyle->addItem(tr("Sphere"), 1);
    headStyle->addItem(tr("Cone"), 2);
    connect(headStyle, SIGNAL(activated(int)), this, SLOT(headStyleChanged(int)));
    lineLayout->addWidget(headStyle, 4, 3);

    endPointRadiusLabel = new QLabel(tr("Radius"), central);
    lineLayout->addWidget(endPointRadiusLabel, 5, 0, Qt::AlignRight);

    endPointRadius = new QLineEdit(central);
    lineLayout->addWidget(endPointRadius, 5, 1);
    connect(endPointRadius, SIGNAL(returnPressed()), this, SLOT(endPointRadiusProcessText()));

    endPointRadiusSizeType = new QComboBox(central);
    endPointRadiusSizeType->addItem(tr("Absolute"), 0);
    endPointRadiusSizeType->addItem(tr("Fraction of bounding box"), 1);
    connect(endPointRadiusSizeType, SIGNAL(activated(int)), this, SLOT(endPointRadiusSizeTypeChanged(int)));
    lineLayout->addWidget(endPointRadiusSizeType, 5, 2);

    endPointRatioLabel = new QLabel(tr("Cone ratio"), central);
    lineLayout->addWidget(endPointRatioLabel, 5, 3, Qt::AlignRight);

    endPointRatio = new QLineEdit(central);
    connect(endPointRatio, SIGNAL(returnPressed()),
            this, SLOT(endPointRatioProcessText()));
    lineLayout->addWidget(endPointRatio, 5, 4);

    // End point variable radius.
    endPointRadiusVarEnabled = new QCheckBox(tr("Variable radius"), central);
    connect(endPointRadiusVarEnabled, SIGNAL(toggled(bool)), this, SLOT(endPointRadiusVarToggled(bool)));
    lineLayout->addWidget(endPointRadiusVarEnabled, 6, 0, 1, 2, Qt::AlignRight);

    endPointRadiusVar = new QvisVariableButton(true, true, true,
                                               QvisVariableButton::Scalars, central);
    connect(endPointRadiusVar, SIGNAL(activated(const QString &)),
            this, SLOT(endPointRadiusVarChanged(const QString&)));
    lineLayout->addWidget(endPointRadiusVar, 6, 2);

    endPointRadiusVarRatioLabel = new QLabel(tr("Max/Min ratio"), central);
    lineLayout->addWidget(endPointRadiusVarRatioLabel, 6, 3, Qt::AlignRight);
    endPointRadiusVarRatio = new QLineEdit(central);
    connect(endPointRadiusVarRatio, SIGNAL(returnPressed()),
            this, SLOT(endPointRadiusVarRatioProcessText()));
    lineLayout->addWidget(endPointRadiusVarRatio, 6, 4);

    endPointResolutionLabel = new QLabel(tr("Resolution"), central);
    lineLayout->addWidget(endPointResolutionLabel, 7, 0, Qt::AlignRight);

    endPointResolution = new QSpinBox(central);
    endPointResolution->setKeyboardTracking(false);
    endPointResolution->setMinimum(3);
    endPointResolution->setMaximum(100);
    lineLayout->addWidget(endPointResolution, 7, 1);
    connect(endPointResolution, SIGNAL(valueChanged(int)), this, SLOT(endPointResolutionChanged(int)));

    //
    // Create point related controls.
    //
    QGroupBox * pointGroup = new QGroupBox(central);
    pointGroup->setTitle(tr("Point "));
    topLayout->addWidget(pointGroup);

    QGridLayout *pointLayout = new QGridLayout(pointGroup);
    pointLayout->setMargin(5);
    pointLayout->setSpacing(10);

    // Create the point control
    pointControl = new QvisPointControl(central);
    connect(pointControl, SIGNAL(pointSizeChanged(double)),
            this, SLOT(pointSizeChanged(double)));
    connect(pointControl, SIGNAL(pointSizePixelsChanged(int)),
            this, SLOT(pointSizePixelsChanged(int)));
    connect(pointControl, SIGNAL(pointSizeVarChanged(const QString &)),
            this, SLOT(pointSizeVarChanged(const QString &)));
    connect(pointControl, SIGNAL(pointSizeVarToggled(bool)),
            this, SLOT(pointSizeVarToggled(bool)));
    connect(pointControl, SIGNAL(pointTypeChanged(int)),
            this, SLOT(pointTypeChanged(int)));
    pointLayout->addWidget(pointControl, 0, 0, 1, 4);

    //
    // Create the rendering group
    //
    QGroupBox * renderingGroup = new QGroupBox(central);
    renderingGroup->setTitle(tr("Rendering"));
    topLayout->addWidget(renderingGroup);

    QGridLayout *renderingLayout = new QGridLayout(renderingGroup);
    renderingLayout->setMargin(5);
    renderingLayout->setSpacing(10);


    // Create the rendering style options
    renderLabel = new QLabel(tr("Draw objects as"), central);
    renderingLayout->addWidget(renderLabel, 0,0);

    // Create the rendering buttons
    renderSurfaces = new QCheckBox(tr("Surfaces"), central);
    renderingLayout->addWidget( renderSurfaces, 0,1);
    connect(renderSurfaces, SIGNAL(toggled(bool)),
            this, SLOT(renderSurfacesChanged(bool)));

    renderWireframe = new QCheckBox(tr("Wireframe"), central);
    renderingLayout->addWidget( renderWireframe, 0,2);
    connect(renderWireframe, SIGNAL(toggled(bool)),
            this, SLOT(renderWireframeChanged(bool)));
    wireframeRenderColor = new QvisColorButton(central);
    renderingLayout->addWidget(wireframeRenderColor, 0,3);
    connect(wireframeRenderColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(wireframeColorChanged(const QColor &)));

    renderPoints = new QCheckBox(tr("Points"), central);
    renderingLayout->addWidget( renderPoints, 0,4);
    connect(renderPoints, SIGNAL(toggled(bool)),
            this, SLOT(renderPointsChanged(bool)));
    pointsRenderColor = new QvisColorButton(central);
    renderingLayout->addWidget(pointsRenderColor, 0,5);
    connect(pointsRenderColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(pointColorChanged(const QColor &)));

    // Create the smoothing options
    renderingLayout->addWidget(new QLabel(tr("Smoothing"), central), 1,0);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));

    QRadioButton* rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb, 0);
    renderingLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb, 1);
    renderingLayout->addWidget(rb, 1, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb, 2);
    renderingLayout->addWidget(rb, 1, 3);
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::CreateExtrasTab
//
// Purpose:
//   Populates the extras tab.
//
// Programmer: Dave Pugmire
// Creation:   Tue Dec 29 14:37:53 EST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateExtrasTab(QWidget *pageExtras)
{
    QGridLayout *topLayout = new QGridLayout(pageExtras);
    topLayout->setMargin(5);
    topLayout->setSpacing(10);

    //
    // Create the blank stuff to fill in gaps.
    //
    QGroupBox * blankGroup = new QGroupBox(central);
//    blankGroup->setTitle(tr("Blank"));
    topLayout->addWidget(blankGroup);

    QGridLayout *blankLayout = new QGridLayout(blankGroup);
    blankLayout->setMargin(5);
    blankLayout->setSpacing(10);

    blankLayout->addWidget(new QLabel(tr(""), central), 0,0);
    blankLayout->addWidget(new QLabel(tr(""), central), 1,0);
    blankLayout->addWidget(new QLabel(tr(""), central), 2,0);
    blankLayout->addWidget(new QLabel(tr(""), central), 3,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 4,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 5,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 6,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 7,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 8,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 9,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 10,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 11,0);
    // blankLayout->addWidget(new QLabel(tr(""), central), 12,0);
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::UpdateWindow
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
// Creation:   Tue Aug 1 17:07:27 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity.
//
//   Brad Whitlock, Sat Jun 16 15:32:18 PST 2001
//   Added code to set the color table.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//   Added code for limits selection.
//
//   Brad Whitlock, Wed Nov 7 16:05:44 PST 2001
//   Modified the code a little so it can use the generated attributes.
//
//   Brad Whitlock, Fri Feb 15 09:37:51 PDT 2002
//   Altered the code to set the opacity slider's opacity.
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002
//   Removed dependency of min/max upon LimitsMode.
//
//   Jeremy Meredith, Tue Dec 10 10:23:07 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 21:59:56 PDT 2003
//   Account for point type.
//
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//   When doing GL_POINT, we ignore point size, so also disable it.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004
//   Replace point-size related cases with QvisPointControl
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//   Fixed SGI compiler error with string conversion to QString
//
//   Brad Whitlock, Thu Jun 26 16:58:00 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Wed Nov 26 11:28:24 EST 2008
//   Added line style/width controls.
//
//   Jeremy Meredith, Fri Feb 20 15:14:29 EST 2009
//   Added support for using per-color alpha values from a color table
//   (instead of just a single global opacity for the whole plot).
//   There's a new toggle for this, and it overrides the whole-plot opacity.
//
//   Kathleen Bonnell, Mon Jan 17 18:02:39 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
//   Eric Brugger, Wed Oct 26 09:18:20 PDT 2016
//   I modified the plot to support independently setting the point style
//   for the two end points of lines.
//
//   Cyrus Harrison, Wed Nov  2 19:09:51 PDT 2016
//   Remove tubeRadiusVarLabel, the check box used for this includes a label.
//
//   Kathleen Biagas, Wed Dec 26 13:08:45 PST 2018
//   Add logic for belowMinColor, aboveMaxColor.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::UpdateWindow(bool doAll)
{
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < pcAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!pcAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
          // scale
        case PseudocolorAttributes::ID_scaling:
            scalingButtons->blockSignals(true);
            scalingButtons->button(pcAtts->GetScaling())->setChecked(true);
            scalingButtons->blockSignals(false);

            if( pcAtts->GetScaling() == PseudocolorAttributes::Skew )
            {
              skewLineEdit->setEnabled(true);
            }
            else
            {
              skewLineEdit->setEnabled(false);
            }

            break;
        case PseudocolorAttributes::ID_skewFactor:
            skewLineEdit->setText(DoubleToQString(pcAtts->GetSkewFactor()));
            break;

            // limits
        case PseudocolorAttributes::ID_limitsMode:
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentIndex(pcAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_min:
            minLineEdit->setText(DoubleToQString(pcAtts->GetMin()));
            break;
        case PseudocolorAttributes::ID_max:
            maxLineEdit->setText(DoubleToQString(pcAtts->GetMax()));
            break;
        case PseudocolorAttributes::ID_minFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(pcAtts->GetMinFlag());
            minLineEdit->setEnabled(pcAtts->GetMinFlag());
            belowMinToggle->setEnabled(pcAtts->GetMinFlag());
            belowMinColor->setEnabled(pcAtts->GetUseBelowMinColor());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
        case PseudocolorAttributes::ID_useBelowMinColor:
            disconnect(belowMinToggle, SIGNAL(toggled(bool)),
                       this, SLOT(belowMinToggled(bool)));
            belowMinToggle->setChecked(pcAtts->GetUseBelowMinColor());
            belowMinColor->setEnabled(pcAtts->GetUseBelowMinColor());
            connect(belowMinToggle, SIGNAL(toggled(bool)),
                       this, SLOT(belowMinToggled(bool)));
            break;
        case PseudocolorAttributes::ID_belowMinColor:
            { // new scope
            QColor temp(pcAtts->GetBelowMinColor().Red(),
                        pcAtts->GetBelowMinColor().Green(),
                        pcAtts->GetBelowMinColor().Blue());
            belowMinColor->blockSignals(true);
            belowMinColor->setButtonColor(temp);
            belowMinColor->blockSignals(false);
            }
            break;
        case PseudocolorAttributes::ID_maxFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(pcAtts->GetMaxFlag());
            maxLineEdit->setEnabled(pcAtts->GetMaxFlag());
            aboveMaxToggle->setEnabled(pcAtts->GetMaxFlag());
            aboveMaxColor->setEnabled(pcAtts->GetUseAboveMaxColor());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
            break;
        case PseudocolorAttributes::ID_useAboveMaxColor:
            disconnect(aboveMaxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(aboveMaxToggled(bool)));
            aboveMaxToggle->setChecked(pcAtts->GetUseAboveMaxColor());
            aboveMaxColor->setEnabled(pcAtts->GetUseAboveMaxColor());
            connect(aboveMaxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(aboveMaxToggled(bool)));
            break;
        case PseudocolorAttributes::ID_aboveMaxColor:
            { // new scope
            QColor temp(pcAtts->GetAboveMaxColor().Red(),
                        pcAtts->GetAboveMaxColor().Green(),
                        pcAtts->GetAboveMaxColor().Blue());
            aboveMaxColor->blockSignals(true);
            aboveMaxColor->setButtonColor(temp);
            aboveMaxColor->blockSignals(false);
            }
            break;

            // centering
        case PseudocolorAttributes::ID_centering:
            centeringButtons->blockSignals(true);
            centeringButtons->button(pcAtts->GetCentering())->setChecked(true);
            centeringButtons->blockSignals(false);
            break;

            // color table
        case PseudocolorAttributes::ID_colorTableName:
            colorTableWidget->setColorTable(pcAtts->GetColorTableName().c_str());
            break;
        case PseudocolorAttributes::ID_invertColorTable:
            colorTableWidget->setInvertColorTable(pcAtts->GetInvertColorTable());
            break;

            // opacity
        case PseudocolorAttributes::ID_opacityVariable:
          opacityVar->setText(pcAtts->GetOpacityVariable().c_str());
          break;

        case PseudocolorAttributes::ID_opacity:
            opacitySlider->blockSignals(true);
            opacitySlider->setValue(int((float)pcAtts->GetOpacity() * 255.f));
            opacitySlider->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_opacityVarMin:
          opacityVarMin->setText(DoubleToQString(pcAtts->GetOpacityVarMin()));
          break;

        case PseudocolorAttributes::ID_opacityVarMax:
          opacityVarMax->setText(DoubleToQString(pcAtts->GetOpacityVarMax()));
          break;

        case PseudocolorAttributes::ID_opacityType:
          if (pcAtts->GetOpacityType() == PseudocolorAttributes::FullyOpaque ||
              pcAtts->GetOpacityType() == PseudocolorAttributes::ColorTable)
          {
            opacitySlider->hide();
            opacityVar->hide();
            opacityVarLabel->hide();
            // opacityMinToggle->hide();
            // opacityMaxToggle->hide();
            // opacityVarMin->hide();
            // opacityVarMax->hide();
            opacityMinMaxGroup->hide();
          }
          else if (pcAtts->GetOpacityType() == PseudocolorAttributes::Constant ||
                   pcAtts->GetOpacityType() == PseudocolorAttributes::Ramp)
          {
            opacitySlider->show();
            opacityVar->hide();
            opacityVarLabel->hide();
            // opacityMinToggle->hide();
            // opacityMaxToggle->hide();
            // opacityVarMin->hide();
            // opacityVarMax->hide();
            opacityMinMaxGroup->hide();
          }
          else if (pcAtts->GetOpacityType() == PseudocolorAttributes::VariableRange)
          {
            opacitySlider->show();
            opacityVar->show();
            opacityVarLabel->show();
            // opacityMinToggle->show();
            // opacityMaxToggle->show();
            // opacityVarMin->show();
            // opacityVarMax->show();
            opacityMinMaxGroup->show();
          }

          opacityType->blockSignals(true);
          opacityType->setCurrentIndex(int(pcAtts->GetOpacityType()));
          opacityType->blockSignals(false);
          break;

        case PseudocolorAttributes::ID_opacityVarMinFlag:
          opacityMinToggle->blockSignals(true);
          opacityMinToggle->setChecked(pcAtts->GetOpacityVarMinFlag());

          opacityVarMin->setEnabled(pcAtts->GetOpacityVarMinFlag());
          opacityMinToggle->blockSignals(false);
          break;

        case PseudocolorAttributes::ID_opacityVarMaxFlag:
          opacityMaxToggle->blockSignals(true);
          opacityMaxToggle->setChecked(pcAtts->GetOpacityVarMaxFlag());

          opacityVarMax->setEnabled(pcAtts->GetOpacityVarMaxFlag());
          opacityMaxToggle->blockSignals(false);
          break;

          // point
        case PseudocolorAttributes::ID_pointSize:
            pointControl->blockSignals(true);
            pointControl->SetPointSize(pcAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointType:
            pointControl->blockSignals(true);
            pointControl->SetPointType(pcAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointSizeVarEnabled:
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(pcAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointSizeVar:
            {
            pointControl->blockSignals(true);
            QString temp(pcAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            }
            break;
        case PseudocolorAttributes::ID_pointSizePixels:
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(pcAtts->GetPointSizePixels());
            pointControl->blockSignals(false);
            break;

            // line
        case PseudocolorAttributes::ID_lineType:
            lineType->blockSignals(true);
            lineType->setCurrentIndex(int(pcAtts->GetLineType()));
            lineType->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(pcAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;

            // tubes and ribbons
        case PseudocolorAttributes::ID_tubeResolution:
            tubeResolution->blockSignals(true);
            tubeResolution->setValue(pcAtts->GetTubeResolution());
            tubeResolution->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_tubeRadiusSizeType:
            tubeRadiusSizeType->blockSignals(true);
            tubeRadiusSizeType->setCurrentIndex((int) pcAtts->GetTubeRadiusSizeType());
            tubeRadiusSizeType->blockSignals(false);
            if (pcAtts->GetTubeRadiusSizeType() == PseudocolorAttributes::Absolute)
            {
                tubeRadius->setText(DoubleToQString(pcAtts->GetTubeRadiusAbsolute()));
            }
            else
            {
                tubeRadius->setText(DoubleToQString(pcAtts->GetTubeRadiusBBox()));
            }
            break;
        case PseudocolorAttributes::ID_tubeRadiusAbsolute:
            if (pcAtts->GetTubeRadiusSizeType() == PseudocolorAttributes::Absolute)
            {
                tubeRadius->setText(DoubleToQString(pcAtts->GetTubeRadiusAbsolute()));
            }
            break;
        case PseudocolorAttributes::ID_tubeRadiusBBox:
            if (pcAtts->GetTubeRadiusSizeType() == PseudocolorAttributes::FractionOfBBox)
            {
                tubeRadius->setText(DoubleToQString(pcAtts->GetTubeRadiusBBox()));
            }
            break;

        case PseudocolorAttributes::ID_tubeRadiusVarEnabled:
            tubeRadiusVar->blockSignals(true);

            tubeRadiusVarEnabled->setChecked( pcAtts->GetTubeRadiusVarEnabled() );
            tubeRadiusVar->setEnabled( pcAtts->GetTubeRadiusVarEnabled() );
            tubeRadiusVarRatioLabel->setEnabled( pcAtts->GetTubeRadiusVarEnabled() );
            tubeRadiusVarRatio->setEnabled( pcAtts->GetTubeRadiusVarEnabled() );
            tubeRadiusVar->blockSignals(false);

        case PseudocolorAttributes::ID_tubeRadiusVar:
            tubeRadiusVar->blockSignals(true);
            tubeRadiusVar->setText(pcAtts->GetTubeRadiusVar().c_str());
            tubeRadiusVar->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_tubeRadiusVarRatio:
            tubeRadiusVarRatio->blockSignals(true);
            tubeRadiusVarRatio->setText(DoubleToQString(pcAtts->GetTubeRadiusVarRatio()));
            tubeRadiusVarRatio->blockSignals(false);
            break;

            // endpoints
        case PseudocolorAttributes::ID_endPointResolution:
            endPointResolution->blockSignals(true);
            endPointResolution->setValue(pcAtts->GetEndPointResolution());
            endPointResolution->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_tailStyle:
        case PseudocolorAttributes::ID_headStyle:
            tailStyle->blockSignals(true);
            tailStyle->setCurrentIndex(int(pcAtts->GetTailStyle()));
            tailStyle->blockSignals(false);
            headStyle->blockSignals(true);
            headStyle->setCurrentIndex(int(pcAtts->GetHeadStyle()));
            headStyle->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_endPointRadiusSizeType:
            endPointRadiusSizeType->blockSignals(true);
            endPointRadiusSizeType->setCurrentIndex((int) pcAtts->GetEndPointRadiusSizeType());
            endPointRadiusSizeType->blockSignals(false);
            if (pcAtts->GetEndPointRadiusSizeType() == PseudocolorAttributes::Absolute)
            {
                endPointRadius->setText(DoubleToQString(pcAtts->GetEndPointRadiusAbsolute()));
            }
            else
            {
                endPointRadius->setText(DoubleToQString(pcAtts->GetEndPointRadiusBBox()));
            }
            break;
        case PseudocolorAttributes::ID_endPointRadiusAbsolute:
            if (pcAtts->GetEndPointRadiusSizeType() == PseudocolorAttributes::Absolute)
            {
                endPointRadius->setText(DoubleToQString(pcAtts->GetEndPointRadiusAbsolute()));
            }
            break;
        case PseudocolorAttributes::ID_endPointRadiusBBox:
            if (pcAtts->GetEndPointRadiusSizeType() == PseudocolorAttributes::FractionOfBBox)
            {
                endPointRadius->setText(DoubleToQString(pcAtts->GetEndPointRadiusBBox()));
            }
            break;
        case PseudocolorAttributes::ID_endPointRatio:
            endPointRatio->setText(DoubleToQString(pcAtts->GetEndPointRatio()));
            break;

        case PseudocolorAttributes::ID_endPointRadiusVarEnabled:
            endPointRadiusVar->blockSignals(true);

            endPointRadiusVarEnabled->setChecked( pcAtts->GetEndPointRadiusVarEnabled() );
            endPointRadiusVar->setEnabled( pcAtts->GetEndPointRadiusVarEnabled() );
            endPointRadiusVarRatioLabel->setEnabled( pcAtts->GetEndPointRadiusVarEnabled() );
            endPointRadiusVarRatio->setEnabled( pcAtts->GetEndPointRadiusVarEnabled() );
            endPointRadiusVar->blockSignals(false);

        case PseudocolorAttributes::ID_endPointRadiusVar:
            endPointRadiusVar->blockSignals(true);
            endPointRadiusVar->setText(pcAtts->GetEndPointRadiusVar().c_str());
            endPointRadiusVar->blockSignals(false);
            break;

        case PseudocolorAttributes::ID_endPointRadiusVarRatio:
            endPointRadiusVarRatio->blockSignals(true);
            endPointRadiusVarRatio->setText(DoubleToQString(pcAtts->GetEndPointRadiusVarRatio()));
            endPointRadiusVarRatio->blockSignals(false);
            break;

            // smoothing
        case PseudocolorAttributes::ID_smoothingLevel:
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->button(pcAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelButtons->blockSignals(false);
            break;

            // render surface
        case PseudocolorAttributes::ID_renderSurfaces:
            renderSurfaces->blockSignals(true);
            renderSurfaces->setChecked( pcAtts->GetRenderSurfaces() );
            renderSurfaces->blockSignals(false);
            break;
            // render wireframe
        case PseudocolorAttributes::ID_renderWireframe:
            renderWireframe->blockSignals(true);
            renderWireframe->setChecked( pcAtts->GetRenderWireframe() );
            renderWireframe->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_wireframeColor:
            { // new scope
            QColor temp(pcAtts->GetWireframeColor().Red(),
                        pcAtts->GetWireframeColor().Green(),
                        pcAtts->GetWireframeColor().Blue());
            wireframeRenderColor->blockSignals(true);
            wireframeRenderColor->setButtonColor(temp);
            wireframeRenderColor->blockSignals(false);
            wireframeRenderColor->setEnabled(pcAtts->GetRenderWireframe());
            }
            break;

            // render point
        case PseudocolorAttributes::ID_renderPoints:
            renderPoints->blockSignals(true);
            renderPoints->setChecked( pcAtts->GetRenderPoints() );
            renderPoints->blockSignals(false);
            pointsRenderColor->setEnabled(pcAtts->GetRenderPoints());
            break;
        case PseudocolorAttributes::ID_pointColor:
            { // new scope
            QColor temp(pcAtts->GetPointColor().Red(),
                        pcAtts->GetPointColor().Green(),
                        pcAtts->GetPointColor().Blue());
            pointsRenderColor->blockSignals(true);
            pointsRenderColor->setButtonColor(temp);
            pointsRenderColor->blockSignals(false);
            }
            break;

            // legend
        case PseudocolorAttributes::ID_legendFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(legendToggle, SIGNAL(toggled(bool)),
                       this, SLOT(legendToggled(bool)));
            legendToggle->setChecked(pcAtts->GetLegendFlag());
            connect(legendToggle, SIGNAL(toggled(bool)),
                    this, SLOT(legendToggled(bool)));
            break;

            // lighting
        case PseudocolorAttributes::ID_lightingFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(lightingToggle, SIGNAL(toggled(bool)),
                       this, SLOT(lightingToggled(bool)));
            lightingToggle->setChecked(pcAtts->GetLightingFlag());
            connect(lightingToggle, SIGNAL(toggled(bool)),
                    this, SLOT(lightingToggled(bool)));
            break;
        }
    } // end for

    // change visibility of certain controls based on
    // rendering type(s) in effect.
    lineSettings();
    endPointSettings();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::GetCurrentValues
//
// Purpose:
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:11:42 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004
//   Replace pointSizeLineEdit and pointSizeVarLineEdit with pointControl.
//
//   Brad Whitlock, Wed Jul 20 14:25:04 PST 2005
//   Added PointSizePixels.
//
//   Brad Whitlock, Wed Apr 23 10:20:14 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jul  9 14:52:14 PDT 2008
//   Use helper methods.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do the skew factor value
    if(which_widget == PseudocolorAttributes::ID_skewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(skewLineEdit, val))
            pcAtts->SetSkewFactor(val);
        else
        {
            ResettingError(tr("skew"), DoubleToQString(pcAtts->GetSkewFactor()));
            pcAtts->SetSkewFactor(pcAtts->GetSkewFactor());
        }
    }

    // Do the minimum value.
    if(which_widget == PseudocolorAttributes::ID_min || doAll)
    {
        double val;
        if(LineEditGetDouble(minLineEdit, val))
            pcAtts->SetMin(val);
        else
        {
            ResettingError(tr("minimum value"), DoubleToQString(pcAtts->GetMin()));
            pcAtts->SetMin(pcAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == PseudocolorAttributes::ID_max || doAll)
    {
        double val;
        if(LineEditGetDouble(maxLineEdit, val))
            pcAtts->SetMax(val);
        else
        {
            ResettingError(tr("maximum value"), DoubleToQString(pcAtts->GetMax()));
            pcAtts->SetMax(pcAtts->GetMax());
        }
    }

    // opacityMin
    if(which_widget == PseudocolorAttributes::ID_opacityVarMin || doAll)
    {
        double val;
        if(LineEditGetDouble(opacityVarMin, val))
            pcAtts->SetOpacityVarMin(val);
        else
        {
            ResettingError(tr("Opacity Min"),
                DoubleToQString(pcAtts->GetOpacityVarMin()));
            pcAtts->SetOpacityVarMin(pcAtts->GetOpacityVarMin());
        }
    }

    // opacityMax
    if(which_widget == PseudocolorAttributes::ID_opacityVarMax || doAll)
    {
        double val;
        if(LineEditGetDouble(opacityVarMax, val))
            pcAtts->SetOpacityVarMax(val);
        else
        {
            ResettingError(tr("Opacity Max"),
                DoubleToQString(pcAtts->GetOpacityVarMax()));
            pcAtts->SetOpacityVarMax(pcAtts->GetOpacityVarMax());
        }
    }

    // tube resolution
    if (which_widget == PseudocolorAttributes::ID_tubeResolution|| doAll)
    {
        // This can only be an integer, so no error checking is needed.
        if (tubeResolution->value() != pcAtts->GetTubeResolution())
            pcAtts->SetTubeResolution(tubeResolution->value());
    }

    // tube radius
    if ((which_widget == PseudocolorAttributes::ID_tubeRadiusAbsolute || doAll)
        && pcAtts->GetTubeRadiusSizeType() == PseudocolorAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(tubeRadius, val))
        {
            if (val > 0.0)
              pcAtts->SetTubeRadiusAbsolute(val);
            else
            {
                ResettingError(tr("Tube radius must be > 0.0"),
                               DoubleToQString(pcAtts->GetTubeRadiusAbsolute()));
                pcAtts->SetTubeRadiusAbsolute(pcAtts->GetTubeRadiusAbsolute());
            }
        }
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(pcAtts->GetTubeRadiusAbsolute()));
            pcAtts->SetTubeRadiusAbsolute(pcAtts->GetTubeRadiusAbsolute());
        }
    }
    if ((which_widget == PseudocolorAttributes::ID_tubeRadiusBBox || doAll)
        && pcAtts->GetTubeRadiusSizeType() == PseudocolorAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(tubeRadius, val))
        {
            if (val > 0.0)
              pcAtts->SetTubeRadiusBBox(val);
            else
            {
                ResettingError(tr("Tube radius must be > 0.0"),
                               DoubleToQString(pcAtts->GetTubeRadiusBBox()));
                pcAtts->SetTubeRadiusBBox(pcAtts->GetTubeRadiusBBox());
            }
        }
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(pcAtts->GetTubeRadiusBBox()));
            pcAtts->SetTubeRadiusBBox(pcAtts->GetTubeRadiusBBox());
        }
    }

    // tube radius variable Ratio
    if (which_widget == PseudocolorAttributes::ID_tubeRadiusVarRatio || doAll)
    {
        double val;
        if (LineEditGetDouble(tubeRadiusVarRatio, val))
        {
            if (val >= 1.0)
                pcAtts->SetTubeRadiusVarRatio(val);
            else
            {
                ResettingError(tr("Tube variable radius ratio must be >= 1.0"),
                               DoubleToQString(pcAtts->GetTubeRadiusVarRatio()));
                pcAtts->SetTubeRadiusVarRatio(pcAtts->GetTubeRadiusVarRatio());
            }
        }
        else
        {
            ResettingError(tr("Tube variable radius ratio"),
                DoubleToQString(pcAtts->GetTubeRadiusVarRatio()));
            pcAtts->SetTubeRadiusVarRatio(pcAtts->GetTubeRadiusVarRatio());
        }
    }

    // End point resolution
    if (which_widget == PseudocolorAttributes::ID_endPointResolution|| doAll)
    {
        // This can only be an integer, so no error checking is needed.
        if (endPointResolution->value() != pcAtts->GetEndPointResolution())
            pcAtts->SetEndPointResolution(endPointResolution->value());
    }

    // End point radius
    if ((which_widget == PseudocolorAttributes::ID_endPointRadiusAbsolute || doAll)
        && pcAtts->GetEndPointRadiusSizeType() == PseudocolorAttributes::Absolute)
    {
        double val;
        if(LineEditGetDouble(endPointRadius, val))
        {
            if (val > 0.0)
                pcAtts->SetEndPointRadiusAbsolute(val);
            else
            {
                ResettingError(tr("End point radius must be > 0.0"),
                               DoubleToQString(pcAtts->GetEndPointRadiusAbsolute()));
                pcAtts->SetEndPointRadiusAbsolute(pcAtts->GetEndPointRadiusAbsolute());
            }
        }
        else
        {
            ResettingError(tr("End point radius"),
                DoubleToQString(pcAtts->GetEndPointRadiusAbsolute()));
            pcAtts->SetEndPointRadiusAbsolute(pcAtts->GetEndPointRadiusAbsolute());
        }
    }

    if ((which_widget == PseudocolorAttributes::ID_endPointRadiusBBox || doAll)
        && pcAtts->GetEndPointRadiusSizeType() == PseudocolorAttributes::FractionOfBBox)
    {
        double val;
        if(LineEditGetDouble(endPointRadius, val))
        {
            if (val > 0.0)
                pcAtts->SetEndPointRadiusBBox(val);
            else
            {
                ResettingError(tr("End point radius must be > 0.0"),
                               DoubleToQString(pcAtts->GetEndPointRadiusBBox()));
                pcAtts->SetEndPointRadiusBBox(pcAtts->GetEndPointRadiusBBox());
            }
        }
        else
        {
            ResettingError(tr("End point radius"),
                DoubleToQString(pcAtts->GetEndPointRadiusBBox()));
            pcAtts->SetEndPointRadiusBBox(pcAtts->GetEndPointRadiusBBox());
        }
    }

    if (which_widget == PseudocolorAttributes::ID_endPointRatio || doAll)
    {
        double val;
        if(LineEditGetDouble(endPointRatio, val))
        {
            if (val > 0.0)
              pcAtts->SetEndPointRatio(val);
            else
            {
              ResettingError(tr("End point height ratio must be > 0.0"),
                             DoubleToQString(pcAtts->GetEndPointRatio()));
              pcAtts->SetEndPointRatio(pcAtts->GetEndPointRatio());
            }
        }
        else
        {
            ResettingError(tr("endPoint ratio"),
                DoubleToQString(pcAtts->GetEndPointRatio()));
            pcAtts->SetEndPointRatio(pcAtts->GetEndPointRatio());
        }
    }
    // End point radius variable ratio
    if (which_widget == PseudocolorAttributes::ID_endPointRadiusVarRatio || doAll)
    {
        double val;
        if(LineEditGetDouble(endPointRadiusVarRatio, val))
        {
            if (val >= 1.0)
                pcAtts->SetEndPointRadiusVarRatio(val);
            else
            {
                ResettingError(tr("EndPoint variable radius ratio must be >= 1.0"),
                               DoubleToQString(pcAtts->GetEndPointRadiusVarRatio()));
                pcAtts->SetEndPointRadiusVarRatio(pcAtts->GetEndPointRadiusVarRatio());
            }
        }
        else
        {
            ResettingError(tr("EndPoint variable radius ratio"),
                DoubleToQString(pcAtts->GetEndPointRadiusVarRatio()));
            pcAtts->SetEndPointRadiusVarRatio(pcAtts->GetEndPointRadiusVarRatio());
        }
    }
    // Do the point size value and point size var value.
    if(doAll)
    {
        pcAtts->SetPointSize(pointControl->GetPointSize());
        pcAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        pcAtts->SetPointSizeVar(pointControl->GetPointSizeVar().toStdString());
    }
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::Apply
//
// Purpose:
//   This method applies the pc attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            pc attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:22:16 PST 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 06:59:33 PST 2001
//    I modified the routine to pass to the viewer proxy the plot
//    type stored within the class instead of the one hardwired from
//    an include file.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        pcAtts->Notify();

        // Tell the viewer to set the pc attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        pcAtts->Notify();
}

//
// Qt Slot functions...
//

void
QvisPseudocolorPlotWindow::apply()
{
    Apply(true);
}

void
QvisPseudocolorPlotWindow::makeDefault()
{
    // Tell the viewer to set the default pc attributes.
    GetCurrentValues(-1);
    pcAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

void
QvisPseudocolorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

void
QvisPseudocolorPlotWindow::scaleClicked(int scale)
{
    // Only do it if it changed.
    if(scale != pcAtts->GetScaling())
    {
        pcAtts->SetScaling(PseudocolorAttributes::Scaling(scale));
        Apply();
    }
}

void
QvisPseudocolorPlotWindow::processSkewText()
{
    GetCurrentValues(PseudocolorAttributes::ID_skewFactor);
    Apply();
}

void
QvisPseudocolorPlotWindow::limitsSelectChanged(int mode)
{
    // Only do it if it changed.
    if(mode != pcAtts->GetLimitsMode())
    {
        pcAtts->SetLimitsMode(PseudocolorAttributes::LimitsMode(mode));
        Apply();
    }
}

void
QvisPseudocolorPlotWindow::processMinLimitText()
{
    GetCurrentValues(PseudocolorAttributes::ID_min);
    Apply();
}

void
QvisPseudocolorPlotWindow::processMaxLimitText()
{
    GetCurrentValues(PseudocolorAttributes::ID_max);
    Apply();
}

void
QvisPseudocolorPlotWindow::minToggled(bool val)
{
    pcAtts->SetMinFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::maxToggled(bool val)
{
    pcAtts->SetMaxFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::belowMinToggled(bool val)
{
    pcAtts->SetUseBelowMinColor(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::aboveMaxToggled(bool val)
{
    pcAtts->SetUseAboveMaxColor(val);
    Apply();
}



// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::belowMinColorChanged
//
// Purpose:
//   This is a Qt slot function that is called when the belowMin color button's
//   color changes.
//
// Arguments:
//   color : The new color for values below minimum.
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::belowMinColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    pcAtts->SetBelowMinColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::aboveMaxColorChanged
//
// Purpose:
//   This is a Qt slot function that is called when the aboveMax color button's
//   color changes.
//
// Arguments:
//   color : The new color for values above maximum.
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::aboveMaxColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    pcAtts->SetAboveMaxColor(temp);
    Apply();
}


void
QvisPseudocolorPlotWindow::centeringClicked(int button)
{
    // Only do it if it changed.
    if(button != pcAtts->GetCentering())
    {
        pcAtts->SetCentering(PseudocolorAttributes::Centering(button));
        Apply();
    }
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::colorTableClicked
//
// Purpose:
//   This is a Qt slot function that sets the desired color table into the
//   pseudocolor plot attributes.
//
// Arguments:
//   useDefault : Whether or not to use the default color table.
//   ctName     : The name of the color table to use.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 18:58:03 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    pcAtts->SetColorTableName(ctName.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::invertColorTableToggled
//
// Purpose:
//   This is a Qt slot function that sets the invert color table flag into the
//   pseudocolor plot attributes.
//
// Arguments:
//   val    :  Whether or not to invert the color table.
//
// Programmer: Kathleen Bonnell
// Creation:   January  17, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::invertColorTableToggled(bool val)
{
    pcAtts->SetInvertColorTable(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::opacityTypeChanged(int val)
{
    pcAtts->SetOpacityType((PseudocolorAttributes::OpacityType)val);
    Apply();
}

void
QvisPseudocolorPlotWindow::opacityVariableChanged(const QString &var)
{
    pcAtts->SetOpacityVariable(var.toStdString());
    Apply();
}

void
QvisPseudocolorPlotWindow::opacityChanged(int opacity, const void*)
{
    pcAtts->SetOpacity((double)opacity/255.);
    Apply();
}


void
QvisPseudocolorPlotWindow::opacityMinToggled(bool val)
{
    pcAtts->SetOpacityVarMinFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::opacityMaxToggled(bool val)
{
    pcAtts->SetOpacityVarMaxFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::processOpacityVarMin()
{
    GetCurrentValues(PseudocolorAttributes::ID_opacityVarMin);
    Apply();
}

void
QvisPseudocolorPlotWindow::processOpacityVarMax()
{
    GetCurrentValues(PseudocolorAttributes::ID_opacityVarMax);
    Apply();
}

// ****************************************************************************
//  Method:  QvisPseudocolorPlotWindow::pointTypeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the point types is clicked.
//
//  Arguments:
//    type   :   The new type
//
//  Programmer:  Hank Childs
//  Creation:    August 21, 2003
//
//  Modifications:
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//   When doing GL_POINT, we ignore point size, so also disable it.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointTypeChanged(int type)
{
    pcAtts->SetPointType((GlyphType) type);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeVarToggled
//
// Purpose:
//   This is a Qt slot function that is called when the pointSizeVar toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the pointSizeVar toggle.
//
// Programmer: Jeremy Meredith
// Creation:   December 20, 2002
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointSizeVarToggled(bool val)
{
    pcAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeChanged
//
// Purpose:
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Kathleen Bonnell
// Creation:   November 12, 2004
//
// Modifications:
//
// ****************************************************************************
void
QvisPseudocolorPlotWindow::pointSizeChanged(double size)
{
    pcAtts->SetPointSize(size);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizePixelsChanged
//
// Purpose:
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:25:58 PST 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisPseudocolorPlotWindow::pointSizePixelsChanged(int size)
{
    pcAtts->SetPointSizePixels(size);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeVarChanged
//
// Purpose:
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and presses the Enter key.
//
// Programmer: Jeremy Meredith
// Creation:   December 20, 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointSizeVarChanged(const QString &var)
{
    pcAtts->SetPointSizeVar(var.toStdString());
    Apply();
}


// ****************************************************************************
// Method: QvisContourPlotWindow::lineTypeChanged
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   line type is changed.
//
// Arguments:
//   newType : The new line type.
//
// Programmer: Jeremy Meredith
// Creation:   November 26, 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::lineTypeChanged(int newType)
{
    pcAtts->SetLineType((PseudocolorAttributes::LineType)newType);
    Apply();
}


// ****************************************************************************
// Method: QvisContourPlotWindow::lineWidthChanged
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   line width widget is changed.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Jeremy Meredith
// Creation:   November 26, 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::lineWidthChanged(int newWidth)
{
    pcAtts->SetLineWidth(newWidth);
    Apply();
}


void
QvisPseudocolorPlotWindow::tubeResolutionChanged(int val)
{
    pcAtts->SetTubeResolution(val);
    Apply();
}


void
QvisPseudocolorPlotWindow::tubeRadiusProcessText()
{
    GetCurrentValues(PseudocolorAttributes::ID_tubeRadiusAbsolute);
    GetCurrentValues(PseudocolorAttributes::ID_tubeRadiusBBox);
    Apply();
}

void
QvisPseudocolorPlotWindow::tubeRadiusSizeTypeChanged(int v)
{
    pcAtts->SetTubeRadiusSizeType((PseudocolorAttributes::SizeType) v);
    Apply();
}

void
QvisPseudocolorPlotWindow::tubeRadiusVarToggled(bool val)
{
    pcAtts->SetTubeRadiusVarEnabled( val );
    Apply();
}

void
QvisPseudocolorPlotWindow::tubeRadiusVarChanged(const QString &var)
{
    pcAtts->SetTubeRadiusVar(var.toStdString());
    Apply();
}

void
QvisPseudocolorPlotWindow::tubeRadiusVarRatioProcessText()
{
    GetCurrentValues(PseudocolorAttributes::ID_tubeRadiusVarRatio);
    Apply();
}

void
QvisPseudocolorPlotWindow::tailStyleChanged(int newStyle)
{
    pcAtts->SetTailStyle((PseudocolorAttributes::EndPointStyle)newStyle);
    Apply();
}

void
QvisPseudocolorPlotWindow::headStyleChanged(int newStyle)
{
    pcAtts->SetHeadStyle((PseudocolorAttributes::EndPointStyle)newStyle);
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRadiusProcessText()
{
    GetCurrentValues(PseudocolorAttributes::ID_endPointRadiusAbsolute);
    GetCurrentValues(PseudocolorAttributes::ID_endPointRadiusBBox);
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointResolutionChanged(int val)
{
    pcAtts->SetEndPointResolution(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRatioProcessText()
{
    GetCurrentValues(PseudocolorAttributes::ID_endPointRatio);
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRadiusSizeTypeChanged(int v)
{
    pcAtts->SetEndPointRadiusSizeType((PseudocolorAttributes::SizeType) v);
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRadiusVarToggled(bool val)
{
    pcAtts->SetEndPointRadiusVarEnabled( val );
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRadiusVarChanged(const QString &var)
{
    pcAtts->SetEndPointRadiusVar(var.toStdString());
    Apply();
}

void
QvisPseudocolorPlotWindow::endPointRadiusVarRatioProcessText()
{
    GetCurrentValues(PseudocolorAttributes::ID_endPointRadiusVarRatio);
    Apply();
}


// ****************************************************************************
//  Method:  QvisPseudocolorPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December  9, 2002
//
//  Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::smoothingLevelChanged(int level)
{
    pcAtts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}

void
QvisPseudocolorPlotWindow::renderSurfacesChanged(bool val)
{
    pcAtts->SetRenderSurfaces(val);
    SetUpdate(false);
    Apply();
}

void
QvisPseudocolorPlotWindow::renderWireframeChanged(bool val)
{
    pcAtts->SetRenderWireframe(val);
    wireframeRenderColor->setEnabled(val);
    SetUpdate(false);
    Apply();
}

void
QvisPseudocolorPlotWindow::renderPointsChanged(bool val)
{
    pcAtts->SetRenderPoints(val);
    pointsRenderColor->setEnabled(val);
    SetUpdate(false);
    Apply();
}

void
QvisPseudocolorPlotWindow::legendToggled(bool val)
{
    pcAtts->SetLegendFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::lightingToggled(bool val)
{
    pcAtts->SetLightingFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::wireframeColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    pcAtts->SetWireframeColor(temp);
    SetUpdate(false);
    Apply();
}

void
QvisPseudocolorPlotWindow::pointColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    pcAtts->SetPointColor(temp);
    SetUpdate(false);
    Apply();
}

// change visibility of certain controls based on
// rendering type(s) in effect.

void
QvisPseudocolorPlotWindow::lineSettings()
{
    bool canGlyphLines = pcAtts->GetRenderSurfaces();
    lineType->setEnabled(canGlyphLines);
    lineTypeLabel->setEnabled(canGlyphLines);
    if (canGlyphLines)
    {
        if( pcAtts->GetLineType() == PseudocolorAttributes::Line )
        {
            lineWidthLabel->show();
            lineWidth->show();

            tubeResolutionLabel->hide();
            tubeResolution->hide();
            tubeRadiusLabel->hide();
            tubeRadius->hide();
            tubeRadiusSizeType->hide();
            tubeRadiusVarEnabled->hide();
            tubeRadiusVar->hide();
            tubeRadiusVar->hide();
            tubeRadiusVarRatioLabel->hide();
            tubeRadiusVarRatio->hide();
        }
        else
        {
            lineWidthLabel->hide();
            lineWidth->hide();

            if( pcAtts->GetLineType() == PseudocolorAttributes::Tube )
            {
                tubeResolutionLabel->show();
                tubeResolution->show();
            }
            else if( pcAtts->GetLineType() == PseudocolorAttributes::Ribbon )
            {
                tubeResolutionLabel->hide();
                tubeResolution->hide();
            }

            tubeRadiusLabel->show();
            tubeRadius->show();
            tubeRadiusSizeType->show();
            tubeRadiusVarEnabled->show();
            tubeRadiusVar->show();
            tubeRadiusVar->show();
            tubeRadiusVarRatioLabel->show();
            tubeRadiusVarRatio->show();
        }
    }
    else
    {
        //if doing wireframe, only want linewidth
        lineWidthLabel->show();
        lineWidth->show();

        tubeResolutionLabel->hide();
        tubeResolution->hide();
        tubeRadiusLabel->hide();
        tubeRadius->hide();
        tubeRadiusSizeType->hide();
        tubeRadiusVarEnabled->hide();
        tubeRadiusVar->hide();
        tubeRadiusVar->hide();
        tubeRadiusVarRatioLabel->hide();
        tubeRadiusVarRatio->hide();
    }
}

void
QvisPseudocolorPlotWindow::endPointSettings()
{
    bool canGlyphPoints = pcAtts->GetRenderSurfaces();
    headStyle->setEnabled(canGlyphPoints);
    headStyleLabel->setEnabled(canGlyphPoints);
    tailStyle->setEnabled(canGlyphPoints);
    tailStyleLabel->setEnabled(canGlyphPoints);

    if(canGlyphPoints)
    {

        bool showEndPointAttributes =
            pcAtts->GetTailStyle() != PseudocolorAttributes::None ||
            pcAtts->GetHeadStyle() != PseudocolorAttributes::None;
        if (showEndPointAttributes)
        {
                endPointRadiusLabel->show();
                endPointRadius->show();
                endPointRadiusSizeType->show();
                endPointRatioLabel->show();
                endPointRatio->show();

                endPointRadiusVarEnabled->show();
                endPointRadiusVar->show();
                endPointRadiusVarRatioLabel->show();
                endPointRadiusVarRatio->show();

                endPointResolutionLabel->show();
                endPointResolution->show();
        }
        else
        {
                endPointRadiusLabel->hide();
                endPointRadius->hide();
                endPointRadiusSizeType->hide();
                endPointRatioLabel->hide();
                endPointRatio->hide();

                endPointRadiusVarEnabled->hide();
                endPointRadiusVar->hide();
                endPointRadiusVarRatioLabel->hide();
                endPointRadiusVarRatio->hide();

                endPointResolutionLabel->hide();
                endPointResolution->hide();
        }

        if (pcAtts->GetTailStyle() == PseudocolorAttributes::Cones ||
            pcAtts->GetHeadStyle() == PseudocolorAttributes::Cones)
        {
                endPointRatioLabel->show();
                endPointRatio->show();
        }
        else
        {
                endPointRatioLabel->hide();
                endPointRatio->hide();
        }

        endPointRadiusLabel->setEnabled(showEndPointAttributes);
        endPointRadius->setEnabled(showEndPointAttributes);
        endPointRadiusSizeType->setEnabled(showEndPointAttributes);
        endPointRatioLabel->setEnabled(showEndPointAttributes);
        endPointRatio->setEnabled(showEndPointAttributes);

        endPointResolutionLabel->setEnabled(showEndPointAttributes);
        endPointResolution->setEnabled(showEndPointAttributes);
    }
    else
    {
        endPointRadiusLabel->hide();
        endPointRadius->hide();
        endPointRadiusSizeType->hide();
        endPointRatioLabel->hide();
        endPointRatio->hide();

        endPointRadiusVarEnabled->hide();
        endPointRadiusVar->hide();
        endPointRadiusVarRatioLabel->hide();
        endPointRadiusVarRatio->hide();

        endPointResolutionLabel->hide();
        endPointResolution->hide();
    }
}

