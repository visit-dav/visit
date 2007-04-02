/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <QvisContourPlotWindow.h>

#include <math.h>
#include <stdio.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <QvisColorButton.h>
#include <QvisColorManagerWidget.h>
#include <QvisColorTableButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <ContourAttributes.h>
#include <ViewerProxy.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: QvisContourPlotWindow::QvisContourPlotWindow
//
// Purpose: 
//   Constructor for the QvisContourPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//    Eric Brugger, Wed Mar 14 12:00:23 PST 2001
//    I added the argument type.
//   
// ****************************************************************************

QvisContourPlotWindow::QvisContourPlotWindow(const int type,
    ContourAttributes *contourAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(contourAtts_, caption, shortName, notepad)
{
    plotType    = type;
    contourAtts = contourAtts_;
    colorModeButtons = 0;
}

// ****************************************************************************
// Method: QvisContourPlotWindow::~QvisContourPlotWindow
//
// Purpose: 
//   Destructor for the QvisContourPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisContourPlotWindow::~QvisContourPlotWindow()
{
    contourAtts = 0;
    delete colorModeButtons;
}

// ****************************************************************************
// Method: QvisContourPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Notes:
//   The color selection portions of this window are not yet implemented.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Feb 28 14:47:56 PST 2001
//   Added radio buttons for selecting scale.
//
//   Jeremy Meredith, Wed Mar 13 10:20:32 PST 2002
//   Added a toggle for wireframe mode.
//
//   Brad Whitlock, Thu Aug 22 10:26:08 PDT 2002
//   I added an opacity slider for single color opacity.
//
//   Brad Whitlock, Tue Dec 3 08:54:59 PDT 2002
//   I added a color table button.
//
//   Brad Whitlock, Mon Sep 8 17:02:29 PST 2003
//   I changed some menu labels to plural.
//
// ****************************************************************************

void
QvisContourPlotWindow::CreateWindowContents()
{
    QGridLayout *checkBoxLayout = new QGridLayout(topLayout, 1, 4, 10);

    // Create the lineSyle widget.
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    checkBoxLayout->addWidget(lineStyle, 0, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(lineStyle, "Line style",
                                central, "lineStyleLabel");
    checkBoxLayout->addWidget(lineStyleLabel, 0, 0);

    // Create the lineSyle widget.
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    checkBoxLayout->addWidget(lineWidth, 0, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, "Line width",
                                central, "lineWidthLabel");
    checkBoxLayout->addWidget(lineWidthLabel, 0, 2);

    // Create the contour color group box.
    contourColorGroup = new QGroupBox(central, "contourColorGroup");
    contourColorGroup->setTitle("Contour colors");
    topLayout->addWidget(contourColorGroup);
    QVBoxLayout *innerLayout = new QVBoxLayout(contourColorGroup);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);
    QVBoxLayout *colorLayout = new QVBoxLayout(innerLayout);
    colorLayout->setSpacing(10);

    // Create the mode buttons that determine if the window is in single,
    // multiple, or color table color mode.
    colorModeButtons = new QButtonGroup(0, "colorModeButtons");
    colorModeButtons->setFrameStyle(QFrame::NoFrame);
    connect(colorModeButtons, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    QGridLayout *innerColorLayout = new QGridLayout(colorLayout,4,3);
    innerColorLayout->setSpacing(10);
    innerColorLayout->setColStretch(2, 1000);

    QRadioButton *rb = new QRadioButton("Color table", contourColorGroup,
        "colorTable");
    colorModeButtons->insert(rb);
    innerColorLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton("Single", contourColorGroup, "singleColor");
    colorModeButtons->insert(rb);
    innerColorLayout->addWidget(rb, 1, 0);
    rb = new QRadioButton("Multiple", contourColorGroup, "multipleColor");
    colorModeButtons->insert(rb);
    innerColorLayout->addWidget(rb, 2, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(contourColorGroup,
        "singleColorButton");
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    innerColorLayout->addWidget(singleColor, 1, 1);
    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(contourColorGroup,
        "singleColorOpacity");
    singleColorOpacity->setMinValue(0);
    singleColorOpacity->setMaxValue(255);
    singleColorOpacity->setTickInterval(64);
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    innerColorLayout->addWidget(singleColorOpacity, 1, 2);

    // Add the multiple colors widget.
    multipleColors = new QvisColorManagerWidget(contourColorGroup,
        "multipleColors");
    multipleColors->setNameLabelText("Level");
    connect(multipleColors, SIGNAL(colorChanged(const QColor &, int)),
            this, SLOT(multipleColorChanged(const QColor &, int)));
    connect(multipleColors, SIGNAL(opacityChanged(int, int)),
            this, SLOT(opacityChanged(int, int)));
    innerColorLayout->addMultiCellWidget(multipleColors, 3, 3, 0, 2);

    // Add the color table button.
    colorTableButton = new QvisColorTableButton(contourColorGroup, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    innerColorLayout->addMultiCellWidget(colorTableButton, 0, 0, 1, 2,
        AlignLeft | AlignVCenter);

    // Add the select by and limits stuff.
    topLayout->addSpacing(5);
    QGridLayout *limitsLayout = new QGridLayout(topLayout, 3, 3);
    limitsLayout->setSpacing(10);

    // Add the select by combo box.
    selectByComboBox = new QComboBox(false, central, "selectByComboBox");
    selectByComboBox->insertItem("N levels");
    selectByComboBox->insertItem("Value(s)");
    selectByComboBox->insertItem("Percent(s)");
    connect(selectByComboBox, SIGNAL(activated(int)),
           this, SLOT(selectByChanged(int)));
    QLabel *selectByLabel = new QLabel(selectByComboBox, "Select by",
                                       central, "selectByLabel");
    limitsLayout->addWidget(selectByLabel, 0, 0);
    limitsLayout->addWidget(selectByComboBox, 0, 1);

    // Add the select by text field.
    selectByLineEdit = new QLineEdit(central, "selectByLineEdit");
    connect(selectByLineEdit, SIGNAL(returnPressed()),
           this, SLOT(processSelectByText()));
    limitsLayout->addWidget(selectByLineEdit, 0, 2);

    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(topLayout);

    // Create a group of radio buttons
    scalingButtons = new QButtonGroup( central, "scaleRadioGroup" );
    scalingButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *scaleLabel = new QLabel(scalingButtons, "Scale", central,
        "scaleLabel");
    scaleLayout->addWidget(scaleLabel);

    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout(scalingButtons);
    scaleButtonsLayout->setSpacing(10);
    rb = new QRadioButton("Linear", scalingButtons );
    rb->setChecked( TRUE );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( "Log" );
    scaleButtonsLayout->addWidget(rb);
    scaleLayout->addWidget( scalingButtons );
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scaleClicked slot.
    connect(scalingButtons, SIGNAL(clicked(int)),
            this, SLOT(scaleClicked(int)));

    //
    // Create the Limits stuff
    //
    QLabel *limitsLabel = new QLabel("Limits", central, "limitsLabel");
    limitsLayout->addWidget(limitsLabel, 1, 0);
    // Create the min toggle and line edit
    minToggle = new QCheckBox("Min", central, "minToggle");
    limitsLayout->addWidget(minToggle, 1, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central, "minLineEdit");
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 1, 2);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox("Max", central, "maxToggle");
    limitsLayout->addWidget(maxToggle, 2, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central, "maxLineEdit");
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 2, 2);

    // Create the legend toggle
    legendCheckBox = new QCheckBox("Legend", central, "legendToggle");
    connect(legendCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    topLayout->addWidget(legendCheckBox);

    // Create the wireframe toggle
    wireframeCheckBox = new QCheckBox("Wireframe", central, "wireframeCheckBox");
    connect(wireframeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(wireframeToggled(bool)));
    topLayout->addWidget(wireframeCheckBox);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::UpdateWindow
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
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//
//   Kathleen Bonnell, Mon Mar 26 18:17:53 PST 2001
//   Disabled lineStyle and lineStyleLabel until we have vtk version
//   in which line stippling is available.
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enable lineStyle and lineStyleLabel. 
//
//   Jeremy Meredith, Wed Mar 13 10:20:32 PST 2002
//   Added a toggle for wireframe mode.
//
//   Brad Whitlock, Thu Aug 22 10:33:06 PDT 2002
//   I added code to update the single color opacity slider and set the 
//   enabled state for the single and multiple color widgets.
//
//   Kathleen Bonnell, Thu Oct  3 13:23:42 PDT 2002  
//   I added code to disable min/max if contourMethod is "value", ensure
//   it is enabled otherwise. 
//
//   Brad Whitlock, Fri Nov 22 12:07:38 PDT 2002
//   I added new attributes to support color tables.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisContourPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    bool updateColors = false;
    bool updateNames = false;
    int  index;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < contourAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!contourAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // defaultPalette
            // Do nothing
            break;
        case 1: // changedColors
            // Do nothing
            break;
        case 2: // colorType
            colorModeButtons->blockSignals(true);
            if(contourAtts->GetColorType() == ContourAttributes::ColorByColorTable) 
                index = 0;
            else if(contourAtts->GetColorType() == ContourAttributes::ColorBySingleColor) 
                index = 1;
            else
                index = 2;
            colorModeButtons->setButton(index);
            colorModeButtons->blockSignals(false);

            singleColor->setEnabled(index == 1);
            singleColorOpacity->setEnabled(index == 1);
            colorTableButton->setEnabled(index == 0);
            break;
        case 3: // colorTableName
            colorTableButton->setColorTable(contourAtts->GetColorTableName().c_str());
            break;
        case 4: // legendFlag
            legendCheckBox->blockSignals(true);
            legendCheckBox->setChecked(contourAtts->GetLegendFlag());
            legendCheckBox->blockSignals(false);
            break;
        case 5: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(contourAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case 6: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(contourAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 7: // singleColor
            { // new scope
            QColor temp(contourAtts->GetSingleColor().Red(),
                        contourAtts->GetSingleColor().Green(),
                        contourAtts->GetSingleColor().Blue());
            singleColor->blockSignals(true);
            singleColor->setButtonColor(temp);
            singleColor->blockSignals(false);
            singleColorOpacity->blockSignals(true);
            singleColorOpacity->setValue(contourAtts->GetSingleColor().Alpha());
            singleColorOpacity->setGradientColor(temp);
            singleColorOpacity->blockSignals(false);
            }
            break;
        case 8: // multiColor
            updateColors = true;
            break;
        case 9: // contourNLevels
            updateNames = true;
            updateColors = true;

            if(contourAtts->GetContourMethod() == ContourAttributes::Level)
                UpdateSelectByText();
            break;
        case 10: // contourValue
            updateNames = true;
            updateColors = true;

            if(contourAtts->GetContourMethod() == ContourAttributes::Value)
                UpdateSelectByText();
            break;
        case 11: // contourPercent
            updateNames = true;
            updateColors = true;

            if(contourAtts->GetContourMethod() == ContourAttributes::Percent)
                UpdateSelectByText();
            break;
        case 12: // contourMethod
            selectByComboBox->blockSignals(true);
            selectByComboBox->setCurrentItem(contourAtts->GetContourMethod());
            selectByComboBox->blockSignals(false);

            // Set the column header for the name on the color manager.
            if(contourAtts->GetContourMethod() == ContourAttributes::Level)
            {
                minToggle->setEnabled(true);
                minLineEdit->setEnabled(contourAtts->GetMinFlag());
                maxToggle->setEnabled(true);
                maxLineEdit->setEnabled(contourAtts->GetMaxFlag());
                multipleColors->setNameLabelText("Level");
            }

            else if(contourAtts->GetContourMethod() == ContourAttributes::Value)
            {
                minToggle->setEnabled(false);
                minLineEdit->setEnabled(false);
                maxToggle->setEnabled(false);
                maxLineEdit->setEnabled(false);
                multipleColors->setNameLabelText("Value");
            }
            else if(contourAtts->GetContourMethod() == ContourAttributes::Percent)
            {
                minToggle->setEnabled(true);
                minLineEdit->setEnabled(contourAtts->GetMinFlag());
                maxToggle->setEnabled(true);
                maxLineEdit->setEnabled(contourAtts->GetMaxFlag());
                multipleColors->setNameLabelText("Percent");
            }

            updateNames = true;
            updateColors = true;
            UpdateSelectByText();
            break;
        case 13: // minFlag
            minToggle->blockSignals(true);
            minToggle->setChecked(contourAtts->GetMinFlag());
            minLineEdit->setEnabled(contourAtts->GetMinFlag());
            minToggle->blockSignals(false);
            break;
        case 14: // maxFlag
            maxToggle->blockSignals(true);
            maxToggle->setChecked(contourAtts->GetMaxFlag());
            maxLineEdit->setEnabled(contourAtts->GetMaxFlag());
            maxToggle->blockSignals(false);
            break;
        case 15: // min
            temp.setNum(contourAtts->GetMin());
            minLineEdit->setText(temp);
            break;
        case 16: // max
            temp.setNum(contourAtts->GetMax());
            maxLineEdit->setText(temp);
            break;
        case 17: // scaling 
            scalingButtons->setButton(contourAtts->GetScaling());
            break;
        case 18: // wireframe
            wireframeCheckBox->blockSignals(true);
            wireframeCheckBox->setChecked(contourAtts->GetWireframe());
            wireframeCheckBox->blockSignals(false);
            break;
        }
    } // end for

    // If we need to update the colors, update them now. This ensures that it
    // is only done one time no matter what combination of attributes was
    // selected.
    bool multiEnabled = (contourAtts->GetColorType() ==
        ContourAttributes::ColorByMultipleColors);
    if(updateColors)
        multiEnabled &= UpdateMultipleAreaColors();

    // If we need to update the names, update them now.
    if(updateNames)
        multiEnabled &= UpdateMultipleAreaNames();

    // Set the enabled state for the multiple color widget.
    multipleColors->setEnabled(multiEnabled);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::UpdateMultipleAreaColors
//
// Purpose: 
//   This method updates the multipleColors widget with the list of contour
//   colors.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Aug 22 12:31:19 PDT 2002
//   I made it return true if the widget should be enabled.
//
// ****************************************************************************

bool
QvisContourPlotWindow::UpdateMultipleAreaColors()
{
    const ColorAttributeList &contourColors = contourAtts->GetMultiColor();
    int   i, nEntries;
    int   cm = contourAtts->GetContourMethod();

    if(cm == ContourAttributes::Level)
        nEntries = contourAtts->GetContourNLevels();
    else if(cm == ContourAttributes::Value)
        nEntries = contourAtts->GetContourValue().size();
    else
        nEntries = contourAtts->GetContourPercent().size();

    // Block the signals from the multipleColors widget.
    multipleColors->blockSignals(true);

    if(nEntries == multipleColors->numEntries())
    {
        for(i = 0; i < nEntries; ++i)
        {
            QColor temp(contourColors[i].Red(), contourColors[i].Green(),
                        contourColors[i].Blue());

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, contourColors[i].Alpha());
        }
    }
    else if(nEntries > multipleColors->numEntries())
    {
        // Set all of the existing colors.
        for(i = 0; i < multipleColors->numEntries(); ++i)
        {
            QColor temp(contourColors[i].Red(), contourColors[i].Green(),
                        contourColors[i].Blue());

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, contourColors[i].Alpha());
        }

        // Add new entries
        for(i = multipleColors->numEntries();
            i < nEntries; ++i)
        {
            QColor temp(contourColors[i].Red(), contourColors[i].Green(),
                        contourColors[i].Blue());

            multipleColors->addEntry(QString(""), temp, contourColors[i].Alpha());
        }
    }
    else // nEntries < multipleColors->numEntries()
    {
        // Set all of the existing names.
        for(i = 0; i < nEntries; ++i)
        {
            QColor temp(contourColors[i].Red(), contourColors[i].Green(),
                        contourColors[i].Blue());

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, contourColors[i].Alpha());
        }

        // Remove excess entries
        int numEntries = multipleColors->numEntries();
        for(i = nEntries;
            i < numEntries; ++i)
        {
            multipleColors->removeLastEntry();
        }
    }

    // Unblock the signals from the multipleColors widget.
    multipleColors->blockSignals(false);

    return (nEntries > 0);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::LevelString
//
// Purpose: 
//   Returns a string version of the data value that we want.
//
// Arguments:
//   i : The index of the data value that we want to use.
//
// Returns:    A string version of the data value that we want.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 14:18:36 PST 2001
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

QString
QvisContourPlotWindow::LevelString(int i)
{
    QString retval("");
    int     cm = contourAtts->GetContourMethod();

    if(cm == ContourAttributes::Level)
        retval.sprintf("%d", i + 1);
    else if(cm == ContourAttributes::Value)
        retval.setNum(contourAtts->GetContourValue()[i]);
    else
        retval.sprintf("%g %%", contourAtts->GetContourPercent()[i]);

    return retval;
}

// ****************************************************************************
// Method: QvisContourPlotWindow::UpdateMultipleAreaNames
//
// Purpose: 
//   This method updates the multipleColors widget with the list of contour
//   names.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Aug 22 12:31:51 PDT 2002
//   I made it return true if the widget should be enabled.
//
// ****************************************************************************

bool
QvisContourPlotWindow::UpdateMultipleAreaNames()
{
    QString temp;
    int     i, vecSize;
    int     cm = contourAtts->GetContourMethod();

    if(cm == ContourAttributes::Level)
        vecSize = contourAtts->GetContourNLevels();
    else if(cm == ContourAttributes::Value)
        vecSize = contourAtts->GetContourValue().size();
    else
        vecSize = contourAtts->GetContourPercent().size();

    if(vecSize == multipleColors->numEntries())
    {
        for(i = 0; i < vecSize; ++i)
        {
            temp = LevelString(i);
            multipleColors->setAttributeName(i, temp);
        }
    }
    else if(vecSize > multipleColors->numEntries())
    {
        // Set all of the existing names.
        for(i = 0; i < multipleColors->numEntries(); ++i)
        {
            temp = LevelString(i);
            multipleColors->setAttributeName(i, temp);
        }

        // Add new entries
        for(i = multipleColors->numEntries(); i < vecSize; ++i)
        {
            temp = LevelString(i);
            multipleColors->addEntry(temp, QColor(0,0,0), 255);
        }
    }
    else // if(vecSize < multipleColors->numEntries())
    {
        // Set all of the existing names.
        for(i = 0; i < vecSize; ++i)
        {
            temp = LevelString(i);
            multipleColors->setAttributeName(i, temp);
        }

        // Remove excess entries
        int numEntries = multipleColors->numEntries();
        for(i = vecSize; i < numEntries; ++i)
        {
            multipleColors->removeLastEntry();
        }
    }

    return (vecSize > 0);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::UpdateSelectByText
//
// Purpose: 
//   This method updates the selectBy line edit when the value, percent, or
//   NLevels change.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 12:18:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::UpdateSelectByText()
{
    QString temp, temp2;
    int     i;

    if(contourAtts->GetContourMethod() == ContourAttributes::Level)
    {
        temp.sprintf("%d", contourAtts->GetContourNLevels());
        selectByLineEdit->setText(temp);
    }
    else if(contourAtts->GetContourMethod() == ContourAttributes::Value)
    {
        for(i = 0; i < contourAtts->GetContourValue().size(); ++i)
        {
            temp2.sprintf("%g ", contourAtts->GetContourValue()[i]);
            temp += temp2;
        }
        selectByLineEdit->setText(temp);
    }
    else if(contourAtts->GetContourMethod() == ContourAttributes::Percent)
    {
        for(i = 0; i < contourAtts->GetContourPercent().size(); ++i)
        {
            temp2.sprintf("%g ", contourAtts->GetContourPercent()[i]);
            temp += temp2;
        }
        selectByLineEdit->setText(temp);
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values from the text fields and puts the values in the
//   contourAtts.
//
// Arguments:
//   which_widget : A number indicating which line edit for which to get
//                  the value. An index of -1 gets them all.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 09:59:39 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the select by line edit.
    if(which_widget == 0 || doAll)
        ProcessSelectByText();

    // Do the minimum value.
    if(which_widget == 1 || doAll)
    {
        temp = minLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            contourAtts->SetMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The minimum value was invalid. "
                "Resetting to the last good value of %g.",
                contourAtts->GetMin());
            Message(msg);
            contourAtts->SetMin(contourAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == 2 || doAll)
    {
        temp = maxLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            contourAtts->SetMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The maximum value was invalid. "
                "Resetting to the last good value of %g.",
                contourAtts->GetMax());
            Message(msg);
            contourAtts->SetMax(contourAtts->GetMax());
        }
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::ProcessSelectByText
//
// Purpose: 
//   Processes the string in the selectByLineEdit and sets the results into
//   the contour attributes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 12:50:35 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 15:20:33 PST 2002
//   Added a code to prevent more than MAX_CONTOURS contours.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added MAX_CONTOURS args to calls to StringToDoubleList 
// ****************************************************************************

void
QvisContourPlotWindow::ProcessSelectByText()
{
    doubleVector temp;

    if(contourAtts->GetContourMethod() == ContourAttributes::Level)
    {
        // Try converting the line edit to a double vector so we can take
        // the first element as the number of levels.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp,
                           ContourAttributes::MAX_CONTOURS);

        // If there were elements in the list use the first one, else use 10.
        int nlevels = (temp.size() > 0) ? int(temp[0]) : 10;

        // Prevent less than one contour
        if(nlevels < 1)
        {
            Warning("VisIt requires at least one contour.");
            nlevels = 1;
        }

        // Prevent an excess number of contours.
        if(nlevels > ContourAttributes::MAX_CONTOURS)
        {
            QString tmp;
            tmp.sprintf("VisIt will not allow more than %d contours.",
                        ContourAttributes::MAX_CONTOURS);
            Warning(tmp);
            nlevels = ContourAttributes::MAX_CONTOURS;
        }

        contourAtts->SetContourNLevels(nlevels);
    }
    else if(contourAtts->GetContourMethod() == ContourAttributes::Value)
    {
        // Convert the text fo a list of doubles and store them in the
        // contour's value vector.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp,
                           ContourAttributes::MAX_CONTOURS);
        contourAtts->SetContourValue(temp);
    }
    else if(contourAtts->GetContourMethod() == ContourAttributes::Percent)
    {
        // Convert the text to a list of doubles and store them in the
        // contour's percent vector.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp,
                           ContourAttributes::MAX_CONTOURS);
        contourAtts->SetContourPercent(temp);
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::Apply
//
// Purpose: 
//   This method applies the contour plot attributes and optionally
//   tells the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            contour plot attributes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Feb 28 14:47:56 PST 2001
//   Added range checking for log plots.
//
//   Eric Brugger, Wed Mar 14 12:00:23 PST 2001
//   I modified the routine to pass to the viewer proxy the plot
//   type stored within the class instead of the one hardwired from
//   an include file.
//   
//   Kathleen Bonnell, Tue Mar 27 15:27:50 PST 2001
//   Removed range checking for log plots, as varMin, varMax no longer
//   members of ContourAttributes.  Range checking will now be done
//   within the plot itself and an exception thrown if invalid.
//
// ****************************************************************************

void
QvisContourPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current contour plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        contourAtts->Notify();

        // Tell the viewer to set the contour plot attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        contourAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisContourPlotWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Eric Brugger, Wed Mar 14 12:00:23 PST 2001
//   I modified the routine to pass to the viewer proxy the plot
//   type stored within the class instead of the one hardwired from
//   an include file.
//   
// ****************************************************************************

void
QvisContourPlotWindow::makeDefault()
{
    // Tell the viewer to set the default contour plot attributes.
    GetCurrentValues(-1);
    contourAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::reset()
{
    // Tell the viewer to reset the contour plot attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisContourPlotWindow::lineStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line style is changed.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::lineStyleChanged(int newStyle)
{
    contourAtts->SetLineStyle(newStyle);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::lineWidthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   line width widget is changed.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::lineWidthChanged(int newWidth)
{
    contourAtts->SetLineWidth(newWidth);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   legend toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::legendToggled(bool val)
{
    contourAtts->SetLegendFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::wireframeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   wireframe toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Jeremy Meredith
// Creation:   March 13, 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::wireframeToggled(bool val)
{
    contourAtts->SetWireframe(val);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::colorModeChanged
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
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 3 09:28:36 PDT 2002
//   I rewrote the method.
//
// ****************************************************************************

void
QvisContourPlotWindow::colorModeChanged(int index)
{
    if(index == 0)
        contourAtts->SetColorType(ContourAttributes::ColorByColorTable);
    else if(index == 1)
        contourAtts->SetColorType(ContourAttributes::ColorBySingleColor);
    else
        contourAtts->SetColorType(ContourAttributes::ColorByMultipleColors);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::singleColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the single color button's
//   color changes.
//
// Arguments:
//   color : The new single color.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Aug 22 10:44:37 PDT 2002
//   I added a little code to preserve the alpha component of the color.
//
// ****************************************************************************

void
QvisContourPlotWindow::singleColorChanged(const QColor &color)
{
    int alpha = contourAtts->GetSingleColor().Alpha();
    ColorAttribute temp(color.red(), color.green(), color.blue(), alpha);
    contourAtts->SetSingleColor(temp);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::singleColorOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that sets the opacity component of the
//   single color.
//
// Arguments:
//   opacity : The new opacity value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 22 10:46:22 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::singleColorOpacityChanged(int opacity)
{
    contourAtts->GetSingleColor().SetAlpha(opacity);
    contourAtts->SelectSingleColor();
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::multipleColorChanged
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
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 19:18:25 PST 2001
//   Made it conform to ContourAttribute's new interface.
//
//   Brad Whitlock, Mon Dec 9 11:01:45 PDT 2002
//   Added code to mark the color as having been changed.
//
// ****************************************************************************

void
QvisContourPlotWindow::multipleColorChanged(const QColor &color, int index)
{
    if(index >= 0 &&
       index < contourAtts->GetMultiColor().GetNumColors())
    {
        contourAtts->GetMultiColor()[index].SetRgb(color.red(), color.green(),
                                                   color.blue());
        contourAtts->SelectMultiColor();
        contourAtts->MarkColorAsChanged(index);

        Apply();
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::opacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the opacity changes for one
//   of the contours in the multiple colors area.
//
// Arguments:
//   opacity : The new opacity.
//   index   : The index of the contour that changed.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:42:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 19:18:25 PST 2001
//   Made it conform to ContourAttribute's new interface.
//
//   Brad Whitlock, Mon Dec 9 11:01:45 PDT 2002
//   Added code to mark the color as having been changed.
//
// ****************************************************************************

void
QvisContourPlotWindow::opacityChanged(int opacity, int index)
{
    if(index >= 0 &&
       index < contourAtts->GetMultiColor().GetNumColors())
    {
        contourAtts->GetMultiColor()[index].SetAlpha(opacity);
        contourAtts->SelectMultiColor();
        contourAtts->MarkColorAsChanged(index);

        Apply();
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::selectByChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contour method changes.
//
// Arguments:
//   mode : The new contour method.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:03:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Nov 22 14:29:15 PST 2002
//   I made it work with the new attributes.
//
//   Brad Whitlock, Mon Dec 9 14:57:25 PST 2002
//   I fixed a bug that prevented the window from working correctly while
//   autoupdate is on.
//
// ****************************************************************************

void
QvisContourPlotWindow::selectByChanged(int mode)
{
    contourAtts->SetContourMethod(ContourAttributes::Select_by(mode));
    if(AutoUpdate())
        UpdateSelectByText();
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::processSelectByText
//
// Purpose: 
//   This is a Qt slot function that is called then the selectbyLineEdit
//   changes values.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:04:18 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::processSelectByText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::minToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the min toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:05:28 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::minToggled(bool val)
{
    contourAtts->SetMinFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::scaleClicked
//
// Purpose: 
//   This is a Qt slot function that is called when a scale button is clicked. 
//
// Arguments:
//   button  :  Which scaling button was selected. 
//
// Programmer: Kathleen Bonnell 
// Creation:   February 27, 2001 
//
// Modifications:
//   Brad Whitlock, Fri Nov 22 14:28:55 PST 2002
//   Made it work with the new attributes.
//
// ****************************************************************************

void
QvisContourPlotWindow::scaleClicked(int button)
{
    // Only do it if it changed.
    ContourAttributes::Scaling val = ContourAttributes::Scaling(button);
    if (val != contourAtts->GetScaling())
    {
        contourAtts->SetScaling(val);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisContourPlotWindow::processMinLimitText
//
// Purpose: 
//   This is a Qt slot function that is called when the min limit changes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:11:38 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::processMinLimitText()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::maxToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the max toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:05:28 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::maxToggled(bool val)
{
    contourAtts->SetMaxFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::processMaxLimitText
//
// Purpose: 
//   This is a Qt slot function that is called when the max limit changes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:11:38 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisContourPlotWindow::processMaxLimitText()
{
    GetCurrentValues(2);
    Apply();
}

// ****************************************************************************
// Method: QvisContourPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table into the
//   contour plot attributes.
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
QvisContourPlotWindow::colorTableClicked(bool, const QString &ctName)
{
    contourAtts->SetColorTableName(ctName.latin1());
    Apply();
}

