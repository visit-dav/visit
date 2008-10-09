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

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qframe.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <SpreadsheetTable.h>
#include <qtabwidget.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <QvisColorTableButton.h>
#include <QvisVariableButton.h>
#include <plugin_vartypes.h>

#include <PlotList.h>
#include <Plot.h>

#include <SpreadsheetTable.h>
#include <SpreadsheetTabWidget.h>
#include <avtLookupTable.h>

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
// ****************************************************************************

SpreadsheetViewer::SpreadsheetViewer(ViewerPlot *p, QWidget *parent, 
    const char *name) : QMainWindow(parent, name),
    Observer((Subject*)p->GetPlotAtts()), cachedAtts(), menuPopulator()
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
    setCaption(tr("Spreadsheet"));

    QFrame *top = new QFrame(this, "vbox");
    setCentralWidget(top);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    topLayout->setSpacing(5);
    topLayout->setMargin(10);
#ifdef Q_WS_MAC
    QHBox *menuContainer = new QHBox(top, "menuContainer");
    topLayout->addWidget(menuContainer);
#endif
    QHBoxLayout *layout = new QHBoxLayout(topLayout);
    layout->setSpacing(5);

    //
    // 3D controls
    //
    controls3D = new QGroupBox(tr("3D"), top, "controls3D");
    layout->addWidget(controls3D, 10);
    QVBoxLayout *inner3D = new QVBoxLayout(controls3D);
    inner3D->addSpacing(10);
    inner3D->setMargin(10);
    QGridLayout *layout3D = new QGridLayout(inner3D, 2, 3);
    layout3D->setSpacing(5);
    inner3D->addStretch(1);

    kLabel = new QLabel("k [1,1]", controls3D, "kLabel");
    layout3D->addWidget(kLabel, 0, 0);

    kSlider = new QSlider(controls3D, "kSlider");
    kSlider->setOrientation(Qt::Horizontal);
    kSlider->setPageStep(1);
    connect(kSlider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(kSlider, SIGNAL(sliderPressed()),
            this, SLOT(sliderPressed()));
    connect(kSlider, SIGNAL(sliderReleased()),
            this, SLOT(sliderReleased()));

    layout3D->addMultiCellWidget(kSlider, 0, 0, 1, 2);

    normalLabel = new QLabel(tr("Normal"), controls3D, "normalLabel");
    layout3D->addWidget(normalLabel, 1, 0);

    normalButtonGroup = new QButtonGroup (0, "normalButtonGroup");
    connect(normalButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(normalChanged(int)));
    normalRadioButtons = new QHBox(controls3D, "normalRadioButtons");
    layout3D->addWidget(normalRadioButtons, 1, 1);
    normalButtonGroup->insert(new QRadioButton(tr("X"), normalRadioButtons, "rb0"), 0);
    normalButtonGroup->insert(new QRadioButton(tr("Y"), normalRadioButtons, "rb1"), 1);
    QRadioButton *rb = new QRadioButton(tr("Z"), normalRadioButtons, "rb2");
    normalButtonGroup->insert(rb, 2);
    normalRadioButtons->setStretchFactor(rb, 5);

    //
    // Display controls
    //
    QGroupBox *display = new QGroupBox(tr("Display"), top, "display");
    layout->addWidget(display);
    QVBoxLayout *innerDisplay = new QVBoxLayout(display);
    innerDisplay->addSpacing(10);
    innerDisplay->setMargin(10);
    QGridLayout *layoutDisplay = new QGridLayout(innerDisplay, 3, 2);
    layoutDisplay->setSpacing(5);

    formatLabel = new QLabel(tr("Format"), display, "formatLabel");
    layoutDisplay->addWidget(formatLabel, 0, 0);
    formatLineEdit = new QLineEdit(display, "formatLineEdit");
    connect(formatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(formatChanged()));
    layoutDisplay->addWidget(formatLineEdit, 0, 1);

    colorTableCheckBox = new QCheckBox(tr("Color"), display, "colorTableCheckBox");
    connect(colorTableCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(colorTableCheckBoxToggled(bool)));
    layoutDisplay->addWidget(colorTableCheckBox, 1, 0);

    // Just a push button for now. It will be a color table button later.
    colorTableButton = new QvisColorTableButton(display, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(selectedColorTable(bool, const QString &)));
    layoutDisplay->addWidget(colorTableButton, 1, 1);

    //
    // Show in viswindow controls
    //
    QGroupBox *show = new QGroupBox(1, Qt::Vertical, tr("Show in visualizaion window"), top, "show");
    topLayout->addWidget(show);
    tracerCheckBox = new QCheckBox(tr("Tracer plane"), show, "tracerCheckBox");
    connect(tracerCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(tracerCheckBoxToggled(bool)));
    patchOutlineCheckBox = new QCheckBox(tr("Patch outline"), show, "patchOutline");
    connect(patchOutlineCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(outlineCheckBoxToggled(bool)));
    currentCellOutlineCheckBox = new QCheckBox(tr("Current cell outline"), show, "currentCellOutline");
    connect(currentCellOutlineCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showCurrentCellOutlineCheckBoxToggled(bool)));

    //
    // Tables
    //
    zTabs = new SpreadsheetTabWidget(top, "zTabs");
    topLayout->addWidget(zTabs, 10);
    nTables = 1;
    nTablesForSlider = 1;
    tables = new SpreadsheetTable*[1];
    tables[0] = new SpreadsheetTable(0, "table");
    tables[0]->setNumRows(20);
    tables[0]->setNumCols(20);
    tables[0]->setReadOnly(true);
    tables[0]->setLUT(colorLUT);
    QFont spreadsheetFont;
    if (spreadsheetFont.fromString(plotAtts->GetSpreadsheetFont().c_str()))
        tables[0]->setFont(spreadsheetFont);

    connect(tables[0], SIGNAL(selectionChanged()),
            this, SLOT(tableSelectionChanged()));
    zTabs->addTab(tables[0], "k=1");
    connect(zTabs, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(tabChanged(QWidget*)));

    //
    // Variables and min,max buttons
    //
    QGridLayout *varLayout = new QGridLayout(topLayout, 2, 3);
    varLayout->setSpacing(5);
    varLayout->setColStretch(1, 5);
    varLayout->setColStretch(2, 5);
    varLabel = new QLabel(tr("Variable"), top, "varLabel");
    varLayout->addWidget(varLabel, 0, 0);
    // Have to display metadata -- the list of variables.
    varButton = new QvisVariableButton(false, false, true, 
        QvisVariableButton::Scalars, top, "varComboBox");
    connect(varButton, SIGNAL(activated(const QString &)),
            this, SLOT(changedVariable(const QString &)));
    varLayout->addMultiCellWidget(varButton, 0, 0, 1, 2);    

    // min, max buttons
    minButton = new QPushButton(tr("Min = "), top, "minButton");
    connect(minButton, SIGNAL(clicked()),
            this, SLOT(minClicked()));
    varLayout->addMultiCellWidget(minButton, 1,1,1,1);

    maxButton = new QPushButton(tr("Max = "), top, "maxButton");
    connect(maxButton, SIGNAL(clicked()),
            this, SLOT(maxClicked()));
    varLayout->addMultiCellWidget(maxButton, 1,1,2,2);


    //
    // Do the main menu.
    //
    filePopup = new QPopupMenu(this);
