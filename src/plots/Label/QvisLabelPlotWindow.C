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

#include <QvisLabelPlotWindow.h>

#include <LabelAttributes.h>
#include <ViewerProxy.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <QvisColorButton.h>

#include <stdio.h>

// ****************************************************************************
// Method: QvisLabelPlotWindow::QvisLabelPlotWindow
//
// Purpose: 
//   Constructor for the QvisLabelPlotWindow class.
//
// Arguments:
//   type      : An identifier used to identify the plot type in the viewer.
//   subj      : The label attributes that the window observes.
//   caption   : The caption displayed in the window decorations.
//   shortName : The name used in the notepad.
//   notepad   : The notepad area where the window posts itself.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 9 14:05:35 PST 2005
//   Added depthTestButtonGroup.
//
// ****************************************************************************

QvisLabelPlotWindow::QvisLabelPlotWindow(const int type,
    LabelAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) : QvisPostableWindowObserver(subj, caption,
        shortName, notepad)
{
    plotType = type;
    labelAtts = subj;

    depthTestButtonGroup = 0;
}

// ****************************************************************************
// Method: QvisLabelPlotWindow::~QvisLabelPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 9 14:05:27 PST 2005
//   Added depthTestButtonGroup.
//
// ****************************************************************************

QvisLabelPlotWindow::~QvisLabelPlotWindow()
{
    delete depthTestButtonGroup;
}

// ****************************************************************************
// Method: QvisLabelPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 2 14:50:32 PST 2005
//   I removed the single cell/node controls. I also added controls for
//   zbuffering and additional controls for changing label color and height
//   for meshes.
//
// ****************************************************************************

