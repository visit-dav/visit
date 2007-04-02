#include <stdio.h>
#include <QvisAnnotationWindow.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qgrid.h>
#include <qradiobutton.h>

#include <QNarrowLineEdit.h>
#include <QvisAnnotationObjectInterface.h>
#include <QvisAnnotationObjectInterfaceFactory.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <DataNode.h>
#include <ViewerProxy.h>

#include <DebugStream.h>

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
//   Brad Whitlock, Thu Oct 30 17:40:39 PST 2003
//   Changed it to a QvisPostableWindowSimpleObserver and added objButtonGroup.
//
// ****************************************************************************

QvisAnnotationWindow::QvisAnnotationWindow(const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
    QvisPostableWindowSimpleObserver::AllExtraButtons, false)
{
    // Initialize the subject pointers.
    annotationAtts = 0;
    annotationObjectList = 0;

    objectInterfaces = 0;
    nObjectInterfaces = 0;
    displayInterface = 0;

    activeTab = 0;

    // Initialize parentless widgets.
    axisLabelsButtons2D = 0;
    axisTitlesButtons2D = 0;
    gridLinesButtons2D = 0;
    axisLabelsButtons = 0;
    gridLinesButtons = 0;
    axisTicksButtons = 0;
    backgroundStyleButtons = 0;

    objButtonGroup = 0;
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
//   Brad Whitlock, Thu Oct 30 16:49:20 PST 2003
//   Added detachment of subjects and deletion of objButtonGroup.
//
// ****************************************************************************

QvisAnnotationWindow::~QvisAnnotationWindow()
{
    if(annotationAtts)
        annotationAtts->Detach(this);

    if(annotationObjectList)
        annotationObjectList->Detach(this);

    // Delete parentless widgets.
    delete axisLabelsButtons2D;
    delete axisTitlesButtons2D;
    delete gridLinesButtons2D;
    delete axisLabelsButtons;
    delete gridLinesButtons;
    delete axisTicksButtons;
    delete backgroundStyleButtons;

    delete objButtonGroup;
    delete [] objectInterfaces;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::ConnectAnnotationAttributes
//
// Purpose: 
//   Makes the window observe the annotation attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 14:29:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::ConnectAnnotationAttributes(AnnotationAttributes *a)
{
    annotationAtts = a;
    a->Attach(this);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::ConnectAnnotationObjectList.
//
// Purpose: 
//   Makes the window observe the annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 14:29:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::ConnectAnnotationObjectList(AnnotationObjectList *a)
{
    annotationObjectList = a;
    a->Attach(this);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::SubjectRemoved
//
// Purpose: 
//   This method disconnects a subject from the window.
//
// Arguments:
//   TheRemovedSubject : The subject to detach.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:16:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == annotationAtts)
        annotationAtts = 0;

    if(TheRemovedSubject == annotationObjectList)
        annotationObjectList = 0;
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
//   Brad Whitlock, Mon Nov 10 16:28:36 PST 2003
//   I added a button that can turn off all annotations.
//
//   Brad Whitlock, Thu Oct 30 16:36:49 PST 2003
//   I moved the code that creates the tabbed controls to different methods.
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

    // Create a button that can turn off all annotations.
    turnOffAllButton = new QPushButton("No annotations", central,
        "turnOffAllButton");
    connect(turnOffAllButton, SIGNAL(clicked()),
            this, SLOT(turnOffAllAnnotations()));
    glayout->addWidget(turnOffAllButton, 1, 1);

    // Create the tab widget.
    tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->addWidget(tabs);    

    //
    // Create the window's tabs.
    //
    Create2DTab();
    Create3DTab();
    CreateColorTab();
    CreateObjectsTab();

    // Show the appropriate page based on the activeTab setting.
    tabs->blockSignals(true);
    if(activeTab == 0)
        tabs->showPage(page2D);
    else if(activeTab == 1)
        tabs->showPage(page3D);
    else if(activeTab == 2)
        tabs->showPage(pageColor);
    else
        tabs->showPage(pageObjects);

    tabs->blockSignals(false);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Create2DTab
//
// Purpose: 
//   Creates the 2D options tab.
//
// Note:       I moved this code from CreateWindowContents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 16:35:59 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003
//   Added button for automatic label scaling, text fields for label exponents.
//
//   Brad Whitlock, Wed Jul 27 16:01:31 PST 2005
//   I made it create a "Grid and Ticks" tab and a "Title and Labels" tab.
//
// ****************************************************************************

void
QvisAnnotationWindow::Create2DTab()
{
    //
    // Create the group of 2D-related widgets and add them as a tab.
    //
    page2D = new QVBox(central, "page2D");
    page2D->setSpacing(5);
    page2D->setMargin(10);
    tabs->addTab(page2D, "2D");

    axesFlagToggle2D = new QCheckBox("Show axes", page2D,
                                     "axesFlagToggle2D");
    connect(axesFlagToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(axesFlagChecked2D(bool)));

    axesGroup2D = new QGroupBox(page2D, "axesGroup2D");
    axesGroup2D->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *lLayout = new QVBoxLayout(axesGroup2D);
    lLayout->setSpacing(5);
    QTabWidget *page2DTabs = new QTabWidget(axesGroup2D, "page2DTabs");
    lLayout->addWidget(page2DTabs);

    page2DTabs->addTab(Create2DTabForGridAndTicks(page2DTabs),
        "Grid and Ticks");
    page2DTabs->addTab(Create2DTabForTitleAndLabels(page2DTabs),
        "Title and Labels");
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Create2DTabForGridAndTicks
//
// Purpose: 
//   Creates the tab that we put the controls for "grid and ticks" on.
//
// Arguments:
//   parentWidget : The tab widget that will contain the controls.
//
// Returns:    The widget that contains all of the controls.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 09:08:43 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::Create2DTabForGridAndTicks(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "Create2DTabForGridAndTicks");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *lLayout = new QGridLayout(top, 10, 3);
    lLayout->setSpacing(5);
    lLayout->setMargin(5);

    axesAutoSetTicksToggle2D = new QCheckBox("Auto set ticks", top,
                                         "axesAutoSetTicksToggle2D");
    connect(axesAutoSetTicksToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(axesAutoSetTicksChecked2D(bool)));
    lLayout->addWidget(axesAutoSetTicksToggle2D, 0, 0);

    // Create the X and Y Axes column headings
    QLabel *l =  new QLabel("X", top, "xAxesLabel");
    lLayout->addWidget(l, 1, 1);
    l =  new QLabel("Y", top, "yAxesLabel");
    lLayout->addWidget(l, 1, 2);

    // Create the group of check boxes for the 2D grid lines.
    l = new QLabel("Show grid lines", top, "gridLines2D");
    lLayout->addWidget(l, 2, 0);

    gridLinesButtons2D = new QButtonGroup(0, "gridLinesButtons2D");
    connect(gridLinesButtons2D, SIGNAL(clicked(int)),
            this, SLOT(gridLinesChanged2D(int)));
    QCheckBox *cb = new QCheckBox(top, "gridLinesButtons_X");
    gridLinesButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox(top, "gridLinesButtons_Y");
    gridLinesButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 2);

    // Create the text fields for the 2D major tick mark minimums.
    majorTickMinimumLabel2D = new QLabel("Major tick minimum", top,
                                         "majorTickMinimumLabel2D");
    lLayout->addWidget(majorTickMinimumLabel2D, 3, 0);

    xMajorTickMinimumLineEdit2D = new QNarrowLineEdit(top,
        "xMajorTickMinimumLineEdit2D");
    connect(xMajorTickMinimumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickMinimumChanged2D()));
    lLayout->addWidget(xMajorTickMinimumLineEdit2D, 3, 1);
    yMajorTickMinimumLineEdit2D = new QNarrowLineEdit(top,
        "yMajorTickMinimumLineEdit2D");
    connect(yMajorTickMinimumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickMinimumChanged2D()));
    lLayout->addWidget(yMajorTickMinimumLineEdit2D, 3, 2);

    // Create the text fields for the 2D major tick mark maximums.
    majorTickMaximumLabel2D = new QLabel("Major tick maximum", top,
                                         "majorTickMaximumLabel2D");
    lLayout->addWidget(majorTickMaximumLabel2D, 4, 0);

    xMajorTickMaximumLineEdit2D = new QNarrowLineEdit(top,
        "xMajorTickMaximumLineEdit2D");
    connect(xMajorTickMaximumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickMaximumChanged2D()));
    lLayout->addWidget(xMajorTickMaximumLineEdit2D, 4, 1);
    yMajorTickMaximumLineEdit2D = new QNarrowLineEdit(top,
        "yMajorTickMaximumLineEdit2D");
    connect(yMajorTickMaximumLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickMaximumChanged2D()));
    lLayout->addWidget(yMajorTickMaximumLineEdit2D, 4, 2);

    // Create the text fields for the 2D major tick mark spacing.
    majorTickSpacingLabel2D = new QLabel("Major tick spacing", top,
                                         "majorTickSpacingLabel2D");
    lLayout->addWidget(majorTickSpacingLabel2D, 5, 0);

    xMajorTickSpacingLineEdit2D = new QNarrowLineEdit(top,
        "xMajorTickSpacingLineEdit2D");
    connect(xMajorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMajorTickSpacingChanged2D()));
    lLayout->addWidget(xMajorTickSpacingLineEdit2D, 5, 1);
    yMajorTickSpacingLineEdit2D = new QNarrowLineEdit(top,
        "yMajorTickSpacingLineEdit2D");
    connect(yMajorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMajorTickSpacingChanged2D()));
    lLayout->addWidget(yMajorTickSpacingLineEdit2D, 5, 2);

    // Create the text fields for the 2D minor tick mark spacing.
    minorTickSpacingLabel2D = new QLabel("Minor tick spacing", top,
                                         "minorTickSpacingLabel2D");
    lLayout->addWidget(minorTickSpacingLabel2D, 6, 0);

    xMinorTickSpacingLineEdit2D = new QNarrowLineEdit(top,
        "xMinorTickSpacingLineEdit2D");
    connect(xMinorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xMinorTickSpacingChanged2D()));
    lLayout->addWidget(xMinorTickSpacingLineEdit2D, 6, 1);
    yMinorTickSpacingLineEdit2D = new QNarrowLineEdit(top,
        "yMinorTickSpacingLineEdit2D");
    connect(yMinorTickSpacingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yMinorTickSpacingChanged2D()));
    lLayout->addWidget(yMinorTickSpacingLineEdit2D, 6, 2);

    // Create the 2D tick mark locations combobox.
    axesTickLocationComboBox2D = new QComboBox(top,
        "axesTickLocationComboBox2D");
    axesTickLocationComboBox2D->insertItem("Inside",  0);
    axesTickLocationComboBox2D->insertItem("Outside", 1);
    axesTickLocationComboBox2D->insertItem("Both",    2);
    connect(axesTickLocationComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTickLocationChanged2D(int)));
    lLayout->addMultiCellWidget(axesTickLocationComboBox2D, 7, 7, 1, 2);
    l = new QLabel(axesTickLocationComboBox2D, "Tick mark locations",
                   top, "axesTickLocationLabel2D");
    lLayout->addWidget(l, 7, 0);

    // Create the 2D tick marks combobox.
    axesTicksComboBox2D = new QComboBox(top, "axesTicksComboBox2D");
    axesTicksComboBox2D->insertItem("Off",         0);
    axesTicksComboBox2D->insertItem("Bottom",      1);
    axesTicksComboBox2D->insertItem("Left",        2);
    axesTicksComboBox2D->insertItem("Bottom-left", 3);
    axesTicksComboBox2D->insertItem("All axes",    4);
    connect(axesTicksComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTicksChanged2D(int)));
    lLayout->addMultiCellWidget(axesTicksComboBox2D, 8, 8, 1, 2);
    l = new QLabel(axesTicksComboBox2D, "Show tick marks",
                   top, "axesTicksLabel2D");
    lLayout->addWidget(l, 8, 0);

    // Create the 2D line width widget.
    axesLineWidth2D = new QvisLineWidthWidget(0, top,
        "axesLineWidth2D");
    lLayout->addMultiCellWidget(axesLineWidth2D, 9, 9, 1, 2);
    connect(axesLineWidth2D, SIGNAL(lineWidthChanged(int)),
            this, SLOT(axesLineWidthChanged2D(int)));
    l = new QLabel("Line width", top, "axesLineWidthLabel2D");
    lLayout->addWidget(l, 9, 0);

    return top0;
}


