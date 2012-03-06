/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <stdio.h>
#include <QvisQueryWindow.h>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>
#include <QSplitter>
#include <QTextStream>

#include <QueryAttributes.h>
#include <PickAttributes.h>
#include <PlotList.h>
#include <QueryList.h>
#include <StringHelpers.h>
#include <DebugStream.h>
#include <QvisVariableButton.h>
#include <QvisPythonFilterEditor.h>

#include <QvisHohlraumFluxQueryWidget.h>
#include <QvisLineoutQueryWidget.h>
#include <QvisPickQueryWidget.h>
#include <QvisTimeQueryOptionsWidget.h>
#include <QvisXRayImageQueryWidget.h>

#include <ViewerProxy.h>

#ifdef _WIN32
#include <QTemporaryFile>
#include <InstallationFunctions.h>
#endif

using std::string;

// ****************************************************************************
// Method: QvisQueryWindow::QvisQueryWindow
//
// Purpose: 
//   This is the constructor for the QvisQueryWindow class.
//
// Arguments:
//   caption   : The name of the window.
//   shortName : The posted name of the window.
//   n         : The notepad to which the window will post.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:47:41 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002 
//   Initialize queryAtts.
// 
//   Brad Whitlock, Fri Nov 7 17:24:42 PST 2003
//   Prevented extra buttons from being created.
//
//   Ellen Tarwater, Tues May 15 2007
//   Added saveCount for 'Save Results as...' functionality.
//
//   Brad Whitlock, Wed Apr  9 11:46:37 PDT 2008
//   QString for caption, shortName.
//
//   Kathleen Bonnell, Tue Jun 24 11:18:13 PDT 2008
//   Added queryVarTypes, the default types of vars for queries.
//
// ****************************************************************************

QvisQueryWindow::QvisQueryWindow(const QString &caption, 
    const QString &shortName, QvisNotepadArea *n) : 
    QvisPostableWindowSimpleObserver(caption, shortName, n, NoExtraButtons, 
                                     false)
{
    queries = 0;
    queryAtts = 0;
    pickAtts = 0;
    plotList = 0;
    saveCount = 0;
    queryVarTypes = QvisVariableButton::Scalars |
                    QvisVariableButton::Vectors |
                    QvisVariableButton::Meshes |
                    QvisVariableButton::Materials |
                    QvisVariableButton::Species |
                    QvisVariableButton::Tensors |
                    QvisVariableButton::SymmetricTensors |
                    QvisVariableButton::Labels |
                    QvisVariableButton::Arrays;
}

// ****************************************************************************
// Method: QvisQueryWindow::~QvisQueryWindow
//
// Purpose: 
//   The destructor for the QvisQueryWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:48:36 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002 
//   Detach queryAtts.
//   
// ****************************************************************************

QvisQueryWindow::~QvisQueryWindow()
{
    if(queries)
        queries->Detach(this);

    if(queryAtts)
        queryAtts->Detach(this);

    if(pickAtts)
        pickAtts->Detach(this);

    if(plotList)
        plotList->Detach(this);
}

// ****************************************************************************
// Method: QvisQueryWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:48:56 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon May 12 13:02:32 PST 2003
//   I added a button to clear out the query results.
//
//   Kathleen Bonnell, Thu Nov 26 08:30:49 PST 2003 
//   I added radio buttons to select between a database query and a 
//   'current plot' query.  (Only appear if query requests them).
//
//   Kathleen Bonnell, Thu Apr  1 18:46:55 PST 2004 
//   Added TimeQuery push button. 
//
//   Kathleen Bonnell, Thu Apr 22 15:31:24 PDT 2004 
//   Made the default for dataOpts be 'actual data'. 
//
//   Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004 
//   Made the default for dataOpts be 'original data'. 
//
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004 
//   Added displayMode.
//
//   Kathleen Bonnell, Wed Sep  8 10:06:16 PDT 2004 
//   Remove coordLabel. 
//
//   Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004 
//   Added useGlobal checkbox. 
//
//   Kathleen Bonnell, Tue Jan 11 16:16:48 PST 2005 
//   Connect useGlobal to its slot.
//
//   Hank Childs, Fri Sep  1 16:21:35 PDT 2006
//   Change mechanism for handling query groups so that this code doesn't need
//   to be modified every time a new group is added.
//
//   Cyrus Harrison, Tue Sep 18 08:13:05 PDT 2007
//   Added support for user settable floating point format string
//
//   Brad Whitlock, Tue Apr  8 15:44:16 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Tue Jun 24 11:18:13 PDT 2008
//   Added varsButton, varsLineEdit.
//
//   Eric Brugger, Mon May 11 13:48:58 PDT 2009
//   Corrected the layout of some of the widgets in the arguments panel so
//   that the fourth text line isn't clobbered by the variable controls.
//
//   Cyrus Harrison,
//   Refactoring for python query integration. Most of functionality was 
//   moved into the CreateStandardQueryWidget() method.
//
// ****************************************************************************

void
QvisQueryWindow::CreateWindowContents()
{

    splitter = new QSplitter(central);
    splitter->setOrientation(Qt::Vertical);

    // create tab widget to separate std queries & custom python queries.

    queryTabs = new QTabWidget(splitter);

    CreateStandardQueryWidget();
    CreatePythonQueryWidget();
    CreateResultsWidget();

    queryTabs->addTab(stdQueryWidget, "Standard Queries");
    queryTabs->addTab(pyQueryWidget, "Python Query Editor");

    splitter->addWidget(queryTabs);
    splitter->addWidget(resultsWidget);

    topLayout->addWidget(splitter);
}



// ****************************************************************************
// Method: QvisQueryWindow::CreateStandardQueryWidget
//
// Purpose: 
//   This method creates the widgets for the standard query tab.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Feb 17 10:32:41 PST 2010
//
// Notes: Refactored from CreateWindowContents()
//
// Modifications:
//   Eric Brugger, Fri Jul  2 15:54:23 PDT 2010
//   I increased the number of text fields to support the x ray image query.
//
//   Dave Pugmire, Tue Nov  9 16:11:37 EST 2010
//   Added streamline info query.
//
//   Kathleen Bonnell, Tue Mar  1 11:07:20 PST 2011
//   Added plotOpts.
//
//   Kathleen Biagas, Fri Jun 10 08:59:13 PDT 2011
//   Added pickQueryWidget, xRayImageQueryWidget, lineoutQueryWidget,
//   hohlraumFluxQueryWidget, timeQueryOptions.
//
// ****************************************************************************

