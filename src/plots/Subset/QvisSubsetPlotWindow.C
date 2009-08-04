/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QvisSubsetPlotWindow.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QItemDelegate>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>

#include <QvisColorButton.h>
#include <QvisColorSwatchListWidget.h>
#include <QvisColorTableButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <QvisPointControl.h>
#include <SubsetAttributes.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisSubsetPlotWindow::QvisSubsetPlotWindow
//
// Purpose: 
//   Constructor for the QvisSubsetPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//    Jeremy Meredith, Fri Jan  2 17:26:04 EST 2009
//    Added Load/Save buttons.  (Other plot windows get this by default.)
//   
// ****************************************************************************

QvisSubsetPlotWindow::QvisSubsetPlotWindow(const int type,
    SubsetAttributes *subsetAtts_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subsetAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtonsAndLoadSave,
                               false)
{
    plotType     = type;
    subsetAtts = subsetAtts_;

    // Initialize widgets that we'll have to delete manually. These are
    // parentless widgets.
    colorModeButtons = 0;
    smoothingLevelButtons = 0;
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::~QvisSubsetPlotWindow
//
// Purpose: 
//   Destructor for the QvisSubsetPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//
// ****************************************************************************

QvisSubsetPlotWindow::~QvisSubsetPlotWindow()
{
    subsetAtts = 0;

}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::CreateWindowContents
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
// ****************************************************************************

void
QvisSubsetPlotWindow::CreateWindowContents()
{
    QHBoxLayout *lineLayout = new QHBoxLayout(0);
    lineLayout->setMargin(0);
    topLayout->addLayout(lineLayout);

    // Create the lineSyle widget.
    lineStyle = new QvisLineStyleWidget(0, central);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(tr("Line style"), central);
    lineStyleLabel->setBuddy(lineStyle);
    lineLayout->addWidget(lineStyleLabel);
    lineLayout->addWidget(lineStyle);

    // Create the lineSyle widget.
    lineWidth = new QvisLineWidthWidget(0, central);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(tr("Line width"), central);
    lineWidthLabel->setBuddy(lineWidth);
    lineLayout->addWidget(lineWidthLabel);
    lineLayout->addWidget(lineWidth);

    // Create the boundary color group box.
    subsetColorGroup = new QGroupBox(central);
    subsetColorGroup->setTitle(tr("Subset colors"));
    topLayout->addWidget(subsetColorGroup);

    // Create the mode buttons that determine if the window is in single
    // or multiple color mode.
    colorModeButtons = new QButtonGroup(subsetColorGroup);
    connect(colorModeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
    QGridLayout *colorLayout = new QGridLayout(subsetColorGroup);
    colorLayout->setSpacing(10);
    colorLayout->setMargin(5);
    colorLayout->setColumnStretch(2, 1000);
    QRadioButton *rb = new QRadioButton(tr("Color table"), subsetColorGroup);
    colorModeButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton(tr("Single"), subsetColorGroup);
    colorModeButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 2, 0);
    rb = new QRadioButton(tr("Multiple"), subsetColorGroup);
    colorModeButtons->addButton(rb, 2);
    colorLayout->addWidget(rb, 3, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(subsetColorGroup);
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 2, 1);

    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        subsetColorGroup, NULL);
    singleColorOpacity->setTickInterval(64);
    singleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    colorLayout->addWidget(singleColorOpacity, 2, 2);

    // Try adding the multiple color button.
    multipleColor = new QvisColorButton(subsetColorGroup);
    connect(multipleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(multipleColorChanged(const QColor &)));
    colorLayout->addWidget(multipleColor, 3, 1);

    // Create the multiple color opacity.
    multipleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        subsetColorGroup, NULL);
    multipleColorOpacity->setTickInterval(64);
    multipleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(multipleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(multipleColorOpacityChanged(int)));
    colorLayout->addWidget(multipleColorOpacity, 3, 2);

    // Create the multiple color list widget.
    multipleColorList = new QvisColorSwatchListWidget(subsetColorGroup);
    multipleColorList->setMinimumHeight(100);
    connect(multipleColorList, SIGNAL(itemSelectionChanged()),
            this, SLOT(subsetSelectionChanged()));
    colorLayout->addWidget(multipleColorList, 4, 1, 1, 2);
    multipleColorLabel = new QLabel(tr("Subsets"), subsetColorGroup);
    multipleColorLabel->setBuddy(multipleColorList);
    colorLayout->addWidget(multipleColorLabel, 4, 0, Qt::AlignRight);

    // Create the color table widget
    colorTableButton = new QvisColorTableButton(subsetColorGroup);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    colorLayout->addWidget(colorTableButton, 1, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

    // Create the overall opacity.
    QHBoxLayout *opLayout = new QHBoxLayout(0);
    opLayout->setMargin(0);
    opLayout->setSpacing(5);
    topLayout->addLayout(opLayout);
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

    // Create the point control 
    pointControl = new QvisPointControl(central);
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
    topLayout->addWidget(pointControl);
 
    // Create the legend toggle
    legendCheckBox = new QCheckBox(tr("Legend"), central);
    connect(legendCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    topLayout->addWidget(legendCheckBox);

    // Create the wireframe toggle
    wireframeCheckBox = new QCheckBox(tr("Wireframe"), central);
    connect(wireframeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(wireframeToggled(bool)));
    topLayout->addWidget(wireframeCheckBox);

    // Create the internal surfaces toggle
    drawInternalCheckBox = new QCheckBox(tr("Draw internal surfaces"), central);
    connect(drawInternalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawInternalToggled(bool)));
    topLayout->addWidget(drawInternalCheckBox);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
    QGridLayout *smoothingLayout = new QGridLayout(0);
    smoothingLayout->setMargin(0);
    topLayout->addLayout(smoothingLayout);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColumnStretch(4, 100);
    smoothingLayout->addWidget(new QLabel(tr("Geometry smoothing"), central), 0, 0);
    rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb, 0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb, 1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb, 2);
    smoothingLayout->addWidget(rb, 0, 3);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::UpdateWindow
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
// ****************************************************************************