// ****************************************************************************
// Method: QvisAnnotationWindow::Create2DTabForTitleAndLabels
//
// Purpose: 
//   Creates the tab that we put the controls for "title and labels" on.
//
// Arguments:
//   parentWidget : The tab widget that will contain the controls.
//
// Returns:    The widget that contains all of the controls.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 09:08:43 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::Create2DTabForTitleAndLabels(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "Create2DTabForTitleAndLabels");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *lLayout = new QGridLayout(top, 11, 3);
    lLayout->setSpacing(5);
    lLayout->setMargin(5);

    labelAutoSetScalingToggle2D = new QCheckBox("Auto scale label values",
        top, "labelAutoSetScalingToggle2D");
    connect(labelAutoSetScalingToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(labelAutoSetScalingChecked2D(bool)));
    lLayout->addMultiCellWidget(labelAutoSetScalingToggle2D, 0, 0, 0, 1);

    // Create the X and Y Axes column headings
    QLabel *l =  new QLabel("X", top, "xAxesLabel");
    lLayout->addWidget(l, 1, 1);
    l =  new QLabel("Y", top, "yAxesLabel");
    lLayout->addWidget(l, 1, 2);

    // Create the group of check boxes for the 2D axis labels.
    l = new QLabel("Show labels", top, "axesLabels2D");
    lLayout->addWidget(l, 2, 0);
    axisLabelsButtons2D = new QButtonGroup(0, "axisLabelsButtons2D");
    connect(axisLabelsButtons2D, SIGNAL(clicked(int)),
            this, SLOT(axisLabelsChanged2D(int)));
    QCheckBox *cb = new QCheckBox(top, "axisLabelsButtons_X");
    axisLabelsButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox(top, "axisLabelsButtons_Y");
    axisLabelsButtons2D->insert(cb);
    lLayout->addWidget(cb, 2, 2);

    // Create the text fields for the 2D label text height.
    l = new QLabel("Label font height", top, "labelFontHeightLabel2D");
    lLayout->addWidget(l, 3, 0);
    xLabelFontHeightLineEdit2D = new QNarrowLineEdit(top,
        "xLabelFontHeightLineEdit2D");
    connect(xLabelFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xLabelFontHeightChanged2D()));
    lLayout->addWidget(xLabelFontHeightLineEdit2D, 3, 1);
    yLabelFontHeightLineEdit2D = new QNarrowLineEdit(top,
        "yLabelFontHeightLineEdit2D");
    connect(yLabelFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yLabelFontHeightChanged2D()));
    lLayout->addWidget(yLabelFontHeightLineEdit2D, 3, 2);

    // Create the text fields for the 2D label scaling.
    labelScalingLabel2D = new QLabel("Label scale (x10^?)", top, "labelScalingLabel2D");
    lLayout->addWidget(labelScalingLabel2D, 4, 0);
    xLabelScalingLineEdit2D = new QNarrowLineEdit(top,
        "xLabelScalingLineEdit2D");
    connect(xLabelScalingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xLabelScalingChanged2D()));
    lLayout->addWidget(xLabelScalingLineEdit2D, 4, 1);
    yLabelScalingLineEdit2D = new QNarrowLineEdit(top,
        "yLabelScalingLineEdit2D");
    connect(yLabelScalingLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yLabelScalingChanged2D()));
    lLayout->addWidget(yLabelScalingLineEdit2D, 4, 2);

    // Create the group of check boxes for the 2D axis titles.
    l = new QLabel("Show titles", top, "axesTitles2D");
    lLayout->addWidget(l, 5, 0);
    axisTitlesButtons2D = new QButtonGroup(0, "axisTitlesButtons2D");
    connect(axisTitlesButtons2D, SIGNAL(clicked(int)),
            this, SLOT(axisTitlesChanged2D(int)));
    cb = new QCheckBox(top, "axisTitlesButtons_X");
    axisTitlesButtons2D->insert(cb);
    lLayout->addWidget(cb, 5, 1);
    cb = new QCheckBox(top, "axisTitlesButtons_Y");
    axisTitlesButtons2D->insert(cb);
    lLayout->addWidget(cb, 5, 2);

    // Create the text fields for the 2D title text height.
    l = new QLabel("Title font height", top, "titleFontHeightLabel2D");
    lLayout->addWidget(l, 6, 0);
    xTitleFontHeightLineEdit2D = new QNarrowLineEdit(top,
        "xTitleFontHeightLineEdit2D");
    connect(xTitleFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xTitleFontHeightChanged2D()));
    lLayout->addWidget(xTitleFontHeightLineEdit2D, 6, 1);
    yTitleFontHeightLineEdit2D = new QNarrowLineEdit(top,
        "yTitleFontHeightLineEdit2D");
    connect(yTitleFontHeightLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yTitleFontHeightChanged2D()));
    lLayout->addWidget(yTitleFontHeightLineEdit2D, 6, 2);

    // Create a toggle and line edit for setting the X-axis title.
    xAxisUserTitleToggle2D = new QCheckBox("Set X-Axis title", top,
        "xAxisUserTitleToggle2D");
    connect(xAxisUserTitleToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(xAxisUserTitleChecked2D(bool)));
    lLayout->addWidget(xAxisUserTitleToggle2D, 7, 0);
    xAxisUserTitleLineEdit2D = new QNarrowLineEdit(top, 
        "xAxisUserTitleLineEdit2D");
    connect(xAxisUserTitleLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xAxisUserTitleLineEditChanged2D()));
    lLayout->addMultiCellWidget(xAxisUserTitleLineEdit2D, 7, 7, 1, 2);

    // Create a toggle and line edit for setting the X-axis Units.
    xAxisUserUnitsToggle2D = new QCheckBox("Set X-Axis units", top,
        "xAxisUserUnitsToggle2D");
    connect(xAxisUserUnitsToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(xAxisUserUnitsChecked2D(bool)));
    lLayout->addWidget(xAxisUserUnitsToggle2D, 8, 0);
    xAxisUserUnitsLineEdit2D = new QNarrowLineEdit(top, 
        "xAxisUserUnitsLineEdit2D");
    connect(xAxisUserUnitsLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(xAxisUserUnitsLineEditChanged2D()));
    lLayout->addMultiCellWidget(xAxisUserUnitsLineEdit2D, 8, 8, 1, 2);

    // Create a toggle and line edit for setting the Y-axis title.
    yAxisUserTitleToggle2D = new QCheckBox("Set Y-Axis title", top,
        "yAxisUserTitleToggle2D");
    connect(yAxisUserTitleToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(yAxisUserTitleChecked2D(bool)));
    lLayout->addWidget(yAxisUserTitleToggle2D, 9, 0);
    yAxisUserTitleLineEdit2D = new QNarrowLineEdit(top, 
        "yAxisUserTitleLineEdit2D");
    connect(yAxisUserTitleLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yAxisUserTitleLineEditChanged2D()));
    lLayout->addMultiCellWidget(yAxisUserTitleLineEdit2D, 9, 9, 1, 2);

    // Create a toggle and line edit for setting the Y-axis Units.
    yAxisUserUnitsToggle2D = new QCheckBox("Set Y-Axis units", top,
        "yAxisUserUnitsToggle2D");
    connect(yAxisUserUnitsToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(yAxisUserUnitsChecked2D(bool)));
    lLayout->addWidget(yAxisUserUnitsToggle2D, 10, 0);
    yAxisUserUnitsLineEdit2D = new QNarrowLineEdit(top, 
        "yAxisUserUnitsLineEdit2D");
    connect(yAxisUserUnitsLineEdit2D, SIGNAL(returnPressed()),
            this, SLOT(yAxisUserUnitsLineEditChanged2D()));
    lLayout->addMultiCellWidget(yAxisUserUnitsLineEdit2D, 10, 10, 1, 2);

    return top0;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Create3DTab
