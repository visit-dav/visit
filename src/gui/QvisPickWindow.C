// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <stdio.h> // for sscanf
#include <string>
#include <vector>

#include <QComboBox>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QSpinBox>
#include <QStringList>
#include <QWidget>
#include <QColorDialog>

#include <QvisTimeQueryOptionsWidget.h>
#include <QvisVariableButton.h>
#include <QvisPickWindow.h>
#include <PickAttributes.h>
#include <PlotList.h>
#include <ViewerProxy.h>
#include <DebugStream.h>
#include <PickVarInfo.h>
#include <DataNode.h>
#include <StringHelpers.h>
#include <QueryList.h>

#ifdef _WIN32
#include <QTemporaryFile>
#include <InstallationFunctions.h>
#endif

using std::string;
using std::vector;

// ****************************************************************************
// Method: PickRecord::PickRecord
//
// Purpose:
//   This is the empty constructor for the PickRecord class.
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//   Jonathan Byrd, Mon Feb 4, 2013
//   Add variable and element to pick record
//
// ****************************************************************************

PickRecord::PickRecord() {
    domain = -1;
    element = -1;
    variable = "<none>";
    value = "";
}

// ****************************************************************************
// Method: PickRecord::PickRecord
//
// Purpose:
//   This is the constructor for the PickRecord class.
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//   Jonathan Byrd, Mon Feb 4, 2013
//   Add variable and element to pick record
//
// ****************************************************************************

PickRecord::PickRecord(int dom, std::string &var, int elem, std::string &val) {
    domain = dom;
    element = elem;
    variable = var;
    value = val;
}

// ****************************************************************************
// Method: PickRecord::reset
//
// Purpose:
//   This resets the PickRecord to 'empty' data not representing
//   any domain for any tab.
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//   Jonathan Byrd, Mon Feb 4, 2013
//   Add variable and element to pick record
// ****************************************************************************

void PickRecord::reset() {
    domain = -1;
    element = -1;
    variable = "<none>";
    value = "";
}

// ****************************************************************************
// Method: QvisPickWindow::QvisPickWindow
//
// Purpose:
//   Cconstructor for the QvisPickWindow class.
//
// Arguments:
//   subj      : The PickAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Brad Whitlock, Thu Nov 7 16:08:55 PST 2002
//   I made the window resize better by passing false for the stretch flag.
//
//   Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//   Made the window only have the Apply button (not reset or make default).
//
//   Brad Whitlock, Wed Aug 27 08:36:19 PDT 2003
//   Added autoShow flag.
//
//   Brad Whitlock, Tue Sep 9 09:30:01 PDT 2003
//   I made nextPage and lastLetter be members.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Added savePicks flag.
//
//   Kathleen Bonnell, Wed Dec 17 15:06:34 PST 2003
//   Made the window have all buttons.
//
//   Ellen Tarwater, Fri May 18, 2007
//   Added save Count for 'Save Pick as...' functionality.
//
//   Katheen Bonnell, Thu Nov 29 15:35:15 PST 2007
//   Added defaultAutoShow, defaultSavePicks, defaultNumTabs so that their
//   counterparts can be 'reset' correclty from user-saved values when needed.
//
//   Brad Whitlock, Wed Apr  9 11:30:06 PDT 2008
//   QString for caption, shortName.
//
//   Kathleen Biagas, Fri Aug 26 11:11:26 PDT 2011
//   Added activeOptionsTab, plotList.
//
//   Kathleen Biagas, Wed Jan 25 16:00:17 MST 2012
//   Added timeOptsTabIndex.
//
// ****************************************************************************

QvisPickWindow::QvisPickWindow(PickAttributes *subj, const QString &caption,
                           const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                           QvisPostableWindowObserver::AllExtraButtons, false),
    lastLetter(" ")
{
    pickAtts = subj;
    defaultAutoShow = autoShow = true;
    nextPage = 0;
    defaultSavePicks = savePicks = false;
    defaultNumTabs = 8;
    saveCount = 0;
    activeOptionsTab = 0;
    timeOptsTabIndex = 0;
    plotList = 0;
}

// ****************************************************************************
// Method: QvisPickWindow::~QvisPickWindow
//
// Purpose:
//   This is the destructor for the QvisPickWindow class.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Kathleen Biagas, Fri Aug 26 11:11:42 PDT 2011
//   Handle plotList.
//
// ****************************************************************************

QvisPickWindow::~QvisPickWindow()
{
    pickAtts = 0;
    if (plotList)
        plotList->Detach(this);
}

// ****************************************************************************
// Method: QvisPickWindow::CreateWindowContents
//
// Purpose:
//   This method creates the widgets for the Pick operator window.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 17:17:40 PST 2002
//   Changed the pages and infoLists arrays to be length MAX_PICK_TABS to
//   get around a weird memory problem where I could not seem to free those
//   arrays.
//
//   Kathleen Bonnell, Thu Jun 27 14:37:29 PDT 2002
//   Set column mode to 1.
//
//   Brad Whitlock, Thu Nov 7 17:34:24 PST 2002
//   Added a stretch factor and a minimum height to the tab widget.
//
//   Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002
//   Added useNodeCoords checkbox.
//
//   Brad Whitlock, Wed Aug 27 08:38:55 PDT 2003
//   I added a checkbox to set autoShow mode.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Added  savePicks checkbox.
//
//   Kathleen Bonnell, Tue Nov 18 14:03:22 PST 2003
//   Added  logicalZone checkbox.
//
//   Kathleen Bonnell, Wed Dec 17 15:19:46 PST 2003
//   Reorganized, added more buttons for user control of what type of
//   output pick returns.
//
//   Kathleen Bonnell, Thu Apr  1 18:42:52 PST 2004
//   Added TimeCurve checkbox.
//
//   Kathleen Bonnell, Wed Jun  9 09:41:15 PDT 2004
//   Added conciseOutput, showMeshName and showTimestep checkboxes.
//
//   Brad Whitlock, Fri Dec 10 09:50:04 PDT 2004
//   Changed a label into a variable button so it is easier to add
//   variables to the pick variables.
//
//   Kathleen Bonnell, Wed Dec 15 08:20:11 PST 2004
//   Added 'displayGlobalIds' checkbox.
//
//   Kathleen Bonnell, Tue Dec 28 16:23:43 PST 2004
//   Added 'displayPickLetter' checkbox.
//
//   Kathleen Bonnell, Mon Oct 31 10:39:28 PST 2005
//   Hide all tabs whose index > MIN_PICK_TABS.
//   Added 'userMaxPickTabs' spinbox.
//
//   Ellen Tarwater, Fri May 18, 2007
//   Added "Save picks as" button
//
//   Hank Childs, Thu Aug 30 14:13:43 PDT 2007
//   Added spreadsheetCheckBox.
//
//   Cyrus Harrison, Mon Sep 17 15:18:50 PDT 2007
//   Added floatFormat
//
//   Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007
//   Added preserveCoord combo box.
//
//   Kathleen Bonnell, Thu Nov 29 15:32:32 PST 2007
//   Added clearPicks push button.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 14:21:32 PDT 2008
//   Qt 4.
//
//   Gunther H. Weber, Fri Aug 15 10:50:10 PDT 2008
//   Add buttons for redoing pick with and without opening a Spreadsheet plot.
//   Added a missing tr() for "Clear picks".
//
//   Kathleen Bonnell, Thu Mar  3 08:07:31 PST 2011
//   Added timeCurveType combo box.
//
//   Kathleen Biagas, Thu Aug 25 09:58:00 PDT 2011
//   Create separate tabs for display, time, and spreadsheet options.
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added PickRecords array
//
//   Dirk Schubert (Allinea Software), Fri Oct 12, 2012
//   Add "Focus DDT on Pick" button optionally (HAVE_DDT)
//
//   Kathleen Biagas, Fri Mar 20 16:07:53 PDT 2015
//   Added button for resetting pick letter.
//
//   Alister Maguire, Wed Sep 27 10:11:04 PDT 2017
//   Added a button for changing the pick highlights color.
//
//   Kathleen Biagas, Fri Nov  8 08:47:59 PST 2019
//   Moved pick results and some settings to the right side of the window.
//
// ****************************************************************************