void
QvisQueryWindow::CreateStandardQueryWidget()
{
    stdQueryWidget = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    stdQueryWidget->setLayout(hLayout);
    QVBoxLayout *vLayout= new QVBoxLayout();
    hLayout->addLayout(vLayout);

    // Create the display mode
    displayMode = new QComboBox(central);
    displayMode->addItem(tr("All"));
    for (int i = 0 ; i < QueryList::NumGroups ; i++)
    {
        QString groupName(QueryList::Groups_ToString((QueryList::Groups)i).c_str());

        // if the groupName ends in "Related", replace with "-related"
        if(groupName.indexOf("Related") ==  groupName.size() - QString("Related").size())
            groupName.replace("Related","-related");
        displayMode->addItem(groupName);
    }

    displayMode->addItem(tr("All queries-over-time"), QueryList::NumGroups+1);
    connect(displayMode, SIGNAL(activated(int)),
            this, SLOT(displayModeChanged(int)));

    vLayout->addWidget(new QLabel(tr("Display "), central));
    vLayout->addWidget(displayMode);

    // Create the query list.
    queryList = new QListWidget(central);

    connect(queryList, SIGNAL(currentRowChanged(int)),
            this, SLOT(selectQuery()));
    QLabel *queryLabel = new QLabel(tr("Queries"), central);
    vLayout->addWidget(queryLabel);
    vLayout->addWidget(queryList);

    // Create the argument panel with its several text fields.
    argPanel = new QGroupBox(central);
    argPanel->setTitle(tr("Query parameters"));

    hLayout->addWidget(argPanel);
    QVBoxLayout *gLayout = new QVBoxLayout(argPanel);

    // Add the vars button to the argument panel
    QHBoxLayout *vbLayout = new QHBoxLayout();
    gLayout->addLayout(vbLayout);
    varsButton = new QvisVariableButton(true, false, true, queryVarTypes,
                                        argPanel);
    varsButton->setText(tr("Variables"));
    varsButton->setChangeTextOnVariableChange(false);
    varsButton->hide();
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    vbLayout->addWidget(varsButton);

    varsLineEdit = new QLineEdit(argPanel);
    varsLineEdit->setText("default"); 
    varsLineEdit->hide();
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(handleText()));
    vbLayout->addWidget(varsLineEdit);

    QGridLayout *sLayout = new QGridLayout();
    sLayout->setMargin(0);
    gLayout->addLayout(sLayout);

    for(int i = 0; i < 6; ++i)
    {
        QString name1, name2;
        name1.sprintf("queryArgLabel%02d", i);
        name2.sprintf("queryArgText%02d", i);
        textFields[i] = new QLineEdit(name2,argPanel);
        connect(textFields[i], SIGNAL(returnPressed()),
                this, SLOT(handleText()));
        textFields[i]->hide();
        sLayout->addWidget(textFields[i], i+2, 1);
        labels[i] = new QLabel(name1,argPanel);
        labels[i]->hide();
        sLayout->addWidget(labels[i], i+2, 0);
    }

    useGlobal = new QCheckBox(tr("Use Global Id"), argPanel);
    connect(useGlobal, SIGNAL(toggled(bool)), this, 
            SLOT(useGlobalToggled(bool)));
    useGlobal->hide();
    sLayout->addWidget(useGlobal, 8, 0, 1, 2);


    // Add the data options radio button group to the argument panel.
    dataOpts = new QButtonGroup(argPanel);
    QRadioButton *origData = new QRadioButton(tr("Original Data"), argPanel);
    dataOpts->addButton(origData,0);
    sLayout->addWidget(origData, 9, 0);
    QRadioButton *actualData = new QRadioButton(tr("Actual Data"), argPanel);
    dataOpts->addButton(actualData,1);
    dataOpts->button(0)->setChecked(true);
    sLayout->addWidget(actualData, 10, 0);

    dumpSteps = new QCheckBox(tr("Dump Steps"), argPanel);
    connect(dumpSteps, SIGNAL(toggled(bool)), this, 
            SLOT(dumpStepsToggled(bool)));
    dumpSteps->hide();
    sLayout->addWidget(dumpSteps, 13, 0, 1, 2);

    // only one of these will be shown at a time, so they can be added
    // to the same location in the layout

    pickQueryWidget = new QvisPickQueryWidget();
    sLayout->addWidget(pickQueryWidget, 14, 0);

    lineoutQueryWidget = new QvisLineoutQueryWidget();
    sLayout->addWidget(lineoutQueryWidget, 14, 0);

    xRayImageQueryWidget = new QvisXRayImageQueryWidget();
    sLayout->addWidget(xRayImageQueryWidget, 14, 0);

    hohlraumFluxQueryWidget = new QvisHohlraumFluxQueryWidget();
    sLayout->addWidget(hohlraumFluxQueryWidget, 14, 0);


    // Add the time query options
    gLayout->addStretch(10);
    timeQueryOptions = new QvisTimeQueryOptionsWidget();
    gLayout->addWidget(timeQueryOptions);
    //
    //  PickQueryWidget needs to know when time is toggled, so it
    //  can enable certain of its own time-specific options.
    //
    connect(timeQueryOptions, SIGNAL(toggled(bool)), 
            pickQueryWidget, SLOT(timeOptionsToggled(bool)));

    // Add the query button to the argument panel.
    QHBoxLayout *qbLayout = new QHBoxLayout();
    gLayout->addLayout(qbLayout);
    qbLayout->addStretch(5);
    queryButton = new QPushButton(tr("Query"), argPanel);
    connect(queryButton, SIGNAL(clicked()),
            this, SLOT(apply()));
    qbLayout->addWidget(queryButton);
    qbLayout->addStretch(5);

    // make sure these are sorted
    UpdateQueryList();
}

// ****************************************************************************
// Method: QvisQueryWindow::CreatePythonQueryWidget
//
// Purpose: 
//   This method creates the widgets for the python query tab.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Feb 17 10:32:41 PST 2010
//
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryWindow::CreatePythonQueryWidget()
{
    pyQueryWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(pyQueryWidget);
    int row = 0;

    pyVarsButton = new QvisVariableButton(true, false, true, queryVarTypes,
                                          pyQueryWidget);
    pyVarsButton->setText(tr("Variables"));
    layout->addWidget(pyVarsButton,row, 0);

    pyVarsButton->setChangeTextOnVariableChange(false);

    connect(pyVarsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addPyVariable(const QString &)));

    pyVarsLineEdit = new QLineEdit(pyQueryWidget);
    pyVarsLineEdit->setText("default");
    layout->addWidget(pyVarsLineEdit,row, 1,1,2);

    row++;

    pyFilterEditLabel = new QLabel(tr("Python Query Script"), pyQueryWidget);
    pyFilterEdit = new QvisPythonFilterEditor();
    layout->addWidget(pyFilterEditLabel, row, 0);
    row++;
    layout->addWidget(pyFilterEdit,row,0,1,3);
    row++;

    connect(pyFilterEdit, SIGNAL(templateSelected(const QString &)),
            this, SLOT(pyTemplateSelected(const QString &)));

    pyQueryButton = new QPushButton(tr("Execute Query"), pyQueryWidget);
    layout->addWidget(pyQueryButton,row,2);

    connect(pyQueryButton, SIGNAL(clicked()),
            this, SLOT(apply()));
}