//
// Purpose: 
//   Creates the 3D options tab.
//
// Note:       I moved this code from CreateWindowContents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 16:34:53 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003
//   Added button for automatic label scaling, text fields for label exponents.
//
//   Brad Whitlock, Thu Jul 28 09:20:43 PDT 2005
//   I split it into two subtabs and added controls to set the axis titles
//   and units.
//
// ****************************************************************************

void
QvisAnnotationWindow::Create3DTab()
{
    //
    // Create the group of 3D-related widgets.
    //
    page3D = new QVBox(central, "page3D");
    page3D->setSpacing(10);
    page3D->setMargin(10);
    tabs->addTab(page3D, "3D");

    // Create the toggle for drawing the axes.
    axes3DFlagToggle = new QCheckBox("Show axes", page3D, "axes3DFlagToggle");
    connect(axes3DFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(axes3DFlagChecked(bool)));

    axes3DGroup = new QGroupBox(page3D, "axesGroup3D");
    axes3DGroup->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *lLayout = new QVBoxLayout(axes3DGroup);
    lLayout->setSpacing(5);
    QTabWidget *page3DTabs = new QTabWidget(axes3DGroup, "page3DTabs");
    lLayout->addWidget(page3DTabs);

    page3DTabs->addTab(Create3DTabForGridAndTicks(page3DTabs),
        "Grid and Ticks");
    page3DTabs->addTab(Create3DTabForTitleAndLabels(page3DTabs),
        "Title and Labels");

    // Create the toggle for the triad.
    QHBox *toggleHBox = new QHBox(page3D);
    triadFlagToggle = new QCheckBox("Show triad", toggleHBox, "triadFlagToggle");
    connect(triadFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(triadFlagChecked(bool)));

    // Create the toggle for the bbox.
    bboxFlagToggle = new QCheckBox("Show bounding box", toggleHBox, "bboxFlagToggle");
    connect(bboxFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(bboxFlagChecked(bool)));
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Create3DTabForGridAndTicks
//
// Purpose: 
//   Creates the grid and tick tab for 3D.
//
// Arguments:
//   parentWidget : The parent of all of the widgets to be created.
//
// Returns:    The widget that contains all of the widgets.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 11:00:29 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::Create3DTabForGridAndTicks(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "Create3DTabForGridAndTicks");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *rLayout = new QGridLayout(top, 4, 4);
    rLayout->setSpacing(5);
    rLayout->setMargin(5);

    // Create the group of check boxes for the 3D grid lines.
    gridLinesButtons = new QButtonGroup(0, "gridLinesButtons");
    connect(gridLinesButtons, SIGNAL(clicked(int)),
            this, SLOT(gridLinesChanged(int)));
    QCheckBox *cb = new QCheckBox("X", top, "gridLinesButtons_X");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 0, 1);
    cb = new QCheckBox("Y", top, "gridLinesButtons_Y");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 0, 2);
    cb = new QCheckBox("Z", top, "gridLinesButtons_Z");
    gridLinesButtons->insert(cb);
    rLayout->addWidget(cb, 0, 3);
    QLabel *l = new QLabel(gridLinesButtons, "Show grid lines",
        top, "gridLinesLabel");
    rLayout->addWidget(l, 0, 0);

    // Create the group of check boxes for the 3D tick marks.
    axisTicksButtons = new QButtonGroup(0, "axisTicksButtons");
    connect(axisTicksButtons, SIGNAL(clicked(int)),
            this, SLOT(axisTicksChanged(int)));
    cb = new QCheckBox("X", top, "axisTicksButtons_X");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox("Y", top, "axisTicksButtons_Y");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 1, 2);
    cb = new QCheckBox("Z", top, "axisTicksButtons_Z");
    axisTicksButtons->insert(cb);
    rLayout->addWidget(cb, 1, 3);
    l = new QLabel(axisTicksButtons, "Show tick marks",
                   top, "axisTicksLabel");
    rLayout->addWidget(l, 1, 0);

    // Create the 3D tick mark locations combobox.
    axes3DTickLocationComboBox = new QComboBox(top, "axes3DTickLocationComboBox");
    axes3DTickLocationComboBox->insertItem("Inside",  0);
    axes3DTickLocationComboBox->insertItem("Outside", 1);
    axes3DTickLocationComboBox->insertItem("Both",    2);
    connect(axes3DTickLocationComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTickLocationChanged(int)));
    rLayout->addMultiCellWidget(axes3DTickLocationComboBox, 2, 2, 1, 3);
    l = new QLabel(axes3DTickLocationComboBox, "Tick mark locations",
                   top, "axes3DTickLocationLabel");
    rLayout->addWidget(l, 2, 0);

    // Create the 3D axes type combobox.
    axes3DTypeComboBox = new QComboBox(top, "axes3DTypeComboBox");
    axes3DTypeComboBox->insertItem("Closest triad",  0);
    axes3DTypeComboBox->insertItem("Furthest triad", 1);
    axes3DTypeComboBox->insertItem("Outside edges",  2);
    axes3DTypeComboBox->insertItem("Static triad",   3);
    axes3DTypeComboBox->insertItem("Static edges",   4);
    connect(axes3DTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTypeChanged(int)));
    rLayout->addMultiCellWidget(axes3DTypeComboBox, 3, 3, 1, 3);
    l = new QLabel(axes3DTypeComboBox, "Axis type",
                   top, "axes3DTypeLabel");
    rLayout->addWidget(l, 3, 0);

    return top0;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Create3DTabForTitleAndLabels
