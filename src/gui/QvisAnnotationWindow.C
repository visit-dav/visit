#include <stdio.h>
#include <QvisAnnotationWindow.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qradiobutton.h>

#include <QNarrowLineEdit.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <AnnotationAttributes.h>
#include <DataNode.h>
#include <ViewerProxy.h>


// ****************************************************************************
// Method: QvisAnnotationWindow::QvisAnnotationWindow
//
// Purpose: 
//   This is the constructor for the QvisAnnotationWindow class.
//
// Arguments:
//   subj    : The AnnotationAttributes object that the window observes.
//   caption : The string that appears in the window decorations.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:45:35 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 17:15:18 PST 2001
//   Initialized the active tab.
//
//   Brad Whitlock, Fri Feb 15 15:34:46 PST 2002
//   Initialized parentless widgets.
//
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Added more control over the axes tick marks and labels.
//
// ****************************************************************************

QvisAnnotationWindow::QvisAnnotationWindow(AnnotationAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    annotationAtts = subj;
    activeTab = 0;

    // Initialize parentless widgets.
    axisLabelsButtons2D = 0;
    axisTitlesButtons2D = 0;
    gridLinesButtons2D = 0;
    axisLabelsButtons = 0;
    gridLinesButtons = 0;
    axisTicksButtons = 0;
    backgroundStyleButtons = 0;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::~QvisAnnotationWindow
//
// Purpose: 
//   This is the destructor for the QvisAnnotationWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:46:30 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:34:19 PST 2002
//   Deleted parentless widgets.
//   
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Added more control over the axes tick marks and labels.
//
// ****************************************************************************

QvisAnnotationWindow::~QvisAnnotationWindow()
{
    annotationAtts = 0;

    // Delete parentless widgets.
    delete axisLabelsButtons2D;
    delete axisTitlesButtons2D;
    delete gridLinesButtons2D;
    delete axisLabelsButtons;
    delete gridLinesButtons;
    delete axisTicksButtons;
    delete backgroundStyleButtons;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateWindowContents
//
// Purpose: 
//   This method creates all of the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:46:53 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Fri Jul  6 14:48:53 PDT 2001
//   Enable gridlines for 2D.
//
//   Brad Whitlock, Mon Aug 27 14:37:28 PST 2001
//   Added color buttons to control the background and foreground colors.
//
//   Kathleen Bonnell, Fri Aug  3 15:04:32 PDT 2001 
//   Enable gridlines for 3D.  Added static edges to 3d axes-type options.
//
//   Brad Whitlock, Fri Sep 21 15:53:34 PST 2001
//   Changed the code so the triad and bbox toggles are not part of the
//   page3d groupbox.
//
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001
//   Added another option to axes3DTypeComboBox. 
//
//   Brad Whitlock, Wed Sep 19 15:40:50 PST 2001
//   I removed an unused layout.
//
//   Brad Whitlock, Thu Jan 10 08:34:41 PDT 2002
//   I added a checkbox to toggle the visibility of the user information.
//
//   Brad Whitlock, Thu Apr 11 11:43:49 PDT 2002
//   I added checkboxes for the legend and the database.
//
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Added more control over the axes tick marks and labels.
//
//   Eric Brugger, Tue Jun 24 16:04:01 PDT 2003
//   Added the ability to control the 2d axes line width and replaced the
//   2d font size setting with individual controls for setting the x label,
//   y label, x title, and y title font heights.
//
// ****************************************************************************

void
QvisAnnotationWindow::CreateWindowContents()
{
    QGridLayout *glayout = new QGridLayout(topLayout, 2, 2);

    // Create a toggle for the user information.
    userInfo = new QCheckBox("User information", central, "userInfo");
    connect(userInfo, SIGNAL(toggled(bool)),
            this, SLOT(userInfoChecked(bool)));
    glayout->addWidget(userInfo, 0, 0);

    // Create a toggle for the database information.
    databaseInfo = new QCheckBox("Database", central, "databaseInfo");
    connect(databaseInfo, SIGNAL(toggled(bool)),
            this, SLOT(databaseInfoChecked(bool)));
    glayout->addWidget(databaseInfo, 0, 1);

    // Create a toggle for the legend.
    legendInfo = new QCheckBox("Legend", central, "legendInfo");
    connect(legendInfo, SIGNAL(toggled(bool)),
            this, SLOT(legendChecked(bool)));
    glayout->addWidget(legendInfo, 1, 0);

    // Create the tab widget.
    QTabWidget *tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->addWidget(tabs);    

    //
    // Create the group of 2D-related widgets and add them as a tab.
    //
    page2D = new QVBox(central, "page2D");
    page2D->setSpacing(5);
    page2D->setMargin(10);
    tabs->addTab(page2D, "2D Options");

    QHBox *toggleHBox = new QHBox(page2D);
    axesFlagToggle2D = new QCheckBox("Draw axes", toggleHBox,
                                     "axesFlagToggle2D");
    connect(axesFlagToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(axesFlagChecked2D(bool)));

    axesAutoSetTicksToggle2D = new QCheckBox("Auto set ticks", toggleHBox,
                                         "axesAutoSetTicksToggle2D");
    connect(axesAutoSetTicksToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(axesAutoSetTicksChecked2D(bool)));

    axesGroup2D = new QGroupBox(page2D, "axesGroup2D");
    axesGroup2D->setFrameStyle(QFrame::NoFrame);
    QGridLayout *lLayout = new QGridLayout(axesGroup2D, 11, 3);
    lLayout->setSpacing(5);

    // Create the X and Y Axes column headings
    QLabel *l =  new QLabel("X", axesGroup2D, "xAxesLabel");
    lLayout->addWidget(l, 0, 1);
    l =  new QLabel("Y", axesGroup2D, "yAxesLabel");
    lLayout->addWidget(l, 0, 2);

    // Create the group of check boxes for the 2D axis labels.
    l = new QLabel("Axis labels", axesGroup2D, "axesLabels2D");
    lLayout->addWidget(l, 1, 0);

    axisLabelsButtons2D = new QButtonGroup(0, "axisLabelsButtons2D");
    connect(axisLabelsButtons2D, SIGNAL(clicked(int)),
            this, SLOT(axisLabelsChanged2D(int)));
    QCheckBox *cb = new QCheckBox(axesGroup2D, "axisLabelsButtons_X");
    axisLabelsButtons2D->insert(cb);
    lLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox(axesGroup2D, "axisLabelsButtons_Y");
    axisLabelsButtons2D->insert(cb);
    lLayout->addWidget(cb, 1, 2);

    // Create the group of check boxes for the 2D axis titles.
    l = new QLabel("Axis titles", axesGroup2D, "axesTitles2D");
    lLayout->addWidget(l, 2, 0);

    axisTitlesButtons2D = new QButtonGroup(0, "axisTitlesButtons2D");
    connect(axisTitlesButtons2D, SIGNAL(clicked(int)),
            this, SLOT(axisTitlesChanged2D(int)));
    cb = new QCheckBox(axesGroup2D, "axisTitlesButtons_X");
    axisTitlesButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox(axesGroup2D, "axisTitlesButtons_Y");
    axisTitlesButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 2);

    // Create the group of check boxes for the 2D grid lines.
    l = new QLabel("Grid lines", axesGroup2D, "gridLines2D");
    lLayout->addWidget(l, 3, 0);

    gridLinesButtons2D = new QButtonGroup(0, "gridLinesButtons2D");
    connect(gridLinesButtons2D, SIGNAL(clicked(int)),
            this, SLOT(gridLinesChanged2D(int)));
    cb = new QCheckBox(axesGroup2D, "gridLinesButtons_X");
    gridLinesButtons2D->insert(cb);
    lLayout->addWidget(cb, 3, 1);
    cb = new QCheckBox(axesGroup2D, "gridLinesButtons_Y");
    gridLinesButtons2D->insert(cb);
    lLayout->addWidget(cb, 3, 2);

    // Create the text fields for the 2D major tick mark minimums.
    majorTickMinimumLabel2D = new QLabel("Major tick minimum", axesGroup2D,
                                         "majorTickMinimumLabel2D");
    lLayout->addWidget(majorTickMinimumLabel2D, 4, 0);

    xMajorTickMinimumLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xMajorTickMinimumLineEdit2D");
    connect(xMajorTickMinimumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickMinimumChanged2D()));
    lLayout->addWidget(xMajorTickMinimumLineEdit2D, 4, 1);
    yMajorTickMinimumLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yMajorTickMinimumLineEdit2D");
    connect(yMajorTickMinimumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickMinimumChanged2D()));
    lLayout->addWidget(yMajorTickMinimumLineEdit2D, 4, 2);

    // Create the text fields for the 2D major tick mark maximums.
    majorTickMaximumLabel2D = new QLabel("Major tick maximum", axesGroup2D,
                                         "majorTickMaximumLabel2D");
    lLayout->addWidget(majorTickMaximumLabel2D, 5, 0);

    xMajorTickMaximumLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xMajorTickMaximumLineEdit2D");
    connect(xMajorTickMaximumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickMaximumChanged2D()));
    lLayout->addWidget(xMajorTickMaximumLineEdit2D, 5, 1);
    yMajorTickMaximumLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yMajorTickMaximumLineEdit2D");
    connect(yMajorTickMaximumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickMaximumChanged2D()));
    lLayout->addWidget(yMajorTickMaximumLineEdit2D, 5, 2);

    // Create the text fields for the 2D major tick mark spacing.
    majorTickSpacingLabel2D = new QLabel("Major tick spacing", axesGroup2D,
                                         "majorTickSpacingLabel2D");
    lLayout->addWidget(majorTickSpacingLabel2D, 6, 0);

    xMajorTickSpacingLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xMajorTickSpacingLineEdit2D");
    connect(xMajorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickSpacingChanged2D()));
    lLayout->addWidget(xMajorTickSpacingLineEdit2D, 6, 1);
    yMajorTickSpacingLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yMajorTickSpacingLineEdit2D");
    connect(yMajorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickSpacingChanged2D()));
    lLayout->addWidget(yMajorTickSpacingLineEdit2D, 6, 2);

    // Create the text fields for the 2D minor tick mark spacing.
    minorTickSpacingLabel2D = new QLabel("Minor tick spacing", axesGroup2D,
                                         "minorTickSpacingLabel2D");
    lLayout->addWidget(minorTickSpacingLabel2D, 7, 0);

    xMinorTickSpacingLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xMinorTickSpacingLineEdit2D");
    connect(xMinorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMinorTickSpacingChanged2D()));
    lLayout->addWidget(xMinorTickSpacingLineEdit2D, 7, 1);
    yMinorTickSpacingLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yMinorTickSpacingLineEdit2D");
    connect(yMinorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMinorTickSpacingChanged2D()));
    lLayout->addWidget(yMinorTickSpacingLineEdit2D, 7, 2);

    // Create the text fields for the 2D label text height.
    l = new QLabel("Label font height", axesGroup2D, "labelFontHeightLabel2D");
    lLayout->addWidget(l, 8, 0);

    xLabelFontHeightLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xLabelFontHeightLineEdit2D");
    connect(xLabelFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xLabelFontHeightChanged2D()));
    lLayout->addWidget(xLabelFontHeightLineEdit2D, 8, 1);
    yLabelFontHeightLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yLabelFontHeightLineEdit2D");
    connect(yLabelFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yLabelFontHeightChanged2D()));
    lLayout->addWidget(yLabelFontHeightLineEdit2D, 8, 2);

    // Create the text fields for the 2D title text height.
    l = new QLabel("Title font height", axesGroup2D, "titleFontHeightLabel2D");
    lLayout->addWidget(l, 9, 0);

    xTitleFontHeightLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "xTitleFontHeightLineEdit2D");
    connect(xTitleFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xTitleFontHeightChanged2D()));
    lLayout->addWidget(xTitleFontHeightLineEdit2D, 9, 1);
    yTitleFontHeightLineEdit2D = new QNarrowLineEdit(axesGroup2D,
        "yTitleFontHeightLineEdit2D");
    connect(yTitleFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yTitleFontHeightChanged2D()));
    lLayout->addWidget(yTitleFontHeightLineEdit2D, 9, 2);

    // Create the 2D line width widget.
    axesLineWidth2D = new QvisLineWidthWidget(0, axesGroup2D,
        "axesLineWidth2D");
    lLayout->addMultiCellWidget(axesLineWidth2D, 10, 10, 1, 2);
    connect(axesLineWidth2D, SIGNAL(lineWidthChanged(int)),
            this, SLOT(axesLineWidthChanged2D(int)));
    l = new QLabel("Line width", axesGroup2D, "axesLineWidthLabel2D");
    lLayout->addWidget(l, 10, 0);

    // Create the 2D tick mark locations combobox.
    axesTickLocationComboBox2D = new QComboBox(axesGroup2D,
        "axesTickLocationComboBox2D");
    axesTickLocationComboBox2D->insertItem("Inside",  0);
    axesTickLocationComboBox2D->insertItem("Outside", 1);
    axesTickLocationComboBox2D->insertItem("Both",    2);
    connect(axesTickLocationComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTickLocationChanged2D(int)));
    lLayout->addMultiCellWidget(axesTickLocationComboBox2D, 11, 11, 1, 2);
    l = new QLabel(axesTickLocationComboBox2D, "Tick mark locations",
                   axesGroup2D, "axesTickLocationLabel2D");
    lLayout->addWidget(l, 11, 0);

    // Create the 2D tick marks combobox.
    axesTicksComboBox2D = new QComboBox(axesGroup2D, "axesTicksComboBox2D");
    axesTicksComboBox2D->insertItem("Off",         0);
    axesTicksComboBox2D->insertItem("Bottom",      1);
    axesTicksComboBox2D->insertItem("Left",        2);
    axesTicksComboBox2D->insertItem("Bottom-left", 3);
    axesTicksComboBox2D->insertItem("All axes",    4);
    connect(axesTicksComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTicksChanged2D(int)));
    lLayout->addMultiCellWidget(axesTicksComboBox2D, 12, 12, 1, 2);
    l = new QLabel(axesTicksComboBox2D, "Tick marks",
                   axesGroup2D, "axesTicksLabel2D");
    lLayout->addWidget(l, 12, 0);
 
    //
    // Create the group of 3D-related widgets.
    //
    page3D = new QVBox(central, "page3D");
    page3D->setSpacing(10);
    page3D->setMargin(10);
    tabs->addTab(page3D, "3D Options");

    axes3DFlagToggle = new QCheckBox("Draw axes", page3D, "axes3DFlagToggle");
    connect(axes3DFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(axes3DFlagChecked(bool)));

    axes3DGroup = new QGroupBox(page3D, "axes3DGroup");
    axes3DGroup->setFrameStyle(QFrame::NoFrame);
    QGridLayout *rLayout = new QGridLayout(axes3DGroup, 6, 4);
    rLayout->setSpacing(10);

    // Create the group of check boxes for the 3D axis labels.
    axisLabelsButtons = new QButtonGroup(0, "axisLabelsButtons");
    connect(axisLabelsButtons, SIGNAL(clicked(int)),
            this, SLOT(axisLabelsChanged(int)));
    cb = new QCheckBox("X", axes3DGroup, "axisLabelsButtons_X");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 0, 1);
    cb = new QCheckBox("Y", axes3DGroup, "axisLabelsButtons_Y");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 0, 2);
    cb = new QCheckBox("Z", axes3DGroup, "axisLabelsButtons_Z");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 0, 3);
    l = new QLabel(axisLabelsButtons, "Axis labels",
                   axes3DGroup, "axisLabelsLabel");
    rLayout->addWidget(l, 0, 0);

    // Create the group of check boxes for the 3D grid lines.
    gridLinesButtons = new QButtonGroup(0, "gridLinesButtons");
    connect(gridLinesButtons, SIGNAL(clicked(int)),
            this, SLOT(gridLinesChanged(int)));
    cb = new QCheckBox("X", axes3DGroup, "gridLinesButtons_X");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox("Y", axes3DGroup, "gridLinesButtons_Y");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 1, 2);
    cb = new QCheckBox("Z", axes3DGroup, "gridLinesButtons_Z");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 1, 3);
    l = new QLabel(gridLinesButtons, "Grid lines",
                   axes3DGroup, "gridLinesLabel");
    rLayout->addWidget(l, 1, 0);

    // Create the group of check boxes for the 3D tick marks.
    axisTicksButtons = new QButtonGroup(0, "axisTicksButtons");
    connect(axisTicksButtons, SIGNAL(clicked(int)),
            this, SLOT(axisTicksChanged(int)));
    cb = new QCheckBox("X", axes3DGroup, "axisTicksButtons_X");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox("Y", axes3DGroup, "axisTicksButtons_Y");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 2, 2);
    cb = new QCheckBox("Z", axes3DGroup, "axisTicksButtons_Z");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 2, 3);
    l = new QLabel(axisTicksButtons, "Tick marks",
                   axes3DGroup, "axisTicksLabel");
    rLayout->addWidget(l, 2, 0);

    // Create the 3D tick mark locations combobox.
    axes3DTickLocationComboBox = new QComboBox(axes3DGroup, "axes3DTickLocationComboBox");
    axes3DTickLocationComboBox->insertItem("Inside",  0);
    axes3DTickLocationComboBox->insertItem("Outside", 1);
    axes3DTickLocationComboBox->insertItem("Both",    2);
    connect(axes3DTickLocationComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTickLocationChanged(int)));
    rLayout->addMultiCellWidget(axes3DTickLocationComboBox, 3, 3, 1, 3);
    l = new QLabel(axes3DTickLocationComboBox, "Tick mark locations",
                   axes3DGroup, "axes3DTickLocationLabel");
    rLayout->addWidget(l, 3, 0);

    // Create the 3D axes type combobox.
    axes3DTypeComboBox = new QComboBox(axes3DGroup, "axes3DTypeComboBox");
    axes3DTypeComboBox->insertItem("Closest triad",  0);
    axes3DTypeComboBox->insertItem("Furthest triad", 1);
    axes3DTypeComboBox->insertItem("Outside edges",  2);
    axes3DTypeComboBox->insertItem("Static triad",   3);
    axes3DTypeComboBox->insertItem("Static edges",   4);
    connect(axes3DTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTypeChanged(int)));
    rLayout->addMultiCellWidget(axes3DTypeComboBox, 4, 4, 1, 3);
    l = new QLabel(axes3DTypeComboBox, "Axis type",
                   axes3DGroup, "axes3DTypeLabel");
    rLayout->addWidget(l, 4, 0);

    // Create the toggle for the triad.
    toggleHBox = new QHBox(page3D);
    triadFlagToggle = new QCheckBox("Triad", toggleHBox, "triadFlagToggle");
    connect(triadFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(triadFlagChecked(bool)));

    // Create the toggle for the bbox.
    bboxFlagToggle = new QCheckBox("Bounding box", toggleHBox, "bboxFlagToggle");
    connect(bboxFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(bboxFlagChecked(bool)));

    //
    // Create the group of color-related widgets.
    //
    pageColor = new QGroupBox(central, "pageColor");
    pageColor->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageColor, "Colors");

    QGridLayout *cLayout = new QGridLayout(pageColor, 3, 2);
    cLayout->setMargin(10);
    cLayout->setSpacing(5);

    // Add the background color widgets.
    backgroundColorButton = new QvisColorButton(pageColor, "backgroundColorButton");
    connect(backgroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(backgroundColorChanged(const QColor &)));
    QLabel *bgColorLabel = new QLabel(backgroundColorButton,
                                      "Background color", pageColor,
                                      "bgColorLabel");
    cLayout->addWidget(bgColorLabel, 0, 0);
    cLayout->addWidget(backgroundColorButton, 0, 1, AlignLeft);

    // Add the foreground color widgets.
    foregroundColorButton = new QvisColorButton(pageColor, "foregroundColorButton");
    connect(foregroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(foregroundColorChanged(const QColor &)));
    QLabel *fgColorLabel = new QLabel(foregroundColorButton,
                                      "Foreground color", pageColor,
                                      "fgColorLabel");
    cLayout->addWidget(fgColorLabel, 1, 0);
    cLayout->addWidget(foregroundColorButton, 1, 1, AlignLeft);

    // Create the background style widgets.
    QLabel *backgroundStyleLabel = new QLabel("Background style", pageColor,
                                              "backgroundStyleLabel");
    cLayout->addWidget(backgroundStyleLabel, 2, 0);
    backgroundStyleButtons = new QButtonGroup(0, "backgroundStyleButtons");
    connect(backgroundStyleButtons, SIGNAL(clicked(int)),
            this, SLOT(backgroundStyleChanged(int)));
    QRadioButton *solid = new QRadioButton("Solid", pageColor, "solid");
    backgroundStyleButtons->insert(solid);
    cLayout->addWidget(solid, 2, 1);
    QRadioButton *gradient = new QRadioButton("Gradient", pageColor, "gradient");
    backgroundStyleButtons->insert(gradient);
    cLayout->addWidget(gradient, 2, 2);

    // Create the gradient style combobox.
    gradientStyleComboBox = new QComboBox(pageColor, "gradientStyleComboBox");
    gradientStyleComboBox->insertItem("Top to bottom", 0);
    gradientStyleComboBox->insertItem("Bottom to top", 1);
    gradientStyleComboBox->insertItem("Left to right", 2);
    gradientStyleComboBox->insertItem("Right to left", 3);
    gradientStyleComboBox->insertItem("Radial",        4);
    connect(gradientStyleComboBox, SIGNAL(activated(int)),
            this, SLOT(gradientStyleChanged(int)));
    cLayout->addMultiCellWidget(gradientStyleComboBox, 3, 3, 1, 2);
    gradientStyleLabel = new QLabel(gradientStyleComboBox,
                                    "Gradient style", pageColor,
                                    "gradientStyleLabel");
    cLayout->addWidget(gradientStyleLabel, 3, 0);

    // Add the gradient color1 widgets.
    gradientColor1Button = new QvisColorButton(pageColor, "gradientColor1Button");
    connect(gradientColor1Button, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(gradientColor1Changed(const QColor &)));
    gradientColor1Label = new QLabel(gradientColor1Button,
                                     "Gradient color 1", pageColor,
                                     "gradColor1Label");
    cLayout->addWidget(gradientColor1Label, 4, 0);
    cLayout->addWidget(gradientColor1Button, 4, 1, AlignLeft);

    // Add the gradiant color2 widgets.
    gradientColor2Button = new QvisColorButton(pageColor, "gradientColor2Button");
    connect(gradientColor2Button, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(gradientColor2Changed(const QColor &)));
    gradientColor2Label = new QLabel(gradientColor2Button,
                                     "Gradient color 2", pageColor,
                                     "gradColor2Label");
    cLayout->addWidget(gradientColor2Label, 5, 0);
    cLayout->addWidget(gradientColor2Button, 5, 1, AlignLeft);

    // Show the appropriate page based on the activeTab setting.
    tabs->blockSignals(true);
    if(activeTab == 0)
        tabs->showPage(page2D);
    else if(activeTab == 1)
        tabs->showPage(page3D);
    else
        tabs->showPage(pageColor);
    tabs->blockSignals(false);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the annotation attributes object that the