void
QvisPickWindow::CreateWindowContents()
{
    QHBoxLayout *sideBySideLayout = new QHBoxLayout();
    topLayout->addLayout(sideBySideLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    resultsTabWidget = new QTabWidget(central);
    resultsTabWidget->setMinimumHeight(200);
    rightLayout->addWidget(resultsTabWidget, 10);

    for (int i = 0; i < MAX_PICK_TABS; i++)
    {
        pages[i] = new QWidget();
        QVBoxLayout *vLayout = new QVBoxLayout(pages[i]);
        vLayout->setMargin(10);
        vLayout->setSpacing(5);
        pages[i]->hide();
        infoLists[i]  = new QTextEdit(pages[i]);
        pickRecords[i] = new PickRecord();
        vLayout->addWidget(infoLists[i]);
        infoLists[i]->setWordWrapMode(QTextOption::WordWrap);
        infoLists[i]->setReadOnly(true);
        if (i < MIN_PICK_TABS)
        {
            pages[i]->show();
            resultsTabWidget->addTab(pages[i]," ");
        }
    }

    QGridLayout *gLayout = new QGridLayout(0);
    rightLayout->addLayout(gLayout);

    userMaxPickTabs = new QSpinBox();
    userMaxPickTabs->setMinimum(MIN_PICK_TABS);
    userMaxPickTabs->setMaximum(MAX_PICK_TABS);
    userMaxPickTabs->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    gLayout->addWidget(userMaxPickTabs, 0, 1);
    QLabel *userMaxPickTabsLabel = new QLabel(tr("Max tabs"));
    userMaxPickTabsLabel->setBuddy(userMaxPickTabs);
    gLayout->addWidget(userMaxPickTabsLabel, 0,0);

    QPushButton *savePicksButton = new QPushButton(tr("Save picks as") + QString("..."));
    connect(savePicksButton, SIGNAL(clicked()),
            this, SLOT(savePickText()));
    gLayout->addWidget(savePicksButton, 0, 2, 1, 2);

    varsButton = new QvisVariableButton(true, false, true, -1, central);
    varsButton->setText(tr("Variables"));
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addPickVariable(const QString &)));
    gLayout->addWidget(varsButton, 1, 0);

    varsLineEdit = new QLineEdit();
    varsLineEdit->setText("default");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    gLayout->addWidget(varsLineEdit, 1, 1, 1, 3);

    QLabel *floatFormatLabel = new QLabel(tr("Float format"));
    gLayout->addWidget(floatFormatLabel, 2, 0);

    floatFormatLineEdit= new QLineEdit();
    floatFormatLineEdit->setText("%g");
    gLayout->addWidget(floatFormatLineEdit, 2, 1, 1, 3);
    connect(floatFormatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(floatFormatProcessText()));

    autoShowCheckBox = new QCheckBox(tr("Automatically show window"));
    connect(autoShowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(autoShowToggled(bool)));
    gLayout->addWidget(autoShowCheckBox, 3, 0, 1, 4);

#ifdef HAVE_DDT
    QPushButton *focusPickInDDTButton =
        new QPushButton(tr("Add Pick to DDT"));
    connect(focusPickInDDTButton, SIGNAL(clicked()),
            this,SLOT(focusPickInDDTClicked()));
    gLayout->addWidget(focusPickInDDTButton,3,2,1,2);
#endif

    savePicksCheckBox = new QCheckBox(tr("Don't clear this window"));
    connect(savePicksCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(savePicksToggled(bool)));
    gLayout->addWidget(savePicksCheckBox, 4, 0, 1, 2);

    QPushButton *clearPicksButton = new QPushButton(tr("Clear picks"));
    connect(clearPicksButton, SIGNAL(clicked()),
            this, SLOT(clearPicks()));
    gLayout->addWidget(clearPicksButton, 4, 2, 1, 2);

    QPushButton *resetLetterButton = new QPushButton(tr("Reset pick letter"));
    connect(resetLetterButton, SIGNAL(clicked()),
            this, SLOT(resetPickLetter()));
    gLayout->addWidget(resetLetterButton, 5, 2, 1, 2);

    setHighlightColorButton = new QPushButton(tr("Set highlight color"));
    connect(setHighlightColorButton, SIGNAL(clicked()),
            this, SLOT(setHighlightColor()));
    gLayout->addWidget(setHighlightColorButton, 5, 0, 1, 2);
    setHighlightColorButton->setEnabled(false);
    QVBoxLayout *leftLayout = new QVBoxLayout();

    optionsTabWidget = new QTabWidget();
    connect(optionsTabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(optionsTabSelected(int)));
    leftLayout->addWidget(optionsTabWidget, 10);

    CreateDisplayOptionsTab();
    CreateTimeOptionsTab();
    CreateSpreadsheetOptionsTab();

    // Show the appropriate page based on the activeOptionsTab setting.
    optionsTabWidget->blockSignals(true);
    optionsTabWidget->setCurrentIndex(activeOptionsTab);
    optionsTabWidget->blockSignals(false);
    sideBySideLayout->addLayout(leftLayout);
    sideBySideLayout->addLayout(rightLayout);
}


// ****************************************************************************
// Method: QvisAnnotationWindow::CreateDisplayOptionsTab
//
// Purpose:
//   Creates the Display options tab.
//
// Programmer: Kathleen Biagas
// Creation:   August 23, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 16:01:18 MST 2012
//   Removed 'Options' from tab title so more tabs can be visible.
//
//   Alister Maguire, Wed Aug  8 15:33:21 PDT 2018
//   Added option to update the swivel focus.
//
// ****************************************************************************

void
QvisPickWindow::CreateDisplayOptionsTab()
{
    pageDisplay = new QWidget(central);
    optionsTabWidget->addTab(pageDisplay, tr("Output display"));

    QGridLayout *dLayout = new QGridLayout(pageDisplay);

    conciseOutputCheckBox = new QCheckBox(tr("Concise output"), pageDisplay);
    connect(conciseOutputCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(conciseOutputToggled(bool)));
    dLayout->addWidget(conciseOutputCheckBox, 0, 0);

    swivelFocusToPick = new QCheckBox(tr("Swivel focus"),
                                      pageDisplay);
    connect(swivelFocusToPick, SIGNAL(toggled(bool)),
            this, SLOT(swivelFocusToPickToggled(bool)));
    dLayout->addWidget(swivelFocusToPick, 0, 2, 1, 2);

    showMeshNameCheckBox = new QCheckBox(tr("Mesh name"), pageDisplay);
    connect(showMeshNameCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showMeshNameToggled(bool)));
    dLayout->addWidget(showMeshNameCheckBox, 1, 0, 1, 2);

    showTimestepCheckBox = new QCheckBox(tr("Timestep"), pageDisplay);
    connect(showTimestepCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showTimestepToggled(bool)));
    dLayout->addWidget(showTimestepCheckBox, 1, 2, 1, 2);

    displayIncEls = new QCheckBox(tr("Incident nodes/zones"), pageDisplay);
    connect(displayIncEls, SIGNAL(toggled(bool)),
            this, SLOT(displayIncElsToggled(bool)));
    dLayout->addWidget(displayIncEls, 2, 0, 1, 2);

    displayGlobalIds = new QCheckBox(tr("Global nodes/zones"), pageDisplay);
    connect(displayGlobalIds, SIGNAL(toggled(bool)),
            this, SLOT(displayGlobalIdsToggled(bool)));
    dLayout->addWidget(displayGlobalIds, 2, 2, 1, 2);

    displayPickLetter = new QCheckBox(tr("Reference pick letter"),
                                      pageDisplay);
    connect(displayPickLetter, SIGNAL(toggled(bool)),
            this, SLOT(displayPickLetterToggled(bool)));
    dLayout->addWidget(displayPickLetter, 4, 0, 1, 4);

    displayPickHighlight = new QCheckBox(tr("Pick highlights"),
                                      pageDisplay);
    connect(displayPickHighlight, SIGNAL(toggled(bool)),
            this, SLOT(displayPickHighlightToggled(bool)));
    dLayout->addWidget(displayPickHighlight, 4, 2, 1, 2);

    // Node settings
    QGroupBox *nodeGroupBox = new QGroupBox(pageDisplay);
    nodeGroupBox->setTitle(tr("For nodes"));
    dLayout->addWidget(nodeGroupBox, 5, 0, 1, 4);
    QGridLayout *nLayout = new QGridLayout(nodeGroupBox);
    nLayout->setMargin(10);
    nLayout->setSpacing(10);

    nodeId = new QCheckBox(tr("Id"), nodeGroupBox);
    connect(nodeId, SIGNAL(toggled(bool)),
            this, SLOT(nodeIdToggled(bool)));
    nLayout->addWidget(nodeId, 0, 0);
    nodePhysical = new QCheckBox(tr("Physical coords"), nodeGroupBox);
    connect(nodePhysical, SIGNAL(toggled(bool)),
            this, SLOT(nodePhysicalToggled(bool)));
    nLayout->addWidget(nodePhysical, 1, 0);
    nodeDomLog = new QCheckBox(tr("Domain-logical coords"), nodeGroupBox);
    connect(nodeDomLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeDomLogToggled(bool)));
    nLayout->addWidget(nodeDomLog, 0, 1);
    nodeBlockLog = new QCheckBox(tr("Block-logical coords"), nodeGroupBox);
    connect(nodeBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeBlockLogToggled(bool)));
    nLayout->addWidget(nodeBlockLog, 1, 1);

    // Zone settings
    QGroupBox *zoneGroupBox = new QGroupBox(pageDisplay);
    zoneGroupBox->setTitle(tr("For zones"));
    dLayout->addWidget(zoneGroupBox, 6, 0, 1, 4);
    QGridLayout *zLayout = new QGridLayout(zoneGroupBox);
    zLayout->setMargin(10);
    zLayout->setSpacing(10);

    zoneId = new QCheckBox(tr("Id"), zoneGroupBox);
    connect(zoneId, SIGNAL(toggled(bool)),
            this, SLOT(zoneIdToggled(bool)));
    zLayout->addWidget(zoneId, 0, 0);
    zoneDomLog = new QCheckBox(tr("Domain-logical coords"), zoneGroupBox);
    connect(zoneDomLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneDomLogToggled(bool)));
    zLayout->addWidget(zoneDomLog, 0, 1);
    zoneBlockLog = new QCheckBox(tr("Block-logical coords"), zoneGroupBox);
    connect(zoneBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneBlockLogToggled(bool)));
    zLayout->addWidget(zoneBlockLog, 1, 1);
}


