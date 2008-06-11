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

#include "QvisPreferencesWindow.h"

#include <GlobalAttributes.h>
#include <ViewerProxy.h>
#include <FileServerList.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// ****************************************************************************
// Method: QvisPreferencesWindow::QvisPreferencesWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added tsFormat and made the window stretch.
//
//   Brad Whitlock, Fri Jan 30 14:16:18 PST 2004
//   Added showSelFiles.
//
//   Brad Whitlock, Fri Apr 9 14:14:09 PST 2004
//   Added allowFileSelectionChangeToggle.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Attach the global attributes.
// 
//   Cyrus Harrison, Thu Nov 29 16:21:48 PST 2007
//   No need to attach the global atts, they are already attached
//   in a parent class.  Also removed the Apply button.
//
//   Brad Whitlock, Wed Apr  9 11:54:33 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisPreferencesWindow::QvisPreferencesWindow(
                       GlobalAttributes *subj,
                       const QString &caption,
                       const QString &shortName,
                       QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad,
                       QvisPostableWindowObserver::NoExtraButtons, true),
    tsFormat()
{
    atts = subj;
    timeStateDisplayMode = 0;
    showSelFiles = true;
    selectedFilesToggle = 0;
    allowFileSelChange = true;
    allowFileSelectionChangeToggle = 0;
}


// ****************************************************************************
// Method: QvisPreferencesWindow::~QvisPreferencesWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added timeStateDisplayMode.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Detach from  the global attributes.
// 
// ****************************************************************************

