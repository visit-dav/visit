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

#include <stdio.h>
#include <QvisAnnotationWindow.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qgrid.h>
#include <qradiobutton.h>
#include <qtooltip.h>

#include <QNarrowLineEdit.h>
#include <QvisAnnotationObjectInterface.h>
#include <QvisAnnotationObjectInterfaceFactory.h>
#include <QvisAxisAttributesWidget.h>
#include <QvisColorButton.h>
#include <QvisDialogLineEdit.h>
#include <QvisFontAttributesWidget.h>
#include <QvisLineWidthWidget.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <PlotList.h>
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
//   Brad Whitlock, Wed Mar 21 21:08:54 PST 2007
//   Made it observe the plot list.
//
//   Brad Whitlock, Thu Feb 7 16:32:02 PST 2008
//   Removed some widgets.
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
    plotList = 0;

    objectInterfaces = 0;
    nObjectInterfaces = 0;
    displayInterface = 0;

    activeTab = 0;

    // Initialize parentless widgets.
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
//   Brad Whitlock, Wed Mar 21 21:09:21 PST 2007
//   Detach plot list.
//
//   Brad Whitlock, Thu Feb 7 16:32:19 PST 2008
//   Removed some widgets.
//
// ****************************************************************************

QvisAnnotationWindow::~QvisAnnotationWindow()
{
    if(annotationAtts)
        annotationAtts->Detach(this);

    if(annotationObjectList)
        annotationObjectList->Detach(this);

    if(plotList)
        plotList->Detach(this);

    // Delete parentless widgets.
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
// Method: QvisAnnotationWindow::ConnectPlotList
//
// Purpose: 
//   Makes the window observe the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:09:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    pl->Attach(this);
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
//   Brad Whitlock, Wed Mar 21 21:10:32 PST 2007
//   Added the plot list.
//
// ****************************************************************************

void
QvisAnnotationWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == annotationAtts)
        annotationAtts = 0;

    if(TheRemovedSubject == annotationObjectList)
        annotationObjectList = 0;

    if(TheRemovedSubject == plotList)
        plotList = 0;
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
//   Cyrus Harrison, Mon Jun 18 08:57:46 PDT 2007
//   Added database info path expansion mode label and combo box.
//
//   Cyrus Harrison, Tue Sep 25 10:44:04 PDT 2007
//   Moved general options to a new tab
//
// ****************************************************************************

void
QvisAnnotationWindow::CreateWindowContents()
{
    // Create the tab widget.
    tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));

    topLayout->addWidget(tabs);    

    //
    // Create the window's tabs.
    ///
    CreateGeneralTab();
    Create2DTab();
    Create3DTab();
    CreateColorTab();
    CreateObjectsTab();

    // Show the appropriate page based on the activeTab setting.
    tabs->blockSignals(true);
    if(activeTab == 0)
        tabs->showPage(pageGeneral);
    else if(activeTab == 1)
        tabs->showPage(page2D);
    else if(activeTab == 2)
        tabs->showPage(page3D);
    else if(activeTab == 3)
        tabs->showPage(pageColor);
    else
        tabs->showPage(pageObjects);

    tabs->blockSignals(false);
}


// ****************************************************************************
// Method: QvisAnnotationWindow::CreateGeneralTab
//
// Purpose: 
//   Creates the general options tab.
//
// Note:       I moved this code from CreateWindowContents.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Oct  2 09:29:53 PDT 2007
//
// ****************************************************************************