//   window observes is changed. It is this method's responsibility to set the
//   state of the window's widgets to match the state of the annotation
//   attributes.
//
// Arguments:
//   doAll : A flag that tells the method to ignore which attributes are
//           selected. All widgets are updated if doAll is true.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:47:23 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Fri Jul  6 14:48:53 PDT 2001
//   Enable gridlines for 2D.
//
//   Brad Whitlock, Mon Aug 27 16:19:59 PST 2001
//   Added code to set the window colors.
//
//   Kathleen Bonnell, Fri Aug  3 15:04:32 PDT 2001 
//   Enable gridlines for 3D.
//
//   Brad Whitlock, Thu Sep 20 15:46:33 PST 2001
//   Changed code to match the new version of the state object.
//
//   Brad Whitlock, Thu Jan 10 08:41:32 PDT 2002
//   Added a toggle for the user info.
//
//   Brad Whitlock, Thu Apr 11 11:47:42 PDT 2002
//   Added a toggle for the database and the legend.
//
//   Eric Brugger, Tue Nov  5 07:53:55 PST 2002
//   Added more control over the axes tick marks and labels.
//
//   Eric Brugger, Fri Jan 24 11:27:09 PST 2003
//   Changed the way that the axes font sizes were set.
//
//   Eric Brugger, Tue Jun 24 16:04:01 PDT 2003
//   Added the ability to control the 2d axes line width and replaced the
//   2d font size setting with individual controls for setting the x label,
//   y label, x title, and y title font heights.
//
// ****************************************************************************

