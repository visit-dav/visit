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
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qstringlist.h>
#include <qvbox.h>

#include <QvisPickWindow.h>
#include <PickAttributes.h>
#include <ViewerProxy.h>
#include <DebugStream.h>
#include <PickVarInfo.h>
#include <DataNode.h>

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
//   Brad Whitlock, Wed Aug 27 08:36:19 PDT 2003
//   Added autoShow flag.
//
//   Brad Whitlock, Tue Sep 9 09:30:01 PDT 2003
//   I made nextPage and lastLetter be members.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003 
//   Added savePicks flag.
//
// ****************************************************************************

QvisPickWindow::QvisPickWindow(PickAttributes *subj, const char *caption, 
                             const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false),
    lastLetter(" ")
{
    pickAtts = subj;
    autoShow = true;
    nextPage = 0;
    savePicks = false;
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
//   Brad Whitlock, Wed Aug 27 08:38:55 PDT 2003
//   I added a checkbox to set autoShow mode.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003 
//   Added  savePicks checkbox.
//
//   Kathleen Bonnell, Tue Nov 18 14:03:22 PST 2003 
//   Added  logicalZone checkbox.
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
        infoLists[i]  = new QMultiLineEdit(pages[i], "infoList");
        infoLists[i]->setWordWrap(QMultiLineEdit::WidgetWidth);
        infoLists[i]->setReadOnly(true);
        tabWidget->addTab(pages[i]," "); 
    }

    QGridLayout *gLayout = new QGridLayout(topLayout, 5, 4);
    varsLineEdit = new QLineEdit(central, "varsLineEdit");
    varsLineEdit->setText("default"); 
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    gLayout->addMultiCellWidget(varsLineEdit, 0, 0, 1, 3);
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

    logicalZone = new QCheckBox("Logical Zone Coordinates", central, "logicalZone");
    connect(logicalZone, SIGNAL(toggled(bool)),
            this, SLOT(logicalZoneToggled(bool)));
    gLayout->addMultiCellWidget(logicalZone, 2, 2, 0, 3);

    autoShowCheckBox = new QCheckBox("Automatically show window", central,
                                     "autoShowCheckBox");
    connect(autoShowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(autoShowToggled(bool)));
    gLayout->addMultiCellWidget(autoShowCheckBox, 3, 3, 0, 3);

    savePicksCheckBox = new QCheckBox("Don't clear this window", central,
                                     "savePicksCheckBox");
    connect(savePicksCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(savePicksToggled(bool)));
    gLayout->addMultiCellWidget(savePicksCheckBox, 4, 4, 0, 3);
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
//   Brad Whitlock, Wed Aug 27 08:37:57 PDT 2003
//   Made the window show itself if autoShow mode is on.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003
//   Don't clear the window if savePicks is checked. 
//
//   Kathleen Bonnell, Tue Nov 18 14:03:22 PST 2003 
//   Update logicalZone. 
//
// ****************************************************************************

void
QvisPickWindow::UpdateWindow(bool doAll)
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
    //  If pick letter changes, it indicates we need to updated the
    //  information in a tab-page.  If pick letter changes, we assume
    //  all other vital-to-be-displayed information has also changed.
    //
    if (pickAtts->IsSelected(2) || pickAtts->IsSelected(0) || doAll)
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

    //  If userSelectedVars changes, 
    //
    if (pickAtts->IsSelected(12) || doAll)
    {
        stringVector userVars = pickAtts->GetVariables();
        std::string allVars2;
        for (int i = 0; i < userVars.size(); i++)
        {
           allVars2 += userVars[i];
           allVars2 += " ";
        }
        varsLineEdit->setText(allVars2.c_str()); 
    }
    if (pickAtts->IsSelected(14) || doAll)
    {
        useNodeCoords->blockSignals(true);
        useNodeCoords->setChecked(pickAtts->GetUseNodeCoords());
        logicalCoords->setEnabled(pickAtts->GetUseNodeCoords());
        useNodeCoords->blockSignals(false);
    }
    if (pickAtts->IsSelected(15) || doAll)
    {
        logicalCoords->blockSignals(true);
        logicalCoords->setChecked(pickAtts->GetLogicalCoords());
        logicalCoords->blockSignals(false);
    }
    if (pickAtts->IsSelected(26) || doAll)
    {
        logicalZone->blockSignals(true);
        logicalZone->setChecked(pickAtts->GetLogicalZone());
        logicalZone->blockSignals(false);
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
//   Kathleen Bonnell, Wed Jun 25 13:45:04 PDT 2003 
//   Reflect new naming convention in PickAttributes. Rework to support
//   nodePick.
//
//   Brad Whitlock, Tue Sep 9 09:07:17 PDT 2003
//   I made the infoLists be QMultiLineEdits instead of QListBoxes.
//
//   Kathleen Bonnell, Wed Sep 10 08:02:02 PDT 2003 
//   Don't clear window if savePicks is checked. 
//   Removed code that printed all the pickAtts info separately.  Code was here
//   because "\n" did not play well with QListBox.  Any time PickAttributes 
//   changed, this code had to be updated as well.  Now simply use 
//   pickAtts->CreateOutputString.
//   
// ****************************************************************************

void
QvisPickWindow::UpdatePage()
{
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
        tabWidget->changeTab(pages[nextPage], temp);

        //
        // Get the output string without the letter, as it is
        // displayed in the tab.
        //
        pickAtts->CreateOutputString(displayString, false);

        // Make the tab display the string.
        infoLists[nextPage]->clear();
        infoLists[nextPage]->insertLine(displayString.c_str());
        infoLists[nextPage]->setCursorPosition(0, 0);

        // Show the tab.
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
 
        pickAtts->SetVariables(userVars);
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
// ****************************************************************************

void
QvisPickWindow::CreateNode(DataNode *parentNode)
{
    // Add the base information.
    QvisPostableWindowObserver::CreateNode(parentNode);

    // Add more information.
    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(caption().latin1());
        if(node)
        {
            node->AddNode(new DataNode("autoShow", autoShow));
            node->AddNode(new DataNode("savePicks", savePicks));
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
// ****************************************************************************

void
QvisPickWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    QvisPostableWindowObserver::SetFromNode(parentNode, borders);

    DataNode *winNode = parentNode->GetNode(caption().latin1());
    if(winNode == 0)
        return;

    // Set the autoShow flag.
    DataNode *node;
    if((node = winNode->GetNode("autoShow")) != 0)
        autoShow = node->AsBool();
    if((node = winNode->GetNode("savePicks")) != 0)
        savePicks = node->AsBool();
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
        viewer->SetPickAttributes();
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


// ****************************************************************************
// Method: QvisPickWindow::logicalZoneToggled
//
// Purpose:
//   This is a Qt slot function that sets the flag indicating whether
//   or not logical zone coordinates should be used. 
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 18, 2003 
//
// ****************************************************************************

void
QvisPickWindow::logicalZoneToggled(bool val)
{
    pickAtts->SetLogicalZone(val);
    Apply();
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
//   This is a Qt slot function that sets the internal savePicks flag when the
//   savePicks checkbox is toggled.
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
