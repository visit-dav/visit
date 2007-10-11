/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h> // for sscanf
#include <string>
#include <vector>

#include <qcheckbox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qvbox.h>

#include <QvisVariableButton.h>
#include <QvisPickWindow.h>
#include <PickAttributes.h>
#include <ViewerProxy.h>
#include <DebugStream.h>
#include <PickVarInfo.h>
#include <DataNode.h>
#include <StringHelpers.h>

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
//   Kathleen Bonnell, Wed Dec 17 15:06:34 PST 2003 
//   Made the window have all buttons.  
//
//   Ellen Tarwater, Fri May 18, 2007
//   Added save Count for 'Save Pick as...' functionality.
//
// ****************************************************************************

QvisPickWindow::QvisPickWindow(PickAttributes *subj, const char *caption, 
                           const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad, 
                           QvisPostableWindowObserver::AllExtraButtons, false),
    lastLetter(" ")
{
    pickAtts = subj;
    autoShow = true;
    nextPage = 0;
    savePicks = false;
    saveCount = 0;
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
//   Added "Save Picks as..." button
//
//   Hank Childs, Thu Aug 30 14:13:43 PDT 2007
//   Added spreadsheetCheckBox.
//
//   Cyrus Harrison, Mon Sep 17 15:18:50 PDT 2007
//   Added floatFormat
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
        pages[i]->hide();
        infoLists[i]  = new QMultiLineEdit(pages[i], "infoList");
        infoLists[i]->setWordWrap(QMultiLineEdit::WidgetWidth);
        infoLists[i]->setReadOnly(true);
        if (i < MIN_PICK_TABS)
        {
            pages[i]->show();
            tabWidget->addTab(pages[i]," "); 
        }
    }
    

    QGridLayout *gLayout = new QGridLayout(topLayout, 13, 4);

    userMaxPickTabs = new QSpinBox(MIN_PICK_TABS, MAX_PICK_TABS, 1, central, "userMaxPickTabs");
    userMaxPickTabs->setButtonSymbols(QSpinBox::PlusMinus);
    gLayout->addWidget(userMaxPickTabs, 0, 1);
    gLayout->addWidget(new QLabel(userMaxPickTabs, "Max Tabs", central), 0,0);

    QPushButton *savePicksButton = new QPushButton("Save Picks as...", central,
            "savePicksButton");
    connect(savePicksButton, SIGNAL(clicked()),
            this, SLOT(savePickText()));
    gLayout->addWidget(savePicksButton, 0, 3);
    
    varsButton = new QvisVariableButton(true, false, true, -1,
        central, "varsButton");
    varsButton->setText("Variables");
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addPickVariable(const QString &)));
    gLayout->addWidget(varsButton, 1, 0);

    varsLineEdit = new QLineEdit(central, "varsLineEdit");
    varsLineEdit->setText("default"); 
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    gLayout->addMultiCellWidget(varsLineEdit, 1, 1, 1, 3);

    
    QLabel *floatFormatLabel = new QLabel("Float Format:",central,"floatFormatLabel");
    gLayout->addWidget(floatFormatLabel, 2, 0);
    
    floatFormatLineEdit= new QLineEdit(central, "floatFormatLineEdit");
    floatFormatLineEdit->setText("%g"); 
    gLayout->addMultiCellWidget(floatFormatLineEdit, 2, 2, 1, 3);
    connect(floatFormatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(floatFormatProcessText()));
        
    conciseOutputCheckBox = new QCheckBox("Concise Output.", central,
                                     "conciseOutputCheckBox");
    connect(conciseOutputCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(conciseOutputToggled(bool)));
    gLayout->addMultiCellWidget(conciseOutputCheckBox, 3, 3, 0, 1);


    showMeshNameCheckBox = new QCheckBox("Show Mesh Name", central, 
                                  "showMeshNameCheckBox");
    connect(showMeshNameCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showMeshNameToggled(bool)));
    gLayout->addMultiCellWidget(showMeshNameCheckBox, 4, 4, 0, 1);

    showTimestepCheckBox = new QCheckBox("Show Timestep", central, 
                                  "showTimestepCheckBox");
    connect(showTimestepCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showTimestepToggled(bool)));
    gLayout->addMultiCellWidget(showTimestepCheckBox, 4, 4, 2, 3);

    displayIncEls = new QCheckBox("Display incident nodes/zones.", central, 
                                  "displayIncEls");
    connect(displayIncEls, SIGNAL(toggled(bool)),
            this, SLOT(displayIncElsToggled(bool)));
    gLayout->addMultiCellWidget(displayIncEls, 5, 5, 0, 1);

    displayGlobalIds = new QCheckBox("Display global nodes/zones.", central, 
                                  "displayGlobalIds");
    connect(displayGlobalIds, SIGNAL(toggled(bool)),
            this, SLOT(displayGlobalIdsToggled(bool)));
    gLayout->addMultiCellWidget(displayGlobalIds, 6, 6, 0, 1);

    displayPickLetter = new QCheckBox("Display reference pick letter.", central, 
                                  "displayPickLetter");
    connect(displayPickLetter, SIGNAL(toggled(bool)),
            this, SLOT(displayPickLetterToggled(bool)));
    gLayout->addMultiCellWidget(displayPickLetter, 7, 7, 0, 1);


    // Node settings
    QGroupBox *nodeGroupBox = new QGroupBox(central, "nodeGroupBox");
    nodeGroupBox->setTitle("Display for Nodes:");
    nodeGroupBox->setMargin(10);
    gLayout->addMultiCellWidget(nodeGroupBox, 8, 8, 0, 3);
    QGridLayout *nLayout = new QGridLayout(nodeGroupBox, 3, 4);
    nLayout->setMargin(10);
    nLayout->setSpacing(10);
    nLayout->addRowSpacing(0, 15);

    nodeId = new QCheckBox("Id", nodeGroupBox, "nodeId");
    connect(nodeId, SIGNAL(toggled(bool)),
            this, SLOT(nodeIdToggled(bool)));
    nLayout->addMultiCellWidget(nodeId, 1, 1, 0, 1);
    nodePhysical = new QCheckBox("Physical Coords", nodeGroupBox, "nodePhysical");
    connect(nodePhysical, SIGNAL(toggled(bool)),
            this, SLOT(nodePhysicalToggled(bool)));
    nLayout->addMultiCellWidget(nodePhysical, 2, 2, 0, 1);
    nodeDomLog = new QCheckBox("Domain-Logical Coords", nodeGroupBox, "nodeDomLog");
    connect(nodeDomLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeDomLogToggled(bool)));
    nLayout->addMultiCellWidget(nodeDomLog, 1, 1, 2, 3);
    nodeBlockLog = new QCheckBox("Block-Logical Coords", nodeGroupBox, "nodeBlockLog");
    connect(nodeBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeBlockLogToggled(bool)));
    nLayout->addMultiCellWidget(nodeBlockLog, 2, 2, 2, 3);

    // Zone settings
    QGroupBox *zoneGroupBox = new QGroupBox(central, "zoneGroupBox");
    zoneGroupBox->setTitle("Display for Zones:");
    zoneGroupBox->setMargin(10);
    gLayout->addMultiCellWidget(zoneGroupBox, 9, 9, 0, 3);
    QGridLayout *zLayout = new QGridLayout(zoneGroupBox, 3, 4);
    zLayout->setMargin(10);
    zLayout->setSpacing(10);
    zLayout->addRowSpacing(0, 15);

    zoneId = new QCheckBox("Id", zoneGroupBox, "zoneId");
    connect(zoneId, SIGNAL(toggled(bool)),
            this, SLOT(zoneIdToggled(bool)));
    zLayout->addMultiCellWidget(zoneId, 1, 1, 0, 1);
    zoneDomLog = new QCheckBox("Domain-Logical Coords", zoneGroupBox, "zoneDomLog");
    connect(zoneDomLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneDomLogToggled(bool)));
    zLayout->addMultiCellWidget(zoneDomLog, 1, 1, 2, 3);
    zoneBlockLog = new QCheckBox("Block-Logical Coords", zoneGroupBox, "zoneBlockLog");
    connect(zoneBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneBlockLogToggled(bool)));
    zLayout->addMultiCellWidget(zoneBlockLog, 2, 2, 2, 3);

   
    autoShowCheckBox = new QCheckBox("Automatically show window", central,
                                     "autoShowCheckBox");
    connect(autoShowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(autoShowToggled(bool)));
    gLayout->addMultiCellWidget(autoShowCheckBox, 10, 10, 0, 3);

    savePicksCheckBox = new QCheckBox("Don't clear this window", central,
                                     "savePicksCheckBox");
    connect(savePicksCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(savePicksToggled(bool)));
    gLayout->addMultiCellWidget(savePicksCheckBox, 11, 11, 0, 3);

    timeCurveCheckBox = new QCheckBox("Create time curve with next pick.", central,
                                     "timeCurveCheckBox");
    connect(timeCurveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(timeCurveToggled(bool)));
    gLayout->addMultiCellWidget(timeCurveCheckBox, 12, 12, 0, 3);

    spreadsheetCheckBox = new QCheckBox("Create spreadsheet with next pick.", central,
                                     "spreadsheetCheckBox");
    connect(spreadsheetCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(spreadsheetToggled(bool)));
    gLayout->addMultiCellWidget(spreadsheetCheckBox, 13, 13, 0, 3);
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
    //  If pick letter changes, it indicates we need to update the
    //  information in a tab-page.  If pick letter changes, we assume
    //  all other vital-to-be-displayed information has also changed.
    //

    // 10 == pickLetter 9 == clearWindow
    if (pickAtts->IsSelected(10) || pickAtts->IsSelected(9) || doAll)
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
    if (pickAtts->IsSelected(0) || doAll)
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

    // displayIncidentElements
    if (pickAtts->IsSelected(1) || doAll)
    {
        displayIncEls->blockSignals(true);
        displayIncEls->setChecked(pickAtts->GetDisplayIncidentElements());
        displayIncEls->blockSignals(false);
 
   }
    // showNodeId
    if (pickAtts->IsSelected(2) || doAll)
    {
        nodeId->blockSignals(true);
        nodeId->setChecked(pickAtts->GetShowNodeId());
        nodeId->blockSignals(false);
    }

    // showNodeDomainLogicalCoords
    if (pickAtts->IsSelected(3) || doAll)
    {
        nodeDomLog->blockSignals(true);
        nodeDomLog->setChecked(pickAtts->GetShowNodeDomainLogicalCoords());
        nodeDomLog->blockSignals(false);
    }

    // showNodeBlockLogicalCoords
    if (pickAtts->IsSelected(4) || doAll)
    {
        nodeBlockLog->blockSignals(true);
        nodeBlockLog->setChecked(pickAtts->GetShowNodeBlockLogicalCoords());
        nodeBlockLog->blockSignals(false);
    }

    // showNodePhysicalLogicalCoords
    if (pickAtts->IsSelected(5) || doAll)
    {
        nodePhysical->blockSignals(true);
        nodePhysical->setChecked(pickAtts->GetShowNodePhysicalCoords());
        nodePhysical->blockSignals(false);
    }

    // showZoneId
    if (pickAtts->IsSelected(6) || doAll)
    {
        zoneId->blockSignals(true);
        zoneId->setChecked(pickAtts->GetShowZoneId());
        zoneId->blockSignals(false);
    }

    // showZoneDomainLogicalCoords
    if (pickAtts->IsSelected(7) || doAll)
    {
        zoneDomLog->blockSignals(true);
        zoneDomLog->setChecked(pickAtts->GetShowZoneDomainLogicalCoords());
        zoneDomLog->blockSignals(false);
    }

    // showZoneBlockLogicalCoords
    if (pickAtts->IsSelected(8) || doAll)
    {
        zoneBlockLog->blockSignals(true);
        zoneBlockLog->setChecked(pickAtts->GetShowZoneBlockLogicalCoords());
        zoneBlockLog->blockSignals(false);
    }

    // doTimeCurve
    if (pickAtts->IsSelected(37) || doAll)
    {
        timeCurveCheckBox->blockSignals(true);
        timeCurveCheckBox->setChecked(pickAtts->GetDoTimeCurve());
        timeCurveCheckBox->blockSignals(false);
    }

    // conciseOutput
    if (pickAtts->IsSelected(42) || doAll)
    {
        conciseOutputCheckBox->blockSignals(true);
        conciseOutputCheckBox->setChecked(pickAtts->GetConciseOutput());
        conciseOutputCheckBox->blockSignals(false);
    }

    // showMeshName
    if (pickAtts->IsSelected(43) || doAll)
    {
        showMeshNameCheckBox->blockSignals(true);
        showMeshNameCheckBox->setChecked(pickAtts->GetShowMeshName());
        showMeshNameCheckBox->blockSignals(false);
    }

    // showTimestep
    if (pickAtts->IsSelected(44) || doAll)
    {
        showTimestepCheckBox->blockSignals(true);
        showTimestepCheckBox->setChecked(pickAtts->GetShowTimeStep());
        showTimestepCheckBox->blockSignals(false);
    }

    // displayGlobalIds
    if (pickAtts->IsSelected(52) || doAll)
    {
        displayGlobalIds->blockSignals(true);
        displayGlobalIds->setChecked(pickAtts->GetDisplayGlobalIds());
        displayGlobalIds->blockSignals(false);
    }

    // displayPickLetter
    if (pickAtts->IsSelected(53) || doAll)
    {
        displayPickLetter->blockSignals(true);
        displayPickLetter->setChecked(pickAtts->GetDisplayPickLetter());
        displayPickLetter->blockSignals(false);
    }

    // createSpreadsheet
    if (pickAtts->IsSelected(62) || doAll)
    {
        spreadsheetCheckBox->blockSignals(true);
        spreadsheetCheckBox->setChecked(pickAtts->GetCreateSpreadsheet());
        spreadsheetCheckBox->blockSignals(false);
    }
    
    // floatFormat
    if (pickAtts->IsSelected(63) || doAll)
    {
        floatFormatLineEdit->blockSignals(true);
        floatFormatLineEdit->setText(pickAtts->GetFloatFormat().c_str());
        floatFormatLineEdit->blockSignals(false);
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
//   Kathleen Bonnell, Mon Oct 31 10:44:07 PST 2005 
//   Use value in userMaxPickTabs for setting nextPage instead of 
//   MAX_PICK_TABS. 
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
    if (which_widget == 1 || doAll)
    {
        string format = floatFormatLineEdit
                               ->displayText().simplifyWhiteSpace().latin1();
        if(!StringHelpers::ValidatePrintfFormatString(format.c_str(),
                                                      "float","EOA"))
            Error("Invalid pick floating point format string.");
        else
            pickAtts->SetFloatFormat(format);
    }

    if (doAll)
    {
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
            node->AddNode(new DataNode("userMaxTabs", userMaxPickTabs->value()));
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
    if((node = winNode->GetNode("userMaxTabs")) != 0)
    {
        userMaxPickTabs->setValue(node->AsInt());
        ResizeTabs();
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
    GetCurrentValues(0);
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
    GetCurrentValues(1);
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
    for (int i = 0; i < tabWidget->count(); i++)
    {
        tabWidget->changeTab(pages[i], temp);
        infoLists[i]->clear();
    }
    tabWidget->showPage(pages[0]);
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
// ****************************************************************************

void
QvisPickWindow::reset()
{
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
    pickAtts->SetDisplayIncidentElements(val);
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
//   
// ****************************************************************************

void
QvisPickWindow::timeCurveToggled(bool val)
{
    pickAtts->SetDoTimeCurve(val);
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
//   
// ****************************************************************************

void
QvisPickWindow::spreadsheetToggled(bool val)
{
    pickAtts->SetCreateSpreadsheet(val);
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
    pickAtts->SetDisplayGlobalIds(val);
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
    pickAtts->SetDisplayPickLetter(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPickWindow::ResizeTabs
//
// Purpose:
//   Resizes the number of tab the tabWidget hold based on the
//   current value in userMaxPickTabs. 
//
// Programmer: Kathleen Bonnell 
// Creation:   October 31, 2005 
//
// ****************************************************************************

void
QvisPickWindow::ResizeTabs()
{
    int currentMax = tabWidget->count();
    int newMax     = userMaxPickTabs->value();

    if (currentMax == newMax)
        return;

    int i;
    QString temp = " ";
    if (newMax < currentMax)
    {
        // Reduce the number of pages that tabWidget holds
        for (i = currentMax-1; i >= newMax; i--)
        {
            tabWidget->changeTab(pages[i], temp);
            infoLists[i]->clear();
            tabWidget->removePage(pages[i]);
            pages[i]->hide();
        }
        if (nextPage >= newMax)
            nextPage = 0;
    }
    else // newMax > currentMax 
    {
        // Increase the number of pages that tabWidget holds
        for (i = currentMax; i < newMax; i++)
        {
            pages[i]->show();
            tabWidget->addTab(pages[i]," "); 
        }
        if (tabWidget->label(nextPage) != " ")
            nextPage = currentMax; 
    }
}

// ****************************************************************************
// Method: QvisPickWindow::savePickText
//
// Purpose: 
//   This is a Qt slot function that saves the pick text in a user selected file.
//
// Programmer: Ellen Tarwater
// Creation:   Friday May 18 2007
//
// Modifications:
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
    
    QString currentDir;
    currentDir = QDir::current().path();
    
    defaultFile = currentDir + "/" + defaultFile;

    // Get the name of the file that the user saved.
    QString sFilter(QString("VisIt save (*") + saveExtension + ")");
    QString fileName = QFileDialog::getSaveFileName(defaultFile, sFilter);

    // If the user chose to save a file, write the pick result text
    // to that file.
    if(!fileName.isNull())
    {
        ++saveCount;
    QFile file( fileName );
    if ( file.open(IO_WriteOnly) )
    {
        QTextStream stream( &file );
        int i;
            for ( i = 0; i < tabWidget->count(); i++ )
            {
                QString txt( infoLists[i]->text() );
                if ( txt.length() > 0 )
                    stream << txt;
            }
   
        file.close();
    }
    else
        Error( "VisIt could not save the pick results"
               "to the selected file" ) ;


   }

}