void
QvisAnnotationWindow::UpdateWindow(bool doAll)
{
    QColor  c;
    QString temp;
    bool isGradient, axesAutoSetTicks;
    bool vals[3];
    bool setAxisLabels2D = false;
    bool setAxisTitles2D = false;
    bool setGridLines2D = false;
    bool setAxisLabels = false;
    bool setAxisTitles = false;
    bool setGridLines = false;
    bool setAxisTicks = false;
    const unsigned char *cptr;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < annotationAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!annotationAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // axesFlag2D
            axesFlagToggle2D->blockSignals(true);
            axesFlagToggle2D->setChecked(annotationAtts->GetAxesFlag2D());
            axesFlagToggle2D->blockSignals(false);
            axesGroup2D->setEnabled(annotationAtts->GetAxesFlag2D());
            break;
        case 1: // axesAutoSetTicks2D
            axesAutoSetTicksToggle2D->blockSignals(true);
            axesAutoSetTicksToggle2D->setChecked(annotationAtts->GetAxesAutoSetTicks2D());
            axesAutoSetTicksToggle2D->blockSignals(false);

            axesAutoSetTicks = (annotationAtts->GetAxesAutoSetTicks2D() == 1);
            majorTickMinimumLabel2D->setEnabled(!axesAutoSetTicks);
            xMajorTickMinimumLineEdit2D->setEnabled(!axesAutoSetTicks);
            yMajorTickMinimumLineEdit2D->setEnabled(!axesAutoSetTicks);
            majorTickMaximumLabel2D->setEnabled(!axesAutoSetTicks);
            xMajorTickMaximumLineEdit2D->setEnabled(!axesAutoSetTicks);
            yMajorTickMaximumLineEdit2D->setEnabled(!axesAutoSetTicks);
            majorTickSpacingLabel2D->setEnabled(!axesAutoSetTicks);
            xMajorTickSpacingLineEdit2D->setEnabled(!axesAutoSetTicks);
            yMajorTickSpacingLineEdit2D->setEnabled(!axesAutoSetTicks);
            minorTickSpacingLabel2D->setEnabled(!axesAutoSetTicks);
            xMinorTickSpacingLineEdit2D->setEnabled(!axesAutoSetTicks);
            yMinorTickSpacingLineEdit2D->setEnabled(!axesAutoSetTicks);
            break;
        case 2: // xAxisLabels2D
        case 3: // yAxisLabels2D
            setAxisLabels2D = true;
            break;
        case 4: // xAxisTitle2D
        case 5: // yAxisTitle2D
            setAxisTitles2D = true;
            break;
        case 6: // xGridLines2D
        case 7: // yGridLines2D
            setGridLines2D = true;
            break;
        case 8: // xMajorTickMinimum2D
            temp.sprintf("%g", annotationAtts->GetXMajorTickMinimum2D());
            xMajorTickMinimumLineEdit2D->setText(temp);
            break;
        case 9: // yMajorTickMinimum2D
            temp.sprintf("%g", annotationAtts->GetYMajorTickMinimum2D());
            yMajorTickMinimumLineEdit2D->setText(temp);
            break;
        case 10: // xMajorTickMaximum2D
            temp.sprintf("%g", annotationAtts->GetXMajorTickMaximum2D());
            xMajorTickMaximumLineEdit2D->setText(temp);
            break;
        case 11: // yMajorTickMaximum2D
            temp.sprintf("%g", annotationAtts->GetYMajorTickMaximum2D());
            yMajorTickMaximumLineEdit2D->setText(temp);
            break;
        case 12: // xMajorTickSpacing2D
            temp.sprintf("%g", annotationAtts->GetXMajorTickSpacing2D());
            xMajorTickSpacingLineEdit2D->setText(temp);
            break;
        case 13: // yMajorTickSpacing2D
            temp.sprintf("%g", annotationAtts->GetYMajorTickSpacing2D());
            yMajorTickSpacingLineEdit2D->setText(temp);
            break;
        case 14: // xMinorTickSpacing2D
            temp.sprintf("%g", annotationAtts->GetXMinorTickSpacing2D());
            xMinorTickSpacingLineEdit2D->setText(temp);
            break;
        case 15: // yMinorTickSpacing2D
            temp.sprintf("%g", annotationAtts->GetYMinorTickSpacing2D());
            yMinorTickSpacingLineEdit2D->setText(temp);
            break;
        case 16: // xLabelFontHeight2D
            temp.sprintf("%g", annotationAtts->GetXLabelFontHeight2D());
            xLabelFontHeightLineEdit2D->setText(temp);
            break;
        case 17: // yLabelFontHeight2D
            temp.sprintf("%g", annotationAtts->GetYLabelFontHeight2D());
            yLabelFontHeightLineEdit2D->setText(temp);
            break;
        case 18: // xTitleFontHeight2D
            temp.sprintf("%g", annotationAtts->GetXTitleFontHeight2D());
            xTitleFontHeightLineEdit2D->setText(temp);
            break;
        case 19: // yTitleFontHeight2D
            temp.sprintf("%g", annotationAtts->GetYTitleFontHeight2D());
            yTitleFontHeightLineEdit2D->setText(temp);
            break;
        case 20: // axesLineWidth2D
            axesLineWidth2D->blockSignals(true);
            axesLineWidth2D->SetLineWidth(annotationAtts->GetAxesLineWidth2D());
            axesLineWidth2D->blockSignals(false);
            break;
        case 21: // axesTickLocation2D
            axesTickLocationComboBox2D->blockSignals(true);
            axesTickLocationComboBox2D->setCurrentItem(annotationAtts->GetAxesTickLocation2D());
            axesTickLocationComboBox2D->blockSignals(false);
            break;
        case 22: // axesTicks2D
            axesTicksComboBox2D->blockSignals(true);
            axesTicksComboBox2D->setCurrentItem(annotationAtts->GetAxesTicks2D());
            axesTicksComboBox2D->blockSignals(false);
            break;
        case 23: // axesFlag
            axes3DFlagToggle->blockSignals(true);
            axes3DFlagToggle->setChecked(annotationAtts->GetAxesFlag());
            axes3DFlagToggle->blockSignals(false);
            axes3DGroup->setEnabled(annotationAtts->GetAxesFlag());
            break;
        case 24: // axesAutoSetTicks
#if 0
            axesAutoSetTicksToggle->blockSignals(true);
            axesAutoSetTicksToggle->setChecked(annotationAtts->GetAutoSetTicks());
            axesAutoSetTicksToggle->blockSignals(false);
            // Make the tick locations text fields not enabled.
#endif
            break;
        case 25: // xAxisLabels
        case 26: // yAxisLabels
        case 27: // zAxisLabels
            setAxisLabels = true;
            break;
        case 28: // xAxisTitle
        case 29: // yAxisTitle
        case 30: // zAxisTitle
            setAxisTitles = true;
            break;
        case 31: // xGridLines
        case 32: // yGridLines
        case 33: // zGridLines
            setGridLines = true;
            break;
        case 34: // xAxisTicks
        case 35: // yAxisTicks
        case 36: // zAxisTicks
            setAxisTicks = true;
            break;
        case 37: // xMajorTickMinimum
        case 38: // yMajorTickMinimum
        case 39: // zMajorTickMinimum
        case 40: // xMajorTickMaximum
        case 41: // yMajorTickMaximum
        case 42: // zMajorTickMaximum
        case 43: // xMajorTickSpacing
        case 44: // yMajorTickSpacing
        case 45: // zMajorTickSpacing
        case 46: // xMinorTickSpacing
        case 47: // yMinorTickSpacing
        case 48: // zMinorTickSpacing
        case 49: // xLabelFontHeight
        case 50: // yLabelFontHeight
        case 51: // zLabelFontHeight
        case 52: // xTitleFontHeight
        case 53: // yTitleFontHeight
        case 54: // zTitleFontHeight
            // IMPLEMENT
            break;
        case 55: // axesTickLocation
            axes3DTickLocationComboBox->blockSignals(true);
            axes3DTickLocationComboBox->setCurrentItem(annotationAtts->GetAxesTickLocation());
            axes3DTickLocationComboBox->blockSignals(false);
            break;
        case 56: // axesType
            axes3DTypeComboBox->blockSignals(true);
            axes3DTypeComboBox->setCurrentItem(annotationAtts->GetAxesType());
            axes3DTypeComboBox->blockSignals(false);
            break;
        case 57: // triadFlag
            triadFlagToggle->blockSignals(true);
            triadFlagToggle->setChecked(annotationAtts->GetTriadFlag());
            triadFlagToggle->blockSignals(false);
            break;
        case 58: // bboxFlag
            bboxFlagToggle->blockSignals(true);
            bboxFlagToggle->setChecked(annotationAtts->GetBboxFlag());
            bboxFlagToggle->blockSignals(false);
            break;
        case 59: // backgroundColor
            cptr = annotationAtts->GetBackgroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            backgroundColorButton->blockSignals(true);
            backgroundColorButton->setButtonColor(c);
            backgroundColorButton->blockSignals(false);
            break;
        case 60: // foregroundColor
            cptr = annotationAtts->GetForegroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            foregroundColorButton->blockSignals(true);
            foregroundColorButton->setButtonColor(c);
            foregroundColorButton->blockSignals(false);
            break;
        case 61: // gradientBackgroundStyle
            gradientStyleComboBox->blockSignals(true);
            gradientStyleComboBox->setCurrentItem(annotationAtts->GetGradientBackgroundStyle());
            gradientStyleComboBox->blockSignals(false);
            break;
        case 62: // gradientColor1
            cptr = annotationAtts->GetGradientColor1().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor1Button->blockSignals(true);
            gradientColor1Button->setButtonColor(c);
            gradientColor1Button->blockSignals(false);
            break;
        case 63: // gradientColor2
            cptr = annotationAtts->GetGradientColor2().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor2Button->blockSignals(true);
            gradientColor2Button->setButtonColor(c);
            gradientColor2Button->blockSignals(false);
            break;
        case 64: // backgroundMode
            vals[0] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::Solid;
            vals[1] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::Gradient;
            SetButtonGroup(backgroundStyleButtons, vals);

            // Set widget sensitivity based on this field.
            isGradient = (annotationAtts->GetBackgroundMode() == 1);
            gradientStyleLabel->setEnabled(isGradient);
            gradientStyleComboBox->setEnabled(isGradient);
            gradientColor1Label->setEnabled(isGradient);
            gradientColor1Button->setEnabled(isGradient);
            gradientColor2Label->setEnabled(isGradient);
            gradientColor2Button->setEnabled(isGradient);
            break;
        case 65: // userInfo
            userInfo->blockSignals(true);
            userInfo->setChecked(annotationAtts->GetUserInfoFlag());
            userInfo->blockSignals(false);
            break;
        case 66: // databaseInfo
            databaseInfo->blockSignals(true);
            databaseInfo->setChecked(annotationAtts->GetDatabaseInfoFlag());
            databaseInfo->blockSignals(false);
            break;
        case 67: // legendInfo
            legendInfo->blockSignals(true);
            legendInfo->setChecked(annotationAtts->GetLegendInfoFlag());
            legendInfo->blockSignals(false);
            break;
        }
    } // end for

    // Set the buttons groups.
    if(setAxisLabels2D)
    {
        vals[0] = annotationAtts->GetXAxisLabels2D();
        vals[1] = annotationAtts->GetYAxisLabels2D();
        SetButtonGroup(axisLabelsButtons2D, vals);
    }

    if(setAxisTitles2D)
    {
        vals[0] = annotationAtts->GetXAxisTitle2D();
        vals[1] = annotationAtts->GetYAxisTitle2D();
        SetButtonGroup(axisTitlesButtons2D, vals);
    }

    if(setGridLines2D)
    {
        vals[0] = annotationAtts->GetXGridLines2D();
        vals[1] = annotationAtts->GetYGridLines2D();
        SetButtonGroup(gridLinesButtons2D, vals);
    }

    if(setAxisLabels)
    {
        vals[0] = annotationAtts->GetXAxisLabels();
        vals[1] = annotationAtts->GetYAxisLabels();
        vals[2] = annotationAtts->GetZAxisLabels();
        SetButtonGroup(axisLabelsButtons, vals);
    }

    if(setAxisTitles)
    {
#if 0
        vals[0] = annotationAtts->GetXAxisTitle();
        vals[1] = annotationAtts->GetYAxisTitle();
        vals[2] = annotationAtts->GetZAxisTitle();
        SetButtonGroup(axisTitlesButtons, vals);
#endif
    }

    if(setGridLines)
    {
        vals[0] = annotationAtts->GetXGridLines();
        vals[1] = annotationAtts->GetYGridLines();
        vals[2] = annotationAtts->GetZGridLines();
        SetButtonGroup(gridLinesButtons, vals);
    }

    if(setAxisTicks)
    {
        vals[0] = annotationAtts->GetXAxisTicks();
        vals[1] = annotationAtts->GetYAxisTicks();
        vals[2] = annotationAtts->GetZAxisTicks();
        SetButtonGroup(axisTicksButtons, vals);
    }
}