// ****************************************************************************
// Method: QvisQueryWindow::CreateResultsWidget
//
// Purpose: 
//   This method creates the widgets that display query results.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Feb 17 10:32:41 PST 2010
//
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryWindow::CreateResultsWidget()
{

    resultsWidget = new QWidget(splitter);
    QVBoxLayout *layout = new QVBoxLayout();
    resultsWidget->setLayout(layout);
    QHBoxLayout *title_layout = new QHBoxLayout();

    QLabel *resultLabel = new QLabel(tr("Query results"), resultsWidget);

    QLabel *floatFormatLabel = new QLabel(tr("Float Format:"), resultsWidget);
    floatFormatText = new QLineEdit("%g",resultsWidget);

    title_layout->addWidget(resultLabel);
    title_layout->addStretch(5);
    title_layout->addWidget(floatFormatLabel);
    title_layout->addWidget(floatFormatText);

    // Create the results list.
    resultText = new QTextEdit(resultsWidget);
    resultText->setReadOnly(true);

    layout->addLayout(title_layout);
    layout->addWidget(resultText);
}

// ****************************************************************************
// Method: QvisQueryWindow::CreateEntireWindow
//
// Purpose: 
//   Creates the entire window.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 9 17:23:39 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Nov 14 10:36:09 PDT 2005
//   Disable the post button if posting is not enabled.
//
//   Ellen Tarwater, Mon May 21
//   Added "Save results as..." button.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisQueryWindow::CreateEntireWindow()
{
    // Return if the window is already created.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    central = new QWidget( this );
    setCentralWidget( central );
    topLayout = new QVBoxLayout(central);

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();

    // Create a button layout and the buttons.
    topLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout(buttonLayout);
    QPushButton *clearResultsButton = new QPushButton(tr("Clear results"), central);
    connect(clearResultsButton, SIGNAL(clicked()),
            this, SLOT(clearResultText()));
    buttonLayout->addWidget(clearResultsButton);

    QPushButton *saveResultsButton = new QPushButton(tr("Save results as") + QString("..."), central);
    connect(saveResultsButton, SIGNAL(clicked()),
            this, SLOT(saveResultText()));
    buttonLayout->addWidget(saveResultsButton);
    
    buttonLayout->addStretch();

    postButton = new QPushButton(tr("Post"), central);
    buttonLayout->addWidget(postButton);
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), central);
    buttonLayout->addWidget(dismissButton);
    if(stretchWindow)
        topLayout->addStretch(0);

    // Make the window post itself when the post button is clicked.
    if(notepad)
    {
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        postButton->setEnabled(postEnabled);
    }
    else
        postButton->setEnabled(false);

    // Hide this window when the dimiss button is clicked.
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
}

// ****************************************************************************
// Method: QvisQueryWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window needs to be updated.
//
// Arguments:
//   doAll : A flag that tells whether or not all widgets need to be updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:49:24 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 25 11:31:41 PDT 2002 
//   QueryAtts can also be used to update the results.
//   
//   Kathleen Bonnell, Thu Apr  1 18:46:55 PST 2004 
//   Call update for timeQueryButton.
//   
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004 
//   Removed unncessary argument from UpdateQueryList.
//
// ****************************************************************************

void
QvisQueryWindow::UpdateWindow(bool doAll)
{
    if(SelectedSubject() == queries || doAll)
        UpdateQueryList();

    if(SelectedSubject() == queryAtts || 
       SelectedSubject() == pickAtts || doAll)
        UpdateResults(doAll);

    if(SelectedSubject() == plotList || doAll)
    {
        UpdateQueryButton();
        UpdateTimeQueryOptions();
    }
}

// ****************************************************************************
// Method: QvisQueryWindow::UpdateQueryButton
//
// Purpose: 
//   Sets the enabled state for the query button.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:50:03 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::UpdateQueryButton()
{
    bool val = (queries->GetNames().size() > 0) &&
               (plotList->GetNumPlots() > 0);
    queryButton->setEnabled(val);
}
 
// ****************************************************************************
// Method: QvisQueryWindow::UpdateTimeQueryOptions
//
// Purpose: 
//   Sets the enabled state for the time query options widget.
//
// Programmer: Kathleen Bonnell 
// Creation:   April 1, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::UpdateTimeQueryOptions()
{
    bool val = (queries->GetNames().size() > 0) &&
               (plotList->GetNumPlots() > 0);
    timeQueryOptions->setEnabled(val);
}

// ****************************************************************************
// Method: QvisQueryWindow::UpdateQueryList
//
// Purpose:
//   Populates the list of available queries.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:55:15 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004
//   Removed unncessary argument.  Restructured to display the queries list
//   according to the displayMode specified by user.  All individual lists
//   are now sorted.
//
//   Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//   Reflect changes in queryList -- timeQuery is now queryMode.
//
//   Kathleen Bonnell, Fri Sep 28 14:46:09 PDT 2007 
//   Added 'canBePublic' which when false, allows queries to NOT be added
//   to this window. 
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisQueryWindow::UpdateQueryList()
{
    const stringVector &names = queries->GetNames();
    const intVector &mode = queries->GetQueryMode();
    const intVector &groups = queries->GetGroups();
    const intVector &canBePublic = queries->GetCanBePublic();

    // Add the arguments to the query list.
    queryList->blockSignals(true);
    QString queryName;
    if(queryList->currentItem())
        queryName = queryList->currentItem()->text();
    int selectedIndex = -1;
    int selectedFunction = displayMode->currentIndex() -1;
    queryList->clear();
    int i;
    for(i = 0; i < names.size(); ++i)
    {
        if (!canBePublic[i])
            continue;
        if (displayMode->currentText() == tr("All"))
        {
            queryList->addItem(QString(names[i].c_str()));
        }
        else if (displayMode->currentText() == tr("All queries-over-time") &&
                 mode[i] != QueryList::QueryOnly)
        {
            queryList->addItem(QString(names[i].c_str()));
        }
        else if (groups[i] == selectedFunction)
        {
            queryList->addItem(QString(names[i].c_str()));
        }
    }
    queryList->setSortingEnabled(true);


    // Now that query names are in the list, set the selection.
    bool listEnabled = false;
    if(queryList->count() > 0)
    {
        listEnabled = true;
        selectedIndex = 0;
        for (i = 0; i < queryList->count(); i++)
        {
            if (queryList->item(i)->text() == queryName)
            {
                selectedIndex = i;
                break;
            }
        }

        queryList->setCurrentRow(selectedIndex);
        queryList->item(selectedIndex)->setSelected(true);
        UpdateArgumentPanel(queryList->currentItem()->text());
    }

    queryList->setEnabled(listEnabled);
    queryList->blockSignals(false);
}

