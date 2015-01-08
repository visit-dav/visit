/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <QvisSelectionsWindow.h>

#include <EngineList.h>
#include <Plot.h>
#include <PlotList.h>
#include <SelectionProperties.h>
#include <SelectionSummary.h>
#include <SelectionVariableSummary.h>
#include <SelectionList.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>

#include <QualifiedFilename.h>
#include <QvisFileOpenDialog.h>
#include <QvisSelectionsDialog.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>

#include <QvisHistogram.h>
#include <QvisHistogramLimits.h>
#include <QvisVariableButton.h>
#include <QvisVariableListLimiter.h>

#include <iostream>

#define DEFAULT_FORCE_UPDATE false
#define DEFAULT_UPDATE_PLOTS true

#define ALLOW_CACHING      true
#define DONT_ALLOW_CACHING false

// ****************************************************************************
// Method: QvisSelectionsWindow::QvisSelectionsWindow
//
// Purpose: 
//   This is the constructor for the QvisSelectionsWindow class.
//
// Arguments:
//   selectionList_  The SelectionList subject to observe
//   
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Wed Sep  7 15:43:35 PDT 2011
//   Initialize allowCaching.
//
// ****************************************************************************

QvisSelectionsWindow::QvisSelectionsWindow(const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
        QvisPostableWindowSimpleObserver::NoExtraButtons, false)
{
    selectionList = 0;
    plotList = 0;
    engineList = 0;
    windowInformation = 0;

    selectionPropsValid = false;
    allowCaching = true;
    selectionCounter = 1;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::~QvisSelectionsWindow
//
// Purpose: 
//   Destructor for the QvisSelectionsWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   
// ****************************************************************************
QvisSelectionsWindow::~QvisSelectionsWindow()
{
    if(selectionList != 0)
        selectionList->Detach(this);
    if(plotList != 0)
        plotList->Detach(this);
    if(engineList != 0)
        engineList->Detach(this);
    if(windowInformation != 0)
        windowInformation->Detach(this);
}

void
QvisSelectionsWindow::ConnectSelectionList(SelectionList *s)
{
    selectionList = s;
    selectionList->Attach(this);
}

void
QvisSelectionsWindow::ConnectPlotList(PlotList *s)
{
    plotList = s;
    plotList->Attach(this);
}

void
QvisSelectionsWindow::ConnectEngineList(EngineList *el)
{
    engineList = el;
    engineList->Attach(this);
}

void
QvisSelectionsWindow::ConnectWindowInformation(WindowInformation *wi)
{
    windowInformation = wi;
    windowInformation->Attach(this);
}

void
QvisSelectionsWindow::SubjectRemoved(Subject *s)
{
    if(selectionList == s)
        selectionList = 0;

    if(plotList == s)
        plotList = 0;

    if(engineList == s)
        engineList = 0;

    if(windowInformation == s)
        windowInformation = 0;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::CreateWindowContents()
{
    QSplitter *mainSplitter = new QSplitter(central);
    topLayout->addWidget(mainSplitter);
    topLayout->setStretchFactor(mainSplitter, 100);

    //
    // Selections group
    //
    QGroupBox *f1 = new QGroupBox(tr("Selections"), central);
    mainSplitter->addWidget(f1);
    QGridLayout *listLayout = new QGridLayout(f1);
    listLayout->setMargin(5);

    selectionListBox = new QListWidget(f1);
    connect(selectionListBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    listLayout->addWidget(selectionListBox, 1,0, 1,2);

    newButton = new QPushButton(tr("New"), f1);
    connect(newButton, SIGNAL(pressed()),
            this, SLOT(addSelection()));
    listLayout->addWidget(newButton, 2,0);

    deleteButton = new QPushButton(tr("Delete"), f1);
    connect(deleteButton, SIGNAL(pressed()),
            this, SLOT(deleteSelection()));
    listLayout->addWidget(deleteButton, 2,1);

    saveButton = new QPushButton(tr("Save"), f1);
    connect(saveButton, SIGNAL(pressed()),
            this, SLOT(saveSelection()));
    listLayout->addWidget(saveButton, 3,0);

    loadButton = new QPushButton(tr("Load"), f1);
    connect(loadButton, SIGNAL(pressed()),
            this, SLOT(loadSelection()));
    listLayout->addWidget(loadButton, 3,1);

    editorTabs = new QTabWidget();
    mainSplitter->addWidget(editorTabs);

    editorTabs->addTab(CreatePropertiesTab(central), tr("Properties"));
    editorTabs->addTab(CreateStatisticsTab(central), tr("Statistics"));
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreatePropertiesTab
//
// Purpose: 
//   Create the properties tab.
//
// Arguments:
//   parent : The parent widget.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:17:36 PST 2010
//
// Modifications:
//   Brad Whitlock, Fri May 20 15:30:19 PDT 2011
//   I made it look more like the image from visitusers.org.
//
//   Brad Whitlock, Mon Nov  7 14:05:38 PST 2011
//   I added controls for setting the id variable.
//
// ****************************************************************************

QWidget *
QvisSelectionsWindow::CreatePropertiesTab(QWidget *parent)
{
    //
    // Properties group
    //
    QWidget *f2 = new QWidget(parent);
    QGridLayout *definitionLayout = new QGridLayout(f2);
    definitionLayout->setMargin(5);

    int row = 0;
    QLabel *plotNameEditLabel = new QLabel(tr("Selection source"), f2);
    plotNameEditLabel->setToolTip(tr("The data source that defines the selection"));
    plotNameLabel = new QLabel(f2);
    definitionLayout->addWidget(plotNameEditLabel, row,0);
    definitionLayout->addWidget(plotNameLabel, row,1, 1,3);
    ++row;

    // Add controls that let the user pick the method for relating cells.
    QGroupBox *idGroup = new QGroupBox(tr("Relate cells using"), f2);
    QGridLayout *iLayout = new QGridLayout(idGroup);
    idVariableType = new QButtonGroup(f2);
    QRadioButton *b0 = new QRadioButton(tr("Domain and cell numbers"), f2);
    idVariableType->addButton(b0, 0);
    iLayout->addWidget(b0, 0, 0);
    QRadioButton *b1 = new QRadioButton(tr("Global cell numbers"), f2);
    idVariableType->addButton(b1, 1);
    iLayout->addWidget(b1, 0, 1);
    QRadioButton *b2 = new QRadioButton(tr("Location"), f2);
    idVariableType->addButton(b2, 2);
    iLayout->addWidget(b2, 0, 2);
    QRadioButton *b3 = new QRadioButton(tr("Variable"), f2);
    idVariableType->addButton(b3, 3);
    iLayout->addWidget(b3, 0, 3);
    b0->setChecked(true);
    connect(idVariableType, SIGNAL(buttonClicked(int)),
            this, SLOT(idVariableTypeChanged(int)));
    idVariableButton = new QvisVariableButton(f2);
    idVariableButton->setVarTypes(QvisVariableButton::Scalars);
    idVariableButton->setAddDefault(false);
    idVariableButton->setEnabled(false);
    connect(idVariableButton, SIGNAL(activated(const QString &)),
            this, SLOT(idVariableChanged(const QString &)));
    iLayout->addWidget(idVariableButton, 0, 4);
    definitionLayout->addWidget(idGroup, row, 0, 1, 5);
    ++row;

    QFrame *spacer = new QFrame(f2);
    spacer->setFrameStyle(QFrame::HLine | QFrame::Raised);
    definitionLayout->addWidget(spacer, row,0, 1,5);
    ++row;

    cqControls = new QGroupBox(f2);
    cqControls->setTitle(tr("Cumulative Query"));
    cqControls->setCheckable(true);
    connect(cqControls, SIGNAL(clicked(bool)),
            this, SLOT(cumulativeQueryClicked(bool)));


    QVBoxLayout *vLayout = new QVBoxLayout(cqControls);
    definitionLayout->addWidget(cqControls, row,0,1,5);
    definitionLayout->setRowStretch(row, 10);
    ++row;

    cqTabs = new QTabWidget(cqControls);
    vLayout->addWidget(cqTabs);
    cqTabs->addTab(CreateCQRangeControls(cqControls), tr("Range"));
    cqTabs->addTab(CreateCQHistogramControls(cqControls), tr("Histogram"));


    automaticallyApply = new QCheckBox(tr("Automatically apply updated selections"), f2);
    connect(automaticallyApply, SIGNAL(toggled(bool)),
            this, SLOT(automaticallyApplyChanged(bool)));
    definitionLayout->addWidget(automaticallyApply, row,0,1,3);


    updateSelectionButton = new QPushButton(tr("Update Selection"), f2);
    connect(updateSelectionButton, SIGNAL(pressed()),
            this, SLOT(updateSelection()));
    definitionLayout->addWidget(updateSelectionButton, row,4);
    ++row;

    return f2;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateCQRangeControls
//
// Purpose: 
//   Create the CQ controls.
//
// Arguments:
//   parent : The parent widget.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:17:07 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisSelectionsWindow::CreateCQRangeControls(QWidget *parent)
{
    QWidget *central = new QWidget(parent);
    QGridLayout *lLayout = new QGridLayout(central);
    lLayout->setMargin(5);

    // Add the variable button
    QLabel *addVar = new QLabel(tr("Add variable"), central);
    lLayout->addWidget(addVar, 0, 0);
    cqVarButton = new QvisVariableButton(central);
    cqVarButton->setVarTypes(QvisVariableButton::Scalars);
    cqVarButton->setAddDefault(false);
    connect(cqVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    lLayout->addWidget(cqVarButton, 0, 1);

    cqInitializeVarButton = new QPushButton(tr("Get variable ranges"), central);
    connect(cqInitializeVarButton, SIGNAL(clicked()),
            this, SLOT(initializeVariableList()));
    cqInitializeVarButton->setToolTip(tr(
        "Populate the list of variables used for the cumulative query\n"
        "selection from the list of variables used in the selection's\n"
        "originating plot. The originating plot must be a Parallel\n"
        "Coordinates plot."));
    lLayout->addWidget(cqInitializeVarButton, 0, 2);

    
    updateQueryButton1 = new QPushButton(tr("Update Query"), central);
    connect(updateQueryButton1, SIGNAL(pressed()),
            this, SLOT(updateQuery()));
    lLayout->addWidget(updateQueryButton1, 0,3);


    // Add the variable list.
    cqLimits = new QvisVariableListLimiter(central);
    cqLimits->setMinimumHeight(200);
    connect(cqLimits, SIGNAL(selectedRangeChanged(const QString &,float,float)),
            this, SLOT(setVariableRange(const QString &,float,float)));
    connect(cqLimits, SIGNAL(deleteVariable(const QString &)),
            this, SLOT(deleteVariable(const QString &)));
    lLayout->addWidget(cqLimits, 1, 0, 1, 4);

    // Add the time controls
    cqTimeGroupBox = CreateTimeControls(central);
    lLayout->addWidget(cqTimeGroupBox, 2, 0, 1, 4);

    return central;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateTimeControls
//
// Purpose: 
//   Create the time controls group box for CQ selections.
//
// Arguments:
//   parent : The parent widget.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:16:16 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QGroupBox *
QvisSelectionsWindow::CreateTimeControls(QWidget *parent)
{
    QGroupBox *central = new QGroupBox(parent);
    central->setTitle(tr("Query over time"));
    QGridLayout *gLayout = new QGridLayout(central);
    gLayout->setMargin(5);

    // Time controls
    gLayout->addWidget(new QLabel(tr("Start"), central), 0, 0);
    cqTimeMin = new QLineEdit(central);
    connect(cqTimeMin, SIGNAL(returnPressed()),
            this, SLOT(processTimeMin()));
    gLayout->addWidget(cqTimeMin, 0, 1);

    gLayout->addWidget(new QLabel(tr("End"), central), 0, 2);
    cqTimeMax = new QLineEdit(central);
    connect(cqTimeMax, SIGNAL(returnPressed()),
            this, SLOT(processTimeMax()));
    gLayout->addWidget(cqTimeMax, 0, 3);

    gLayout->addWidget(new QLabel(tr("Stride"), central), 0, 4);
    cqTimeStride = new QLineEdit(central);
    connect(cqTimeStride, SIGNAL(returnPressed()),
            this, SLOT(processTimeStride()));
    gLayout->addWidget(cqTimeStride, 0, 5);

    return central;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateCQHistogramControls
//
// Purpose: 
//   Create the histogram controls for CQ selections.
//
// Arguments:
//
// Returns:    A widget containing the parent widget.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 16:06:09 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:23:00 PDT 2011
//   Change cqHistogramVariable into a variable button.
//
// ****************************************************************************

QWidget *
QvisSelectionsWindow::CreateCQHistogramControls(QWidget *parent)
{
    QWidget *central = new QWidget(parent);
    QVBoxLayout *vLayout = new QVBoxLayout(central);
    vLayout->setMargin(5);

    QWidget *titleParent = new QWidget(central);
    vLayout->addWidget(titleParent);
    QHBoxLayout *thLayout = new QHBoxLayout(titleParent);
    thLayout->setMargin(0);

    QLabel *histLabel = new QLabel(tr("Histogram"), titleParent);
    thLayout->addWidget(histLabel);
    cqHistogramTitle = new QLabel(titleParent);
    thLayout->addWidget(cqHistogramTitle, Qt::AlignLeft);

    updateQueryButton2 = new QPushButton(tr("Update Query"), central);
    connect(updateQueryButton2, SIGNAL(pressed()),
            this, SLOT(updateQuery()));
    thLayout->addWidget(updateQueryButton2);

    cqHistogram = new QvisHistogram(central);
    cqHistogram->setDrawBinLines(true);
    cqHistogram->setSelectionEnabled(false);
    cqHistogram->setMinimumHeight(150);
    vLayout->addWidget(cqHistogram);

    QWidget *axisParent = new QWidget(central);
    vLayout->addWidget(axisParent);

    QHBoxLayout *thAxisLayout = new QHBoxLayout(axisParent);
    thAxisLayout->setMargin(0);

    cqHistogramMinAxisLabel = new QLabel(tr("Bin 0 (0)"), titleParent);
    thAxisLayout->addWidget(cqHistogramMinAxisLabel);

    cqHistogramAxisLabel = new QLabel(titleParent);
    cqHistogramAxisLabel->setText("");
    thAxisLayout->addWidget(cqHistogramAxisLabel, Qt::AlignLeft);

    cqHistogramMaxAxisLabel = new QLabel(tr("Bin 9 (0)"), titleParent);
    thAxisLayout->addWidget(cqHistogramMaxAxisLabel);

    //
    // Axis controls
    //
    QGroupBox *axisGroup = new QGroupBox(tr("Axis"), central);
    vLayout->addWidget(axisGroup);
    QGridLayout *aLayout = new QGridLayout(axisGroup);
    aLayout->setMargin(5);

    aLayout->addWidget(new QLabel(tr("Display axis type"), axisGroup), 0,0);

    QRadioButton *timeSlice = new QRadioButton(tr("Time slice"), axisGroup);
    QRadioButton *id = new QRadioButton(tr("ID"), axisGroup);
    QRadioButton *matches = new QRadioButton(tr("Matches"), axisGroup);
    cqHistogramVariableButton = new QRadioButton(tr("Variable"), axisGroup);
    cqHistogramType = new QButtonGroup(axisGroup);
    connect(cqHistogramType, SIGNAL(buttonClicked(int)),
            this, SLOT(histogramTypeChanged(int)));
    cqHistogramType->addButton(timeSlice,0);
    cqHistogramType->addButton(matches,1);
    cqHistogramType->addButton(id,2);
    cqHistogramType->addButton(cqHistogramVariableButton,3);
    aLayout->addWidget(timeSlice, 0, 1);
    aLayout->addWidget(id, 1, 1);
    aLayout->addWidget(matches, 0, 2);
    aLayout->addWidget(cqHistogramVariableButton, 1, 2);

    cqHistogramVariable = new QvisVariableButton(axisGroup);
    cqHistogramVariable->setAddExpr(true);
    cqHistogramVariable->setAddDefault(false);
    cqHistogramVariable->setVarTypes(QvisVariableButton::Scalars);
    cqHistogramVariable->setChangeTextOnVariableChange(true);
    aLayout->addWidget(cqHistogramVariable, 1, 3);
    connect(cqHistogramVariable, SIGNAL(activated(const QString &)),
            this, SLOT(histogramVariableChanged(const QString &)));

    cqHistogramNumBinsLabel = new QLabel(tr("Number of bins"), axisGroup);
    aLayout->addWidget(cqHistogramNumBinsLabel, 2, 0);
    cqHistogramNumBins = new QSpinBox(axisGroup);
    cqHistogramNumBins->setRange(1,1024);
    connect(cqHistogramNumBins, SIGNAL(valueChanged(int)),
            this, SLOT(histogramNumBinsChanged(int)));
    aLayout->addWidget(cqHistogramNumBins, 2, 1);

    //
    // Summation controls
    //
    QGroupBox *summationGroup = new QGroupBox(tr("Summation"), central);
    vLayout->addWidget(summationGroup);
    QGridLayout *sLayout = new QGridLayout(summationGroup);
    sLayout->setMargin(5);

    sLayout->addWidget(new QLabel(tr("Type"), central), 0, 0);   
    cqSummation = new QComboBox(summationGroup);
    cqSummation->addItem(tr("Include cells matching in any time step"));
    cqSummation->addItem(tr("Include cells matching in all time steps"));
    connect(cqSummation, SIGNAL(activated(int)),
            this, SLOT(summationChanged(int)));
    sLayout->addWidget(cqSummation, 0, 1, 1, 3);

    cqHistogramMinLabel = new QLabel(tr("Minimum bin"), summationGroup);
    cqHistogramMin = new QSpinBox(summationGroup);
    cqHistogramMin->setRange(0,1023);
    connect(cqHistogramMin, SIGNAL(valueChanged(int)),
            this, SLOT(histogramStartChanged(int)));
    sLayout->addWidget(cqHistogramMinLabel, 1, 0);
    sLayout->addWidget(cqHistogramMin, 1, 1);

    cqHistogramMaxLabel = new QLabel(tr("Maximum bin"), summationGroup);
    cqHistogramMax = new QSpinBox(summationGroup);
    cqHistogramMax->setRange(0,1023);
    connect(cqHistogramMax, SIGNAL(valueChanged(int)),
            this, SLOT(histogramEndChanged(int)));
    sLayout->addWidget(cqHistogramMaxLabel, 1, 2);
    sLayout->addWidget(cqHistogramMax, 1, 3);

    return central;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateStatisticsTab
//
// Purpose: 
//   Create the widgets for the statistics tab.
//
// Arguments:
//   parent : the parent widget.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:15:46 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisSelectionsWindow::CreateStatisticsTab(QWidget *parent)
{
    QWidget *central = new QWidget(parent);
    QGridLayout *gLayout = new QGridLayout(central);
    gLayout->setMargin(5);

    statVars = new QTableWidget(central);
    statVars->verticalHeader()->hide();
    gLayout->addWidget(statVars, 0, 0, 1, 3);

    gLayout->addWidget(new QLabel(tr("Cells in selection:"), central), 1, 0);
    statSelectedCells = new QLabel(central);
    gLayout->addWidget(statSelectedCells, 1, 1);

    gLayout->addWidget(new QLabel(tr("Cells in dataset:"), central), 2, 0);
    statTotalCells = new QLabel(central);
    gLayout->addWidget(statTotalCells, 2, 1);

    return central;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.  Currently ignored.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Mon Oct 11 16:31:15 PDT 2010
//   I made it observe the engine list so we can set the load selection button's
//   enabled state to true if we have 1 engine but no plots.
//
// ****************************************************************************

void
QvisSelectionsWindow::UpdateWindow(bool doAll)
{
    if(selectionList == 0 || plotList == 0)
        return;

    if(SelectedSubject() == selectionList ||
       SelectedSubject() == engineList ||
       doAll)
    {
        automaticallyApply->blockSignals(true);
        automaticallyApply->setChecked(selectionList->GetAutoApplyUpdates());
        automaticallyApply->blockSignals(false);

        // Update the list of selections.
        int cur = selectionListBox->currentRow();
        selectionListBox->blockSignals(true);
        selectionListBox->clear();
        for (int i = 0; i < selectionList->GetNumSelections(); ++i)
            selectionListBox->addItem(selectionList->GetSelections(i).GetName().c_str());
        if(selectionListBox->count() > 0)
        {
            cur = (cur < selectionListBox->count()) ? cur : 0;
            cur = (cur < 0) ? 0 : cur;
            selectionListBox->setCurrentRow(cur);
        }
        selectionListBox->blockSignals(false);

        // Update the selection properties and summary so it shows the results for
        // the current selection.
        UpdateWindowSingleItem();
    }

    if(SelectedSubject() == plotList || 
       SelectedSubject() == windowInformation ||
       doAll)
    {
        QString pName, sName;
        NewEnabled(pName, sName);

        bool enabled = !sName.isEmpty() ||
                       !pName.isEmpty();

        newButton->setEnabled(enabled);
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::GetCurrentValues
//
// Purpose: 
//   Gets the values from text widgets
//
// Arguments:
//   which_widget : The widget that we need to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 10:30:14 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 15:33:08 PDT 2011
//   Make sure we read back the variable min/max values.
//
// ****************************************************************************

void
QvisSelectionsWindow::GetCurrentValues(int which_widget)
{
    bool doAll = which_widget == -1;

    if(which_widget == SelectionProperties::ID_minTimeState || doAll)
    {
        int val;
        if(LineEditGetInt(cqTimeMin, val))
            selectionProps.SetMinTimeState(val);
        else
            ResettingError(tr("minimum time state"), IntToQString(selectionProps.GetMinTimeState()));
    }

    if(which_widget == SelectionProperties::ID_maxTimeState || doAll)
    {
        int val;
        if(LineEditGetInt(cqTimeMax, val))
            selectionProps.SetMaxTimeState(val);
        else if(cqTimeMax->text().contains("max"))
            selectionProps.SetMaxTimeState(-1);
        else
            ResettingError(tr("maximum time state"), IntToQString(selectionProps.GetMaxTimeState()));
    }

    if(which_widget == SelectionProperties::ID_timeStateStride || doAll)
    {
        int val;
        bool err = true;
        if(LineEditGetInt(cqTimeStride, val))
        {
            if(val >= 1)
            {
                selectionProps.SetTimeStateStride(val);
                err = false;
            }
        }

        if(err)
            ResettingError(tr("time stride"), IntToQString(selectionProps.GetTimeStateStride()));
    }

    if(which_widget == SelectionProperties::ID_variableMins ||
       which_widget == SelectionProperties::ID_variableMaxs || doAll)
    {
        for(int i = 0; i < cqLimits->getNumVariables(); ++i)
        {
            float r0, r1;
            cqLimits->getVariable(i)->getSelectedRange(r0, r1);

            if((size_t)i < selectionProps.GetVariableMins().size())
            {
                selectionProps.GetVariableMins()[i] = r0;
                selectionProps.GetVariableMaxs()[i] = r1;
                selectionProps.SelectVariableMins();
                selectionProps.SelectVariableMaxs();
            }
        }
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Mon Aug 22 16:42:43 PDT 2011
//   I added an updatePlots argument.
//
//   Brad Whitlock, Wed Sep  7 15:45:27 PDT 2011
//   I added a caching argument.
//
// ****************************************************************************

void
QvisSelectionsWindow::Apply(bool forceUpdate, bool updatePlots, bool caching)
{
    allowCaching &= caching;

    if(forceUpdate || AutoUpdate())
    {
        GetCurrentValues(-1);

        if(selectionListBox->currentItem() != 0)
        {
            GetViewerMethods()->UpdateNamedSelection(selectionProps.GetName(), 
                selectionProps, updatePlots, allowCaching);
        }

        allowCaching = true;
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::GetLoadHost
//
// Purpose: 
//   Get the host to use for the load button.
//
// Arguments:
//
// Returns:    The most likely host name to use for loading a selection.
//
// Note:       We use the first selected plot's host name as the selection
//             load host. Otherwise, we use the only engine's host name.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 11 15:15:51 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

QString
QvisSelectionsWindow::GetLoadHost() const
{
    QString loadHost; 

    int index = plotList->FirstSelectedIndex();
    if(index != -1)
    {
        QualifiedFilename db(plotList->GetPlots(index).GetDatabaseName());
        loadHost = QString(db.host.c_str());
    }
    if(loadHost.isEmpty())
    {
        const stringVector &engines = engineList->GetEngineName();
        if(engines.size() == 1) 
            loadHost = QString(engines[0].c_str());
    }

    // If loadHost == the expanded localhost name then we want to
    // just return localhost. This prevents us from opening an extra mdserver
    // when we want to open a selection on localhost.
    if(GetViewerProxy()->GetLocalHostName() == loadHost.toStdString())
        loadHost = "localhost";

    return loadHost;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateHistogram
//
// Purpose: 
//   Update the histogram on the histogram tab.
//
// Programmer: Brad Whitlock
// Creation:   Sat May 21 00:53:37 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::UpdateHistogram(const double *values, int nvalues, 
                                      int minBin, int maxBin, bool useBins,
                                      double minAxisValue, double maxAxisValue )
{
    if(values == 0 || nvalues == 0)
    {
        cqHistogram->setHistogramTexture(0,0);

        cqHistogramMinAxisLabel->setText(tr(""));
        cqHistogramMaxAxisLabel->setText(tr(""));
        cqHistogramAxisLabel->setText(tr(""));
    }
    else
    {
        if( nvalues != selectionProps.GetHistogramNumBins() )
        {
          selectionProps.SetHistogramNumBins(nvalues);
          selectionProps.SetHistogramStartBin(0);
          selectionProps.SetHistogramEndBin(nvalues-1);

          minBin = 0;
          maxBin = nvalues-1;
          
          Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
          
          UpdateMinMaxBins(true, true, true);
        }

        float *normalized = new float[nvalues];
        bool  *mask = new bool[nvalues];
        double minVal = values[0];
        double maxVal = values[0];
        for(int i = 1; i < nvalues; ++i)
        {
          if( minVal > values[i] )
            minVal = values[i];

          if( maxVal < values[i] )
            maxVal = values[i];
        }

        for(int i = 0; i < nvalues; ++i)
        {
            if( maxVal != 0 )
              normalized[i] = values[i] / maxVal;
            else
              normalized[i] = 0;

            mask[i] = (minBin <= i && i <= maxBin);
        }

        cqHistogram->setHistogramTexture(normalized, useBins ? mask : 0,
                                         nvalues);

        delete [] normalized;
        delete [] mask;

        std::stringstream minstr;
        minstr << std::string("Bin 0 (min = ") << minAxisValue << ")";
        cqHistogramMinAxisLabel->setText(tr(minstr.str().c_str()));

        
        std::stringstream maxstr;
        maxstr << std::string("Bin ")
                              << (selectionProps.GetHistogramNumBins() - 1)
                              << " (max = " << maxAxisValue << ")";
        cqHistogramMaxAxisLabel->setText(tr(maxstr.str().c_str()));
        
        std::stringstream labelstr;

        if(selectionProps.GetHistogramType() ==
           SelectionProperties::HistogramID)
          labelstr << std::string( "         Bin average;  min : " );
        else
          labelstr << std::string( "         Bin totals;  min : " );

        labelstr << minVal
                 << std::string( "    max : " ) << maxVal;
                                 
        cqHistogramAxisLabel->setText(tr(labelstr.str().c_str()));
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateHistogram
//
// Purpose: 
//   Update the histogram on the histogram tab.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:58:43 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::UpdateHistogram()
{ 
    int sumIndex = selectionList->GetSelectionSummary(selectionProps.GetName());

    // Histogram controls
    if(sumIndex >= 0)
    {
        const SelectionSummary &summary =
          selectionList->GetSelectionSummary(sumIndex);

        const doubleVector &hist = summary.GetHistogramValues();

        if(hist.empty())
          UpdateHistogram(0,0,0,0,false,0,0);
        else
        {
            UpdateHistogram(&hist[0], (int)hist.size(), 
                            selectionProps.GetHistogramStartBin(),
                            selectionProps.GetHistogramEndBin(),
                            true,
//                          selectionProps.GetHistogramType() != SelectionProperties::HistogramTime,
                            summary.GetHistogramMinBin(),
                            summary.GetHistogramMaxBin() );
        }
    }
    else
    {
      UpdateHistogram(0,0,0,0,false,0,0);
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateHistogramTitle
//
// Purpose: 
//   Update the histogram title based on selectionProps.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:59:07 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::UpdateHistogramTitle()
{
    if(selectionProps.GetSelectionType() != 
        SelectionProperties::CumulativeQuerySelection)
        cqHistogramTitle->setText("");
    else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramTime)
        cqHistogramTitle->setText(tr("Number of Cells vs. Time"));
    else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramMatches)
        cqHistogramTitle->setText(tr("Frequency vs. Matches"));
    else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramID)
        cqHistogramTitle->setText(tr("Average frequency vs. ID"));
    else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable)
        cqHistogramTitle->setText(tr("Frequency vs. Variable"));
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateMinMaxBins
//
// Purpose: 
//   Update the min and max limits for the histogram start/end spin boxes.
//
// Arguments:
//   updateMin : Whether to update the min widgets.
//   updateMax : Whether to update the max widgets.
//   updateValues : Whether to update the values.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:59:35 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::UpdateMinMaxBins(bool updateMin, bool updateMax, 
    bool updateValues)
{
//    bool notTime = selectionProps.GetHistogramType() != SelectionProperties::HistogramTime;

    // Set the min value and extents.
    int hMin = 0, hMax = 100000;
    if(updateMin)
    {
        int val = selectionProps.GetHistogramStartBin();
        if(val < 0)
            val = 0;
//        if(notTime)
        {
            hMin = 0;
            hMax = qMin(selectionProps.GetHistogramEndBin(),
                        selectionProps.GetHistogramNumBins()-1);
        }
        cqHistogramMin->blockSignals(true);
        cqHistogramMin->setRange(hMin, hMax);

        if(updateValues)
            cqHistogramMin->setValue(val);
        cqHistogramMin->blockSignals(false);
//         cqHistogramMin->setEnabled(notTime);
//         cqHistogramMinLabel->setEnabled(notTime);
    }

    if(updateMax)
    {
        // Set the max value and extents.
        int val = selectionProps.GetHistogramEndBin();
        if(val >= selectionProps.GetHistogramNumBins()-1)
            val = selectionProps.GetHistogramNumBins()-1;
//        if(notTime)
        {
            hMin = qMax(selectionProps.GetHistogramStartBin(), 0);
            hMax = selectionProps.GetHistogramNumBins()-1;
        }
        cqHistogramMax->blockSignals(true);
        cqHistogramMax->setRange(hMin, hMax);

        if(updateValues)
            cqHistogramMax->setValue(val);
        cqHistogramMax->blockSignals(false);
//         cqHistogramMax->setEnabled(notTime);
//         cqHistogramMaxLabel->setEnabled(notTime);
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateSelectionProperties
//
// Purpose: 
//   Update the property controls using the selectionProps object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:14:20 PST 2010
//
// Modifications:
//   Brad Whitlock, Mon Nov  7 14:11:28 PST 2011
//   I added code to set the id type and variable.
//
//   Dave Pugmire, Thu Mar 15 11:19:33 EDT 2012
//   Add location named selections.
//
// ****************************************************************************

void
QvisSelectionsWindow::UpdateSelectionProperties()
{
    UpdateHistogramTitle();

    if (!selectionPropsValid)
    {
        plotNameLabel->setText(tr("none"));

        idVariableType->blockSignals(true);
        idVariableType->button(0)->setChecked(true);
        idVariableType->blockSignals(false);

        idVariableButton->blockSignals(true);
        idVariableButton->setVariable(QString());
        idVariableButton->blockSignals(false);

        // Range controls
        cqControls->blockSignals(true);
        cqControls->setChecked(false);
        cqControls->blockSignals(false);

        cqLimits->setNumVariables(0);

        cqTimeMin->setText("");
        cqTimeMax->setText("");
        cqTimeStride->setText("");

        // Histogram controls
        UpdateHistogram(0,0,0,0,false,0,0);

        cqHistogramType->blockSignals(true);
        cqHistogramType->button(0)->setChecked(true);
        cqHistogramType->blockSignals(false);
        cqHistogramVariableButton->setEnabled(true);

        SelectionProperties defaults;

        std::stringstream nbins;
        nbins << (defaults.GetHistogramNumBins() - 1);
        std::string tmpstr = std::string("Bin ") + nbins.str() + " (0)";
        cqHistogramMaxAxisLabel->setText(tr(tmpstr.c_str()));
 
        cqHistogramNumBins->blockSignals(true);
        cqHistogramNumBins->setValue(defaults.GetHistogramNumBins());
        cqHistogramNumBins->blockSignals(false);

        cqHistogramMin->blockSignals(true);
        cqHistogramMin->setValue(defaults.GetHistogramStartBin());
        cqHistogramMin->blockSignals(false);

        cqHistogramMax->blockSignals(true);
        cqHistogramMax->setValue(defaults.GetHistogramEndBin());
        cqHistogramMax->blockSignals(false);

        cqSummation->blockSignals(true);
        cqSummation->setCurrentIndex(0);
        cqSummation->blockSignals(false);
    }
    else
    {
        // Update the window based on the working copy of the selection 
        // properties.
        if(selectionProps.GetSource().empty())
            plotNameLabel->setText(tr("none"));
        else
        {
            QString plotDesc(GetPlotDescription(selectionProps.GetSource().c_str()));
            if(plotDesc.isEmpty())
                plotNameLabel->setText(selectionProps.GetSource().c_str());
            else
                plotNameLabel->setText(plotDesc);
        }

        int idvar = 0;
        if(selectionProps.GetIdVariableType() == SelectionProperties::UseZoneIDForID)
            idvar = 0;
        else if(selectionProps.GetIdVariableType() == SelectionProperties::UseGlobalZoneIDForID)
            idvar = 1;
        else if(selectionProps.GetIdVariableType() == SelectionProperties::UseLocationsForID)
            idvar = 2;
        else if(selectionProps.GetIdVariableType() == SelectionProperties::UseVariableForID)
            idvar = 3;
        idVariableType->blockSignals(true);
        idVariableType->button(idvar)->setChecked(true);
        idVariableType->blockSignals(false);

        idVariableButton->blockSignals(true);
        idVariableButton->setVariable(QString(selectionProps.GetIdVariable().c_str()));
        idVariableButton->blockSignals(false);
        idVariableButton->setEnabled(idvar == 2);

        cqControls->blockSignals(true);
        cqControls->setChecked(selectionProps.GetSelectionType() == 
            SelectionProperties::CumulativeQuerySelection);
        cqControls->blockSignals(false);

        //
        // Set the variable limits
        //
        int sumIndex = selectionList->GetSelectionSummary(selectionProps.GetName());
        int nvars = (int)selectionProps.GetVariables().size();
        cqLimits->setNumVariables(nvars);
        for(int i = 0; i < nvars; ++i)
        {
            const std::string &varname = selectionProps.GetVariables()[i];

            QvisHistogramLimits *limits = cqLimits->getVariable(i);
            limits->setVariable(varname.c_str());
            // reset
            limits->invalidateTotalRange();
            limits->setHistogram(0,0);

            float r0 = (float)selectionProps.GetVariableMins()[i];
            float r1 = (float)selectionProps.GetVariableMaxs()[i];
            limits->setSelectedRange(r0, r1);

            // Try and get the variable total min/max and histogram from 
            // the selection summary if it exists.
            if(sumIndex >= 0)
            {
                const SelectionSummary &summary = selectionList->
                    GetSelectionSummary(sumIndex);
                for(int s = 0; s < summary.GetNumVariables(); ++s)
                {
                    const SelectionVariableSummary &vsummary = 
                        summary.GetVariables(s);
                    if(vsummary.GetName() == varname)
                    {
                        limits->setTotalRange(vsummary.GetMinimum(),
                                              vsummary.GetMaximum());

                        float hist[256];
                        vsummary.NormalizedHistogram(hist);
                        limits->setHistogram(hist, 256);
                        break;
                    }
                }
            }
        }

        cqTimeMin->setText(QString().setNum(selectionProps.GetMinTimeState()));
        int maxts = selectionProps.GetMaxTimeState();
        if(maxts == -1)
        {
            // Get the max value from the plot database

            // For now:
            cqTimeMax->setText("max");
        }
        else
            cqTimeMax->setText(QString().setNum(maxts));
        cqTimeStride->setText(QString().setNum(selectionProps.GetTimeStateStride()));

        // Update the histogram controls
        UpdateHistogram();

        cqHistogramType->blockSignals(true);
        if(selectionProps.GetHistogramType() == SelectionProperties::HistogramTime)
            cqHistogramType->button(0)->setChecked(true);
        else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramMatches)
            cqHistogramType->button(1)->setChecked(true);
        else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramID)
            cqHistogramType->button(2)->setChecked(true);
        else if(selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable)
            cqHistogramType->button(3)->setChecked(true);
        cqHistogramType->blockSignals(false);
        cqHistogramVariableButton->setEnabled(!selectionProps.GetVariables().empty());

        cqHistogramVariable->blockSignals(true);
        cqHistogramVariable->setVariable(selectionProps.GetHistogramVariable().c_str());
        cqHistogramVariable->blockSignals(false);
        cqHistogramVariable->setEnabled(!selectionProps.GetVariables().empty() &&
            selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable);

        cqHistogramNumBins->blockSignals(true);
        cqHistogramNumBins->setValue(selectionProps.GetHistogramNumBins());
        cqHistogramNumBins->blockSignals(false);
        bool setBins =
          selectionProps.GetHistogramType() == SelectionProperties::HistogramID ||
          selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable;
        cqHistogramNumBins->setEnabled(setBins);
        cqHistogramNumBinsLabel->setEnabled(setBins);

        UpdateMinMaxBins(true, true, true);

        cqSummation->blockSignals(true);
        cqSummation->setCurrentIndex((selectionProps.GetCombineRule() == 
            SelectionProperties::CombineOr) ? 0 : 1);
        cqSummation->blockSignals(false);
    }

    deleteButton->setEnabled(selectionPropsValid);
    saveButton->setEnabled(selectionPropsValid);

    // Set the enabled state of the load button.
    loadButton->setEnabled(!GetLoadHost().isEmpty());

    updateSelectionButton->setEnabled(selectionPropsValid);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateSelectionSummary
//
// Purpose: 
//   Update the selection summary controls using the summary named by selectionProps
//   and the summary stored in the selectionList.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 11:38:42 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::UpdateSelectionSummary()
{
    int sumIndex = -1;
    if(selectionPropsValid)
        sumIndex = selectionList->GetSelectionSummary(selectionProps.GetName());

    if(sumIndex >= 0)
    {
        const SelectionSummary &summary = selectionList->GetSelectionSummary(sumIndex);

        statVars->clear();
        statVars->setColumnCount(3);
        QStringList header;
        header << tr("Variable") 
               << tr("Minimum") 
               << tr("Maximum");
        statVars->setHorizontalHeaderLabels(header);

        statVars->setRowCount(summary.GetNumVariables());
        for(int i = 0; i < summary.GetNumVariables(); ++i)
        {
            const SelectionVariableSummary &vsummary = summary.GetVariables(i);
            QTableWidgetItem *item0 = new QTableWidgetItem(vsummary.GetName().c_str());
            QTableWidgetItem *item1 = new QTableWidgetItem(QString().setNum(vsummary.GetMinimum()));
            QTableWidgetItem *item2 = new QTableWidgetItem(QString().setNum(vsummary.GetMaximum()));

            item0->setFlags(Qt::ItemIsSelectable); // disable edits
            item1->setFlags(Qt::ItemIsSelectable);
            item2->setFlags(Qt::ItemIsSelectable);

            statVars->setItem(i, 0, item0);
            statVars->setItem(i, 1, item1);
            statVars->setItem(i, 2, item2);
        }

        statSelectedCells->setText(QString().setNum(summary.GetCellCount()));
        statTotalCells->setText(QString().setNum(summary.GetTotalCellCount()));
    }
    else
    {
        statVars->clear();
        statSelectedCells->setText("");
        statTotalCells->setText("");
    }
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSelectionsWindow::automaticallyApplyChanged
//
// Purpose: 
//   Sets the named selection auto apply mode.
//
// Arguments:
//   val : Whether the selections update automatically when plots change.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 11 16:22:37 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::automaticallyApplyChanged(bool val)
{
    GetViewerMethods()->SetNamedSelectionAutoApply(val);
}

// ****************************************************************************
//  Method:  QvisSelectionsWindow::UpdateWindowSingleItem
//
//  Purpose:
//    Update the pane of the window where a single selection is being edited.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Aug  6 15:44:09 PDT 2010
//
//  Modifications:
//    Brad Whitlock, Mon Oct 11 15:15:37 PDT 2010
//    Set the enabled state for the load button.
//
//    Brad Whitlock, Tue Dec 28 21:23:12 PST 2010
//    I added cumulative query support.
//
// ****************************************************************************

void
QvisSelectionsWindow::UpdateWindowSingleItem()
{
    int index = selectionListBox->currentRow();
    if (index <  0)
    {
        selectionPropsValid = false;
        selectionProps = SelectionProperties();
    }
    else
    {
        selectionPropsValid = true;
        // Copy the list's selection properties into the working copy.
        selectionProps = selectionList->GetSelections(index);
    }
    allowCaching = true;

    UpdateSelectionProperties();
    UpdateSelectionSummary();

    cqControls->setEnabled(index >= 0);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::addSelection
//
// Purpose: 
//   This is a Qt slot function that adds a new Selection that is empty.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 02:42:04 PDT 2011
//   I added support for different id types.
//
// ****************************************************************************

void
QvisSelectionsWindow::addSelection()
{
    // Find an unused Selection name
    bool okay = false;
    QString newName;
    while (!okay)
    {
        newName.sprintf("selection%d", selectionCounter);
        if(selectionList->GetSelection(newName.toStdString()) >= 0)
            selectionCounter++;
        else
            okay = true;
    }

    QString pName, sName;
    NewEnabled(pName, sName);

    QString selName, selSource;
    bool plotSource = false;
    QvisSelectionsDialog::IDVariableType idType = QvisSelectionsDialog::UseZoneIDForID;
    QString idVariable;

    int dlgRet = -1;
    if(pName.isEmpty() && sName.isEmpty())
    {
        // do nothing
    }
    else if(!pName.isEmpty() && !sName.isEmpty())
    {
        QString plotDesc(GetPlotDescription(pName));
        if(plotDesc.isEmpty())
            plotDesc = pName;

        QvisSelectionsDialog dlg(QvisSelectionsDialog::SOURCE_USE_DB_OR_PLOT, this);
        dlg.setSelectionName(newName);
        dlg.setPlotName(plotDesc);
        dlg.setDBName(sName);
        dlgRet = dlg.exec(selName, selSource, plotSource, idType, idVariable);

        if(plotSource)
            selSource = pName;
    }
    else if(!sName.isEmpty())
    {
        QvisSelectionsDialog dlg(QvisSelectionsDialog::SOURCE_USE_DB, this);
        dlg.setSelectionName(newName);
        dlg.setDBName(sName);
        dlgRet = dlg.exec(selName, selSource, plotSource, idType, idVariable);
    }

    if(dlgRet == QDialog::Accepted && !selName.isEmpty())
    {
        // Tell the viewer to add the new named selection based on the db
        SelectionProperties p;
        p.SetName(selName.toStdString());
        p.SetSource(selSource.toStdString());
        if(idType == QvisSelectionsDialog::UseZoneIDForID)
            p.SetIdVariableType(SelectionProperties::UseZoneIDForID);
        else if(idType == QvisSelectionsDialog::UseGlobalZoneIDForID)
            p.SetIdVariableType(SelectionProperties::UseGlobalZoneIDForID);
        else if(idType == QvisSelectionsDialog::UseLocationsForID)
            p.SetIdVariableType(SelectionProperties::UseLocationsForID);
        else if(idType == QvisSelectionsDialog::UseVariableForID)
        {
            if(idVariable.isEmpty())
            {
                Error(tr("The selection was not created because the "
                         "id variable was not set."));
                return;
            }
            p.SetIdVariableType(SelectionProperties::UseVariableForID);
            p.SetIdVariable(idVariable.toStdString());
        }
        GetViewerMethods()->CreateNamedSelection(p.GetName(), p, plotSource);

        // Create a temporary entry in the selectionListBox so the new selection
        // will get selected when its data comes from the viewer.
        selectionListBox->blockSignals(true);
        selectionListBox->addItem(selName);
        selectionListBox->setCurrentRow(selectionListBox->count()-1);
        selectionListBox->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::NewEnabled
//
// Purpose: 
//   Returns plot name and db name that can be used to create new selections.
//
// Arguments:
//   plotName : The name of the plot that might be used to generate a selection.
//   dbName   : The name of the database that might be used to generate a selection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:24:29 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::NewEnabled(QString &plotName, QString &dbName)
{
    // Determine if there is a plot to use.
    int index = plotList->FirstSelectedIndex();
    if(index != -1)
    {
        if(plotList->GetPlots(index).GetStateType() == Plot::Completed)
            plotName = QString(plotList->GetPlots(index).GetPlotName().c_str());
    }

    QString src(GetViewerState()->GetWindowInformation()->GetActiveSource().c_str());
    if(src != "notset")
        dbName = src;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::delSelection
//
// Purpose: 
//   This is a Qt slot function that is called to delete a Selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::deleteSelection()
{
    if(selectionListBox->currentItem() != 0)
    {
        GetViewerMethods()->DeleteNamedSelection(selectionListBox->currentItem()->
            text().toStdString());
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::updateQuery
//
// Purpose: 
//   This is a Qt slot function that is called to update a Selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Tue Dec 28 22:19:05 PST 2010
//   Send the selection properties down.
//
// ****************************************************************************

void
QvisSelectionsWindow::updateQuery()
{
    // Force an update of the selection but do not update the plots that use it.
    bool updatePlots = false;
    Apply(true, updatePlots, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::updateSelection
//
// Purpose: 
//   This is a Qt slot function that is called to update a Selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Tue Dec 28 22:19:05 PST 2010
//   Send the selection properties down.
//
// ****************************************************************************

void
QvisSelectionsWindow::updateSelection()
{
    // Force an update of the selection and update the plots that use it.
    Apply(true, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::loadSelection
//
// Purpose: 
//   This is a Qt slot function that is called to load a selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::loadSelection()
{
    QString loadHost(GetLoadHost());
    if(loadHost.isEmpty())
    {
        Warning(tr("VisIt can't determine the host for the compute engine to "
                   "use for loading the selection").arg(loadHost));
    }
    else
    {
        // Get the list of files at ~/.visit on the remote side that end with .ns
        QString loadFile(loadHost + ":~/.visit/");
        QString selName = QvisFileOpenDialog::getOpenFileName(loadFile,
            "*.ns", tr("Load selection from file"));
        if(!selName.isEmpty())
        {
            QualifiedFilename f(selName.toStdString());
            GetViewerMethods()->LoadNamedSelection(f.filename.substr(0, f.filename.size()-3),
                                                   loadHost.toStdString(), "");
        }
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::saveSelection
//
// Purpose: 
//   This is a Qt slot function that is called to save a selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::saveSelection()
{
    if(selectionListBox->currentItem() != 0)
    {
        GetViewerMethods()->SaveNamedSelection(selectionListBox->currentItem()->
            text().toStdString());
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::highlightSelection
//
// Purpose: 
//   Slot function that highlights a specific selection in the list.
//
// Arguments:
//   selName : The name of the selection to highlight.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 11 10:52:48 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::highlightSelection(const QString &selName)
{
    QList<QListWidgetItem *> items = selectionListBox->findItems(selName, Qt::MatchFixedString);
    if(items.count() > 0)
        selectionListBox->setCurrentItem(items.first());
}

// ****************************************************************************
// Method: QvisSelectionsWindow::cumulativeQueryClicked
//
// Purpose: 
//   Change the selection type to/from CQ.
//
// Arguments:
//   value : True for CQ selection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:02:03 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::cumulativeQueryClicked(bool value)
{
    selectionProps.SetSelectionType(value ? SelectionProperties::CumulativeQuerySelection :
        SelectionProperties::BasicSelection);
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::addVariable
//
// Purpose: 
//   Add a new variable to the CQ selection.
//
// Arguments:
//   var : The variable to add to the selection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:02:31 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::addVariable(const QString &var)
{
    selectionProps.GetVariables().push_back(var.toStdString());
    selectionProps.GetVariableMins().push_back(SelectionProperties::MIN);
    selectionProps.GetVariableMaxs().push_back(SelectionProperties::MAX);

    // Update the window using the new selectionProps.
    UpdateSelectionProperties();
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::setVariableRange
//
// Purpose: 
//   Set the variable range for the specified variable into the selection.
//
// Arguments:
//   var : The variable for which we want to set the range.
//   r0  : The min range.
//   r1  : The max range.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:02:59 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::setVariableRange(const QString &var, float r0, float r1)
{
    const stringVector &vars = selectionProps.GetVariables();
    for(size_t i = 0; i < vars.size(); ++i)
    {
        if(vars[i] == var.toStdString())
        {
            selectionProps.GetVariableMins()[i] = r0;
            selectionProps.GetVariableMaxs()[i] = r1;
            Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
            return;
        }
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::deleteVariable
//
// Purpose: 
//   Remove a variable from the CQ selection.
//
// Arguments:
//   var : The variable to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:03:46 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::deleteVariable(const QString &var)
{
    const stringVector &vars = selectionProps.GetVariables();
    for(size_t i = 0; i < vars.size(); ++i)
    {
        if(vars[i] == var.toStdString())
        {
            stringVector newvar;
            doubleVector newmin, newmax;
            for(size_t j = 0; j < vars.size(); ++j)
            {
                if(j == i)
                    continue;
                newvar.push_back(vars[j]);
                newmin.push_back(selectionProps.GetVariableMins()[j]);
                newmax.push_back(selectionProps.GetVariableMaxs()[j]);
            }
            selectionProps.SetVariables(newvar);
            selectionProps.SetVariableMins(newmin);
            selectionProps.SetVariableMaxs(newmax);

            Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
            return;
        }
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::summationChanged
//
// Purpose: 
//   Set the summation mode for the selection.
//
// Arguments:
//   val : The new "summation" mode.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:05:50 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::summationChanged(int val)
{
    if(val == 0)
        selectionProps.SetCombineRule(SelectionProperties::CombineOr);
    else
        selectionProps.SetCombineRule(SelectionProperties::CombineAnd);
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::processTimeMin
//
// Purpose: 
//   This Qt slot function is called when we need to process the min time.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:06:27 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::processTimeMin()
{
    GetCurrentValues(SelectionProperties::ID_minTimeState);
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::processTimeMax
//
// Purpose: 
//   This Qt slot function is called when we need to process the max time.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:06:27 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::processTimeMax()
{
    GetCurrentValues(SelectionProperties::ID_maxTimeState);
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::processTimeStride
//
// Purpose: 
//   This Qt slot function is called when we need to process the time stride.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:06:27 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::processTimeStride()
{
    GetCurrentValues(SelectionProperties::ID_timeStateStride);
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::initializeVariableList
//
// Purpose: 
//   Tell the viewer to get the selection variables from a ParallelCoordinates plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:07:57 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::initializeVariableList()
{
    Apply(true, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);

    // Ask the viewer to populate the selection's variables using the
    // current plot's attributes.
    GetViewerMethods()->InitializeNamedSelectionVariables(selectionProps.GetName());
}

// ****************************************************************************
// Method: QvisSelectionsWindow::histogramTypeChanged
//
// Purpose: 
//   This Qt slot function is called when we change the histogram type.
//
// Arguments:
//   value : The new histogram type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:06:27 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:27:41 PDT 2011
//   I removed some error checking that was no longer needed.
//
// ****************************************************************************

void
QvisSelectionsWindow::histogramTypeChanged(int value)
{
    switch(value)
    {
    case 0:
        selectionProps.SetHistogramType(SelectionProperties::HistogramTime);
        break;
    case 1:
        selectionProps.SetHistogramType(SelectionProperties::HistogramMatches);
        break;
    case 2:
        selectionProps.SetHistogramType(SelectionProperties::HistogramID);
        break;
    case 3:
        selectionProps.SetHistogramType(SelectionProperties::HistogramVariable);
        break;
    }

    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
    UpdateHistogram(0,0,0,0,false,0,0); // invalidate the histogram
    UpdateHistogramTitle();

    cqHistogramVariableButton->setEnabled(!selectionProps.GetVariables().empty());
    cqHistogramVariable->setEnabled(!selectionProps.GetVariables().empty() &&
            selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable);
    bool setBins =
      selectionProps.GetHistogramType() == SelectionProperties::HistogramID ||
      selectionProps.GetHistogramType() == SelectionProperties::HistogramVariable;
    cqHistogramNumBins->setEnabled(setBins);
    cqHistogramNumBinsLabel->setEnabled(setBins);

    UpdateMinMaxBins(true, true, true);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::histogramVariableChanged
//
// Purpose: 
//   This Qt slot function is called when we change the histogram variable.
//
// Arguments:
//   var : The histogram variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:08:26 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:25:54 PDT 2011
//   I changed the histogram variable to a string.
//
// ****************************************************************************

void
QvisSelectionsWindow::histogramVariableChanged(const QString &var)
{
    selectionProps.SetHistogramVariable(var.toStdString());
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::histogramNumBinsChanged
//
// Purpose: 
//   This Qt slot is called when we adjust the number of histogram bins.
//
// Arguments:
//   index : The new number of bins.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:09:28 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::histogramNumBinsChanged(int index)
{
    selectionProps.SetHistogramNumBins(index);

    // Adjust the min,max if needed
    int b0 = selectionProps.GetHistogramStartBin();
    int b1 = selectionProps.GetHistogramEndBin();
    if(b0 >= selectionProps.GetHistogramNumBins())
        b0 = selectionProps.GetHistogramNumBins()-1;
    if(b1 >= selectionProps.GetHistogramNumBins())
        b1 = selectionProps.GetHistogramNumBins()-1;

    selectionProps.SetHistogramStartBin(b0);
    selectionProps.SetHistogramEndBin(b1);

    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);

    UpdateMinMaxBins(true, true, true);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::histogramStartChanged
//
// Purpose: 
//   This Qt slot is called when we adjust the histogram start bin.
//
// Arguments:
//   index : The new start bin.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:10:11 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::histogramStartChanged(int index)
{
    selectionProps.SetHistogramStartBin(index);

    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);

    UpdateMinMaxBins(false, true, false);
    UpdateHistogram();
}

// ****************************************************************************
// Method: QvisSelectionsWindow::histogramEndChanged
//
// Purpose: 
//   This Qt slot is called when we adjust the histogram end bin.
//
// Arguments:
//   index : The new end bin.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 16:10:11 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::histogramEndChanged(int index)
{
    selectionProps.SetHistogramEndBin(index);

    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
    UpdateMinMaxBins(true, false, false);
    UpdateHistogram();
}

// ****************************************************************************
// Method: QvisSelectionsWindow::idVariableChanged
//
// Purpose: 
//   This slot is called when we change the id variable.
//
// Arguments:
//   var : The new id variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 14:18:45 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::idVariableChanged(const QString &var)
{
    selectionProps.SetIdVariable(var.toStdString());
    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, ALLOW_CACHING);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::idVariableTypeChanged
//
// Purpose: 
//   This slot is called when we change the id variable type.
//
// Arguments:
//   val : The new id variable type.
//
// Note:       We say no caching because we probably need to reexecute the pipeline.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 14:20:13 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::idVariableTypeChanged(int val)
{
    if(val == 0)
        selectionProps.SetIdVariableType(SelectionProperties::UseZoneIDForID);
    else if(val == 1)
        selectionProps.SetIdVariableType(SelectionProperties::UseGlobalZoneIDForID);
    else if(val == 2)
        selectionProps.SetIdVariableType(SelectionProperties::UseLocationsForID);
    else if(val == 3)
        selectionProps.SetIdVariableType(SelectionProperties::UseVariableForID);

    idVariableButton->setEnabled(
        selectionProps.GetIdVariableType() == SelectionProperties::UseVariableForID);

    Apply(DEFAULT_FORCE_UPDATE, DEFAULT_UPDATE_PLOTS, DONT_ALLOW_CACHING);
}