// ****************************************************************************
// Method: QvisViewWindow::GetCurrentValues
//
// Purpose:
//   Get the current values for the text fields.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   Eric Brugger, Tue Jun 24 16:04:01 PDT 2003
//   Added the ability to control the 2d axes line width and replaced the
//   2d font size setting with individual controls for setting the x label,
//   y label, x title, and y title font heights.
//
// ****************************************************************************

void
QvisAnnotationWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the 2d x major tick minimum value
    if (which_widget == 0 || doAll)
    {
        temp = xMajorTickMinimumLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXMajorTickMinimum2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X major tick mark minimum value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXMajorTickMinimum2D());
            Error(msg);
            annotationAtts->SetXMajorTickMinimum2D(
                annotationAtts->GetXMajorTickMinimum2D());
        }
    }

    // Do the 2d y major tick minimum value
    if (which_widget == 1 || doAll)
    {
        temp = yMajorTickMinimumLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYMajorTickMinimum2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y major tick mark minimum value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYMajorTickMinimum2D());
            Error(msg);
            annotationAtts->SetYMajorTickMinimum2D(
                annotationAtts->GetYMajorTickMinimum2D());
        }
    }

    // Do the 2d x major tick maximum value
    if (which_widget == 2 || doAll)
    {
        temp = xMajorTickMaximumLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXMajorTickMaximum2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X major tick mark maximum value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXMajorTickMaximum2D());
            Error(msg);
            annotationAtts->SetXMajorTickMaximum2D(
                annotationAtts->GetXMajorTickMaximum2D());
        }
    }

    // Do the 2d y major tick maximum value
    if (which_widget == 3 || doAll)
    {
        temp = yMajorTickMaximumLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYMajorTickMaximum2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y major tick mark maximum value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYMajorTickMaximum2D());
            Error(msg);
            annotationAtts->SetYMajorTickMaximum2D(
                annotationAtts->GetYMajorTickMaximum2D());
        }
    }

    // Do the 2d x major tick spacing value
    if (which_widget == 4 || doAll)
    {
        temp = xMajorTickSpacingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXMajorTickSpacing2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X major tick mark spacing value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXMajorTickSpacing2D());
            Error(msg);
            annotationAtts->SetXMajorTickSpacing2D(
                annotationAtts->GetXMajorTickSpacing2D());
        }
    }

    // Do the 2d y major tick spacing value
    if (which_widget == 5 || doAll)
    {
        temp = yMajorTickSpacingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYMajorTickSpacing2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y major tick mark spacing value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYMajorTickSpacing2D());
            Error(msg);
            annotationAtts->SetYMajorTickSpacing2D(
                annotationAtts->GetYMajorTickSpacing2D());
        }
    }

    // Do the 2d x minor tick spacing value
    if (which_widget == 6 || doAll)
    {
        temp = xMinorTickSpacingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXMinorTickSpacing2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X minor tick mark spacing value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXMinorTickSpacing2D());
            Error(msg);
            annotationAtts->SetXMinorTickSpacing2D(
                annotationAtts->GetXMinorTickSpacing2D());
        }
    }

    // Do the 2d y minor tick spacing value
    if (which_widget == 7 || doAll)
    {
        temp = yMinorTickSpacingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYMinorTickSpacing2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y minor tick mark spacing value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYMinorTickSpacing2D());
            Error(msg);
            annotationAtts->SetYMinorTickSpacing2D(
                annotationAtts->GetYMinorTickSpacing2D());
        }
    }

    // Do the 2d x label text height value
    if (which_widget == 8 || doAll)
    {
        temp = xLabelFontHeightLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXLabelFontHeight2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X label text height value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXLabelFontHeight2D());
            Error(msg);
            annotationAtts->SetXLabelFontHeight2D(
                annotationAtts->GetXLabelFontHeight2D());
        }
    }

    // Do the 2d y label text height value
    if (which_widget == 9 || doAll)
    {
        temp = yLabelFontHeightLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYLabelFontHeight2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y label text height value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYLabelFontHeight2D());
            Error(msg);
            annotationAtts->SetYLabelFontHeight2D(
                annotationAtts->GetYLabelFontHeight2D());
        }
    }

    // Do the 2d x title text height value
    if (which_widget == 10 || doAll)
    {
        temp = xTitleFontHeightLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetXTitleFontHeight2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X title text height value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetXTitleFontHeight2D());
            Error(msg);
            annotationAtts->SetXTitleFontHeight2D(
                annotationAtts->GetXTitleFontHeight2D());
        }
    }

    // Do the 2d y title text height value
    if (which_widget == 11 || doAll)
    {
        temp = yTitleFontHeightLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                annotationAtts->SetYTitleFontHeight2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y title text height value was invalid. "
                "Resetting to the last good value of %g.",
                annotationAtts->GetYTitleFontHeight2D());
            Error(msg);
            annotationAtts->SetYTitleFontHeight2D(
                annotationAtts->GetYTitleFontHeight2D());
        }
    }
}