// ****************************************************************************
// Method: QvisQueryWindow::UpdateResults
//
// Purpose: 
//   Displays the query results.
//
// Note:       This method will change big time when queries are really
//             implemented in the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:55:39 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 25 11:31:41 PDT 2002 
//   Use pickAtts to set resultText only if the pick was fulfilled.
//   Also use queryAtts to set resultText. 
//
//   Brad Whitlock, Fri May 9 17:27:05 PST 2003
//   I made it append the query results to the existing text.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Hank Childs, Sun Dec  5 15:43:32 PST 2010
//   Make the focus of the query window be at the bottom of the window.
//
//   Kathleen Biagas, Fri Jun 10 14:01:28 PDT 2011
//   Allow output to this window to be suppressed.
//
// ****************************************************************************

void
QvisQueryWindow::UpdateResults(bool)
{
    if (SelectedSubject() == pickAtts && pickAtts->GetFulfilled())
    {
        string str;
        pickAtts->CreateOutputString(str);
        resultText->append(str.c_str());
        resultText->moveCursor(QTextCursor::Down,QTextCursor::MoveAnchor);
        //resultText->setCursorPosition(resultText->numLines() - 1, 0);
    }
    else if (SelectedSubject() == queryAtts && !queryAtts->GetSuppressOutput())
    {
        string str;
        str = queryAtts->GetResultsMessage();
        resultText->append(str.c_str());
        resultText->moveCursor(QTextCursor::Down,QTextCursor::MoveAnchor);
    }
}

// ****************************************************************************
// Method: QvisQueryWindow::UpdateArgumentPanel
//
// Purpose: 
//   Shows the widgets to gather query parameters.
//
// Arguments:
//   index : The index of the desired query.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:56:28 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Nov 18 09:42:12 PST 2002
//   Don't show the variable widget for Eulerian queries.
//   
//   Hank Childs, Tue Mar 18 21:30:09 PST 2003
//   Added revolved surface area.
//
//   Jeremy Meredith, Sat Apr 12 11:31:22 PDT 2003
//   Added compactness.
//
//   Kathleen Bonnell, Wed Jul 23 16:02:22 PDT 2003
//   Added 'Variable by Zone'.
//
//   Hank Childs, Fri Oct  3 16:22:03 PDT 2003
//   Added L2Norm and more.
//
//   Kathleen Bonnell, Mon Nov 17 14:01:45 PST 2003
//   Added 'Plot MinMax'.
//
//   Kathleen Bonnell, Thu Nov 26 08:30:49 PST 2003
//   Reworked code to create panel based on specified window type for a
//   particular query.  Removed all references to specific query names.
//
//   Kathleen Bonnell, Thu Apr  1 18:46:55 PST 2004
//   Added code to handle new time query capabilities.
//
//   Kathleen Bonnell, Thu Apr 22 15:31:24 PDT 2004
//   Made the default for dataOpts be 'actual data'.
//
//   Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004
//   Made the default for dataOpts be 'original data'.
//
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004
//   Changed argument from index to qname -- because queryList box may
//   have fewer items than all queries.
//
//   Kathleen Bonnell, Wed Sep  8 10:06:16 PDT 2004
//   Removed references to QueryList::CoordRep and coordLabel,
//   no longer exists.
//
//   Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004
//   Added logic to handle useGlobal checkbox.
//
//   Kathleen Bonnell, Tue Jan 11 16:16:48 PST 2005
//   Enabled state of labels[0] and textFields[0] may change if useGlobal is
//   toggled, so reset the state to true here.
//
//   Kathleen Bonnell, Mon May  9 13:27:49 PDT 2005
//   Removed 'Samples' from 'DoublePoint' (lineout) query.
//
//   Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//   Reflect changes in queryList -- timeQuery is now queryMode.
//   Allow 'Query' button to not be shown for QueryMde == TimeOnly.
//
//   Hank Childs, Mon Jul 10 17:23:24 PDT 2006
//   Add support for a line distribution.
//
//   Dave Bremer, Fri Dec  8 17:52:22 PST 2006
//   Added a GUI for the hohlraum flux query.
//
//   Cyrus Harrison, Fri Mar 16 14:03:41 PDT 200
//   Added output file gui for the connected components summary query.
//
//   Cyrus Harrison, Wed Dec 19 14:53:19 PST 2007
//   Added gui for shapelet decomposition query.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Tue Jun 24 11:18:13 PDT 2008
//   Queries that require variables now use varsButton and varsLineEdit.
//
//   Kathleen Bonnell, Tue Jun 24 13:38:45 PDT 2008 
//   Limit the variables for Hohlraum Flux to Scalars and Arrays.
//
//   Eric Brugger, Mon May 11 13:48:58 PDT 2009
//   I added an argument to the hohlraum flux query that indicates if the
//   emissivity divided by the absorbtivity should be used in place of the
//   emissivity.
//
//   Eric Brugger, Fri Jul  2 15:54:23 PDT 2010
//   I added the x ray image query.
//
//   Kathleen Bonnell, Tue Mar  1 11:07:43 PST 2011
//   Added plotOpts.
//
//   Kathleen Biagas, Fri Jun 10 08:59:32 PDT 2011
//   Added pickQueryWidget.
//
// ****************************************************************************