#ifdef Q_WS_MAC
    QPushButton *fileButton = new QPushButton(tr("&File"), menuContainer, "fileButton");
    fileButton->setPopup(filePopup);
#else
    saveMenuId = menuBar()->insertItem(tr("&File"), filePopup);
#endif
    saveMenu_SaveTextId = filePopup->insertItem(tr("Save as text . . ."), this, SLOT(saveAsText()), CTRL+Key_S);

    editPopup = new QPopupMenu(this);
#ifdef Q_WS_MAC
    QPushButton *editButton = new QPushButton(tr("&Edit"), menuContainer, "editButton");
    editButton->setPopup(editPopup);
#else
    editMenuId = menuBar()->insertItem(tr("&Edit"), editPopup);
#endif
    editMenu_CopyId = editPopup->insertItem(tr("&Copy"), this, SLOT(copySelectionToClipboard()), CTRL+Key_C);
    editPopup->insertSeparator();
    editPopup->insertItem(tr("Select &All"), this, SLOT(selectAll()), CTRL+Key_A);
    editPopup->insertItem(tr("Select &None"), this, SLOT(selectNone()), CTRL+Key_N);

    operationsPopup = new QPopupMenu(this);
#ifdef Q_WS_MAC
    QPushButton *opButton = new QPushButton(tr("&Operations"), menuContainer, "opButton");
    opButton->setPopup(operationsPopup);
