#include <stdio.h>
#include <QvisQueryWindow.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstringlist.h>

#include <QueryAttributes.h>
#include <PickAttributes.h>
#include <PlotList.h>
#include <QueryList.h>

#include <DebugStream.h>

#include <ViewerProxy.h>

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
// ****************************************************************************

QvisQueryWindow::QvisQueryWindow(const char *caption, const char *shortName,
    QvisNotepadArea *n) : QvisPostableWindowSimpleObserver(
    caption, shortName, n, NoExtraButtons, false)
{
    queries = 0;
    queryAtts = 0;
    pickAtts = 0;
    plotList = 0;
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
// ****************************************************************************

void
QvisQueryWindow::CreateWindowContents()
{
    QHBoxLayout *hLayout = new QHBoxLayout(topLayout);
    QVBoxLayout *vLayout = new QVBoxLayout(hLayout);
    // Create the query list.
    queryList = new QListBox(central, "queryList");
    queryList->setSelectionMode(QListBox::Single);
    connect(queryList, SIGNAL(selectionChanged()),
            this, SLOT(selectQuery()));
    QLabel *queryLabel = new QLabel(queryList, "Queries", central,
        "queryLabel");
    vLayout->addWidget(queryLabel);
    vLayout->addWidget(queryList);

    // Create the argument panel with its several text fields.
    argPanel = new QGroupBox(central, "argPanel");
    argPanel->setTitle("Query parameters");
    argPanel->setMargin(10);
    hLayout->addWidget(argPanel);
    QVBoxLayout *gLayout = new QVBoxLayout(argPanel);
    gLayout->addSpacing(15);
    QGridLayout *sLayout = new QGridLayout(gLayout, 6, 2);
    sLayout->setMargin(10);
    sLayout->setSpacing(5);
    coordLabel = new QLabel("** Use screen coordinates **", argPanel, "coordLabel");
    sLayout->addMultiCellWidget(coordLabel, 0, 0, 0, 1);
    coordLabel->hide();    
//    sLayout->addRowSpacing(0, 15);
    for(int i = 0; i < 4; ++i)
    {
        QString name1, name2;
        name1.sprintf("queryArgLabel%02d", i);
        name2.sprintf("queryArgText%02d", i);
        textFields[i] = new QLineEdit(argPanel, name2);
        connect(textFields[i], SIGNAL(returnPressed()),
                this, SLOT(handleText()));
        textFields[i]->hide();
        sLayout->addWidget(textFields[i], i+1, 1);
        labels[i] = new QLabel(argPanel, name1);
        labels[i]->hide();
        sLayout->addWidget(labels[i], i+1, 0);
    }
  
    // Add the plot optionos radio button group to the argument panel.
    dataOpts = new QButtonGroup(0, "dataOpts");
    QRadioButton *origData = new QRadioButton("Original Data", argPanel, "origData");
    dataOpts->insert(origData);
    sLayout->addWidget(origData, 5, 0);
    QRadioButton *actualData = new QRadioButton("Actual Data", argPanel, "actualData");
    dataOpts->insert(actualData);
    dataOpts->setButton(0);
    sLayout->addWidget(actualData, 6, 0);

    // Add the time button to the argument panel.
    gLayout->addStretch(10);
    QHBoxLayout *tbLayout = new QHBoxLayout(gLayout);
    tbLayout->addStretch(5);
    timeQueryButton = new QPushButton("Time Curve", 
                                      argPanel, "timeQueryButton");
    connect(timeQueryButton, SIGNAL(clicked()),
            this, SLOT(timeApply()));
    tbLayout->addWidget(timeQueryButton);
    tbLayout->addStretch(5);
    gLayout->addSpacing(10);

    // Add the query button to the argument panel.
    gLayout->addStretch(10);
    QHBoxLayout *qbLayout = new QHBoxLayout(gLayout);
    qbLayout->addStretch(5);
    queryButton = new QPushButton("Query", argPanel, "queryButton");
    connect(queryButton, SIGNAL(clicked()),
            this, SLOT(apply()));
    qbLayout->addWidget(queryButton);
    qbLayout->addStretch(5);
    gLayout->addSpacing(10);

    // Create the results list.
    resultText = new QMultiLineEdit(central, "resultText");
    resultText->setReadOnly(true);
    QLabel *resultLabel = new QLabel(resultText, "Query results",
        central, "resultLabel");
    topLayout->addWidget(resultLabel);
    topLayout->addWidget(resultText);
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
    topLayout = new QVBoxLayout(central, 10);

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();

    // Create a button layout and the buttons.
    topLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    QPushButton *clearResultsButton = new QPushButton("Clear results", central,
            "clearResultsButton");
    connect(clearResultsButton, SIGNAL(clicked()),
            this, SLOT(clearResultText()));
    buttonLayout->addWidget(clearResultsButton);
    buttonLayout->addStretch();

    postButton = new QPushButton("Post", central,
        "postButton");
    buttonLayout->addWidget(postButton);
    QPushButton *dismissButton = new QPushButton("Dismiss", central,
        "dismissButton");
    buttonLayout->addWidget(dismissButton);
    if(stretchWindow)
        topLayout->addStretch(0);

    // Make the window post itself when the post button is clicked.
    if(notepad)
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
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
// ****************************************************************************

void
QvisQueryWindow::UpdateWindow(bool doAll)
{
    if(SelectedSubject() == queries || doAll)
        UpdateQueryList(doAll);

    if(SelectedSubject() == queryAtts || 
       SelectedSubject() == pickAtts || doAll)
        UpdateResults(doAll);

    if(SelectedSubject() == plotList || doAll)
    {
        UpdateQueryButton();
        UpdateTimeQueryButton();
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
// Method: QvisQueryWindow::UpdateTimeQueryButton
//
// Purpose: 
//   Sets the enabled state for the time query button.
//
// Programmer: Kathleen Bonnell 
// Creation:   April 1, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::UpdateTimeQueryButton()
{
    bool val = (queries->GetNames().size() > 0) &&
               (plotList->GetNumPlots() > 0);
    timeQueryButton->setEnabled(val);
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
//   
// ****************************************************************************

void
QvisQueryWindow::UpdateQueryList(bool)
{
    const stringVector &names = queries->GetNames();

    // Add the arguments to the query list.
    queryList->blockSignals(true);
    int selectedIndex = queryList->currentItem();
    queryList->clear();
    for(int i = 0; i < names.size(); ++i)
        queryList->insertItem(QString(names[i].c_str()));

    // Now that query names are in the list, set the selection.
    bool listEnabled = false;
    if(names.size() > 0)
    {
        listEnabled = true;
        if(selectedIndex == -1 || selectedIndex >= names.size())
            selectedIndex = 0;

        queryList->setCurrentItem(selectedIndex);
        queryList->setSelected(selectedIndex, true);
        UpdateArgumentPanel(selectedIndex);
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
// ****************************************************************************

void
QvisQueryWindow::UpdateResults(bool)
{
    if (SelectedSubject() == pickAtts && pickAtts->GetFulfilled())
    {
        std::string str;
        pickAtts->CreateOutputString(str);
        resultText->insertLine(str.c_str());
        resultText->setCursorPosition(resultText->numLines() - 1, 0);
    }
    else if (SelectedSubject() == queryAtts)
    {
        std::string str;
        str = queryAtts->GetResultsMessage();
        resultText->insertLine(str.c_str());
        resultText->setCursorPosition(resultText->numLines() - 1, 0);
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
// ****************************************************************************

void
QvisQueryWindow::UpdateArgumentPanel(int index)
{
    const intVector &rep = queries->GetCoordRep();
    const intVector &winType = queries->GetWinType();
    const intVector &timeQuery = queries->GetTimeQuery();

    // reset a few defaults
    dataOpts->setButton(0);
    
    if(index < winType.size())
    {
        bool showWidgets[4] = {false, false, false, false};
        bool showCoordLabel = false;
        bool showDataOptions = false;
        QueryList::WindowType winT = (QueryList::WindowType)winType[index];
        QueryList::CoordinateRepresentation r;
        r = (QueryList::CoordinateRepresentation)rep[index];
        bool showTime = (bool)timeQuery[index];
      
        labels[0]->setText("Variables");
        textFields[0]->setText("default");

        if (winT == QueryList::SinglePoint)
        {
            labels[0]->setText("Query point");
            if(r == QueryList::WorldSpace)
                textFields[0]->setText("0 0 0");
            else
            {
                showCoordLabel = true;
                textFields[0]->setText("100 100");
            }
            labels[1]->setText("Variables");
            textFields[1]->setText("default");
            showWidgets[0] = true;
            showWidgets[1] = true;
        }
        else if (winT == QueryList::DoublePoint)
        {
            labels[0]->setText("Start point");
            if(r == QueryList::WorldSpace)
                textFields[0]->setText("0 0 0");
            else
            {
                showCoordLabel = true;
                textFields[0]->setText("0 0");
            }
            labels[1]->setText("End point");
            if(r == QueryList::WorldSpace)
                textFields[1]->setText("1 0 0");
            else
                textFields[1]->setText("100 100");
            labels[2]->setText("Samples");
            textFields[2]->setText("50");
            labels[3]->setText("Variables");
            textFields[3]->setText("default");
            showWidgets[0] = true;
            showWidgets[1] = true;
            showWidgets[2] = true;
            showWidgets[3] = true;
        }
        else if (winT == QueryList::DomainZone)
        {
            labels[0]->setText("Domain");
            textFields[0]->setText("0");
            labels[1]->setText("Zone");
            textFields[1]->setText("0");
            labels[2]->setText("Variables");
            textFields[2]->setText("default");
            showWidgets[0] = true;
            showWidgets[1] = true;
            showWidgets[2] = true;
        }
        else if (winT == QueryList::DomainNode)
        {
            labels[0]->setText("Domain");
            textFields[0]->setText("0");
            labels[1]->setText("Node");
            textFields[1]->setText("0");
            labels[2]->setText("Variables");
            textFields[2]->setText("default");
            showWidgets[0] = true;
            showWidgets[1] = true;
            showWidgets[2] = true;
        }
        else if (winT == QueryList::ActualData)
        {
            showDataOptions = true;
        }
        else if (winT == QueryList::ActualDataVars)
        {
            labels[0]->setText("Variables");
            textFields[0]->setText("default");
            showWidgets[0] = true;
            showDataOptions = true;
        }

        // hide and show the right text widgets.
        for(int i = 0; i < 4; ++i)
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

        if (showDataOptions)
        {
            dataOpts->find(0)->show();
            dataOpts->find(1)->show();
        }
        else
        {
            dataOpts->find(0)->hide();
            dataOpts->find(1)->hide();
        }

        if(showCoordLabel)
            coordLabel->show();
        else
            coordLabel->hide();

        if (showTime)
            timeQueryButton->show();
        else 
            timeQueryButton->hide();
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
// ****************************************************************************

void
QvisQueryWindow::Apply(bool ignore, bool doTime)
{
    if(AutoUpdate() || ignore)
    {
        int index = queryList->currentItem();
        int useActualData = dataOpts->id(dataOpts->selected());
        const stringVector &names = queries->GetNames();
        const intVector &types = queries->GetTypes();
        const intVector &rep = queries->GetCoordRep();
        const intVector &winType = queries->GetWinType();
        if(index >= 0 && index < types.size())
        {
            QueryList::QueryType t = (QueryList::QueryType)types[index];
            QueryList::WindowType winT = (QueryList::WindowType)winType[index];
            bool noErrors = true;
            double p0[3] = {0., 0., 0.}, p1[3] = {0., 0., 0.};
            stringVector vars;
 
            // Gather the query parameters according to the type of
            // window we're using.
            if(winT == QueryList::Basic)
            {
                if(!GetVars(0, vars))
                    noErrors = false;

                if(noErrors)
                {
                    if (t == QueryList::DatabaseQuery)
                    {
                        viewer->DatabaseQuery(names[index], vars, doTime, useActualData);
                    }
                    else 
                    {
                        debug5 << "QueryWindow -- Attempted use BasicWindow "
                               << "with non DatabaseQuery." << endl;
                    }
                }
            }
            else if ((winT == QueryList::DomainZone) ||
                     (winT == QueryList::DomainNode))
            {
                int dom = 0, el = 0;
                bool goodDomain = GetNumber(0, &dom);
                if (goodDomain)
                    goodDomain = (dom >= 0);
                if (!goodDomain)
                    Error("The domain must be an integer >= 0.");
                bool goodEl = GetNumber(1, &el);
                if (goodEl)
                    goodEl = (el >= 0);
                if (!goodEl)
                {
                    if (winT == QueryList::DomainZone)
                        Error("The zone must be an integer >= 0.");
                    else
                        Error("The node must be an integer >= 0.");
                }

                if(!GetVars(2, vars))
                    noErrors = false;

                if(noErrors && goodDomain && goodEl)
                {
                    if (t == QueryList::DatabaseQuery)
                    {
                        viewer->DatabaseQuery(names[index], vars, doTime, el, dom);
                    }
                    else if (t == QueryList::PointQuery)
                    {
                        viewer->PointQuery(names[index], p0, vars, doTime, el, dom);
                    }
                    else 
                    {
                        debug5 << "QueryWindow -- Attempted use DomainWindow "
                               << "with non Database or non Point Query." << endl;
                    }
                }
            }
            else if(winT == QueryList::SinglePoint)
            {
                if(!GetPoint(0, "query point", rep[index], p0))
                    noErrors = false;
                if(!GetVars(1, vars))
                    noErrors = false;

                if(noErrors)
                {
                    if (t == QueryList::PointQuery)
                    {
                        viewer->PointQuery(names[index], p0, vars, doTime);
                    }
                    else 
                    {
                        debug5 << "QueryWindow -- Attempted use SinglePointWindow "
                               << "with non PointQuery." << endl;
                    }
                }
            }
            else if(winT == QueryList::DoublePoint)
            {
                if(!GetPoint(0, "start point", rep[index], p0))
                    noErrors = false;
                if(!GetPoint(1, "end point", rep[index], p1))
                    noErrors = false;

                int sample = 50;
                bool goodSample = GetNumber(2, &sample);
                if(goodSample)
                    goodSample = (sample > 1);
                if(!goodSample)
                {
                    Error("The number of samples is not valid. "
                          "It is a single integer value greater than 1.");
                }
                if(!GetVars(3, vars))
                    noErrors = false;

                if(noErrors && goodSample)
                {
                    if (t == QueryList::LineQuery)
                    {
                        viewer->LineQuery(names[index], p0, p1, vars, sample);
                    }
                    else 
                    {
                        debug5 << "QueryWindow -- Attempted use DoublePointWindow "
                               << "with non LineQuery." << endl;
                    }
                }
            }
            else if ((winT == QueryList::ActualData) ||
                     (winT == QueryList::ActualDataVars))
            {
                if(!GetVars(0, vars))
                    noErrors = false;
                if (noErrors)
                {
                    if (t == QueryList::DatabaseQuery)
                    {
                        viewer->DatabaseQuery(names[index], vars, doTime, useActualData);
                    }
                    else 
                    {
                        debug5 << "QueryWindow -- Attempted use ActualDataWindow "
                               << "with non DatabaseQuery." << endl;
                    }
                }
            }

            // Display a status message.
            if(noErrors)
            {
                QString str;
                str.sprintf("Performing %s query.", names[index].c_str());
                Status(str);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisQueryWindow::GetPoint
//
// Purpose: 
//   Gets a point from the i'th text field.
//
// Arguments:
//   index : The index of the text field to use.
//   pname : The point name being read.
//   rep   : The coordinate representation.
//   pt    : The array in which the point will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 17:54:53 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 22 14:02:37 PDT 2003
//   Allow for only 2 world-space coordinates, setting z to 0 if not provided.
//   
// ****************************************************************************

bool
QvisQueryWindow::GetPoint(int index, const QString &pname, int rep, double pt[3])
{
    bool okay = false;

    if(index >= 0 && index < 4)
    {
        QString temp(textFields[index]->displayText().simplifyWhiteSpace());
        okay = !temp.isEmpty();
        if(okay)
        {
            QueryList::CoordinateRepresentation r;
            r = (QueryList::CoordinateRepresentation)rep;
            if(r == QueryList::WorldSpace)
            {
                pt[2] = 0.;
                int numScanned = sscanf(temp.latin1(), "%lg %lg %lg",
                        &pt[0], &pt[1], &pt[2]);
                okay = (numScanned == 2 || numScanned == 3);
                if(!okay)
                {
                    QString msg;
                    msg.sprintf("The %s is not valid. It should consist of "
                                 "two or three real world coordinate values.",
                                 pname.latin1());
                    Error(msg);
                }
            }
            else if(r == QueryList::ScreenSpace)
            {
                int x, y;
                okay = (sscanf(temp.latin1(), "%d %d", &x, &y) == 2);
                if(okay)
                {
                    pt[0] = (double)x; pt[1] = (double)y; pt[2] = 0.;
                }
                else
                {
                    QString msg;
                    msg.sprintf("The %s is not valid. It should consist of "
                                "two integer screen coordinate values.",
                                pname.latin1());
                    Error(msg);
                }
            }
        }
    }

    return okay;
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
//   
// ****************************************************************************

bool
QvisQueryWindow::GetNumber(int index, int *num)
{
    bool okay = false;

    if(index >= 0 && index < 4)
    {
        QString temp(textFields[index]->displayText().simplifyWhiteSpace());
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.latin1(), "%d", num) == 1);
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
// ****************************************************************************

bool
QvisQueryWindow::GetVars(int index, stringVector &vars)
{
    bool okay = false;

    if(index >= 0 && index < 4)
    {
        QString temp(textFields[index]->displayText().simplifyWhiteSpace());
        okay = !temp.isEmpty();
        if(okay)
        {
            // Split the variable list using the spaces.
            QStringList sList(QStringList::split(" ", temp));

            // Copy the list into the string vector.
            for(int i = 0; i < sList.count(); ++i)
                vars.push_back(std::string(sList[i].latin1()));
        }
    }

    if(!okay)
        Error("The list of variables contains an error.");

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
// Method: QvisQueryWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the TimeCurve button is 
//   clicked.
//
// Programmer: Kathleen Bonnell 
// Creation:   Thu Apr  1 18:42:52 PST 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisQueryWindow::timeApply()
{
    Apply(true, true);
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
//   
// ****************************************************************************

void
QvisQueryWindow::selectQuery()
{
    int index = queryList->currentItem();
    if(index >= 0)
        UpdateArgumentPanel(index);
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
