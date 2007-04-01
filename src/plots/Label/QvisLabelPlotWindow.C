#include <QvisLabelPlotWindow.h>

#include <LabelAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
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
//   
// ****************************************************************************

QvisLabelPlotWindow::QvisLabelPlotWindow(const int type,
    LabelAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) : QvisPostableWindowObserver(subj, caption,
        shortName, notepad)
{
    plotType = type;
    labelAtts = subj;
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
//   
// ****************************************************************************

QvisLabelPlotWindow::~QvisLabelPlotWindow()
{
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
    QGridLayout *selLayout = new QGridLayout(selTopLayout, 3, 3);
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

    //
    // Create formatting widgets
    //
    formattingGroupBox = new QGroupBox(central, "formattingGroupBox");
    formattingGroupBox->setTitle("Formatting");
    topLayout->addWidget(formattingGroupBox);
    QVBoxLayout *fmtTopLayout = new QVBoxLayout(formattingGroupBox);
    fmtTopLayout->setMargin(10);
    fmtTopLayout->addSpacing(15);
    QGridLayout *fmtLayout = new QGridLayout(fmtTopLayout, 6, 2);
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
    
    useForegroundTextColorToggle = new QCheckBox("Use foreground color",
        formattingGroupBox, "useForegroundTextColorToggle");
    connect(useForegroundTextColorToggle, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundTextColorToggled(bool)));
    fmtLayout->addMultiCellWidget(useForegroundTextColorToggle, 1,1, 0, 1);

    textColorButton = new QvisColorButton(formattingGroupBox, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(textColorChanged(const QColor&)));
    textColorLabel = new QLabel(textColorButton, "Label color",
        formattingGroupBox, "textColorLabel");
    fmtLayout->addWidget(textColorLabel, 2, 0);
    fmtLayout->addWidget(textColorButton, 2,1, Qt::AlignLeft);

    textHeightSpinBox = new QSpinBox(1, 100, 1, formattingGroupBox,
        "textHeightSpinBox");
    textHeightSpinBox->setSuffix("%");
    connect(textHeightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(textHeightChanged(int)));
    fmtLayout->addWidget(textHeightSpinBox, 3, 1);
    fmtLayout->addWidget(new QLabel("Label height",
        formattingGroupBox, "labelDisplayLabel"), 3, 0);

    horizontalJustificationComboBox = new QComboBox(formattingGroupBox,
        "horizontalJustificationComboBox");
    horizontalJustificationComboBox->insertItem("Center");
    horizontalJustificationComboBox->insertItem("Left");
    horizontalJustificationComboBox->insertItem("Right");
    connect(horizontalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(horizontalJustificationChanged(int)));
    fmtLayout->addWidget(horizontalJustificationComboBox, 4, 1);
    fmtLayout->addWidget(new QLabel(horizontalJustificationComboBox,
        "Horizontal justification", formattingGroupBox, "hjustLabel"), 4, 0);

    verticalJustificationComboBox = new QComboBox(formattingGroupBox,
        "verticalJustificationComboBox");
    verticalJustificationComboBox->insertItem("Center");
    verticalJustificationComboBox->insertItem("Top");
    verticalJustificationComboBox->insertItem("Bottom");
    connect(verticalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(verticalJustificationChanged(int)));
    fmtLayout->addWidget(verticalJustificationComboBox, 5, 1);
    fmtLayout->addWidget(new QLabel(verticalJustificationComboBox,
        "Vertical justification", formattingGroupBox, "vjustLabel"), 5, 0);

    //
    // Create formatting widgets
    //
    singleValueGroupBox = new QGroupBox(central, "singleValueGroupBox");
    singleValueGroupBox->setTitle("Single value options");
    topLayout->addWidget(singleValueGroupBox);
    QVBoxLayout *svalTopLayout = new QVBoxLayout(singleValueGroupBox);
    svalTopLayout->setMargin(10);
    svalTopLayout->addSpacing(15);
    QGridLayout *svalLayout = new QGridLayout(svalTopLayout, 3, 2);
    svalLayout->setSpacing(5);

    showSingleNodeToggle = new QCheckBox("Show single node", 
        singleValueGroupBox, "showSingleNodeToggle");
    connect(showSingleNodeToggle, SIGNAL(toggled(bool)),
            this, SLOT(showSingleNodeToggled(bool)));
    svalLayout->addWidget(showSingleNodeToggle, 0,0);

    singleNodeLineEdit = new QLineEdit(singleValueGroupBox, "singleNodeLineEdit");
    connect(singleNodeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(singleNodeProcessText()));
    svalLayout->addWidget(singleNodeLineEdit, 0, 1);

    showSingleCellToggle = new QCheckBox("Show single cell",
        singleValueGroupBox, "showSingleCellToggle");
    connect(showSingleCellToggle, SIGNAL(toggled(bool)),
            this, SLOT(showSingleCellToggled(bool)));
    svalLayout->addWidget(showSingleCellToggle, 1,0);

    singleCellLineEdit = new QLineEdit(singleValueGroupBox, "singleCellLineEdit");
    connect(singleCellLineEdit, SIGNAL(returnPressed()),
            this, SLOT(singleCellProcessText()));
    svalLayout->addWidget(singleCellLineEdit, 1,1);

    markerTextLineEdit = new QLineEdit(singleValueGroupBox, "markerTextLineEdit");
    connect(markerTextLineEdit, SIGNAL(returnPressed()),
            this, SLOT(markerProcessText()));
    svalLayout->addWidget(markerTextLineEdit, 2,1);
    svalLayout->addWidget(new QLabel(markerTextLineEdit,
       "Marker text", singleValueGroupBox, "markerTextLabel"), 2, 0);

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
        case 0: //legendFlag
            legendToggle->blockSignals(true);
            legendToggle->setChecked(labelAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case 1: //showNodes
            showNodesToggle->blockSignals(true);
            showNodesToggle->setChecked(labelAtts->GetShowNodes());
            showNodesToggle->blockSignals(false);
            break;
        case 2: //showCells
            showCellsToggle->blockSignals(true);
            showCellsToggle->setChecked(labelAtts->GetShowCells());
            showCellsToggle->blockSignals(false);
            break;
        case 3: //restrictNumberOfLabels
            restrictNumberOfLabelsToggle->blockSignals(true);
            restrictNumberOfLabelsToggle->setChecked(labelAtts->GetRestrictNumberOfLabels());
            restrictNumberOfLabelsToggle->blockSignals(false);
            numberOfLabelsSpinBox->setEnabled(labelAtts->GetRestrictNumberOfLabels());
            break;
        case 4: //drawLabelsFacing
            drawLabelsFacingComboBox->blockSignals(true);
            drawLabelsFacingComboBox->setCurrentItem(int(labelAtts->GetDrawLabelsFacing()));
            drawLabelsFacingComboBox->blockSignals(false);
            break;
        case 5: //showSingleNode
            showSingleNodeToggle->blockSignals(true);
            showSingleNodeToggle->setChecked(labelAtts->GetShowSingleNode());
            showSingleNodeToggle->blockSignals(false);
            break;
        case 6: //showSingleCell
            showSingleCellToggle->blockSignals(true);
            showSingleCellToggle->setChecked(labelAtts->GetShowSingleCell());
            showSingleCellToggle->blockSignals(false);
            break;
        case 7: //useForegroundTextColor
            useForegroundTextColorToggle->blockSignals(true);
            useForegroundTextColorToggle->setChecked(labelAtts->GetUseForegroundTextColor());
            useForegroundTextColorToggle->blockSignals(false);

            textColorButton->setEnabled(!labelAtts->GetUseForegroundTextColor());
            textColorLabel->setEnabled(!labelAtts->GetUseForegroundTextColor());
            break;
        case 8: //labelDisplayFormat
            labelDisplayFormatComboBox->blockSignals(true);
            labelDisplayFormatComboBox->setCurrentItem(int(labelAtts->GetLabelDisplayFormat()));
            labelDisplayFormatComboBox->blockSignals(false);
            break;
        case 9: //numberOfLabels
            numberOfLabelsSpinBox->blockSignals(true);
            numberOfLabelsSpinBox->setValue(labelAtts->GetNumberOfLabels());
            numberOfLabelsSpinBox->blockSignals(false);
            break;
        case 10: //textColor
            tempcolor = QColor(labelAtts->GetTextColor().Red(),
                               labelAtts->GetTextColor().Green(),
                               labelAtts->GetTextColor().Blue());
            textColorButton->setButtonColor(tempcolor);
            break;
        case 11: //textHeight
            textHeightSpinBox->blockSignals(true);
            textHeightSpinBox->setValue(int(labelAtts->GetTextHeight() * 100.f + 0.5f));
            textHeightSpinBox->blockSignals(false);
            break;
        case 12: //textLabel
            markerTextLineEdit->setText(labelAtts->GetTextLabel().c_str());
            break;
        case 13: //horizontalJustification
            horizontalJustificationComboBox->blockSignals(true);
            horizontalJustificationComboBox->setCurrentItem(int(labelAtts->
                GetHorizontalJustification()));
            horizontalJustificationComboBox->blockSignals(false);
            break;
        case 14: //verticalJustification
            verticalJustificationComboBox->blockSignals(true);
            verticalJustificationComboBox->setCurrentItem(int(labelAtts->
                GetVerticalJustification()));
            verticalJustificationComboBox->blockSignals(false);
            break;
        case 15: //singleNodeIndex
            temp.sprintf("%d", labelAtts->GetSingleNodeIndex());
            singleNodeLineEdit->setText(temp);
            break;
        case 16: //singleCellIndex
            temp.sprintf("%d", labelAtts->GetSingleCellIndex());
            singleCellLineEdit->setText(temp);
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
//   
// ****************************************************************************

void
QvisLabelPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do singleNodeIndex
    if(which_widget == 0 || doAll)
    {
        temp = singleNodeLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            labelAtts->SetSingleNodeIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value entered for the single node index was invalid. "
                "Resetting to the last good value of %d.",
                labelAtts->GetSingleNodeIndex());
            Message(msg);
            labelAtts->SetSingleNodeIndex(labelAtts->GetSingleNodeIndex());
        }
    }

    // Do singleCellIndex
    if(which_widget == 1 || doAll)
    {
        temp = singleCellLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            labelAtts->SetSingleCellIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of singleCellIndex was invalid. "
                "Resetting to the last good value of %d.",
                labelAtts->GetSingleCellIndex());
            Message(msg);
            labelAtts->SetSingleCellIndex(labelAtts->GetSingleCellIndex());
        }
    }

    // Do textLabel
    if(which_widget == 2 || doAll)
    {
        temp = markerTextLineEdit->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            labelAtts->SetTextLabel(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of textLabel was invalid. "
                "Resetting to the last good value of %s.",
                labelAtts->GetTextLabel().c_str());
            Message(msg);
            labelAtts->SetTextLabel(labelAtts->GetTextLabel());
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

        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
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
    viewer->ResetPlotOptions(plotType);
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
QvisLabelPlotWindow::textColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    labelAtts->SetTextColor(temp);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::useForegroundTextColorToggled(bool val)
{
    labelAtts->SetUseForegroundTextColor(val);
    Apply();
}

void
QvisLabelPlotWindow::textHeightChanged(int val)
{
    float textHeight = float(val) * 0.01f;
    labelAtts->SetTextHeight(textHeight);
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
QvisLabelPlotWindow::showSingleNodeToggled(bool val)
{
    labelAtts->SetShowSingleNode(val);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::singleNodeProcessText()
{
    GetCurrentValues(0);
    Apply();
}

void
QvisLabelPlotWindow::showSingleCellToggled(bool val)
{
    labelAtts->SetShowSingleCell(val);
    SetUpdate(false);
    Apply();
}


void
QvisLabelPlotWindow::singleCellProcessText()
{
    GetCurrentValues(1);
    Apply();
}

void
QvisLabelPlotWindow::markerProcessText()
{
    GetCurrentValues(2);
    Apply();
}