#else
    operationMenuId = menuBar()->insertItem(tr("&Operations"), operationsPopup);
#endif
    operationsPopup->insertItem(tr("Sum"), this, SLOT(operationSum()));
    operationsPopup->insertItem(tr("Average"), this, SLOT(operationAverage()));
    updateMenuEnabledState(tables[0]);
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
//   
// ****************************************************************************

SpreadsheetViewer::~SpreadsheetViewer()
{
    input = 0;

    // Make sure that we don't cause any tabChanged signals when we later
    // delete the tabs so disconnect the signal.
    disconnect(zTabs, SIGNAL(currentChanged(QWidget*)),
               this, SLOT(tabChanged(QWidget*)));

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
                zTabs->showPage(tables[plotAtts->GetSliceIndex()]);
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
        setCaption(caption);

        // Set the variable in the variable button based on the plot's
        // active variable.
        updateVariableMenus();
        varButton->blockSignals(true);
        varButton->setVariable(plot->GetVariableName().c_str());
        varButton->blockSignals(false);

        // Update the menu enabled state.
        updateMenuEnabledState((QTable *)zTabs->currentPage());
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
        QTable *page = (QTable*)zTabs->currentPage();
        if(page != 0)
            page->updateContents();
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
        case 0: //subsetName
            if (cachedAtts.GetSubsetName() != plotAtts->GetSubsetName())
            {
                // Invalidate pointer to data set so that pick information
                // is not applied to the incorrect subset
                input = 0; 
            }
            break;
        case 1: //formatString
            formatLineEdit->setText(plotAtts->GetFormatString().c_str());

            // If we've changed format strings then we need to update the spreadsheet.
            if(cachedAtts.GetFormatString() != plotAtts->GetFormatString())
                needsUpdate = true;
            break;
        case 2: //useColorTable
            colorTableButton->setEnabled(plotAtts->GetUseColorTable());
            colorTableCheckBox->blockSignals(true);
            colorTableCheckBox->setChecked(plotAtts->GetUseColorTable());
            colorTableCheckBox->blockSignals(false);

            // If we've changed then we need to update the spreadsheet.
            if(cachedAtts.GetUseColorTable() != plotAtts->GetUseColorTable())
                 needsUpdate = true;
            break;
        case 3: //colorTableName
            colorTableButton->setText(plotAtts->GetColorTableName().c_str());

            // If we've changed then we need to update the spreadsheet.
            if(cachedAtts.GetColorTableName() != plotAtts->GetColorTableName())
                 needsUpdate = true;
            break;
        case 4: //showTracerPlane
            tracerCheckBox->blockSignals(true);
            tracerCheckBox->setChecked(plotAtts->GetShowTracerPlane());
            tracerCheckBox->blockSignals(false);
            break;
        case 5: //tracerColor
            zTabs->setHighlightColor(QColor(plotAtts->GetTracerColor().Red(),
                                            plotAtts->GetTracerColor().Green(),
                                            plotAtts->GetTracerColor().Blue()));
            break;
        case 6: //normal
            normalButtonGroup->blockSignals(true);
            normalButtonGroup->setButton(plotAtts->GetNormal());
            normalButtonGroup->blockSignals(false);

            // If we've changed normals then we need to update the spreadsheet.
            if(cachedAtts.GetNormal() != plotAtts->GetNormal())
                needsRebuild = true;
            break;
        case 7: //sliceIndex
            sliceIndexSet = true;
#ifdef SINGLE_TAB_WINDOW
            needsRebuild = true;
#endif
            break;
        case 8: //currentPick
        case 9: //currentPickValid
        case 10: //pastPicks
            // Check to see if the pick points changed.
            pickPt.clear();
            cellId.clear();
            needsPickUpdate |= PickPointsChanged();
            break;
        case 13: // fontName
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
        case 14: //showPatchOutline
            patchOutlineCheckBox->blockSignals(true);
            patchOutlineCheckBox->setChecked(plotAtts->GetShowPatchOutline());
            patchOutlineCheckBox->blockSignals(false);
            break;
        case 15: //showCurrentCellOutline
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
            if(tables[i] == zTabs->currentPage())
                 tables[i]->updateContents();
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
            zTabs->showPage(tables[plotAtts->GetSliceIndex()]);
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
    else if(input->IsA("vtkUnstructuredGrid"))
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
    QTable *page = (QTable *)zTabs->currentPage();
    if(page != 0)
        page->updateContents();
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
            tables[t]->setUpdatesEnabled(false);
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
        fmt = QString("Min = ") + QString(plotAtts->GetFormatString().c_str());\
        tmp.sprintf(fmt.latin1(), minValue);\
        minButton->setText(tmp);\
        minButton->setEnabled(true);\
        debug5 << mName << "min=" << minValue << ", minCell=[" << minCell[0] << "," << minCell[1] << "," << minCell[2] << "]" << endl;\
        fmt = QString("Max = ") + QString(plotAtts->GetFormatString().c_str());\
        tmp.sprintf(fmt.latin1(), maxValue);\
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
                int col = dims[2]-1-k;
                for(int j = 0; j < dims[1]; ++j)
                {
                    for(int i = 0; i < dims[0]; ++i, ++index)
                    {
                        // If the data has ghost zones then skip ghosts so they
                        // don't mess up min/max calculations.
                        if(ghostZones != 0 && ghostZones[index] != 0)
                            continue;

                        double *val = arr->GetTuple(index);
                        EVAL_MINMAX(j, i, col /*k*/)
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
                QString name; name.sprintf("%d", i);
                t[i] = new SpreadsheetTable(0, name.latin1());
                t[i]->setUpdatesEnabled(false);
                t[i]->setLUT(colorLUT);
                QFont spreadsheetFont;
                if (spreadsheetFont.fromString(plotAtts->GetSpreadsheetFont().c_str()))
                    t[i]->setFont(spreadsheetFont);
                connect(t[i], SIGNAL(selectionChanged()),
                        this, SLOT(tableSelectionChanged()));
                zTabs->addTab(t[i], name);
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
                zTabs->removePage(tables[i]);
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
            name.sprintf(tr("Unstructured"));
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::X)
            name.sprintf("i=%d", i+base+offset);
        else if(plotAtts->GetNormal() == SpreadsheetAttributes::Y)
            name.sprintf("j=%d", i+base+offset);
        else
            name.sprintf("k=%d", i+base+offset);
        zTabs->setTabLabel(tables[i], name);
    }
    zTabs->blockSignals(false);

    // Always update the label in case we're using a different DB.
    updateSliderLabel();

    // Only update the slider value if the number of tabs changed.
    if(updateSlider)
    {
        kSlider->blockSignals(true);
        kSlider->setMinValue(0);
        kSlider->setMaxValue(ntabs - 1);
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
            debug5 << mName << "base_index = {"
                 << base_index[0] << ", "
                 << base_index[1] << ", "
                 << base_index[2] << "}\n";
        }
        else
            debug5 << mName << "No base index" << endl;
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
    QTable *page = (QTable *)zTabs->currentPage();
    if(page != 0)
        page->updateContents();

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
// ****************************************************************************

void
SpreadsheetViewer::updateMenuEnabledState(QTable *table)
{
    //
    // If the sender is the current page then update the menus based on
    // whether it has any selections.
    //
    if(zTabs->currentPage() == table)
    {
        bool enabled = table->numSelections() > 0;

        filePopup->setItemEnabled(saveMenu_SaveTextId, enabled);
        editPopup->setItemEnabled(editMenu_CopyId, enabled);
#ifndef Q_WS_MAC
        menuBar()->setItemEnabled(operationMenuId, enabled);
#endif
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
//
//   Hank Childs, Sun Oct 28 21:48:23 PST 2007
//   Account for layers of ghost zones when calculating indices.
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
        tables[t]->setCurrentCell(-1,-1);
        tables[t]->clearSelectedCellLabels();
        tables[t]->clearSelection(true);
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
                int row = tables[activeTable]->numRows() - ijk[rowAxis] - 1;

                // Select the new cell in the active table.
                debug1 << mName << "Selecting current cell (" << row << ", "
                       << col << ")" << std::endl;
                QTableSelection sel;
                sel.init(row, col);
                sel.expandTo(row, col);
                tables[activeTable]->addSelection(sel);
                tables[activeTable]->ensureCellVisible(row, col);
                tables[activeTable]->addSelectedCellLabel(row, col, plotAtts->GetCurrentPickLetter());

#ifndef SINGLE_TAB_WINDOW
                debug1 << mName << "Setting current cell (" << row << ", " << col << ")"
                       << std::endl;
                tables[activeTable]->setCurrentCell(row, col);
#else
                if (ijk[sliceAxis] == plotAtts->GetSliceIndex())
                {
                    debug1 << mName << "Setting current cell (" << row << ", " << col << ")"
                           << std::endl;
                    tables[activeTable]->setCurrentCell(row, col);
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
                int oldRow = tables[0]->numRows() - old_ijk[rowAxis] - 1;
                int oldCol = old_ijk[columnAxis];

                // If old pick is in same slice as current pick -> highlight it
                if (old_ijk[sliceAxis] == plotAtts->GetSliceIndex())
                {
                    debug1 << mName << "Highlight cell (" << oldRow << ", "
                        << oldCol << ") in single slice" << endl;
                    QTableSelection sel;
                    sel.init(oldRow, oldCol);
                    sel.expandTo(oldRow, oldCol);
                    tables[0]->addSelection(sel);
                    tables[0]->addSelectedCellLabel(oldRow, oldCol, pastPickLetters[i]);
                }
#else
                // Get row and column of old pick
                int oldRow = tables[old_ijk[sliceAxis]]->numRows() - old_ijk[rowAxis] - 1;
                int oldCol = old_ijk[columnAxis];

                // In multi-tab mode highlight selections in all tables
                if (old_ijk[sliceAxis] <nTables)
                {
                    debug1 << mName << "Highlight cell (" << oldRow << ", "
                           << oldCol << ") in table " << old_ijk[sliceAxis] << endl;
                    QTableSelection sel;
                    sel.init(oldRow, oldCol);
                    sel.expandTo(oldRow, oldCol);
                    tables[old_ijk[sliceAxis]]->addSelection(sel);
                    tables[old_ijk[sliceAxis]]->addSelectedCellLabel(oldRow, oldCol, pastPickLetters[i]);
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
    std::string formatString(formatLineEdit->text().latin1());
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
        zTabs->showPage(tables[tabIndex]);
        zTabs->blockSignals(false);

        updateSliderLabel();
        updateMenuEnabledState(tables[tabIndex]);
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
//   val : The new slice value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:16:44 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewer::tabChanged(QWidget *tab)
{
    if(nTables > 1 && isVisible())
    {
        int index = zTabs->indexOf(tab);
        if(index >= 0)
        {
            SetUpdate(false);
            plotAtts->SetSliceIndex(index);
            plotAtts->Notify();

            kSlider->blockSignals(true);
            kSlider->setValue(index);
            kSlider->blockSignals(false);

            updateSliderLabel();
            updateMenuEnabledState(tables[index]);
        }
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
        zTabs->showPage(tables[minCell[0]]);

        // Remove the selections that may be on the table.
        for(int i = 0; i < tables[minCell[0]]->numSelections(); ++i)
            tables[minCell[0]]->removeSelection(i);

        // Select the new cell.
        QTableSelection sel;
        sel.init(minCell[1], minCell[2]);
        sel.expandTo(minCell[1], minCell[2]);
        tables[minCell[0]]->addSelection(sel);
        tables[minCell[0]]->ensureCellVisible(minCell[1], minCell[2]);
    }
#else
    if(minCell[0] != -1 && minCell[1] != -1 && minCell[2] != -1)
    {
        // Show the right slice of data.
        plotAtts->SetSliceIndex(minCell[0]);
        plotAtts->Notify();

        // Remove the selections that may be on the table.
        for(int i = 0; i < tables[0]->numSelections(); ++i)
            tables[0]->removeSelection(i);

        // Select the new cell.
        QTableSelection sel;
        sel.init(minCell[1], minCell[2]);
        sel.expandTo(minCell[1], minCell[2]);
        tables[0]->addSelection(sel);
        tables[0]->ensureCellVisible(minCell[1], minCell[2]);
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
// ****************************************************************************

void
SpreadsheetViewer::maxClicked()
{
    // maxCell[0] = The index of the table that contains max
    // maxCell[1] = The row of the table that contains max
    // maxCell[2] = The column of the table that contains max
#ifndef SINGLE_TAB_WINDOW
    if(maxCell[0] != -1 && maxCell[1] != -1 && maxCell[2] != -1 &&
       maxCell[0] < nTables)
    {
        // Show the page and don't block signals so we are sure to also
        // update the kSlider via the tabShanged slot
        zTabs->showPage(tables[maxCell[0]]);

        // Remove the selections that may be on the table.
        for(int i = 0; i < tables[maxCell[0]]->numSelections(); ++i)
            tables[maxCell[0]]->removeSelection(i);

        // Select the new cell.
        QTableSelection sel;
        sel.init(maxCell[1], maxCell[2]);
        sel.expandTo(maxCell[1], maxCell[2]);
        tables[maxCell[0]]->addSelection(sel);
        tables[maxCell[0]]->ensureCellVisible(maxCell[1], maxCell[2]);
    }
#else
    if(maxCell[0] != -1 && maxCell[1] != -1 && maxCell[2] != -1)
    {
        // Show the right slice of data.
        plotAtts->SetSliceIndex(maxCell[0]);
        plotAtts->Notify();

        // Remove the selections that may be on the table.
        for(int i = 0; i < tables[0]->numSelections(); ++i)
            tables[0]->removeSelection(i);

        // Select the new cell.
        QTableSelection sel;
        sel.init(maxCell[1], maxCell[2]);
        sel.expandTo(maxCell[1], maxCell[2]);
        tables[0]->addSelection(sel);
        tables[0]->ensureCellVisible(maxCell[1], maxCell[2]);
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
    plotAtts->SetColorTableName(ctName.latin1());
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
    plot->GetViewerMethods()->ChangeActivePlotsVar(newVar.latin1());
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
        QString fileName = QFileDialog::getSaveFileName(tr("selection.txt"),
            tr("Text (*.txt)"));

        // If the user chose to save a file, write it out.
        if(!fileName.isNull())
        {
            SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();
            QString txt(t->selectedCellsAsText());

            // Save the text to a file.
            QFile file(fileName);
            if(file.open(IO_WriteOnly))
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
                plot->Error(err.latin1());
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
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();

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
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();
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
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();
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
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();
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
        SpreadsheetTable *t = (SpreadsheetTable *)zTabs->currentPage();
        double avg = t->selectedCellsAverage();
        QString avgStr;
        avgStr.sprintf(plotAtts->GetFormatString().c_str(), avg);
        QString msg(tr("The average value of the selected cells is: %1.").arg(avgStr));
        QMessageBox::information(this, "Average results", msg, QMessageBox::Ok);
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
//   
// ****************************************************************************

void
SpreadsheetViewer::tableSelectionChanged()
{
    updateMenuEnabledState((QTable *)sender());
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