// ****************************************************************************
// Method: QvisAnnotationWindow::CreateTimeOptionsTab
//
// Purpose:
//   Creates the Time options tab.
//
// Programmer: Kathleen Biagas
// Creation:   August 23, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 16:01:18 MST 2012
//   Removed 'Options' from tab title so more tabs can be visible.
//
//   Alister Maguire, Wed Oct 16 08:41:57 MST 2019
//   Added extra argumentto QvisTimeQueryOptionsWidget creation that
//   disables creation of the data origin buttons.
//
// ****************************************************************************

void
QvisPickWindow::CreateTimeOptionsTab()
{
    pageTime = new QWidget(central);
    timeOptsTabIndex = optionsTabWidget->addTab(pageTime, tr("Time pick"));

    QVBoxLayout *tLayout = new QVBoxLayout(pageTime);

    timeOpts = new QvisTimeQueryOptionsWidget(
        tr("Do time curve with next pick"), pageTime, false);
    connect(timeOpts, SIGNAL(toggled(bool)),
            this, SLOT(timeCurveToggled(bool)));
    tLayout->addWidget(timeOpts);

    preserveCoord= new QComboBox(pageTime);
    preserveCoord->addItem(tr("Time curve use picked element"));
    preserveCoord->addItem(tr("Time curve use picked coordinates"));
    preserveCoord->setCurrentIndex(0);
    connect(preserveCoord, SIGNAL(activated(int)),
            this, SLOT(preserveCoordActivated(int)));
    tLayout->addWidget(preserveCoord);

    timeCurveType= new QComboBox(pageTime);
    timeCurveType->addItem(tr("Time curve use single Y axis"));
    timeCurveType->addItem(tr("Time curve use multiple Y axes"));
    timeCurveType->setCurrentIndex(0);
    connect(timeCurveType, SIGNAL(activated(int)),
            this, SLOT(timeCurveTypeActivated(int)));
    tLayout->addWidget(timeCurveType);

    redoPickButton = new QPushButton(tr("Repeat pick"), pageTime);
    connect(redoPickButton, SIGNAL(clicked()),
            this, SLOT(redoPickClicked()));
    tLayout->addWidget(redoPickButton);
}

// ****************************************************************************
// Method: QvisAnnotationWindow::CreateSpreadsheetOptionsTab
//
// Purpose:
//   Creates the Spreadsheet options tab.
//
// Programmer: Kathleen Biagas
// Creation:   August 23, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 16:01:18 MST 2012
//   Removed 'Options' from tab title so more tabs can be visible.
//
// ****************************************************************************

void
QvisPickWindow::CreateSpreadsheetOptionsTab()
{
    pageSpreadsheet = new QWidget(central);
    optionsTabWidget->addTab(pageSpreadsheet, tr("Spreadsheet"));

    QVBoxLayout *sLayout = new QVBoxLayout(pageSpreadsheet);

    spreadsheetCheckBox = new QCheckBox(tr("Create spreadsheet with next pick"),
                                        pageSpreadsheet);
    connect(spreadsheetCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(spreadsheetToggled(bool)));
    sLayout->addWidget(spreadsheetCheckBox);

    QPushButton *redoPickWithSpreadsheetButton =
        new QPushButton(tr("Display in spreadsheet"), pageSpreadsheet);
    connect(redoPickWithSpreadsheetButton, SIGNAL(clicked()),
            this, SLOT(redoPickWithSpreadsheetClicked()));
    sLayout->addWidget(redoPickWithSpreadsheetButton);
}


// ****************************************************************************
// Method: QvisPickWindow::UpdateWindow
//
// Purpose:
//   This method is called when the window needs to be updated.
//
// Programmer: Kathleen Biagas
// Creation:   August 26, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::UpdateWindow(bool doAll)
{
    if (SelectedSubject() == pickAtts || doAll)
        UpdateAll(doAll);
    if (SelectedSubject() == plotList || doAll)
        UpdateTimeOptions();
}


// ****************************************************************************
// Method: QvisPickWindow::UpdateAll
//
// Purpose:
//   This method updates the window's widgets to reflect changes made
//   in the PickAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the PickAttribute object's selected
//           states.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002
//   Updated x for IsSelected(x) to match new ordering due to clearWindow flag.
//
//   Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//   Updated x for IsSelected(x) to match new ordering due to cellPoint
//   internal member.
//
//   Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002
//   Update new pickAtts member useNodeCoords, logicalCoords.
//
//   Brad Whitlock, Wed Aug 27 08:37:57 PDT 2003
//   Made the window show itself if autoShow mode is on.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Don't clear the window if savePicks is checked.
//
//   Kathleen Bonnell, Tue Nov 18 14:03:22 PST 2003
//   Update logicalZone.
//
//   Kathleen Bonnell, Wed Dec 17 15:19:46 PST 2003
//   More options, reordered old options.
//
//   Kathleen Bonnell, Thu Apr  1 18:42:52 PST 2004
//   Added TimeCurve checkbox.
//
//   Kathleen Bonnell, Wed Jun  9 09:41:15 PDT 2004
//   Added conciseOutput, showMeshName and showTimestep checkboxes.
//
//   Kathleen Bonnell, Wed Dec 15 08:20:11 PST 2004
//   Added 'displayGlobalIds'.
//
//   Kathleen Bonnell, Tue Dec 28 16:23:43 PST 2004
//   Added 'displayPickLetter'.
//
//   Hank Childs, Thu Aug 30 14:16:57 PDT 2007
//   Added CreateSpreadsheet.
//
//   Cyrus Harrison,  Mon Sep 17 15:15:47 PDT 2007
//   Added floatFormat
//
//   Kathleen Bonnell, Fri Nov  9 14:00:27 PST 2007
//   Added timePreserveCoord and fixed some select numbers to match atts.
//
//   Brad Whitlock, Mon Dec 17 09:33:48 PST 2007
//   Made it use ids.
//
//   Kathleen Bonnell, Thu Mar  3 08:08:03 PST 2011
//   Added timeCurveType.
//
//   Kathleen Biagas, Fri Aug 26 11:13:18 PDT 2011
//   timeCurve options handle by timeQueryOptionsWidget.
//
//   Kathleen Biagas, Wed Jan 25 16:03:59 MST 2012
//   Set enabled state of redoPickButton when 'doTimeCurve' is selected.
//
//   Alister Maguire, Thu Sep 28 15:06:20 PDT 2017
//   If 'show pick highlights' is enabled, show the color
//   setting button. Otherwise, hide it.
//
// ****************************************************************************