void
QvisAnnotationWindow::CreateGeneralTab()
{
    //
    // Create the group of widgets that control general annotation options
    //
    pageGeneral = new QWidget(central, "pageGeneral");
    tabs->addTab(pageGeneral, "General");

    // use two layouts, so we can have a compact look
    QVBoxLayout *general_layout = new QVBoxLayout(pageGeneral);
    QGridLayout *glayout = new QGridLayout(general_layout, 3, 2);
    glayout->setSpacing(10);
    glayout->setMargin(10);
    general_layout->addStretch(10);

    // Create a toggle for the legend.
    int row = 0;
    legendInfo = new QCheckBox("Legend", pageGeneral, "legendInfo");
    connect(legendInfo, SIGNAL(toggled(bool)),
            this, SLOT(legendChecked(bool)));
    glayout->addWidget(legendInfo, row, 0);

    // Create a button that can turn off all annotations.
    turnOffAllButton = new QPushButton("No annotations", pageGeneral,
        "turnOffAllButton");
    connect(turnOffAllButton, SIGNAL(clicked()),
            this, SLOT(turnOffAllAnnotations()));
    glayout->addWidget(turnOffAllButton, row, 1);
    ++row;

    //
    // Create the database information
    //
    databaseInfo = new QGroupBox(pageGeneral, "dbGroup");
    databaseInfo->setTitle("Database");
    databaseInfo->setCheckable(true);
    connect(databaseInfo, SIGNAL(toggled(bool)),
            this, SLOT(databaseInfoChecked(bool)));
    glayout->addMultiCellWidget(databaseInfo, row, row, 0, 1);
    QVBoxLayout *dbInnerLayout = new QVBoxLayout(databaseInfo);
    dbInnerLayout->setMargin(10);
    dbInnerLayout->addSpacing(15);
    dbInnerLayout->setSpacing(10);
    QGridLayout *dLayout = new QGridLayout(dbInnerLayout, 3, 2);
    dLayout->setSpacing(5);
    dLayout->setColStretch(1, 10);
    ++row;
    databasePathExpansionMode = new QComboBox(databaseInfo,
        "databasePathExpansionMode");
    databasePathExpansionMode->insertItem("File", 0);
    databasePathExpansionMode->insertItem("Directory", 1);
    databasePathExpansionMode->insertItem("Full", 2);
    databasePathExpansionMode->insertItem("Smart", 3);
    databasePathExpansionMode->insertItem("Smart Directory", 4);
    connect(databasePathExpansionMode, SIGNAL(activated(int)),
            this, SLOT(databasePathExpansionModeChanged(int)));
    databasePathExpansionModeLabel = new QLabel(databasePathExpansionMode,
        "Path Expansion", databaseInfo, "databasePathExpansionModeLabel");
    dLayout->addWidget(databasePathExpansionModeLabel, 0, 0);
    dLayout->addWidget(databasePathExpansionMode, 0, 1);

    QFrame *dbSep = new QFrame(databaseInfo, "labelSep");
    dbSep->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    dLayout->addMultiCellWidget(dbSep, 1, 1, 0, 1);

    databaseInfoFont = new QvisFontAttributesWidget(databaseInfo, "databaseInfoFont");
    connect(databaseInfoFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(databaseInfoFontChanged(const FontAttributes &)));
    dLayout->addMultiCellWidget(databaseInfoFont, 2, 2, 0, 1);

    //
    // Create the user information
    //
    userInfo = new QGroupBox(pageGeneral, "userInfo");
    userInfo->setTitle("User information");
    userInfo->setCheckable(true);
    connect(userInfo, SIGNAL(toggled(bool)),
            this, SLOT(userInfoChecked(bool)));
    glayout->addMultiCellWidget(userInfo, row, row, 0, 1);
    QVBoxLayout *uInnerLayout = new QVBoxLayout(userInfo);
    uInnerLayout->setMargin(10);
    uInnerLayout->addSpacing(15);
    uInnerLayout->setSpacing(10);
    QVBoxLayout *uLayout = new QVBoxLayout(uInnerLayout);
    uLayout->setSpacing(5);
    ++row;
    userInfoFont = new QvisFontAttributesWidget(userInfo, "userInfoFont");
    connect(userInfoFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(userInfoFontChanged(const FontAttributes &)));
    uLayout->addWidget(userInfoFont); 
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
//   Brad Whitlock, Thu Feb 7 17:01:36 PST 2008
//   I changed the method so it creates "General 2D", "X-Axis", and "Y-Axis"
//   tabs instead of the previous organization.
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

    axes2DGroup = new QGroupBox(page2D, "axesGroup2D");
    axes2DGroup->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *lLayout = new QVBoxLayout(axes2DGroup);
    lLayout->setSpacing(5);
    QTabWidget *page2DTabs = new QTabWidget(axes2DGroup, "page2DTabs");
    lLayout->addWidget(page2DTabs);

    // Create the general options page.
    page2DTabs->addTab(CreateGeneralTab2D(page2DTabs), "General 2D");

    // Add the X-axis page.
    axes2D[0] = new QvisAxisAttributesWidget(page2DTabs, "xAxis2D", false, true);
    connect(axes2D[0], SIGNAL(axisChanged(const AxisAttributes &)),
            this, SLOT(xAxisChanged2D(const AxisAttributes &)));
    page2DTabs->addTab(axes2D[0], "X-Axis");

    // Add the Y-axis page.
    axes2D[1] = new QvisAxisAttributesWidget(page2DTabs, "yAxis2D", false, true);
    connect(axes2D[1], SIGNAL(axisChanged(const AxisAttributes &)),
            this, SLOT(yAxisChanged2D(const AxisAttributes &)));
    page2DTabs->addTab(axes2D[1], "Y-Axis");
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateGeneralTab2D
//
// Purpose: 
//   Creates the options for the general 2D tab.
//
// Arguments:
//   parentWidget : The parent of the widgets we'll create.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 16:45:23 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::CreateGeneralTab2D(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "CreateGeneralTab2D");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *lLayout = new QGridLayout(top, 5, 2);
    lLayout->setSpacing(5);
    lLayout->setMargin(5);
    lLayout->setColStretch(1, 10);

    // Create auto set scaling check box.
    int row = 0;
    labelAutoSetScalingToggle2D = new QCheckBox("Auto scale label values",
        top, "labelAutoSetScalingToggle2D");
    connect(labelAutoSetScalingToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(labelAutoSetScalingChecked2D(bool)));
    lLayout->addMultiCellWidget(labelAutoSetScalingToggle2D, row, row, 0, 1);
    ++row;

    // Create auto set ticks check box.
    axesAutoSetTicksToggle2D = new QCheckBox("Auto set ticks", top,
                                         "axesAutoSetTicksToggle2D");
    connect(axesAutoSetTicksToggle2D, SIGNAL(toggled(bool)),
            this, SLOT(axesAutoSetTicksChecked2D(bool)));
    lLayout->addMultiCellWidget(axesAutoSetTicksToggle2D, row, row, 0, 1);
    ++row;

    // Create the 2D tick mark locations combobox.
    axesTickLocationComboBox2D = new QComboBox(top,
        "axesTickLocationComboBox2D");
    axesTickLocationComboBox2D->insertItem("Inside",  0);
    axesTickLocationComboBox2D->insertItem("Outside", 1);
    axesTickLocationComboBox2D->insertItem("Both",    2);
    connect(axesTickLocationComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTickLocationChanged2D(int)));
    lLayout->addWidget(axesTickLocationComboBox2D, row, 1);
    QLabel *l = new QLabel(axesTickLocationComboBox2D, "Tick mark locations",
                   top, "axesTickLocationLabel2D");
    lLayout->addWidget(l, row, 0);
    ++row;

    // Create the 2D tick marks combobox.
    axesTicksComboBox2D = new QComboBox(top, "axesTicksComboBox2D");
    axesTicksComboBox2D->insertItem("Off",         0);
    axesTicksComboBox2D->insertItem("Bottom",      1);
    axesTicksComboBox2D->insertItem("Left",        2);
    axesTicksComboBox2D->insertItem("Bottom-left", 3);
    axesTicksComboBox2D->insertItem("All axes",    4);
    connect(axesTicksComboBox2D, SIGNAL(activated(int)),
            this, SLOT(axesTicksChanged2D(int)));
    lLayout->addWidget(axesTicksComboBox2D, row, 1);
    l = new QLabel(axesTicksComboBox2D, "Show tick marks",
                   top, "axesTicksLabel2D");
    lLayout->addWidget(l, row, 0);
    ++row;

    // Create the 2D line width widget.
    axesLineWidth2D = new QvisLineWidthWidget(0, top,
        "axesLineWidth2D");
    lLayout->addWidget(axesLineWidth2D, row, 1);
    connect(axesLineWidth2D, SIGNAL(lineWidthChanged(int)),
            this, SLOT(axesLineWidthChanged2D(int)));
    l = new QLabel("Line width", top, "axesLineWidthLabel2D");
    lLayout->addWidget(l, row, 0);
    ++row;

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
//   Brad Whitlock, Thu Feb 7 17:03:30 PST 2008
//   I split the tabs into "General 3D", "X-Axis", "Y-Axis", and "Z-Axis".
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
    QHBox *buttons = new QHBox(page3D, "buttons");
    axes3DVisible = new QCheckBox("Show axes", buttons, "axes3DVisible");
    connect(axes3DVisible, SIGNAL(toggled(bool)),
            this, SLOT(axes3DFlagChecked(bool)));

    // Create the toggle for the triad.
    triadFlagToggle = new QCheckBox("Show triad", buttons, "triadFlagToggle");
    connect(triadFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(triadFlagChecked(bool)));

    // Create the toggle for the bbox.
    bboxFlagToggle = new QCheckBox("Show bounding box", buttons, "bboxFlagToggle");
    connect(bboxFlagToggle, SIGNAL(toggled(bool)),
            this, SLOT(bboxFlagChecked(bool)));

    axes3DGroup = new QGroupBox(page3D, "axesGroup3D");
    axes3DGroup->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *lLayout = new QVBoxLayout(axes3DGroup);
    lLayout->setSpacing(5);
    QTabWidget *page3DTabs = new QTabWidget(axes3DGroup, "page3DTabs");
    lLayout->addWidget(page3DTabs);

    // Create the general 3D options page.
    page3DTabs->addTab(CreateGeneralTab3D(page3DTabs),
        "General 3D");

    // Add the X-axis page.
    axes3D[0] = new QvisAxisAttributesWidget(page3DTabs, "xAxis", true, false);
    connect(axes3D[0], SIGNAL(axisChanged(const AxisAttributes &)),
            this, SLOT(xAxisChanged(const AxisAttributes &)));
    page3DTabs->addTab(axes3D[0], "X-Axis");

    // Add the Y-axis page.
    axes3D[1] = new QvisAxisAttributesWidget(page3DTabs, "yAxis", true, false);
    connect(axes3D[1], SIGNAL(axisChanged(const AxisAttributes &)),
            this, SLOT(yAxisChanged(const AxisAttributes &)));
    page3DTabs->addTab(axes3D[1], "Y-Axis");

    // Add the Z-axis page.
    axes3D[2] = new QvisAxisAttributesWidget(page3DTabs, "zAxis", true, false);
    connect(axes3D[2], SIGNAL(axisChanged(const AxisAttributes &)),
            this, SLOT(zAxisChanged(const AxisAttributes &)));
    page3DTabs->addTab(axes3D[2], "Z-Axis");
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateGeneralTab3D
//
// Purpose: 
//   Creates the general 3D options page.
//
// Arguments:
//   parentWidget : The parent widget for the widgets that we're creating.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 17:07:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisAnnotationWindow::CreateGeneralTab3D(QWidget *parentWidget)
{
    QWidget *top0 = new QWidget(parentWidget);
    QVBoxLayout *top0Layout = new QVBoxLayout(top0);
    top0Layout->addSpacing(10);
    QGroupBox *top = new QGroupBox(top0, "Create3DTabForGridAndTicks");
    top->setFrameStyle(QFrame::NoFrame);
    top0Layout->addWidget(top);
    top0Layout->addStretch(10);
    QGridLayout *rLayout = new QGridLayout(top, 5, 2);
    rLayout->setSpacing(5);
    rLayout->setMargin(5);
    rLayout->setColStretch(1, 10);

    int row = 0;
    labelAutoSetScalingToggle = new QCheckBox("Auto scale label values", top,
                                         "labelAutoSetScalingToggle");
    connect(labelAutoSetScalingToggle, SIGNAL(toggled(bool)),
            this, SLOT(labelAutoSetScalingChecked(bool)));
    rLayout->addMultiCellWidget(labelAutoSetScalingToggle, row, row, 0, 1);
    ++row;

    // Create auto set ticks check box.
    axesAutoSetTicksToggle = new QCheckBox("Auto set ticks", top,
                                         "axesAutoSetTicksToggle");
    connect(axesAutoSetTicksToggle, SIGNAL(toggled(bool)),
            this, SLOT(axesAutoSetTicksChecked(bool)));
    rLayout->addMultiCellWidget(axesAutoSetTicksToggle, row, row, 0, 1);
    ++row;

    // Create the 3D tick mark locations combobox.
    axes3DTickLocationComboBox = new QComboBox(top, "axes3DTickLocationComboBox");
    axes3DTickLocationComboBox->insertItem("Inside",  0);
    axes3DTickLocationComboBox->insertItem("Outside", 1);
    axes3DTickLocationComboBox->insertItem("Both",    2);
    connect(axes3DTickLocationComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTickLocationChanged(int)));
    rLayout->addWidget(axes3DTickLocationComboBox, row, 1);
    QLabel *l = new QLabel(axes3DTickLocationComboBox, "Tick mark locations",
                   top, "axes3DTickLocationLabel");
    rLayout->addWidget(l, row, 0);
    ++row;

    // Create the 3D axes type combobox.
    axes3DTypeComboBox = new QComboBox(top, "axes3DTypeComboBox");
    axes3DTypeComboBox->insertItem("Closest triad",  0);
    axes3DTypeComboBox->insertItem("Furthest triad", 1);
    axes3DTypeComboBox->insertItem("Outside edges",  2);
    axes3DTypeComboBox->insertItem("Static triad",   3);
    axes3DTypeComboBox->insertItem("Static edges",   4);
    connect(axes3DTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(axes3DTypeChanged(int)));
    rLayout->addWidget(axes3DTypeComboBox, row, 1);
    l = new QLabel(axes3DTypeComboBox, "Axis type",
                   top, "axes3DTypeLabel");
    rLayout->addWidget(l, row, 0);
    ++row;

    // Create the 2D line width widget.
    axesLineWidth = new QvisLineWidthWidget(0, top,
        "axesLineWidth");
    rLayout->addWidget(axesLineWidth, row, 1);
    connect(axesLineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(axesLineWidthChanged(int)));
    l = new QLabel("Line width", top, "axesLineWidthLabel");
    rLayout->addWidget(l, row, 0);
    ++row;

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
//   Brad Whitlock, Wed Nov 14 11:34:45 PDT 2007
//   Added background image support.
//
// ****************************************************************************

