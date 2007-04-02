#include <QvisFilledBoundaryPlotWindow.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>

#include <QvisColorButton.h>
#include <QvisColorTableButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <QvisPointControl.h>
#include <FilledBoundaryAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Class: QListBoxTextWithColor
//
// Purpose:
//   This class is the type of item that we put into the listbox. It is 
//   mostly just a stock QListBoxText except that the paint() method draws
//   a square of color to the left of the name.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 1 09:46:11 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Dec 2 09:17:10 PDT 2002
//   Made it look different when it is disabled.
//
// ****************************************************************************

class QListBoxTextWithColor : public QListBoxText
{
public:
    QListBoxTextWithColor(QListBox *listbox, const QString &text,
        const QColor &c) : QListBoxText(listbox, text)
    {
        C = c;
    }

    virtual ~QListBoxTextWithColor() { };
    void setText(const QString &str) { QListBoxItem::setText(str); };
    void setColor(const QColor &c)   { C = c; };
    const QColor &color() const      { return C; };

    void paint(QPainter *painter)
    {
        QFontMetrics fm = painter->fontMetrics();
        QRect r(fm.boundingRect("XX"));
        QBrush brush(C);
        QPen oldPen(painter->pen()), newPen(listBox()->colorGroup().text());

        if(!listBox()->isEnabled())
        {
            brush.setStyle(QBrush::Dense5Pattern);
            painter->fillRect(QRect(0, 0, listBox()->visibleWidth(), height(listBox())),
                QBrush(listBox()->colorGroup().base()));
        }
        else
        {
            if(selected())
            {
                newPen = QPen(listBox()->colorGroup().highlightedText());
                painter->fillRect(QRect(0, 0, listBox()->visibleWidth(), height(listBox())),
                    QBrush(listBox()->colorGroup().highlight()));
            }
            else
            {
                painter->fillRect(QRect(0, 0, listBox()->visibleWidth(), height(listBox())),
                    QBrush(listBox()->colorGroup().base()));
            }
        }
        int fontY = fm.ascent() + fm.leading()/2;
        painter->fillRect(QRect(4, fontY-1, r.width(), height(listBox()) - 2*fontY), brush);
        painter->setPen(newPen);
        painter->drawText(r.width()+8, fontY, text());
        painter->setPen(oldPen);
    }

protected:
    QColor C;
};


// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   Constructor for the QvisFilledBoundaryPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