void
QvisSubsetPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    bool updateMultiple = false;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < subsetAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!subsetAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case SubsetAttributes::ID_colorType:
            if(subsetAtts->GetColorType() == SubsetAttributes::ColorBySingleColor) 
                colorModeButtons->button(1)->setChecked(true);
            else if(subsetAtts->GetColorType() == SubsetAttributes::ColorByMultipleColors) 
                colorModeButtons->button(2)->setChecked(true);
            else
                colorModeButtons->button(0)->setChecked(true);
            break;
        case SubsetAttributes::ID_colorTableName:
            colorTableButton->setColorTable(subsetAtts->GetColorTableName().c_str());
            break;
        case SubsetAttributes::ID_filledFlag:
            // nothing anymore
            break;
        case SubsetAttributes::ID_legendFlag:
            legendCheckBox->blockSignals(true);
            legendCheckBox->setChecked(subsetAtts->GetLegendFlag());
            legendCheckBox->blockSignals(false);
            break;
        case SubsetAttributes::ID_lineStyle:
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(subsetAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case SubsetAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(subsetAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case SubsetAttributes::ID_singleColor:
            { // new scope
            QColor temp(subsetAtts->GetSingleColor().Red(),
                        subsetAtts->GetSingleColor().Green(),
                        subsetAtts->GetSingleColor().Blue());
            singleColor->blockSignals(true);
            singleColor->setButtonColor(temp);
            singleColor->blockSignals(false);

            singleColorOpacity->setGradientColor(temp);
            singleColorOpacity->blockSignals(true);
            singleColorOpacity->setValue(subsetAtts->GetSingleColor().Alpha());
            singleColorOpacity->blockSignals(false);
            }
            break;
        case SubsetAttributes::ID_multiColor:
            updateMultiple = true;
            break;
        case SubsetAttributes::ID_subsetNames:
            updateMultiple = true;
            break;
        case SubsetAttributes::ID_subsetType:
            break;
        case SubsetAttributes::ID_opacity:
            overallOpacity->blockSignals(true);
            overallOpacity->setValue((int)(subsetAtts->GetOpacity() * 255.f));
            overallOpacity->blockSignals(false);
            break;
        case SubsetAttributes::ID_wireframe:
            wireframeCheckBox->blockSignals(true);
            wireframeCheckBox->setChecked(subsetAtts->GetWireframe());
            wireframeCheckBox->blockSignals(false);
            break;
        case SubsetAttributes::ID_drawInternal:
            drawInternalCheckBox->blockSignals(true);
            drawInternalCheckBox->setChecked(subsetAtts->GetDrawInternal());
            drawInternalCheckBox->blockSignals(false);
            break;
        case SubsetAttributes::ID_smoothingLevel:
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->button(subsetAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelButtons->blockSignals(false);
            break;
        case SubsetAttributes::ID_pointSize:
            pointControl->blockSignals(true);
            pointControl->SetPointSize(subsetAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case SubsetAttributes::ID_pointType:
            pointControl->blockSignals(true);
            pointControl->SetPointType(subsetAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case SubsetAttributes::ID_pointSizeVarEnabled:
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(
                          subsetAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case SubsetAttributes::ID_pointSizeVar:
            pointControl->blockSignals(true);
            temp = QString(subsetAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case SubsetAttributes::ID_pointSizePixels:
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(subsetAtts->GetPointSizePixels());
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
    singleColor->setEnabled(subsetAtts->GetColorType() ==
        SubsetAttributes::ColorBySingleColor);
    singleColorOpacity->setEnabled(subsetAtts->GetColorType() ==
        SubsetAttributes::ColorBySingleColor);
    bool mEnabled = (subsetAtts->GetMultiColor().GetNumColors() > 0) &&
                    (subsetAtts->GetColorType() == SubsetAttributes::ColorByMultipleColors);
    multipleColorLabel->setEnabled(mEnabled);
    multipleColorList->setEnabled(mEnabled);
    multipleColor->setEnabled(mEnabled);
    multipleColorOpacity->setEnabled(mEnabled);
    colorTableButton->setEnabled(subsetAtts->GetColorType() ==
        SubsetAttributes::ColorByColorTable);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::UpdateMultipleArea
//
// Purpose: 
//   This method updates the multipleColors widget with the list of boundary
//   names.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 12:12:45 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::UpdateMultipleArea()
{
    const stringVector &matNames = subsetAtts->GetSubsetNames();
    intVector selectedsubsets;
    bool update = true;
    int i;
    QListWidgetItem *item;

    multipleColorList->blockSignals(true);

    //
    // Make a pass through the widget to see how many subsets are selected.
    //
    for(i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->item(i)->isSelected())
            selectedsubsets.push_back(i);
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
            ColorAttribute c(subsetAtts->GetMultiColor()[i]);
            
            same &= CompareItem(i, QString(matNames[i].c_str()),
                                   QColor(c.Red(), c.Green(), c.Blue()));
        }

        // If the strings are not the same then modifiy the widgets.
        if(!same)
        {
            for(i = 0; i < matNames.size(); ++i)
                UpdateItem(i);
        }
        else
            update = false;
    }
    else if(matNames.size() > multipleColorList->count())
    {
        // Set all of the existing names.
        for(i = 0; i < multipleColorList->count(); ++i)
            UpdateItem(i);

        // Add new entries
        for(i = multipleColorList->count(); i < matNames.size(); ++i)
        {
            QString        itemText(matNames[i].c_str());
            ColorAttribute c(subsetAtts->GetMultiColor()[i]);
            QColor         itemColor(c.Red(), c.Green(), c.Blue());
            multipleColorList->addItem(matNames[i].c_str(), itemColor);
        }
    }
    else // if(matNames.size() < multipleColorList->count())
    {
        // Set all of the existing names.
        for(i = 0; i < matNames.size(); ++i)
            UpdateItem(i);

        // Remove excess entries
        int numEntries = multipleColorList->count();
        for(i = matNames.size(); i < numEntries; ++i)
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
        for(i = 0; i < selectedsubsets.size(); ++i)
        {
            if(selectedsubsets[i] < multipleColorList->count())
            {
                item = multipleColorList->item(selectedsubsets[i]);
                item->setSelected(true);
                if(first)
                {
                    first = false;
                    multipleColorList->setCurrentItem(item);
                }
                noneSelected = false;
            }
        }

        // If there are no subsets selected then select the first. If there 
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
        int selectedIndex = (selectedsubsets.size() > 0) ?
            selectedsubsets[0] : 0;
        // Make sure that the selected index is in the range of visible colors.
        if(selectedIndex >= matNames.size())
            selectedIndex = 0;

        SetMultipleColorWidgets(selectedIndex);
    }

    multipleColorList->blockSignals(false);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::CompareItem
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
QvisSubsetPlotWindow::CompareItem(int i, const QString &name, 
    const QColor &c) const
{
    QString itemName(multipleColorList->text(i));
    QColor  itemColor(multipleColorList->color(i));
    return  itemName == name && itemColor == c;
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::UpdateItem
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 12:10:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::UpdateItem(int i)
{
    multipleColorList->setText(i, subsetAtts->GetSubsetNames()[i].c_str());
    ColorAttribute c(subsetAtts->GetMultiColor()[i]);
    multipleColorList->setColor(i, QColor(c.Red(), c.Green(), c.Blue()));
}

// ****************************************************************************
// Method: QvisSubsetWindow::SetMultipleColorWidgets
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::SetMultipleColorWidgets(int index)
{
    ColorAttribute c(subsetAtts->GetMultiColor()[index]);
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
// Method: QvisSubsetPlotWindow::Apply
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Uncommented GetCurrentValues.
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current boundary plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        subsetAtts->Notify();

        // Tell the viewer to set the boundary plot attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        subsetAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisSubsetPlotWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Uncommented GetCurrentValues.
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::makeDefault()
{
    // Tell the viewer to set the default boundary plot attributes.
    GetCurrentValues(-1);
    subsetAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::reset()
{
    // Tell the viewer to reset the boundary plot attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::lineStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line style is changed.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::lineStyleChanged(int newStyle)
{
    subsetAtts->SetLineStyle(newStyle);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::lineWidthChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::lineWidthChanged(int newWidth)
{
    subsetAtts->SetLineWidth(newWidth);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::legendToggled
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::legendToggled(bool val)
{
    subsetAtts->SetLegendFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::wireframeToggled
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::wireframeToggled(bool val)
{
    subsetAtts->SetWireframe(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::colorModeChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//
// ****************************************************************************

void
QvisSubsetPlotWindow::colorModeChanged(int index)
{
    if(index == 0)
        subsetAtts->SetColorType(SubsetAttributes::ColorByColorTable);
    else if(index == 1)
        subsetAtts->SetColorType(SubsetAttributes::ColorBySingleColor);
    else
        subsetAtts->SetColorType(SubsetAttributes::ColorByMultipleColors);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::singleColorChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue(),
                        subsetAtts->GetSingleColor().Alpha());
    subsetAtts->SetSingleColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::singleColorOpacityChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::singleColorOpacityChanged(int opacity)
{
    ColorAttribute temp(subsetAtts->GetSingleColor());
    temp.SetAlpha(opacity);
    subsetAtts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::multipleColorChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:50:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::multipleColorChanged(const QColor &color)
{
    // If any subsets are selected, change their colors.
    if(multipleColorList->currentItem() != 0)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->item(i)->isSelected() &&
               (i < subsetAtts->GetMultiColor().GetNumColors()))
            {
                subsetAtts->GetMultiColor()[i].SetRgb(color.red(),
                                                      color.green(),
                                                      color.blue());
            }
        }

        subsetAtts->SelectMultiColor();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::opacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the opacity changes for one
//   of the subsets in the multiple colors area.
//
// Arguments:
//   opacity : The new opacity.
//   index   : The index of the boundary that changed.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:51:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::multipleColorOpacityChanged(int opacity)
{
    // If any subsets are selected, change their opacities.
    if(multipleColorList->currentItem() != 0)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->item(i)->isSelected() &&
               (i < subsetAtts->GetMultiColor().GetNumColors()))
            {
                subsetAtts->GetMultiColor()[i].SetAlpha(opacity);
            }
        }

        subsetAtts->SelectMultiColor();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisSubsetPlotwindow::subsetSelectionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the boundary selection
//   changes.
//
// Programmer: Jeremy Meredith
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 13:52:02 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::subsetSelectionChanged()
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
// Method: QvisSubsetPlotWindow::overallOpacityChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::overallOpacityChanged(int opacity)
{
    subsetAtts->SetOpacity(double(opacity) / 255.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisSubsetPlotWindow::smoothingLevelChanged
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
//  Note:  taken almost verbatim from the Boundary plot
//
//  Modifications:
//
// ****************************************************************************

void
QvisSubsetPlotWindow::smoothingLevelChanged(int level)
{
    subsetAtts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::colorTableClicked
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::colorTableClicked(bool useDefault, const QString &ctName)
{
    subsetAtts->SetColorTableName(ctName.toStdString());
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::GetCurrentValues
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
//   Brad Whitlock, Wed Jul 20 18:00:29 PST 2005
//   Added SetPointSizePixels.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the point size and pointsize var
    if(doAll)
    {
        subsetAtts->SetPointSize(pointControl->GetPointSize());
        subsetAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        subsetAtts->SetPointSizeVar(pointControl->GetPointSizeVar().toStdString());
    }
}


// ****************************************************************************
//  Method:  QvisSubsetPlotWindow::pointTypeChanged
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
QvisSubsetPlotWindow::pointTypeChanged(int type)
{
    subsetAtts->SetPointType((SubsetAttributes::PointType) type);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::pointSizeVarToggled
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
QvisSubsetPlotWindow::pointSizeVarToggled(bool val)
{
    subsetAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::processPointSizeVarText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and pressed the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::pointSizeVarChanged(const QString &var)
{
    subsetAtts->SetPointSizeVar(var.toStdString()); 
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::pointSizeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and pressed the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 10, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::pointSizeChanged(double d)
{
    subsetAtts->SetPointSize(d); 
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::pointSizePixelsChanged
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
QvisSubsetPlotWindow::pointSizePixelsChanged(int size)
{
    subsetAtts->SetPointSizePixels(size); 
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::drawInternalToggled
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
//  Note:  taken almost verbatim from the Boundary plot
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::drawInternalToggled(bool val)
{
    subsetAtts->SetDrawInternal(val);
    Apply();
}