void
QvisLabelPlotWindow::CreateWindowContents()
{
    //
    // Create label selection-related widgets.
    //
    selectionGroupBox = new QGroupBox(central, "selectionGroupBox");
    selectionGroupBox->setTitle("Selection");
    topLayout->addWidget(selectionGroupBox);
    QVBoxLayout *selTopLayout = new QVBoxLayout(selectionGroupBox);
    selTopLayout->setMargin(10);
    selTopLayout->addSpacing(15);
    QGridLayout *selLayout = new QGridLayout(selTopLayout, 4, 3);
    selLayout->setSpacing(5);

    showNodesToggle = new QCheckBox("Show nodes", selectionGroupBox,
        "showNodesToggle");
    connect(showNodesToggle, SIGNAL(toggled(bool)),
            this, SLOT(showNodesToggled(bool)));
    selLayout->addWidget(showNodesToggle, 0,0);

    showCellsToggle = new QCheckBox("Show cells", selectionGroupBox,
        "showCellsToggle");
    connect(showCellsToggle, SIGNAL(toggled(bool)),
            this, SLOT(showCellsToggled(bool)));
    selLayout->addWidget(showCellsToggle, 0,1);

    restrictNumberOfLabelsToggle = new QCheckBox("Restrict number of labels to",
        selectionGroupBox, "restrictNumberOfLabelsToggle");
    connect(restrictNumberOfLabelsToggle, SIGNAL(toggled(bool)),
            this, SLOT(restrictNumberOfLabelsToggled(bool)));
    selLayout->addMultiCellWidget(restrictNumberOfLabelsToggle, 1,1,0,1);

    numberOfLabelsSpinBox = new QSpinBox(1, 200000, 200, selectionGroupBox,
        "numberOfLabelsSpinBox");
    //numberOfLabelsSpinBox->setSuffix(" labels");
    connect(numberOfLabelsSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(numberOfLabelsChanged(int)));
    selLayout->addWidget(numberOfLabelsSpinBox, 1, 2);

    drawLabelsFacingComboBox = new QComboBox(selectionGroupBox,
        "drawLabelsFacingComboBox");
    drawLabelsFacingComboBox->insertItem("Front");
    drawLabelsFacingComboBox->insertItem("Back");
    drawLabelsFacingComboBox->insertItem("Front or Back");
    connect(drawLabelsFacingComboBox, SIGNAL(activated(int)),
            this, SLOT(drawLabelsFacingChanged(int)));
    selLayout->addWidget(drawLabelsFacingComboBox, 2, 2);
    selLayout->addMultiCellWidget(new QLabel("Draw labels that face",
        selectionGroupBox, "drawLabelsLabel"), 2, 2, 0, 1);

    depthTestButtonGroup = new QButtonGroup(0, "depthTestButtonGroup");
    QHBox *dtHBox = new QHBox(selectionGroupBox, "dtHBox");
    dtHBox->setSpacing(5);
    dtHBox->setMargin(0);
    QRadioButton *rb = new QRadioButton("Auto", dtHBox);
    depthTestButtonGroup->insert(rb, 0);
    rb = new QRadioButton("Always", dtHBox);
    depthTestButtonGroup->insert(rb, 1);
    rb = new QRadioButton("Never", dtHBox);
    depthTestButtonGroup->insert(rb, 2);
    connect(depthTestButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(depthTestButtonGroupChanged(int)));
    selLayout->addWidget(new QLabel("Depth test mode", selectionGroupBox,
        "dtLabel"), 3, 0);
    selLayout->addMultiCellWidget(dtHBox, 3, 3, 1, 2);

    //
    // Create formatting widgets
    //
    formattingGroupBox = new QGroupBox(central, "formattingGroupBox");
    formattingGroupBox->setTitle("Formatting");
    topLayout->addWidget(formattingGroupBox);
    QVBoxLayout *fmtTopLayout = new QVBoxLayout(formattingGroupBox);
    fmtTopLayout->setMargin(10);
    fmtTopLayout->addSpacing(15);
    QGridLayout *fmtLayout = new QGridLayout(fmtTopLayout, 7, 2);
    fmtLayout->setSpacing(5);

    labelDisplayFormatComboBox = new QComboBox(formattingGroupBox,
        "labelDisplayFormatComboBox");
    labelDisplayFormatComboBox->insertItem("Natural");
    labelDisplayFormatComboBox->insertItem("Logical index");
    labelDisplayFormatComboBox->insertItem("Index");
    connect(labelDisplayFormatComboBox, SIGNAL(activated(int)),
            this, SLOT(labelDisplayFormatChanged(int)));
    fmtLayout->addWidget(labelDisplayFormatComboBox, 0, 1);
    fmtLayout->addWidget(new QLabel(labelDisplayFormatComboBox,
        "Label display format", formattingGroupBox, "labelDisplayLabel"), 0, 0);
    
    specifyTextColor1Toggle = new QCheckBox("Specify label color",
        formattingGroupBox, "specifyTextColor1Toggle");
    connect(specifyTextColor1Toggle, SIGNAL(toggled(bool)),
            this, SLOT(specifyTextColor1Toggled(bool)));
    fmtLayout->addWidget(specifyTextColor1Toggle, 1, 0);

    textColor1Button = new QvisColorButton(formattingGroupBox, "textColor1Button");
    connect(textColor1Button, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(textColor1Changed(const QColor&)));
    fmtLayout->addWidget(textColor1Button, 1, 1, Qt::AlignLeft);

    specifyTextColor2Toggle = new QCheckBox("Specify node label color",
        formattingGroupBox, "specifyTextColor2Toggle");
    connect(specifyTextColor2Toggle, SIGNAL(toggled(bool)),
            this, SLOT(specifyTextColor2Toggled(bool)));
    fmtLayout->addWidget(specifyTextColor2Toggle, 2, 0);

    textColor2Button = new QvisColorButton(formattingGroupBox, "textColor2Button");
    connect(textColor2Button, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(textColor2Changed(const QColor&)));
    fmtLayout->addWidget(textColor2Button, 2, 1, Qt::AlignLeft);

    textHeight1SpinBox = new QSpinBox(1, 100, 1, formattingGroupBox,
        "textHeight1SpinBox");
    textHeight1SpinBox->setSuffix("%");
    connect(textHeight1SpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(textHeight1Changed(int)));
    textHeight1Label = new QLabel("Label height",
        formattingGroupBox, "labelDisplayLabel");
    fmtLayout->addWidget(textHeight1Label, 3, 0);
    fmtLayout->addWidget(textHeight1SpinBox, 3, 1);

    textHeight2SpinBox = new QSpinBox(1, 100, 1, formattingGroupBox,
        "textHeight2SpinBox");
    textHeight2SpinBox->setSuffix("%");
    connect(textHeight2SpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(textHeight2Changed(int)));
    textHeight2Label = new QLabel("Node label height",
        formattingGroupBox, "labelDisplayLabel");
    fmtLayout->addWidget(textHeight2Label, 4, 0);
    fmtLayout->addWidget(textHeight2SpinBox, 4, 1);

    horizontalJustificationComboBox = new QComboBox(formattingGroupBox,
        "horizontalJustificationComboBox");
    horizontalJustificationComboBox->insertItem("Center");
    horizontalJustificationComboBox->insertItem("Left");
    horizontalJustificationComboBox->insertItem("Right");
    connect(horizontalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(horizontalJustificationChanged(int)));
    fmtLayout->addWidget(horizontalJustificationComboBox, 5, 1);
    fmtLayout->addWidget(new QLabel(horizontalJustificationComboBox,
        "Horizontal justification", formattingGroupBox, "hjustLabel"), 5, 0);

    verticalJustificationComboBox = new QComboBox(formattingGroupBox,
        "verticalJustificationComboBox");
    verticalJustificationComboBox->insertItem("Center");
    verticalJustificationComboBox->insertItem("Top");
    verticalJustificationComboBox->insertItem("Bottom");
    connect(verticalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(verticalJustificationChanged(int)));
    fmtLayout->addWidget(verticalJustificationComboBox, 6, 1);
    fmtLayout->addWidget(new QLabel(verticalJustificationComboBox,
        "Vertical justification", formattingGroupBox, "vjustLabel"), 6, 0);

    // Lagend toggle
    legendToggle = new QCheckBox("Legend", central, "legendFlag");
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    topLayout->addWidget(legendToggle, 0,0);
}