void
QvisAnnotationWindow::CreateColorTab()
{
    int row = 0;

    //
    // Create the group of color-related widgets.
    //
    pageColor = new QGroupBox(central, "pageColor");
    pageColor->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageColor, "Colors");

    QVBoxLayout *vcLayout = new QVBoxLayout(pageColor);
    vcLayout->setMargin(10);
    QGridLayout *cLayout = new QGridLayout(vcLayout, 11, 2);
    cLayout->setSpacing(10);

    // Add the background color widgets.
    backgroundColorButton = new QvisColorButton(pageColor, "backgroundColorButton");
    connect(backgroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(backgroundColorChanged(const QColor &)));
    QLabel *bgColorLabel = new QLabel(backgroundColorButton,
                                      "Background color", pageColor,
                                      "bgColorLabel");
    cLayout->addWidget(bgColorLabel, row, 0);
    cLayout->addWidget(backgroundColorButton, row, 1, AlignLeft);
    ++row;

    // Add the foreground color widgets.
    foregroundColorButton = new QvisColorButton(pageColor, "foregroundColorButton");
    connect(foregroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(foregroundColorChanged(const QColor &)));
    QLabel *fgColorLabel = new QLabel(foregroundColorButton,
                                      "Foreground color", pageColor,
                                      "fgColorLabel");
    cLayout->addWidget(fgColorLabel, row, 0);
    cLayout->addWidget(foregroundColorButton, row, 1, AlignLeft);
    ++row;

    // Create the background style widgets.
    QLabel *backgroundStyleLabel = new QLabel("Background style", pageColor,
                                              "backgroundStyleLabel");
    cLayout->addWidget(backgroundStyleLabel, row, 0);
    backgroundStyleButtons = new QButtonGroup(0, "backgroundStyleButtons");
    connect(backgroundStyleButtons, SIGNAL(clicked(int)),
            this, SLOT(backgroundStyleChanged(int)));
    QGridLayout *mLayout = new QGridLayout(1, 4, 5, "mLayout");
    cLayout->addMultiCellLayout(mLayout, row, row, 1, 2);
    QRadioButton *solid = new QRadioButton("Solid", pageColor, "solid");
    backgroundStyleButtons->insert(solid);
    mLayout->addWidget(solid, 0, 0);
    QRadioButton *gradient = new QRadioButton("Gradient", pageColor, "gradient");
    backgroundStyleButtons->insert(gradient);
    mLayout->addWidget(gradient, 0, 1);
    QRadioButton *image = new QRadioButton("Image", pageColor, "image");
    backgroundStyleButtons->insert(image);
    mLayout->addWidget(image, 0, 2);
    QRadioButton *imageSphere = new QRadioButton("Image sphere", pageColor, "imageSphere");
    backgroundStyleButtons->insert(imageSphere);
    mLayout->addWidget(imageSphere, 0, 3);
    ++row;

    QFrame *splitter = new QFrame(pageColor, "splitter");
    splitter->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter, row, row, 0, 3);  
    ++row;

    // Create the gradient style combobox.
    gradientStyleComboBox = new QComboBox(pageColor, "gradientStyleComboBox");
    gradientStyleComboBox->insertItem("Top to bottom", 0);
    gradientStyleComboBox->insertItem("Bottom to top", 1);
    gradientStyleComboBox->insertItem("Left to right", 2);
    gradientStyleComboBox->insertItem("Right to left", 3);
    gradientStyleComboBox->insertItem("Radial",        4);
    connect(gradientStyleComboBox, SIGNAL(activated(int)),
            this, SLOT(gradientStyleChanged(int)));
    cLayout->addMultiCellWidget(gradientStyleComboBox, row, row, 1, 2);
    gradientStyleLabel = new QLabel(gradientStyleComboBox,
                                    "Gradient style", pageColor,
                                    "gradientStyleLabel");
    cLayout->addWidget(gradientStyleLabel, row, 0);
    ++row;

    // Add the gradient color1 widgets.
    gradientColor1Button = new QvisColorButton(pageColor, "gradientColor1Button");
    connect(gradientColor1Button, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(gradientColor1Changed(const QColor &)));
    gradientColor1Label = new QLabel(gradientColor1Button,
                                     "Gradient color 1", pageColor,
                                     "gradColor1Label");
    cLayout->addWidget(gradientColor1Label, row, 0);
    cLayout->addWidget(gradientColor1Button, row, 1, AlignLeft);
    ++row;

    // Add the gradiant color2 widgets.
    gradientColor2Button = new QvisColorButton(pageColor, "gradientColor2Button");
    connect(gradientColor2Button, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(gradientColor2Changed(const QColor &)));
    gradientColor2Label = new QLabel(gradientColor2Button,
                                     "Gradient color 2", pageColor,
                                     "gradColor2Label");
    cLayout->addWidget(gradientColor2Label, row, 0);
    cLayout->addWidget(gradientColor2Button, row, 1, AlignLeft);
    ++row;

    QFrame *splitter2 = new QFrame(pageColor, "splitter2");
    splitter2->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter2, row, row, 0, 3);  
    ++row;

    // Add the image selection widget
    backgroundImage = new QvisDialogLineEdit(pageColor, "backgroundImage");
    backgroundImage->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    connect(backgroundImage, SIGNAL(returnPressed()),
            this, SLOT(backgroundImageChanged()));
    cLayout->addMultiCellWidget(backgroundImage, row, row, 1, 2);
    backgroundImageLabel = new QLabel(backgroundImage, "Background image",
        pageColor, "backgroundImageLabel");
    cLayout->addWidget(backgroundImageLabel, row, 0);
    QString disclaimer("The local file must be accessible to the "
        "compute engine in order to be used in scalable rendering mode.");
    QToolTip::add(backgroundImage, disclaimer);
    QToolTip::add(backgroundImageLabel, disclaimer);
    ++row;

    // Add the image repeat x,y widgets. 
    imageRepeatX = new QSpinBox(1, 100, 1, pageColor, "imageRepeatX");
    imageRepeatX->setButtonSymbols(QSpinBox::PlusMinus);
    connect(imageRepeatX, SIGNAL(valueChanged(int)),
            this, SLOT(imageRepeatXChanged(int)));
    cLayout->addWidget(imageRepeatX, row, 1);
    imageRepeatXLabel = new QLabel(imageRepeatX, "Repetitions in X", pageColor,
        "imageRepeatXLabel");
    cLayout->addWidget(imageRepeatXLabel, row, 0);
    ++row;

    imageRepeatY = new QSpinBox(1, 100, 1, pageColor, "imageRepeatY");
    imageRepeatY->setButtonSymbols(QSpinBox::PlusMinus);
    connect(imageRepeatY, SIGNAL(valueChanged(int)),
            this, SLOT(imageRepeatYChanged(int)));
    cLayout->addWidget(imageRepeatY, row, 1);
    imageRepeatYLabel = new QLabel(imageRepeatY, "Repetitions in Y", pageColor,
        "imageRepeatYLabel");
    cLayout->addWidget(imageRepeatYLabel, row, 0);
    ++row;

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
//   Brad Whitlock, Tue Mar 20 15:19:03 PST 2007
//   Made it so some interfaces don't have to provide a new instance button.
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
    hLayout->setStretchFactor(annotListLayout, 10);
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
            objectInterfaces[i] = factory.CreateInterface(
                (AnnotationObject::AnnotationType)i, pageObjects);

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
                if(objectInterfaces[i]->AllowInstantiation())
                {
                    QPushButton *btn = new QPushButton(objectInterfaces[i]->GetName(),
                        newObjectGroup);
                    objButtonGroup->insert(btn, i);
                    objButtonLayout->addWidget(btn);
                }
    
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
//   Brad Whitlock, Wed Mar 21 21:11:09 PST 2007
//   Update the annotation object interfaces when the plot list changes.
//
// ****************************************************************************