void
QvisPickWindow::UpdateAll(bool doAll)
{
    if (pickAtts == 0)
        return;

    // Update the autoShow toggle.
    autoShowCheckBox->blockSignals(true);
    autoShowCheckBox->setChecked(autoShow);
    autoShowCheckBox->blockSignals(false);

    savePicksCheckBox->blockSignals(true);
    savePicksCheckBox->setChecked(savePicks);
    savePicksCheckBox->blockSignals(false);

    //
    //  If pick letter changes, it indicates we need to update the
    //  information in a tab-page.  If pick letter changes, we assume
    //  all other vital-to-be-displayed information has also changed.
    //

    // 10 == pickLetter 9 == clearWindow
    if (pickAtts->IsSelected(PickAttributes::ID_pickLetter) ||
        pickAtts->IsSelected(PickAttributes::ID_clearWindow) || doAll)
    {
        bool clearWindow = (savePicks ? false :
                            pickAtts->GetClearWindow());
        // If autoShow mode is on, make sure that the window is showing.
        if(!doAll && autoShow && !isPosted &&
           !clearWindow && pickAtts->GetFulfilled())
        {
            show();
        }

        UpdatePage();
    }

    //  If variables changes,
    //
    if (pickAtts->IsSelected(PickAttributes::ID_variables) || doAll)
    {
        stringVector userVars = pickAtts->GetVariables();
        std::string allVars2;
        for (size_t i = 0; i < userVars.size(); i++)
        {
           allVars2 += userVars[i];
           allVars2 += " ";
        }
        varsLineEdit->setText(allVars2.c_str());
    }

    // displayIncidentElements
    if (pickAtts->IsSelected(PickAttributes::ID_showIncidentElements) || doAll)
    {
        displayIncEls->blockSignals(true);
        displayIncEls->setChecked(pickAtts->GetShowIncidentElements());
        displayIncEls->blockSignals(false);

   }
    // showNodeId
    if (pickAtts->IsSelected(PickAttributes::ID_showNodeId) || doAll)
    {
        nodeId->blockSignals(true);
        nodeId->setChecked(pickAtts->GetShowNodeId());
        nodeId->blockSignals(false);
    }

    // showNodeDomainLogicalCoords
    if (pickAtts->IsSelected(PickAttributes::ID_showNodeDomainLogicalCoords) || doAll)
    {
        nodeDomLog->blockSignals(true);
        nodeDomLog->setChecked(pickAtts->GetShowNodeDomainLogicalCoords());
        nodeDomLog->blockSignals(false);
    }

    // showNodeBlockLogicalCoords
    if (pickAtts->IsSelected(PickAttributes::ID_showNodeBlockLogicalCoords) || doAll)
    {
        nodeBlockLog->blockSignals(true);
        nodeBlockLog->setChecked(pickAtts->GetShowNodeBlockLogicalCoords());
        nodeBlockLog->blockSignals(false);
    }

    // showNodePhysical coords
    if (pickAtts->IsSelected(PickAttributes::ID_showNodePhysicalCoords) || doAll)
    {
        nodePhysical->blockSignals(true);
        nodePhysical->setChecked(pickAtts->GetShowNodePhysicalCoords());
        nodePhysical->blockSignals(false);
    }

    // showZoneId
    if (pickAtts->IsSelected(PickAttributes::ID_showZoneId) || doAll)
    {
        zoneId->blockSignals(true);
        zoneId->setChecked(pickAtts->GetShowZoneId());
        zoneId->blockSignals(false);
    }

    // showZoneDomainLogicalCoords
    if (pickAtts->IsSelected(PickAttributes::ID_showZoneDomainLogicalCoords) || doAll)
    {
        zoneDomLog->blockSignals(true);
        zoneDomLog->setChecked(pickAtts->GetShowZoneDomainLogicalCoords());
        zoneDomLog->blockSignals(false);
    }

    // showZoneBlockLogicalCoords
    if (pickAtts->IsSelected(PickAttributes::ID_showZoneBlockLogicalCoords) || doAll)
    {
        zoneBlockLog->blockSignals(true);
        zoneBlockLog->setChecked(pickAtts->GetShowZoneBlockLogicalCoords());
        zoneBlockLog->blockSignals(false);
    }

    // doTimeCurve
    if (pickAtts->IsSelected(PickAttributes::ID_doTimeCurve) || doAll)
    {
        timeOpts->blockSignals(true);
        timeOpts->setChecked(pickAtts->GetDoTimeCurve());
        preserveCoord->setEnabled(pickAtts->GetDoTimeCurve());
        timeCurveType->setEnabled(pickAtts->GetDoTimeCurve());
        redoPickButton->setEnabled(pickAtts->GetDoTimeCurve());
        timeOpts->blockSignals(false);
    }

    // conciseOutput
    if (pickAtts->IsSelected(PickAttributes::ID_conciseOutput) || doAll)
    {
        conciseOutputCheckBox->blockSignals(true);
        conciseOutputCheckBox->setChecked(pickAtts->GetConciseOutput());
        conciseOutputCheckBox->blockSignals(false);
    }

    // showTimeStep
    if (pickAtts->IsSelected(PickAttributes::ID_showTimeStep) || doAll)
    {
        showTimestepCheckBox->blockSignals(true);
        showTimestepCheckBox->setChecked(pickAtts->GetShowTimeStep());
        showTimestepCheckBox->blockSignals(false);
    }

    // showMeshName
    if (pickAtts->IsSelected(PickAttributes::ID_showMeshName) || doAll)
    {
        showMeshNameCheckBox->blockSignals(true);
        showMeshNameCheckBox->setChecked(pickAtts->GetShowMeshName());
        showMeshNameCheckBox->blockSignals(false);
    }

    // displayGlobalIds
    if (pickAtts->IsSelected(PickAttributes::ID_showGlobalIds) || doAll)
    {
        displayGlobalIds->blockSignals(true);
        displayGlobalIds->setChecked(pickAtts->GetShowGlobalIds());
        displayGlobalIds->blockSignals(false);
    }

    // displayPickLetter
    if (pickAtts->IsSelected(PickAttributes::ID_showPickLetter) || doAll)
    {
        displayPickLetter->blockSignals(true);
        displayPickLetter->setChecked(pickAtts->GetShowPickLetter());
        displayPickLetter->blockSignals(false);
    }

    // displayPickHighlight
    if (pickAtts->IsSelected(PickAttributes::ID_showPickHighlight) || doAll)
    {
        displayPickHighlight->blockSignals(true);
        bool showHighlight = pickAtts->GetShowPickHighlight();
        if (showHighlight)
            setHighlightColorButton->setEnabled(true);
        else
            setHighlightColorButton->setEnabled(false);
        displayPickHighlight->setChecked(showHighlight);
        displayPickHighlight->blockSignals(false);
    }

    // createSpreadsheet
    if (pickAtts->IsSelected(PickAttributes::ID_createSpreadsheet) || doAll)
    {
        spreadsheetCheckBox->blockSignals(true);
        spreadsheetCheckBox->setChecked(pickAtts->GetCreateSpreadsheet());
        spreadsheetCheckBox->blockSignals(false);
    }

    // floatFormat
    if (pickAtts->IsSelected(PickAttributes::ID_floatFormat) || doAll)
    {
        floatFormatLineEdit->blockSignals(true);
        floatFormatLineEdit->setText(pickAtts->GetFloatFormat().c_str());
        floatFormatLineEdit->blockSignals(false);
    }

    // timePreserveCoord
    if (pickAtts->IsSelected(PickAttributes::ID_timePreserveCoord) || doAll)
    {
        preserveCoord->blockSignals(true);
        preserveCoord->setCurrentIndex((int)pickAtts->GetTimePreserveCoord());
        preserveCoord->blockSignals(false);
    }

    // timeCurveType
    if (pickAtts->IsSelected(PickAttributes::ID_timePreserveCoord) || doAll)
    {
        timeCurveType->blockSignals(true);
        timeCurveType->setCurrentIndex((int)pickAtts->GetTimeCurveType());
        timeCurveType->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPickWindow::UpdateTimeQueryOptions
//
// Purpose:
//   Sets the enabled state for the time options widget.
//
// Programmer: Kathleen Biagas
// Creation:   August 26, 2011
//
// Modifications:
//    Kathleen Biagas, Wed Jan 25 15:51:52 MST 2012
//    Set enabled state of redoPickButton.
//
//    Kathleen Biagas, Wed Apr 11 19:16:26 PDT 2012
//    Call UpdateState on timeOpts instead of setEnabled.
//
// ****************************************************************************

void
QvisPickWindow::UpdateTimeOptions()
{
    timeOpts->UpdateState(plotList);
    preserveCoord->setEnabled(timeOpts->isEnabled() && timeOpts->isChecked());
    timeCurveType->setEnabled(timeOpts->isEnabled() && timeOpts->isChecked());
    redoPickButton->setEnabled(timeOpts->isEnabled() && timeOpts->isChecked());
}


// ****************************************************************************
// Method: QvisPickWindow::UpdatePage
//
// Purpose:
//   This method updates the nextPage on the tab widget to reflect changes made
//   in the PickAttributes object that the window watches.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 17:26:05 PST 2002
//   Changed the code so it uses more references instead of copies.
//
//   Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002
//   Added call to ClearPages if flag is set.
//
//   Kathleen Bonnell, Thu Jun 27 14:37:29 PDT 2002
//   Changed the way that vars are inserted in the page. Set column mode to 1.
//
//   Kathleen Bonnell, Tue Jul 30 10:18:05 PDT 2002
//   Don't display domain number for single-domain problems, or z-coord for
//   2d.
//
//   Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002
//   Count number of items added, and use it for setRowMode. Added nodeCoords.
//
//   Kathleen Bonnell, Thu Apr 17 15:34:45 PDT 2003
//   Use pickAtts.CellPoint (intead of pickAtts.PickPoint) for pick
//   intersection point.  Specify when the intersection point is in
//   transformed space.
//
//   Kathleen Bonnell, Wed Jun 25 13:45:04 PDT 2003
//   Reflect new naming convention in PickAttributes. Rework to support
//   nodePick.
//
//   Brad Whitlock, Tue Sep 9 09:07:17 PDT 2003
//   I made the infoLists be QTextEdits instead of QListWidgetes.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Don't clear window if savePicks is checked.
//   Removed code that printed all the pickAtts info separately.  Code was here
//   because "\n" did not play well with QListWidget.  Any time PickAttributes
//   changed, this code had to be updated as well.  Now simply use
//   pickAtts->CreateOutputString.
//
//   Kathleen Bonnell, Mon Oct 31 10:44:07 PST 2005
//   Use value in userMaxPickTabs for setting nextPage instead of
//   MAX_PICK_TABS.
//
//   Jonathan Byrd Mon Feb 4, 2013
//   Record data concerning the latest pick in the pickRecord array
//
//   Kathleen Biagas, Tue Apr 25 17:09:57 PDT 2017
//   Added call to ResizeTabs, in case user changed #tabs without clicking
//   'Apply' before perfoming picks.  It's a no-op if #tabs hasn't changed.
//
// ****************************************************************************

void
QvisPickWindow::UpdatePage()
{
    ResizeTabs(); // no-op if numtabs hasn't changed

    QString pickLetter(pickAtts->GetPickLetter().c_str());

    if (!savePicks && pickAtts->GetClearWindow())
    {
        nextPage = 0;
        ClearPages();
    }
    else if (lastLetter != pickLetter && pickAtts->GetFulfilled())
    {
        QString temp;
        std::string displayString;

        // Change the tab heading.
        lastLetter = pickLetter;
        temp.sprintf(" %s ", pickAtts->GetPickLetter().c_str());
        resultsTabWidget->setTabText(nextPage, temp);

        //
        // Get the output string without the letter, as it is
        // displayed in the tab.
        //
        pickAtts->CreateOutputString(displayString, false);

        // Make the tab display the string.
        infoLists[nextPage]->clear();
        infoLists[nextPage]->setText(displayString.c_str());

        // Record the data concerning this pick
        pickRecords[nextPage]->domain  = pickAtts->GetDomain();
        pickRecords[nextPage]->element = pickAtts->GetElementNumber();
        pickRecords[nextPage]->variable = pickAtts->GetActiveVariable();

        // Record the value of the active variable of this pick
        char buff[256];
        buff[0] = '\0';
        for (int i=0; i<pickAtts->GetNumVarInfos(); ++i)
        {
            const PickVarInfo  &info = pickAtts->GetVarInfo(i);
            const doubleVector &values = info.GetValues();

            // For the active variable only
            if (info.GetVariableName() == pickRecords[nextPage]->variable)
            {
                if (info.GetVariableType() == "scalar" && values.size()==1)
                {
                    snprintf(buff, 256, info.GetFloatFormat().c_str(), values[0]);
                }
            }
        }
        std::string targetValue(buff);
        pickRecords[nextPage]->value = buff;

        // Show the tab.
        resultsTabWidget->setCurrentIndex(nextPage);
        nextPage = (nextPage + 1) % userMaxPickTabs->value();
    }
}

// ****************************************************************************
// Method: QvisPickWindow::GetCurrentValues
//
// Purpose:
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Kathleen Bonnell
// Creation:   Mon Sep 25 15:11:42 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Tue Mar 26 14:03:24 PST 2002
//   Improved parsing of the varsLineEdit text.
//
//   Kathleen Bonnell, Mon Oct 31 10:44:07 PST 2005
//   Added call to ResizeTabs.
//
//   Cyrus Harrison, Thu Sep 13 12:21:05 PDT 2007
//   Added logic for floaing point format string
//
//   Cyrus Harrison, Wed Sep 26 09:50:46 PDT 2007
//   Added validation of the floating point format string.
//
//   Brad Whitlock, Mon Dec 17 09:34:38 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 14:55:57 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Fri Aug 26 11:13:54 PDT 2011
//   Added timeQueryOptionsWidget.
//
// ****************************************************************************

void
QvisPickWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    //
    //  This 'if' is not really necessary right now, since the
    //  varsLineEdit is the only widget whose values can change, but
    //  leave the logic in place for possible future updates.
    //

    //
    // Do the user-selectedVars.
    //
    if (which_widget == PickAttributes::ID_variables || doAll)
    {
        QString temp;
        stringVector userVars;
        temp = varsLineEdit->displayText().simplified();
        QStringList lst(temp.split(" "));

        QStringList::Iterator it;

        for (it = lst.begin(); it != lst.end(); ++it)
        {
            userVars.push_back((*it).toStdString());
        }

        pickAtts->SetVariables(userVars);
    }
    if (which_widget == PickAttributes::ID_floatFormat || doAll)
    {
        string format = floatFormatLineEdit
                               ->displayText().simplified().toStdString();
        if(!StringHelpers::ValidatePrintfFormatString(format.c_str(),
                                                      "float","EOA"))
            Error(tr("Invalid pick floating point format string."));
        else
            pickAtts->SetFloatFormat(format);
    }

    if (doAll)
    {
        MapNode timeOptions;
        timeOpts->GetTimeQueryOptions(timeOptions);
        pickAtts->SetTimeOptions(timeOptions);
        ResizeTabs();
    }
}

// ****************************************************************************
// Method: QvisPickWindow::CreateNode
//
// Purpose:
//   This method saves the window's information to a DataNode tree.
//
// Arguments:
//   parentNode : The node to which the information is added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 08:51:48 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Added a node for savePicks.
//
//   Kathleen Bonnell, Mon Oct 31 10:47:53 PST 2005
//   Added a node for userMaxTabs.
//
//   Brad Whitlock, Fri Jun  6 14:56:43 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Fri Aug 26 11:14:11 PDT 2011
//   Added activeOptionsTab.
//
// ****************************************************************************

void
QvisPickWindow::CreateNode(DataNode *parentNode)
{
    // Add the base information.
    QvisPostableWindowObserver::CreateNode(parentNode);

    // Add more information.
    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(windowTitle().toStdString());
        if(node)
        {
            node->AddNode(new DataNode("autoShow", autoShow));
            node->AddNode(new DataNode("savePicks", savePicks));
            node->AddNode(new DataNode("userMaxTabs",userMaxPickTabs->value()));
            node->AddNode(new DataNode("activeOptionsTab",activeOptionsTab));
        }
    }
}

