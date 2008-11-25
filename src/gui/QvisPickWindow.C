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
//   Katheen Bonnell, Thu Nov 29 15:35:15 PST 2007 
//   Added defaultAutoShow, defaultSavePicks, defaultNumTabs so that their
//   counterparts can be 'reset' correclty from user-saved values when needed. 
//
//   Brad Whitlock, Wed Apr  9 11:30:06 PDT 2008
//   QString for caption, shortName.
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
//   Added a missing tr() for "Clear Picks".
//
// ****************************************************************************

void
QvisPickWindow::CreateWindowContents()
{
    tabWidget = new QTabWidget(central);
    tabWidget->setMinimumHeight(200);
    topLayout->addWidget(tabWidget, 10);

    for (int i = 0; i < MAX_PICK_TABS; i++)
    {
        pages[i] = new QWidget(central);
        QVBoxLayout *vLayout = new QVBoxLayout(pages[i]);
        vLayout->setMargin(10);
        vLayout->setSpacing(5);
        pages[i]->hide();
        infoLists[i]  = new QTextEdit(pages[i]);
        vLayout->addWidget(infoLists[i]);
        infoLists[i]->setWordWrapMode(QTextOption::WordWrap);
        infoLists[i]->setReadOnly(true);
        if (i < MIN_PICK_TABS)
        {
            pages[i]->show();
            tabWidget->addTab(pages[i]," "); 
        }
    }
    
    QGridLayout *gLayout = new QGridLayout(0);
    topLayout->addLayout(gLayout);

    userMaxPickTabs = new QSpinBox(central);
    userMaxPickTabs->setMinimum(MIN_PICK_TABS);
    userMaxPickTabs->setMaximum(MAX_PICK_TABS);
    userMaxPickTabs->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    gLayout->addWidget(userMaxPickTabs, 0, 1);
    QLabel *userMaxPickTabsLabel = new QLabel(tr("Max Tabs"), central);
    userMaxPickTabsLabel->setBuddy(userMaxPickTabs);
    gLayout->addWidget(userMaxPickTabsLabel, 0,0);

    QPushButton *savePicksButton = new QPushButton(tr("Save Picks as") + QString("..."), central);
    connect(savePicksButton, SIGNAL(clicked()),
            this, SLOT(savePickText()));
    gLayout->addWidget(savePicksButton, 0, 2, 1, 2);

    varsButton = new QvisVariableButton(true, false, true, -1, central);
    varsButton->setText(tr("Variables"));
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addPickVariable(const QString &)));
    gLayout->addWidget(varsButton, 1, 0);

    varsLineEdit = new QLineEdit(central);
    varsLineEdit->setText("default"); 
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    gLayout->addWidget(varsLineEdit, 1, 1, 1, 3);

    
    QLabel *floatFormatLabel = new QLabel(tr("Float Format"),central);
    gLayout->addWidget(floatFormatLabel, 2, 0);
    
    floatFormatLineEdit= new QLineEdit(central);
    floatFormatLineEdit->setText("%g"); 
    gLayout->addWidget(floatFormatLineEdit, 2, 1, 1, 3);
    connect(floatFormatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(floatFormatProcessText()));
        
    conciseOutputCheckBox = new QCheckBox(tr("Concise Output"), central);
    connect(conciseOutputCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(conciseOutputToggled(bool)));
    gLayout->addWidget(conciseOutputCheckBox, 3, 0);


    showMeshNameCheckBox = new QCheckBox(tr("Show Mesh Name"), central);
    connect(showMeshNameCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showMeshNameToggled(bool)));
    gLayout->addWidget(showMeshNameCheckBox, 4, 0, 1, 2);

    showTimestepCheckBox = new QCheckBox(tr("Show Timestep"), central);
    connect(showTimestepCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showTimestepToggled(bool)));
    gLayout->addWidget(showTimestepCheckBox, 4, 2, 1, 2);

    displayIncEls = new QCheckBox(tr("Display incident nodes/zones"), central);
    connect(displayIncEls, SIGNAL(toggled(bool)),
            this, SLOT(displayIncElsToggled(bool)));
    gLayout->addWidget(displayIncEls, 5, 0, 1, 4);

    displayGlobalIds = new QCheckBox(tr("Display global nodes/zones"), central);
    connect(displayGlobalIds, SIGNAL(toggled(bool)),
            this, SLOT(displayGlobalIdsToggled(bool)));
    gLayout->addWidget(displayGlobalIds, 6, 0, 1, 4);

    displayPickLetter = new QCheckBox(tr("Display reference pick letter"), 
                                      central);
    connect(displayPickLetter, SIGNAL(toggled(bool)),
            this, SLOT(displayPickLetterToggled(bool)));
    gLayout->addWidget(displayPickLetter, 7, 0, 1, 4);


    // Node settings
    QGroupBox *nodeGroupBox = new QGroupBox(central);
    nodeGroupBox->setTitle(tr("Display for Nodes"));
    gLayout->addWidget(nodeGroupBox, 8, 0, 1, 4);
    QGridLayout *nLayout = new QGridLayout(nodeGroupBox);
    nLayout->setMargin(10);
    nLayout->setSpacing(10);

    nodeId = new QCheckBox(tr("Id"), nodeGroupBox);
    connect(nodeId, SIGNAL(toggled(bool)),
            this, SLOT(nodeIdToggled(bool)));
    nLayout->addWidget(nodeId, 0, 0);
    nodePhysical = new QCheckBox(tr("Physical Coords"), nodeGroupBox);
    connect(nodePhysical, SIGNAL(toggled(bool)),
            this, SLOT(nodePhysicalToggled(bool)));
    nLayout->addWidget(nodePhysical, 1, 0);
    nodeDomLog = new QCheckBox(tr("Domain-Logical Coords"), nodeGroupBox);
    connect(nodeDomLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeDomLogToggled(bool)));
    nLayout->addWidget(nodeDomLog, 0, 1);
    nodeBlockLog = new QCheckBox(tr("Block-Logical Coords"), nodeGroupBox);
    connect(nodeBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(nodeBlockLogToggled(bool)));
    nLayout->addWidget(nodeBlockLog, 1, 1);

    // Zone settings
    QGroupBox *zoneGroupBox = new QGroupBox(central);
    zoneGroupBox->setTitle(tr("Display for Zones"));
    gLayout->addWidget(zoneGroupBox, 9, 0, 1, 4);
    QGridLayout *zLayout = new QGridLayout(zoneGroupBox);
    zLayout->setMargin(10);
    zLayout->setSpacing(10);

    zoneId = new QCheckBox(tr("Id"), zoneGroupBox);
    connect(zoneId, SIGNAL(toggled(bool)),
            this, SLOT(zoneIdToggled(bool)));
    zLayout->addWidget(zoneId, 0, 0);
    zoneDomLog = new QCheckBox(tr("Domain-Logical Coords"), zoneGroupBox);
    connect(zoneDomLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneDomLogToggled(bool)));
    zLayout->addWidget(zoneDomLog, 0, 1);
    zoneBlockLog = new QCheckBox(tr("Block-Logical Coords"), zoneGroupBox);
    connect(zoneBlockLog, SIGNAL(toggled(bool)),
            this, SLOT(zoneBlockLogToggled(bool)));
    zLayout->addWidget(zoneBlockLog, 1, 1);

   
    autoShowCheckBox = new QCheckBox(tr("Automatically show window"), central);
    connect(autoShowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(autoShowToggled(bool)));
    gLayout->addWidget(autoShowCheckBox, 10, 0, 1, 4);

    savePicksCheckBox = new QCheckBox(tr("Don't clear this window"), central);
    connect(savePicksCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(savePicksToggled(bool)));
    gLayout->addWidget(savePicksCheckBox, 11, 0, 1, 2);

    QPushButton *clearPicksButton = new QPushButton(tr("Clear Picks"), central);
    connect(clearPicksButton, SIGNAL(clicked()),
            this, SLOT(clearPicks()));
    gLayout->addWidget(clearPicksButton, 11, 2, 1, 2);

    timeCurveCheckBox = new QCheckBox(tr("Create time curve with next pick"), 
                                      central);
    connect(timeCurveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(timeCurveToggled(bool)));
    gLayout->addWidget(timeCurveCheckBox, 12, 0, 1, 2);

    QPushButton *redoPickButton = new QPushButton(tr("Repeat Pick"), central);
    connect(redoPickButton, SIGNAL(clicked()),
            this, SLOT(redoPickClicked()));
    gLayout->addWidget(redoPickButton, 12, 2, 1, 2);

    preserveCoord= new QComboBox(central);
    preserveCoord->addItem(tr("Time curve use picked element"));
    preserveCoord->addItem(tr("Time curve use picked coordinates"));
    preserveCoord->setCurrentIndex(0);
    connect(preserveCoord, SIGNAL(activated(int)),
            this, SLOT(preserveCoordActivated(int)));
    gLayout->addWidget(preserveCoord, 13, 0, 1, 4);

    spreadsheetCheckBox = new QCheckBox(tr("Create spreadsheet with next pick"), 
                                        central);
    connect(spreadsheetCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(spreadsheetToggled(bool)));
    gLayout->addWidget(spreadsheetCheckBox, 14, 0, 1, 2);

    QPushButton *redoPickWithSpreadsheetButton =
        new QPushButton(tr("Display in Spreadsheet"), central);
    connect(redoPickWithSpreadsheetButton, SIGNAL(clicked()),
            this, SLOT(redoPickWithSpreadsheetClicked()));
    gLayout->addWidget(redoPickWithSpreadsheetButton, 14, 2, 1, 2);
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
//   Kathleen Bonnell, Fri Nov  9 14:00:27 PST 2007 
//   Added timePreserveCoord and fixed some select numbers to match atts. 
//
//   Brad Whitlock, Mon Dec 17 09:33:48 PST 2007
//   Made it use ids.
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
        for (int i = 0; i < userVars.size(); i++)
        {
           allVars2 += userVars[i];
           allVars2 += " ";
        }
        varsLineEdit->setText(allVars2.c_str()); 
    }

    // displayIncidentElements
    if (pickAtts->IsSelected(PickAttributes::ID_displayIncidentElements) || doAll)
    {
        displayIncEls->blockSignals(true);
        displayIncEls->setChecked(pickAtts->GetDisplayIncidentElements());
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

    // showNodePhysical Coords
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
        timeCurveCheckBox->blockSignals(true);
        timeCurveCheckBox->setChecked(pickAtts->GetDoTimeCurve());
        preserveCoord->setEnabled(pickAtts->GetDoTimeCurve());
        timeCurveCheckBox->blockSignals(false);
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
    if (pickAtts->IsSelected(PickAttributes::ID_displayGlobalIds) || doAll)
    {
        displayGlobalIds->blockSignals(true);
        displayGlobalIds->setChecked(pickAtts->GetDisplayGlobalIds());
        displayGlobalIds->blockSignals(false);
    }

    // displayPickLetter
    if (pickAtts->IsSelected(PickAttributes::ID_displayPickLetter) || doAll)
    {
        displayPickLetter->blockSignals(true);
        displayPickLetter->setChecked(pickAtts->GetDisplayPickLetter());
        displayPickLetter->blockSignals(false);
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
        tabWidget->setTabText(nextPage, temp);

        //
        // Get the output string without the letter, as it is
        // displayed in the tab.
        //
        pickAtts->CreateOutputString(displayString, false);

        // Make the tab display the string.
        infoLists[nextPage]->clear();
        infoLists[nextPage]->setText(displayString.c_str());

        // Show the tab.
        tabWidget->setCurrentIndex(nextPage);
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
// ****************************************************************************

void
QvisPickWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    QvisPostableWindowObserver::SetFromNode(parentNode, borders);

    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

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
//   This method clears all the pages on the tab widget to reflect a clean-slate.
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
    for (int i = 0; i < tabWidget->count(); i++)
    {
        tabWidget->setTabText(i, temp);
        infoLists[i]->clear();
    }
    tabWidget->setCurrentIndex(0);
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
// Method: QvisPickWindow::preserveCoordToggled
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
// Method: QvisPickWindow::ResizeTabs
//
// Purpose:
//   Resizes the number of tab the tabWidget hold based on the
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
            tabWidget->setTabText(i, temp);
            infoLists[i]->clear();
            tabWidget->removeTab(i);
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
        if (tabWidget->tabText(nextPage) != " ")
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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 14:59:53 PDT 2008
//   Qt 4.
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
            int i;
            for ( i = 0; i < tabWidget->count(); i++ )
            {
                QString txt( infoLists[i]->toPlainText() );
                if ( txt.length() > 0 )
                    stream << txt;
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
// Method: QvisPickWindow::redoPickClicked()
//
// Purpose: 
//   This is a Qt slot function that repeats the last pick.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug 15 10:52:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void QvisPickWindow::redoPickClicked()
{
    // Save old state
    createSpreadsheetSave = pickAtts->GetCreateSpreadsheet();
    displayPickLetterSave = pickAtts->GetDisplayPickLetter();
    reusePickLetterSave = pickAtts->GetReusePickLetter();

    // Do not display a new pick letter, do not advance pick letter, create
    // a spreadsheet
    pickAtts->SetDisplayPickLetter(false);
    pickAtts->SetReusePickLetter(true);
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

void QvisPickWindow::redoPickWithSpreadsheetClicked()
{
    // Save old state
    createSpreadsheetSave = pickAtts->GetCreateSpreadsheet();
    displayPickLetterSave = pickAtts->GetDisplayPickLetter();
    reusePickLetterSave = pickAtts->GetReusePickLetter();

    // Do not display a new pick letter, do not advance pick letter, create
    // a spreadsheet
    pickAtts->SetCreateSpreadsheet(true);
    pickAtts->SetDisplayPickLetter(false);
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
//   
// ****************************************************************************

void
QvisPickWindow::redoPick()
{
    if (pickAtts->GetPickType() == PickAttributes::Zone)
        GetViewerMethods()->PointQuery("Pick", pickAtts->GetPickPoint(), pickAtts->GetVariables());
    else if (pickAtts->GetPickType() == PickAttributes::Node)
        GetViewerMethods()->PointQuery("NodePick", pickAtts->GetPickPoint(),  pickAtts->GetVariables());

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
    pickAtts->SetDisplayPickLetter(displayPickLetterSave);
    pickAtts->SetReusePickLetter(reusePickLetterSave);
    pickAtts->Notify();
    GetViewerMethods()->SetPickAttributes();
}