QvisPreferencesWindow::~QvisPreferencesWindow()
{
    delete timeStateDisplayMode;
    if (atts)
        atts->Detach(this);
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:45:16 PST 2003
//   I added a toggle for posting windows when showing them.
//
//   Brad Whitlock, Mon Oct 13 16:54:32 PST 2003
//   Added radio buttons for changing the timestate display mode.
//
//   Brad Whitlock, Fri Jan 30 14:16:40 PST 2004
//   I added a toggle button for showing the selected files.
//
//   Brad Whitlock, Fri Apr 9 14:14:22 PST 2004
//   I added a toggle button for highlighting the selected files. I also
//   turned the group box from "Time formatting" to "File panel properties".
//
//   Brad Whitlock, Fri Aug 6 09:21:57 PDT 2004
//   I added toggles for makeDefaultConfirm and automaticallyApplyOperator.
//
//   Mark C. Miller, Wed Jun  1 11:12:25 PDT 2005
//   I added toggles for setTryHarderCyclesTimes
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added toggles for createMeshQuality, createTimeDerivative.
// 
//   Cyrus Harrison, Wed Nov 28 13:31:30 PST 2007
//   Added creation of createVectorMagnitudeToggle check box
//
//   Brad Whitlock, Thu Jan 24 11:23:13 PDT 2008
//   Added newPlotsInheritSILRestrictionToggle, grouped database options.
//
//   Brad Whitlock, Thu Jan 31 10:22:59 PST 2008
//   Added session file options.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//   Added support for ignoring bad extents from dbs. 
// ****************************************************************************

void
QvisPreferencesWindow::CreateWindowContents()
{
    cloneWindowOnFirstRefToggle =
        new QCheckBox(tr("Clone window on first reference"),
                      central, "cloneWindowOnFirstRefToggle");
    connect(cloneWindowOnFirstRefToggle, SIGNAL(toggled(bool)),
            this, SLOT(cloneWindowOnFirstRefToggled(bool)));
    topLayout->addWidget(cloneWindowOnFirstRefToggle);

    postWindowsWhenShownToggle =
        new QCheckBox(tr("Post windows when shown"),
                      central, "postWindowsWhenShownToggle");
    connect(postWindowsWhenShownToggle, SIGNAL(toggled(bool)),
            this, SLOT(postWindowsWhenShownToggled(bool)));
    topLayout->addWidget(postWindowsWhenShownToggle);

    makeDefaultConfirmToggle =
        new QCheckBox(tr("Prompt before setting default attributes"),
                      central, "makeDefaultConfirmToggle");
    connect(makeDefaultConfirmToggle, SIGNAL(toggled(bool)),
            this, SLOT(makeDefaultConfirmToggled(bool)));
    topLayout->addWidget(makeDefaultConfirmToggle);

    automaticallyApplyOperatorToggle =
        new QCheckBox(tr("Prompt before applying new operator"),
                      central, "automaticallyApplyOperatorToggle");
    connect(automaticallyApplyOperatorToggle, SIGNAL(toggled(bool)),
            this, SLOT(automaticallyApplyOperatorToggled(bool)));
    topLayout->addWidget(automaticallyApplyOperatorToggle);

    newPlotsInheritSILRestrictionToggle =
        new QCheckBox(tr("New plots inherit SIL restriction"),
                      central, "newPlotsInheritSILRestrictionToggle");
    connect(newPlotsInheritSILRestrictionToggle, SIGNAL(toggled(bool)),
            this, SLOT(newPlotsInheritSILRestrictionToggled(bool)));
    topLayout->addWidget(newPlotsInheritSILRestrictionToggle);

    //
    // Create group box for database controls.
    //
    QGroupBox *dbControlsGroup = new QGroupBox(central, "dbControlsGroup");
    dbControlsGroup->setTitle(tr("Databases"));
    topLayout->addWidget(dbControlsGroup, 5);
    QVBoxLayout *dbInnerTopLayout = new QVBoxLayout(dbControlsGroup);
    dbInnerTopLayout->setMargin(10);
    dbInnerTopLayout->addSpacing(15);
    dbInnerTopLayout->setSpacing(10);
    QVBoxLayout *dbOptionsLayout = new QVBoxLayout(dbInnerTopLayout);
    dbOptionsLayout->setSpacing(5);

    tryHarderCyclesTimesToggle =
        new QCheckBox(tr("Try harder to get accurate cycles/times"),
                      dbControlsGroup, "tryHarderCyclesTimesToggle");
    connect(tryHarderCyclesTimesToggle, SIGNAL(toggled(bool)),
            this, SLOT(tryHarderCyclesTimesToggled(bool)));
    dbOptionsLayout->addWidget(tryHarderCyclesTimesToggle);

    ignoreDbExtentsToggle =
        new QCheckBox(tr("Ignore database extents (may degrade performance)"),
                      dbControlsGroup, "ignoreDbExtentsToggle");
    connect(ignoreDbExtentsToggle, SIGNAL(toggled(bool)),
            this, SLOT(ignoreDbExtentsToggled(bool)));
    dbOptionsLayout->addWidget(ignoreDbExtentsToggle);

    treatAllDBsAsTimeVaryingToggle =
        new QCheckBox(tr("Treat all databases as time-varying"),
                      dbControlsGroup, "treatAllDBsAsTimeVaryingToggle");
    connect(treatAllDBsAsTimeVaryingToggle, SIGNAL(toggled(bool)),
            this, SLOT(treatAllDBsAsTimeVaryingToggled(bool)));
    dbOptionsLayout->addWidget(treatAllDBsAsTimeVaryingToggle);

    createMeshQualityToggle =
        new QCheckBox(tr("Automatically create mesh quality expressions"),
                      dbControlsGroup, "createMeshQualityToggle");
    connect(createMeshQualityToggle, SIGNAL(toggled(bool)),
            this, SLOT(createMeshQualityToggled(bool)));
    dbOptionsLayout->addWidget(createMeshQualityToggle);

    createTimeDerivativeToggle =
        new QCheckBox(tr("Automatically create time derivative expressions"),
                      dbControlsGroup, "createTimeDerivativeToggle");
    connect(createTimeDerivativeToggle, SIGNAL(toggled(bool)),
            this, SLOT(createTimeDerivativeToggled(bool)));
    dbOptionsLayout->addWidget(createTimeDerivativeToggle);
    
    createVectorMagnitudeToggle =
        new QCheckBox(tr("Automatically create vector magnitude expressions"),
                      dbControlsGroup, "createVectorMagnitudeToggle ");
    connect(createVectorMagnitudeToggle, SIGNAL(toggled(bool)),
            this, SLOT(createVectorMagnitudeToggled(bool)));
    dbOptionsLayout->addWidget(createVectorMagnitudeToggle);

    //
    // Create group box for session file controls.
    //
    QGroupBox *sessionControlsGroup = new QGroupBox(central, "sessionControlsGroup");
    sessionControlsGroup->setTitle(tr("Session files"));
    topLayout->addWidget(sessionControlsGroup, 5);
    QVBoxLayout *sessionInnerTopLayout = new QVBoxLayout(sessionControlsGroup);
    sessionInnerTopLayout->setMargin(10);
    sessionInnerTopLayout->addSpacing(15);
    sessionInnerTopLayout->setSpacing(10);
    QVBoxLayout *sessionOptionsLayout = new QVBoxLayout(sessionInnerTopLayout);
    sessionOptionsLayout->setSpacing(5);

    userDirForSessionFilesToggle =
        new QCheckBox(tr("User directory is default location for session files"),
                      sessionControlsGroup, "userDirForSessionFilesToggle");
    connect(userDirForSessionFilesToggle, SIGNAL(toggled(bool)),
            this, SLOT(userDirForSessionFilesToggled(bool)));
    sessionOptionsLayout->addWidget(userDirForSessionFilesToggle);

    saveCrashRecoveryFileToggle =
        new QCheckBox(tr("Periodically save a crash recovery file"),
                      sessionControlsGroup, "saveCrashRecoveryFileToggle");
    connect(saveCrashRecoveryFileToggle, SIGNAL(toggled(bool)),
            this, SLOT(saveCrashRecoveryFileToggled(bool)));
    sessionOptionsLayout->addWidget(saveCrashRecoveryFileToggle);

    //
    // Create group box for time controls.
    //
    QGroupBox *filePanelControlsGroup = new QGroupBox(central, "filePanelControlsGroup");
    filePanelControlsGroup->setTitle(tr("File panel properties"));
    topLayout->addWidget(filePanelControlsGroup, 5);
    QVBoxLayout *innerTopLayout = new QVBoxLayout(filePanelControlsGroup);
    innerTopLayout->setMargin(10);
    innerTopLayout->addSpacing(15);
    innerTopLayout->setSpacing(10);
    QGridLayout *tsModeLayout = new QGridLayout(innerTopLayout, 5, 3);
    tsModeLayout->setSpacing(5);

    //
    // Widgets that let you control the file panel.
    //
    selectedFilesToggle = new QCheckBox(tr("Show selected files"),
        filePanelControlsGroup, "selectedFilesToggle");
    selectedFilesToggle->setChecked(showSelFiles);
    connect(selectedFilesToggle, SIGNAL(toggled(bool)),
            this, SLOT(selectedFilesToggled(bool)));
    tsModeLayout->addMultiCellWidget(selectedFilesToggle, 0, 0, 0, 3);

    allowFileSelectionChangeToggle = new QCheckBox(
        tr("Automatically highlight open file"), filePanelControlsGroup,
        "allowFileSelectionChangeToggle");
    allowFileSelectionChangeToggle->setChecked(allowFileSelChange);
    connect(allowFileSelectionChangeToggle, SIGNAL(toggled(bool)),
            this, SLOT(allowFileSelectionChangeToggled(bool)));
    tsModeLayout->addMultiCellWidget(allowFileSelectionChangeToggle,
        1, 1, 0, 3);

    //
    // Create radio button controls to let us change the timestate display mode.
    //
    tsModeLayout->addMultiCellWidget(new QLabel(tr("Display time using:"),
        filePanelControlsGroup), 2, 2, 0, 2);
    timeStateDisplayMode = new QButtonGroup(0, "timeStateDisplayMode");
    QRadioButton *rb = new QRadioButton(tr("Cycles"), filePanelControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 3, 0);
    rb = new QRadioButton(tr("Times"), filePanelControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 3, 1);
    rb = new QRadioButton(tr("Cycles and times"), filePanelControlsGroup);
    timeStateDisplayMode->insert(rb);
    tsModeLayout->addWidget(rb, 3, 2);
    timeStateDisplayMode->setButton(int(tsFormat.GetDisplayMode()));
    connect(timeStateDisplayMode, SIGNAL(clicked(int)),
            this, SLOT(handleTimeStateDisplayModeChange(int)));

    //
    // Create widgets that let you set the time format.
    //
    tsModeLayout->addMultiCellWidget(
        new QLabel(tr("Number of significant digits"), filePanelControlsGroup),
        4, 4, 0, 1);
    timeStateNDigits = new QSpinBox(1, 16, 1, filePanelControlsGroup, "timeStateNDigits");
    timeStateNDigits->setValue(tsFormat.GetPrecision());
    connect(timeStateNDigits, SIGNAL(valueChanged(int)),
            this, SLOT(timeStateNDigitsChanged(int)));
    tsModeLayout->addWidget(timeStateNDigits, 4, 2);

    topLayout->addStretch(100);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::Update
//
// Purpose: 
//   This method is called when the GlobalAttributes that this window
//   is updated.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 9, 2007 
//
// Modifications:
//
//   Cyrus Harrison, Wed Nov 28 13:28:47 PST 2007
//   Added support for flag for creating vector magnitude expressions
//
//   Cyrus Harrison, 
//   Fixed missing call to parent class Update method. This was preventing
//   UpdateWindow from being called  - making the window out of sync with
//   the app values.
//
//   Brad Whitlock, Fri Dec 14 14:02:26 PST 2007
//   Make sure the SetTreatAllDBsAsTimeVarying flag gets set in the file server.
//
// ****************************************************************************

void
QvisPreferencesWindow::Update(Subject *TheChangedSubject)
{
    QvisPostableWindowSimpleObserver::Update(TheChangedSubject);

    if (atts == 0)
        return;

    if (TheChangedSubject == atts)
    {
        if(atts->IsSelected(GlobalAttributes::ID_treatAllDBsAsTimeVarying))
            fileServer->SetTreatAllDBsAsTimeVarying(
                atts->GetTreatAllDBsAsTimeVarying());
        if(atts->IsSelected(GlobalAttributes::ID_createMeshQualityExpressions))
            fileServer->SetCreateMeshQualityExpressions(
                atts->GetCreateMeshQualityExpressions());
        if(atts->IsSelected(GlobalAttributes::ID_createTimeDerivativeExpressions))
            fileServer->SetCreateTimeDerivativeExpressions(
                atts->GetCreateTimeDerivativeExpressions());
        if(atts->IsSelected(GlobalAttributes::ID_createVectorMagnitudeExpressions))
            fileServer->SetCreateVectorMagnitudeExpressions(
                atts->GetCreateVectorMagnitudeExpressions());
    }
}

// ****************************************************************************
// Method: QvisPreferencesWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:15:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:46:50 PST 2003
//   I added a toggle button for posting windows when they are shown.
//
//   Brad Whitlock, Fri Jan 30 14:19:11 PST 2004
//   I added a toggle for showing the selected files.
//
//   Brad Whitlock, Fri Apr 9 14:22:34 PST 2004
//   I added a toggle for automatically highlighting the open file.
//
//   Brad Whitlock, Fri Aug 6 09:21:57 PDT 2004
//   I added toggles for makeDefaultConfirm and automaticallyApplyOperator.
//
//   Mark C. Miller, Wed Jun  1 11:12:25 PDT 2005
//   I added toggles for setTryHarderCyclesTimes
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added toggles for createMeshQuality, createTimeDerivative.
// 
//   Cyrus Harrison, Wed Nov 28 13:28:47 PST 2007
//   Added toggle for createVectorMagnitude
//
//   Brad Whitlock, Fri Dec 14 15:53:39 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jan 24 11:30:29 PDT 2008
//   Set the inherit SIL restriction toggle.
//
//   Brad Whitlock, Thu Jan 31 10:35:13 PST 2008
//   Added userDirForSessionFiles, saveCrashRecoveryFile.
//
//   Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//   Added logic for ignoring bad extents from dbs.
// ****************************************************************************

void
QvisPreferencesWindow::UpdateWindow(bool doAll)
{
    if (doAll || atts->IsSelected(GlobalAttributes::ID_makeDefaultConfirm))
    {
        //
        // Prompt before making default attributes.
        //
        makeDefaultConfirmToggle->blockSignals(true);
        makeDefaultConfirmToggle->setChecked(
            atts->GetMakeDefaultConfirm());
        makeDefaultConfirmToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_cloneWindowOnFirstRef))
    {
        //
        // Clone window on first reference
        //
        cloneWindowOnFirstRefToggle->blockSignals(true);
        cloneWindowOnFirstRefToggle->setChecked(
            atts->GetCloneWindowOnFirstRef());
        cloneWindowOnFirstRefToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_automaticallyAddOperator))
    {
        //
        // Automatically add operator.
        //
        automaticallyApplyOperatorToggle->blockSignals(true);
        automaticallyApplyOperatorToggle->setChecked(
            !atts->GetAutomaticallyAddOperator());
        automaticallyApplyOperatorToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_newPlotsInheritSILRestriction))
    {
        //
        // New plots inherit SIL restriction
        //
        newPlotsInheritSILRestrictionToggle->blockSignals(true);
        newPlotsInheritSILRestrictionToggle->setChecked(
            atts->GetNewPlotsInheritSILRestriction());
        newPlotsInheritSILRestrictionToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_tryHarderCyclesTimes))
    {
        //
        // Try harder to get accurate cycles and times 
        //
        tryHarderCyclesTimesToggle->blockSignals(true);
        tryHarderCyclesTimesToggle->setChecked(
            atts->GetTryHarderCyclesTimes());
        tryHarderCyclesTimesToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_ignoreExtentsFromDbs))
    {
        //
        // Try harder to get accurate cycles and times 
        //
        ignoreDbExtentsToggle->blockSignals(true);
        ignoreDbExtentsToggle->setChecked(
            atts->GetIgnoreExtentsFromDbs());
        ignoreDbExtentsToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_treatAllDBsAsTimeVarying))
    {
        treatAllDBsAsTimeVaryingToggle->blockSignals(true);
        treatAllDBsAsTimeVaryingToggle->setChecked(
            atts->GetTreatAllDBsAsTimeVarying());
        treatAllDBsAsTimeVaryingToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_createMeshQualityExpressions))
    {
        createMeshQualityToggle->blockSignals(true);
        createMeshQualityToggle->setChecked(
            atts->GetCreateMeshQualityExpressions());
        createMeshQualityToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_createTimeDerivativeExpressions))
    {
        createTimeDerivativeToggle->blockSignals(true);
        createTimeDerivativeToggle->setChecked(
            atts->GetCreateTimeDerivativeExpressions());
        createTimeDerivativeToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_createVectorMagnitudeExpressions))
    {
        createVectorMagnitudeToggle->blockSignals(true);
        createVectorMagnitudeToggle->setChecked(
            atts->GetCreateVectorMagnitudeExpressions());
        createVectorMagnitudeToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_userDirForSessionFiles))
    {
        userDirForSessionFilesToggle->blockSignals(true);
        userDirForSessionFilesToggle->setChecked(
            atts->GetUserDirForSessionFiles());
        userDirForSessionFilesToggle->blockSignals(false);
    }

    if (doAll || atts->IsSelected(GlobalAttributes::ID_saveCrashRecoveryFile))
    {
        saveCrashRecoveryFileToggle->blockSignals(true);
        saveCrashRecoveryFileToggle->setChecked(
            atts->GetSaveCrashRecoveryFile());
        saveCrashRecoveryFileToggle->blockSignals(false);
    }

    if(doAll)
    {
        postWindowsWhenShownToggle->blockSignals(true);
        postWindowsWhenShownToggle->setChecked(postWhenShown);
        postWindowsWhenShownToggle->blockSignals(false);

        selectedFilesToggle->blockSignals(true);
        selectedFilesToggle->setChecked(showSelFiles);
        selectedFilesToggle->blockSignals(false);

        selectedFilesToggle->blockSignals(true);
        selectedFilesToggle->setChecked(showSelFiles);
        selectedFilesToggle->blockSignals(false);

        allowFileSelectionChangeToggle->blockSignals(true);
        allowFileSelectionChangeToggle->setChecked(allowFileSelChange);
        allowFileSelectionChangeToggle->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPreferencesWindow::SetTimeStateDisplayMode
//
// Purpose: 
//   Sets the timeDisplayMode toggle.
//
// Arguments:
//   mode : The new display mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:05:17 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::SetTimeStateFormat(const TimeFormat &fmt)
{
    tsFormat = fmt;
    if(timeStateDisplayMode != 0 && timeStateNDigits != 0)
    {
        timeStateDisplayMode->blockSignals(true);
        timeStateDisplayMode->setButton(int(tsFormat.GetDisplayMode()));
        timeStateDisplayMode->blockSignals(false);

        timeStateNDigits->blockSignals(true);
        timeStateNDigits->setValue(tsFormat.GetPrecision());
        timeStateNDigits->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPreferencesWindow::SetShowSelectedFiles
//
// Purpose: 
//   This method sets the toggle for the selected files.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:40:03 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::SetShowSelectedFiles(bool val)
{
    showSelFiles = val;

    if(selectedFilesToggle != 0)
    {
        selectedFilesToggle->blockSignals(true);
        selectedFilesToggle->setChecked(showSelFiles);
        selectedFilesToggle->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPreferencesWindow::SetAllowFileSelectionChange
//
// Purpose: 
//   This method sets the toggle for the automatic file highlighting.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 9 14:24:33 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::SetAllowFileSelectionChange(bool val)
{
    allowFileSelChange = val;

    if(allowFileSelectionChangeToggle != 0)
    {
        allowFileSelectionChangeToggle->blockSignals(true);
        allowFileSelectionChangeToggle->setChecked(allowFileSelChange);
        allowFileSelectionChangeToggle->blockSignals(false);
    }
}

//
// Qt Slot functions
//


// ****************************************************************************
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
void
QvisPreferencesWindow::cloneWindowOnFirstRefToggled(bool val)
{
    atts->SetCloneWindowOnFirstRef(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::postWindowsWhenShownToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag that tells the GUI whether
//   or not windows should automatically post when they are shown.
//
// Arguments:
//   val : The new post value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 5 15:47:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::postWindowsWhenShownToggled(bool val)
{
    postWhenShown = val;
}

// ****************************************************************************
// Method: QvisPreferencesWindow::makeDefaultConfirmToggled
//
// Purpose: 
//   This is a Qt slot function that gets the flag that tells the GUI whether
//   it should prompt users before "make defaults".
//
// Arguments:
//   val : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 6 09:28:57 PDT 2004
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::makeDefaultConfirmToggled(bool val)
{
    atts->SetMakeDefaultConfirm(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::automaticallyApplyOperatorToggled
//
// Purpose: 
//   This is a Qt slot function that gets the flag that tells the GUI whether
//   it should prompt users before automatically adding an operator when
//   the user sets the operator attributes when none were applied.
//
// Arguments:
//   val : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 6 09:28:57 PDT 2004
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::automaticallyApplyOperatorToggled(bool val)
{
    atts->SetAutomaticallyAddOperator(!val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::newPlotsInheritSILRestrictionToggled
//
// Purpose: 
//   This is a Qt slot function that gets the flag that tells whether new plots
//   should inherit the SIL retriction of a previous plot.
//
// Arguments:
//   val : The new value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 24 11:33:33 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::newPlotsInheritSILRestrictionToggled(bool val)
{
    atts->SetNewPlotsInheritSILRestriction(val);
    SetUpdate(false);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::handleTimeStateDisplayModeChange
//
// Purpose: 
//   This is a Qt slot function that is called when the display mode radio
//   buttons are clicked.
//
// Arguments:
//   val : The new timestate display mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 09:57:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::handleTimeStateDisplayModeChange(int val)
{
    tsFormat.SetDisplayMode((TimeFormat::DisplayMode)val);
    emit changeTimeFormat(tsFormat);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::timeStateNDigitsChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the time format spin box
//   is changed.
//
// Arguments:
//   val : The new number of digits in the time format.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 13:34:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::timeStateNDigitsChanged(int val)
{
    tsFormat.SetPrecision(val);
    emit changeTimeFormat(tsFormat);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::selectedFilesToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the selectedFilesToggle
//   is clicked.
//
// Arguments:
//   val : Whether the selected files should show.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:28:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::selectedFilesToggled(bool val)
{
    showSelFiles = val;
    emit showSelectedFiles(val);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::allowFileSelectionChangeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//    allowFileSelectionChange toggle is clicked.
//
// Arguments:
//   val : Whether the selected files should show.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:28:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::allowFileSelectionChangeToggled(bool val)
{
    allowFileSelChange = val;
    emit allowFileSelectionChange(val);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::tryHarderCyclesTimesToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   tryHarderCyclesTimesToggle is clicked.
//
// Programmer: Mark C. Miller 
// Creation:   June 1, 2005 
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::tryHarderCyclesTimesToggled(bool val)
{
    atts->SetTryHarderCyclesTimes(val);
    fileServer->SetForceReadAllCyclesTimes(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::ignoreDbExtentsToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   ignoreDbExtentsToggle is clicked.
//
// Programmer: Mark C. Miller 
// Creation:   May 27, 2008 
//
// ****************************************************************************

void
QvisPreferencesWindow::ignoreDbExtentsToggled(bool val)
{
    atts->SetIgnoreExtentsFromDbs(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::treatAllDBsAsTimeVarying
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   treatAllDBsAsTimeVarying is clicked.
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007 
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::treatAllDBsAsTimeVaryingToggled(bool val)
{
    atts->SetTreatAllDBsAsTimeVarying(val);
    fileServer->SetTreatAllDBsAsTimeVarying(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::createMeshQualityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   createMeshQualityToggle is clicked.
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007 
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::createMeshQualityToggled(bool val)
{
    atts->SetCreateMeshQualityExpressions(val);
    fileServer->SetCreateMeshQualityExpressions(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::createTimeDerivativeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   createTimeDerivativeToggle is clicked.
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007 
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::createTimeDerivativeToggled(bool val)
{
    atts->SetCreateTimeDerivativeExpressions(val);
    fileServer->SetCreateTimeDerivativeExpressions(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::createVectorMagnitudeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//   createVectorMagnitudeToggle is clicked.
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007 
//
// Modifications:
//    Cyrus Harrison, Fri Nov 30 10:56:10 PST 2007
//    Notify the GlobalAttributes 
//
// ****************************************************************************

void
QvisPreferencesWindow::createVectorMagnitudeToggled(bool val)
{
    atts->SetCreateVectorMagnitudeExpressions(val);
    fileServer->SetCreateVectorMagnitudeExpressions(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::userDirForSessionFilesToggled
//
// Purpose: 
//   This is a Qt slot function called when userDirForSessionFiles is toggled.
//
// Arguments:
//   val : The new value.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 10:32:48 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPreferencesWindow::userDirForSessionFilesToggled(bool val)
{
    atts->SetUserDirForSessionFiles(val);
    SetUpdate(false);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::saveCrashRecoveryFileToggled
//
// Purpose: 
//   This is a Qt slot function called when saveCrashRecoveryFile is toggled.
//
// Arguments:
//   val : The new value.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 10:32:48 PST 2008
//
// Modifications:
//   
// ****************************************************************************


void
QvisPreferencesWindow::saveCrashRecoveryFileToggled(bool val)
{
    atts->SetSaveCrashRecoveryFile(val);
    SetUpdate(false);
    atts->Notify();
}