// ****************************************************************************
// Method: QvisLabelPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 2 14:51:19 PST 2005
//   Changed field ordering. Added widgets that allow for setting colors for
//   node and cell labels.
//
// ****************************************************************************

void
QvisLabelPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    QColor  tempcolor;

    for(int i = 0; i < labelAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!labelAtts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
        case 0: //varType
            {// new scope
            bool varIsMesh = labelAtts->GetVarType() == LabelAttributes::LABEL_VT_MESH;
            showNodesToggle->setEnabled(varIsMesh);
            showCellsToggle->setEnabled(varIsMesh);
            specifyTextColor1Toggle->setText(varIsMesh?"Specify cell label color":"Specify label color");  
            textHeight1Label->setText(varIsMesh?"Cell label height" : "Label height");
            if(varIsMesh)
            {
                textColor2Button->show();
                specifyTextColor2Toggle->show();
                textHeight2Label->show();
                textHeight2SpinBox->show();
                updateGeometry();
            }
            else
            {
                textColor2Button->hide();
                specifyTextColor2Toggle->hide();
                textHeight2Label->hide();
                textHeight2SpinBox->hide();
                updateGeometry();
            }
            }
            break;
        case 1: //legendFlag
            legendToggle->blockSignals(true);
            legendToggle->setChecked(labelAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case 2: //showNodes
            showNodesToggle->blockSignals(true);
            showNodesToggle->setChecked(labelAtts->GetShowNodes());
            showNodesToggle->blockSignals(false);
            break;
        case 3: //showCells
            showCellsToggle->blockSignals(true);
            showCellsToggle->setChecked(labelAtts->GetShowCells());
            showCellsToggle->blockSignals(false);
            break;
        case 4: //restrictNumberOfLabels
            restrictNumberOfLabelsToggle->blockSignals(true);
            restrictNumberOfLabelsToggle->setChecked(labelAtts->GetRestrictNumberOfLabels());
            restrictNumberOfLabelsToggle->blockSignals(false);
            numberOfLabelsSpinBox->setEnabled(labelAtts->GetRestrictNumberOfLabels());
            break;
        case 5: //drawLabelsFacing
            drawLabelsFacingComboBox->blockSignals(true);
            drawLabelsFacingComboBox->setCurrentItem(int(labelAtts->GetDrawLabelsFacing()));
            drawLabelsFacingComboBox->blockSignals(false);
            break;
        case 6: //labelDisplayFormat
            labelDisplayFormatComboBox->blockSignals(true);
            labelDisplayFormatComboBox->setCurrentItem(int(labelAtts->GetLabelDisplayFormat()));
            labelDisplayFormatComboBox->blockSignals(false);
            break;
        case 7: //numberOfLabels
            numberOfLabelsSpinBox->blockSignals(true);
            numberOfLabelsSpinBox->setValue(labelAtts->GetNumberOfLabels());
            numberOfLabelsSpinBox->blockSignals(false);
            break;
        case 8: //specifyTextColor1
            specifyTextColor1Toggle->blockSignals(true);
            specifyTextColor1Toggle->setChecked(labelAtts->GetSpecifyTextColor1());
            specifyTextColor1Toggle->blockSignals(false);

            textColor1Button->setEnabled(labelAtts->GetSpecifyTextColor1());
            break;
        case 9: //textColor1
            tempcolor = QColor(labelAtts->GetTextColor1().Red(),
                               labelAtts->GetTextColor1().Green(),
                               labelAtts->GetTextColor1().Blue());
            textColor1Button->setButtonColor(tempcolor);
            break;
        case 10: //textHeight1
            textHeight1SpinBox->blockSignals(true);
            textHeight1SpinBox->setValue(int(labelAtts->GetTextHeight1() * 100.f + 0.5f));
            textHeight1SpinBox->blockSignals(false);
            break;

        case 11: //specifyTextColor2
            specifyTextColor2Toggle->blockSignals(true);
            specifyTextColor2Toggle->setChecked(labelAtts->GetSpecifyTextColor2());
            specifyTextColor2Toggle->blockSignals(false);

            textColor2Button->setEnabled(labelAtts->GetSpecifyTextColor2());
            break;
        case 12: //textColor2
            tempcolor = QColor(labelAtts->GetTextColor2().Red(),
                               labelAtts->GetTextColor2().Green(),
                               labelAtts->GetTextColor2().Blue());
            textColor2Button->setButtonColor(tempcolor);
            break;
        case 13: //textHeight2
            textHeight2SpinBox->blockSignals(true);
            textHeight2SpinBox->setValue(int(labelAtts->GetTextHeight2() * 100.f + 0.5f));
            textHeight2SpinBox->blockSignals(false);
            break;
        case 14: //horizontalJustification
            horizontalJustificationComboBox->blockSignals(true);
            horizontalJustificationComboBox->setCurrentItem(int(labelAtts->
                GetHorizontalJustification()));
            horizontalJustificationComboBox->blockSignals(false);
            break;
        case 15: //verticalJustification
            verticalJustificationComboBox->blockSignals(true);
            verticalJustificationComboBox->setCurrentItem(int(labelAtts->
                GetVerticalJustification()));
            verticalJustificationComboBox->blockSignals(false);
            break;
        case 16: //depthTestMode
            depthTestButtonGroup->blockSignals(true);
            depthTestButtonGroup->setButton(int(labelAtts->GetDepthTestMode()));
            depthTestButtonGroup->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisLabelPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Nov 29 15:27:08 PST 2004
//   Added code to get the values from the numberOfLabelsSpinBox and
//   textHeightSpinBox so it is not necessary to hit the Enter key.
//
//   Brad Whitlock, Tue Aug 2 14:54:10 PST 2005
//   I removed some fields and added code for textHeight2SpinBox.
//
// ****************************************************************************

void
QvisLabelPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do numberOfLabelsSpinBox
    if(which_widget == 0 || doAll)
    {
        temp = numberOfLabelsSpinBox->text().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            labelAtts->SetNumberOfLabels(val);
        }

        if(!okay)
        {
            msg.sprintf("The value entered for the number of labels was invalid. "
                "Resetting to the last good value of %d.",
                labelAtts->GetNumberOfLabels());
            Message(msg);
            labelAtts->SetNumberOfLabels(labelAtts->GetNumberOfLabels());
        }
    }

    // Do textHeight1SpinBox
    if(which_widget == 1 || doAll)
    {
        temp = textHeight1SpinBox->text();
        int plen = (temp.find(textHeight1SpinBox->prefix()) != -1) ? 
            textHeight1SpinBox->prefix().length() : 0;
        int slen = (temp.find(textHeight1SpinBox->suffix()) != -1) ? 
            textHeight1SpinBox->suffix().length() : 0;
        temp = temp.mid(plen, temp.length() - plen - slen);
        temp.simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int ival = temp.toInt(&okay);
            float val = float(ival) * 0.01f;
            labelAtts->SetTextHeight1(val);
        }

        if(!okay)
        {
            msg.sprintf("The value entered for the number of labels was invalid. "
                "Resetting to the last good value of %d.",
                int(labelAtts->GetTextHeight1() * 100.));
            Message(msg);
            labelAtts->SetTextHeight1(labelAtts->GetTextHeight1());
        }
    }

    // Do textHeight2SpinBox
    if(which_widget == 2 || doAll)
    {
        temp = textHeight2SpinBox->text();
        int plen = (temp.find(textHeight2SpinBox->prefix()) != -1) ? 
            textHeight2SpinBox->prefix().length() : 0;
        int slen = (temp.find(textHeight2SpinBox->suffix()) != -1) ? 
            textHeight2SpinBox->suffix().length() : 0;
        temp = temp.mid(plen, temp.length() - plen - slen);
        temp.simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int ival = temp.toInt(&okay);
            float val = float(ival) * 0.01f;
            labelAtts->SetTextHeight2(val);
        }

        if(!okay)
        {
            msg.sprintf("The value entered for the number of labels was invalid. "
                "Resetting to the last good value of %d.",
                int(labelAtts->GetTextHeight2() * 100.));
            Message(msg);
            labelAtts->SetTextHeight2(labelAtts->GetTextHeight2());
        }
    }
}

