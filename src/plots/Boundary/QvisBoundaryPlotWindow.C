// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisBoundaryPlotWindow.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QRadioButton>
#include <QSlider>

#include <QvisColorButton.h>
#include <QvisColorSwatchListWidget.h>
#include <QvisColorTableWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <BoundaryAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::QvisBoundaryPlotWindow
//
// Purpose: 
//   Constructor for the QvisBoundaryPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Jeremy Meredith, Fri Jan  2 17:26:04 EST 2009
//    Added Load/Save buttons.  (Other plot windows get this by default.)
//   
// ****************************************************************************

QvisBoundaryPlotWindow::QvisBoundaryPlotWindow(const int type,
    BoundaryAttributes *boundaryAtts_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(boundaryAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtonsAndLoadSave,
                               false)
{
    plotType     = type;
    boundaryAtts = boundaryAtts_;

    // Initialize widgets that we'll have to delete manually. These are
    // parentless widgets.
    colorModeButtons = 0;
    smoothingLevelButtons = 0;
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::~QvisBoundaryPlotWindow
//
// Purpose: 
//   Destructor for the QvisBoundaryPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

QvisBoundaryPlotWindow::~QvisBoundaryPlotWindow()
{
    boundaryAtts = 0;

}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Jeremy Meredith
// Creation:   June 12, 2003
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Added pointControl.
//
//   Brad Whitlock, Wed Jul 20 17:58:11 PST 2005
//   Added a new slot
//
//   Brad Whitlock, Tue Apr 22 16:24:27 PDT 2008
//   Added tr()'s.
//
//   Brad Whitlock, Thu Jul 17 11:35:40 PDT 2008
//   Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 17:45:38 MST 2011
//   Changed colorTableButton to colorTableWidget to gain the invert toggle.
//
//   Kathleen Biagas, Tue Dec 20 13:58:48 PST 2016
//   Removed point controls.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::CreateWindowContents()
{
    // Create the boundary color group box.
    boundaryColorGroup = new QGroupBox(central);
    boundaryColorGroup->setTitle(tr("Boundary colors"));
    topLayout->addWidget(boundaryColorGroup);

    QGridLayout *colorLayout = new QGridLayout(boundaryColorGroup);
    colorLayout->setContentsMargins(5,5,5,5);
    colorLayout->setSpacing(10);
    colorLayout->setColumnStretch(2, 1000);

    // Create the mode buttons that determine if the window is in single
    // or multiple color mode.
    colorModeButtons = new QButtonGroup(boundaryColorGroup);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(colorModeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
#else
    connect(colorModeButtons, SIGNAL(idClicked(int)),
            this, SLOT(colorModeChanged(int)));
#endif

    QRadioButton *rb = new QRadioButton(tr("Color table"), boundaryColorGroup);
    colorModeButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton(tr("Single"), boundaryColorGroup);
    colorModeButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 2, 0);
    rb = new QRadioButton(tr("Multiple"), boundaryColorGroup);
    colorModeButtons->addButton(rb, 2);
    colorLayout->addWidget(rb, 3, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(boundaryColorGroup);
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 2, 1);

    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        boundaryColorGroup, NULL);
    singleColorOpacity->setTickInterval(64);
    singleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    colorLayout->addWidget(singleColorOpacity, 2, 2);

    // Try adding the multiple color button.
    multipleColor = new QvisColorButton(boundaryColorGroup);
    connect(multipleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(multipleColorChanged(const QColor &)));
    colorLayout->addWidget(multipleColor, 3, 1);

    // Create the multiple color opacity.
    multipleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        boundaryColorGroup, NULL);
    multipleColorOpacity->setTickInterval(64);
    multipleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(multipleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(multipleColorOpacityChanged(int)));
    colorLayout->addWidget(multipleColorOpacity, 3, 2);

    // Create the multiple color list widget.
    multipleColorList = new QvisColorSwatchListWidget(boundaryColorGroup);
    multipleColorList->setMinimumHeight(100);
    connect(multipleColorList, SIGNAL(itemSelectionChanged()),
            this, SLOT(boundarySelectionChanged()));
    colorLayout->addWidget(multipleColorList, 4, 1, 1, 2);
    multipleColorLabel = new QLabel(tr("Boundaries"), boundaryColorGroup);
    multipleColorLabel->setBuddy(multipleColorList);
    colorLayout->addWidget(multipleColorLabel, 4, 0, Qt::AlignRight);

    // Create the color table widget
    colorTableWidget = new QvisColorTableWidget(boundaryColorGroup, true);
    connect(colorTableWidget, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    connect(colorTableWidget,
            SIGNAL(invertColorTableToggled(bool)),
            this,
            SLOT(invertColorTableToggled(bool)));
    colorLayout->addWidget(colorTableWidget, 1, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

    // Create the overall opacity.
    QHBoxLayout *opLayout = new QHBoxLayout(0);
    opLayout->setContentsMargins(0,0,0,0);
    opLayout->setSpacing(5);
    colorLayout->addLayout(opLayout, 5, 0, 1, 3);

    overallOpacity = new QvisOpacitySlider(0, 255, 25, 255, central, 
                     NULL);
    overallOpacity->setTickInterval(64);
    overallOpacity->setGradientColor(QColor(0, 0, 0));
    connect(overallOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(overallOpacityChanged(int)));

    QLabel *overallOpacityLabel = new QLabel(tr("Opacity"), central);
    overallOpacityLabel->setBuddy(overallOpacity);
    overallOpacityLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    opLayout->addWidget(overallOpacityLabel);
    opLayout->addWidget(overallOpacity);
    opLayout->setStretchFactor(overallOpacity, 10);

    //
    // Create the option stuff
    //
    QGroupBox * optionscGroup = new QGroupBox(central);
    optionscGroup->setTitle(tr("Options"));
    topLayout->addWidget(optionscGroup);

    QGridLayout *optionsLayout = new QGridLayout(optionscGroup);
    optionsLayout->setContentsMargins(5,5,5,5);
    optionsLayout->setSpacing(10);
 
    // Create the wireframe toggle
    wireframeToggle = new QCheckBox(tr("Wireframe"), central);
    connect(wireframeToggle, SIGNAL(toggled(bool)),
            this, SLOT(wireframeToggled(bool)));
    optionsLayout->addWidget(wireframeToggle, 0, 0);

    //
    // Create the style stuff
    //

    QGroupBox * styleGroup = new QGroupBox(central);
    styleGroup->setTitle(tr("Lines"));
    topLayout->addWidget(styleGroup);

    QGridLayout *styleLayout = new QGridLayout(styleGroup);
    styleLayout->setContentsMargins(5,5,5,5);
    styleLayout->setSpacing(10);
 
    // Create the lineWidth widget.
    styleLayout->addWidget(new QLabel(tr("Line width"), central), 1, 2);

    lineWidth = new QvisLineWidthWidget(0, central);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    styleLayout->addWidget(lineWidth, 1, 3);

    //
    // Create the geometry group
    //
    QGroupBox * smoothingGroup = new QGroupBox(central);
    smoothingGroup->setTitle(tr("Geometry"));
    topLayout->addWidget(smoothingGroup);

    QGridLayout *smoothingLayout = new QGridLayout(smoothingGroup);
    smoothingLayout->setContentsMargins(5,5,5,5);
    smoothingLayout->setSpacing(10);

    smoothingLayout->addWidget(new QLabel(tr("Smoothing"), central), 0,0);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#else
    connect(smoothingLevelButtons, SIGNAL(idClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#endif

    rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb, 0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb, 1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb, 2);
    smoothingLayout->addWidget(rb, 0, 3);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setContentsMargins(5,5,5,5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   June 12, 2003
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Added pointControl cases.
//
//   Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//   Fixed SGI compiler error with string conversion to QString
//
//   Brad Whitlock, Wed Jul 20 17:58:45 PST 2005
//   Added pointSizePixels.
//
//   Hank Childs, Thu Jun  8 13:41:24 PDT 2006
//   Fix compiler warning for casting.
//
//   Brad Whitlock, Thu Jul 17 11:43:27 PDT 2008
//   Qt 4.
//
//   Kathleen Bonnell, Mon Jan 17 17:45:38 MST 2011
//   Changed colorTableButton to colorTableWidget to gain the invert toggle.
//
//   Kathleen Biagas, Tue Dec 20 13:59:24 PST 2016
//   Removed filledFlag, boundaryType and point related.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    bool updateMultiple = false;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < boundaryAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!boundaryAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case BoundaryAttributes::ID_colorType:
            if(boundaryAtts->GetColorType() == BoundaryAttributes::ColorBySingleColor) 
                colorModeButtons->button(1)->setChecked(true);
            else if(boundaryAtts->GetColorType() == BoundaryAttributes::ColorByMultipleColors) 
                colorModeButtons->button(2)->setChecked(true);
            else
                colorModeButtons->button(0)->setChecked(true);
            break;
        case BoundaryAttributes::ID_colorTableName:
            colorTableWidget->setColorTable(boundaryAtts->GetColorTableName().c_str());
            break;
        case BoundaryAttributes::ID_invertColorTable:
            colorTableWidget->setInvertColorTable(boundaryAtts->GetInvertColorTable());
            break;
        case BoundaryAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(boundaryAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case BoundaryAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(boundaryAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case BoundaryAttributes::ID_singleColor:
            { // new scope
            QColor temp(boundaryAtts->GetSingleColor().Red(),
                        boundaryAtts->GetSingleColor().Green(),
                        boundaryAtts->GetSingleColor().Blue());
            singleColor->blockSignals(true);
            singleColor->setButtonColor(temp);
            singleColor->blockSignals(false);

            singleColorOpacity->setGradientColor(temp);
            singleColorOpacity->blockSignals(true);
            singleColorOpacity->setValue(boundaryAtts->GetSingleColor().Alpha());
            singleColorOpacity->blockSignals(false);
            }
            break;
        case BoundaryAttributes::ID_multiColor:
            updateMultiple = true;
            break;
        case BoundaryAttributes::ID_boundaryNames:
            updateMultiple = true;
            break;
        case BoundaryAttributes::ID_opacity:
            overallOpacity->blockSignals(true);
            overallOpacity->setValue((int)(boundaryAtts->GetOpacity() * 255.f));
            overallOpacity->blockSignals(false);
            break;
        case BoundaryAttributes::ID_wireframe:
            wireframeToggle->blockSignals(true);
            wireframeToggle->setChecked(boundaryAtts->GetWireframe());
            wireframeToggle->blockSignals(false);
            break;
        case BoundaryAttributes::ID_smoothingLevel:
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->button(boundaryAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelButtons->blockSignals(false);
            break;
        }
    } // end for

    // If we need to update multiple colors then do it.
    if(updateMultiple)
        UpdateMultipleArea();

    //
    // Set the enabled state for the color-related widgets.
    //
    singleColor->setEnabled(boundaryAtts->GetColorType() ==
        BoundaryAttributes::ColorBySingleColor);
    singleColorOpacity->setEnabled(boundaryAtts->GetColorType() ==
        BoundaryAttributes::ColorBySingleColor);
    bool mEnabled = (boundaryAtts->GetMultiColor().GetNumColors() > 0) &&
                    (boundaryAtts->GetColorType() == BoundaryAttributes::ColorByMultipleColors);
    multipleColorLabel->setEnabled(mEnabled);
    multipleColorList->setEnabled(mEnabled);
    multipleColor->setEnabled(mEnabled);
    multipleColorOpacity->setEnabled(mEnabled);
    colorTableWidget->setEnabled(boundaryAtts->GetColorType() ==
        BoundaryAttributes::ColorByColorTable);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::UpdateMultipleArea
//
// Purpose: 
//   This method updates the multipleColors widget with the list of boundary
//   names.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 12:12:45 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::UpdateMultipleArea()
{
    const stringVector &matNames = boundaryAtts->GetBoundaryNames();
    intVector selectedBoundaries;
    bool update = true;
    QListWidgetItem *item;

    multipleColorList->blockSignals(true);

    //
    // Make a pass through the widget to see how many boundaries are selected.
    //
    for(int i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->item(i)->isSelected())
            selectedBoundaries.push_back(i);
    }

    //
    // Modify the names.
    //
    if(matNames.size() == (size_t)multipleColorList->count())
    {
        // See if the strings are the same..
        bool same = true;
        for(int i = 0; i < (int)matNames.size() && same; ++i)
        {
            ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
            
            same &= CompareItem(i, QString(matNames[i].c_str()),
                                   QColor(c.Red(), c.Green(), c.Blue()));
        }

        // If the strings are not the same then modifiy the widgets.
        if(!same)
        {
            for(int i = 0; i < (int)matNames.size(); ++i)
                UpdateItem(i);
        }
        else
            update = false;
    }
    else if(matNames.size() > (size_t)multipleColorList->count())
    {
        // Set all of the existing names.
        for(int i = 0; i < multipleColorList->count(); ++i)
            UpdateItem(i);

        // Add new entries
        for(int i = multipleColorList->count(); i < (int)matNames.size(); ++i)
        {
            QString        itemText(matNames[i].c_str());
            ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
            QColor         itemColor(c.Red(), c.Green(), c.Blue());
            multipleColorList->addItem(matNames[i].c_str(), itemColor);
        }
    }
    else // if(matNames.size() < multipleColorList->count())
    {
        // Set all of the existing names.
        for(int i = 0; i < (int)matNames.size(); ++i)
            UpdateItem(i);

        // Remove excess entries
        int numEntries = multipleColorList->count();
        for(int i = (int)matNames.size(); i < numEntries; ++i)
        {
            QListWidgetItem *item = multipleColorList->takeItem(multipleColorList->count() - 1);
            if(item != 0)
                delete item;
        }
    }

    //
    // Now that the names have been modified, do the selection.
    //
    if(update)
    {
        multipleColorList->clearSelection();
        bool noneSelected = true, first = true;
        for(size_t i = 0; i < selectedBoundaries.size(); ++i)
        {
            if(selectedBoundaries[i] < multipleColorList->count())
            {
                item = multipleColorList->item(selectedBoundaries[i]);
                item->setSelected(true);
                if(first)
                {
                    first = false;
                    multipleColorList->setCurrentItem(item);
                }
                noneSelected = false;
            }
        }

        // If there are no boundaries selected then select the first. If there 
        // is more than one boundary selected then update the listbox to cover
        // the case where we have to update the color for more than one
        // listboxitem.
        if(noneSelected && multipleColorList->count() > 0)
        {
            item = multipleColorList->item(0);
            item->setSelected(true);
            multipleColorList->setCurrentItem(item);
        }
    }

    //
    // If there is anything to select, then set the color widgets so they
    // show the first
    //
    if(matNames.size() > 0)
    {
        int selectedIndex = (selectedBoundaries.size() > 0) ?
            selectedBoundaries[0] : 0;
        // Make sure that the selected index is in the range of visible colors.
        if(selectedIndex >= (int)matNames.size())
            selectedIndex = 0;

        SetMultipleColorWidgets(selectedIndex);
    }

    multipleColorList->blockSignals(false);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::CompareItem
//
// Purpose: 
//   Compares an item against a name and a color.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 11:54:42 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisBoundaryPlotWindow::CompareItem(int i, const QString &name, 
    const QColor &c) const
{
    QString itemName(multipleColorList->text(i));
    QColor  itemColor(multipleColorList->color(i));
    return  itemName == name && itemColor == c;
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::UpdateItem
//
// Purpose: 
//   Sets the i'th listbox item to the current colors for that item.
//
// Arguments:
//   i : The index of the item to be updated.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 12:10:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::UpdateItem(int i)
{
    multipleColorList->setText(i, boundaryAtts->GetBoundaryNames()[i].c_str());
    ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
    multipleColorList->setColor(i, QColor(c.Red(), c.Green(), c.Blue()));
}

// ****************************************************************************
// Method: QvisBoundaryWindow::SetMultipleColorWidgets
//
// Purpose: 
//   Sets the colors for the multiple color widgets.
//
// Arguments:
//   index : The index into the boundary list.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::SetMultipleColorWidgets(int index)
{
    ColorAttribute c(boundaryAtts->GetMultiColor()[index]);
    QColor temp(c.Red(), c.Green(), c.Blue());
    multipleColor->blockSignals(true);
    multipleColor->setButtonColor(temp);
    multipleColor->blockSignals(false);

    multipleColorOpacity->setGradientColor(temp);
    multipleColorOpacity->blockSignals(true);
    multipleColorOpacity->setValue(c.Alpha());
    multipleColorOpacity->blockSignals(false);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::Apply
//
// Purpose: 
//   This method applies the boundary plot attributes and optionally
//   tells the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            boundary plot attributes.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Uncommented GetCurrentValues.
//   
//   Kathleen Biagas, Tue Dec 20 14:01:16 PST 2016
//   Removed call to no-longer needed GetCurrentValues.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        boundaryAtts->Notify();

        // Tell the viewer to set the boundary plot attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        boundaryAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Uncommented GetCurrentValues.
//   
//   Kathleen Biagas, Tue Dec 20 14:01:16 PST 2016
//   Removed call to no-longer needed GetCurrentValues.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::makeDefault()
{
    // Tell the viewer to set the default boundary plot attributes.
    boundaryAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::reset()
{
    // Tell the viewer to reset the boundary plot attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisBoundaryPlotWindow::lineWidthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line width widget is changed.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::lineWidthChanged(int newWidth)
{
    boundaryAtts->SetLineWidth(newWidth);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   legend toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::legendToggled(bool val)
{
    boundaryAtts->SetLegendFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::wireframeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   wireframe toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::wireframeToggled(bool val)
{
    boundaryAtts->SetWireframe(val);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::colorModeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single/multiple color
//   radio buttons are clicked.
//
// Arguments:
//   index : The index of the button that called this method. If index is 0
//           then the single color button was clicked, otherwise the multiple
//           colors button was clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::colorModeChanged(int index)
{
    if(index == 0)
        boundaryAtts->SetColorType(BoundaryAttributes::ColorByColorTable);
    else if(index == 1)
        boundaryAtts->SetColorType(BoundaryAttributes::ColorBySingleColor);
    else
        boundaryAtts->SetColorType(BoundaryAttributes::ColorByMultipleColors);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::singleColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single color button's
//   color changes.
//
// Arguments:
//   color : The new single color.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue(),
                        boundaryAtts->GetSingleColor().Alpha());
    boundaryAtts->SetSingleColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::singleColorOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single color opacity
//   changes.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::singleColorOpacityChanged(int opacity)
{
    ColorAttribute temp(boundaryAtts->GetSingleColor());
    temp.SetAlpha(opacity);
    boundaryAtts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::multipleColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new color is selected
//   for one of the color buttons in the multiple colors area.
//
// Arguments:
//   color : The new color for the button.
//   index : The index of the color that changed.
// 
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:50:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::multipleColorChanged(const QColor &color)
{
    // If any boundaries are selected, change their colors.
    if(multipleColorList->currentItem() != 0)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->item(i)->isSelected() &&
               (i < boundaryAtts->GetMultiColor().GetNumColors()))
            {
                boundaryAtts->GetMultiColor()[i].SetRgb(color.red(),
                                                      color.green(),
                                                      color.blue());
            }
        }

        boundaryAtts->SelectMultiColor();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::opacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the opacity changes for one
//   of the boundaries in the multiple colors area.
//
// Arguments:
//   opacity : The new opacity.
//   index   : The index of the boundary that changed.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:51:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::multipleColorOpacityChanged(int opacity)
{
    // If any boundaries are selected, change their opacities.
    if(multipleColorList->currentItem() != 0)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->item(i)->isSelected() &&
               (i < boundaryAtts->GetMultiColor().GetNumColors()))
            {
                boundaryAtts->GetMultiColor()[i].SetAlpha(opacity);
            }
        }

        boundaryAtts->SelectMultiColor();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisBoundaryPlotwindow::boundarySelectionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the boundary selection
//   changes.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:52:02 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::boundarySelectionChanged()
{
    // Go through until we find the first selected item.
    int index = -1;
    for(int i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->item(i)->isSelected())
        {
            index = i;
            break;
        }
    }

    if(index != -1)
    {
        SetMultipleColorWidgets(index);
    }
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::overallOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the overall opacity slider
//   is clicked.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::overallOpacityChanged(int opacity)
{
    boundaryAtts->SetOpacity(double(opacity) / 255.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisBoundaryPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer: Jeremy Meredith
//  Creation:    May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
// ****************************************************************************

void
QvisBoundaryPlotWindow::smoothingLevelChanged(int level)
{
    boundaryAtts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table into the
//   boundary plot attributes.
//
// Arguments:
//   useDefault : Whether or not to use the default color table.
//   ctName     : The name of the color table to use.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::colorTableClicked(bool useDefault, const QString &ctName)
{
    boundaryAtts->SetColorTableName(ctName.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisBoundaryPlotWindow::invertColorTableToggled
//
// Purpose: 
//   This is a Qt slot function that sets the invert color table flag into the
//   boundary plot attributes.
//
// Arguments:
//   val    :  Whether or not to invert the color table.
//
// Programmer: Kathleen Bonnell
// Creation:   January  17, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryPlotWindow::invertColorTableToggled(bool val)
{
    boundaryAtts->SetInvertColorTable(val);
    Apply();
}