void
QvisQueryWindow::UpdateArgumentPanel(const QString &qname)
{
    const intVector &winType = queries->GetWinType();
    const intVector &queryMode = queries->GetQueryMode();
    const stringVector &names = queries->GetNames();
    const intVector &requiresVars = queries->GetRequiresVarSelection();

    int index = -1;
    for (int i = 0; i < names.size(); i++)
    {
        if (string(qname.toStdString()) == names[i])
        {
            index = i;
            break;
        }
    }
    // reset a few defaults
    dataOpts->button(0)->setChecked(true);
    useGlobal->setChecked(0);
    dumpSteps->setChecked(0);
    labels[0]->setEnabled(true);
    textFields[0]->setEnabled(true);

    // turnf off specialized widgets
    hohlraumFluxQueryWidget->setEnabled(false);
    hohlraumFluxQueryWidget->hide();
    lineoutQueryWidget->setEnabled(false);
    lineoutQueryWidget->hide();
    pickQueryWidget->setEnabled(false);
    pickQueryWidget->hide();
    xRayImageQueryWidget->setEnabled(false);
    xRayImageQueryWidget->hide();

    if(index >= 0 && index < winType.size())
    {
        bool showWidgets[6] = {false, false, false, false, false, false};
        bool showDataOptions = false;
        bool showGlobal = false;
        bool showDumpSteps = false;
        QueryList::WindowType winT   = (QueryList::WindowType)winType[index];
        bool showTime = queryMode[index] != QueryList::QueryOnly;
        bool timeOnly = queryMode[index] == QueryList::TimeOnly;
        bool showVars = requiresVars[index];
        bool showTimeCurvePlotType = false;
        varsLineEdit->setText("default");
        varsButton->setVarTypes(queryVarTypes);

        if (winT == QueryList::Pick)
        {
            pickQueryWidget->setEnabled(true);
            pickQueryWidget->show();
        }
        else if (winT == QueryList::Lineout)
        {
            lineoutQueryWidget->setEnabled(true);
            lineoutQueryWidget->show();
            varsButton->setVarTypes(QvisVariableButton::Scalars);
        }
        else if (winT == QueryList::DomainZone)
        {
            labels[0]->setText("Domain");
            textFields[0]->setText("0");
            labels[1]->setText("Zone");
            textFields[1]->setText("0");
            showWidgets[0] = true;
            showWidgets[1] = true;
            useGlobal->setText("Use Global Zone");
            showGlobal = true;
        }
        else if (winT == QueryList::DomainZoneVars)
        {
            varsLineEdit->setText("var_for_x var_for_y");
            labels[0]->setText(tr("Domain"));
            textFields[0]->setText("0");
            labels[1]->setText(tr("Zone"));
            textFields[1]->setText("0");
            showWidgets[0] = true;
            showWidgets[1] = true;
            useGlobal->setText("Use Global Zone");
            showGlobal = true;
            showTimeCurvePlotType = (queries->GetNumVars()[index] < 2);
        }
        else if (winT == QueryList::DomainNode)
        {
            labels[0]->setText(tr("Domain"));
            textFields[0]->setText("0");
            labels[1]->setText(tr("Node"));
            textFields[1]->setText("0");
            showWidgets[0] = true;
            showWidgets[1] = true;
            useGlobal->setText("Use Global Node");
            showGlobal = true;
        }
        else if (winT == QueryList::DomainNodeVars)
        {
            varsLineEdit->setText("var_for_x var_for_y");
            labels[0]->setText(tr("Domain"));
            textFields[0]->setText("0");
            labels[1]->setText(tr("Node"));
            textFields[1]->setText("0");
            showWidgets[0] = true;
            showWidgets[1] = true;
            useGlobal->setText("Use Global Node");
            showGlobal = true;
            showTimeCurvePlotType = (queries->GetNumVars()[index] < 2);
        }
        else if (winT == QueryList::ActualData)
        {
            showDataOptions = true;
        }
        else if (winT == QueryList::ActualDataVars)
        {
            showDataOptions = true;
        }
        else if (winT == QueryList::LineDistribution)
        {
            labels[0]->setText(tr("Number of Lines"));
            textFields[0]->setText("1000");
            showWidgets[0] = true;
            labels[1]->setText(tr("Number of Bins"));
            textFields[1]->setText("100");
            showWidgets[1] = true;
            labels[2]->setText(tr("Min Length"));
            textFields[2]->setText("0.");
            showWidgets[2] = true;
            labels[3]->setText(tr("Max Length"));
            textFields[3]->setText("1.");
            showWidgets[3] = true;
        }
        else if (winT == QueryList::HohlraumFlux)
        {
            varsLineEdit->setText("absorbtivity emissivity");
            varsButton->setVarTypes(QvisVariableButton::Scalars |
                                    QvisVariableButton::Arrays);

            hohlraumFluxQueryWidget->setEnabled(true);
            hohlraumFluxQueryWidget->show();
        }
        else if (winT == QueryList::ConnCompSummary)
        {
            labels[0]->setText(tr("Output File"));
            textFields[0]->setText("cc_summary.okc");
            showWidgets[0] = true;
        }
        else if (winT == QueryList::ShapeletsDecomp)
        {
            labels[0]->setText(tr("Beta"));
            textFields[0]->setText("5.0");
            showWidgets[0] = true;
            labels[1]->setText(tr("NMax"));
            textFields[1]->setText("16");
            showWidgets[1] = true;
            labels[2]->setText(tr("Recomp Output (Vtk)"));
            textFields[2]->setText(tr("[skip]"));
            showWidgets[2] = true;
        }
        else if (winT == QueryList::XRayImage)
        {
            varsLineEdit->setText("absorbtivity emissivity");
            varsButton->setVarTypes(QvisVariableButton::Scalars |
                                    QvisVariableButton::Arrays);

            xRayImageQueryWidget->setEnabled(true);
            xRayImageQueryWidget->show();
        }
        else if (winT == QueryList::StreamlineInfo)
        {
            showDumpSteps = true;
        }

        // hide and show the right text widgets.
        for(int i = 0; i < 6; ++i)
        {
            if(showWidgets[i])
            {
                labels[i]->show();
                textFields[i]->show();
            }
            else
            {
                labels[i]->hide();
                textFields[i]->hide();
            }
        }
        if (showVars)
        {
            varsButton->show();
            varsLineEdit->show();
        }
        else
        {
            varsButton->hide();
            varsLineEdit->hide();
        }
        if (showGlobal)
        {
            useGlobal->show();
        }
        else
        {
            useGlobal->hide();
        }
        if (showDumpSteps)
            dumpSteps->show();
        else
            dumpSteps->hide();

        if (showDataOptions)
        {
            dataOpts->button(0)->show();
            dataOpts->button(1)->show();
        }
        else
        {
            dataOpts->button(0)->hide();
            dataOpts->button(1)->hide();
        }

        if (showTime)
        {
            timeQueryOptions->show();
        }
        else
        {
            timeQueryOptions->hide();
        }

        timeQueryOptions->setCheckable(!timeOnly);
        queryButton->show();
    }
}

// ****************************************************************************
// Method: QvisQueryWindow::SubjectRemoved
//
// Purpose:
//   Called when subjects that the window observes are destroyed.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject being removed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:57:28 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//   Test for queryAtts, too.
//
// ****************************************************************************

void
QvisQueryWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(queries == TheRemovedSubject)
        queries = 0;
    else if(queryAtts == TheRemovedSubject)
        queryAtts = 0;
    else if(pickAtts == TheRemovedSubject)
        pickAtts = 0;
    else if(plotList == TheRemovedSubject)
        plotList = 0;
}

