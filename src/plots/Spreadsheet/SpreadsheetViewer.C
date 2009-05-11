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
#include <SpreadsheetViewer.h>

#include <float.h>
#include <cassert>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <SpreadsheetTable.h>

// Need these?
#include <QCloseEvent>

#include <QvisColorTableButton.h>
#include <QvisVariableButton.h>
#include <plugin_vartypes.h>

#include <PlotList.h>
#include <Plot.h>

#include <SpreadsheetCurveViewer.h>
#include <SpreadsheetTable.h>
#include <SpreadsheetTabWidget.h>
#include <avtLookupTable.h>
#include <avtDatabaseMetaData.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <vtkVisItUtility.h>
#include <vtkVisItCellLocator.h>

#include <ViewerPlot.h>
#include <ViewerMethods.h>
#include <ViewerState.h>
#include <DebugStream.h>

#include <Subject.h>
#include <SpreadsheetAttributes.h>

#define plotAtts ((SpreadsheetAttributes *)plot->GetPlotAtts())

// Since the Mac's tab bars don't have limits on their width, make the
// window use just a single tab to display data on the Mac.
#ifdef Q_WS_MACX
#define SINGLE_TAB_WINDOW
#endif

// ****************************************************************************
// Method: SpreadsheetViewer::SpreadsheetViewer
//
// Purpose: 
//   Constructor for the SpreadsheetViewer class.
//
// Arguments:
//   p      : The ViewerPlot object that will be tied to this display.
//   parent : The parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:00:14 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 17:30:31 PST 2007
//   Changed layout of window so the tracer plane can be turned off.
//
//   Gunther H. Weber, Thu Sep 27 13:33:36 PDT 2007
//   Add support for setting spreadsheet font
//
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Wed Nov 28 15:20:58 PST 2007
//   Added toggle for showing current cell outline
//
//   Brad Whitlock, Tue Apr 22 10:25:39 PDT 2008
//   Make the menu options be buttons on the Mac since the viewer is not
//   allowed to make a menu.
//
//   Brad Whitlock, Wed Apr 23 11:12:31 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Mon Aug 11 16:13:52 PDT 2008
//   Qt 4.
//
// ****************************************************************************

SpreadsheetViewer::SpreadsheetViewer(ViewerPlot *p, QWidget *parent) :
    QMainWindow(parent), Observer((Subject*)p->GetPlotAtts()), 
    cachedAtts(), menuPopulator()
{
    // Initialize members.
    input = 0;
    allowRender = true;
    plot = p;
    minCell[0] = minCell[1] = minCell[2] = -1;
    minValue = 0.;
    maxCell[0] = maxCell[1] = maxCell[2] = -1;
    maxValue = 0.;
    colorLUT  = new avtLookupTable;
    colorLUT->SetColorTable("hot", true);
    sliding = false;

    // Create widgets.
    setWindowTitle(tr("Spreadsheet"));

    QFrame *top = new QFrame(this);
    setCentralWidget(top);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    topLayout->setSpacing(5);
    topLayout->setMargin(10);
#ifdef Q_WS_MAC
    QWidget *menuContainer = new QWidget(top);
    QHBoxLayout *menuLayout = new QHBoxLayout(menuContainer);
    topLayout->addWidget(menuContainer);
#endif
    QHBoxLayout *layout = new QHBoxLayout(0);
    topLayout->addLayout(layout);
    layout->setSpacing(5);

    //
    // 3D controls
    //
    controls3D = new QGroupBox(tr("3D"), top);
    layout->addWidget(controls3D, 10);
    QVBoxLayout *inner3D = new QVBoxLayout(controls3D);
    inner3D->addSpacing(10);
    inner3D->setMargin(10);
    QGridLayout *layout3D = new QGridLayout(0);
    inner3D->addLayout(layout3D);
    layout3D->setSpacing(5);
    inner3D->addStretch(1);

    kLabel = new QLabel("k [1,1]", controls3D);
    layout3D->addWidget(kLabel, 0, 0);

    kSlider = new QSlider(controls3D);
    kSlider->setOrientation(Qt::Horizontal);
    kSlider->setPageStep(1);
    connect(kSlider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(kSlider, SIGNAL(sliderPressed()),
            this, SLOT(sliderPressed()));
    connect(kSlider, SIGNAL(sliderReleased()),
            this, SLOT(sliderReleased()));

    layout3D->addWidget(kSlider, 0, 1, 1, 2);

    normalLabel = new QLabel(tr("Normal"), controls3D);
    layout3D->addWidget(normalLabel, 1, 0);

    normalButtonGroup = new QButtonGroup (0);
    connect(normalButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(normalChanged(int)));
    normalRadioButtons = new QWidget(controls3D);
    QHBoxLayout *nLayout = new QHBoxLayout(normalRadioButtons);
    nLayout->setMargin(0);
    layout3D->addWidget(normalRadioButtons, 1, 1);
    QRadioButton *rb = new QRadioButton(tr("X"), normalRadioButtons);
    normalButtonGroup->addButton(rb, 0);
    nLayout->addWidget(rb);
    rb = new QRadioButton(tr("Y"), normalRadioButtons);
    normalButtonGroup->addButton(rb, 1);
    nLayout->addWidget(rb);
    rb = new QRadioButton(tr("Z"), normalRadioButtons);
    normalButtonGroup->addButton(rb, 2);
    nLayout->addWidget(rb);
    nLayout->setStretchFactor(rb, 5);

    //
    // Display controls
    //
    QGroupBox *display = new QGroupBox(tr("Display"), top);
    layout->addWidget(display);
    QGridLayout *layoutDisplay = new QGridLayout(display);

    formatLabel = new QLabel(tr("Format"), display);
    layoutDisplay->addWidget(formatLabel, 0, 0);
    formatLineEdit = new QLineEdit(display);
    connect(formatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(formatChanged()));
    layoutDisplay->addWidget(formatLineEdit, 0, 1);

    colorTableCheckBox = new QCheckBox(tr("Color"), display);
    connect(colorTableCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(colorTableCheckBoxToggled(bool)));
    layoutDisplay->addWidget(colorTableCheckBox, 1, 0);

    // Just a push button for now. It will be a color table button later.
    colorTableButton = new QvisColorTableButton(display);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(selectedColorTable(bool, const QString &)));
    layoutDisplay->addWidget(colorTableButton, 1, 1);

    //
    // Show in viswindow controls
    //
    QGroupBox *show = new QGroupBox(tr("Show in visualization window"), top);
    topLayout->addWidget(show);
    QHBoxLayout *sLayout = new QHBoxLayout(show);
    tracerCheckBox = new QCheckBox(tr("Tracer plane"), show);
    sLayout->addWidget(tracerCheckBox);
    connect(tracerCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(tracerCheckBoxToggled(bool)));
    patchOutlineCheckBox = new QCheckBox(tr("Patch outline"), show);
    sLayout->addWidget(patchOutlineCheckBox);
    connect(patchOutlineCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(outlineCheckBoxToggled(bool)));
    currentCellOutlineCheckBox = new QCheckBox(tr("Current cell outline"), show);
    sLayout->addWidget(currentCellOutlineCheckBox);
    connect(currentCellOutlineCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showCurrentCellOutlineCheckBoxToggled(bool)));

    //
    // Tables
    //
    zTabs = new SpreadsheetTabWidget(top);
    topLayout->addWidget(zTabs, 10);
    nTables = 1;
    nTablesForSlider = 1;
    tables = new SpreadsheetTable*[1];
    tables[0] = new SpreadsheetTable(0);
    tables[0]->setLUT(colorLUT);
    QFont spreadsheetFont;
    if (spreadsheetFont.fromString(plotAtts->GetSpreadsheetFont().c_str()))
        tables[0]->setFont(spreadsheetFont);

    connect(tables[0], SIGNAL(selectionChanged()),
            this, SLOT(tableSelectionChanged()));
    zTabs->addTab(tables[0], "k=1");
    connect(zTabs, SIGNAL(currentChanged(int)),
            this, SLOT(tabChanged(int)));

    //
    // Variables and min,max buttons
    //
    QGridLayout *varLayout = new QGridLayout(0);
    topLayout->addLayout(varLayout);
    varLayout->setSpacing(5);
    varLayout->setColumnStretch(1, 5);
    varLayout->setColumnStretch(2, 5);
    varLabel = new QLabel(tr("Variable"), top);
    varLayout->addWidget(varLabel, 0, 0);
    // Have to display metadata -- the list of variables.
    varButton = new QvisVariableButton(false, false, true, 
        QvisVariableButton::Scalars, top);
    connect(varButton, SIGNAL(activated(const QString &)),
            this, SLOT(changedVariable(const QString &)));
    varLayout->addWidget(varButton, 0, 1, 1, 2);

    // min, max buttons
    minButton = new QPushButton(tr("Min = "), top);
    connect(minButton, SIGNAL(clicked()),
            this, SLOT(minClicked()));
    varLayout->addWidget(minButton, 1,1);

    maxButton = new QPushButton(tr("Max = "), top);
    connect(maxButton, SIGNAL(clicked()),
            this, SLOT(maxClicked()));
    varLayout->addWidget(maxButton, 1,2);


    //
    // Do the main menu.
    //
    fileMenu = new QMenu(tr("&File"), this);
#ifdef Q_WS_MAC
    QPushButton *fileButton = new QPushButton(tr("&File"), menuContainer);
    menuLayout->addWidget(fileButton);
    fileButton->setMenu(fileMenu);
#else
    menuBar()->addMenu(fileMenu);
#endif
    fileMenu_SaveText = fileMenu->addAction(tr("Save as text . . ."), this, SLOT(saveAsText()), Qt::CTRL+Qt::Key_S);

    editMenu = new QMenu(tr("&Edit"), this);
#ifdef Q_WS_MAC
    QPushButton *editButton = new QPushButton(tr("&Edit"), menuContainer);
    menuLayout->addWidget(editButton);
    editButton->setMenu(editMenu);
