// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisLabelPlotWindow.h>

#include <LabelAttributes.h>
#include <ViewerProxy.h>
#include <StringHelpers.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QvisFontAttributesWidget.h>


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
    LabelAttributes *subj, const QString &caption, const QString &shortName,
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
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Set parent for depthTestButtonGroup, we can avoid explicit delete. 
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
//   Brad Whitlock, Tue Aug 2 14:50:32 PST 2005
//   I removed the single cell/node controls. I also added controls for
//   zbuffering and additional controls for changing label color and height
//   for meshes.
//
//   Dave Bremer, Wed Sep 19 19:39:40 PDT 2007
//   I added a line for specifying a printf-style template.
//
//   Brad Whitlock, Tue Apr 22 16:54:16 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Hank Childs, Wed Oct 20 11:04:12 PDT 2010
//   Change text heights from spin box to "double spin box".
//
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisLabelPlotWindow::CreateWindowContents()
{
    //
    // Create label selection-related widgets.
    //
    selectionGroupBox = new QGroupBox(central);
    selectionGroupBox->setTitle(tr("Selection"));
    topLayout->addWidget(selectionGroupBox);
    QVBoxLayout *selTopLayout = new QVBoxLayout(selectionGroupBox);

    QGridLayout *selLayout = new QGridLayout();
    selTopLayout->addLayout(selLayout);

    showNodesToggle = new QCheckBox(tr("Show nodes"), selectionGroupBox);
    connect(showNodesToggle, SIGNAL(toggled(bool)),
            this, SLOT(showNodesToggled(bool)));
    selLayout->addWidget(showNodesToggle, 0,0);

    showCellsToggle = new QCheckBox(tr("Show cells"), selectionGroupBox);
    connect(showCellsToggle, SIGNAL(toggled(bool)),
            this, SLOT(showCellsToggled(bool)));
    selLayout->addWidget(showCellsToggle, 0,1);

    restrictNumberOfLabelsToggle = new QCheckBox(tr("Restrict number of labels to"));
    connect(restrictNumberOfLabelsToggle, SIGNAL(toggled(bool)),
            this, SLOT(restrictNumberOfLabelsToggled(bool)));
    selLayout->addWidget(restrictNumberOfLabelsToggle, 1,0,1,2);

    numberOfLabelsSpinBox = new QSpinBox(selectionGroupBox);
    numberOfLabelsSpinBox->setKeyboardTracking(false);
    numberOfLabelsSpinBox->setRange(1, 200000);
    numberOfLabelsSpinBox->setSingleStep(200);
    connect(numberOfLabelsSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(numberOfLabelsChanged(int)));
    selLayout->addWidget(numberOfLabelsSpinBox, 1, 2);

    drawLabelsFacingComboBox = new QComboBox(selectionGroupBox);
    drawLabelsFacingComboBox->addItem(tr("Front"));
    drawLabelsFacingComboBox->addItem(tr("Back"));
    drawLabelsFacingComboBox->addItem(tr("Front or Back"));
    connect(drawLabelsFacingComboBox, SIGNAL(activated(int)),
            this, SLOT(drawLabelsFacingChanged(int)));
    selLayout->addWidget(drawLabelsFacingComboBox, 2, 2);
    selLayout->addWidget(new QLabel(tr("Draw labels that face"),selectionGroupBox)
                         , 2, 0, 1, 2);

    depthTestButtonGroup = new QButtonGroup(selectionGroupBox);
    
    QHBoxLayout *dtLayout = new QHBoxLayout();
    
    QRadioButton *rb = new QRadioButton(tr("Auto"), selectionGroupBox);
    depthTestButtonGroup->addButton(rb, 0);
    dtLayout->addWidget(rb);
    
    rb = new QRadioButton(tr("Always"), selectionGroupBox);
    depthTestButtonGroup->addButton(rb, 1);
    dtLayout->addWidget(rb);
    
    rb = new QRadioButton(tr("Never"), selectionGroupBox);
    depthTestButtonGroup->addButton(rb, 2);
    dtLayout->addWidget(rb);
    
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(depthTestButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(depthTestButtonGroupChanged(int)));
#else
    connect(depthTestButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(depthTestButtonGroupChanged(int)));
#endif
    selLayout->addWidget(new QLabel(tr("Depth test mode"), selectionGroupBox), 3, 0);
    selLayout->addLayout(dtLayout, 3, 1, 1, 2);

    //
    // Create formatting widgets
    //
    formattingGroupBox = new QGroupBox(central);
    formattingGroupBox->setTitle(tr("Formatting"));
    topLayout->addWidget(formattingGroupBox);
    QVBoxLayout *fmtTopLayout = new QVBoxLayout(formattingGroupBox);
    QGridLayout *fmtLayout = new QGridLayout();
    fmtTopLayout->addLayout(fmtLayout);

    labelDisplayFormatComboBox = new QComboBox(formattingGroupBox);
    labelDisplayFormatComboBox->addItem(tr("Natural"));
    labelDisplayFormatComboBox->addItem(tr("Logical index"));
    labelDisplayFormatComboBox->addItem(tr("Index"));
    connect(labelDisplayFormatComboBox, SIGNAL(activated(int)),
            this, SLOT(labelDisplayFormatChanged(int)));
    fmtLayout->addWidget(labelDisplayFormatComboBox, 0, 1);
    fmtLayout->addWidget(new QLabel(tr("Label display format"), 
                                    formattingGroupBox), 0, 0);


    cellFrame = new QFrame(formattingGroupBox);
    cellFrame->setFrameStyle(QFrame::HLine);
    fmtLayout->addWidget(cellFrame, 1, 0, 1, 2);
    cellLabel = new QLabel(tr("Cell labels"), formattingGroupBox);
    fmtLayout->addWidget(cellLabel, 2, 0);

    textFont1 = new QvisFontAttributesWidget(formattingGroupBox);
    textFont1->hideOpacity();
    connect(textFont1, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(textFont1Changed(const FontAttributes &)));
    fmtLayout->addWidget(textFont1, 3, 0, 1, 2);

    nodeFrame = new QFrame(formattingGroupBox);
    nodeFrame->setFrameStyle(QFrame::HLine);
    fmtLayout->addWidget(nodeFrame, 4, 0, 1, 2);
    nodeLabel = new QLabel(tr("Node labels"), formattingGroupBox);
    fmtLayout->addWidget(nodeLabel, 5, 0);


    textFont2 = new QvisFontAttributesWidget(formattingGroupBox);
    textFont2->hideOpacity();
    connect(textFont2, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(textFont2Changed(const FontAttributes &)));
    fmtLayout->addWidget(textFont2, 6, 0, 1, 2);

    bottomFrame = new QFrame(formattingGroupBox);
    bottomFrame->setFrameStyle(QFrame::HLine);
    fmtLayout->addWidget(bottomFrame, 7, 0, 1, 2);

    horizontalJustificationComboBox = new QComboBox(formattingGroupBox);
    horizontalJustificationComboBox->addItem(tr("Center"));
    horizontalJustificationComboBox->addItem(tr("Left"));
    horizontalJustificationComboBox->addItem(tr("Right"));
    connect(horizontalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(horizontalJustificationChanged(int)));
    fmtLayout->addWidget(horizontalJustificationComboBox, 8, 1);
    fmtLayout->addWidget(new QLabel(tr("Horizontal justification"), 
                                    formattingGroupBox), 8, 0);

    verticalJustificationComboBox = new QComboBox(formattingGroupBox);
    verticalJustificationComboBox->addItem(tr("Center"));
    verticalJustificationComboBox->addItem(tr("Top"));
    verticalJustificationComboBox->addItem(tr("Bottom"));
    connect(verticalJustificationComboBox, SIGNAL(activated(int)),
            this, SLOT(verticalJustificationChanged(int)));
    fmtLayout->addWidget(verticalJustificationComboBox, 9, 1);
    fmtLayout->addWidget(new QLabel(tr("Vertical justification"), 
                                    formattingGroupBox), 9, 0);

    
    formatTemplate = new QLineEdit(QString(labelAtts->GetFormatTemplate().c_str()), 
                                   formattingGroupBox);
    connect(formatTemplate, SIGNAL(returnPressed()), 
            this, SLOT(formatTemplateChanged()));

    fmtLayout->addWidget(formatTemplate, 10, 1);
    fmtLayout->addWidget(new QLabel(tr("Format template"), formattingGroupBox), 10, 0);

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
//   Dave Bremer, Wed Sep 19 19:39:40 PDT 2007
//   I added a code for updating a printf-style template.
//
//   Brad Whitlock, Tue Apr 22 16:56:14 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Hank Childs, Wed Oct 20 11:04:12 PDT 2010
//   Change text heights from spin box to "double spin box".
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
        case LabelAttributes::ID_varType:
            {// new scope
            bool varIsMesh = labelAtts->GetVarType() == LabelAttributes::LABEL_VT_MESH;
            showNodesToggle->setEnabled(varIsMesh);
            showCellsToggle->setEnabled(varIsMesh);

            if(varIsMesh)
            {
                textFont2->show();
                cellFrame->show();
                cellLabel->show();
                nodeFrame->show();
                nodeLabel->show();
                bottomFrame->show();
                updateGeometry();
            }
            else
            {
                textFont2->hide();
                cellFrame->hide();
                cellLabel->hide();
                nodeFrame->hide();
                nodeLabel->hide();
                bottomFrame->hide();
                updateGeometry();
            }
            }
            break;
        case LabelAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(labelAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case LabelAttributes::ID_showNodes:
            showNodesToggle->blockSignals(true);
            showNodesToggle->setChecked(labelAtts->GetShowNodes());
            showNodesToggle->blockSignals(false);
            break;
        case LabelAttributes::ID_showCells:
            showCellsToggle->blockSignals(true);
            showCellsToggle->setChecked(labelAtts->GetShowCells());
            showCellsToggle->blockSignals(false);
            break;
        case LabelAttributes::ID_restrictNumberOfLabels:
            restrictNumberOfLabelsToggle->blockSignals(true);
            restrictNumberOfLabelsToggle->setChecked(labelAtts->GetRestrictNumberOfLabels());
            restrictNumberOfLabelsToggle->blockSignals(false);
            numberOfLabelsSpinBox->setEnabled(labelAtts->GetRestrictNumberOfLabels());
            break;
        case LabelAttributes::ID_drawLabelsFacing:
            drawLabelsFacingComboBox->blockSignals(true);
            drawLabelsFacingComboBox->setCurrentIndex(int(labelAtts->GetDrawLabelsFacing()));
            drawLabelsFacingComboBox->blockSignals(false);
            break;
        case LabelAttributes::ID_labelDisplayFormat:
            labelDisplayFormatComboBox->blockSignals(true);
            labelDisplayFormatComboBox->setCurrentIndex(int(labelAtts->GetLabelDisplayFormat()));
            labelDisplayFormatComboBox->blockSignals(false);
            break;
        case LabelAttributes::ID_numberOfLabels:
            numberOfLabelsSpinBox->blockSignals(true);
            numberOfLabelsSpinBox->setValue(labelAtts->GetNumberOfLabels());
            numberOfLabelsSpinBox->blockSignals(false);
            break;
        case LabelAttributes::ID_textFont1:
             {
             textFont1->setFontAttributes(labelAtts->GetTextFont1());
             }
            break;
        case LabelAttributes::ID_textFont2:
             {
             textFont2->setFontAttributes(labelAtts->GetTextFont2());
             }
            break;
        case LabelAttributes::ID_horizontalJustification:
            horizontalJustificationComboBox->blockSignals(true);
            horizontalJustificationComboBox->setCurrentIndex(int(labelAtts->
                GetHorizontalJustification()));
            horizontalJustificationComboBox->blockSignals(false);
            break;
        case LabelAttributes::ID_verticalJustification:
            verticalJustificationComboBox->blockSignals(true);
            verticalJustificationComboBox->setCurrentIndex(int(labelAtts->
                GetVerticalJustification()));
            verticalJustificationComboBox->blockSignals(false);
            break;
        case LabelAttributes::ID_depthTestMode:
            depthTestButtonGroup->blockSignals(true);
            depthTestButtonGroup->button(int(labelAtts->GetDepthTestMode()))->setChecked(true);
            depthTestButtonGroup->blockSignals(false);
            break;
        case LabelAttributes::ID_formatTemplate:
            formatTemplate->blockSignals(true);
            formatTemplate->setText(QString(labelAtts->GetFormatTemplate().c_str()));
            formatTemplate->blockSignals(false);
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
//   Brad Whitlock, Tue Apr 22 16:58:45 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Eric Brugger, Tue Aug 24 10:42:22 PDT 2010
//   I added code for formatTemplate. 
//
//   Hank Childs, Wed Oct 20 11:04:12 PDT 2010
//   Change text heights from spin box to "double spin box".
//
// ****************************************************************************

void
QvisLabelPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do numberOfLabels
    if(which_widget == LabelAttributes::ID_numberOfLabels || doAll)
    {
        temp = numberOfLabelsSpinBox->text().trimmed();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                labelAtts->SetNumberOfLabels(val);
        }

        if(!okay)
        {
            ResettingError(tr("number of labels"),
                IntToQString(labelAtts->GetNumberOfLabels()));
            labelAtts->SetNumberOfLabels(labelAtts->GetNumberOfLabels());
        }
    }

    // Do textFont1
    if(which_widget == LabelAttributes::ID_textFont1  || doAll)
    {
        labelAtts->SetTextFont1(textFont1->getFontAttributes());
    }

    // Do textFont2
    if(which_widget == LabelAttributes::ID_textFont2  || doAll)
    {
        labelAtts->SetTextFont2(textFont2->getFontAttributes());
    }

    // Do formatTemplate
    if(which_widget == LabelAttributes::ID_formatTemplate || doAll)
    {
        okay = true;

        std::string temp = formatTemplate->displayText().trimmed().toStdString();

        // Test the new value and don't apply it if it's an invalid printf
        // string. In practice snprintf never throws an error for wrong
        // type or number of %f slots.
        if (!StringHelpers::ValidatePrintfFormatString(temp.c_str(), "float"))
        {
            Message(tr("Must enter a printf-style template that would be valid for a single floating point number."));
            okay = false;
        }

        if (okay)
        {
            char test[36];
            int len = snprintf(test, 36, temp.c_str(), 0.0f);
            if (len >= 35)
            {
                Message(tr("The template produces values that are too long.  36 character limit."));
                okay = false;
            }
        }

        if(okay)
            labelAtts->SetFormatTemplate(temp);
        else
        {
            ResettingError(tr("Format template"),
                QString(labelAtts->GetFormatTemplate().c_str()));
            labelAtts->SetFormatTemplate(labelAtts->GetFormatTemplate());
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
QvisLabelPlotWindow::textFont1Changed(const FontAttributes &f)
{
    labelAtts->SetTextFont1(f);
    SetUpdate(false);
    Apply();
}

void
QvisLabelPlotWindow::textFont2Changed(const FontAttributes &f)
{
    labelAtts->SetTextFont2(f);
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


void 
QvisLabelPlotWindow::formatTemplateChanged()
{
    bool okay = true;

    std::string newval = formatTemplate->displayText().trimmed().toStdString();

    //Test the new value and don't apply it if it's an invalid printf string.
    //In practice snprintf never throws an error for wrong type or number
    //of %f slots.
    if (!StringHelpers::ValidatePrintfFormatString(newval.c_str(), "float"))
    {
        Message(tr("Must enter a printf-style template that would be valid for a single floating point number."));
        okay = false;
    }
    
    if (okay)
    {
        char test[36];
        int len = snprintf(test, 36, newval.c_str(), 0.0f);
        if (len >= 35)
        {
            Message(tr("The template produces values that are too long.  36 character limit."));
            okay = false;
        }
    }

    if(okay)
    {
        labelAtts->SetFormatTemplate(newval);
        SetUpdate(false);
        Apply();
    }
    else
    {
        ResettingError(tr("Format template"),
            QString(labelAtts->GetFormatTemplate().c_str()));
        labelAtts->SetFormatTemplate(labelAtts->GetFormatTemplate());
        labelAtts->Notify();
    }
}