// ****************************************************************************
// Method: QvisAnnotationWindow::SetButtonGroup
//
// Purpose: 
//   Sets the toggle state for all checkboxes in a button group.
//
// Arguments:
//   bg   : The button group that we're setting.
//   vals : An array containing the values that are used to set the toggles.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:49:34 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::SetButtonGroup(QButtonGroup *bg, bool *vals)
{
    bg->blockSignals(true);
    for(int i = 0; i < bg->count(); ++i)
    {
        QCheckBox *cb = (QCheckBox *)bg->find(i);
        cb->blockSignals(true);
        cb->setChecked(vals[i]);
        cb->blockSignals(false);
    }
    bg->blockSignals(false);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Apply
//
// Purpose: 
//   This method tells the annotation attributes to notify all observers of
//   changes in the object.
//
// Arguments:
//   dontIgnore : If this is true, the new state takes effect immediately.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:50:38 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Aug 30 10:46:21 PDT 2001
//   Updated the code to reflect name change to SetAnnotationAttributes.
//
//   Eric Brugger, Tue Nov  5 07:53:55 PST 2002
//   Added more control over the axes tick marks and labels.
//
// ****************************************************************************

void
QvisAnnotationWindow::Apply(bool dontIgnore)
{
    if(AutoUpdate() || dontIgnore)
    {
        // Get the current annotation attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);

        annotationAtts->Notify();
        viewer->SetAnnotationAttributes();
    }
    else
        annotationAtts->Notify();
}

// ****************************************************************************
// Method: QvisPostableWindow::CreateNode
//
// Purpose: 
//   Writes the window's extra information to the config file.
//
// Arguments:
//   parentNode : The node to which the window's attributes are added.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 17:11:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowObserver::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(std::string(caption().latin1()));

        // Save the current tab.
        node->AddNode(new DataNode("activeTab", activeTab));
    }
}