// ****************************************************************************
// Method: QvisPickWindow::SetFromNode
//
// Purpose:
//   Reads window attributes from the DataNode representation of the config
//   file.
//
// Arguments:
//   parentNode : The data node that contains the window's attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 08:54:55 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Added a node for savePicks.
//
//   Kathleen Bonnell, Mon Oct 31 10:47:53 PST 2005
//   Added a node for userMaxTabs.
//
//   Katheen Bonnell, Thu Nov 29 15:35:15 PST 2007
//   Added defaultAutoShow, defaultSavePicks, defaultNumTabs so that their
//   counterparts can be 'reset' correclty from user-saved values when needed.
//
//   Brad Whitlock, Fri Jun  6 14:56:56 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Fri Aug 26 11:14:30 PDT 2011
//   Added activeOptionsTab.
//
// ****************************************************************************

void
QvisPickWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    QvisPostableWindowObserver::SetFromNode(parentNode, borders);

    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
    {
        return;
    }

    // Set the autoShow flag.
    DataNode *node;
    if((node = winNode->GetNode("autoShow")) != 0)
        defaultAutoShow = autoShow = node->AsBool();
    if((node = winNode->GetNode("savePicks")) != 0)
        defaultSavePicks = savePicks = node->AsBool();
    if((node = winNode->GetNode("userMaxTabs")) != 0)
    {
        defaultNumTabs = node->AsInt();
        userMaxPickTabs->setValue(node->AsInt());
        ResizeTabs();
    }
    if((node = winNode->GetNode("activeOptionsTab")) != 0)
    {
        activeOptionsTab = node->AsInt();
        if (activeOptionsTab < 0 || activeOptionsTab > 2)
            activeOptionsTab = 0;
        // Show the appropriate page based on the activeOptionsTab setting.
        optionsTabWidget->blockSignals(true);
        optionsTabWidget->setCurrentIndex(activeOptionsTab);
        optionsTabWidget->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPickWindow::Apply
//
// Purpose:
//   This method applies the pick attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            pick attributes.
//
// Programmer: Kathleen Bonnell
// Creation:   Mon Sep 25 15:22:16 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Tue Jul  1 09:21:57 PDT 2003
//   Added call to viewer->SetPickAttributes.
//
//   Kathleen Bonnell, Tue Dec 28 16:17:23 PST 2004
//   Set pickAtts fulfilled flag to  false so that obervers won't think
//   that a pick has been performed and the results should be displayed.
//
// ****************************************************************************

void
QvisPickWindow::Apply(bool ignore)
{
    pickAtts->SetFulfilled(false);
    if(AutoUpdate() || ignore)
    {
        // Get the current pick attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        pickAtts->Notify();
        GetViewerMethods()->SetPickAttributes();
    }
    else
    {
        pickAtts->Notify();
        GetViewerMethods()->SetPickAttributes();
    }
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisPickWindow::apply
//
// Purpose:
//   This is a Qt slot function to apply the pick attributes.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// ****************************************************************************

void
QvisPickWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisPickWindow::variableProcessText
//
// Purpose:
//   Qt slot function that propagates changes of the variable list to the
//   pick attributes.
//
// Programmer: Kathleen Bonnell
// Creation:   ?
//
// ****************************************************************************
void
QvisPickWindow::variableProcessText()
{
    GetCurrentValues(PickAttributes::ID_variables);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::floatFormatProcessText
//
// Purpose:
//   Qt slot function that propagates changes of the floating point format
//   string to the pick attributes.
//
// Programmer: Cyrus Harrison
// Creation:   September 13, 2007
//
// ****************************************************************************
void
QvisPickWindow::floatFormatProcessText()
{
    GetCurrentValues(PickAttributes::ID_floatFormat);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::ClearPages
//
// Purpose:
//   This method clears all the pages on the tab widget to reflect a
//   clean-slate
//
// Programmer: Kathleen Bonnell
// Creation:   March 25, 2002
//
// Modifications:
//   Brad Whitlock, Fri Jun  6 14:48:04 PDT 2008
//   Qt 4.
//
// ****************************************************************************
void
QvisPickWindow::ClearPages()
{
    QString temp = " ";
    for (int i = 0; i < resultsTabWidget->count(); i++)
    {
        resultsTabWidget->setTabText(i, temp);
        infoLists[i]->clear();
        pickRecords[i]->reset();
    }
    resultsTabWidget->setCurrentIndex(0);
}


// ****************************************************************************
// Method: QvisPickWindow::autoShowToggled
//
// Purpose:
//   This is a Qt slot function that sets the internal autoShow flag when the
//   autoShow checkbox is toggled.
//
// Arguments:
//   val : The new autoShow value.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 08:46:20 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::autoShowToggled(bool val)
{
    autoShow = val;
}


// ****************************************************************************
// Method: QvisPickWindow::savePicksToggled
//
// Purpose:
//   This is a Qt slot function that sets the internal savePicks flag when
//   the savePicks checkbox is toggled.
//
// Arguments:
//   val : The new savePicks value.
//
// Programmer: Kathleen Bonnell
// Creation:   September 9, 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::savePicksToggled(bool val)
{
    savePicks = val;
}


// ****************************************************************************
// Method: QvisPickWindow::makeDefault
//
// Purpose:
//   This is a Qt slot function to make the current pick attributes
//   the defaults.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// ****************************************************************************

void
QvisPickWindow::makeDefault()
{
    // Tell the viewer to set the default pc attributes.
    GetCurrentValues(-1);
    pickAtts->Notify();
    GetViewerMethods()->SetDefaultPickAttributes();
}


// ****************************************************************************
// Method: QvisPickWindow::reset
//
// Purpose:
//   This is a Qt slot function to reset the PickAttributes to the
//   last applied values.
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2001
//
// Modifications:
//   Katheen Bonnell, Thu Nov 29 15:35:15 PST 2007
//   Allow 'autoShow', 'savePicks' and 'maxTabs' to be reset.
//
// ****************************************************************************

void
QvisPickWindow::reset()
{
    // Set the autoShow back to default
    autoShow = defaultAutoShow;
    autoShowCheckBox->blockSignals(true);
    autoShowCheckBox->setChecked(autoShow);
    autoShowCheckBox->blockSignals(false);

    // Set the max tabs back to default
    userMaxPickTabs->setValue(defaultNumTabs);
    ResizeTabs();

    // Set the "don't clear window" back to default
    savePicks = defaultSavePicks;
    savePicksCheckBox->blockSignals(true);
    savePicksCheckBox->setChecked(savePicks);
    savePicksCheckBox->blockSignals(false);


    // Tell the viewer to reset the pick attributes to the last
    // applied values.
    //
    GetViewerMethods()->ResetPickAttributes();
}


// ****************************************************************************
// Method: QvisPickWindow::displayIncElsToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the indicdent elements should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::displayIncElsToggled(bool val)
{
    pickAtts->SetShowIncidentElements(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::nodeIdToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the node id should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::nodeIdToggled(bool val)
{
    pickAtts->SetShowNodeId(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::nodeDomLogToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the nodes' domain logical coordinates should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::nodeDomLogToggled(bool val)
{
    pickAtts->SetShowNodeDomainLogicalCoords(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::nodeBlockLogToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the nodes' block logical coordinates should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::nodeBlockLogToggled(bool val)
{
    pickAtts->SetShowNodeBlockLogicalCoords(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::nodePhysicalToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the nodes' physical coordinates should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::nodePhysicalToggled(bool val)
{
    pickAtts->SetShowNodePhysicalCoords(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::zoneIdToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the zones' id should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::zoneIdToggled(bool val)
{
    pickAtts->SetShowZoneId(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::zoneDomLogToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the zone's domain logical coordinates should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::zoneDomLogToggled(bool val)
{
    pickAtts->SetShowZoneDomainLogicalCoords(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::zoneBlockLogToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the zones' block logical coordinates should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// ****************************************************************************

void
QvisPickWindow::zoneBlockLogToggled(bool val)
{
    pickAtts->SetShowZoneBlockLogicalCoords(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::timeCurveToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the next pick should create a time curve.
//
// Arguments:
//   val : The new timeCurve value.
//
// Programmer: Kathleen Bonnell
// Creation:   April 1, 2004
//
// Modifications:
//   Kathleen Biagas, Fri Jan 13 14:40:28 PST 2012
//   Make do time-curve with next pick and create spreadsheet with next pick
//   mutually exclusive.
//
// ****************************************************************************

void
QvisPickWindow::timeCurveToggled(bool val)
{
    pickAtts->SetDoTimeCurve(val);
    if (val)
        pickAtts->SetCreateSpreadsheet(false);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::spreadsheetToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the next pick should create a spreadsheet.
//
// Arguments:
//   val : The new spreadsheet value.
//
// Programmer: Hank Childs
// Creation:   August 30, 2007
//
// Modifications:
//   Kathleen Biagas, Fri Jan 13 14:40:28 PST 2012
//   Make do time-curve with next pick and create spreadsheet with next pick
//   mutually exclusive.
//
// ****************************************************************************

void
QvisPickWindow::spreadsheetToggled(bool val)
{
    pickAtts->SetCreateSpreadsheet(val);
    if (val)
        pickAtts->SetDoTimeCurve(false);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::conciseOutputToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the pick output should be printed in concise format.
//
// Arguments:
//   val : The new conciseOutput value.
//
// Programmer: Kathleen Bonnell
// Creation:   June 9, 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::conciseOutputToggled(bool val)
{
    pickAtts->SetConciseOutput(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::showMeshNameToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the pick output should display the mesh name.
//
// Arguments:
//   val : The new showMeshName value.
//
// Programmer: Kathleen Bonnell
// Creation:   June 9, 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::showMeshNameToggled(bool val)
{
    pickAtts->SetShowMeshName(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::showTimestepToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the pick output should display the timestep.
//
// Arguments:
//   val : The new showTimestep value.
//
// Programmer: Kathleen Bonnell
// Creation:   June 9, 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::showTimestepToggled(bool val)
{
    pickAtts->SetShowTimeStep(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::swivelFocusToPickToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the camera focus should swivel to the pick point.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Alister Maguire
// Creation:   Wed Aug  8 15:33:21 PDT 2018
//
// ****************************************************************************

void
QvisPickWindow::swivelFocusToPickToggled(bool val)
{
    pickAtts->SetSwivelFocusToPick(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::addPickVariable
//
// Purpose:
//   This is a Qt slot function that is called when the user selects a new
//   pick variable.
//
// Arguments:
//   var : The pick variable to add.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 10 09:57:14 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::addPickVariable(const QString &var)
{
    // Add the new pick variable to the pick variable line edit.
    QString pickVarString(varsLineEdit->displayText());
    if(pickVarString.length() > 0)
        pickVarString += " ";
    pickVarString += var;
    varsLineEdit->setText(pickVarString);

    // Process the list of pick vars.
    variableProcessText();
}

// ****************************************************************************
// Method: QvisPickWindow::displayGlobalIdsToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not globalIds should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 15, 2004
//
// ****************************************************************************

void
QvisPickWindow::displayGlobalIdsToggled(bool val)
{
    pickAtts->SetShowGlobalIds(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::displayPickLetterToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the pick letter should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell
// Creation:   December 15, 2004
//
// ****************************************************************************

void
QvisPickWindow::displayPickLetterToggled(bool val)
{
    pickAtts->SetShowPickLetter(val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::displayPickHighlihgtToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the pick highlight should be displayed.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Matt Larsen
// Creation:   Jul 18, 2016
//
// ****************************************************************************

void
QvisPickWindow::displayPickHighlightToggled(bool val)
{
    pickAtts->SetShowPickHighlight(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::preserveCoordActivated
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not a time-curve pick should preserved the picked coordinates.
//
// Arguments:
//   val : The new timePreserveCoord value.
//
// Programmer: Kathleen Bonnell
// Creation:   November 9, 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::preserveCoordActivated(int val)
{
    pickAtts->SetTimePreserveCoord((bool)val);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::timeCurveTypeActivated
//
// Purpose:
//   This is a Qt slot function that sets the value indicating whether
//   a time-curve pick will generate a curve with single-y axis (Curve plot)
//   or multiple y axes (MultiCurve plot).
//
// Arguments:
//   val : The new timeCurveType value.
//
// Programmer: Kathleen Bonnell
// Creation:   March 3, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::timeCurveTypeActivated(int val)
{
    pickAtts->SetTimeCurveType((PickAttributes::TimeCurveType)val);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::ResizeTabs
//
// Purpose:
//   Resizes the number of tab the resultsTabWidget hold based on the
//   current value in userMaxPickTabs.
//
// Programmer: Kathleen Bonnell
// Creation:   October 31, 2005
//
// Modifications:
//   Brad Whitlock, Fri Jun  6 14:50:19 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPickWindow::ResizeTabs()
{
    int currentMax = resultsTabWidget->count();
    int newMax     = userMaxPickTabs->value();

    if (currentMax == newMax)
        return;

    int i;
    QString temp = " ";
    if (newMax < currentMax)
    {
        // Reduce the number of pages that resultsTabWidget holds
        for (i = currentMax-1; i >= newMax; i--)
        {
            resultsTabWidget->setTabText(i, temp);
            infoLists[i]->clear();
            pickRecords[i]->reset();
            resultsTabWidget->removeTab(i);
            pages[i]->hide();
        }
        if (nextPage >= newMax)
            nextPage = 0;
    }
    else // newMax > currentMax
    {
        // Increase the number of pages that resultsTabWidget holds
        for (i = currentMax; i < newMax; i++)
        {
            pages[i]->show();
            resultsTabWidget->addTab(pages[i]," ");
        }
        if (resultsTabWidget->tabText(nextPage) != " ")
            nextPage = currentMax;
    }
}


// ****************************************************************************
// Method: QvisPickWindow::savePickText
//
// Purpose:
//   This is a Qt slot function that saves the pick text in a user
//   selected file.
//
// Programmer: Ellen Tarwater
// Creation:   Friday May 18 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 14:59:53 PDT 2008
//   Qt 4.
//
//   Kathleen Bonnell, Fri May 13 13:28:45 PDT 2011
//   On Windows, explicitly test writeability of the 'cwd' before passing it
//   to getSaveFileName (eg don't present user with a place to save a file if
//   they cannot save there!)
//
//   Kathleen Biagas, Wed Oct 29 11:59:46 PDT 2014
//   Add PickLetter (tab text) to output, add newlines between picks.
//
// ****************************************************************************

void
QvisPickWindow::savePickText()
{
    QString saveExtension(".txt");

    // Create the name of a VisIt save file to use.
    QString defaultFile;
    defaultFile.sprintf("visit%04d", saveCount);
    defaultFile += saveExtension;

    QString useDir;
    useDir = QDir::current().path();

#ifdef _WIN32
    { // new scope
        // force a temporary file creation in cwd
        QTemporaryFile tf("mytemp");
        if (!tf.open())
        {
            useDir = GetUserVisItDirectory().c_str();
        }
    }
#endif

    defaultFile = useDir + "/" + defaultFile;

    // Get the name of the file that the user saved.
    QString sFilter(QString("VisIt ") + QString("save") + QString(" (*") + saveExtension + ")");
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Choose save filename"), defaultFile, sFilter);

    // If the user chose to save a file, write the pick result text
    // to that file.
    if(!fileName.isNull())
    {
        ++saveCount;
        QFile file( fileName );
        if ( file.open(QIODevice::WriteOnly) )
        {
            QTextStream stream( &file );
            for ( int i = 0; i < resultsTabWidget->count(); ++i )
            {
                QString txt( infoLists[i]->toPlainText() );
                if ( txt.length() > 0 )
                    stream << resultsTabWidget->tabText(i).simplified()
                           << "\n" << txt << "\n";
            }

            file.close();
        }
        else
            Error(tr("VisIt could not save the pick results"
                     "to the selected file")) ;
    }
}


// ****************************************************************************
// Method: QvisPickWindow::clearPicks
//
// Purpose:
//   This is a Qt slot function that clears the picks from the active window.
//
// Programmer: Kathleen Bonnell
// Creation:   November 29, 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::clearPicks()
{
    GetViewerMethods()->ClearPickPoints();
}


// ****************************************************************************
// Method: QvisPickWindow::resetPickLetter
//
// Purpose:
//   This is a Qt slot function for resetting the Pick letter.
//
// Programmer: Kathleen Biagas
// Creation:   March 20, 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::resetPickLetter()
{
    GetViewerMethods()->ResetPickLetter();
}


// ****************************************************************************
// Method: QvisPickWindow::setHighlightColor
//
// Purpose:
//    This is a Qt slot function that opens up a color picker
//    and allows the user to change the color of the pick
//    highlights.
//
// Programmer: Alister Maguire
// Creation: Sep 27, 2017
//
// Modifications:
//
//    Alister Maguire, Thu Apr 26 10:59:22 PDT 2018
//    Removed conversion to float. Range is now 0 - 255 int.
//
//    Alister Maguire, Mon Aug 20 09:51:26 PDT 2018
//    Show the currently selected color in the color picker.
//
// ****************************************************************************

void
QvisPickWindow::setHighlightColor()
{
    int  *cRGB = pickAtts->GetPickHighlightColor();
    QColor cColor(cRGB[0], cRGB[1], cRGB[2]);

    QColor nColor = QColorDialog::getColor(cColor, this);
    int nRGB[3];
    nRGB[0] = nColor.red();
    nRGB[1] = nColor.green();
    nRGB[2] = nColor.blue();

    pickAtts->SetPickHighlightColor(nRGB);
}


// ****************************************************************************
// Method: QvisPickWindow::redoPickClicked()
//
// Purpose:
//   This is a Qt slot function that repeats the last pick.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug 15 10:52:49 PDT 2008
//
// Modifications:
//   Kathleen Biagas, Fri Jan 13 14:41:44 PST 2012
//   Turn off CreateSpreadsheet.  Display the pick letter if not doing Time.
//
// ****************************************************************************

void
QvisPickWindow::redoPickClicked()
{
    // Save old state
    createSpreadsheetSave = pickAtts->GetCreateSpreadsheet();
    displayPickLetterSave = pickAtts->GetShowPickLetter();
    reusePickLetterSave = pickAtts->GetReusePickLetter();

    // Do not display a new pick letter, do not advance pick letter,
    // do not create a spreadsheet
    pickAtts->SetShowPickLetter(!pickAtts->GetDoTimeCurve());
    pickAtts->SetReusePickLetter(true);
    pickAtts->SetCreateSpreadsheet(false);
    pickAtts->Notify();
    GetViewerMethods()->SetPickAttributes();

    // Make sure that pick results are displayed even though we are reusing the
    // pick letter.
    lastLetter = " ";

    // Tell GUI to initiate the repick once the attribute change is complete
    // The GUI will then call our redoPick() slot method.
    emit initiateRedoPick();
}

// ****************************************************************************
// Method: QvisPickWindow::redoPickWithSpreadsheetClicked()
//
// Purpose:
//   This is a Qt slot function that repeats the last pick to bring up a
//   spreadsheet.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug 15 10:52:49 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::redoPickWithSpreadsheetClicked()
{
    // Save old state
    createSpreadsheetSave = pickAtts->GetCreateSpreadsheet();
    displayPickLetterSave = pickAtts->GetShowPickLetter();
    reusePickLetterSave = pickAtts->GetReusePickLetter();

    // Do not display a new pick letter, do not advance pick letter, create
    // a spreadsheet
    pickAtts->SetCreateSpreadsheet(true);
    pickAtts->SetShowPickLetter(false);
    pickAtts->SetReusePickLetter(true);
    pickAtts->Notify();
    GetViewerMethods()->SetPickAttributes();

    // Tell GUI to initiate the repick once thre attribute change is complete
    // The GUI will then call our redoPick() slot method.
    emit initiateRedoPick();
}

// ****************************************************************************
// Method: QvisPickWindow::redoPick()
//
// Purpose:
//   This is a Qt slot function that is called by the GUI after the
//   PickAttributes change initiated by a redoPickClicked() or
//   redoPickWithSpreadsheetClicked() method changed.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug 15 10:52:49 PDT 2008
//
// Modifications:
//    Gunther H. Weber, Tue May 17 19:45:24 PDT 2011
//    Replaced generic "Pick" with "ZonePick" to get re-picking to work again.
//
//    Kathleen Biagas, Tue Jun 21 12:00:49 PDT 2011
//    ViewerMethods only has a Query method now, so fill out a MapNode
//    with Pick parameters to pass to it.
//
//    Kathleen Biagas, Fri Jan 13 14:44:07 PST 2012
//    Set up MapNode with correct params and correct param names.
//
// ****************************************************************************

void
QvisPickWindow::redoPick()
{
    MapNode params;
    params["query_name"] = string("Pick");
    params["query_type"] = (QueryList::QueryType)QueryList::PointQuery;
    params["vars"]  = pickAtts->GetVariables();

    if (pickAtts->GetDoTimeCurve() && pickAtts->GetTimePreserveCoord())
    {
        double *pp = pickAtts->GetPickPoint();
        doubleVector ppv;
        ppv.push_back(pp[0]);
        ppv.push_back(pp[1]);
        ppv.push_back(pp[2]);
        params["coord"] = ppv;
        if (pickAtts->GetPickType() == PickAttributes::Zone)
            params["pick_type"] = string("Zone");
        else if (pickAtts->GetPickType() == PickAttributes::Node)
            params["pick_type"] = string("Node");
        else if (pickAtts->GetPickType() == PickAttributes::DomainZone)
            params["pick_type"] = string("Zone");
        else if (pickAtts->GetPickType() == PickAttributes::DomainNode)
            params["pick_type"] = string("Node");
    }
    else
    {
        // use the easier route, element and domain
        params["element"] = pickAtts->GetElementNumber();
        if (pickAtts->GetDomain() == -1)
            params["domain"] = 0;
        else
            params["domain"] = pickAtts->GetDomain();
        if (pickAtts->GetPickType() == PickAttributes::Zone)
            params["pick_type"] = string("DomainZone");
        else if (pickAtts->GetPickType() == PickAttributes::Node)
            params["pick_type"] = string("DomainNode");
        else if (pickAtts->GetPickType() == PickAttributes::DomainZone)
            params["pick_type"] = string("DomainZone");
        else if (pickAtts->GetPickType() == PickAttributes::DomainNode)
            params["pick_type"] = string("DomainNode");
    }

    GetViewerMethods()->Query(params);

    // Tell GUI to restore the pick attributes once the pick is complete
    emit initiateRestorePickAttributesAfterRepick();
}

// ****************************************************************************
// Method: QvisPickWindow::restorePickAttributesAfterRepick()
//
// Purpose:
//   This is a Qt slot function that is called by the GUI to restore the
//   PickAttributes to their original values after a repeated pick operation
//   is completed.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug 15 10:52:49 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisPickWindow::restorePickAttributesAfterRepick()
{
    // Restore the pick attributes
    pickAtts->SetCreateSpreadsheet(createSpreadsheetSave);
    pickAtts->SetShowPickLetter(displayPickLetterSave);
    pickAtts->SetReusePickLetter(reusePickLetterSave);
    pickAtts->Notify();
    GetViewerMethods()->SetPickAttributes();
}

// ****************************************************************************
// Method: QvisPickWindow::optionsTabSelected
//
// Purpose:
//   This is a Qt slot function that is called when options tabs are changed.
//
// Arguments:
//   index : The new active tab.
//
// Programmer: Kathleen Biagas
// Creation:   August 26, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 16:06:29 MST 2012
//   When timeOpts tab is the one selected, update time options.
//
// ****************************************************************************

void
QvisPickWindow::optionsTabSelected(int index)
{
    activeOptionsTab = index;
    if (index == timeOptsTabIndex && isVisible())
    {
        // set enabled state
        UpdateTimeOptions();
    }
}


// ****************************************************************************
// Method: QvisPickWindow::ConnectPlotList
//
// Purpose:
//   This method connects this window to the plotList.
//
// Arguments:
//   pl        The plot list whichis being connected.
//
// Programmer: Kathleen Biagas
// Creation:   August 26, 2011
//
// Modifications:
//
//   Jonathan Byrd (Allinea Software) Sun Dec 18, 2011
//   Added button to focus DDT on the domain of the current page's Pick.
//
// ****************************************************************************

void
QvisPickWindow::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    plotList->Attach(this);
}


// ****************************************************************************
// Method: QvisPickWindow::SubjectRemoved
//
// Purpose:
//   Called when subjects that the window observes are destroyed.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject being removed.
//
// Programmer: Kathleen Biagas
// Creation:   August 26, 2011
//
// Modifications:
//
//   Jonathan Byrd, Sun Dec 18, 2011
//   Record the domain for the Pick on the page that has been updated
//
// ****************************************************************************

void
QvisPickWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(pickAtts == TheRemovedSubject)
        pickAtts = 0;
    if(plotList == TheRemovedSubject)
        plotList = 0;
}

// ****************************************************************************
// Method: QvisPickWindow::focusPickInDDTClicked
//
// Purpose:
//   This is a Qt slot function that focus's the attached DDT on the
//   domain of the currently selected pick
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//   Jonathan Byrd, Mon Feb 4, 2013
//   Focus on domain, variable and element, not just domain
//
// ****************************************************************************
void
QvisPickWindow::focusPickInDDTClicked()
{
    const PickRecord *target = pickRecords[resultsTabWidget->currentIndex()];
    GetViewerMethods()->DDTFocus(target->domain, target->variable, target->element, target->value);
}