#else
    menuBar()->addMenu(editMenu);
#endif
    editMenu_Copy = editMenu->addAction(tr("&Copy"), this, SLOT(copySelectionToClipboard()), Qt::CTRL+Qt::Key_C);
    editMenu->addSeparator();
    editMenu->addAction(tr("Select &All"), this, SLOT(selectAll()), Qt::CTRL+Qt::Key_A);
    editMenu->addAction(tr("Select &None"), this, SLOT(selectNone()), Qt::CTRL+Qt::Key_N);

    operationsMenu = new QMenu(tr("&Operations"), this);
#ifdef Q_WS_MAC
    opButton = new QPushButton(tr("&Operations"), menuContainer);
    menuLayout->addWidget(opButton);
    opButton->setMenu(operationsMenu);
    opButton->setEnabled(false);
#else
    menuBar()->addMenu(operationsMenu);
#endif
    operationsMenu->addAction(tr("Sum"), this, SLOT(operationSum()));
    operationsMenu->addAction(tr("Average"), this, SLOT(operationAverage()));
    operationsMenu->addAction(tr("Create curve: row vs. X"), this, SLOT(operationCurveX()));
    operationsMenu->addAction(tr("Create curve: column vs. Y"), this, SLOT(operationCurveY()));
    updateMenuEnabledState(0);
}

// ****************************************************************************
// Method: SpreadsheetViewer::~SpreadsheetViewer
//
// Purpose: 
//   Destructor for the SpreadsheetViewer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:01:13 PST 2007
//
// Modifications:
//    Gunther H. Weber, Mon Feb  2 15:44:28 PST 2009
//    Changed signal/slot signature in disconnect call from
//    currentChanged(QWidget*) to currentChanged(int) since Qt 4 changed
//    the argument.
//
// ****************************************************************************

SpreadsheetViewer::~SpreadsheetViewer()
{
    input = 0;

    // Make sure that we don't cause any tabChanged signals when we later
    // delete the tabs so disconnect the signal.
    disconnect(zTabs, SIGNAL(currentChanged(int)),
               this, SLOT(tabChanged(int)));

    // Delete the button group since it has no parent.
    delete normalButtonGroup;

    // Delete the tables since they had no parents.
    for(int i = 0; i < nTables; ++i)
        delete tables[i];
    delete [] tables;

    delete colorLUT;
}

// ****************************************************************************
// Method: SpreadsheetViewer::setAllowRender
//
// Purpose: 
//   Sets whether the window will respond to render events from the vis window.
//
// Arguments:
//   val : True to make the window render when the vis window wants.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:01:29 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 17:29:40 PST 2007
//   Set the input to 0 since if we're here then we're rendering a new dataset
//   but we have not made it to the render method yet.
//
// ****************************************************************************

void
SpreadsheetViewer::setAllowRender(bool val)
{
    allowRender = val;
    input = 0;
}