// ****************************************************************************
// Method: QvisAnnotationWindow::SetFromNode
//
// Purpose: 
//   Reads window attributes from the DataNode representation of the config
//   file.
//
// Arguments:
//   parentNode : The data node that contains the window's attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 17:09:08 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node;
    if((node = winNode->GetNode("activeTab")) != 0)
    {
        activeTab = node->AsInt();
        if(activeTab != 0 && activeTab != 1 && activeTab != 2)
            activeTab = 0;
    }

    // Call the base class's function.
    QvisPostableWindowObserver::SetFromNode(parentNode, borders);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::apply
//
// Purpose: 
//   This is a Qt slot function that applies the annotation attributes
//   unconditionally.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:52:03 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the "Make default" button
//   is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:59:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::makeDefault()
{
    // Tell the viewer to set the default annotation attributes.
    annotationAtts->Notify();
    viewer->SetDefaultAnnotationAttributes();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the "Reset" button
//   is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:59:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::reset()
{
    // Tell the viewer to reset the annotation attributes to the last applied
    // values.
    viewer->ResetAnnotationAttributes();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axesFlagChecked2D
//
// Purpose: 
//   This is a Qt slot function that sets the 2D flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:52:39 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Renamed the method and modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesFlagChecked2D(bool val)
{
    annotationAtts->SetAxesFlag2D(val);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axesAutoSetTicksChecked2D
//
// Purpose: 
//   This is a Qt slot function that sets the 2D auto set ticks flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesAutoSetTicksChecked2D(bool val)
{
    annotationAtts->SetAxesAutoSetTicks2D(val);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axisLabelsChanged2D
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 2d axes will
//   have labels.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1)
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axisLabelsChanged2D(int index)
{
    if(index == 0)
       annotationAtts->SetXAxisLabels2D(!annotationAtts->GetXAxisLabels2D());
    else if(index == 1)
       annotationAtts->SetYAxisLabels2D(!annotationAtts->GetYAxisLabels2D());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axisTitlesChanged2D
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 2d axes will
//   have titles.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1)
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axisTitlesChanged2D(int index)
{
    if(index == 0)
       annotationAtts->SetXAxisTitle2D(!annotationAtts->GetXAxisTitle2D());
    else if(index == 1)
       annotationAtts->SetYAxisTitle2D(!annotationAtts->GetYAxisTitle2D());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::gridLinesChanged2D
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 2d axes will
//   have grid lines.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1)
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::gridLinesChanged2D(int index)
{
    if(index == 0)
       annotationAtts->SetXGridLines2D(!annotationAtts->GetXGridLines2D());
    else if(index == 1)
       annotationAtts->SetYGridLines2D(!annotationAtts->GetYGridLines2D());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xMajorTickMinimumChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x major tick mark
//   minimum is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xMajorTickMinimumChanged2D()
{
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yMajorTickMinimumChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y major tick mark
//   minimum is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yMajorTickMinimumChanged2D()
{
    GetCurrentValues(1);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xMajorTickMaximumChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x major tick mark
//   maximum is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xMajorTickMaximumChanged2D()
{
    GetCurrentValues(2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yMajorTickMaximumChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y major tick mark
//   maximum is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yMajorTickMaximumChanged2D()
{
    GetCurrentValues(3);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xMajorTickSpacingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x major tick mark
//   spacing is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xMajorTickSpacingChanged2D()
{
    GetCurrentValues(4);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yMajorTickSpacingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y major tick mark
//   spacing is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yMajorTickSpacingChanged2D()
{
    GetCurrentValues(5);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xMinorTickSpacingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x minor tick mark
//   spacing is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xMinorTickSpacingChanged2D()
{
    GetCurrentValues(6);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yMinorTickSpacingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y minor tick mark
//   spacing is changed.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov  4 12:21:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yMinorTickSpacingChanged2D()
{
    GetCurrentValues(7);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xLabelFontHeightChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x label text
//   height is changed.
//
// Programmer: Eric Brugger
// Creation:   Tue Jun 24 16:04:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xLabelFontHeightChanged2D()
{
    GetCurrentValues(8);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yLabelFontHeightChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y label text
//   height is changed.
//
// Programmer: Eric Brugger
// Creation:   Tue Jun 24 16:04:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yLabelFontHeightChanged2D()
{
    GetCurrentValues(9);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xTitleFontHeightChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x title text
//   height is changed.
//
// Programmer: Eric Brugger
// Creation:   Tue Jun 24 16:04:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xTitleFontHeightChanged2D()
{
    GetCurrentValues(10);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yTitleFontHeightChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y title text
//   height is changed.
//
// Programmer: Eric Brugger
// Creation:   Tue Jun 24 16:04:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yTitleFontHeightChanged2D()
{
    GetCurrentValues(11);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axesLineWidthChanged2D
//
// Purpose:
//   This is a Qt slot function that is called when the 2d axes line width
//   is changed.
//
// Arguments:
//   index:    The new line width.
//
// Programmer: Eric Brugger
// Creation:   Tue Jun 24 16:04:01 PDT 2003
//
// Modifications:
//
// ****************************************************************************
 
void
QvisAnnotationWindow::axesLineWidthChanged2D(int index)
{
    annotationAtts->SetAxesLineWidth2D(index);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axes2DTicksChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which set of 2d
//   tick marks are to be used.
//
// Arguments:
//   index : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:56:11 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Renamed the method and modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesTicksChanged2D(int index)
{
    if (index == 0)
        annotationAtts->SetAxesTicks2D(AnnotationAttributes::Off);
    else if (index == 1)
        annotationAtts->SetAxesTicks2D(AnnotationAttributes::Bottom);
    else if (index == 2)
        annotationAtts->SetAxesTicks2D(AnnotationAttributes::Left);
    else if (index == 3)
        annotationAtts->SetAxesTicks2D(AnnotationAttributes::BottomLeft);
    else if (index == 4)
        annotationAtts->SetAxesTicks2D(AnnotationAttributes::All);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axes2DTickLocationChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes where the 2d tick
//   marks will appear.
//
// Arguments:
//   index : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:56:51 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Renamed the method and modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesTickLocationChanged2D(int index)
{
    if (index == 0)
        annotationAtts->SetAxesTickLocation2D(AnnotationAttributes::Inside);
    else if (index == 1)
        annotationAtts->SetAxesTickLocation2D(AnnotationAttributes::Outside);
    else if (index == 2)
        annotationAtts->SetAxesTickLocation2D(AnnotationAttributes::Both);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axes3DFlagChecked
//
// Purpose: 
//   This is a Qt slot function that sets the 3D flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:52:39 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axes3DFlagChecked(bool val)
{
    annotationAtts->SetAxesFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axisLabelsChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 3d axes will
//   have labels.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1, Z=2)
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:57:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axisLabelsChanged(int index)
{
    if(index == 0)
       annotationAtts->SetXAxisLabels(!annotationAtts->GetXAxisLabels());
    else if(index == 1)
       annotationAtts->SetYAxisLabels(!annotationAtts->GetYAxisLabels());
    else if(index == 2)
       annotationAtts->SetZAxisLabels(!annotationAtts->GetZAxisLabels());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::gridLinesChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 3d axes will
//   have grid lines.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1, Z=2)
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:57:47 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Renamed the method and modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::gridLinesChanged(int index)
{
    if(index == 0)
       annotationAtts->SetXGridLines(!annotationAtts->GetXGridLines());
    else if(index == 1)
       annotationAtts->SetYGridLines(!annotationAtts->GetYGridLines());
    else if(index == 2)
       annotationAtts->SetZGridLines(!annotationAtts->GetZGridLines());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axisTicksChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes which 3d axes will
//   have tick marks.
//
// Arguments:
//   index : The index of the axis. (e.g. X=0, Y=1, Z=2)
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:57:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axisTicksChanged(int index)
{
    if(index == 0)
       annotationAtts->SetXAxisTicks(!annotationAtts->GetXAxisTicks());
    else if(index == 1)
       annotationAtts->SetYAxisTicks(!annotationAtts->GetYAxisTicks());
    else if(index == 2)
       annotationAtts->SetZAxisTicks(!annotationAtts->GetZAxisTicks());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axes3DTickLocationChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes where the 3d tick
//   marks will appear.
//
// Arguments:
//   index : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jun 17 23:59:33 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axes3DTickLocationChanged(int index)
{
    if (index == 0)
        annotationAtts->SetAxesTickLocation(AnnotationAttributes::Inside);
    else if (index == 1)
        annotationAtts->SetAxesTickLocation(AnnotationAttributes::Outside);
    else if (index == 1)
        annotationAtts->SetAxesTickLocation(AnnotationAttributes::Both);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axes3DTypeChanged
//
// Purpose: 
//   This is a Qt slot function that tells the attributes how the axes will
//   be drawn.
//
// Arguments:
//   index : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 18 00:00:30 PDT 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axes3DTypeChanged(int index)
{
    if (index == 0)
        annotationAtts->SetAxesType(AnnotationAttributes::ClosestTriad);
    else if (index == 1)
        annotationAtts->SetAxesType(AnnotationAttributes::FurthestTriad);
    else if (index == 2)
        annotationAtts->SetAxesType(AnnotationAttributes::OutsideEdges);
    else if (index == 3)
        annotationAtts->SetAxesType(AnnotationAttributes::StaticTriad);
    else if (index == 4)
        annotationAtts->SetAxesType(AnnotationAttributes::StaticEdges);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::triadFlagChecked
//
// Purpose: 
//   This is a Qt slot function that tells the attributes whether or not the
//   3D triad should be drawn.
//
// Arguments:
//   val : Whether or not to draw the triad.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 18 00:01:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::triadFlagChecked(bool val)
{
    annotationAtts->SetTriadFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::bboxFlagChecked
//
// Purpose: 
//   This is a Qt slot function that tells the attributes whether or not to
//   in bbox mode. Is this correct?
//
// Arguments:
//   val : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 18 00:02:06 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::bboxFlagChecked(bool val)
{
    annotationAtts->SetBboxFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::backgroundColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the color is changed by
//   using the background color button.
//
// Arguments:
//   c : The new background color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 14:47:46 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 20 15:48:24 PST 2001
//   Modified so it conforms to the new state object.
//
// ****************************************************************************

void
QvisAnnotationWindow::backgroundColorChanged(const QColor &c)
{
    ColorAttribute newColor(c.red(), c.green(), c.blue());
    annotationAtts->SetBackgroundColor(newColor);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::foregroundColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the color is changed by
//   using the foreground color button.
//
// Arguments:
//   c : The new foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 14:47:46 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 20 15:47:51 PST 2001
//   Modified so it conforms to the new state object.
//
// ****************************************************************************

void
QvisAnnotationWindow::foregroundColorChanged(const QColor &c)
{
    ColorAttribute newColor(c.red(), c.green(), c.blue());
    annotationAtts->SetForegroundColor(newColor);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::gradientColor1Changed
//
// Purpose: 
//   This is a Qt slot function that is called when the first gradient color
//   is changed.
//
// Arguments:
//   c : The new color for the first gradient color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 17:25:42 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 20 15:49:12 PST 2001
//   Changed the code to conform to the new state object.
//   
// ****************************************************************************

void
QvisAnnotationWindow::gradientColor1Changed(const QColor &c)
{
    ColorAttribute newColor(c.red(), c.green(), c.blue());
    annotationAtts->SetGradientColor1(newColor);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::gradientColor2Changed
//
// Purpose: 
//   This is a Qt slot function that is called when the second gradient color
//   is changed.
//
// Arguments:
//   c : The new color for the second gradient color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 17:25:42 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 20 15:49:12 PST 2001
//   Changed the code to conform to the new state object.
//
// ****************************************************************************

void
QvisAnnotationWindow::gradientColor2Changed(const QColor &c)
{
    ColorAttribute newColor(c.red(), c.green(), c.blue());
    annotationAtts->SetGradientColor2(newColor);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::backgroundStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   background style radio buttons.
//
// Arguments:
//   index : The new background mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 17:28:26 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::backgroundStyleChanged(int index)
{
    if (index == 0)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::Solid);
    else if (index == 1)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::Gradient);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::gradientStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   gradient style radio buttons.
//
// Arguments:
//   index : The new gradient mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 17:28:26 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Nov  4 12:21:02 PST 2002
//   Modified to match changes in annotationAtts.
//   
// ****************************************************************************

void
QvisAnnotationWindow::gradientStyleChanged(int index)
{
    if (index == 0)
        annotationAtts->SetGradientBackgroundStyle(
            AnnotationAttributes::TopToBottom);
    else if (index == 1)
        annotationAtts->SetGradientBackgroundStyle(
            AnnotationAttributes::BottomToTop);
    else if (index == 2)
        annotationAtts->SetGradientBackgroundStyle(
            AnnotationAttributes::LeftToRight);
    else if (index == 3)
        annotationAtts->SetGradientBackgroundStyle(
            AnnotationAttributes::RightToLeft);
    else if (index == 4)
        annotationAtts->SetGradientBackgroundStyle(
            AnnotationAttributes::Radial);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::tabSelected
//
// Purpose: 
//   This is a Qt slot function that is called when the tabs are changed.
//
// Arguments:
//   index : The new active tab.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 17:14:17 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::tabSelected(const QString &tabLabel)
{
    if(tabLabel == QString("2D Options"))
        activeTab = 0;
    else if(tabLabel == QString("3D Options"))
        activeTab = 1;
    else
        activeTab = 2;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::userInfoChecked
//
// Purpose: 
//   This is a Qt slot function that sets the user info flag.
//
// Arguments:
//   val : The new user info value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 08:43:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::userInfoChecked(bool val)
{
    annotationAtts->SetUserInfoFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::databaseInfoChecked
//
// Purpose: 
//   This is a Qt slot function that sets the database info flag.
//
// Arguments:
//   val : The new database info value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 11:49:16 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::databaseInfoChecked(bool val)
{
    annotationAtts->SetDatabaseInfoFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::legendChecked
//
// Purpose: 
//   This is a Qt slot function that sets the legend info flag.
//
// Arguments:
//   val : The new legend info value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 11:49:52 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::legendChecked(bool val)
{
    annotationAtts->SetLegendInfoFlag(val);
    SetUpdate(false);
    Apply();
}