// ****************************************************************************
// Method: QvisLabelPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLabelPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        labelAtts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        labelAtts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisLabelPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLabelPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisLabelPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLabelPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    labelAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisLabelPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisLabelPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisLabelPlotWindow::showNodesToggled(bool val)
{
    labelAtts->SetShowNodes(val);
    SetUpdate(false);
    Apply();
}


void
QvisLabelPlotWindow::showCellsToggled(bool val)
{
    labelAtts->SetShowCells(val);
    SetUpdate(false);
    Apply();
}


void
QvisLabelPlotWindow::restrictNumberOfLabelsToggled(bool val)
{
    labelAtts->SetRestrictNumberOfLabels(val);
    Apply();
}

void
QvisLabelPlotWindow::numberOfLabelsChanged(int val)
{
    labelAtts->SetNumberOfLabels(val);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::drawLabelsFacingChanged(int val)
{
    if(val != labelAtts->GetDrawLabelsFacing())
    {
        labelAtts->SetDrawLabelsFacing(LabelAttributes::LabelDrawFacing(val));
        SetUpdate(false);
        Apply();
    }
}

void
QvisLabelPlotWindow::legendToggled(bool val)
{
    labelAtts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::labelDisplayFormatChanged(int val)
{
    if(val != labelAtts->GetLabelDisplayFormat())
    {
        labelAtts->SetLabelDisplayFormat(LabelAttributes::LabelIndexDisplay(val));
        SetUpdate(false);
        Apply();
    }
}

void
QvisLabelPlotWindow::textColor1Changed(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    labelAtts->SetTextColor1(temp);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::textColor2Changed(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    labelAtts->SetTextColor2(temp);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::specifyTextColor1Toggled(bool val)
{
    labelAtts->SetSpecifyTextColor1(val);
    Apply();
}

void
QvisLabelPlotWindow::specifyTextColor2Toggled(bool val)
{
    labelAtts->SetSpecifyTextColor2(val);
    Apply();
}

void
QvisLabelPlotWindow::textHeight1Changed(int val)
{
    float textHeight = float(val) * 0.01f;
    labelAtts->SetTextHeight1(textHeight);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::textHeight2Changed(int val)
{
    float textHeight = float(val) * 0.01f;
    labelAtts->SetTextHeight2(textHeight);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::horizontalJustificationChanged(int val)
{
    if(val != labelAtts->GetHorizontalJustification())
    {
        labelAtts->SetHorizontalJustification(LabelAttributes::LabelHorizontalAlignment(val));
        SetUpdate(false);
        Apply();
    }
}

void
QvisLabelPlotWindow::verticalJustificationChanged(int val)
{
    if(val != labelAtts->GetVerticalJustification())
    {
        labelAtts->SetVerticalJustification(LabelAttributes::LabelVerticalAlignment(val));
        SetUpdate(false);
        Apply();
    }
}

void
QvisLabelPlotWindow::depthTestButtonGroupChanged(int val)
{
    labelAtts->SetDepthTestMode((LabelAttributes::DepthTestMode)val);
    SetUpdate(false);
    Apply();
}