// ****************************************************************************
// Method: SpreadsheetViewer::render
//
// Purpose: 
//   This method tells the window to update itself using the specified
//   VTK dataset.
//
// Note:       This method is called from the viewer plugin info's alternate
//             display methods to force the window to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:02:09 PST 2007
//
// Modifications:
//
//   Gunther H. Weber, Fri Sep 14 14:04:10 PDT 2007
//   Select appropriate picks the first time a data set is rendered (i.e.,
//   input is NULL).
// 
//   Brad Whitlock, Wed Apr 23 11:13:35 PDT 2008
//   Added tr().
//
//   Brad Whitlock, Tue Aug 26 15:33:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::render(vtkDataSet *ds)
{
    if(allowRender)
    {
        allowRender = false;

        show();
        raise();

        // If input is NULL then there may be picks in the attributes that need
        // to be highlighted 
        bool needPickUpdate = !input; 
        bool sliceIndexSet = false;

        // Set the input pointer
        input = ds;

        // Move slice (before updateSpreadsheet so that the it builds the table
        // for the proper slice)
        if (needPickUpdate)
            sliceIndexSet = moveSliceToCurrentPick();

        // Populate the spreadsheet.
        updateSpreadsheet();

        // Update selections in table now that it is built
        if (needPickUpdate)
            selectPickPoints();

        // Now that we've updated, change the slice index if that was changed too.
        if(sliceIndexSet)
        {
            if (plotAtts->GetSliceIndex() < nTables)
            {
                zTabs->blockSignals(true);
                zTabs->setCurrentIndex(plotAtts->GetSliceIndex());
                zTabs->blockSignals(false);
            }

            kSlider->blockSignals(true);
            kSlider->setValue(plotAtts->GetSliceIndex());
            kSlider->blockSignals(false);

            updateSliderLabel();
        }

        // Save the current plot attributes so we can compare them
        // against the ones when Update is called to see if we need
        // to update the spreadsheet.
        cachedAtts = *plotAtts;

        // Update the caption.
        QString caption = tr("Spreadsheet - %1: %2").
            arg(plot->GetVariableName().c_str()).
            arg(plotAtts->GetSubsetName().c_str());
        setWindowTitle(caption);

        // Set the variable in the variable button based on the plot's
        // active variable.
        updateVariableMenus();
        varButton->blockSignals(true);
        varButton->setVariable(plot->GetVariableName().c_str());
        varButton->blockSignals(false);

        // Update the menu enabled state.
        updateMenuEnabledState(zTabs->currentIndex());
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::updateVariableMenus
//
// Purpose: 
//   Updates the menu populator using the plot as input.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 18:07:31 PST 2007
//
// Modifications:
//   
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying to
//    PopulateVariableLists
// ****************************************************************************

void
SpreadsheetViewer::updateVariableMenus()
{
    // Update the variable menu, if needed.
    const bool treatAllDBsAsTimeVarying = false; // don't have ViewerFileServer
    if(menuPopulator.PopulateVariableLists(
        plot->GetSource(),
        plot->GetMetaData(),
        *plot->GetSILRestriction(),
        plot->GetViewerState()->GetExpressionList(),
	treatAllDBsAsTimeVarying))
    {
        QvisVariableButton::UpdatePlotSourceButtons(&menuPopulator);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::enterEvent
//
// Purpose: 
//   This method is called when the mouse enters the window.
//
// Arguments:
//   e : The event to handle.
//
// Note:       We use this method to make sure that the plot source variable
//             menus use the current menu populator.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 18:08:05 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::enterEvent(QEvent *e)
{
    QMainWindow::enterEvent(e);
    updateVariableMenus();
}

// ****************************************************************************
// Method: SpreadsheetViewer::closeEvent
//
// Purpose: 
//   Minimize the window instead of closing it.
//
// Arguments:
//   e : The event to handle.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 28 18:49:50 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::closeEvent(QCloseEvent *e)
{
    e->ignore();
    showMinimized();
}

// ****************************************************************************
// Method: SpreadsheetViewer::setColorTable
//
// Purpose: 
//   This method is called when the plot needs to tell the window the name of
//   the color table to use.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    True if the change in color table requires the plot to be
//             redrawn.
//
// Note:       This method is called from the Viewer version of the
//             avtSpreadsheetRenderer class to ensure that the Qt display
//             responds to color table changes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:03:20 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 16:00:36 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
SpreadsheetViewer::setColorTable(const char *ctName)
{
    bool namesMatch = (plotAtts->GetColorTableName() == std::string(ctName));
    bool colorTableChanged = false;

    if (plotAtts->GetColorTableName() == "Default")
        colorTableChanged = colorLUT->SetColorTable(NULL, namesMatch); 
    else
        colorTableChanged = colorLUT->SetColorTable(ctName, namesMatch); 

    if(colorTableChanged)
    {
        // Send a paint event to the currently visible page so its cells update
        if(zTabs->currentWidget() != 0)
            zTabs->currentWidget()->update();
    }

    return colorTableChanged;
}

// ****************************************************************************
// Method: SpreadsheetViewer::PickPointsChanged()
//
// Purpose: 
//    Determine based on current plotAtts and cached attributes if the list
//    of pick points changed.
//
// Returns:    Return true, if pick points changed and false otherwise.
//
// Programmer: Hank Childs
// Creation:   Mon Sep 10 15:05:01 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SpreadsheetViewer::PickPointsChanged() const
{
    bool changed = false;

    if (cachedAtts.GetCurrentPickValid() != plotAtts->GetCurrentPickValid())
    {
        changed = true;
    }
    else if (cachedAtts.GetPastPicks().size()!=plotAtts->GetPastPicks().size())
    {
        changed = true;
    }
    else
    {
        if (cachedAtts.GetCurrentPick()[0] != plotAtts->GetCurrentPick()[0] ||
            cachedAtts.GetCurrentPick()[1] != plotAtts->GetCurrentPick()[1] ||
            cachedAtts.GetCurrentPick()[2] != plotAtts->GetCurrentPick()[2])
        {
            changed = true;
        }
        int nvals = cachedAtts.GetPastPicks().size();
        for (int i = 0 ; i < nvals ; i++)
        {
            if (cachedAtts.GetPastPicks()[i] != plotAtts->GetPastPicks()[i])
            {
                changed = true;
            }
        }
    }
    return changed;
}

// ****************************************************************************
// Method: SpreadsheetViewer::Update
//
// Purpose: 
//   This method implements the Observer interface and allows the window to
//   display SpreadsheetAttributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:05:34 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//   
//   Hank Childs, Tue Sep  4 10:53:55 PDT 2007
//   Highlight and label pick points in spreadsheet. This functionality
//   required changes to handling other events as well (such as invalidating
//   the input pointer if the sub set name changes) to make the pick transfer
//   work properly.
//
//   Gunther H. Weber, Thu Sep 27 13:33:36 PDT 2007
//   Add support for setting spreadsheet font
//
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Wed Nov 28 15:20:58 PST 2007
//   Added toggle for showing current cell outline
//
//   Brad Whitlock, Tue Aug 26 15:38:24 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::Update(Subject *)
{
    if(sliding)
        return;

    bool needsUpdate = false;
    bool needsRebuild = false;
    bool sliceIndexSet = false;
    bool needsPickUpdate = false;
    for(int i = 0; i < plotAtts->NumAttributes(); ++i)
    {
        if(!plotAtts->IsSelected(i))
            continue;

        switch(i)
        {
        case SpreadsheetAttributes::ID_subsetName:
            if (cachedAtts.GetSubsetName() != plotAtts->GetSubsetName())
            {
                // Invalidate pointer to data set so that pick information
                // is not applied to the incorrect subset
                input = 0; 
            }
            break;
        case SpreadsheetAttributes::ID_formatString:
            formatLineEdit->setText(plotAtts->GetFormatString().c_str());

            // If we've changed format strings then we need to update the spreadsheet.
            if(cachedAtts.GetFormatString() != plotAtts->GetFormatString())
                needsUpdate = true;
            break;
        case SpreadsheetAttributes::ID_useColorTable:
            colorTableButton->setEnabled(plotAtts->GetUseColorTable());
            colorTableCheckBox->blockSignals(true);
            colorTableCheckBox->setChecked(plotAtts->GetUseColorTable());
            colorTableCheckBox->blockSignals(false);

            // If we've changed then we need to update the spreadsheet.
            if(cachedAtts.GetUseColorTable() != plotAtts->GetUseColorTable())
                 needsUpdate = true;
            break;
        case SpreadsheetAttributes::ID_colorTableName:
            colorTableButton->setText(plotAtts->GetColorTableName().c_str());

            // If we've changed then we need to update the spreadsheet.
            if(cachedAtts.GetColorTableName() != plotAtts->GetColorTableName())
                 needsUpdate = true;
            break;
        case SpreadsheetAttributes::ID_showTracerPlane:
            tracerCheckBox->blockSignals(true);
            tracerCheckBox->setChecked(plotAtts->GetShowTracerPlane());
            tracerCheckBox->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_tracerColor:
            zTabs->setHighlightColor(QColor(plotAtts->GetTracerColor().Red(),
                                            plotAtts->GetTracerColor().Green(),
                                            plotAtts->GetTracerColor().Blue()));
            break;
        case SpreadsheetAttributes::ID_normal:
            normalButtonGroup->blockSignals(true);
            normalButtonGroup->button(plotAtts->GetNormal())->setChecked(true);
            normalButtonGroup->blockSignals(false);

            // If we've changed normals then we need to update the spreadsheet.
            if(cachedAtts.GetNormal() != plotAtts->GetNormal())
                needsRebuild = true;
            break;
        case SpreadsheetAttributes::ID_sliceIndex:
            sliceIndexSet = true;
#ifdef SINGLE_TAB_WINDOW
            needsRebuild = true;
#endif
            break;
        case SpreadsheetAttributes::ID_currentPick:
        case SpreadsheetAttributes::ID_currentPickValid:
        case SpreadsheetAttributes::ID_pastPicks:
            // Check to see if the pick points changed.
            pickPt.clear();
            cellId.clear();
            needsPickUpdate |= PickPointsChanged();
            break;
        case SpreadsheetAttributes::ID_spreadsheetFont:
            { // Start a new block to avoid complaints about skipping QFont initialization
                QFont spreadsheetFont;
                if (spreadsheetFont.fromString(plotAtts->GetSpreadsheetFont().c_str()))
                {
                    for (int i=0; i<nTables; ++i)
                    {
                        tables[i]->setFont(spreadsheetFont);
                    }
                }
                break;
            }
        case SpreadsheetAttributes::ID_showPatchOutline:
            patchOutlineCheckBox->blockSignals(true);
            patchOutlineCheckBox->setChecked(plotAtts->GetShowPatchOutline());
            patchOutlineCheckBox->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_showCurrentCellOutline:
            currentCellOutlineCheckBox->blockSignals(true);
            currentCellOutlineCheckBox->setChecked(plotAtts->GetShowCurrentCellOutline());
            currentCellOutlineCheckBox->blockSignals(false);
            break;
        }
    }

    // Move slice if there is a pick update. The moveSliceToCurrentPick method
    // returns true if it needed to set the slice index in the plot attributes.
    if(needsPickUpdate)
        sliceIndexSet |= moveSliceToCurrentPick();

    // Update the spreadsheet if we've changed attributes that have caused it
    // to need to be redrawn.
    if(needsRebuild)
    {
        updateSpreadsheet();
    }
    else if(needsUpdate)
    {
        // Make the visible table update itself.
        for(int i = 0; i < nTables; ++i)
        {
            tables[i]->setRenderInColor(plotAtts->GetUseColorTable());
            tables[i]->setFormatString(plotAtts->GetFormatString().c_str());

            // Send a paint event to the currently visible page so its cells update
            if(tables[i] == zTabs->currentWidget())
                 tables[i]->update();
        }

        // Update the min/max buttons.
        updateMinMaxButtons();

    }

    // Cache attributes
    cachedAtts = *plotAtts;

    // Update pick point selections.
#ifndef SINGLE_TAB_WINDOW
    if(needsPickUpdate)
#else
    // In single slice mode, we need to update selections if the slice changed
    if(needsPickUpdate || sliceIndexSet)
#endif
    {
        selectPickPoints();
    }

    // Now that we've updated, change the slice index if that was changed too.
    if(sliceIndexSet)
    {
        if(plotAtts->GetSliceIndex() < nTables)
        {
            zTabs->blockSignals(true);
            zTabs->setCurrentIndex(plotAtts->GetSliceIndex());
            zTabs->blockSignals(false);
        }
   
        kSlider->blockSignals(true);
        kSlider->setValue(plotAtts->GetSliceIndex());
        kSlider->blockSignals(false);

        updateSliderLabel();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::updateSpreadsheet
//
// Purpose: 
//   This method takes the input VTK dataset and makes sure that the spreadsheet
//   properly displays the VTK dataset.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:06:26 PST 2007
//
// Modifications:
//   Brad Whitlock, Fri Jan 30 15:43:07 PST 2009
//   Treat polydata as unstructured.
//
// ****************************************************************************

void
SpreadsheetViewer::updateSpreadsheet()
{
    const char *mName = "SpreadsheetViewer::updateSpreadsheet(): ";
    if(input == 0)
    {
        debug1 << mName << "input is NULL" << endl;
        return;
    }

    // Translate VTK object into Qt display.
    if(input->IsA("vtkRectilinearGrid"))
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)input;
        int dims[3];
        rgrid->GetDimensions(dims);
        displayStructuredGrid(dims);
        calculateMinMaxCells(dims, true);
    }
    else if(input->IsA("vtkStructuredGrid"))
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)input;
        int dims[3];
        sgrid->GetDimensions(dims);
        displayStructuredGrid(dims);
        calculateMinMaxCells(dims, true);
    }
    else if(input->IsA("vtkUnstructuredGrid") ||
            input->IsA("vtkPolyData"))
    {
        displayUnstructuredGrid();
        int dims[3] = {1,1,1};
        dims[0] = input->GetNumberOfCells();
        calculateMinMaxCells(dims, false);
    }
    else
        debug1 << mName << "Unsupported mesh type!" << endl;

    // Make sure that the LUT is built.
    colorLUT->GetLookupTable()->SetTableRange(minValue, maxValue);
    colorLUT->GetLookupTable()->Build();

    // Send a paint event to the currently visible page so its cells update
    if(zTabs->currentIndex() != -1)
        zTabs->currentWidget()->update();
}

// ****************************************************************************
// Method: SpreadsheetViewer::GetBaseIndexFromMetaData
//
// Purpose: 
//   Set the base_index from the plot's metadata.
//
// Arguments:
//   base_index : The base_index array to set.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 09:13:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::GetBaseIndexFromMetaData(int *base_index) const
{
    int base = 0;
    const char *mName = "SpreadsheetViewer::GetBaseIndexFromMetaData: ";

    const avtDatabaseMetaData *md = plot->GetMetaData();
    if(md != 0)
    {
        const avtMeshMetaData *mmd = md->GetMesh(plot->GetMeshName());
        if(mmd != 0)
        {
            if(plot->GetVariableCentering() == AVT_NODECENT)
                base = mmd->nodeOrigin;
            else
                base = mmd->cellOrigin;
        }
    }
    base_index[0] = base;
    base_index[1] = base;
    base_index[2] = base;
    debug5 << mName << "From metadata, base_index = {"
           << base_index[0] << ", "
           << base_index[1] << ", "
           << base_index[2] << "}\n";
}

// ****************************************************************************
// Method: SpreadsheetViewer::displayStructuredGrid
//
// Purpose: 
//   This method updates the spreadsheet so it displays the data as a
//   structured grid.
//
// Arguments:
//   meshDims : The dimensions of the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:08:30 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//
//   Gunther H. Weber, Wed Oct 17 16:28:11 PDT 2007
//   Adapt base index if data set has real dims field data
//
//   Brad Whitlock, Fri Jan 30 16:02:12 PST 2009
//   I fixed a logic error that produced all zeros in the table.
//
//   Brad Whitlock, Fri May  8 09:09:29 PDT 2009
//   Set the mesh's base index differently if there are no base_index or 
//   realDims field data arrays. We use the mesh's cell and node origins.
//
// ****************************************************************************

void
SpreadsheetViewer::displayStructuredGrid(int meshDims[3])
{
    const char *mName = "SpreadsheetViewer::displayStructuredGrid: ";
    int base_index[] = {0,0,0};
    int dims[] = {meshDims[0], meshDims[1], meshDims[2]};

    // Get the variable and the variable dims from the dataset.
    vtkDataArray *arr = input->GetPointData()->GetScalars();
    vtkDataArray *ghostArray = 0;
    unsigned char *ghostZones = 0;
    if(arr != 0)
    {
        debug5 << mName << "node centered scalars" << endl;
    }
    else if((arr = input->GetCellData()->GetScalars()) != 0)
    {
        debug5 << mName << "cell centered scalars" << endl;
        dims[0]--;
        dims[1]--;

        if(dims[2] > 1)
            dims[2]--;

        // Try to get a pointer to the ghost zones array.
        ghostArray = input->GetCellData()->GetArray("avtGhostZones");
        if(ghostArray != 0 && ghostArray->GetDataType() == VTK_UNSIGNED_CHAR)
            ghostZones = (unsigned char *)ghostArray->GetVoidPointer(0);
    }

    // Try and populate base_index
    vtkDataArray *baseIndex = input->GetFieldData()->GetArray("base_index");
    if(baseIndex != 0 && baseIndex->GetNumberOfTuples() == 3)
    {
        base_index[0] = (int)baseIndex->GetTuple1(0);
        base_index[1] = (int)baseIndex->GetTuple1(1);
        base_index[2] = (int)baseIndex->GetTuple1(2);
        debug5 << mName << "base_index = {"
             << base_index[0] << ", "
             << base_index[1] << ", "
             << base_index[2] << "}\n";
    }
    else
        debug5 << mName << "No base index" << endl;

    // Try to adjust for real dims
    vtkDataArray *realDims = input->GetFieldData()->GetArray("avtRealDims");
    if(realDims != 0 && realDims->GetNumberOfTuples() == 6)
    {
        base_index[0] -= (int)realDims->GetTuple1(0);
        base_index[1] -= (int)realDims->GetTuple1(2);
        base_index[2] -= (int)realDims->GetTuple1(4);
        debug5 << mName << "Ghost zones change base_index to  base_index = {"
             << base_index[0] << ", "
             << base_index[1] << ", "
             << base_index[2] << "}\n";
    }
    else
        debug5 << mName << "No real dims" << endl;

    // Use the mesh's cellOrigin and nodeOrigin.
    if(baseIndex == 0 && realDims == 0)
        GetBaseIndexFromMetaData(base_index);

    if(arr != 0)
    {
        // If the data are 2D then make sure that the normal is set to Z.
        if(dims[2] <= 1 && plotAtts->GetNormal() != SpreadsheetAttributes::Z)
        {
            plotAtts->SetNormal(SpreadsheetAttributes::Z);
            QTimer::singleShot(100, this, SLOT(postNotify()));
        }

        // Turn off updates for the tab widget and the tables in it.
        zTabs->setUpdatesEnabled(false);
        for(int t = 0; t < nTables; ++t)
            tables[t]->setUpdatesEnabled(false);

        SpreadsheetTable::DisplayMode dMode;
        if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
        {
            // Make sure that we have the right number of tabs.
            setNumberOfTabs(dims[0], base_index[0], true);
            dMode = SpreadsheetTable::SliceX;
        }
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
        {
            // Make sure that we have the right number of tabs.
            setNumberOfTabs(dims[1], base_index[1], true);
            dMode = SpreadsheetTable::SliceY;
        }
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Z)
        {
            // Make sure that we have the right number of tabs.
            setNumberOfTabs(dims[2], base_index[2], true);
            dMode = SpreadsheetTable::SliceZ;
        }

        // Make sure that each table can access the VTK data.
#ifndef SINGLE_TAB_WINDOW
        int offset = 0;
#else
        int offset = plotAtts->GetSliceIndex();
#endif
        for(int t = 0; t < nTables; ++t)
        {
            tables[t]->blockSignals(true);

            // Tell the table about our data so it can display it 
            // appropriately.
            tables[t]->setDataArray(arr, ghostArray, dims, 
                dMode, offset + t, base_index);
            tables[t]->setFormatString(plotAtts->GetFormatString().c_str());
            tables[t]->setRenderInColor(plotAtts->GetUseColorTable());

            tables[t]->blockSignals(false);
            tables[t]->setUpdatesEnabled(true);
        }

        // Turn on updates for the tab widget
        zTabs->setUpdatesEnabled(true);
    }
    else
        debug5 << mName << "Scalars not found !" << endl;
}

// ****************************************************************************
// Method: SpreadsheetViewer::displayUnstructuredGrid
//
// Purpose: 
//   Displays the input data as an unstructured grid.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:12:11 PST 2007
//
// Modifications:
//   Brad Whitlock, Fri May  8 09:09:29 PDT 2009
//   Set the mesh's base index differently if there is no base_index  
//   field data array. We use the mesh's cell and node origins.
//   
// ****************************************************************************

void
SpreadsheetViewer::displayUnstructuredGrid()
{
    const char *mName = "SpreadsheetViewer::displayUnstructuredGrid: ";

    debug5 << mName << "Display as unstructured grid." << endl;
    QString tmp, s;
    QString formatString(plotAtts->GetFormatString().c_str());

    // Try and populate base_index
    int base_index[] = {0,0,0};
    vtkDataArray *baseIndex = input->GetFieldData()->GetArray("base_index");
    if(baseIndex != 0 && baseIndex->GetNumberOfTuples() == 3)
    {
        base_index[0] = (int)baseIndex->GetTuple1(0);
        base_index[1] = (int)baseIndex->GetTuple1(1);
        base_index[2] = (int)baseIndex->GetTuple1(2);
    }
    else
        GetBaseIndexFromMetaData(base_index);

    vtkDataArray *arr = input->GetPointData()->GetScalars();
    if(arr != 0)
    {
        debug5 << mName << "node centered scalars" << endl;

        // We only need one tab for unstructured data.
        setNumberOfTabs(1, base_index[0], false);

        // Tell the table about our data so it can display it 
        // appropriately.
        int dims[] = {1,1,1};
        dims[1] = arr->GetNumberOfTuples();
        tables[0]->setDataArray(arr, 0, dims, 
                SpreadsheetTable::UCDNode, 0, base_index);
        tables[0]->setFormatString(plotAtts->GetFormatString().c_str());
        tables[0]->setRenderInColor(plotAtts->GetUseColorTable());
    }
    else if((arr = input->GetCellData()->GetScalars()) != 0)
    {
        debug5 << mName << "cell centered scalars" << endl;

        vtkDataArray *ghostArray = input->GetCellData()->GetArray("avtGhostZones");

        // Make sure that we have the right number of tabs.
        setNumberOfTabs(1, base_index[0], false);

        // Tell the table about our data so it can display it 
        // appropriately.
        int dims[] = {1,1,1};
        dims[1] = arr->GetNumberOfTuples();
        tables[0]->setDataArray(arr, ghostArray, dims, 
                SpreadsheetTable::UCDCell, 0, base_index);
        tables[0]->setFormatString(plotAtts->GetFormatString().c_str());
        tables[0]->setRenderInColor(plotAtts->GetUseColorTable());
    }
    else
        debug5 << mName << "Scalars not found !" << endl;
}

//
// Macros to help min,max calculation easier.
//

#define BEGIN_MINMAX \
        minCell[0] = minCell[1] = minCell[2] = -1;\
        maxCell[0] = maxCell[1] = maxCell[2] = -1;\
        minValue = maxValue = 0.;\

#define EVAL_MINMAX(T, R, C) \
        if(minCell[0] == -1 || val[0] < minValue)\
        {\
            minCell[0] = T;\
            minCell[1] = R;\
            minCell[2] = C;\
            minValue = val[0];\
        }\
        if(minCell[0] == -1 || val[0] > maxValue)\
        {\
            maxCell[0] = T;\
            maxCell[1] = R;\
            maxCell[2] = C;\
            maxValue = val[0];\
        }

#define END_MINMAX \
        QString fmt, tmp;\
        fmt = tr("Min = ") + QString(plotAtts->GetFormatString().c_str());\
        tmp.sprintf(fmt.toStdString().c_str(), minValue);\
        minButton->setText(tmp);\
        minButton->setEnabled(true);\
        debug5 << mName << "min=" << minValue << ", minCell=[" << minCell[0] << "," << minCell[1] << "," << minCell[2] << "]" << endl;\
        fmt = tr("Max = ") + QString(plotAtts->GetFormatString().c_str());\
        tmp.sprintf(fmt.toStdString().c_str(), maxValue);\
        maxButton->setText(tmp);\
        maxButton->setEnabled(true);\
        debug5 << mName << "max=" << maxValue << ", maxCell=[" << maxCell[0] << "," << maxCell[1] << "," << maxCell[2] << "]" << endl;


// ****************************************************************************
// Method: SpreadsheetViewer::calculateMinMaxCells
//
// Purpose: 
//   Calculates the min,max of the dataset and where they are located within
//   the spreadsheet.
//
// Arguments:
//   meshDims   : The dimensions of the mesh.
//   structured : Whether the input is on a structured mesh.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:12:46 PST 2007
//
// Modifications:
//   Brad Whitlock, Thu Aug 28 14:09:58 PDT 2008
//   Fixed a bug with Y slicing.
//
// ****************************************************************************

void
SpreadsheetViewer::calculateMinMaxCells(int meshDims[3], 
    bool structured)
{
    const char *mName = "SpreadsheetViewer::calculateMinMaxCells: ";
    debug5 << mName << "Calculating min, max." << endl;
    int dims[] = {meshDims[0], meshDims[1], meshDims[2]};

    // Get the variable and the variable dims from the dataset.
    vtkDataArray *arr = input->GetPointData()->GetScalars();
    vtkDataArray *ghostArray = 0;
    unsigned char *ghostZones = 0;
    if(arr != 0)
    {
        debug5 << mName << "node centered scalars" << endl;
    }
    else if((arr = input->GetCellData()->GetScalars()) != 0)
    {
        debug5 << mName << "cell centered scalars" << endl;
        dims[0]--;
        dims[1]--;

        if(dims[2] > 1)
            dims[2]--;

        // Try to get a pointer to the ghost zones array.
        ghostArray = input->GetCellData()->GetArray("avtGhostZones");
        if(ghostArray != 0 && ghostArray->GetDataType() == VTK_UNSIGNED_CHAR)
            ghostZones = (unsigned char *)ghostArray->GetVoidPointer(0);
    }

    if(arr != 0 && structured)
    {
        if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
        {
            vtkIdType index = 0;
            BEGIN_MINMAX
            for(int k = 0; k < dims[2]; ++k)
            {
                for(int j = 0; j < dims[1]; ++j)
                {
                    int row = dims[1]-1-j;
                    for(int i = 0; i < dims[0]; ++i, ++index)
                    {
                        // If the data has ghost zones then skip ghosts so they
                        // don't mess up min/max calculations.
                        if(ghostZones != 0 && ghostZones[index] != 0)
                            continue;

                        double *val = arr->GetTuple(index);
                        EVAL_MINMAX(i, row /*j*/, k)
                    }
                }
            }
            END_MINMAX
        }
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
        {
            vtkIdType index = 0;
            BEGIN_MINMAX
            for(int k = 0; k < dims[2]; ++k)
            {
                int col = k;//dims[2]-1-k;
                for(int j = 0; j < dims[1]; ++j)
                {
                    for(int i = 0; i < dims[0]; ++i, ++index)
                    {
                        int row = dims[0]-1-i;
                        // If the data has ghost zones then skip ghosts so they
                        // don't mess up min/max calculations.
                        if(ghostZones != 0 && ghostZones[index] != 0)
                            continue;

                        double *val = arr->GetTuple(index);
                        EVAL_MINMAX(j, row, col /*k*/)
                    }
                }
            }
            END_MINMAX
        }
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Z)
        {
            vtkIdType index = 0;
            BEGIN_MINMAX
            for(int k = 0; k < dims[2]; ++k)
            {
                for(int j = 0; j < dims[1]; ++j)
                {
                    int row = dims[1]-1-j;
                    for(int i = 0; i < dims[0]; ++i, ++index)
                    {
                        // If the data has ghost zones then skip ghosts so they
                        // don't mess up min/max calculations.
                        if(ghostZones != 0 && ghostZones[index] != 0)
                            continue;

                        double *val = arr->GetTuple(index);
                        EVAL_MINMAX(k, row/*j*/, i)
                    }
                }
            }
            END_MINMAX
        }
    }
    else if(arr != 0)
    { 
        // Unstructured
        vtkIdType index = 0;
        BEGIN_MINMAX
        for(index = 0; index < arr->GetNumberOfTuples(); ++index)
        {
            // If the data has ghost zones then skip ghosts so they
            // don't mess up min/max calculations.
            if(ghostZones != 0 && ghostZones[index] != 0)
                continue;

            double *val = arr->GetTuple(index);
            EVAL_MINMAX(0, index, 0)
        }
        END_MINMAX
    }
    else
        debug5 << mName << "Scalars not found !" << endl;
}

