#include <stdio.h> // for sscanf
#if !defined(_WIN32)
#include <strstream.h>
#endif
#include <string>
#include <vector>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qtabwidget.h>
#include <qstringlist.h>
#include <qvbox.h>

#include <QvisPickWindow.h>
#include <PickAttributes.h>
#include <ViewerProxy.h>
#include <DebugStream.h>
#include <PickVarInfo.h>

using std::string;
using std::vector;


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
// ****************************************************************************

QvisPickWindow::QvisPickWindow(PickAttributes *subj, const char *caption, 
                             const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false)
{
    pickAtts = subj;
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
//
// ****************************************************************************

QvisPickWindow::~QvisPickWindow()
{
    pickAtts = 0;
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
// ****************************************************************************

void
QvisPickWindow::CreateWindowContents()
{
    tabWidget = new QTabWidget(central, "tabWidget");
    tabWidget->setMinimumHeight(200);
    topLayout->addWidget(tabWidget, 10);
    for (int i = 0; i < MAX_PICK_TABS; i++)
    {
        pages[i] = new QVBox(central, "page");
        pages[i]->setMargin(10);
        pages[i]->setSpacing(5);
        infoLists[i]  = new QListBox(pages[i], "infoList");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->insertItem("          ");
        infoLists[i]->setRowMode(9);
        infoLists[i]->setColumnMode(1);
        infoLists[i]->setVariableHeight(false);
        tabWidget->addTab(pages[i]," "); 
    }
    QGridLayout *gLayout = new QGridLayout(topLayout, 2, 4);

    varsLineEdit = new QLineEdit(central, "varsLineEdit");
    varsLineEdit->setText("default"); 
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    gLayout->addWidget(varsLineEdit, 0, 1);
    gLayout->addWidget(new QLabel(varsLineEdit, "variables", 
                                  central, "varLabel"), 0, 0);

    useNodeCoords = new QCheckBox("Return coordinates of nodes", central, 
                                  "useNodeCoords");
    connect(useNodeCoords, SIGNAL(toggled(bool)),
            this, SLOT(useNodeCoordsToggled(bool)));
    gLayout->addMultiCellWidget(useNodeCoords, 1, 1, 0, 1);

    logicalCoords = new QCheckBox("Logical ", central, "logicalCoords");
    logicalCoords->setEnabled(useNodeCoords->isChecked());
    connect(logicalCoords, SIGNAL(toggled(bool)),
            this, SLOT(logicalCoordsToggled(bool)));
    gLayout->addMultiCellWidget(logicalCoords, 1, 1, 2, 3);
}

// ****************************************************************************
// Method: QvisPickWindow::UpdateWindow
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
// ****************************************************************************
void
QvisPickWindow::UpdateWindow(bool doAll)
{
    if (pickAtts == 0)
        return;

    //
    //  If pick letter changes, it indicates we need to updated the
    //  information in a tab-page.  If pick letter changes, we assume
    //  all other vital-to-be-displayed information has also changed.
    //
    if (pickAtts->IsSelected(3) || pickAtts->IsSelected(0) || doAll)
    {
        UpdatePage();
    }

    //  If userSelectedVars changes, 
    //
    if (pickAtts->IsSelected(13) || doAll)
    {
        stringVector userVars = pickAtts->GetUserSelectedVars();
        std::string allVars2;
        for (int i = 0; i < userVars.size(); i++)
        {
           allVars2 += userVars[i];
           allVars2 += " ";
        }
        varsLineEdit->setText(allVars2.c_str()); 
    }
    if (pickAtts->IsSelected(15) || doAll)
    {
        useNodeCoords->blockSignals(true);
        useNodeCoords->setChecked(pickAtts->GetUseNodeCoords());
        logicalCoords->setEnabled(pickAtts->GetUseNodeCoords());
        useNodeCoords->blockSignals(false);
    }
    if (pickAtts->IsSelected(16) || doAll)
    {
        logicalCoords->blockSignals(true);
        logicalCoords->setChecked(pickAtts->GetLogicalCoords());
        logicalCoords->blockSignals(false);
    }
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
// ****************************************************************************

void
QvisPickWindow::UpdatePage()
{
    QString temp, temp2;
    static int nextPage = 0;
    static string lastLetter = " ";
    int nRows = 0;
    if (pickAtts->GetClearWindow())
    {
        nextPage = 0;
        ClearPages();
        return;
    }
    if (lastLetter != pickAtts->GetPickLetter() && pickAtts->GetFulfilled())
    {
        lastLetter = pickAtts->GetPickLetter();
        temp2.sprintf(" %s ", pickAtts->GetPickLetter().c_str());
        tabWidget->changeTab(pages[nextPage], temp2);

        string fileName;
        const string &dBaseName = pickAtts->GetDatabaseName();
        int pos = dBaseName.find_last_of('/');
        if (pos >= dBaseName.size())
            fileName = dBaseName;
        else
            fileName = dBaseName.substr(pos+1);

        infoLists[nextPage]->clear();
        infoLists[nextPage]->setColumnMode(1);
        infoLists[nextPage]->setVariableHeight(false);
        if (pickAtts->GetDomain() == -1)
        {
            temp2.sprintf("%s   timestep %d",
                fileName.c_str(), pickAtts->GetTimeStep());
        }
        else 
        {
            temp2.sprintf("%s   timestep %d  domain %d",
                fileName.c_str(), 
                pickAtts->GetTimeStep(),
                pickAtts->GetDomain());
        }
        infoLists[nextPage]->insertItem(temp2);
        nRows++;

        if (pickAtts->GetDimension() == 2)
        {
            if (!pickAtts->GetNeedTransformMessage())
            {
                temp2.sprintf("Point:  <%f, %f>",
                    pickAtts->GetCellPoint()[0],
                    pickAtts->GetCellPoint()[1]);
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
            }
            else 
            {
                temp2.sprintf("Point:  (in transformed space)");
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
                temp2.sprintf("        <%f, %f>",
                    pickAtts->GetCellPoint()[0],
                    pickAtts->GetCellPoint()[1]);
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
            }
        }
        else 
        {
            if (!pickAtts->GetNeedTransformMessage())
            {
                temp2.sprintf("Point:  <%f, %f, %f>",
                    pickAtts->GetCellPoint()[0],
                    pickAtts->GetCellPoint()[1],
                    pickAtts->GetCellPoint()[2]);
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
            }
            else 
            {
                temp2.sprintf("Point:  (in transformed space)");
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
                temp2.sprintf("        <%f, %f, %f>",
                    pickAtts->GetCellPoint()[0],
                    pickAtts->GetCellPoint()[1],
                    pickAtts->GetCellPoint()[2]);
                infoLists[nextPage]->insertItem(temp2);
                nRows++;
            }
        }

        temp2.sprintf("Zone:  %i", pickAtts->GetZoneNumber());
        infoLists[nextPage]->insertItem(temp2);
        nRows++;

        const intVector &nodes = pickAtts->GetNodes();
        const stringVector &nodeCoords = pickAtts->GetNodeCoords();
        temp.sprintf("Nodes:  ");
        bool useCoords = pickAtts->GetUseNodeCoords() && !nodeCoords.empty();
        if (useCoords)
        {
            infoLists[nextPage]->insertItem(temp);
            nRows++;
            temp.sprintf("     ");
        }
        int i;
        for (i = 0; i < nodes.size(); i++)
        {
            temp2.sprintf("%d  ", nodes[i]);
            temp += temp2;
            if (useCoords)
            {
                temp2.sprintf("%s", nodeCoords[i].c_str());
                temp += temp2;
                infoLists[nextPage]->insertItem(temp);
                nRows++;
                temp.sprintf("     ");
            }
        }
        if (!useCoords)
        {
            infoLists[nextPage]->insertItem(temp);
            nRows++;
        }

        //
        //  Displaying the vars in a list box. 
        //
        int numVars = pickAtts->GetNumPickVarInfos();
        for (i = 0; i < numVars; i++)
        {
            std::vector<std::string> vi;
            pickAtts->GetPickVarInfo(i).CreateOutputStrings(vi);
            for (int j = 0; j < vi.size(); j++)
            {
                infoLists[nextPage]->insertItem(vi[j].c_str());
                nRows++;
            }
            vi.clear();
        }
        
        infoLists[nextPage]->setRowMode(nRows);
        tabWidget->showPage(pages[nextPage]);
        nextPage = (nextPage + 1) % MAX_PICK_TABS;
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
    if (which_widget == 0 || doAll)
    {
        QString temp;
        stringVector userVars;
        temp = varsLineEdit->displayText().simplifyWhiteSpace();
        QStringList lst(QStringList::split(" ", temp));
 
        QStringList::Iterator it;
 
        for (it = lst.begin(); it != lst.end(); ++it)
        {
            userVars.push_back((*it).latin1());
        }
 
        pickAtts->SetUserSelectedVars(userVars);
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
// ****************************************************************************

void
QvisPickWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current pick attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        pickAtts->Notify();
    }
    else
    {
        pickAtts->Notify();
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

void
QvisPickWindow::variableProcessText()
{
    GetCurrentValues(0);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::ClearPages
//
// Purpose: 
//   This method clears all the pages on the tab widget to reflect a clean-slate.
//
// Programmer: Kathleen Bonnell
// Creation:   March 25, 2002 
//
// ****************************************************************************
void
QvisPickWindow::ClearPages()
{
    QString temp = " ";
    for (int i = 0; i < MAX_PICK_TABS; i++)
    {
        tabWidget->changeTab(pages[i], temp);
        infoLists[i]->clear();
    }
    tabWidget->showPage(pages[0]);
}

// ****************************************************************************
// Method: QvisPickWindow::useNodeCoordsToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not the node coordinates should be used. 
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 23, 2002 
//
// ****************************************************************************
 
void
QvisPickWindow::useNodeCoordsToggled(bool val)
{
    pickAtts->SetUseNodeCoords(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::logicalCoordsToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not logical node coordinates should be used. 
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 27, 2002 
//
// ****************************************************************************
 
void
QvisPickWindow::logicalCoordsToggled(bool val)
{
    pickAtts->SetLogicalCoords(val);
    Apply();
}