QvisFilledBoundaryPlotWindow::QvisFilledBoundaryPlotWindow(const int type,
    FilledBoundaryAttributes *boundaryAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(boundaryAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons,
                               false)
{
    plotType     = type;
    boundaryAtts = boundaryAtts_;

    // Initialize widgets that we'll have to delete manually. These are
    // parentless widgets.
    colorModeButtons = 0;
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::~QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   Destructor for the QvisFilledBoundaryPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

QvisFilledBoundaryPlotWindow::~QvisFilledBoundaryPlotWindow()
{
    boundaryAtts = 0;

    // no parents, delete them manually.
    delete colorModeButtons;
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Wed Apr 14 16:43:06 PDT 2004
//    Added mixed color.
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Added pointControl.
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::CreateWindowContents()
{
    QGridLayout *checkBoxLayout = new QGridLayout(topLayout, 2, 4, 10);

    // Create the lineSyle widget.
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    checkBoxLayout->addWidget(lineStyle, 1, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(lineStyle, "Line style",
                                central, "lineStyleLabel");
    checkBoxLayout->addWidget(lineStyleLabel, 1, 0);

    // Create the lineSyle widget.
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    checkBoxLayout->addWidget(lineWidth, 1, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, "Line width",
                                central, "lineWidthLabel");
    checkBoxLayout->addWidget(lineWidthLabel, 1, 2);

    // Create the boundary color group box.
    boundaryColorGroup = new QGroupBox(central, "boundaryColorGroup");
    boundaryColorGroup->setTitle("FilledBoundary colors");
    topLayout->addWidget(boundaryColorGroup);
    QVBoxLayout *innerLayout = new QVBoxLayout(boundaryColorGroup);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);

    // Create the mode buttons that determine if the window is in single
    // or multiple color mode.
    colorModeButtons = new QButtonGroup(0, "colorModeButtons");
    connect(colorModeButtons, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    QGridLayout *colorLayout = new QGridLayout(innerLayout, 5, 3);
    colorLayout->setSpacing(10);
    colorLayout->setColStretch(2, 1000);
    QRadioButton *rb = new QRadioButton("Color table", boundaryColorGroup,
        "colorTable");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton("Single", boundaryColorGroup, "singleColor");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 2, 0);
    rb = new QRadioButton("Multiple", boundaryColorGroup, "multipleColor");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 3, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(boundaryColorGroup,
        "singleColorButton");
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 2, 1);

    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        boundaryColorGroup, "singleColorOpacity", NULL);
    singleColorOpacity->setTickInterval(64);
    singleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    colorLayout->addWidget(singleColorOpacity, 2, 2);

    // Try adding the multiple color button.
    multipleColor = new QvisColorButton(boundaryColorGroup,
        "multipleColor");
    connect(multipleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(multipleColorChanged(const QColor &)));
    colorLayout->addWidget(multipleColor, 3, 1);

    // Create the multiple color opacity.
    multipleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        boundaryColorGroup, "multipleColorOpacity", NULL);
    multipleColorOpacity->setTickInterval(64);
    multipleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(multipleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(multipleColorOpacityChanged(int)));
    colorLayout->addWidget(multipleColorOpacity, 3, 2);

    // Create the multiple color listbox.
    multipleColorList = new QListBox(boundaryColorGroup, "multipleColorList");
    multipleColorList->setMinimumHeight(100);
    multipleColorList->setSelectionMode(QListBox::Extended);
    connect(multipleColorList, SIGNAL(selectionChanged()),
            this, SLOT(boundarySelectionChanged()));
    colorLayout->addMultiCellWidget(multipleColorList, 4, 4, 1, 2);
    multipleColorLabel = new QLabel(multipleColorList, "Boundaries",
        boundaryColorGroup, "multipleColorLabel");
    colorLayout->addWidget(multipleColorLabel, 4, 0, Qt::AlignRight);

    // Create the color table widget
    colorTableButton = new QvisColorTableButton(boundaryColorGroup, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    colorLayout->addMultiCellWidget(colorTableButton, 1, 1, 1, 2, AlignLeft | AlignVCenter);

    // Create the overall opacity.
    QGridLayout *opLayout = new QGridLayout(topLayout, 7, 2);
    opLayout->setSpacing(5);
    overallOpacity = new QvisOpacitySlider(0, 255, 25, 255, central, 
                    "overallOpacity", NULL);
    overallOpacity->setTickInterval(64);
    overallOpacity->setGradientColor(QColor(0, 0, 0));
    connect(overallOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(overallOpacityChanged(int)));
    opLayout->addWidget(overallOpacity, 0, 1);

    QLabel *overallOpacityLabel = new QLabel(overallOpacity, "Opacity", 
                                      central, "overallOpacityLabel"); 
    overallOpacityLabel->setAlignment(AlignLeft | AlignVCenter);
    opLayout->addWidget(overallOpacityLabel, 0, 0);

    // Create the point control 
    pointControl = new QvisPointControl(central, "pointControl");
    connect(pointControl, SIGNAL(pointSizeChanged(double)),
            this, SLOT(pointSizeChanged(double)));
    connect(pointControl, SIGNAL(pointSizePixelsChanged(int)),
            this, SLOT(pointSizePixelsChanged(int)));
    connect(pointControl, SIGNAL(pointSizeVarChanged(const QString &)),
            this, SLOT(pointSizeVarChanged(const QString &)));
    connect(pointControl, SIGNAL(pointSizeVarToggled(bool)),
            this, SLOT(pointSizeVarToggled(bool)));
    connect(pointControl, SIGNAL(pointTypeChanged(int)),
            this, SLOT(pointTypeChanged(int)));
    opLayout->addMultiCellWidget(pointControl, 1, 1, 0, 1);

    // Create the legend toggle
    legendCheckBox = new QCheckBox("Legend", central, "legendToggle");
    connect(legendCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    opLayout->addWidget(legendCheckBox, 2, 0);

    // Create the wireframe toggle
    wireframeCheckBox = new QCheckBox("Wireframe", central, "wireframeCheckBox");
    connect(wireframeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(wireframeToggled(bool)));
    opLayout->addWidget(wireframeCheckBox, 3, 0);

    // Create the internal surfaces toggle
    drawInternalCheckBox = new QCheckBox("Draw internal surfaces", central, "drawInternalCheckBox");
    connect(drawInternalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawInternalToggled(bool)));
    opLayout->addMultiCellWidget(drawInternalCheckBox, 4,4, 0,1);

    // Create the clean zones only toggle
    cleanZonesOnlyCheckBox = new QCheckBox("Clean zones only", central, "cleanZonesOnlyCheckBox");
    connect(cleanZonesOnlyCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(cleanZonesOnlyToggled(bool)));
    opLayout->addWidget(cleanZonesOnlyCheckBox, 5, 0);

    // Create the mixed color button.
    QHBox *mixedColorBox = new QHBox(central);
    mixedColorLabel = new QLabel("--  mixed color: ", mixedColorBox);
    mixedColor = new QvisColorButton(mixedColorBox, "mixedColor");
    mixedColor->setButtonColor(QColor(255, 255, 255));
    connect(mixedColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(mixedColorChanged(const QColor &)));
    opLayout->addWidget(mixedColorBox, 5, 1);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(0, "smoothingButtons");
    connect(smoothingLevelButtons, SIGNAL(clicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
    QGridLayout *smoothingLayout = new QGridLayout(1, 5);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel("Geometry smoothing", central), 0,0);
    rb = new QRadioButton("None", central, "NoSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton("Fast", central, "LowSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton("High", central, "HighSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 3);
    opLayout->addMultiCellLayout(smoothingLayout, 6,6 , 0,1);
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::UpdateWindow
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Wed Apr 14 16:43:06 PDT 2004
//    Added mixed color.
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Added pointControl.
//
//    Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//    Fixed SGI compiler error with string conversion to QString
//
//    Brad Whitlock, Wed Jul 20 17:42:45 PST 2005
//    Added pointSizePixels.
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::UpdateWindow(bool doAll)
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
        case 0: // colorType
            if(boundaryAtts->GetColorType() == FilledBoundaryAttributes::ColorBySingleColor) 
                colorModeButtons->setButton(1);
            else if(boundaryAtts->GetColorType() == FilledBoundaryAttributes::ColorByMultipleColors) 
                colorModeButtons->setButton(2);
            else
                colorModeButtons->setButton(0);
            break;
        case 1: // colorTableName
            colorTableButton->setColorTable(boundaryAtts->GetColorTableName().c_str());
            break;
        case 2: // filledFlag
            // nothing anymore
            break;
        case 3: // legendFlag
            legendCheckBox->blockSignals(true);
            legendCheckBox->setChecked(boundaryAtts->GetLegendFlag());
            legendCheckBox->blockSignals(false);
            break;
        case 4: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(boundaryAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case 5: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(boundaryAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 6: // singleColor
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
        case 7: // multiColor
            updateMultiple = true;
            break;
        case 8: // boundaryNames
            updateMultiple = true;
            break;
        case 9: // needDomainLabels
            break;
        case 10: // opacity
            overallOpacity->blockSignals(true);
            overallOpacity->setValue((float)boundaryAtts->GetOpacity() * 255.f);
            overallOpacity->blockSignals(false);
            break;
        case 11: // wireframe
            wireframeCheckBox->blockSignals(true);
            wireframeCheckBox->setChecked(boundaryAtts->GetWireframe());
            wireframeCheckBox->blockSignals(false);
            break;
        case 12: // drawInternal
            drawInternalCheckBox->blockSignals(true);
            drawInternalCheckBox->setChecked(boundaryAtts->GetDrawInternal());
            drawInternalCheckBox->blockSignals(false);
            break;
        case 13: // smoothingLevel
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->setButton(boundaryAtts->GetSmoothingLevel());
            smoothingLevelButtons->blockSignals(false);
            break;
        case 14: // cleanZonesOnlyToggled
            cleanZonesOnlyCheckBox->blockSignals(true);
            cleanZonesOnlyCheckBox->setChecked(boundaryAtts->GetCleanZonesOnly());
            cleanZonesOnlyCheckBox->blockSignals(false);
            break;
        case 15: // mixedColor
            mixedColor->blockSignals(true);
            mixedColor->
                setButtonColor(QColor(boundaryAtts->GetMixedColor().Red(),
                                      boundaryAtts->GetMixedColor().Green(),
                                      boundaryAtts->GetMixedColor().Blue()));
            mixedColor->blockSignals(false);
            break;
        case 16: // pointSize
            pointControl->blockSignals(true);
            pointControl->SetPointSize(boundaryAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case 17: // pointType
            pointControl->blockSignals(true);
            pointControl->SetPointType(boundaryAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case 18: // pointSizeVarEnabled
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(boundaryAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case 19: // pointSizeVar
            pointControl->blockSignals(true);
            temp = QString(boundaryAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case 20: // pointSizePixels
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(boundaryAtts->GetPointSizePixels());
            pointControl->blockSignals(false);
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
        FilledBoundaryAttributes::ColorBySingleColor);
    singleColorOpacity->setEnabled(boundaryAtts->GetColorType() ==
        FilledBoundaryAttributes::ColorBySingleColor);
    bool mEnabled = (boundaryAtts->GetMultiColor().GetNumColorAttributes() > 0) &&
                    (boundaryAtts->GetColorType() == FilledBoundaryAttributes::ColorByMultipleColors);
    multipleColorLabel->setEnabled(mEnabled);
    multipleColorList->setEnabled(mEnabled);
    multipleColor->setEnabled(mEnabled);
    multipleColorOpacity->setEnabled(mEnabled);
    colorTableButton->setEnabled(boundaryAtts->GetColorType() ==
        FilledBoundaryAttributes::ColorByColorTable);
    mixedColor->setEnabled(boundaryAtts->GetCleanZonesOnly());
    mixedColorLabel->setEnabled(boundaryAtts->GetCleanZonesOnly());
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::UpdateMultipleArea
//
// Purpose: 
//   This method updates the multipleColors widget with the list of boundary
//   names.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::UpdateMultipleArea()
{
    const stringVector &matNames = boundaryAtts->GetBoundaryNames();
    intVector selectedBoundaries;
    bool update = true;
    int i;
    QListBoxTextWithColor *item;

    multipleColorList->blockSignals(true);

    //
    // Make a pass through the widget to see how many boundaries are selected.
    //
    for(i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->isSelected(i))
            selectedBoundaries.push_back(i);
    }

    //
    // Modify the names.
    //
    if(matNames.size() == multipleColorList->count())
    {
        // See if the strings are the same..
        bool same = true;
        for(i = 0; i < matNames.size() && same; ++i)
        {
            item = (QListBoxTextWithColor *)multipleColorList->item(i);
            ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
            same &= (item->text() == QString(matNames[i].c_str()) &&
                     item->color() == QColor(c.Red(), c.Green(), c.Blue()));
        }

        // If the strings are not the same then modifiy the widgets.
        if(!same)
        {
            int topItem = multipleColorList->topItem();
            for(i = 0; i < matNames.size(); ++i)
                UpdateItem(i);

            // Make sure the current item is still visible.
            if(topItem > 0)
                multipleColorList->setTopItem(topItem);
        }
        else
            update = false;
    }
    else if(matNames.size() > multipleColorList->count())
    {
        int topItem = multipleColorList->topItem();

        // Set all of the existing names.
        for(i = 0; i < multipleColorList->count(); ++i)
            UpdateItem(i);

        // Add new entries
        for(i = multipleColorList->count(); i < matNames.size(); ++i)
        {
            QString        itemText(matNames[i].c_str());
            ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
            QColor         itemColor(c.Red(), c.Green(), c.Blue());

            item = new QListBoxTextWithColor(multipleColorList, itemText,
                                             itemColor);
        }
        // Make sure the current item is still visible.
        if(topItem > 0)
            multipleColorList->setTopItem(topItem);
    }
    else // if(matNames.size() < multipleColorList->count())
    {
        int topItem = multipleColorList->topItem();

        // Set all of the existing names.
        for(i = 0; i < matNames.size(); ++i)
            UpdateItem(i);

        // Remove excess entries
        int numEntries = multipleColorList->count();
        for(i = matNames.size(); i < numEntries; ++i)
        {
            multipleColorList->removeItem(multipleColorList->count() - 1);
        }

        if(multipleColorList->count() > 0)
        {
            if(topItem > multipleColorList->count())
                topItem = 0;
            multipleColorList->setTopItem(topItem);
        }
    }

    //
    // Now that the names have been modified, do the selection.
    //
    if(update)
    {
        multipleColorList->clearSelection();
        bool noneSelected = true, first = true;
        for(i = 0; i < selectedBoundaries.size(); ++i)
        {
            if(selectedBoundaries[i] < multipleColorList->count())
            {
                multipleColorList->setSelected(selectedBoundaries[i], true);
                if(first)
                {
                    first = false;
                    multipleColorList->setCurrentItem(selectedBoundaries[i]);
                }
                noneSelected = false;
            }
        }

        // If there are no boundaries selected then select the first. If there 
        // is more than one boundary selected then update the listbox to cover
        // the case where we have to update the color for more than one
        // listboxitem.
        if(noneSelected)
        {
            multipleColorList->setSelected(0, true);
            multipleColorList->setCurrentItem(0);
        }
        else// if(selectedBoundaries.size() > 1) // remove check
        {
#define LISTBOX_UPDATE_KLUDGE
#ifdef LISTBOX_UPDATE_KLUDGE
            // Force the listbox to redraw itself and *all* of its listbox items.
            multipleColorList->resize(multipleColorList->width(),
                                      multipleColorList->height()+1);
            multipleColorList->resize(multipleColorList->width(),
                                      multipleColorList->height()-1);
#else
            multipleColorList->update();
#endif
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
        // Make sure that the selected index is in the range of visitble colors.
        if(selectedIndex >= matNames.size())
            selectedIndex = 0;

        SetMultipleColorWidgets(selectedIndex);
    }

    multipleColorList->blockSignals(false);
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::UpdateItem
//
// Purpose: 
//   Sets the i'th listbox item to the current colors for that item.
//
// Arguments:
//   i : The index of the item to be updated.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::UpdateItem(int i)
{
    QListBoxTextWithColor *item;
    item = (QListBoxTextWithColor *)multipleColorList->item(i);
    if(item)
    {
        item->setText(boundaryAtts->GetBoundaryNames()[i].c_str());
        ColorAttribute c(boundaryAtts->GetMultiColor()[i]);
        item->setColor(QColor(c.Red(), c.Green(), c.Blue()));
    }
}

// ****************************************************************************
// Method: QvisFilledBoundaryWindow::SetMultipleColorWidgets
//
// Purpose: 
//   Sets the colors for the multiple color widgets.
//
// Arguments:
//   index : The index into the boundary list.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::SetMultipleColorWidgets(int index)
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
// Method: QvisFilledBoundaryPlotWindow::Apply
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Uncommented GetCurrentValues. 
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current boundary plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        boundaryAtts->Notify();

        // Tell the viewer to set the boundary plot attributes.
        viewer->SetPlotOptions(plotType);
    }
    else
        boundaryAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Uncommented GetCurrentValues. 
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::makeDefault()
{
    // Tell the viewer to set the default boundary plot attributes.
    GetCurrentValues(-1);
    boundaryAtts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::reset()
{
    // Tell the viewer to reset the boundary plot attributes to the last
    // applied values.
    viewer->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::lineStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line style is changed.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::lineStyleChanged(int newStyle)
{
    boundaryAtts->SetLineStyle(newStyle);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::lineWidthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line width widget is changed.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::lineWidthChanged(int newWidth)
{
    boundaryAtts->SetLineWidth(newWidth);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   legend toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::legendToggled(bool val)
{
    boundaryAtts->SetLegendFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::wireframeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   wireframe toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::wireframeToggled(bool val)
{
    boundaryAtts->SetWireframe(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::cleanZonesOnlyToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   cleanZonesOnly toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2003
//
// Modifications:
//    Jeremy Meredith, Wed Apr 14 16:43:06 PDT 2004
//    Added mixed color.
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::cleanZonesOnlyToggled(bool val)
{
    boundaryAtts->SetCleanZonesOnly(val);
    mixedColor->setEnabled(val);
    mixedColorLabel->setEnabled(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::drawInternalToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   draw internal surfaces toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::drawInternalToggled(bool val)
{
    boundaryAtts->SetDrawInternal(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::colorModeChanged
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::colorModeChanged(int index)
{
    if(index == 0)
        boundaryAtts->SetColorType(FilledBoundaryAttributes::ColorByColorTable);
    else if(index == 1)
        boundaryAtts->SetColorType(FilledBoundaryAttributes::ColorBySingleColor);
    else
        boundaryAtts->SetColorType(FilledBoundaryAttributes::ColorByMultipleColors);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::singleColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single color button's
//   color changes.
//
// Arguments:
//   color : The new single color.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue(),
                        boundaryAtts->GetSingleColor().Alpha());
    boundaryAtts->SetSingleColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::singleColorOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single color opacity
//   changes.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::singleColorOpacityChanged(int opacity)
{
    ColorAttribute temp(boundaryAtts->GetSingleColor());
    temp.SetAlpha(opacity);
    boundaryAtts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::multipleColorChanged
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::multipleColorChanged(const QColor &color)
{
    // If any boundaries are selected, change their colors.
    if(multipleColorList->currentItem() != -1)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->isSelected(i) &&
               (i < boundaryAtts->GetMultiColor().GetNumColorAttributes()))
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
// Method: QvisFilledBoundaryPlotWindow::opacityChanged
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::multipleColorOpacityChanged(int opacity)
{
    // If any boundaries are selected, change their opacities.
    if(multipleColorList->currentItem() != -1)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->isSelected(i) &&
               (i < boundaryAtts->GetMultiColor().GetNumColorAttributes()))
            {
                boundaryAtts->GetMultiColor()[i].SetAlpha(opacity);
            }
        }

        boundaryAtts->SelectMultiColor();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotwindow::boundarySelectionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the boundary selection
//   changes.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::boundarySelectionChanged()
{
    // Go through until we find the first selected item.
    int index = -1;
    for(int i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->isSelected(i))
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
// Method: QvisFilledBoundaryPlotWindow::overallOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the overall opacity slider
//   is clicked.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::overallOpacityChanged(int opacity)
{
    boundaryAtts->SetOpacity(double(opacity) / 255.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisFilledBoundaryPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer: Jeremy Meredith
//  Creation:    May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::smoothingLevelChanged(int level)
{
    boundaryAtts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::colorTableClicked
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
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::colorTableClicked(bool useDefault, const QString &ctName)
{
    boundaryAtts->SetColorTableName(ctName.latin1());
    Apply();
}


// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::mixedColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the mixed color button's
//   color changes.
//
// Arguments:
//   color : The new single color.
//
// Programmer: Jeremy Meredith
// Creation:   April 13, 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::mixedColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue(),
                        boundaryAtts->GetMixedColor().Alpha());
    boundaryAtts->SetMixedColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 17:44:59 PST 2005
//   Added SetPointSizePixels.
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the point size and pointSizeVar
    if(doAll)
    {
        boundaryAtts->SetPointSize(pointControl->GetPointSize());
        boundaryAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        boundaryAtts->SetPointSizeVar(pointControl->GetPointSizeVar().latin1());
    }
}


// ****************************************************************************
//  Method:  QvisFilledBoundaryPlotWindow::pointTypeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the point type buttons
//    is clicked.
//
//  Arguments:
//    type   :   The new type
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::pointTypeChanged(int type)
{
    boundaryAtts->SetPointType((FilledBoundaryAttributes::PointType) type);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::pointSizeVarToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the pointSizeVar toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the pointSizeVar toggle.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::pointSizeVarToggled(bool val)
{
    boundaryAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::pointSizeVarChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and presses the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::pointSizeVarChanged(const QString &var)
{
    boundaryAtts->SetPointSizeVar(var.latin1());
    Apply();
}


// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::pointSizeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::pointSizeChanged(double d)
{
    boundaryAtts->SetPointSize(d);
    Apply();
}

// ****************************************************************************
// Method: QvisFilledBoundaryPlotWindow::pointSizePixelsChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:25:58 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilledBoundaryPlotWindow::pointSizePixelsChanged(int size)
{
    boundaryAtts->SetPointSizePixels(size); 
    Apply();
}