// ****************************************************************************
// Method: SpreadsheetViewer::updateMinMaxButtons
//
// Purpose: 
//   Updates the min,max buttons with the min,max values.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:13:43 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::updateMinMaxButtons()
{
    const char *mName = "SpreadsheetViewer::updateMinMaxButtons";
    END_MINMAX
}

// ****************************************************************************
// Method: SpreadsheetViewer::setNumberOfTabs
//
// Purpose: 
//   Makes sure that the window has the correct number of tabs for the
//   dataset that will be displayed.
//
// Arguments:
//   nt   : The new number of tabs.
//   base : The base index for the number of tabs so they are labelled from
//          this number instead of 0.
//   structured : Whether the input data is structured.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:14:01 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//   
//   Gunther H. Weber, Thu Sep 27 13:33:36 PDT 2007
//   Add support for setting spreadsheet font
//
//   Brad Whitlock, Wed Apr 23 11:26:17 PDT 2008
//   Set nTablesForSlider.
//
//   Brad Whitlock, Tue Aug 26 15:31:41 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Fri Jan 30 15:37:10 PST 2009
//   I fixed a bug with removing tabs from the tab widget.
//
// ****************************************************************************

void
SpreadsheetViewer::setNumberOfTabs(int nt, int base, bool structured)
{
    int ntabs = (nt < 1) ? 1 : nt;

    controls3D->setEnabled(ntabs > 1);
    bool updateSlider = true;

    zTabs->blockSignals(true);

#ifndef SINGLE_TAB_WINDOW
    if(ntabs == nTables)
    {
        updateSlider = false;
    }
    else if(ntabs > nTables)
    {
        // Add tabs
        SpreadsheetTable **t = new SpreadsheetTable*[ntabs];
        for(int i = 0; i < ntabs; ++i)
        {
            if(i < nTables)
                t[i] = tables[i];
            else
            {
                t[i] = new SpreadsheetTable(0);
                t[i]->setUpdatesEnabled(false);
                t[i]->setLUT(colorLUT);
                QFont spreadsheetFont;
                if (spreadsheetFont.fromString(plotAtts->GetSpreadsheetFont().c_str()))
                    t[i]->setFont(spreadsheetFont);
                connect(t[i], SIGNAL(selectionChanged()),
                        this, SLOT(tableSelectionChanged()));
                zTabs->addTab(t[i],"");
            }
        }
        nTables = ntabs;
        nTablesForSlider = ntabs;
        delete [] tables;
        tables = t;
    }
    else
    {
        // Remove tabs
        SpreadsheetTable **t = new SpreadsheetTable*[ntabs];
        for(int i = 0; i < nTables; ++i)
        {
            if(i < ntabs)
                t[i] = tables[i];
            else
            {
                zTabs->removeTab(zTabs->indexOf(tables[i]));
                disconnect(tables[i], SIGNAL(selectionChanged()),
                           this, SLOT(tableSelectionChanged()));
                delete tables[i];
            }
        }
        nTables = ntabs;
        nTablesForSlider = ntabs;
        delete [] tables;
        tables = t;
    }
#else
    nTablesForSlider = ntabs;
#endif

#ifndef SINGLE_TAB_WINDOW
    int offset = 0;
#else
    int offset = plotAtts->GetSliceIndex();
#endif
    // Set the names of the tabs
    for(int i = 0; i < nTables; ++i)
    {
        QString name;
        if(!structured)
            name = tr("Unstructured");
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
            name.sprintf("i=%d", i+base+offset);
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
            name.sprintf("j=%d", i+base+offset);
        else
            name.sprintf("k=%d", i+base+offset);
        zTabs->setTabText(i, name);
    }
    zTabs->blockSignals(false);

    // Always update the label in case we're using a different DB.
    updateSliderLabel();

    // Only update the slider value if the number of tabs changed.
    if(updateSlider)
    {
        kSlider->blockSignals(true);
        kSlider->setMinimum(0);
        kSlider->setMaximum(ntabs - 1);
        kSlider->blockSignals(false);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::updateSliderLabel
//
// Purpose: 
//   Updates the slider label based on the slice index.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:55:56 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:17:21 PDT 2008
//   Made the labels work better on the Mac where we have only a single
//   tab in the window.
//
//   Brad Whitlock, Fri May  8 09:16:45 PDT 2009
//   Get a base index from the metadata.
//
// ****************************************************************************

void
SpreadsheetViewer::updateSliderLabel()
{
    const char *mName = "SpreadsheetViewer::updateSliderLabel: ";
    int base_index[] = {0,0,0};

    // Try and populate base_index
    if(input == 0)
    {
        debug5 << mName << "input is NULL. Not checking for base_index" << endl;
    }
    else
    {
        vtkDataArray *baseIndex = input->GetFieldData()->GetArray("base_index");
        if(baseIndex != 0 && baseIndex->GetNumberOfTuples() == 3)
        {
            base_index[0] = (int)baseIndex->GetTuple1(0);
            base_index[1] = (int)baseIndex->GetTuple1(1);
            base_index[2] = (int)baseIndex->GetTuple1(2);
        }
        else
            GetBaseIndexFromMetaData(base_index);

        debug5 << mName << "base_index = {"
               << base_index[0] << ", "
               << base_index[1] << ", "
               << base_index[2] << "}\n";
    }

    QString kl;
    if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
    {
        kl.sprintf("i=%d [%d,%d]", base_index[0] + plotAtts->GetSliceIndex(), 
            base_index[0], base_index[0] + nTablesForSlider - 1);
    }
    else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
    {
        kl.sprintf("j=%d [%d,%d]", base_index[1] + plotAtts->GetSliceIndex(), 
            base_index[1], base_index[1] + nTablesForSlider - 1);
    }
    else
    {
        kl.sprintf("k=%d [%d,%d]", base_index[2] + plotAtts->GetSliceIndex(), 
            base_index[2], base_index[2] + nTablesForSlider - 1);
    } 
    kLabel->setText(kl);
}

// ****************************************************************************
// Method: SpreadsheetViewer::clear
//
// Purpose: 
//   Disassociates the VTK data from the spreadsheets and redraws the active
//   spreadsheet so it is cleared.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:15:26 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 15:54:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::clear()
{
    input = 0;

    // Clear the data arrays from the tables.
    for(int k = 0; k < nTables; ++k)
        tables[k]->clearDataArray();

    // Update the visible table.
    if(zTabs->currentIndex() != -1)
        zTabs->currentWidget()->update();

    minButton->setEnabled(false);
    maxButton->setEnabled(false);
}

// ****************************************************************************
// Method: SpreadsheetViewer::updateMenuEnabledState
//
// Purpose: 
//   Sets the enabled state of the window's menus based on the table's
//   number of selections.
//
// Arguments:
//   table : The table that we're using to set the menu enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 12:24:56 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 10:26:54 PDT 2008
//   Don't set the enabled state for the operation menu on the Mac.
//
//   Brad Whitlock, Tue Aug 26 15:28:00 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Fri Jan 30 15:17:13 PST 2009
//   Check for non-NULL table.
//
// ****************************************************************************

void
SpreadsheetViewer::updateMenuEnabledState(int tableIndex)
{
    //
    // If the sender is the current page then update the menus based on
    // whether it has any selections.
    //
    if(zTabs->currentIndex() == tableIndex)
    {
        QTableView *table = (QTableView *)zTabs->currentWidget();
        if(table != 0)
        {
            bool enabled = table->selectionModel()->hasSelection();

            fileMenu_SaveText->setEnabled(enabled);
            editMenu_Copy->setEnabled(enabled);
#ifndef Q_WS_MAC
            operationsMenu->setEnabled(enabled);
#else
            opButton->setEnabled(enabled);
#endif
        }
    }
}

// ****************************************************************************
// Method: SpreadSheetViewer::moveSliceToCurrentPick()
//
// Purpose: 
//   Moves the displayed slice to the current pick point
//
// Arguments:
//
// Returns:    
//;   Whether the slice was moved
//
// Note:       
//
// Programmer: Gunther H. Weber
// Creation:   Mon Sep 10 15:05:01 PDT 2007
//
// Modifications:
//   
//   Hank Childs, Sun Oct 28 21:48:23 PST 2007
//   Account for layers of ghost zones when calculating indices.
//   
// ****************************************************************************

bool
SpreadsheetViewer::moveSliceToCurrentPick()
{
    const char *mName = "SpreadSheetViewer::moveSliceToCurrentPick: ";
    bool retval = false;

    if(input == NULL)
        return false;

    debug1 << "In " << mName << std::endl;

    // ... Calculate position (slice, row, column) of current pick
    if (plotAtts->GetCurrentPickValid())
    {
        int sliceAxis = -1;
        int rowAxis, columnAxis;
        switch (plotAtts->GetNormal())
        {
            case SpreadsheetAttributes::X:
                sliceAxis = 0;
                rowAxis = 1;
                columnAxis = 2;
                break;
            case SpreadsheetAttributes::Y:
                sliceAxis = 1;
                rowAxis = 0;
                columnAxis = 2;
                break;
            case SpreadsheetAttributes::Z:
                sliceAxis = 2;
                rowAxis = 1;
                columnAxis = 0;
                break;
            default:
                debug1 << mName << "Invalid normal specified in plot attributes.";
                debug1<< std::endl;
                break;
        }

        if (sliceAxis != -1)
        {
            int ijk[3];

            double *currentPick = plotAtts->GetCurrentPick();
            int cellId = GetCell(currentPick[0], currentPick[1], currentPick[2]);

            vtkVisItUtility::GetLogicalIndices(input, true, cellId, ijk, false, false);
            debug5 << mName << "ijk=" << ijk[0] << " " << ijk[1] << " " << ijk[2] << std::endl;

            if (ijk[0] == -1)
            {
                debug1 << mName << "Cannot compute logical index for cell ";
                debug1 << cellId << std::endl;
            }
            // ... Select appropriate slice
            else if (ijk[0] != -1)
            {
                // If the slice index is not the current slice index then
                // change the current slice index to match that of the pick.
                if(ijk[sliceAxis] != plotAtts->GetSliceIndex())
                {
                    debug1 << mName << "Setting slice index to: " << ijk[sliceAxis] << endl;
                    // Set the slice index that we calculated into the plotAtts.
                    plotAtts->SetSliceIndex(ijk[sliceAxis]);
 
                    // Issue a Notify from the main event loop.
                    QTimer::singleShot(0, this, SLOT(postNotify()));
                    retval = true;
                }
            }
        }
    }
    return retval;
}

// ****************************************************************************
// Method: SpreadSheetViewer::selectPickPoints
//
// Purpose: 
//   Updates pick points in spreadsheet.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Gunther H. Weber
// Creation:   Mon Sep 10 15:05:01 PDT 2007
//
// Modifications:
//   Hank Childs, Sun Oct 28 21:48:23 PST 2007
//   Account for layers of ghost zones when calculating indices.
//
//   Brad Whitlock, Thu Aug 28 13:53:57 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::selectPickPoints()
{
    const char *mName = "SpreadSheetViewer::selectPickPoints: ";

    if(input == NULL)
        return;

    debug5 << mName << "Clearing old pick selections." << std::endl;
    for(int t = 0; t < nTables; ++t)
    {
        tables[t]->selectionModel()->clear();
        tables[t]->clearSelectedCellLabels();
    }

    // ... Calculate position (slice, row, column) of current pick
    if (plotAtts->GetCurrentPickValid())
    {
        int sliceAxis = -1;
        int rowAxis, columnAxis;
        switch (plotAtts->GetNormal())
        {
            case SpreadsheetAttributes::X:
                sliceAxis = 0;
                rowAxis = 1;
                columnAxis = 2;
                break;
            case SpreadsheetAttributes::Y:
                sliceAxis = 1;
                rowAxis = 0;
                columnAxis = 2;
                break;
            case SpreadsheetAttributes::Z:
                sliceAxis = 2;
                rowAxis = 1;
                columnAxis = 0;
                break;
            default:
                debug1 << mName << "Invalid normal specified in plot attributes.";
                debug1<< std::endl;
                break;
        }

        if (sliceAxis != -1)
        {
            int ijk[3];

            double *currentPick = plotAtts->GetCurrentPick();
            int cellId = GetCell(currentPick[0], currentPick[1], currentPick[2]);

            vtkVisItUtility::GetLogicalIndices(input, true, cellId, ijk, false, false);

            debug5 << mName << "CP: ijk=" << ijk[0] << " " << ijk[1] << " " << ijk[2] << std::endl;

            if (ijk[0] == -1)
            {
                debug1 << mName << "Cannot compute logical index for cell ";
                debug1 << cellId << std::endl;
            }
            // ... Select current cell in table
#ifndef SINGLE_TAB_WINDOW
            else if (ijk[0] != -1)
#else
            // In single slice mode we only need to handle the current 
            // pick if the spreadsheet is visible
            else if (ijk[0] != -1 && ijk[sliceAxis] == plotAtts->GetSliceIndex())
#endif
            {
#ifdef SINGLE_TAB_WINDOW
                int activeTable = 0;
#else
                int activeTable = ijk[sliceAxis];
#endif
                int col = ijk[columnAxis];
                // Convert logical index row to spreadsheet row
                int row = tables[activeTable]->model()->rowCount() - ijk[rowAxis] - 1;

                // Select the new cell in the active table.
                debug1 << mName << "Selecting current cell (" << row << ", "
                       << col << ")" << std::endl;

                // Select the new cell.
                QModelIndex index(tables[activeTable]->model()->index(row, col));
                tables[activeTable]->selectionModel()->clear();
                tables[activeTable]->selectionModel()->select(index, QItemSelectionModel::Select);
                tables[activeTable]->scrollTo(index);
                tables[activeTable]->addSelectedCellLabel(row, col, plotAtts->GetCurrentPickLetter().c_str());

#ifndef SINGLE_TAB_WINDOW
                debug1 << mName << "Setting current cell (" << row << ", " << col << ")"
                       << std::endl; 
                QModelIndex id = tables[activeTable]->model()->index(row, col);
                tables[activeTable]->selectionModel()->setCurrentIndex(id, QItemSelectionModel::ClearAndSelect);
#else
                if (ijk[sliceAxis] == plotAtts->GetSliceIndex())
                {
                    debug1 << mName << "Setting current cell (" << row << ", " << col << ")"
                           << std::endl;
                    QModelIndex id = tables[activeTable]->model()->index(row, col);
                    tables[activeTable]->selectionModel()->setCurrentIndex(id, QItemSelectionModel::ClearAndSelect);
                }
                else
                {
                    debug1 << mName << "Current pick is not visible." << std::endl;
                }
#endif
            }

            // Now, go through the old picks 
            const vector<double>& pastPicks = plotAtts->GetPastPicks();
            const vector<string>& pastPickLetters = plotAtts->GetPastPickLetters();
            int numOldPicks = pastPicks.size() / 3;
            int old_ijk[3];
            for (int i = 0 ; i < numOldPicks ; i++)
            {
                int cellId = GetCell(pastPicks[3*i], pastPicks[3*i+1], pastPicks[3*i+2]);
                vtkVisItUtility::GetLogicalIndices(input, true, cellId, old_ijk, false, false);

                // If old pick is same cell as current pick then skip it
                if (old_ijk[0] == ijk[0] && old_ijk[1] == ijk[1] && old_ijk[2] == ijk[2])
                    continue;

                debug5 << mName << "OP: ijk=" << old_ijk[0] << " " << old_ijk[1] << " " << old_ijk[2] << std::endl;

#ifdef SINGLE_TAB_WINDOW
                // Get row and column of old pick
                int oldRow = tables[0]->model()->rowCount() - old_ijk[rowAxis] - 1;
                int oldCol = old_ijk[columnAxis];

                // If old pick is in same slice as current pick -> highlight it
                if (old_ijk[sliceAxis] == plotAtts->GetSliceIndex())
                {
                    debug1 << mName << "Highlight cell (" << oldRow << ", "
                        << oldCol << ") in single slice" << endl;
                    QModelIndex index(tables[0]->model()->index(oldRow, oldCol));
                    tables[0]->selectionModel()->select(index, QItemSelectionModel::Select);
                    tables[0]->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
                    tables[0]->scrollTo(index);
                    tables[0]->addSelectedCellLabel(oldRow, oldCol, pastPickLetters[i].c_str());
                }
#else
                // Get row and column of old pick
                int oldRow = tables[old_ijk[sliceAxis]]->model()->rowCount() - old_ijk[rowAxis] - 1;
                int oldCol = old_ijk[columnAxis];

                // In multi-tab mode highlight selections in all tables
                if (old_ijk[sliceAxis] <nTables)
                {
                    debug1 << mName << "Highlight cell (" << oldRow << ", "
                           << oldCol << ") in table " << old_ijk[sliceAxis] << endl;
                    SpreadsheetTable *table = tables[old_ijk[sliceAxis]];
                    QModelIndex index(table->model()->index(oldRow, oldCol));
                    table->selectionModel()->select(index, QItemSelectionModel::Select);
                    table->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
                    table->scrollTo(index);
                    table->addSelectedCellLabel(oldRow, oldCol, pastPickLetters[i].c_str());
                }
#endif
            }
        }
    }
    else
    {
        debug1 << mName << "Current pick not valid. " << std::endl;
    }
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: SpreadsheetViewer::formatChanged
//
// Purpose: 
//   This slot is called when the format is changed and causes the spreadsheets
//   to update using the new format.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:16:07 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::formatChanged()
{
    // Set the attributes and notify the viewer about the changes
    std::string formatString(formatLineEdit->text().toStdString());
    plotAtts->SetFormatString(formatString);
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::sliderChanged
//
// Purpose: 
//   This slot is called when the slider changes. We use it to set the active
//   slice index in the plot attributes and also make sure that the right
//   spreadshset page is visible.
//
// Arguments:
//   val : The new slice value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:16:44 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//
//   Brad Whitlock, Tue Aug 26 15:52:50 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::sliderChanged(int slice)
{
#ifndef SINGLE_TAB_WINDOW
    int tabIndex = slice;
#else
    int tabIndex = 0;
#endif
    if(tabIndex >= 0 && tabIndex < nTables)
    {
        plotAtts->SetSliceIndex(slice);
        postNotify();

        zTabs->blockSignals(true);
        zTabs->setCurrentIndex(tabIndex);
        zTabs->blockSignals(false);

        updateSliderLabel();
        updateMenuEnabledState(tabIndex);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::sliderPressed
//
// Purpose: 
//   Sets a flag that tells us to ignore updates while the slider is moving.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:43:02 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::sliderPressed()
{
    sliding = true;

    // Turn off automatic client updates so calling Notify() on the attributes
    // here will have an effect in the viewer but not in the clients.
    plot->AlternateDisplaySetAllowClientUpdates(false);
}

// ****************************************************************************
// Method: SpreadsheetViewer::sliderReleased
//
// Purpose: 
//   Sets a flag that tells us not to ignore updates.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:43:02 PST 2007
//
// Modifications:
//
// ****************************************************************************

void
SpreadsheetViewer::sliderReleased()
{
    sliding = false;

    // Turn on automatic client updates so calling Notify() on the attributes
    // here will have an effect in the viewer and in the clients.
    plot->AlternateDisplaySetAllowClientUpdates(true);
    SetUpdate(false);
    plotAtts->Notify();
}


// ****************************************************************************
// Method: SpreadsheetViewer::tabChanged
//
// Purpose: 
//   This slot is called when the active tab changes. We use it to set the 
//   active slice index in the plot attributes and also make sure that the 
//   right spreadshset page is visible.
//
// Arguments:
//   index : The new slice value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:16:44 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 15:53:01 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::tabChanged(int index)
{
    if(nTables > 1 && isVisible() && index >= 0)
    {
        SetUpdate(false);
        plotAtts->SetSliceIndex(index);
        plotAtts->Notify();

        kSlider->blockSignals(true);
        kSlider->setValue(index);
        kSlider->blockSignals(false);

        updateSliderLabel();
        updateMenuEnabledState(index);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::normalChanged
//
// Purpose: 
//   This slot is called when we want to change how the data are sliced.
//
// Arguments:
//   val : The new slice direction.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:18:06 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::normalChanged(int val)
{
    if(val != (int)plotAtts->GetNormal())
    {
        plotAtts->SetNormal((SpreadsheetAttributes::NormalAxis)val);
        plotAtts->SetSliceIndex(0);
        plotAtts->Notify();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::colorTableCheckBoxToggled
//
// Purpose: 
//   This slot is called when we want to change whether colors are used in
//   the spreadsheet.
//
// Arguments:
//   val : The new flag for whether colors are used.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:18:34 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::colorTableCheckBoxToggled(bool val)
{
    plotAtts->SetUseColorTable(val);
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::tracerCheckBoxToggled
//
// Purpose: 
//   This slot turns the highlight plane on/off.
//
// Arguments:
//   val : True to draw the highlight.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:19:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::tracerCheckBoxToggled(bool val)
{
    plotAtts->SetShowTracerPlane(val);
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::outlineCheckBoxToggled
//
// Purpose: 
//   This slot turns the patch outline on/off.
//
// Arguments:
//   val : True to draw the patch outline.
//
// Programmer: Gunther H. Weber
// Creation:   Tue Oct 16 20:40:50 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::outlineCheckBoxToggled(bool val)
{
    plotAtts->SetShowPatchOutline(val);
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::showCurrentCellOutlineCheckBoxToggled
//
// Purpose: 
//   This slot turns the current cell on/off.
//
// Arguments:
//   val : True to draw the current cell outline.
//
// Programmer: Gunther H. Weber
// Creation:   Wed Nov 28 15:27:10 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::showCurrentCellOutlineCheckBoxToggled(bool val)
{
    plotAtts->SetShowCurrentCellOutline(val);
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::minClicked
//
// Purpose: 
//   This slot is called when the min button is clicked and we make the
//   window highlight the min value in the spreadsheet.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:19:50 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//
//   Brad Whitlock, Wed Aug 27 15:49:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::minClicked()
{
    // minCell[0] = The index of the table that contains min
    // minCell[1] = The row of the table that contains min
    // minCell[2] = The column of the table that contains min
#ifndef SINGLE_TAB_WINDOW
    if(minCell[0] != -1 && minCell[1] != -1 && minCell[2] != -1 &&
       minCell[0] < nTables)
    {
        // Show the page and don't block signals so we are sure to also
        // update the kSlider via the tabShanged slot
        zTabs->setCurrentIndex(minCell[0]);

        // Select the new cell.
        SpreadsheetTable *table = tables[minCell[0]];
        QModelIndex index(table->model()->index(minCell[1], minCell[2]));
        table->selectionModel()->clear();
        table->selectionModel()->select(index, QItemSelectionModel::Select);
        table->scrollTo(index);
    }
#else
    if(minCell[0] != -1 && minCell[1] != -1 && minCell[2] != -1)
    {
        // Show the right slice of data.
        plotAtts->SetSliceIndex(minCell[0]);
        plotAtts->Notify();

        // Select the new cell.
        SpreadsheetTable *table = tables[0];
        QModelIndex index(table->model()->index(minCell[1], minCell[2]));
        table->selectionModel()->clear();
        table->selectionModel()->select(index, QItemSelectionModel::Select);
        table->scrollTo(index);
    }
#endif
}

// ****************************************************************************
// Method: SpreadsheetViewer::maxClicked
//
// Purpose: 
//   This slot is called when the max button is clicked and we make the
//   window highlight the max value in the spreadsheet.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:19:50 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 17:24:26 PST 2007
//   Support using a single tab of values.
//
//   Brad Whitlock, Wed Aug 27 16:15:32 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::maxClicked()
{
    // maxCell[0] = The index of the table that contains min
    // maxCell[1] = The row of the table that contains min
    // maxCell[2] = The column of the table that contains min
#ifndef SINGLE_TAB_WINDOW
    if(maxCell[0] != -1 && maxCell[1] != -1 && maxCell[2] != -1 &&
       maxCell[0] < nTables)
    {
        // Show the page and don't block signals so we are sure to also
        // update the kSlider via the tabShanged slot
        zTabs->setCurrentIndex(maxCell[0]);

        // Select the new cell.
        SpreadsheetTable *table = tables[maxCell[0]];
        QModelIndex index(table->model()->index(maxCell[1], maxCell[2]));
        table->selectionModel()->clear();
        table->selectionModel()->select(index, QItemSelectionModel::Select);
        table->scrollTo(index);
    }
#else
    if(maxCell[0] != -1 && maxCell[1] != -1 && maxCell[2] != -1)
    {
        // Show the right slice of data.
        plotAtts->SetSliceIndex(maxCell[0]);
        plotAtts->Notify();

        // Select the new cell.
        SpreadsheetTable *table = tables[0];
        QModelIndex index(table->model()->index(maxCell[1], maxCell[2]));
        table->selectionModel()->clear();
        table->selectionModel()->select(index, QItemSelectionModel::Select);
        table->scrollTo(index);
    }
#endif
}

// ****************************************************************************
// Method: SpreadsheetViewer::postNotify
//
// Purpose: 
//   This slot is used to force a plot attributes notify from the main event loop.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:20:42 PST 2007
//
// Modifications:
//   
//    Hank Childs, Thu Sep 20 11:18:18 PDT 2007
//    Make sure the active plots stay the same.
//
// ****************************************************************************

void
SpreadsheetViewer::postNotify()
{
    //
    // See what was active before we update the attributes.
    //
    vector<int> activePlots;
    PlotList *plist = plot->GetViewerState()->GetPlotList();
    int nplots = plist->GetNumPlots();
    for (int i = 0 ; i < nplots ; i++)
    {
        const Plot &p = plist->GetPlots(i);
        if (p.GetActiveFlag())
           activePlots.push_back(i);
    }

    //
    // Update the attributes.  This will change the active plots.
    //
    plotAtts->Notify();

    if (activePlots.size() > 0)
    {
        //
        // Set the active plots to be what they originally were.
        //
        plot->GetViewerMethods()->SetActivePlots(activePlots);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::selectedColorTable
//
// Purpose: 
//   This slot is called when we change the color table name.
//
// Arguments:
//   ctName : The name of the new color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:21:14 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::selectedColorTable(bool, const QString &ctName)
{
    // Set the color table in the attributes.
    plotAtts->SetColorTableName(ctName.toStdString());
    plotAtts->Notify();
}

// ****************************************************************************
// Method: SpreadsheetViewer::changedVariable
//
// Purpose: 
//   This slot if called when we select a new variable from the variable button.
//
// Arguments:
//   newVar : The new variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:23:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::changedVariable(const QString &newVar)
{
    // Set the attributes to make sure that plot is selected.
    SetUpdate(false);
    plotAtts->Notify();

    // Change the plot variable. May need to have this encoded into
    // the xfer object to avoid possible reentrancy problems.
    plot->GetViewerMethods()->ChangeActivePlotsVar(newVar.toStdString());
}

// ****************************************************************************
// Method: SpreadsheetViewer::saveAsText
//
// Purpose: 
//   This slot saves the selected cells to a text file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:24:17 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:28:00 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Aug 26 16:03:11 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Oct  9 14:12:35 PDT 2008
//   Added a message about the vertical ordering being different.
//
// ****************************************************************************

void
SpreadsheetViewer::saveAsText()
{
    if(nTables > 0)
    {
        // Get the name of the file that the user wants to save
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), 
            tr("selection.txt"), tr("Text (*.txt)"));

        // If the user chose to save a file, write it out.
        if(!fileName.isNull())
        {
            SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
            QString txt(t->selectedCellsAsText());

            // Save the text to a file.
            QFile file(fileName);
            if(file.open(QIODevice::WriteOnly))
            {
                QTextStream stream( &file );
                stream << tr("* Note that the vertical ordering of this file's "
                             "contents is opposite from what you would see in "
                             "the Spreadsheet window.\n");
                stream << txt;
                file.close();
            }
            else
            {
                QString err(tr("Could not write %1.").arg(fileName));
                plot->Error(err.toStdString().c_str());
            }
        }
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::copySelectionToClipboard
//
// Purpose: 
//   This slot saves the selected cells to the clipboard so they can be
//   pasted into other applications.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:24:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::copySelectionToClipboard()
{
    if(nTables > 0)
    {
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();

        // Copy the text to the clipbard.
        QClipboard *cb = QApplication::clipboard();
        cb->setText(t->selectedCellsAsText());
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::selectAll
//
// Purpose: 
//   This slot selects all cells in the active table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:25:30 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::selectAll()
{
    if(nTables > 0)
    {
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        t->selectAll();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::selectNone
//
// Purpose: 
//   This slot clears the selection from the active table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:25:48 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::selectNone()
{
    if(nTables > 0)
    {
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        t->selectNone();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::operationSum
//
// Purpose: 
//   This slot calculates the sum of the selected cells and displays the sum
//   in a message box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:26:05 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:35:09 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
SpreadsheetViewer::operationSum()
{
    if(nTables > 0)
    {
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        double sum = t->selectedCellsSum();
        QString sumStr;
        sumStr.sprintf(plotAtts->GetFormatString().c_str(), sum);
        QString msg(tr("The sum of the selected cells is: %1.").arg(sumStr));
        QMessageBox::information(this, tr("Sum results"), msg, QMessageBox::Ok);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::operationAverage
//
// Purpose: 
//   This slot calculates the average of the selected cells and displays the 
//   average in a message box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:26:05 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:35:09 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
SpreadsheetViewer::operationAverage()
{
    if(nTables > 0)
    {
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        double avg = t->selectedCellsAverage();
        QString avgStr;
        avgStr.sprintf(plotAtts->GetFormatString().c_str(), avg);
        QString msg(tr("The average value of the selected cells is: %1.").arg(avgStr));
        QMessageBox::information(this, "Average results", msg, QMessageBox::Ok);
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::GetDataVsCoordinate
//
// Purpose: 
//   Extract data and make a curve.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:53:13 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
SpreadsheetViewer::GetDataVsCoordinate(double *curve, const vtkIdType *indices, 
    int nvals, int coord) const
{
    const char *mName = "SpreadsheetViewer::GetDataVsCoordinate: ";

    // Get the variable and the variable dims from the dataset.
    vtkDataArray *arr = input->GetPointData()->GetScalars();
    bool cellCentered = false;
    if(arr != 0)
    {
        debug5 << mName << "node centered scalars" << endl;
    }
    else if((arr = input->GetCellData()->GetScalars()) != 0)
    {
        cellCentered = true;
        debug5 << mName << "cell centered scalars" << endl;
    }
    else
        return false;

    int comp = 0;
    if(coord == 0)
    {
        if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
            comp = 2;
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
            comp = 0;
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Z)
            comp = 0;
    }
    else
    {
        if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
            comp = 1;
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
            comp = 2;
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Z)
            comp = 1;
    }

    bool retval = false;
    if(input->IsA("vtkRectilinearGrid"))
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)input;
        if(comp == 0)
        {
            for(int k = 0; k < nvals; ++k)
            {
                curve[k*2  ] = rgrid->GetXCoordinates()->GetTuple1(k);
                curve[k*2+1] = arr->GetTuple1(indices[k]);
            }
        }
        else if(comp == 1)
        {
            for(int i = 0; i < nvals; ++i)
            {
                curve[i*2  ] = rgrid->GetYCoordinates()->GetTuple1(i);
                curve[i*2+1] = arr->GetTuple1(indices[i]);
            }
        }
        else if(comp == 2)
        {
            for(int i = 0; i < nvals; ++i)
            {
                curve[i*2  ] = rgrid->GetZCoordinates()->GetTuple1(i);
                curve[i*2+1] = arr->GetTuple1(indices[i]);
            }
        }
        retval = true;
    }
    else if(input->IsA("vtkStructuredGrid"))
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)input;
        if(cellCentered)
        {
            int dims[3], cdims[3];
            sgrid->GetDimensions(dims);
            cdims[0] = dims[0]-1;
            cdims[1] = dims[1]-1;
            cdims[2] = (dims[2] > 1) ? (dims[2]-1) : dims[2];

            for(int i = 0; i < nvals; ++i)
            {
                // Turn cell indices[i] into a cell I,J,K
                int K = indices[i] / (cdims[0]*cdims[1]);
                int I2 = indices[i] - (K * (cdims[0]*cdims[1]));
                int J = I2 / cdims[0];
                int I = I2 % cdims[0];

                // Turn cell I,J,K into node index
                vtkIdType nodeId = K*dims[1]*dims[0] + J*dims[0] + I;

                curve[i*2  ] = sgrid->GetPoint(nodeId)[comp];
                curve[i*2+1] = arr->GetTuple1(indices[i]); 
            }
        }
        else
        {
            for(int i = 0; i < nvals; ++i)
            {
                curve[i*2  ] = sgrid->GetPoint(indices[i])[comp];
                curve[i*2+1] = arr->GetTuple1(indices[i]);
            }
        }
        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: SpreadsheetViewer::DisplayCurve
//
// Purpose: 
//   Display curve data in a new window.
//
// Arguments:
//   vals : The values to display.
//   nvals : The number of values.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:52:44 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::DisplayCurve(const double *vals, int nvals)
{
    SpreadsheetCurveViewer *v = new SpreadsheetCurveViewer(plot, this);
    v->setData(vals, nvals);
    v->show();
}

// ****************************************************************************
// Method: SpreadsheetViewer::operationCurveX
//
// Purpose: 
//   Extract a row of data, match it with the X values and make a curve.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:52:04 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::operationCurveX()
{
    if(nTables > 0)
    {
        // These indices are indices into the arr array.
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        int nvals = 0;
        vtkIdType *indices = t->selectedRowIndices(nvals);
        if(nvals > 0)
        {
            double *curve = new double[nvals * 2];
            if(GetDataVsCoordinate(curve, indices, nvals, 0))
            {
                DisplayCurve(curve, nvals);
            }

            delete [] curve;
            delete [] indices;
        }
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::operationCurveY
//
// Purpose: 
//   Extract a column of data, match it with the X values and make a curve.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:52:04 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::operationCurveY()
{
    if(nTables > 0)
    {
        // These indices are indices into the arr array.
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentWidget();
        int nvals = 0;
        vtkIdType *indices = t->selectedColumnIndices(nvals);
        if(nvals > 0)
        {
            double *curve = new double[nvals * 2];
            if(GetDataVsCoordinate(curve, indices, nvals, 1))
            {
                DisplayCurve(curve, nvals);
            }

            delete [] curve;
            delete [] indices;
        }
    }
}

// ****************************************************************************
// Method: SpreadsheetViewer::tableSelectionChanged
//
// Purpose: 
//   This slot is called by any table when its selection changes. We use it to
//   ensure that the menus are updated appropriately for the active table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 13:27:01 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 11:16:34 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetViewer::tableSelectionChanged()
{
    updateMenuEnabledState(zTabs->indexOf((QWidget *)sender()));
}

// ****************************************************************************
// Method: SpreadsheetViewer::GetCell
//
// Purpose: 
//     Given a pick location, this determines which cell the pick location lies
//     in.
//
// Arguments:
//     X       The x location of the pick.
//     Y       The y location of the pick.
//     Z       The z location of the pick.
//
// Returns:    The index of the cell that was picked.  <0 for errors.
//
// Programmer: Hank Childs
// Creation:   September 4, 2007
//
// Modifications:
//   
// ****************************************************************************

int
SpreadsheetViewer::GetCell(double X, double Y, double Z)
{
    int  i;
    int  cell = -1;

    if (input == NULL)
        return -1;

    double pt[3] = { X, Y, Z };

    int prevPicks = cellId.size();
    for (i = 0 ; i < prevPicks ; i++)
    {
        if (pickPt[3*i] == X && pickPt[3*i+1] == Y && pickPt[3*i+2] == Z)
            return cellId[i];
    }

    if (input->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) input;
        int ijk[3];
        bool success =
                 vtkVisItUtility::ComputeStructuredCoordinates(rgrid, pt, ijk);
        if (!success)
            return -1;
        int dims[3];
        rgrid->GetDimensions(dims);
        cell = ijk[2]*(dims[0]-1)*(dims[1]-1) + ijk[1]*(dims[0]-1) + ijk[0];
    }
    else
    {
        vtkVisItCellLocator *loc = vtkVisItCellLocator::New();
        loc->SetDataSet(input);
        loc->BuildLocator();
       
        int subId = 0;
        double cp[3] = {0., 0., 0.};
        int foundCell;
        double dist;
        int success = loc->FindClosestPointWithinRadius(pt, FLT_MAX, cp,
                                                   foundCell, subId, dist);
        loc->Delete();

        if (foundCell < 0)
            cell = -1;
        else
            cell = foundCell;
    }

    pickPt.push_back(X);
    pickPt.push_back(Y);
    pickPt.push_back(Z);
    cellId.push_back(cell);

    return cell;
}

