// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisPreferencesWindow.h"

#include <GlobalAttributes.h>
#include <ViewerProxy.h>
#include <FileServerList.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>

#include <visit-config.h>

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
//   Jeremy Meredith, Fri Nov  6 11:38:38 EST 2009
//   File panel selected files list now starts out hidden.
//
//   Eric Brugger, Tue Aug 24 12:33:44 PDT 2010
//   I added a toggle button to enable warning message popups.
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
    showSelFiles = false;
    selectedFilesToggle = 0;
    allowFileSelChange = true;
    allowFileSelectionChangeToggle = 0;
    enableWarnPopups = true;
    enableWarningPopupsToggle = 0;

    backendType = NULL;
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
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//   Added support for ignoring bad extents from dbs.
//
//   Hank Childs, Wed Mar 17 20:13:21 PDT 2010
//   Added "Expand new plots"
//
//   Brad Whitlock, Fri May  7 14:31:00 PDT 2010
//   I transplanted replacePlotsToggle.
//
//   Eric Brugger, Tue Aug 24 12:33:44 PDT 2010
//   I added a toggle button to enable warning message popups.
//
//   Kathleen Biagas, Wed Aug  7 13:07:54 PDT 2013
//   Added controls for precision type.
//
//   David Camp, Thu Aug  8 08:50:06 PDT 2013
//   Added the restore from last session feature.
//
//   Cameron Christensen, Tuesday, June 10, 2014
//   Added a preference for setting the backend type.
//
//   Eric Brugger, Tue Sep 30 15:02:07 PDT 2014
//   I made the code for setting the backend type depend on having EAVL.
//
//   Kathleen Biagas, Mon Dec 22 10:46:10 PST 2014
//   Added a preference for automatically removing duplicate nodes.
//
//   Eric Brugger, Thu Dec 10 11:15:48 PST 2015
//   I added support for the VTKm backend type.
//
//   Kathleen Biagas, Wed Jan 30 10:43:52 PST 2019
//   Removed support for EAVL.
//
//   Kathleen Biagas, Fri Nov  8 07:27:31 PST 2019
//   Moved content into tabs.
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateWindowContents()
{
    QTabWidget *tabs = new QTabWidget(central);
    topLayout->addWidget(tabs);

    //
    // General tab
    //
    QWidget *generalTab = new QWidget();
    tabs->addTab(generalTab, tr("General"));
    CreateGeneralTab(generalTab);

    //
    // Databases tab
    //
    QWidget *databasesTab = new QWidget();
    tabs->addTab(databasesTab, tr("Database"));
    CreateDatabasesTab(databasesTab);

    //
    // Session files tab
    //
    QWidget *sessionFilesTab = new QWidget();
    tabs->addTab(sessionFilesTab, tr("Session file"));
    CreateSessionFilesTab(sessionFilesTab);

    //
    // File panel properties tab
    //
    QWidget *filePanelTab = new QWidget();
    tabs->addTab(filePanelTab, tr("File panel"));
    CreateFilePanelTab(filePanelTab);
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateGeneralTab
//
// Purpose:
//   Creates the widgets for general preferences on the general tab.
//
// Programmer: Kathleen Biagas
// Creation:   November 8, 2019
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateGeneralTab(QWidget *gTab)
{
    QVBoxLayout *genLayout = new QVBoxLayout(gTab);

    cloneWindowOnFirstRefToggle =
        new QCheckBox(tr("Clone window on first reference"));
    connect(cloneWindowOnFirstRefToggle, SIGNAL(toggled(bool)),
            this, SLOT(cloneWindowOnFirstRefToggled(bool)));
    genLayout->addWidget(cloneWindowOnFirstRefToggle);

    postWindowsWhenShownToggle =
        new QCheckBox(tr("Post windows when shown"));
    connect(postWindowsWhenShownToggle, SIGNAL(toggled(bool)),
            this, SLOT(postWindowsWhenShownToggled(bool)));
    genLayout->addWidget(postWindowsWhenShownToggle);

    makeDefaultConfirmToggle =
        new QCheckBox(tr("Prompt before setting default attributes"));
    connect(makeDefaultConfirmToggle, SIGNAL(toggled(bool)),
            this, SLOT(makeDefaultConfirmToggled(bool)));
    genLayout->addWidget(makeDefaultConfirmToggle);

    automaticallyApplyOperatorToggle =
        new QCheckBox(tr("Prompt before applying new operator"));
    connect(automaticallyApplyOperatorToggle, SIGNAL(toggled(bool)),
            this, SLOT(automaticallyApplyOperatorToggled(bool)));
    genLayout->addWidget(automaticallyApplyOperatorToggle);

    newPlotsInheritSILRestrictionToggle =
        new QCheckBox(tr("New plots inherit SIL restriction"));
    connect(newPlotsInheritSILRestrictionToggle, SIGNAL(toggled(bool)),
            this, SLOT(newPlotsInheritSILRestrictionToggled(bool)));
    genLayout->addWidget(newPlotsInheritSILRestrictionToggle);

    expandNewPlotsToggle =
        new QCheckBox(tr("New plots automatically expanded"));
    connect(expandNewPlotsToggle, SIGNAL(toggled(bool)),
            this, SLOT(expandNewPlotsToggled(bool)));
    genLayout->addWidget(expandNewPlotsToggle);

    replacePlotsToggle = new QCheckBox(tr("Replace plots"));
    connect(replacePlotsToggle, SIGNAL(toggled(bool)),
            this, SLOT(replacePlotsToggled(bool)));
    genLayout->addWidget(replacePlotsToggle);

    enableWarningPopupsToggle = new QCheckBox(tr("Enable warning message popups"));
    connect(enableWarningPopupsToggle, SIGNAL(toggled(bool)),
            this, SLOT(enableWarningPopupsToggled(bool)));
    genLayout->addWidget(enableWarningPopupsToggle);

    //
    // Create radio button controls to change the precision.
    //
    QGroupBox *precisionGroup = new QGroupBox();
    precisionGroup->setTitle(tr("Floating point precision:"));
    genLayout->addWidget(precisionGroup);
    precisionType = new QButtonGroup();
    connect(precisionType, SIGNAL(buttonClicked(int)),
            this, SLOT(precisionTypeChanged(int)));

    QHBoxLayout *precLayout = new QHBoxLayout(precisionGroup);
    QRadioButton *dec = new QRadioButton(tr("Float"));
    precisionType->addButton(dec,0);
    precLayout->addWidget(dec);
    QRadioButton *nat = new QRadioButton(tr("Native"));
    precisionType->addButton(nat,1);
    precLayout->addWidget(nat);
    QRadioButton *inc = new QRadioButton(tr("Double"));
    precisionType->addButton(inc,2);
    precLayout->addWidget(inc);

#if defined(HAVE_LIBVTKH)
    //
    // Create radio button controls to change the backend.
    //
    QGroupBox *backendGroup = new QGroupBox();
    backendGroup->setTitle(tr("Parallel computation library:"));
    genLayout->addWidget(backendGroup);
    backendType = new QButtonGroup();
    connect(backendType, SIGNAL(buttonClicked(int)),
            this, SLOT(backendTypeChanged(int)));

    QHBoxLayout *backendLayout = new QHBoxLayout(backendGroup);
    QRadioButton *b0 = new QRadioButton(tr("VTK"));
    backendType->addButton(b0,0);
    backendLayout->addWidget(b0);
    QRadioButton *b1 = new QRadioButton(tr("VTKm"));
    backendType->addButton(b1,1);
    backendLayout->addWidget(b1);
#endif
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateDatabasesTab
//
// Purpose:
//   Creates the widgets for databases preferences on the databases tab.
//
// Programmer: Kathleen Biagas
// Creation:   November 8, 2019
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateDatabasesTab(QWidget *dTab)
{
    QVBoxLayout *dbOptionsLayout = new QVBoxLayout(dTab);

    tryHarderCyclesTimesToggle =
        new QCheckBox(tr("Try harder to get accurate cycles/times"));
    connect(tryHarderCyclesTimesToggle, SIGNAL(toggled(bool)),
            this, SLOT(tryHarderCyclesTimesToggled(bool)));
    dbOptionsLayout->addWidget(tryHarderCyclesTimesToggle);

    ignoreDbExtentsToggle =
        new QCheckBox(tr("Ignore database extents (may degrade performance)"));
    connect(ignoreDbExtentsToggle, SIGNAL(toggled(bool)),
            this, SLOT(ignoreDbExtentsToggled(bool)));
    dbOptionsLayout->addWidget(ignoreDbExtentsToggle);

    treatAllDBsAsTimeVaryingToggle =
        new QCheckBox(tr("Treat all databases as time-varying"));
    connect(treatAllDBsAsTimeVaryingToggle, SIGNAL(toggled(bool)),
            this, SLOT(treatAllDBsAsTimeVaryingToggled(bool)));
    dbOptionsLayout->addWidget(treatAllDBsAsTimeVaryingToggle);

    createMeshQualityToggle =
        new QCheckBox(tr("Automatically create mesh quality expressions"));
    connect(createMeshQualityToggle, SIGNAL(toggled(bool)),
            this, SLOT(createMeshQualityToggled(bool)));
    dbOptionsLayout->addWidget(createMeshQualityToggle);

    createTimeDerivativeToggle =
        new QCheckBox(tr("Automatically create time derivative expressions"));
    connect(createTimeDerivativeToggle, SIGNAL(toggled(bool)),
            this, SLOT(createTimeDerivativeToggled(bool)));
    dbOptionsLayout->addWidget(createTimeDerivativeToggle);

    createVectorMagnitudeToggle =
        new QCheckBox(tr("Automatically create vector magnitude expressions"));
    connect(createVectorMagnitudeToggle, SIGNAL(toggled(bool)),
            this, SLOT(createVectorMagnitudeToggled(bool)));
    dbOptionsLayout->addWidget(createVectorMagnitudeToggle);

    removeDuplicateNodesToggle =
        new QCheckBox(tr("Automatically remove duplicate nodes from fully disconnected unstructured grids"));
    connect(removeDuplicateNodesToggle, SIGNAL(toggled(bool)),
            this, SLOT(removeDuplicateNodesToggled(bool)));
    dbOptionsLayout->addWidget(removeDuplicateNodesToggle);

    // fill up bottom space
    dbOptionsLayout->addStretch(1);
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateSessionFilesTab
//
// Purpose:
//   Creates the widgets for sessionfile preferences on the sessionfile tab.
//
// Programmer: Kathleen Biagas
// Creation:   November 8, 2019
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateSessionFilesTab(QWidget *sfTab)
{
    QVBoxLayout *sessionOptionsLayout = new QVBoxLayout(sfTab);

    userDirForSessionFilesToggle =
        new QCheckBox(tr("User directory is default location for session files"));
    connect(userDirForSessionFilesToggle, SIGNAL(toggled(bool)),
            this, SLOT(userDirForSessionFilesToggled(bool)));
    sessionOptionsLayout->addWidget(userDirForSessionFilesToggle);

    saveCrashRecoveryFileToggle =
        new QCheckBox(tr("Periodically save a crash recovery file"));
    connect(saveCrashRecoveryFileToggle, SIGNAL(toggled(bool)),
            this, SLOT(saveCrashRecoveryFileToggled(bool)));
    sessionOptionsLayout->addWidget(saveCrashRecoveryFileToggle);

    userRestoreSessionFileToggle =
        new QCheckBox(tr("Restore session on startup"));

    connect(userRestoreSessionFileToggle, SIGNAL(toggled(bool)),
            this, SLOT(userRestoreSessionFileToggled(bool)));
    sessionOptionsLayout->addWidget(userRestoreSessionFileToggle);

    // fill up bottom space
    sessionOptionsLayout->addStretch(1);
}


// ****************************************************************************
// Method: QvisPreferencesWindow::CreateFilePanelTab
//
// Purpose:
//   Creates the widgets for filepanel preferences on the filepanel tab.
//
// Programmer: Kathleen Biagas
// Creation:   November 8, 2019
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::CreateFilePanelTab(QWidget *fpTab)
{
    QVBoxLayout *fpLayout = new QVBoxLayout(fpTab);

    QGridLayout *tsModeLayout = new QGridLayout();

    //
    // Widgets that let you control the file panel.
    //
    selectedFilesToggle = new QCheckBox(tr("Show selected files"));
    selectedFilesToggle->setChecked(showSelFiles);
    connect(selectedFilesToggle, SIGNAL(toggled(bool)),
            this, SLOT(selectedFilesToggled(bool)));
    tsModeLayout->addWidget(selectedFilesToggle, 0, 0, 1, 4);

    allowFileSelectionChangeToggle = new QCheckBox(
        tr("Automatically highlight open file"));
    allowFileSelectionChangeToggle->setChecked(allowFileSelChange);
    connect(allowFileSelectionChangeToggle, SIGNAL(toggled(bool)),
            this, SLOT(allowFileSelectionChangeToggled(bool)));
    tsModeLayout->addWidget(allowFileSelectionChangeToggle, 1, 0, 1, 4);

    //
    // Create radio button controls to let us change the timestate display mode.
    //
    tsModeLayout->addWidget(new QLabel(tr("Display time using:")), 2, 0, 1, 3);
    timeStateDisplayMode = new QButtonGroup();
    QRadioButton *rb = new QRadioButton(tr("Cycles"));
    timeStateDisplayMode->addButton(rb,0);
    tsModeLayout->addWidget(rb, 3, 0);
    rb = new QRadioButton(tr("Times"));
    timeStateDisplayMode->addButton(rb,1);
    tsModeLayout->addWidget(rb, 3, 1);
    rb = new QRadioButton(tr("Cycles and times"));
    timeStateDisplayMode->addButton(rb,2);
    tsModeLayout->addWidget(rb, 3, 2);
    timeStateDisplayMode->button(int(tsFormat.GetDisplayMode()))->setChecked(true);
    connect(timeStateDisplayMode, SIGNAL(buttonClicked(int)),
            this, SLOT(handleTimeStateDisplayModeChange(int)));

    //
    // Create widgets that let you set the time format.
    //
    tsModeLayout->addWidget(new QLabel(tr("Number of significant digits")),
                            4, 0, 1, 2);
    timeStateNDigits = new QSpinBox();
    timeStateNDigits->setRange(1,16);
    timeStateNDigits->setSingleStep(1);
    timeStateNDigits->setValue(tsFormat.GetPrecision());
    connect(timeStateNDigits, SIGNAL(valueChanged(int)),
            this, SLOT(timeStateNDigitsChanged(int)));
    tsModeLayout->addWidget(timeStateNDigits, 4, 2);

    fpLayout->addLayout(tsModeLayout);
    // fill up bottom space
    fpLayout->addStretch(1);
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
//
//   Jeremy Meredith, Fri Nov  6 11:39:11 EST 2009
//   Removed duplicate code which looked like a copy/paste error.
//
//   Hank Childs, Wed Mar 17 20:13:21 PDT 2010
//   Added support for "Expand New Plots".
//
//   Brad Whitlock, Fri May  7 14:34:37 PDT 2010
//   I transplanted some replace plots code.
//
//   Eric Brugger, Tue Aug 24 12:33:44 PDT 2010
//   I added a toggle button to enable warning message popups.
//
//   Kathleen Biagas, Wed Aug  7 13:08:23 PDT 2013
//   Handle precisionType.
//
//   David Camp, Thu Aug  8 08:50:06 PDT 2013
//   Added the restore from last session feature.
//
//   Cameron Christensen, Tuesday, June 10, 2014
//   Added a preference for setting the backend type.
//
//   Kathleen Biagas, Mon Dec 22 10:46:10 PST 2014
//   Added a preference for automatically removing duplicate nodes.
//
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

    if (doAll || atts->IsSelected(GlobalAttributes::ID_expandNewPlots))
    {
        //
        // New plots expanded by default
        //
        expandNewPlotsToggle->blockSignals(true);
        expandNewPlotsToggle->setChecked(atts->GetExpandNewPlots());
        expandNewPlotsToggle->blockSignals(false);
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

    if (doAll || atts->IsSelected(GlobalAttributes::ID_removeDuplicateNodes))
    {
        removeDuplicateNodesToggle->blockSignals(true);
        removeDuplicateNodesToggle->setChecked(
            atts->GetRemoveDuplicateNodes());
        removeDuplicateNodesToggle->blockSignals(false);
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

    if (doAll || atts->IsSelected(GlobalAttributes::ID_userRestoreSessionFile))
    {
        userRestoreSessionFileToggle->blockSignals(true);
        userRestoreSessionFileToggle->setChecked(
            atts->GetUserRestoreSessionFile());
        userRestoreSessionFileToggle->blockSignals(false);
    }

    if(doAll || atts->IsSelected(GlobalAttributes::ID_replacePlots))
    {
        replacePlotsToggle->blockSignals(true);
        replacePlotsToggle->setChecked(atts->GetReplacePlots());
        replacePlotsToggle->blockSignals(false);
    }

    if(doAll || atts->IsSelected(GlobalAttributes::ID_precisionType))
    {
        precisionType->blockSignals(true);
        precisionType->button((int)atts->GetPrecisionType())->setChecked(true);
        precisionType->blockSignals(false);
    }

    if(doAll || atts->IsSelected(GlobalAttributes::ID_backendType))
    {
        if(backendType != NULL)
        {
            backendType->blockSignals(true);
            backendType->button((int)atts->GetBackendType())->setChecked(true);
            backendType->blockSignals(false);
        }
    }

    if(doAll)
    {
        postWindowsWhenShownToggle->blockSignals(true);
        postWindowsWhenShownToggle->setChecked(postWhenShown);
        postWindowsWhenShownToggle->blockSignals(false);

        selectedFilesToggle->blockSignals(true);
        selectedFilesToggle->setChecked(showSelFiles);
        selectedFilesToggle->blockSignals(false);

        allowFileSelectionChangeToggle->blockSignals(true);
        allowFileSelectionChangeToggle->setChecked(allowFileSelChange);
        allowFileSelectionChangeToggle->blockSignals(false);

        enableWarningPopupsToggle->blockSignals(true);
        enableWarningPopupsToggle->setChecked(enableWarnPopups);
        enableWarningPopupsToggle->blockSignals(false);
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
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPreferencesWindow::SetTimeStateFormat(const TimeFormat &fmt)
{
    tsFormat = fmt;
    if(timeStateDisplayMode != 0 && timeStateNDigits != 0)
    {
        timeStateDisplayMode->blockSignals(true);
        timeStateDisplayMode->button(int(tsFormat.GetDisplayMode()))->setChecked(true);
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

// ****************************************************************************
// Method: QvisPreferencesWindow::SetEnableWarningPopups
//
// Purpose:
//   This method sets the toggle for the enable warning popups.
//
// Programmer: Eric Brugger
// Creation:   Tue Aug 24 12:33:44 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::SetEnableWarningPopups(bool val)
{
    enableWarnPopups = val;

    if(enableWarningPopupsToggle != 0)
    {
        enableWarningPopupsToggle->blockSignals(true);
        enableWarningPopupsToggle->setChecked(enableWarnPopups);
        enableWarningPopupsToggle->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisPreferencesWindow::GetEnableWarningPopups
//
// Purpose:
//   This method returns the value of enable warning popups.
//
// Programmer: Eric Brugger
// Creation:   Tue Aug 24 12:33:44 PDT 2010
//
// Modifications:
//
// ****************************************************************************

bool
QvisPreferencesWindow::GetEnableWarningPopups()
{
    return enableWarnPopups;
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
// Method: QvisPreferencesWindow::expandNewPlotsToggled
//
// Purpose:
//   This is a Qt slot function that gets the flag that tells whether new plots
//   should be automatically expanded.
//
// Arguments:
//   val : The new value.
//
// Programmer: Hank Childs
// Creation:   March 17, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::expandNewPlotsToggled(bool val)
{
    atts->SetExpandNewPlots(val);
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

// ****************************************************************************
// Method: QvisPreferencesWindow::replacePlotsToggled
//
// Purpose:
//   This is a Qt slot function that is called when the replace plots checkbox
//   is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 4 11:45:12 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::replacePlotsToggled(bool val)
{
    atts->SetReplacePlots(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::EnableWarningPopupsToggled
//
// Purpose:
//   This is a Qt slot function that is called when the enable warning
//   popups checkbox is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Eric Brugger
// Creation:   Tue Aug 24 12:33:44 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::enableWarningPopupsToggled(bool val)
{
    enableWarnPopups = val;
    emit enableWarningPopups(val);
}

// ****************************************************************************
// Method: QvisPreferencesWindow::userRestoreSessionFileToggle
//
// Purpose:
//   This is a Qt slot function called when userRestoreSessionFileToggle is toggled.
//
// Arguments:
//   val : The new value.
//
// Returns:
//
// Note:
//
// Programmer: David Camp
// Creation:   Tue Jul 30 08:34:16 PDT 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::userRestoreSessionFileToggled(bool val)
{
    atts->SetUserRestoreSessionFile(val);
    SetUpdate(false);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::precisionTypeChanged
//
// Purpose:
//   This is a Qt slot function that is called when the precision
//   is changed.
//
// Arguments:
//   val : The new precision value.
//
// Programmer: Kathleen Biagas
// Creation:   July 25, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::precisionTypeChanged(int val)
{
    atts->SetPrecisionType(GlobalAttributes::PrecisionType(val));
    GetViewerProxy()->GetViewerMethods()->SetPrecisionType(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::backendTypeChanged
//
// Purpose:
//   This is a Qt slot function that is called when the backend
//   is changed.
//
// Arguments:
//   val : The new backend value.
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::backendTypeChanged(int val)
{
    atts->SetBackendType(GlobalAttributes::BackendType(val));
    GetViewerProxy()->GetViewerMethods()->SetBackendType(val);
    atts->Notify();
}

// ****************************************************************************
// Method: QvisPreferencesWindow::removeDuplicateNodesToggled
//
// Purpose:
//   This is a Qt slot function that is called when removeDuplicateNodes
//   is toggled.
//
// Arguments:
//   val : The new value.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisPreferencesWindow::removeDuplicateNodesToggled(bool val)
{
    atts->SetRemoveDuplicateNodes(val);
    GetViewerProxy()->GetViewerMethods()->SetRemoveDuplicateNodes(val);
    atts->Notify();
}