//
// Methods to connect the observed subjects.
//

void
QvisQueryWindow::ConnectQueryList(QueryList *q)
{
    queries = q;
    queries->Attach(this);
}


void
QvisQueryWindow::ConnectQueryAttributes(QueryAttributes *p)
{
    queryAtts = p;
    queryAtts->Attach(this);
}

void
QvisQueryWindow::ConnectPickAttributes(PickAttributes *p)
{
    pickAtts = p;
    pickAtts->Attach(this);
}

void
QvisQueryWindow::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    plotList->Attach(this);
}

// ****************************************************************************
// Method: QvisQueryWindow::Apply
//
// Purpose: 
//   This method is called when we actually want to do a query.
//
// Arguments:
//   ignore : A flag that can make the function apply unconditionally.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:58:47 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//   Removed call to viewer->SetPickAttributes.   
//
//   Brad Whitlock, Thu Dec 26 17:42:37 PST 2002
//   I made it use stringVectors when passing variables names to the
//   viewer proxy.
//
//   Brad Whitlock, Mon May 12 14:26:17 PST 2003
//   I removed the line of code that cleared the query results.
//
//   Kathleen Bonnell, Wed May 14 17:32:20 PDT 2003 
//   Removed Pick related hack. 
//
//   Kathleen Bonnell, Wed Jul 23 16:02:22 PDT 2003 
//   Added special checks for 'Variable by Zone' query.  
//   Include 'sample' in call to viewer->LineQuery.
//
//   Kathleen Bonnell, Thu Nov 26 08:30:49 PST 2003 
//   Removed references to specific query names.  Reworked code to
//   utilize new WindowType ivar.
//
//   Kathleen Bonnell,  Thu Apr  1 18:46:55 PST 2004
//   Added doTime arge to viewer query calls.
//
//   Kathleen Bonnell, Thu Apr 22 15:31:24 PDT 2004
//   Added useActualData to basic DatabaseQuery call.
//
//   Kathleen Bonnell, Tue May 25 16:09:15 PDT 2004
//   Switch order of dom and el in viewerProxy call.
//
//   Kathleen Bonnell, Wed Sep  8 10:06:16 PDT 2004
//   Removed references to QueryList::CoordRep, no longer exists.
//
//   Kathleen Bonnell, Fri Sep 10 13:28:33 PDT 2004
//   The 'currentItem' of the queryList is not the correct index into queries.
//   Test the currentText against queries->names to get valid index.
//
//   Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004
//   Added logic to handle useGlobal checkbox and new WindowTypes.
//
//   Dave Bremer, Fri Dec  8 17:52:22 PST 2006
//   Added argument parsing for the hohlraum flux query.
//
//   Cyrus Harrison, Fri Mar 16 14:05:34 PDT 2007
//   Added argument parsing for connected components summary.
//
//   Cyrus Harrison, Wed Sep 26 09:15:13 PDT 2007
//   Added check for valid floating point format string before executing
//   query.
//
//   Cyrus Harrison, Wed Dec 19 14:53:19 PST 2007
//   Added parsing for shapelet decomposition query.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Sat Oct 18 21:33:18 PDT 2008
//   Fixed parsing error for Connected Components Summary Query, caused by 
//   migration of GetVars to a new text field widget.
//
//   Eric Brugger, Mon May 11 13:48:58 PDT 2009
//   I added an argument to the hohlraum flux query that indicates if the
//   emissivity divided by the absorbtivity should be used in place of the
//   emissivity.  I also corrected the parsing of the hohlraum flux query
//   since it was broken.
//
//   Cyrus Harrison, Wed Feb 17 11:54:26 PST 2010
//   Moved majority of query execution logic to ExecuteStandardQuery().
//   Added path for executing python queries via ExecutePythonQuery().
//
// ****************************************************************************

void
QvisQueryWindow::Apply(bool ignore)
{
    string format = floatFormatText
                         ->displayText().simplified().toStdString();

    if(!StringHelpers::ValidatePrintfFormatString(format.c_str(),
                                                  "float","EOA"))
    {
        Error(tr("Invalid query floating point format string."));
        return;
    }

    GetViewerMethods()->SetQueryFloatFormat(format);

    if(AutoUpdate() || ignore)
    {
        // check which tab is active to determine how to procede
        if(queryTabs->currentIndex() == 0) // std query
            ExecuteStandardQuery();
        else if(queryTabs->currentIndex() == 1) // python script query
            ExecutePythonQuery();

    }
}

// ****************************************************************************
// Method: QvisQueryWindow::ExecuteStandardQuery
//
// Purpose:
//   Prepares arguments and executes the selected standard query.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Wed Feb 17 11:55:51 PST 2010
//
// Note: Refactored from Apply()
//
// Modifications:
//   Eric Brugger, Fri Jul  2 15:54:23 PDT 2010
//   I added the x ray image query.
//  
//   Kathleen Bonnell, Tue Mar  1 11:08:16 PST 2011
//   For TimePicks, send along curvePlotType.
//
//   Kathleen Biagas, Thu Jan 12 09:55:40 PST 2012
//   For Pick, put time options in a MapNode.
//
// ****************************************************************************

