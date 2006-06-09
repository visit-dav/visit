/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <QvisSubsetPlotWindow.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
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
#include <SubsetAttributes.h>
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
// Method: QvisSubsetPlotWindow::QvisSubsetPlotWindow
//
// Purpose: 
//   Constructor for the QvisSubsetPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:49:54 PDT 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 14:14:38 PST 2001
//    I added the argument type.
//   
// ****************************************************************************

QvisSubsetPlotWindow::QvisSubsetPlotWindow(const int type,
    SubsetAttributes *subsetAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subsetAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons,
                               false)
{
    plotType     = type;
    subsetAtts = subsetAtts_;

    // Initialize widgets that we'll have to delete manually. These are
    // parentless widgets.
    colorModeButtons = 0;
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::~QvisSubsetPlotWindow
//
// Purpose: 
//   Destructor for the QvisSubsetPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:01 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 1 15:14:35 PST 2002
//   Added code to delete widgets without parents.
//
// ****************************************************************************

QvisSubsetPlotWindow::~QvisSubsetPlotWindow()
{
    subsetAtts = 0;

    // no parents, delete them manually.
    delete colorModeButtons;
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:30 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 8 12:13:11 PDT 2000
//   Added code for color selection.
//
//   Jeremy Meredith, Tue Mar 12 17:15:49 PST 2002
//   Added a toggle for wireframe mode.
//
//   Jeremy Meredith, March 14, 2002
//   Added a toggle to draw internal surfaces.
//
//   Jeremy Meredith, Fri Mar 15 13:00:20 PST 2002
//   Fixed checkboxes and opacity slider at bottom of window.
//
//   Brad Whitlock, Tue Dec 3 12:43:52 PDT 2002
//   I added a color table button.
//
//   Jeremy Meredith, Tue Dec 10 10:25:41 PST 2002
//   Added smoothing options.
//
//   Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004 
//   Added pointControl. 
//
//   Brad Whitlock, Wed Jul 20 14:27:00 PST 2005
//   Added pointSizePixelsChanged slot.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::CreateWindowContents()
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

    // Create the subset color group box.
    subsetColorGroup = new QGroupBox(central, "subsetColorGroup");
    subsetColorGroup->setTitle("Subset colors");
    topLayout->addWidget(subsetColorGroup);
    QVBoxLayout *innerLayout = new QVBoxLayout(subsetColorGroup);
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
    QRadioButton *rb = new QRadioButton("Color table", subsetColorGroup,
        "colorTable");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton("Single", subsetColorGroup, "singleColor");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 2, 0);
    rb = new QRadioButton("Multiple", subsetColorGroup, "multipleColor");
    colorModeButtons->insert(rb);
    colorLayout->addWidget(rb, 3, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(subsetColorGroup,
        "singleColorButton");
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 2, 1);

    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        subsetColorGroup, "singleColorOpacity", NULL);
    singleColorOpacity->setTickInterval(64);
    singleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    colorLayout->addWidget(singleColorOpacity, 2, 2);

    // Try adding the multiple color button.
    multipleColor = new QvisColorButton(subsetColorGroup,
        "multipleColor");
    connect(multipleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(multipleColorChanged(const QColor &)));
    colorLayout->addWidget(multipleColor, 3, 1);

    // Create the multiple color opacity.
    multipleColorOpacity = new QvisOpacitySlider(0, 255, 25, 255,
        subsetColorGroup, "multipleColorOpacity", NULL);
    multipleColorOpacity->setTickInterval(64);
    multipleColorOpacity->setGradientColor(QColor(0, 0, 0));
    connect(multipleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(multipleColorOpacityChanged(int)));
    colorLayout->addWidget(multipleColorOpacity, 3, 2);

    // Create the multiple color listbox.
    multipleColorList = new QListBox(subsetColorGroup, "multipleColorList");
    multipleColorList->setMinimumHeight(100);
    multipleColorList->setSelectionMode(QListBox::Extended);
    connect(multipleColorList, SIGNAL(selectionChanged()),
            this, SLOT(subsetSelectionChanged()));
    colorLayout->addMultiCellWidget(multipleColorList, 4, 4, 1, 2);
    multipleColorLabel = new QLabel(multipleColorList, "Subsets",
        subsetColorGroup, "multipleColorLabel");
    colorLayout->addWidget(multipleColorLabel, 4, 0, Qt::AlignRight);

    // Create the color table widget
    colorTableButton = new QvisColorTableButton(subsetColorGroup, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    colorLayout->addMultiCellWidget(colorTableButton, 1, 1, 1, 2, AlignLeft | AlignVCenter);

    // Create the overall opacity.
    QGridLayout *opLayout = new QGridLayout(topLayout, 6, 2);
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
    opLayout->addMultiCellLayout(smoothingLayout, 5,5 , 0,1);

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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:53:23 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 8 12:12:55 PDT 2000
//   Added code for color selection.
//
//   Kathleen Bonnell, Mon Mar 26 18:17:53 PST 2001 
//   Disabled lineStyle and lineStyleLabel until we have vtk version
//   in which line stippling is available. 
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enabled lineStyle and lineStyleLabel. 
//
//   Brad Whitlock, Thu Jan 31 12:16:10 PDT 2002
//   Modified to support multiple colors in a different way.
//
//   Jeremy Meredith, Tue Mar 12 17:15:49 PST 2002
//   Added a toggle for wireframe mode.
//
//   Jeremy Meredith, Thu Mar 14 17:51:16 PST 2002
//   Added a toggle for drawing internal surfaces.
//
//   Brad Whitlock, Mon Nov 25 17:13:27 PST 2002
//   Updated code to fit new version of the attributes.
//
//   Jeremy Meredith, Tue Dec 10 10:25:51 PST 2002
//   Added smoothing options.
//
//   Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004 
//   Added point options.
//
//   Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//   Fixed SGI compiler error with string conversion to QString
//
//   Brad Whitlock, Wed Jul 20 18:10:08 PST 2005
//   Added pointSizePixels.
//
//   Hank Childs, Thu Jun  8 13:41:24 PDT 2006
//   Fix compiler warning for casting.
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
        case 0: // colorType
            if(subsetAtts->GetColorType() == SubsetAttributes::ColorBySingleColor) 
                colorModeButtons->setButton(1);
            else if(subsetAtts->GetColorType() == SubsetAttributes::ColorByMultipleColors) 
                colorModeButtons->setButton(2);
            else
                colorModeButtons->setButton(0);
            break;
        case 1: // colorTableName
            colorTableButton->setColorTable(subsetAtts->GetColorTableName().c_str());
            break;
        case 2: // filledFlag
            // nothing anymore
            break;
        case 3: // legendFlag
            legendCheckBox->blockSignals(true);
            legendCheckBox->setChecked(subsetAtts->GetLegendFlag());
            legendCheckBox->blockSignals(false);
            break;
        case 4: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(subsetAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case 5: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(subsetAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 6: // singleColor
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
        case 7: // multiColor
            updateMultiple = true;
            break;
        case 8: // subsetNames
            updateMultiple = true;
            break;
        case 9: // needDomainLabels
            break;
        case 10: // opacity
            overallOpacity->blockSignals(true);
            overallOpacity->setValue((int)(subsetAtts->GetOpacity() * 255.f));
            overallOpacity->blockSignals(false);
            break;
        case 11: // wireframe
            wireframeCheckBox->blockSignals(true);
            wireframeCheckBox->setChecked(subsetAtts->GetWireframe());
            wireframeCheckBox->blockSignals(false);
            break;
        case 12: // drawInternal
            drawInternalCheckBox->blockSignals(true);
            drawInternalCheckBox->setChecked(subsetAtts->GetDrawInternal());
            drawInternalCheckBox->blockSignals(false);
            break;
        case 13: // smoothingLevel
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->setButton(subsetAtts->GetSmoothingLevel());
            smoothingLevelButtons->blockSignals(false);
            break;
        case 14: // pointSize
            pointControl->blockSignals(true);
            pointControl->SetPointSize(subsetAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case 15: // pointType
            pointControl->blockSignals(true);
            pointControl->SetPointType(subsetAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case 16: // pointSizeVarEnabled
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(subsetAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case 17: // pointSizeVar
            pointControl->blockSignals(true);
            temp = QString(subsetAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case 18: // pointSizePixels
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
    bool mEnabled = (subsetAtts->GetMultiColor().GetNumColorAttributes() > 0) &&
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
//   This method updates the multipleColors widget with the list of subset
//   names.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 12:19:06 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Jan 31 12:17:26 PDT 2002
//   Rewrote it so it supports setting both the names and the colors.
//
//   Brad Whitlock, Thu Feb 6 14:02:19 PST 2003
//   I modified the code so it forces all listview items to update when
//   there are the same number of colors but the colors are different (as in
//   keyframing).
//
// ****************************************************************************

void
QvisSubsetPlotWindow::UpdateMultipleArea()
{
    const stringVector &matNames = subsetAtts->GetSubsetNames();
    intVector selectedSubsets;
    bool update = true;
    int i;
    QListBoxTextWithColor *item;

    multipleColorList->blockSignals(true);

    //
    // Make a pass through the widget to see how many subsets are selected.
    //
    for(i = 0; i < multipleColorList->count(); ++i)
    {
        if(multipleColorList->isSelected(i))
            selectedSubsets.push_back(i);
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
            ColorAttribute c(subsetAtts->GetMultiColor()[i]);
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
            ColorAttribute c(subsetAtts->GetMultiColor()[i]);
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
        for(i = 0; i < selectedSubsets.size(); ++i)
        {
            if(selectedSubsets[i] < multipleColorList->count())
            {
                multipleColorList->setSelected(selectedSubsets[i], true);
                if(first)
                {
                    first = false;
                    multipleColorList->setCurrentItem(selectedSubsets[i]);
                }
                noneSelected = false;
            }
        }

        // If there are no subsets selected then select the first. If there 
        // is more than one subset selected then update the listbox to cover
        // the case where we have to update the color for more than one
        // listboxitem.
        if(noneSelected)
        {
            multipleColorList->setSelected(0, true);
            multipleColorList->setCurrentItem(0);
        }
        else// if(selectedSubsets.size() > 1) // remove check
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
        int selectedIndex = (selectedSubsets.size() > 0) ?
            selectedSubsets[0] : 0;
        // Make sure that the selected index is in the range of visitble colors.
        if(selectedIndex >= matNames.size())
            selectedIndex = 0;

        SetMultipleColorWidgets(selectedIndex);
    }

    multipleColorList->blockSignals(false);
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
// Programmer: Brad Whitlock
// Creation:   Fri Feb 1 12:25:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::UpdateItem(int i)
{
    QListBoxTextWithColor *item;
    item = (QListBoxTextWithColor *)multipleColorList->item(i);
    if(item)
    {
        item->setText(subsetAtts->GetSubsetNames()[i].c_str());
        ColorAttribute c(subsetAtts->GetMultiColor()[i]);
        item->setColor(QColor(c.Red(), c.Green(), c.Blue()));
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::SetMultipleColorWidgets
//
// Purpose: 
//   Sets the colors for the multiple color widgets.
//
// Arguments:
//   index : The index into the subset list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 12:30:58 PDT 2002
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
//   This method applies the subset plot attributes and optionally
//   tells the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            subset plot attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:48:08 PDT 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 14:14:38 PST 2001
//    I modified the routine to pass to the viewer proxy the plot
//    type stored within the class instead of the one hardwired from
//    an include file.
//   
//   Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004 
//   Uncommented GetCurrentValues. 
//
// ****************************************************************************

void
QvisSubsetPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current subset plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        subsetAtts->Notify();

        // Tell the viewer to set the subset plot attributes.
        viewer->SetPlotOptions(plotType);
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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:54:23 PDT 2000
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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:54:54 PDT 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 14:14:38 PST 2001
//    I modified the routine to pass to the viewer proxy the plot
//    type stored within the class instead of the one hardwired from
//    an include file.
//   
//   Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004 
//   Uncommented GetCurrentValues. 
//
// ****************************************************************************

void
QvisSubsetPlotWindow::makeDefault()
{
    // Tell the viewer to set the default subset plot attributes.
    GetCurrentValues(-1);
    subsetAtts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisSubsetPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:55:52 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::reset()
{
    // Tell the viewer to reset the subset plot attributes to the last
    // applied values.
    viewer->ResetPlotOptions(plotType);
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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:55:52 PDT 2000
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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:55:52 PDT 2000
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
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 08:55:52 PDT 2000
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
// Creation:   March 12, 2002
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
// Creation:   March 14, 2002
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
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 11:59:55 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 25 17:55:07 PST 2002
//   I changed the code to work with the new version of the attributes.
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
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 14:06:56 PST 2000
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
// Programmer: Brad Whitlock
// Creation:   Wed Jan 30 11:36:44 PDT 2002
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
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 14:02:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 18:56:15 PST 2001
//   Changed a method call to conform to a new interface.
//
//   Brad Whitlock, Wed Jan 30 11:38:42 PDT 2002
//   Rewrote it to work with the new window.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::multipleColorChanged(const QColor &color)
{
    // If any subsets are selected, change their colors.
    if(multipleColorList->currentItem() != -1)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->isSelected(i) &&
               (i < subsetAtts->GetMultiColor().GetNumColorAttributes()))
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
//   index   : The index of the subset that changed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 14:07:46 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 18:56:15 PST 2001
//   Changed a method call to conform to a new interface.
//
//   Brad Whitlock, Wed Jan 30 11:38:42 PDT 2002
//   Rewrote it to work with the new window.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::multipleColorOpacityChanged(int opacity)
{
    // If any subsets are selected, change their opacities.
    if(multipleColorList->currentItem() != -1)
    {
        for(int i = 0; i < multipleColorList->count(); ++i)
        {
            if(multipleColorList->isSelected(i) &&
               (i < subsetAtts->GetMultiColor().GetNumColorAttributes()))
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
//   This is a Qt slot function that is called when the subset selection
//   changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 13:15:21 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::subsetSelectionChanged()
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
// Method: QvisSubsetPlotWindow::overallOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the overall opacity slider
//   is clicked.
//
// Arguments:
//   opacity : The new opacity.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 1 16:01:20 PST 2002
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
//  Programmer:  Jeremy Meredith
//  Creation:    December  9, 2002
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
//   subset plot attributes.
//
// Arguments:
//   useDefault : Whether or not to use the default color table.
//   ctName     : The name of the color table to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 17:53:36 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::colorTableClicked(bool useDefault, const QString &ctName)
{
    subsetAtts->SetColorTableName(ctName.latin1());
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
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 18:11:27 PST 2005
//   Added pointSizePixels.
//
// ****************************************************************************

void
QvisSubsetPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the point size
    if(doAll)
    {
        subsetAtts->SetPointSize(pointControl->GetPointSize());
        subsetAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        subsetAtts->SetPointSizeVar(pointControl->GetPointSizeVar().latin1());
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
//  Creation:    November 4, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
QvisSubsetPlotWindow::pointTypeChanged(int type)
{
    subsetAtts->SetPointType((SubsetAttributes::PointType) type);
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
// Creation:   November 4, 2004 
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::pointSizeVarToggled(bool val)
{
    subsetAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::pointSizeVarChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and presses the Enter key.
//
// Arguments:
//   val :     The new value of the pointSizeVar text.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::pointSizeVarChanged(const QString &var)
{
    subsetAtts->SetPointSizeVar(var.latin1()); 
    Apply();
}


// ****************************************************************************
// Method: QvisSubsetPlotWindow::pointSizeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Arguments:
//   size :     The new point size.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPlotWindow::pointSizeChanged(double size)
{
    subsetAtts->SetPointSize(size); 
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