//
// Purpose: 
//   Creates the title and labels tab for 3D.
//
// Arguments:
//   parentWidget : The parent of all of the widgets to be created.
//
// Returns:    The widget that contains all of the widgets.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 11:00:29 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::Create3DTabForTitleAndLabels(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "Create3DTabForTitleAndLabels");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *rLayout = new QGridLayout(top, 11, 3);
    rLayout->setSpacing(5);
    rLayout->setMargin(5);

    labelAutoSetScalingToggle = new QCheckBox("Auto scale label values", top,
                                         "labelAutoSetScalingToggle");
    connect(labelAutoSetScalingToggle, SIGNAL(toggled(bool)),
            this, SLOT(labelAutoSetScalingChecked(bool)));
    rLayout->addMultiCellWidget(labelAutoSetScalingToggle, 0, 0, 0, 2);

    // Create the group of check boxes for the 3D axis labels.
    axisLabelsButtons = new QButtonGroup(0, "axisLabelsButtons");
    connect(axisLabelsButtons, SIGNAL(clicked(int)),
            this, SLOT(axisLabelsChanged(int)));
    QCheckBox *cb = new QCheckBox("X", top, "axisLabelsButtons_X");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox("Y", top, "axisLabelsButtons_Y");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 1, 2);
    cb = new QCheckBox("Z", top, "axisLabelsButtons_Z");
    axisLabelsButtons->insert(cb);
    rLayout->addWidget(cb, 1, 3);
    QLabel *l = new QLabel(axisLabelsButtons, "Show labels",
                   top, "axisLabelsLabel");
    rLayout->addWidget(l, 1, 0);

    // Create the text fields for the 3D label scaling.
    labelScalingLabel = new QLabel("Label scale (x10^?)", top, 
                                   "labelScalingLabel");
    rLayout->addWidget(labelScalingLabel, 2, 0);
    xLabelScalingLineEdit = new QNarrowLineEdit(top,
        "xLabelScalingLineEdit");
    connect(xLabelScalingLineEdit, SIGNAL(returnPressed()),
            this, SLOT(xLabelScalingChanged()));
    rLayout->addWidget(xLabelScalingLineEdit, 2, 1);
    yLabelScalingLineEdit = new QNarrowLineEdit(top,
        "yLabelScalingLineEdit");
    connect(yLabelScalingLineEdit, SIGNAL(returnPressed()),
            this, SLOT(yLabelScalingChanged()));
    rLayout->addWidget(yLabelScalingLineEdit, 2, 2);
    zLabelScalingLineEdit = new QNarrowLineEdit(top,
        "zLabelScalingLineEdit");
    connect(zLabelScalingLineEdit, SIGNAL(returnPressed()),
            this, SLOT(zLabelScalingChanged()));
    rLayout->addWidget(zLabelScalingLineEdit, 2, 3);

    // Create a toggle and line edit for setting the X-axis title.
    xAxisUserTitleToggle = new QCheckBox("Set X-Axis title", top,
        "xAxisUserTitleToggle");
    connect(xAxisUserTitleToggle, SIGNAL(toggled(bool)),
            this, SLOT(xAxisUserTitleChecked(bool)));
    rLayout->addWidget(xAxisUserTitleToggle, 3, 0);
    xAxisUserTitleLineEdit = new QNarrowLineEdit(top, 
        "xAxisUserTitleLineEdit");
    connect(xAxisUserTitleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(xAxisUserTitleLineEditChanged()));
    rLayout->addMultiCellWidget(xAxisUserTitleLineEdit, 3, 3, 1, 3);

    // Create a toggle and line edit for setting the X-axis Units.
    xAxisUserUnitsToggle = new QCheckBox("Set X-Axis units", top,
        "xAxisUserUnitsToggle");
    connect(xAxisUserUnitsToggle, SIGNAL(toggled(bool)),
            this, SLOT(xAxisUserUnitsChecked(bool)));
    rLayout->addWidget(xAxisUserUnitsToggle, 4, 0);
    xAxisUserUnitsLineEdit = new QNarrowLineEdit(top, 
        "xAxisUserUnitsLineEdit");
    connect(xAxisUserUnitsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(xAxisUserUnitsLineEditChanged()));
    rLayout->addMultiCellWidget(xAxisUserUnitsLineEdit, 4, 4, 1, 3);

    // Create a toggle and line edit for setting the Y-axis title.
    yAxisUserTitleToggle = new QCheckBox("Set Y-Axis title", top,
        "yAxisUserTitleToggle");
    connect(yAxisUserTitleToggle, SIGNAL(toggled(bool)),
            this, SLOT(yAxisUserTitleChecked(bool)));
    rLayout->addWidget(yAxisUserTitleToggle, 5, 0);
    yAxisUserTitleLineEdit = new QNarrowLineEdit(top, 
        "yAxisUserTitleLineEdit");
    connect(yAxisUserTitleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(yAxisUserTitleLineEditChanged()));
    rLayout->addMultiCellWidget(yAxisUserTitleLineEdit, 5, 5, 1, 3);

    // Create a toggle and line edit for setting the Y-axis Units.
    yAxisUserUnitsToggle = new QCheckBox("Set Y-Axis units", top,
        "yAxisUserUnitsToggle");
    connect(yAxisUserUnitsToggle, SIGNAL(toggled(bool)),
            this, SLOT(yAxisUserUnitsChecked(bool)));
    rLayout->addWidget(yAxisUserUnitsToggle, 6, 0);
    yAxisUserUnitsLineEdit = new QNarrowLineEdit(top, 
        "yAxisUserUnitsLineEdit");
    connect(yAxisUserUnitsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(yAxisUserUnitsLineEditChanged()));
    rLayout->addMultiCellWidget(yAxisUserUnitsLineEdit, 6, 6, 1, 3);

    // Create a toggle and line edit for setting the Z-axis title.
    zAxisUserTitleToggle = new QCheckBox("Set Z-Axis title", top,
        "zAxisUserTitleToggle");
    connect(zAxisUserTitleToggle, SIGNAL(toggled(bool)),
            this, SLOT(zAxisUserTitleChecked(bool)));
    rLayout->addWidget(zAxisUserTitleToggle, 7, 0);
    zAxisUserTitleLineEdit = new QNarrowLineEdit(top, 
        "zAxisUserTitleLineEdit");
    connect(zAxisUserTitleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(zAxisUserTitleLineEditChanged()));
    rLayout->addMultiCellWidget(zAxisUserTitleLineEdit, 7, 7, 1, 3);

    // Create a toggle and line edit for setting the Z-axis Units.
    zAxisUserUnitsToggle = new QCheckBox("Set Z-Axis units", top,
        "zAxisUserUnitsToggle");
    connect(zAxisUserUnitsToggle, SIGNAL(toggled(bool)),
            this, SLOT(zAxisUserUnitsChecked(bool)));
    rLayout->addWidget(zAxisUserUnitsToggle, 8, 0);
    zAxisUserUnitsLineEdit = new QNarrowLineEdit(top, 
        "zAxisUserUnitsLineEdit");
    connect(zAxisUserUnitsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(zAxisUserUnitsLineEditChanged()));
    rLayout->addMultiCellWidget(zAxisUserUnitsLineEdit, 8, 8, 1, 3);

    return top0;
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateColorTab
//
// Purpose: 
//   Creates the color tab.
//
// Note:       This code used to be in CreateWindowContents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 16:33:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::CreateColorTab()
{
    //
    // Create the group of color-related widgets.
    //
    pageColor = new QGroupBox(central, "pageColor");
    pageColor->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageColor, "Colors");

    QVBoxLayout *vcLayout = new QVBoxLayout(pageColor);
    vcLayout->setMargin(10);
    QGridLayout *cLayout = new QGridLayout(vcLayout, 3, 2);
    cLayout->setSpacing(10);

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
    vcLayout->addStretch(50);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateObjectsTab
//
// Purpose: 
//   Creates the objects tab.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 17:00:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::CreateObjectsTab()
{
    //
    // Create the group of color-related widgets.
    //
    pageObjects = new QGroupBox(central, "pageObjects");
    pageObjects->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageObjects, "Objects");

    QVBoxLayout *objTopLayout = new QVBoxLayout(pageObjects);
    objTopLayout->setMargin(10);
    objTopLayout->setSpacing(5);

    QHBoxLayout *hLayout = new QHBoxLayout(objTopLayout);

    //
    // Create the buttons that let us create new annotation objects.
    //
    QGroupBox *newObjectGroup = new QGroupBox(pageObjects, "newObjectGroup");
    newObjectGroup->setTitle("Create new");
    hLayout->addWidget(newObjectGroup);

    QVBoxLayout *objButtonLayout = new QVBoxLayout(newObjectGroup);
    objButtonLayout->setMargin(10);
    objButtonLayout->addSpacing(15);
    objButtonLayout->setSpacing(5);
    objButtonGroup = new QButtonGroup(0, "objButtonGroup");
    connect(objButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(addNewAnnotationObject(int)));

    //
    // Create the annotation object list and controls to do things to them.
    //
    QGridLayout *annotListLayout = new QGridLayout(hLayout, 3, 2);
    annotListLayout->addMultiCellWidget(new QLabel("Annotation objects",
        pageObjects), 0, 0, 0, 1);
    annotationListBox = new QListBox(pageObjects, "annotationListBox");
    annotationListBox->setMinimumHeight(100);
    annotationListBox->setSelectionMode(QListBox::Extended);
    connect(annotationListBox, SIGNAL(selectionChanged()),
            this, SLOT(setActiveAnnotations()));
    annotListLayout->addMultiCellWidget(annotationListBox, 1, 1, 0, 1);
    annotListLayout->setRowStretch(1, 50);

    hideShowAnnotationButton = new QPushButton("Hide/Show", pageObjects);
    connect(hideShowAnnotationButton, SIGNAL(clicked()),
            this, SLOT(hideActiveAnnotations()));
    annotListLayout->addWidget(hideShowAnnotationButton, 2, 0);

    deleteAnnotationButton = new QPushButton("Delete", pageObjects);
    connect(deleteAnnotationButton, SIGNAL(clicked()),
            this, SLOT(deleteActiveAnnotations()));
    annotListLayout->addWidget(deleteAnnotationButton, 2, 1);

    //
    // Create all of the annotation interface widgets.
    //
    QvisAnnotationObjectInterfaceFactory factory;
    if(factory.GetMaxInterfaces() > 0)
    {
        nObjectInterfaces = factory.GetMaxInterfaces();
        objectInterfaces = new QvisAnnotationObjectInterface *[nObjectInterfaces];

        bool notFirstInterface = false;
        for(int i = 0; i < factory.GetMaxInterfaces(); ++i)
        {
            objectInterfaces[i] = factory.CreateInterface(i, pageObjects);

            if(objectInterfaces[i])
            {
                // Connect a signal from the annotation interface that allows
                // it to tell this window to apply the changes.
                connect(objectInterfaces[i], SIGNAL(applyChanges()),
                        this, SLOT(applyObjectListChanges()));
                // Connect a signal from the annotation interface that allows
                // it to tell this window to ignore updates.
                connect(objectInterfaces[i], SIGNAL(setUpdateForWindow(bool)),
                        this, SLOT(setUpdateForWindow(bool)));

                // Add a new button to create the annotation.
                QPushButton *btn = new QPushButton(objectInterfaces[i]->GetName(),
                    newObjectGroup);
                objButtonGroup->insert(btn, i);
                objButtonLayout->addWidget(btn);
            
                // Add the annotation interface to the top object layout.
                objTopLayout->addWidget(objectInterfaces[i]);

                // Hide all but the first interface.
                if(notFirstInterface)
                    objectInterfaces[i]->hide();
                notFirstInterface = true;
            }
            else
            {
                debug1 << "QvisAnnotationObjectInterfaceFactory can't create "
                          "an interface for index=" << i << " yet." << endl;
            }
        }
    }

    objButtonLayout->addStretch(10);
    
}