void
QvisQueryWindow::ExecuteStandardQuery()
{
    const stringVector &names = queries->GetNames();
    const intVector &types = queries->GetTypes();
    const intVector &winType = queries->GetWinType();

    QString currentText = queryList->currentItem()->text();
    int index = -1;
    for (int i = 0; i < names.size(); i++)
    {
        if (currentText.toStdString() == names[i])
        {
            index = i;
            break;
        }
    }

    if(index >= 0 && index < types.size())
    {
        QueryList::QueryType t = (QueryList::QueryType)types[index];
        QueryList::WindowType winT = (QueryList::WindowType)winType[index];
        double p0[3] = {0., 0., 0.}, p1[3] = {0., 0., 0.};
        stringVector vars;

        bool noErrors = GetVars(vars);

        MapNode queryParams;
        queryParams["query_name"] = names[index];
        queryParams["query_type"] = types[index];
        if (!vars.empty())
            queryParams["vars"] = vars;

        // Gather the query parameters according to the type of
        // window we're using.
        if(winT == QueryList::Basic)
        {
            ;//basic queries don't require any more parameters
        }
        else if ((winT == QueryList::DomainZone) ||
                 (winT == QueryList::DomainNode) || 
                 (winT == QueryList::DomainZoneVars) ||
                 (winT == QueryList::DomainNodeVars))
        {
            int dom = 0, el = 0;
            bool goodDomain = GetNumber(0, &dom);
            if (goodDomain)
                goodDomain = (dom >= 0);
            if (!goodDomain)
                Error(tr("The domain must be an integer >= 0."));
            bool goodEl = GetNumber(1, &el);
            if (goodEl)
                goodEl = (el >= 0);
            if (!goodEl)
            {
                if (winT == QueryList::DomainZone ||
                    winT == QueryList::DomainZoneVars)
                    Error(tr("The zone must be an integer >= 0."));
                else
                    Error(tr("The node must be an integer >= 0."));
            }

            if(noErrors && goodDomain && goodEl)
            {
                queryParams["domain"] = dom;
                queryParams["element"] = el;
                queryParams["use_global_id"] = (int)useGlobal->isChecked();
            }
        }
        else if(winT == QueryList::Lineout)
        {
            if(!lineoutQueryWidget->GetQueryParameters(queryParams))
                noErrors = false;
        }
        else if ((winT == QueryList::ActualData) ||
                 (winT == QueryList::ActualDataVars))
        {
            if (noErrors)
                queryParams["use_actual_data"] = (int)dataOpts->button(1)->isChecked();
        }
        else if (winT == QueryList::LineDistribution)
        {
            int nLines=0;
            if(!GetNumber(0, &nLines))
                noErrors = false;
            int nBins = 0;
            if(!GetNumber(1, &nBins))
                noErrors = false;
            double min = 0.;
            if(!GetFloatingPointNumber(2, &min))
                noErrors = false;
            double max = 0.;
            if(!GetFloatingPointNumber(3, &max))
                noErrors = false;
            if (noErrors)
            {
                queryParams["min"] = min;
                queryParams["max"] = max;
                queryParams["num_lines"] = nLines;
                queryParams["num_bins"]  = nBins;
            }
        }
        else if (winT == QueryList::HohlraumFlux)
        {
            if (vars.size() != 2)
                noErrors = false;

            if (!hohlraumFluxQueryWidget->GetQueryParameters(queryParams))
                noErrors = false;
        }
        else if (winT == QueryList::ConnCompSummary)
        {
            // get from textFields[0] (this used to be hooked up to GetVars ...)
            string ofile = textFields[0]->text().simplified().toStdString();
            if(ofile == "")
                noErrors = false;
            if (noErrors)
                queryParams["output_file"] = ofile;
        }
        else if (winT == QueryList::ShapeletsDecomp)
        {
            string ofile = "";
            QString ofqs;

            doubleVector dvec;
            dvec.resize(1);
            if(!GetFloatingPointNumber(0, &dvec[0]))
                noErrors = false;

            int nmax = 0;
            if(!GetNumber(1, &nmax))
                noErrors = false;

            ofqs = textFields[2]->displayText();
            ofile = ofqs.simplified().toStdString();

            if(noErrors)
            {
                queryParams["beta"] = dvec[0];
                queryParams["nmax"] = nmax;
                if(ofile != "[skip]")
                    queryParams["recomp_file"] = ofile;
            }
        }
        else if (winT == QueryList::XRayImage)
        {
            if (vars.size() != 2)
                noErrors = false;

            if (!xRayImageQueryWidget->GetQueryParameters(queryParams))
                noErrors = false;
        }
        else if (winT == QueryList::StreamlineInfo)
        {
            if(noErrors)
            {
                queryParams["dump_steps"] = (int)dumpSteps->isChecked();
            }
        }
        else if(winT == QueryList::Pick)
        {
            if (!pickQueryWidget->GetQueryParameters(queryParams))
                noErrors = false;
        }

        if(!timeQueryOptions->isCheckable() || timeQueryOptions->isChecked())
        {
            if (winT != QueryList::Pick)
            {
                noErrors = timeQueryOptions->GetTimeQueryOptions(queryParams);
            }
            else
            {
                // Pick needs its time options all together in one MapNode.
                MapNode timeOptions;
                noErrors = timeQueryOptions->GetTimeQueryOptions(timeOptions);
                if (noErrors)
                    queryParams["time_options"] = timeOptions;
            }
            if (noErrors)
                queryParams["do_time"] = 1;
        }

        // Display a status message.
        if(noErrors)
        {
            GetViewerMethods()->Query(queryParams);
            QString str = tr("Performing %1 query.").
                            arg(names[index].c_str());
            Status(str);
        }
    }
}


// ****************************************************************************
// Method: QvisQueryWindow::ExecutePythonQuery
//
// Purpose:
//   Prepares arguments and executes a python query.
//
// Arguments:
//
// Programmer: Cyrus Harrison
// Creation:   Mon Wed Feb 17 11:55:51 PST 2010
//
//
// Modifications:
//  Cyrus Harrison, Tue Sep 21 11:23:00 PDT 2010
//  Support passing of arbitary arguments via "args" keyword.
//
//  Kathleen Biagas, Fri Jun 10 08:59:13 PDT 2011
//  Send args to viewer in a MapNode.
//
// ****************************************************************************

void
QvisQueryWindow::ExecutePythonQuery()
{
    stringVector py_args;
    QString vars(pyVarsLineEdit->displayText().trimmed());
    QStringList vlist(vars.split(" "));

    // Split the variable list using the spaces.
    QStringListIterator itr(vlist);
    while(itr.hasNext())
        py_args.push_back(itr.next().toStdString());
    // blank spot for 'args' that can be passed via the cli
    py_args.push_back("");

    // get python script from pyFilterEdit
    QString query_def = pyFilterEdit->getSource();
    py_args.push_back(query_def.toStdString());

    MapNode params;
    params["query_name"] = string("Python");
    params["query_type"] = (QueryList::QueryType)QueryList::DatabaseQuery;
    params["vars"] = py_args;

    GetViewerMethods()->Query(params);
    Status("Executing Python Script Query");
}


// ****************************************************************************
// Method: QvisQueryWindow::GetNumber
//
// Purpose: 
//   Gets an integer from the i'th text field.
//
// Arguments:
//   index : The index of the text field to use.
//   num   : The integer in which the number will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:54:53 PST 2002
//
// Modifications:
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

bool
QvisQueryWindow::GetNumber(int index, int *num)
{
    bool okay = false;

    if(index >= 0 && index < 4)
    {
        QString temp(textFields[index]->displayText().simplified());
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.toStdString().c_str(), "%d", num) == 1);
        }
    }

    return okay;
}

// ****************************************************************************
// Method: QvisQueryWindow::GetFloatingPointNumber
//
// Purpose:
//   Gets an floating point number from the i'th text field.
//
// Arguments:
//   index : The index of the text field to use.
//   num   : The floating point number for storage.
//
// Returns:    True if it worked.
//
// Programmer: Hank Childs
// Creation:   July 10, 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