void
QvisAnnotationWindow::UpdateWindow(bool doAll)
{
    if(annotationAtts == 0 || annotationObjectList == 0)
        return;

    if(SelectedSubject() == annotationAtts || doAll)
        UpdateAnnotationControls(doAll);
    if(SelectedSubject() == annotationObjectList ||
       SelectedSubject() == plotList ||
       doAll)
        UpdateAnnotationObjectControls(doAll);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::UpdateAxes2D
//
// Purpose: 
//   Updates the 2D axis settings in the window.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 17:25:58 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::UpdateAxes2D()
{
    const Axes2D &axes = annotationAtts->GetAxes2D();

    axesFlagToggle2D->blockSignals(true);
    axesFlagToggle2D->setChecked(axes.GetVisible());
    axesFlagToggle2D->blockSignals(false);
    axes2DGroup->setEnabled(axes.GetVisible());

    axesAutoSetTicksToggle2D->blockSignals(true);
    axesAutoSetTicksToggle2D->setChecked(axes.GetAutoSetTicks());
    axesAutoSetTicksToggle2D->blockSignals(false);

    labelAutoSetScalingToggle2D->blockSignals(true);
    labelAutoSetScalingToggle2D->setChecked(axes.GetAutoSetScaling());
    labelAutoSetScalingToggle2D->blockSignals(false);

    axesLineWidth2D->blockSignals(true);
    axesLineWidth2D->SetLineWidth(axes.GetLineWidth());
    axesLineWidth2D->blockSignals(false);
 
    axesTickLocationComboBox2D->blockSignals(true);
    axesTickLocationComboBox2D->setCurrentItem(axes.GetTickLocation());
    axesTickLocationComboBox2D->blockSignals(false);

    axesTicksComboBox2D->blockSignals(true);
    axesTicksComboBox2D->setCurrentItem(axes.GetTickAxes());
    axesTicksComboBox2D->blockSignals(false);

    // Update the controls in the axes.
    axes2D[0]->setAutoScaling(axes.GetAutoSetScaling());
    axes2D[0]->setAutoTickMarks(axes.GetAutoSetTicks());
    axes2D[0]->setAxisAttributes(axes.GetXAxis());
    axes2D[1]->setAutoScaling(axes.GetAutoSetScaling());
    axes2D[1]->setAutoTickMarks(axes.GetAutoSetTicks());
    axes2D[1]->setAxisAttributes(axes.GetYAxis());
}

// ****************************************************************************
// Method: QvisAnnotationWindow::UpdateAxes3D
//
// Purpose: 
//   Updates the 3D axis settings in the window.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 17:38:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::UpdateAxes3D()
{
    const Axes3D &axes = annotationAtts->GetAxes3D();

    axes3DVisible->blockSignals(true);
    axes3DVisible->setChecked(axes.GetVisible());
    axes3DVisible->blockSignals(false);
    axes3DGroup->setEnabled(axes.GetVisible());

    axesAutoSetTicksToggle->blockSignals(true);
    axesAutoSetTicksToggle->setChecked(axes.GetAutoSetTicks());
    axesAutoSetTicksToggle->blockSignals(false);

    labelAutoSetScalingToggle->blockSignals(true);
    labelAutoSetScalingToggle->setChecked(axes.GetAutoSetScaling());
    labelAutoSetScalingToggle->blockSignals(false);

    axesLineWidth->blockSignals(true);
    axesLineWidth->SetLineWidth(axes.GetLineWidth());
    axesLineWidth->blockSignals(false);

    axes3DTickLocationComboBox->blockSignals(true);
    axes3DTickLocationComboBox->setCurrentItem(axes.GetTickLocation());
    axes3DTickLocationComboBox->blockSignals(false);

    axes3DTypeComboBox->blockSignals(true);
    axes3DTypeComboBox->setCurrentItem(axes.GetAxesType());
    axes3DTypeComboBox->blockSignals(false);

    triadFlagToggle->blockSignals(true);
    triadFlagToggle->setChecked(axes.GetTriadFlag());
    triadFlagToggle->blockSignals(false);

    bboxFlagToggle->blockSignals(true);
    bboxFlagToggle->setChecked(axes.GetBboxFlag());
    bboxFlagToggle->blockSignals(false);

    // Update the controls in the axes.
    axes3D[0]->setAutoScaling(axes.GetAutoSetScaling());
    axes3D[0]->setAutoTickMarks(axes.GetAutoSetTicks());
    axes3D[0]->setAxisAttributes(axes.GetXAxis());
    axes3D[1]->setAutoScaling(axes.GetAutoSetScaling());
    axes3D[1]->setAutoTickMarks(axes.GetAutoSetTicks());
    axes3D[1]->setAxisAttributes(axes.GetYAxis());
    axes3D[2]->setAutoScaling(axes.GetAutoSetScaling());
    axes3D[2]->setAutoTickMarks(axes.GetAutoSetTicks());
    axes3D[2]->setAxisAttributes(axes.GetZAxis());
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
//   Brad Whitlock, Wed Nov 14 11:34:45 PDT 2007
//   Added background image support.
//
//   Brad Whitlock, Fri Dec 14 16:41:02 PST 2007
//   Made it use ids for case labels.
//
//   Brad Whitlock, Thu Feb 7 17:43:01 PST 2008
//   Updated to new AnnotationAttributes interface.
//
// ****************************************************************************

void
QvisAnnotationWindow::UpdateAnnotationControls(bool doAll)
{
    QColor c;
    bool isGradient, vals[4];
    const unsigned char *cptr = 0;

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
        case AnnotationAttributes::ID_axes2D:
            UpdateAxes2D();
            break;
        case AnnotationAttributes::ID_axes3D:
            UpdateAxes3D();
            break;
        case AnnotationAttributes::ID_userInfoFlag:
            userInfo->blockSignals(true);
            userInfo->setChecked(annotationAtts->GetUserInfoFlag());
            userInfo->blockSignals(false);
            break;
        case AnnotationAttributes::ID_userInfoFont:
            userInfoFont->setFontAttributes(annotationAtts->GetUserInfoFont());
            break;
        case AnnotationAttributes::ID_databaseInfoFlag:
            databaseInfo->blockSignals(true);
            databaseInfo->setChecked(annotationAtts->GetDatabaseInfoFlag());
            databaseInfo->blockSignals(false);

            databasePathExpansionMode->setEnabled(annotationAtts->GetDatabaseInfoFlag());
            databasePathExpansionModeLabel->setEnabled(annotationAtts->GetDatabaseInfoFlag());
            break;
        case AnnotationAttributes::ID_databaseInfoFont:
            databaseInfoFont->setFontAttributes(annotationAtts->GetDatabaseInfoFont());
            break;
        case AnnotationAttributes::ID_databaseInfoExpansionMode:
            databasePathExpansionMode->blockSignals(true);
            databasePathExpansionMode->setCurrentItem(
                                annotationAtts->GetDatabaseInfoExpansionMode());
            databasePathExpansionMode->blockSignals(false);
            break;
        case AnnotationAttributes::ID_legendInfoFlag:
            legendInfo->blockSignals(true);
            legendInfo->setChecked(annotationAtts->GetLegendInfoFlag());
            legendInfo->blockSignals(false);
            break;
        case AnnotationAttributes::ID_backgroundColor:
            cptr = annotationAtts->GetBackgroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            backgroundColorButton->blockSignals(true);
            backgroundColorButton->setButtonColor(c);
            backgroundColorButton->blockSignals(false);
            break;
        case AnnotationAttributes::ID_foregroundColor:
            cptr = annotationAtts->GetForegroundColor().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            foregroundColorButton->blockSignals(true);
            foregroundColorButton->setButtonColor(c);
            foregroundColorButton->blockSignals(false);
            break;
        case AnnotationAttributes::ID_gradientBackgroundStyle:
            gradientStyleComboBox->blockSignals(true);
            gradientStyleComboBox->setCurrentItem(annotationAtts->GetGradientBackgroundStyle());
            gradientStyleComboBox->blockSignals(false);
            break;
        case AnnotationAttributes::ID_gradientColor1:
            cptr = annotationAtts->GetGradientColor1().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor1Button->blockSignals(true);
            gradientColor1Button->setButtonColor(c);
            gradientColor1Button->blockSignals(false);
            break;
        case AnnotationAttributes::ID_gradientColor2:
            cptr = annotationAtts->GetGradientColor2().GetColor();
            c = QColor(int(cptr[0]), int(cptr[1]), int(cptr[2]));
            gradientColor2Button->blockSignals(true);
            gradientColor2Button->setButtonColor(c);
            gradientColor2Button->blockSignals(false);
            break;
        case AnnotationAttributes::ID_backgroundMode:
            vals[0] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::Solid;
            vals[1] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::Gradient;
            vals[2] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::Image;
            vals[3] = annotationAtts->GetBackgroundMode()==AnnotationAttributes::ImageSphere;
            SetButtonGroup(backgroundStyleButtons, vals);

            // Set widget sensitivity based on this field.
            isGradient = (annotationAtts->GetBackgroundMode() == 1);
            gradientStyleLabel->setEnabled(isGradient);
            gradientStyleComboBox->setEnabled(isGradient);
            gradientColor1Label->setEnabled(isGradient);
            gradientColor1Button->setEnabled(isGradient);
            gradientColor2Label->setEnabled(isGradient);
            gradientColor2Button->setEnabled(isGradient);
            backgroundImage->setEnabled(vals[2] || vals[3]);
            backgroundImageLabel->setEnabled(vals[2] || vals[3]);
            imageRepeatX->setEnabled(vals[2] || vals[3]);
            imageRepeatXLabel->setEnabled(vals[2] || vals[3]);
            imageRepeatY->setEnabled(vals[2] || vals[3]);
            imageRepeatYLabel->setEnabled(vals[2] || vals[3]);
            break;
        case AnnotationAttributes::ID_backgroundImage:
            backgroundImage->setText(annotationAtts->GetBackgroundImage().c_str());
            break;
        case AnnotationAttributes::ID_imageRepeatX:
            imageRepeatX->blockSignals(true);
            imageRepeatX->setValue(annotationAtts->GetImageRepeatX());
            imageRepeatX->blockSignals(false);
            break;
        case AnnotationAttributes::ID_imageRepeatY:
            imageRepeatY->blockSignals(true);
            imageRepeatY->setValue(annotationAtts->GetImageRepeatY());
            imageRepeatY->blockSignals(false);
            break;
        }
    } // end for
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
//   Brad Whitlock, Tue Mar 20 15:25:46 PST 2007
//   Set the enabled state for the hide/show and delete buttons.
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
    bool hideDeleteEnabled = true;
    annotationListBox->blockSignals(true);
    annotationListBox->clear();
    for(i = 0; i < annotationObjectList->GetNumAnnotations(); ++i)
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

                if(annot.GetActive())
                    hideDeleteEnabled &= objectInterfaces[annotType]->AllowInstantiation();
            }
        }
    }
    annotationListBox->blockSignals(false);

    // Set the enabled state for the hide/show and delete buttons.
    hideShowAnnotationButton->setEnabled(hideDeleteEnabled);
    deleteAnnotationButton->setEnabled(hideDeleteEnabled);

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
//   Brad Whitlock, Fri Feb 8 10:47:31 PDT 2008
//   Totally rewrote.
//
// ****************************************************************************