// ****************************************************************************
// Method: QvisAnnotationWindow::Update
//
// Purpose: 
//   This method is called when a Subject that this window observes it updated.
//
// Arguments:
//   TheChangedSubject : The subject that is causing the update.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 18:02:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::UpdateWindow(bool doAll)
{
    if(annotationAtts == 0 || annotationObjectList == 0)
        return;

    if(SelectedSubject() == annotationAtts || doAll)
        UpdateAnnotationControls(doAll);
    if(SelectedSubject() == annotationObjectList || doAll)
        UpdateAnnotationObjectControls(doAll);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::UpdateAnnotationControls
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
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added the ability to control the 2d & 3d label scaling exponents.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Brad Whitlock, Wed Jul 27 15:58:54 PST 2005
//   Added support for setting user-specified titles and labels.
//
// ****************************************************************************

void
QvisAnnotationWindow::UpdateAnnotationControls(bool doAll)
{
    QColor  c;
    QString temp;
    bool isGradient, axesAutoSetTicks, labelAutoSetScaling;
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
        case 2: // labelAutoSetScaling2D
            labelAutoSetScalingToggle2D->blockSignals(true);
            labelAutoSetScalingToggle2D->setChecked(annotationAtts->GetLabelAutoSetScaling2D());
            labelAutoSetScalingToggle2D->blockSignals(false);

            labelAutoSetScaling = (annotationAtts->GetLabelAutoSetScaling2D() == 1);
            labelScalingLabel2D->setEnabled(!labelAutoSetScaling);
            xLabelScalingLineEdit2D->setEnabled(!labelAutoSetScaling);
            yLabelScalingLineEdit2D->setEnabled(!labelAutoSetScaling);
            break;
        case 3: // xAxisLabels2D
        case 4: // yAxisLabels2D
            setAxisLabels2D = true;
            break;
        case 5: // xAxisTitle2D
        case 6: // yAxisTitle2D
            setAxisTitles2D = true;
            break;
        case 7: // xGridLines2D
        case 8: // yGridLines2D
            setGridLines2D = true;
            break;
        case 9: // xMajorTickMinimum2D
            temp.setNum(annotationAtts->GetXMajorTickMinimum2D());
            xMajorTickMinimumLineEdit2D->setText(temp);
            break;
        case 10: // yMajorTickMinimum2D
            temp.setNum(annotationAtts->GetYMajorTickMinimum2D());
            yMajorTickMinimumLineEdit2D->setText(temp);
            break;
        case 11: // xMajorTickMaximum2D
            temp.setNum(annotationAtts->GetXMajorTickMaximum2D());
            xMajorTickMaximumLineEdit2D->setText(temp);
            break;
        case 12: // yMajorTickMaximum2D
            temp.setNum(annotationAtts->GetYMajorTickMaximum2D());
            yMajorTickMaximumLineEdit2D->setText(temp);
            break;
        case 13: // xMajorTickSpacing2D
            temp.setNum(annotationAtts->GetXMajorTickSpacing2D());
            xMajorTickSpacingLineEdit2D->setText(temp);
            break;
        case 14: // yMajorTickSpacing2D
            temp.setNum(annotationAtts->GetYMajorTickSpacing2D());
            yMajorTickSpacingLineEdit2D->setText(temp);
            break;
        case 15: // xMinorTickSpacing2D
            temp.setNum(annotationAtts->GetXMinorTickSpacing2D());
            xMinorTickSpacingLineEdit2D->setText(temp);
            break;
        case 16: // yMinorTickSpacing2D
            temp.setNum(annotationAtts->GetYMinorTickSpacing2D());
            yMinorTickSpacingLineEdit2D->setText(temp);
            break;
        case 17: // xLabelFontHeight2D
            temp.setNum(annotationAtts->GetXLabelFontHeight2D());
            xLabelFontHeightLineEdit2D->setText(temp);
            break;
        case 18: // yLabelFontHeight2D
            temp.setNum(annotationAtts->GetYLabelFontHeight2D());
            yLabelFontHeightLineEdit2D->setText(temp);
            break;
        case 19: // xTitleFontHeight2D
            temp.setNum(annotationAtts->GetXTitleFontHeight2D());
            xTitleFontHeightLineEdit2D->setText(temp);
            break;
        case 20: // yTitleFontHeight2D
            temp.setNum(annotationAtts->GetYTitleFontHeight2D());
            yTitleFontHeightLineEdit2D->setText(temp);
            break;
        case 21: // xLabelScaling2D
            temp.sprintf("%d", annotationAtts->GetXLabelScaling2D());
            xLabelScalingLineEdit2D->setText(temp);
            break;
        case 22: // yLabelScaling2D
            temp.sprintf("%d", annotationAtts->GetYLabelScaling2D());
            yLabelScalingLineEdit2D->setText(temp);
            break;
        case 23: // axesLineWidth2D
            axesLineWidth2D->blockSignals(true);
            axesLineWidth2D->SetLineWidth(annotationAtts->GetAxesLineWidth2D());
            axesLineWidth2D->blockSignals(false);
            break;
        case 24: // axesTickLocation2D
            axesTickLocationComboBox2D->blockSignals(true);
            axesTickLocationComboBox2D->setCurrentItem(annotationAtts->GetAxesTickLocation2D());
            axesTickLocationComboBox2D->blockSignals(false);
            break;
        case 25: // axesTicks2D
            axesTicksComboBox2D->blockSignals(true);
            axesTicksComboBox2D->setCurrentItem(annotationAtts->GetAxesTicks2D());
            axesTicksComboBox2D->blockSignals(false);
            break;
        case 26: // xAxisUserTitle2D
            xAxisUserTitleLineEdit2D->setText(annotationAtts->GetXAxisUserTitle2D().c_str());
            break;
        case 27: // yAxisUserTitle2D
            yAxisUserTitleLineEdit2D->setText(annotationAtts->GetYAxisUserTitle2D().c_str());
            break;
        case 28: // xAxisUserTitleFlag2D
            xAxisUserTitleToggle2D->blockSignals(true);
            xAxisUserTitleToggle2D->setChecked(annotationAtts->GetXAxisUserTitleFlag2D());
            xAxisUserTitleToggle2D->blockSignals(false);

            xAxisUserTitleLineEdit2D->setEnabled(annotationAtts->GetXAxisUserTitleFlag2D());
            break;
        case 29: // yAxisUserTitleFlag2D
            yAxisUserTitleToggle2D->blockSignals(true);
            yAxisUserTitleToggle2D->setChecked(annotationAtts->GetYAxisUserTitleFlag2D());
            yAxisUserTitleToggle2D->blockSignals(false);

            yAxisUserTitleLineEdit2D->setEnabled(annotationAtts->GetYAxisUserTitleFlag2D());
            break;
        case 30: // xAxisUserUnits2D
            xAxisUserUnitsLineEdit2D->setText(annotationAtts->GetXAxisUserUnits2D().c_str());
            break;
        case 31: // yAxisUserUnits2D
            yAxisUserUnitsLineEdit2D->setText(annotationAtts->GetYAxisUserUnits2D().c_str());
            break;
        case 32: // xAxisUserUnitsFlag2D
            xAxisUserUnitsToggle2D->blockSignals(true);
            xAxisUserUnitsToggle2D->setChecked(annotationAtts->GetXAxisUserUnitsFlag2D());
            xAxisUserUnitsToggle2D->blockSignals(false);

            xAxisUserUnitsLineEdit2D->setEnabled(annotationAtts->GetXAxisUserUnitsFlag2D());
            break;
        case 33: // yAxisUserUnitsFlag2D
            yAxisUserUnitsToggle2D->blockSignals(true);
            yAxisUserUnitsToggle2D->setChecked(annotationAtts->GetYAxisUserUnitsFlag2D());
            yAxisUserUnitsToggle2D->blockSignals(false);

            yAxisUserUnitsLineEdit2D->setEnabled(annotationAtts->GetYAxisUserUnitsFlag2D());
            break;
        case 34: // axesFlag
            axes3DFlagToggle->blockSignals(true);
            axes3DFlagToggle->setChecked(annotationAtts->GetAxesFlag());
            axes3DFlagToggle->blockSignals(false);
            axes3DGroup->setEnabled(annotationAtts->GetAxesFlag());
            break;
        case 35: // axesAutoSetTicks
#if 0
            axesAutoSetTicksToggle->blockSignals(true);
            axesAutoSetTicksToggle->setChecked(annotationAtts->GetAutoSetTicks());
            axesAutoSetTicksToggle->blockSignals(false);
            // Make the tick locations text fields not enabled.
#endif
            break;
        case 36: // labelAutoSetScaling
            labelAutoSetScalingToggle->blockSignals(true);
            labelAutoSetScalingToggle->setChecked(
                annotationAtts->GetLabelAutoSetScaling());
            labelAutoSetScalingToggle->blockSignals(false);

            labelAutoSetScaling = (annotationAtts->GetLabelAutoSetScaling() == 1);
            labelScalingLabel->setEnabled(!labelAutoSetScaling);
            xLabelScalingLineEdit->setEnabled(!labelAutoSetScaling);
            yLabelScalingLineEdit->setEnabled(!labelAutoSetScaling);
            zLabelScalingLineEdit->setEnabled(!labelAutoSetScaling);
            break;
        case 37: // xAxisLabels
        case 38: // yAxisLabels
        case 39: // zAxisLabels
            setAxisLabels = true;
            break;
        case 40: // xAxisTitle
        case 41: // yAxisTitle
        case 42: // zAxisTitle
            setAxisTitles = true;
            break;
        case 43: // xGridLines
        case 44: // yGridLines
        case 45: // zGridLines
            setGridLines = true;
            break;
        case 46: // xAxisTicks
        case 47: // yAxisTicks
        case 48: // zAxisTicks
            setAxisTicks = true;
            break;
        case 49: // xMajorTickMinimum
        case 50: // yMajorTickMinimum
        case 51: // zMajorTickMinimum
        case 52: // xMajorTickMaximum
        case 53: // yMajorTickMaximum
        case 54: // zMajorTickMaximum
        case 55: // xMajorTickSpacing
        case 56: // yMajorTickSpacing
        case 57: // zMajorTickSpacing
        case 58: // xMinorTickSpacing
        case 59: // yMinorTickSpacing
        case 60: // zMinorTickSpacing
        case 61: // xLabelFontHeight
        case 62: // yLabelFontHeight
        case 63: // zLabelFontHeight
        case 64: // xTitleFontHeight
        case 65: // yTitleFontHeight
        case 66: // zTitleFontHeight
            // IMPLEMENT
            break;
        case 67: // xLabelScaling
            temp.sprintf("%d", annotationAtts->GetXLabelScaling());
            xLabelScalingLineEdit->setText(temp);
            break;
        case 68: // yLabelScaling
            temp.sprintf("%d", annotationAtts->GetYLabelScaling());
            yLabelScalingLineEdit->setText(temp);
            break;
        case 69: // zLabelScaling
            temp.sprintf("%d", annotationAtts->GetZLabelScaling());
            zLabelScalingLineEdit->setText(temp);
            break;
        case 70: // xAxisUserTitle
            xAxisUserTitleLineEdit->setText(annotationAtts->GetXAxisUserTitle().c_str());
            break;
        case 71: // yAxisUserTitle
            yAxisUserTitleLineEdit->setText(annotationAtts->GetYAxisUserTitle().c_str());
            break;
        case 72: // yAxisUserTitle
            zAxisUserTitleLineEdit->setText(annotationAtts->GetZAxisUserTitle().c_str());
            break;
        case 73: // xAxisUserTitleFlag
            xAxisUserTitleToggle->blockSignals(true);
            xAxisUserTitleToggle->setChecked(annotationAtts->GetXAxisUserTitleFlag());
            xAxisUserTitleToggle->blockSignals(false);

            xAxisUserTitleLineEdit->setEnabled(annotationAtts->GetXAxisUserTitleFlag());
            break;
        case 74: // yAxisUserTitleFlag
            yAxisUserTitleToggle->blockSignals(true);
            yAxisUserTitleToggle->setChecked(annotationAtts->GetYAxisUserTitleFlag());
            yAxisUserTitleToggle->blockSignals(false);

            yAxisUserTitleLineEdit->setEnabled(annotationAtts->GetYAxisUserTitleFlag());
            break;
        case 75: // zAxisUserTitleFlag
            zAxisUserTitleToggle->blockSignals(true);
            zAxisUserTitleToggle->setChecked(annotationAtts->GetZAxisUserTitleFlag());
            zAxisUserTitleToggle->blockSignals(false);

            zAxisUserTitleLineEdit->setEnabled(annotationAtts->GetZAxisUserTitleFlag());
            break;
        case 76: // xAxisUserUnits
            xAxisUserUnitsLineEdit->setText(annotationAtts->GetXAxisUserUnits().c_str());
            break;
        case 77: // yAxisUserUnits
            yAxisUserUnitsLineEdit->setText(annotationAtts->GetYAxisUserUnits().c_str());
            break;
        case 78: // zAxisUserUnits
            zAxisUserUnitsLineEdit->setText(annotationAtts->GetZAxisUserUnits().c_str());
            break;
        case 79: // xAxisUserUnitsFlag
            xAxisUserUnitsToggle->blockSignals(true);
            xAxisUserUnitsToggle->setChecked(annotationAtts->GetXAxisUserUnitsFlag());
            xAxisUserUnitsToggle->blockSignals(false);

            xAxisUserUnitsLineEdit->setEnabled(annotationAtts->GetXAxisUserUnitsFlag());
            break;
        case 80: // yAxisUserUnitsFlag
            yAxisUserUnitsToggle->blockSignals(true);
            yAxisUserUnitsToggle->setChecked(annotationAtts->GetYAxisUserUnitsFlag());
            yAxisUserUnitsToggle->blockSignals(false);

            yAxisUserUnitsLineEdit->setEnabled(annotationAtts->GetYAxisUserUnitsFlag());
            break;
        case 81: // zAxisUserUnitsFlag
            zAxisUserUnitsToggle->blockSignals(true);
            zAxisUserUnitsToggle->setChecked(annotationAtts->GetZAxisUserUnitsFlag());
            zAxisUserUnitsToggle->blockSignals(false);

            zAxisUserUnitsLineEdit->setEnabled(annotationAtts->GetZAxisUserUnitsFlag());
            break;
        case 82: // axesTickLocation
            axes3DTickLocationComboBox->blockSignals(true);
            axes3DTickLocationComboBox->setCurrentItem(annotationAtts->GetAxesTickLocation());
            axes3DTickLocationComboBox->blockSignals(false);
            break;
        case 83: // axesType
            axes3DTypeComboBox->blockSignals(true);
            axes3DTypeComboBox->setCurrentItem(annotationAtts->GetAxesType());
            axes3DTypeComboBox->blockSignals(false);
            break;
        case 84: // triadFlag
            triadFlagToggle->blockSignals(true);
            triadFlagToggle->setChecked(annotationAtts->GetTriadFlag());
            triadFlagToggle->blockSignals(false);
            break;
        case 85: // bboxFlag
            bboxFlagToggle->blockSignals(true);
            bboxFlagToggle->setChecked(annotationAtts->GetBboxFlag());
            bboxFlagToggle->blockSignals(false);
            break;
        case 86: // backgroundColor
            cptr = annotationAtts->GetBackgroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            backgroundColorButton->blockSignals(true);
            backgroundColorButton->setButtonColor(c);
            backgroundColorButton->blockSignals(false);
            break;
        case 87: // foregroundColor
            cptr = annotationAtts->GetForegroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            foregroundColorButton->blockSignals(true);
            foregroundColorButton->setButtonColor(c);
            foregroundColorButton->blockSignals(false);
            break;
        case 88: // gradientBackgroundStyle
            gradientStyleComboBox->blockSignals(true);
            gradientStyleComboBox->setCurrentItem(annotationAtts->GetGradientBackgroundStyle());
            gradientStyleComboBox->blockSignals(false);
            break;
        case 89: // gradientColor1
            cptr = annotationAtts->GetGradientColor1().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor1Button->blockSignals(true);
            gradientColor1Button->setButtonColor(c);
            gradientColor1Button->blockSignals(false);
            break;
        case 90: // gradientColor2
            cptr = annotationAtts->GetGradientColor2().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor2Button->blockSignals(true);
            gradientColor2Button->setButtonColor(c);
            gradientColor2Button->blockSignals(false);
            break;
        case 91: // backgroundMode
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
        case 92: // userInfo
            userInfo->blockSignals(true);
            userInfo->setChecked(annotationAtts->GetUserInfoFlag());
            userInfo->blockSignals(false);
            break;
        case 93: // databaseInfo
            databaseInfo->blockSignals(true);
            databaseInfo->setChecked(annotationAtts->GetDatabaseInfoFlag());
            databaseInfo->blockSignals(false);
            break;
        case 94: // legendInfo
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
// Method: QvisAnnotationWindow::UpdateAnnotationObjectControls
//
// Purpose: 
//   This method is called when the annotation object list from the viewer is
//   modified.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 30 18:01:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::UpdateAnnotationObjectControls(bool doAll)
{
    //
    // Add all of the annotation objects to the list box.
    //
    QvisAnnotationObjectInterface *firstInterface = 0;
    int i, firstInterfaceIndex = -1;
    annotationListBox->blockSignals(true);
    annotationListBox->clear();
    for(i = 0; i < annotationObjectList->GetNumAnnotationObjects(); ++i)
    {
        const AnnotationObject &annot = annotationObjectList->operator[](i);
        int annotType = int(annot.GetObjectType());

        if(annotType >= 0 && annotType < nObjectInterfaces)
        {
            if(objectInterfaces[annotType] != 0)
            {
                if(firstInterface == 0 && annot.GetActive())
                {
                    firstInterface = objectInterfaces[annotType];
                    firstInterfaceIndex = i;
                }

                // Let the interface determine the text it should display
                // in the menu.
                QString mText(objectInterfaces[annotType]->GetMenuText(annot));
                annotationListBox->insertItem(mText);
                annotationListBox->setSelected(i, annot.GetActive());
            }
        }
    }
    annotationListBox->blockSignals(false);

    //
    // If no objects were selected, then make sure that we show the first
    // interface.
    //
    if(!firstInterface)
    {
        // Look for the first non NULL interface.
        for(i = 0; i < nObjectInterfaces; ++i)
        {
            if(objectInterfaces[i])
            {
                firstInterface = objectInterfaces[i];
                break;
            }
        }
    }

    //
    // Make sure that the appropriate object interface is showing and that
    // it is updated.
    //
    displayInterface = 0;
    for(i = 0; i < nObjectInterfaces; ++i)
    {
        if(objectInterfaces[i])
        {
            if(objectInterfaces[i] == firstInterface)
            {
                if(firstInterfaceIndex != -1)
                {
                    // We have an active annotation interface that we're
                    // making sure gets shown. Make sure that it has the
                    // right values in the widgets.
                    AnnotationObject &annot =
                        annotationObjectList->operator[](firstInterfaceIndex);
                    objectInterfaces[i]->Update(&annot);
                    objectInterfaces[i]->setEnabled(true);
                    displayInterface = objectInterfaces[i];

                    // Set the current item in the annotation list box.
                    annotationListBox->blockSignals(true);
                    annotationListBox->setCurrentItem(firstInterfaceIndex);
                    annotationListBox->blockSignals(false);
                }
                else
                    objectInterfaces[i]->setEnabled(false);
                objectInterfaces[i]->show();
            }
            else
                objectInterfaces[i]->hide();
        }
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
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added the ability to control the 2d & 3d label scaling exponents.
//
//   Brad Whitlock, Wed Jul 27 17:35:59 PST 2005
//   Added code to get axis labels and units.
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

    // Do the 2d x label scaling value
    if (which_widget == 12 || doAll)
    {
        temp = xLabelScalingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            int v;
            if (sscanf(temp.latin1(), "%d", &v) == 1)
            {
                annotationAtts->SetXLabelScaling2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D X Label scaling value was invalid. "
                "Resetting to the last good value of %d.",
                annotationAtts->GetXLabelScaling2D());
            Error(msg);
            annotationAtts->SetXLabelScaling2D(
                annotationAtts->GetXLabelScaling2D());
        }
    }

    // Do the 2d y label scaling value
    if (which_widget == 13 || doAll)
    {
        temp = yLabelScalingLineEdit2D->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            int v;
            if (sscanf(temp.latin1(), "%d", &v) == 1)
            {
                annotationAtts->SetYLabelScaling2D(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 2D Y Label scaling value was invalid. "
                "Resetting to the last good value of %d.",
                annotationAtts->GetYLabelScaling2D());
            Error(msg);
            annotationAtts->SetYLabelScaling2D(
                annotationAtts->GetYLabelScaling2D());
        }
    }

    // Do the 3d x label scaling value
    if (which_widget == 14 || doAll)
    {
        temp = xLabelScalingLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            int v;
            if (sscanf(temp.latin1(), "%d", &v) == 1)
            {
                annotationAtts->SetXLabelScaling(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 3D X Label scaling value was invalid. "
                "Resetting to the last good value of %d.",
                annotationAtts->GetXLabelScaling());
            Error(msg);
            annotationAtts->SetXLabelScaling(
                annotationAtts->GetXLabelScaling());
        }
    }

    // Do the 3d y label scaling value
    if (which_widget == 15 || doAll)
    {
        temp = yLabelScalingLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            int v;
            if (sscanf(temp.latin1(), "%d", &v) == 1)
            {
                annotationAtts->SetYLabelScaling(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 3D Y Label scaling value was invalid. "
                "Resetting to the last good value of %d.",
                annotationAtts->GetYLabelScaling());
            Error(msg);
            annotationAtts->SetYLabelScaling(
                annotationAtts->GetYLabelScaling());
        }
    }

    // Do the 3d z label scaling value
    if (which_widget == 16 || doAll)
    {
        temp = zLabelScalingLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if (okay)
        {
            int v;
            if (sscanf(temp.latin1(), "%d", &v) == 1)
            {
                annotationAtts->SetZLabelScaling(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            msg.sprintf("The 3D Z Label scaling value was invalid. "
                "Resetting to the last good value of %d.",
                annotationAtts->GetZLabelScaling());
            Error(msg);
            annotationAtts->SetZLabelScaling(
                annotationAtts->GetZLabelScaling());
        }
    }

    // Do xAxisUserTitle2D
    if (which_widget == 17 || doAll)
    {
        temp = xAxisUserTitleLineEdit2D->displayText().stripWhiteSpace();
        annotationAtts->SetXAxisUserTitle2D(temp.latin1());
    }

    // Do yAxisUserTitle2D
    if (which_widget == 18 || doAll)
    {
        temp = yAxisUserTitleLineEdit2D->displayText().stripWhiteSpace();
        annotationAtts->SetYAxisUserTitle2D(temp.latin1());
    }

    // Do xAxisUserUnits2D
    if (which_widget == 19 || doAll)
    {
        temp = xAxisUserUnitsLineEdit2D->displayText().stripWhiteSpace();
        annotationAtts->SetXAxisUserUnits2D(temp.latin1());
    }

    // Do yAxisUserUnits2D
    if (which_widget == 20 || doAll)
    {
        temp = yAxisUserUnitsLineEdit2D->displayText().stripWhiteSpace();
        annotationAtts->SetYAxisUserUnits2D(temp.latin1());
    }

    // Do xAxisUserTitle
    if (which_widget == 21 || doAll)
    {
        temp = xAxisUserTitleLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetXAxisUserTitle(temp.latin1());
    }

    // Do yAxisUserTitle
    if (which_widget == 22 || doAll)
    {
        temp = yAxisUserTitleLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetYAxisUserTitle(temp.latin1());
    }

    // Do zAxisUserTitle
    if (which_widget == 23 || doAll)
    {
        temp = zAxisUserTitleLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetZAxisUserTitle(temp.latin1());
    }

    // Do xAxisUserUnits
    if (which_widget == 24 || doAll)
    {
        temp = xAxisUserUnitsLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetXAxisUserUnits(temp.latin1());
    }

    // Do yAxisUserUnits
    if (which_widget == 25 || doAll)
    {
        temp = yAxisUserUnitsLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetYAxisUserUnits(temp.latin1());
    }

    // Do zAxisUserUnits
    if (which_widget == 26 || doAll)
    {
        temp = zAxisUserUnitsLineEdit->displayText().stripWhiteSpace();
        annotationAtts->SetZAxisUserUnits(temp.latin1());
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
// Method: QvisAnnotationWindow::ApplyObjectList
//
// Purpose: 
//   This method is called when an annotation interface wants to tell the
//   viewer about a change to the annotation attributes.
//
// Arguments:
//   dontIgnore : If this is true, the new state takes effect immediately.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 4 12:27:38 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::ApplyObjectList(bool dontIgnore)
{
    // If we're showing an interface, make it get its current values.
    if(displayInterface)
        displayInterface->GetCurrentValues(-1);

    annotationObjectList->Notify();

    if(AutoUpdate() || dontIgnore)
    {
        viewer->SetAnnotationObjectOptions();
    }
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
//   Brad Whitlock, Fri Oct 31 13:43:39 PST 2003
//   I made it be a QvisPostableWindowSimpleObserver.
//
// ****************************************************************************

void
QvisAnnotationWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

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
//   Brad Whitlock, Thu Oct 30 16:41:49 PST 2003
//   I allowed activeTab 3 and made it use QvisPostableWindowSimpleObserver's
//   SetFromNode method.
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
        if(activeTab != 0 && activeTab != 1 && activeTab != 2 && activeTab != 3)
            activeTab = 0;
    }

    // Call the base class's function.
    QvisPostableWindowSimpleObserver::SetFromNode(parentNode, borders);
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
//   Brad Whitlock, Fri Nov 7 17:46:00 PST 2003
//   I made it apply the object list too.
//
// ****************************************************************************

void
QvisAnnotationWindow::apply()
{
    ApplyObjectList(true);
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
//   Brad Whitlock, Fri Nov 7 17:46:37 PST 2003
//   I added code to tell the viewer to also set the default annotation
//   object list.
//
// ****************************************************************************

void
QvisAnnotationWindow::makeDefault()
{
    // Tell the viewer to set the default annotation attributes.
    annotationAtts->Notify();
    viewer->SetDefaultAnnotationAttributes();

    annotationObjectList->Notify();
    viewer->SetDefaultAnnotationObjectList();
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
//   Brad Whitlock, Fri Nov 7 17:47:29 PST 2003
//   I added code to tell the viewer to reset the annotation object list.
//
// ****************************************************************************

void
QvisAnnotationWindow::reset()
{
    // Tell the viewer to reset the annotation attributes to the last applied
    // values.
    viewer->ResetAnnotationAttributes();

    // Tell the viewer to reset the annotation object list to the last applied
    // values.
    viewer->ResetAnnotationObjectList();
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
// Method: QvisAnnotationWindow::labelAutoSetScaling2D
//
// Purpose: 
//   This is a Qt slot function that sets the 2D auto set label scaling flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::labelAutoSetScalingChecked2D(bool val)
{
    annotationAtts->SetLabelAutoSetScaling2D(val);
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
// Method: QvisAnnotationWindow::xLabelScalingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d x label scaling is
//   changed.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xLabelScalingChanged2D()
{
    GetCurrentValues(12);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yLabelScalingChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when the 2d y label scaling is
//   changed.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yLabelScalingChanged2D()
{
    GetCurrentValues(13);
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


void
QvisAnnotationWindow::xAxisUserTitleChecked2D(bool val)
{
    annotationAtts->SetXAxisUserTitleFlag2D(val);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserTitleLineEditChanged2D()
{
    GetCurrentValues(17);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserTitleChecked2D(bool val)
{
    annotationAtts->SetYAxisUserTitleFlag2D(val);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserTitleLineEditChanged2D()
{
    GetCurrentValues(18);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserUnitsChecked2D(bool val)
{
    annotationAtts->SetXAxisUserUnitsFlag2D(val);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserUnitsLineEditChanged2D()
{
    GetCurrentValues(19);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserUnitsChecked2D(bool val)
{
    annotationAtts->SetYAxisUserUnitsFlag2D(val);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserUnitsLineEditChanged2D()
{
    GetCurrentValues(20);
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
// Method: QvisAnnotationWindow::labelAutoSetScaling
//
// Purpose: 
//   This is a Qt slot function that sets the 3D auto set label scaling flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::labelAutoSetScalingChecked(bool val)
{
    annotationAtts->SetLabelAutoSetScaling(val);
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
// Method: QvisAnnotationWindow::xLabelScalingChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the 3d x label scaling is
//   changed.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xLabelScalingChanged()
{
    GetCurrentValues(14);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yLabelScalingChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the 3d y label scaling is
//   changed.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yLabelScalingChanged()
{
    GetCurrentValues(15);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::zLabelScalingChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the 3d z label scaling is
//   changed.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::zLabelScalingChanged()
{
    GetCurrentValues(16);
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

void
QvisAnnotationWindow::xAxisUserTitleChecked(bool val)
{
    annotationAtts->SetXAxisUserTitleFlag(val);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserTitleLineEditChanged()
{
    GetCurrentValues(21);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserTitleChecked(bool val)
{
    annotationAtts->SetYAxisUserTitleFlag(val);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserTitleLineEditChanged()
{
    GetCurrentValues(22);
    Apply();
}

void
QvisAnnotationWindow::zAxisUserTitleChecked(bool val)
{
    annotationAtts->SetZAxisUserTitleFlag(val);
    Apply();
}

void
QvisAnnotationWindow::zAxisUserTitleLineEditChanged()
{
    GetCurrentValues(23);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserUnitsChecked(bool val)
{
    annotationAtts->SetXAxisUserUnitsFlag(val);
    Apply();
}

void
QvisAnnotationWindow::xAxisUserUnitsLineEditChanged()
{
    GetCurrentValues(24);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserUnitsChecked(bool val)
{
    annotationAtts->SetYAxisUserUnitsFlag(val);
    Apply();
}

void
QvisAnnotationWindow::yAxisUserUnitsLineEditChanged()
{
    GetCurrentValues(25);
    Apply();
}

void
QvisAnnotationWindow::zAxisUserUnitsChecked(bool val)
{
    annotationAtts->SetZAxisUserUnitsFlag(val);
    Apply();
}

void
QvisAnnotationWindow::zAxisUserUnitsLineEditChanged()
{
    GetCurrentValues(26);
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

// ****************************************************************************
// Method: QvisAnnotationWindow::turnOffAllAnnotations
//
// Purpose: 
//   Turns off all annotations.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 14:40:08 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Nov 26 14:48:03 PST 2003
//   Added code to hide all of the annotation objects.
//
// ****************************************************************************

void
QvisAnnotationWindow::turnOffAllAnnotations()
{
    //
    // Hide all of the annotations that are not annotation objects.
    //
    annotationAtts->SetAxesFlag2D(false);
    annotationAtts->SetAxesFlag(false);
    annotationAtts->SetTriadFlag(false);
    annotationAtts->SetBboxFlag(false);
    annotationAtts->SetUserInfoFlag(false);
    annotationAtts->SetDatabaseInfoFlag(false);
    annotationAtts->SetLegendInfoFlag(false);
    Apply();

    //
    // Hide all of the annotation objects.
    //
    if(annotationObjectList->GetNumAnnotationObjects() > 0)
    {
        for(int i = 0; i < annotationObjectList->GetNumAnnotationObjects(); ++i)
        {           
            AnnotationObject &annot = annotationObjectList->operator[](i);
            annot.SetVisible(false);
        }

        ApplyObjectList();
    }
}

// ****************************************************************************
// Method: QvisAnnotationWindow::setUpdateForWindow
//
// Purpose: 
//   This is a Qt slot function that sets the window's update value, which
//   determines whether or not the window updates when it gets a Notify.
//
// Arguments:
//   val : The new update value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:11:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::setUpdateForWindow(bool val)
{
    SetUpdate(val);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::applyObjectListChanges
//
// Purpose: 
//   This is a Qt slot function that allows the annotation interfaces to
//   force changes to the annotation object list to be sent to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:12:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::applyObjectListChanges()
{
    ApplyObjectList();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::addNewAnnotationObject
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to create an object of
//   the specified type.
//
// Arguments:
//   annotType : The type of annotation object to create.
//
// Note:       The annotation object list is applied first so we don't lose
//             changes to objects when we create a new one.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:12:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::addNewAnnotationObject(int annotType)
{
    //
    // Make the viewer apply the changes or they will be lost when it creates
    // the new annotation object. This isn't so bad.
    //
    ApplyObjectList(true);

    //
    // Tell the viewer to create a new annotation object.
    //
    viewer->AddAnnotationObject(annotType);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::setActiveAnnotations
//
// Purpose: 
//   This is a Qt slot function that sets the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:14:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::setActiveAnnotations()
{
    // Set the active flag on the annotation objects in the annotation
    // object list.
    for(int i = 0; i < annotationObjectList->GetNumAnnotationObjects(); ++i)
    {
        bool isSelected = (i < annotationListBox->count()) ?
                          annotationListBox->isSelected(i) : false;
        AnnotationObject &annot = annotationObjectList->operator[](i);
        annot.SetActive(isSelected);
    }

    // Apply the changes but make sure that we do it once we're back in
    // the main event loop because updating the annotation object list
    // can clear out and repopulate the annotation list box.
    QTimer::singleShot(100, this, SLOT(applyObjectListChanges()));
}

// ****************************************************************************
// Method: QvisAnnotationWindow::hideActiveAnnotations
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to hide the active
//   annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:14:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::hideActiveAnnotations()
{
    //
    // Store any changes made to the widgets since the last apply into the
    // annotation list or else they'll be lost when the viewer sends back
    // the updated annotation list.
    //
    ApplyObjectList(true);

    // Tell the viewer to hide the active annotations.
    viewer->HideActiveAnnotationObjects();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::deleteActiveAnnotations
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to delete the active
//   annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:15:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::deleteActiveAnnotations()
{
    //
    // Make the viewer apply any changes or else they'll be lost when the
    // viewer sends back the updated annotation list.
    //
    ApplyObjectList(true);

    // Tell the viewer to delete the active annotations.
    viewer->DeleteActiveAnnotationObjects();
}