bool
QvisQueryWindow::GetFloatingPointNumber(int index, double *num)
{
    bool okay = false;

    if(index >= 0 && index < 4)
    {
        QString temp(textFields[index]->displayText().simplified());
        okay = !temp.isEmpty();
        if(okay)
        {
            float tmp;
            okay = (sscanf(temp.toStdString().c_str(), "%g", &tmp) == 1);
            *num = tmp;
        }
    }

    return okay;
}

// ****************************************************************************
// Method: QvisQueryWindow::GetVars
//
// Purpose:
//   Gets a list of variables from the i'th text field.
//
// Arguments:
//   index : The index of the text field to use.
//   vars  : The output string vector.
//
// Returns:    True if it worked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:54:53 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 17:45:37 PST 2002
//   I made it store its strings into a stringVector.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Tue Jun 24 11:18:13 PDT 2008
//   Reworked to retrieve vars from varsLineEdit. Removed 'index' arg.
//
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   When spitting to create the vars list, discard empty parts.
//
// ****************************************************************************

bool
QvisQueryWindow::GetVars(stringVector &vars)
{
    bool okay = false;

    QString temp(varsLineEdit->displayText().trimmed());

    // Split the variable list using the spaces.
    QStringList sList = temp.split(" ",QString::SkipEmptyParts);

    QStringList::Iterator it;

    for (it = sList.begin(); it != sList.end(); ++it)
    {
        vars.push_back((*it).toStdString());
    }
    okay = !vars.empty();
    if(!okay)
        Error(tr("The list of variables contains an error."));

    return okay;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisQueryWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Query button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:57:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisQueryWindow::selectQuery
//
// Purpose: 
//   This is a Qt slot function that is called when we select a new query.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:57:37 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Sat Sep  4 11:49:58 PDT 2004 
//   Changed argument to UpdateArgumentPaenl from index to qname -- 
//   because queryList box may have fewer items than all queries. 
//   
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisQueryWindow::selectQuery()
{
    int index = queryList->currentRow();
    if(index >= 0)
        UpdateArgumentPanel(queryList->currentItem()->text());
}

// ****************************************************************************
// Method: QvisQueryWindow::handleText
//
// Purpose: 
//   This is a Qt slot function that is called when Return is pressed in a 
//   text field.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:58:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::handleText()
{
    Apply();
}

// ****************************************************************************
// Method: QvisQueryWindow::clearResultText
//
// Purpose: 
//   This is a Qt slot function that clears the results text.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 9 17:28:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::clearResultText()
{
    resultText->setText("");
}

// ****************************************************************************
// Method: QvisQueryWindow::displayModeChanged
//
// Purpose: 
//   Updates the query list when display mode changes. 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::displayModeChanged(int)
{
    UpdateQueryList(); 
}


// ****************************************************************************
// Method: QvisQueryWindow::useGlobalToggled
//
// Purpose: 
//   A slot function called when the useGlobal checkbox is toggled. 
//   Sets the 'enabled' state of the label and textfield correpsonding
//   to 'Domain'.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 11, 2005 
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::useGlobalToggled(bool val)
{
    labels[0]->setEnabled(!val);
    textFields[0]->setEnabled(!val);
}


// ****************************************************************************
// Method:  QvisQueryWindow::dumpStepsToggled
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
// ****************************************************************************

void
QvisQueryWindow::dumpStepsToggled(bool val)
{
    dumpSteps->setChecked(val);
}


// ****************************************************************************
// Method: QvisQueryWindow::saveResultText
//
// Purpose: 
//   This is a Qt slot function that saves the results text in a user selected file.
//
// Programmer: Ellen Tarwater
// Creation:   Tuesday May 15 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 24 16:21:00 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Thu Dec  4 09:38:44 PST 2008
//   Added default file support to the save as dialog.
//   Skip file save if no results are available. 
//
//    Kathleen Bonnell, Fri May 13 13:28:45 PDT 2011
//    On Windows, explicitly test writeability of the 'cwd' before passing it 
//    to getSaveFileName (eg don't present user with a place to save a file if 
//    they cannot save there!)
//
// ****************************************************************************

void
QvisQueryWindow::saveResultText()
{
    // make sure there are results to save!
    
    QString result_txt( resultText->toPlainText() );
    if ( result_txt.length() == 0 )
    {
        Error(tr("There are currently no query results to save."));
        return;
    }

    QString saveExtension(".txt");

    // Create the name of a VisIt save file to use.
    QString defaultFile;
    defaultFile.sprintf("visit%04d", saveCount);
    defaultFile += saveExtension;
    
    QString useDir = QDir::current().path();
  
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
    QString sFilter(QString("VisIt ") + tr("save") + QString(" (*") + saveExtension + ")");
    
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Query Results As"),
                                                    defaultFile,
                                                    sFilter);
    // If the user choose to save a file, write the query result text
    // to that file.
    if(!fileName.isNull())
    {
        ++saveCount;
        
        QFile file( fileName );
        if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
        {
            QTextStream stream( &file );
            stream << result_txt + "\n";
        
            file.close();
        }
        else
            Error(tr("VisIt could not save the query results "
                     "to the selected file"));
        
    }
}


// ****************************************************************************
// Method: QvisQueryWindow::addVariable
//
// Purpose: 
//   This is a Qt slot function that is called when the user selects a new
//   variable.
//
// Arguments:
//   var : The variable to add.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 24, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::addVariable(const QString &var)
{
    // Add the new variable to the variable line edit.
    QString varString(varsLineEdit->displayText());
    if(varString.length() > 0)
        varString += " ";
    varString += var;
    varsLineEdit->setText(varString);
}

// ****************************************************************************
// Method: QvisQueryWindow::addPyVariable
//
// Purpose:
//   Qt slot function that is called when the user adds a new variable to
//   a python query.
//
// Arguments:
//   var : The variable to add.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Feb 17 11:27:15 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryWindow::addPyVariable(const QString &var)
{
    // Add the new variable to the variable line edit.
    QString res(pyVarsLineEdit->displayText());
    if(res.length() > 0)
        res += QString(" ");
    res += var;
    pyVarsLineEdit->setText(res);
}


// ****************************************************************************
// Method: QvisQueryWindow::pyTemplateSelected
//
// Purpose:
//   This is a Qt slot function that loads a filter template.
//
// Arguments:
//   tname : Template type.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 15:22:08 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisQueryWindow::pyTemplateSelected(const QString &tname)
{
    QString fname("");
    QString tdir = QvisPythonFilterEditor::templateDirectory();

    if(tname == QString("Advanced"))
        fname = tdir + QString("advanced_query.py");
    else if(tname == QString("Simple"))
        fname = tdir + QString("simple_query.py");

    pyFilterEdit->loadScript(fname);
}