void
QvisAnnotationWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == AnnotationAttributes::ID_axes2D || doAll)
    {
        annotationAtts->GetAxes2D().SetXAxis(axes2D[0]->getAxisAttributes());
        annotationAtts->GetAxes2D().SetYAxis(axes2D[1]->getAxisAttributes());
        annotationAtts->SelectAxes2D();
    }

    if(which_widget == AnnotationAttributes::ID_axes3D || doAll)
    {
        annotationAtts->GetAxes3D().SetXAxis(axes3D[0]->getAxisAttributes());
        annotationAtts->GetAxes3D().SetYAxis(axes3D[1]->getAxisAttributes());
        annotationAtts->GetAxes3D().SetZAxis(axes3D[2]->getAxisAttributes());
        annotationAtts->SelectAxes3D();
    }

    if(which_widget == AnnotationAttributes::ID_userInfoFont || doAll)
    {
        annotationAtts->SetDatabaseInfoFont(userInfoFont->getFontAttributes());
    }

    if(which_widget == AnnotationAttributes::ID_databaseInfoFont || doAll)
    {
        annotationAtts->SetDatabaseInfoFont(databaseInfoFont->getFontAttributes());
    }

    if (which_widget == AnnotationAttributes::ID_backgroundImage || doAll)
    {
        QString temp(backgroundImage->displayText().stripWhiteSpace());
        annotationAtts->SetBackgroundImage(temp.latin1());
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
        GetViewerMethods()->SetAnnotationAttributes();
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
        GetViewerMethods()->SetAnnotationObjectOptions();
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
        if(activeTab < 0 || activeTab > 4)
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
    GetViewerMethods()->SetDefaultAnnotationAttributes();

    annotationObjectList->Notify();
    GetViewerMethods()->SetDefaultAnnotationObjectList();
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
    GetViewerMethods()->ResetAnnotationAttributes();

    // Tell the viewer to reset the annotation object list to the last applied
    // values.
    GetViewerMethods()->ResetAnnotationObjectList();
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
//   Brad Whitlock, Tue Mar 20 14:33:49 PST 2007
//   Fixed so it uses the new names.
//
// ****************************************************************************

void
QvisAnnotationWindow::tabSelected(const QString &tabLabel)
{
    if(tabLabel == QString("2D"))
        activeTab = 0;
    else if(tabLabel == QString("3D"))
        activeTab = 1;
    else if(tabLabel == QString("Colors"))
        activeTab = 2;
    else if(tabLabel == QString("Objects"))
        activeTab = 3;
    else
    {
        debug1 << "QvisAnnotationWindow::tabSelected: Unsupported tab name. FIX ME!" << endl;
    }
}

// ****************************************************************************
// Qt slots for the General tab
// ****************************************************************************

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
// Method: QvisAnnotationWindow::userInfoFontChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user font changes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:29:48 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::userInfoFontChanged(const FontAttributes &f)
{
    annotationAtts->SetUserInfoFont(f);
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
//   Cyrus Harrison, Mon Jun 18 08:59:15 PDT 2007
//   Added enable/disable for path expansion mode
//
//   Brad Whitlock, Fri Feb 8 13:56:08 PST 2008
//   Moved db expansion coding to the UpdateWindow method.
//
// ****************************************************************************

void
QvisAnnotationWindow::databaseInfoChecked(bool val)
{
    annotationAtts->SetDatabaseInfoFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::databasePathExpansionModeChanged
//
// Purpose:
//   This is a Qt slot function that sets the database path expansion mode.
//
// Arguments:
//   index : The new path expansion mode
//
// Programmer: Cyrus Harrison
// Creation:   Monday June 18, 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 19 09:36:24 PDT 2007
//   Removed cout debug print.
//
//   Cyrus Harrison, Thu Sep 27 09:10:25 PDT 2007
//   Added support for new path expansion options
//
// ****************************************************************************

void
QvisAnnotationWindow::databasePathExpansionModeChanged(int index)
{
    if (index == 0)
    {annotationAtts->SetDatabaseInfoExpansionMode(AnnotationAttributes::File);}
    else if (index == 1)
    {annotationAtts->SetDatabaseInfoExpansionMode(AnnotationAttributes::Directory);}
    else if (index == 2)
    {annotationAtts->SetDatabaseInfoExpansionMode(AnnotationAttributes::Full);}
    else if (index == 3)
    {annotationAtts->SetDatabaseInfoExpansionMode(AnnotationAttributes::Smart);}
    else if (index == 4)
    {annotationAtts->SetDatabaseInfoExpansionMode(AnnotationAttributes::SmartDirectory);}
    
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::databaseInfoFontChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the database font changes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:29:48 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::databaseInfoFontChanged(const FontAttributes &f)
{
    annotationAtts->SetDatabaseInfoFont(f);
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
//   Brad Whitlock, Fri Jan 25 11:50:50 PST 2008
//   Updated for new AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::turnOffAllAnnotations()
{
    //
    // Hide all of the annotations that are not annotation objects.
    //
    annotationAtts->GetAxes2D().SetVisible(false);
    annotationAtts->GetAxes3D().SetVisible(false);
    annotationAtts->GetAxes3D().SetTriadFlag(false);
    annotationAtts->GetAxes3D().SetBboxFlag(false);
    annotationAtts->SetUserInfoFlag(false);
    annotationAtts->SetDatabaseInfoFlag(false);
    annotationAtts->SetLegendInfoFlag(false);
    Apply();

    //
    // Hide all of the annotation objects.
    //
    if(annotationObjectList->GetNumAnnotations() > 0)
    {
        for(int i = 0; i < annotationObjectList->GetNumAnnotations(); ++i)
        {           
            AnnotationObject &annot = annotationObjectList->operator[](i);
            annot.SetVisible(false);
        }

        ApplyObjectList();
    }
}

// ****************************************************************************
// Qt slots for the 2D tab
// ****************************************************************************

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
//   Brad Whitlock, Fri Feb 8 10:56:51 PDT 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::axesFlagChecked2D(bool val)
{
    annotationAtts->GetAxes2D().SetVisible(val);
    annotationAtts->SelectAxes2D();
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
//   Brad Whitlock, Fri Feb 8 10:56:51 PDT 2008
//   Updated AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesAutoSetTicksChecked2D(bool val)
{
    annotationAtts->GetAxes2D().SetAutoSetTicks(val);
    annotationAtts->SelectAxes2D();
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
//   Brad Whitlock, Fri Feb 8 10:56:51 PDT 2008
//   Updated AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::labelAutoSetScalingChecked2D(bool val)
{
    annotationAtts->GetAxes2D().SetAutoSetScaling(val);
    annotationAtts->SelectAxes2D();
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
//   Brad Whitlock, Fri Feb 8 10:56:51 PDT 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************
 
void
QvisAnnotationWindow::axesLineWidthChanged2D(int index)
{
    annotationAtts->GetAxes2D().SetLineWidth(index);
    annotationAtts->SelectAxes2D();
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
//   Brad Whitlock, Fri Feb 8 11:00:54 PDT 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::axesTicksChanged2D(int index)
{
    if (index == 0)
        annotationAtts->GetAxes2D().SetTickAxes(Axes2D::Off);
    else if (index == 1)
        annotationAtts->GetAxes2D().SetTickAxes(Axes2D::Bottom);
    else if (index == 2)
        annotationAtts->GetAxes2D().SetTickAxes(Axes2D::Left);
    else if (index == 3)
        annotationAtts->GetAxes2D().SetTickAxes(Axes2D::BottomLeft);
    else if (index == 4)
        annotationAtts->GetAxes2D().SetTickAxes(Axes2D::All);
    annotationAtts->SelectAxes2D();
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
//   Brad Whitlock, Fri Feb 8 11:03:19 PDT 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::axesTickLocationChanged2D(int index)
{
    if (index == 0)
        annotationAtts->GetAxes2D().SetTickLocation(Axes2D::Inside);
    else if (index == 1)
        annotationAtts->GetAxes2D().SetTickLocation(Axes2D::Outside);
    else if (index == 2)
        annotationAtts->GetAxes2D().SetTickLocation(Axes2D::Both);
    annotationAtts->SelectAxes2D();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xAxisChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when anything in the 2D X-axis
//   page changes.
//
// Arguments:
//   aa : The new axis attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:13:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xAxisChanged2D(const AxisAttributes &aa)
{
    annotationAtts->GetAxes2D().SetXAxis(aa);
    annotationAtts->SelectAxes2D();
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yAxisChanged2D
//
// Purpose: 
//   This is a Qt slot function that is called when anything in the 2D Y-axis
//   page changes.
//
// Arguments:
//   aa : The new axis attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:13:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yAxisChanged2D(const AxisAttributes &aa)
{
    annotationAtts->GetAxes2D().SetYAxis(aa);
    annotationAtts->SelectAxes2D();
    Apply();
}

// ****************************************************************************
// Qt slots for the 3D tab
// ****************************************************************************

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
//   Brad Whitlock, Fri Feb 8 11:04:27 PDT 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::axes3DFlagChecked(bool val)
{
    annotationAtts->GetAxes3D().SetVisible(val);
    annotationAtts->SelectAxes3D();
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
//   Brad Whitlock, Fri Feb 8 11:04:27 PDT 2008
//   Updated AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::labelAutoSetScalingChecked(bool val)
{
    annotationAtts->GetAxes3D().SetAutoSetScaling(val);
    annotationAtts->SelectAxes3D();
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axesAutoSetTicksChecked
//
// Purpose: 
//   This is a Qt slot function that sets the 3D auto set ticks flag.
//
// Arguments:
//   val : The new toggle state.
//
// Note:       SetUpdate(false) is not called because we want the widget
//             sensitivity to update.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 12:11:47 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::axesAutoSetTicksChecked(bool val)
{
    annotationAtts->GetAxes3D().SetAutoSetTicks(val);
    annotationAtts->SelectAxes3D();
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
//   Brad Whitlock, Fri Jan 25 11:50:50 PST 2008
//   Updated AnnotationAttributes.
//
// ****************************************************************************

void
QvisAnnotationWindow::axes3DTickLocationChanged(int index)
{
    if (index == 0)
        annotationAtts->GetAxes3D().SetTickLocation(Axes3D::Inside);
    else if (index == 1)
        annotationAtts->GetAxes3D().SetTickLocation(Axes3D::Outside);
    else if (index == 1)
        annotationAtts->GetAxes3D().SetTickLocation(Axes3D::Both);
    annotationAtts->SelectAxes3D();
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
//   Brad Whitlock, Fri Jan 25 11:50:50 PST 2008
//   Updated AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::axes3DTypeChanged(int index)
{
    if (index == 0)
        annotationAtts->GetAxes3D().SetAxesType(Axes3D::ClosestTriad);
    else if (index == 1)
        annotationAtts->GetAxes3D().SetAxesType(Axes3D::FurthestTriad);
    else if (index == 2)
        annotationAtts->GetAxes3D().SetAxesType(Axes3D::OutsideEdges);
    else if (index == 3)
        annotationAtts->GetAxes3D().SetAxesType(Axes3D::StaticTriad);
    else if (index == 4)
        annotationAtts->GetAxes3D().SetAxesType(Axes3D::StaticEdges);
    annotationAtts->SelectAxes3D();
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
//   Brad Whitlock, Fri Jan 25 11:50:50 PST 2008
//   Updated AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::triadFlagChecked(bool val)
{
    annotationAtts->GetAxes3D().SetTriadFlag(val);
    annotationAtts->SelectAxes3D();
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
//   Brad Whitlock, Fri Jan 25 11:50:50 PST 2008
//   Updated for new AnnotationAttributes.
//   
// ****************************************************************************

void
QvisAnnotationWindow::bboxFlagChecked(bool val)
{
    annotationAtts->GetAxes3D().SetBboxFlag(val);
    annotationAtts->SelectAxes3D();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::axesLineWidthChanged
//
// Purpose:
//   This is a Qt slot function that is called when the 3D axes line width
//   is changed.
//
// Arguments:
//   index:    The new line width.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:16:17 PDT 2008
//
// Modifications:
//
// ****************************************************************************
 
void
QvisAnnotationWindow::axesLineWidthChanged(int index)
{
    annotationAtts->GetAxes3D().SetLineWidth(index);
    annotationAtts->SelectAxes3D();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::xAxisChanged
//
// Purpose: 
//   This is a Qt slot function that is called when anything in the 3D X-axis
//   page changes.
//
// Arguments:
//   aa : The new axis attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:13:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::xAxisChanged(const AxisAttributes &aa)
{
    annotationAtts->GetAxes3D().SetXAxis(aa);
    annotationAtts->SelectAxes3D();
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::yAxisChanged
//
// Purpose: 
//   This is a Qt slot function that is called when anything in the 3D Y-axis
//   page changes.
//
// Arguments:
//   aa : The new axis attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:13:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::yAxisChanged(const AxisAttributes &aa)
{
    annotationAtts->GetAxes3D().SetYAxis(aa);
    annotationAtts->SelectAxes3D();
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::zAxisChanged
//
// Purpose: 
//   This is a Qt slot function that is called when anything in the 3D Z-axis
//   page changes.
//
// Arguments:
//   aa : The new axis attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 11:13:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::zAxisChanged(const AxisAttributes &aa)
{
    annotationAtts->GetAxes3D().SetZAxis(aa);
    annotationAtts->SelectAxes3D();
    Apply();
}

// ****************************************************************************
// Qt slots for the Colors tab
// ****************************************************************************

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
//   Brad Whitlock, Wed Nov 14 13:26:34 PST 2007
//   Added support for image backgrounds.
//
// ****************************************************************************

void
QvisAnnotationWindow::backgroundStyleChanged(int index)
{
    if (index == 0)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::Solid);
    else if (index == 1)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::Gradient);
    else if (index == 2)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::Image);
    else if (index == 3)
        annotationAtts->SetBackgroundMode(AnnotationAttributes::ImageSphere);
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
// Method: QvisAnnotationWindow::backgroundImageChanged.
//
// Purpose: 
//   This is a Qt slot function that is called when the background image changes.
//
// Programmer: Brad Whitlock
// Creation:   
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWindow::backgroundImageChanged()
{
    GetCurrentValues(AnnotationAttributes::ID_backgroundImage);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::imageRepeatXChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the number of image repeats
//   in X changes.
//
// Arguments:
//   value : The new number of repeats.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:11:15 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::imageRepeatXChanged(int value)
{
    annotationAtts->SetImageRepeatX(value);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnnotationWindow::imageRepeatYChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the number of image repeats
//   in Y changes.
//
// Arguments:
//   value : The new number of repeats.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:11:15 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWindow::imageRepeatYChanged(int value)
{
    annotationAtts->SetImageRepeatY(value);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Qt slots for the Objects tab
// ****************************************************************************

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
//   Brad Whitlock, Tue Mar 20 09:47:08 PDT 2007
//   Allow the user to name the object.
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

    // Prompt the user for a name for the new annotation object.
    bool ok = false;
    QString newName(GetViewerState()->GetAnnotationObjectList()->GetNewObjectName().c_str());
    QString annotName = QInputDialog::getText("VisIt", 
        "Enter a name for the new annotation object.",
        QLineEdit::Normal, newName, &ok, this);

    //
    // Tell the viewer to create a new annotation object.
    //
    if(ok && !annotName.isEmpty())
    {
        GetViewerMethods()->AddAnnotationObject(annotType, annotName.latin1());
    }
    else
        Warning("No annotation object was created.");
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
//   Brad Whitlock, Tue Mar 20 15:14:24 PST 2007
//   Disable the hide/show and delete buttons when the interface does not
//   permit objects to be instantiated.
//
// ****************************************************************************

void
QvisAnnotationWindow::setActiveAnnotations()
{
    // Set the active flag on the annotation objects in the annotation
    // object list.
    for(int i = 0; i < annotationObjectList->GetNumAnnotations(); ++i)
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
    GetViewerMethods()->HideActiveAnnotationObjects();
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
    GetViewerMethods()->DeleteActiveAnnotationObjects();
}
