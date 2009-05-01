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
#include <QvisAppearanceWidget.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

#include <QvisColorButton.h>
#include <QvisColorManagerWidget.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>

#include <VisItViewer.h>

#include <AttributeSubject.h>
#include <DataNode.h>

// ****************************************************************************
// Method: QvisAppearanceWidget::QvisAppearanceWidget
//
// Purpose: 
//   Constructor.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//
// ****************************************************************************

QvisAppearanceWidget::QvisAppearanceWidget(QWidget *parent, VisItViewer *v) :
    QWidget(parent)
{
    viewer = v;

    CreateWindow();
}

// ****************************************************************************
// Method: QvisAppearanceWidget::~QvisAppearanceWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//   
// ****************************************************************************

QvisAppearanceWidget::~QvisAppearanceWidget()
{
}

// ****************************************************************************
// Method: QvisAppearanceWidget::Update
//
// Purpose: 
//   Update the widgets associated with the specified subject.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWidget::Update(Subject *subject)
{
    if (subject == multiCurveAtts)
        UpdateMultiCurveWidgets();
}

// ****************************************************************************
// Method: QvisAppearanceWidget::SubjectRemoved
//
// Purpose: 
//   Tell the window that the subject being observed is no longer valid.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWidget::SubjectRemoved(Subject *subject)
{
    if (subject == multiCurveAtts)
       multiCurveAtts = 0;
}

// ****************************************************************************
// Method: QvisAppearanceWidget::ConnectMultiCurveAttributes
//
// Purpose:
//   Makes the window observe the multi curve attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAppearanceWidget::ConnectMultiCurveAttributes(AttributeSubject *m)
{
    multiCurveAtts = m;
    m->Attach(this);
}

// ****************************************************************************
// Method: QvisAppearanceWidget::CreateWindow
//
// Purpose:
//   Create the contents of the window.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAppearanceWidget::CreateWindow()
{
    QVBoxLayout *aLayout = new QVBoxLayout(this);
    aLayout->setMargin(5);
    aLayout->setSpacing(10);

    // Create the curve color group box.
    curveColorGroup = new QGroupBox(this);
    curveColorGroup->setTitle(tr("Curve colors"));
    aLayout->addWidget(curveColorGroup);

    // Create the mode buttons that determine if the window is in single
    // or multiple color mode.
    colorModeButtons = new QButtonGroup(0);
    connect(colorModeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
    QGridLayout *colorLayout = new QGridLayout(curveColorGroup);
    colorLayout->setSpacing(10);
    colorLayout->setColumnStretch(2, 1000);

    QRadioButton *rb = new QRadioButton(tr("Single"), curveColorGroup);
    colorModeButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Multiple"), curveColorGroup);
    colorModeButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 1, 0);

    // Create the single color button.
    singleColor = new QvisColorButton(curveColorGroup);
    singleColor->setButtonColor(QColor(255, 0, 0));
    connect(singleColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(singleColorChanged(const QColor &)));
    colorLayout->addWidget(singleColor, 0, 1);

    // Create the single color opacity.
    singleColorOpacity = new QvisOpacitySlider(0,255,64,0,curveColorGroup);
    connect(singleColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(singleColorOpacityChanged(int)));
    colorLayout->addWidget(singleColorOpacity, 0, 2);

    // Add the multiple colors widget.
    multipleColors = new QvisColorManagerWidget(curveColorGroup);
    multipleColors->setNameLabelText(tr("Curve"));
    connect(multipleColors, SIGNAL(colorChanged(const QColor &, int)),
            this, SLOT(multipleColorChanged(const QColor &, int)));
    connect(multipleColors, SIGNAL(opacityChanged(int, int)),
            this, SLOT(opacityChanged(int, int)));
    colorLayout->addWidget(multipleColors, 2, 0, 1, 3);

    // Create a layout for the remaining widgets.
    QGridLayout *mainLayout = new QGridLayout(0);
    aLayout->addLayout(mainLayout);

    // Add the line style widget.
    lineStyleLabel = new QLabel(tr("Line Style"), this);
    mainLayout->addWidget(lineStyleLabel,0,0);
    lineStyle = new QvisLineStyleWidget(0, this);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyle, 0,1);

    // Add the line width widget.
    lineWidthLabel = new QLabel(tr("Line Width"), this);
    mainLayout->addWidget(lineWidthLabel,1,0);
    lineWidth = new QvisLineWidthWidget(0, this);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidth, 1,1);

    // Add the y axis title format widget.
    yAxisTitleFormatLabel = new QLabel(tr("Y-Axis title format"), this);
    mainLayout->addWidget(yAxisTitleFormatLabel,2,0);
    yAxisTitleFormat = new QLineEdit(this);
    connect(yAxisTitleFormat, SIGNAL(returnPressed()),
            this, SLOT(yAxisTitleFormatProcessText()));
    mainLayout->addWidget(yAxisTitleFormat, 2,1);

    // Add the use y range widget.
    useYAxisTickSpacing = new QCheckBox(tr("Y-Axis tick spacing"), this);
    connect(useYAxisTickSpacing, SIGNAL(toggled(bool)),
            this, SLOT(useYAxisTickSpacingChanged(bool)));
    mainLayout->addWidget(useYAxisTickSpacing, 3,0);

    // Add the y range widget.
    yAxisTickSpacing = new QLineEdit(this);
    connect(yAxisTickSpacing, SIGNAL(returnPressed()),
            this, SLOT(yAxisTickSpacingProcessText()));
    mainLayout->addWidget(yAxisTickSpacing, 3,1);

    // Add the display markers widget.
    displayMarkers = new QCheckBox(tr("Display markers"), this);
    connect(displayMarkers, SIGNAL(toggled(bool)),
            this, SLOT(displayMarkersChanged(bool)));
    mainLayout->addWidget(displayMarkers, 4,0);

    // Add the display ids widget.
    displayIds = new QCheckBox(tr("Display ids"), this);
    connect(displayIds, SIGNAL(toggled(bool)),
            this, SLOT(displayIdsChanged(bool)));
    mainLayout->addWidget(displayIds, 5,0);

    // Add the legend widget.
    legend = new QCheckBox(tr("Legend"), this);
    connect(legend, SIGNAL(toggled(bool)),
            this, SLOT(legendChanged(bool)));
    mainLayout->addWidget(legend, 6,0);
}

// ****************************************************************************
// Method: QvisAppearanceWidget::UpdateMultiCurveWidgets
//
// Purpose:
//   Update the widgets associated with the multi curve attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 17:52:46 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAppearanceWidget::UpdateMultiCurveWidgets()
{
    // Get a data node representation of the multi curve attributes.
    DataNode root("root");
    multiCurveAtts->CreateNode(&root, true, false);
    DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");

    // Update the color type.
    DataNode *colorTypeNode = multiCurveNode->GetNode("colorType");
    int colorType = 1;
    if (colorTypeNode->AsString() == "ColorBySingleColor")
        colorType = 0;
    else if (colorTypeNode->AsString() == "ColorByMultipleColors")
        colorType = 1;
    colorModeButtons->blockSignals(true);
    colorModeButtons->button(colorType)->setChecked(true);
    colorModeButtons->blockSignals(false);

    // Update the single color.
    DataNode *singleColorNode = multiCurveNode->GetNode("singleColor");
    DataNode *colorAttributeNode = singleColorNode->GetNode("ColorAttribute");
    DataNode *colorNode = colorAttributeNode->GetNode("color");
    const unsigned char *color = colorNode->AsUnsignedCharArray();
    QColor tempColor = QColor((int)color[0],
                              (int)color[1],
                              (int)color[2]);
    singleColor->blockSignals(true);
    singleColor->setButtonColor(tempColor);
    singleColor->blockSignals(false);

    singleColorOpacity->blockSignals(true);
    singleColorOpacity->setValue((int)color[3]);
    singleColorOpacity->blockSignals(false);

    // Update the multi color.
    DataNode *multiColorNode = multiCurveNode->GetNode("multiColor");
    DataNode *calNode = multiColorNode->GetNode("ColorAttributeList");
    DataNode **children = calNode->GetChildren();
    int nEntries = calNode->GetNumChildren();

    multipleColors->blockSignals(true);

    if (nEntries == multipleColors->numEntries())
    {
        for (int i = 0; i < nEntries; ++i)
        {
            DataNode *colorNode = children[i]->GetNode("color");
            const unsigned char *color = colorNode->AsUnsignedCharArray();

            QColor temp((int)color[0], (int)color[1], (int)color[2]);

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, (int)color[3]);
        }
    }
    else if (nEntries > multipleColors->numEntries())
    {
        // Set all of the existing colors.
        for (int i = 0; i < multipleColors->numEntries(); ++i)
        {
            DataNode *colorNode = children[i]->GetNode("color");
            const unsigned char *color = colorNode->AsUnsignedCharArray();

            QColor temp((int)color[0], (int)color[1], (int)color[2]);

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, (int)color[3]);
        }

        // Add new entries
        for (int i = multipleColors->numEntries(); i < nEntries; ++i)
        {
            DataNode *colorNode = children[i]->GetNode("color");
            const unsigned char *color = colorNode->AsUnsignedCharArray();

            QColor temp((int)color[0], (int)color[1], (int)color[2]);

            multipleColors->addEntry(QString(""), temp, (int)color[3]);
        }
    }
    else // nEntries < multipleColors->numEntries()
    {
        // Set all of the existing names.
        for (int i = 0; i < nEntries; ++i)
        {
            DataNode *colorNode = children[i]->GetNode("color");
            const unsigned char *color = colorNode->AsUnsignedCharArray();

            QColor temp((int)color[0], (int)color[1], (int)color[2]);

            multipleColors->setColor(i, temp);
            multipleColors->setOpacity(i, (int)color[3]);
        }

        // Remove excess entries.
        int numEntries = multipleColors->numEntries();
        for (int i = nEntries; i < numEntries; ++i)
        {
            multipleColors->removeLastEntry();
        }
    }

    multipleColors->blockSignals(false);

    // Update the line style.
    DataNode *lineStyleNode = multiCurveNode->GetNode("lineStyle");
    lineStyle->blockSignals(true);
    lineStyle->SetLineStyle(lineStyleNode->AsInt());
    lineStyle->blockSignals(false);

    // Update the line width.
    DataNode *lineWidthNode = multiCurveNode->GetNode("lineWidth");
    lineWidth->blockSignals(true);
    lineWidth->SetLineWidth(lineWidthNode->AsInt());
    lineWidth->blockSignals(false);

    // Update the y axis title format.
    DataNode *yAxisTitleFormatNode = multiCurveNode->GetNode("yAxisTitleFormat");
    yAxisTitleFormat->setText(QString(yAxisTitleFormatNode->AsString().c_str()));

    // Update the use y axis range.
    DataNode *useYAxisTickSpacingNode = multiCurveNode->GetNode("useYAxisTickSpacing");
    if (useYAxisTickSpacingNode->AsBool() == true)
    {
        yAxisTickSpacing->setEnabled(true);
    }
    else
    {
        yAxisTickSpacing->setEnabled(false);
    }
    useYAxisTickSpacing->blockSignals(true);
    useYAxisTickSpacing->setChecked(useYAxisTickSpacingNode->AsBool());
    useYAxisTickSpacing->blockSignals(false);

    // Update the y axis range.
    DataNode *yAxisTickSpacingNode = multiCurveNode->GetNode("yAxisTickSpacing");
    yAxisTickSpacing->setText(QString().setNum(yAxisTickSpacingNode->AsDouble()));

    // Update the display markers.
    DataNode *displayMarkersNode = multiCurveNode->GetNode("displayMarkers");
    displayMarkers->blockSignals(true);
    displayMarkers->setChecked(displayMarkersNode->AsBool());
    displayMarkers->blockSignals(false);

    // Update the display ids.
    DataNode *displayIdsNode = multiCurveNode->GetNode("displayIds");
    displayIds->blockSignals(true);
    displayIds->setChecked(displayIdsNode->AsBool());
    displayIds->blockSignals(false);

    // Update the legend.
    DataNode *legendFlagNode = multiCurveNode->GetNode("legendFlag");
    legend->blockSignals(true);
    legend->setChecked(legendFlagNode->AsBool());
    legend->blockSignals(false);
}

bool
QvisAppearanceWidget::LineEditGetDouble(QLineEdit *lineEdit, double &val)
{
    bool okay = false;
    val = lineEdit->displayText().trimmed().toDouble(&okay);
    return okay;
}

//
// Qt slot functions
//
void
QvisAppearanceWidget::colorModeChanged(int index)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("colorType", index);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::singleColorChanged(const QColor &color)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        DataNode root("root");
        atts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");
        DataNode *singleColorNode = multiCurveNode->GetNode("singleColor");
        DataNode *colorAttributeNode = singleColorNode->GetNode("ColorAttribute");
        DataNode *colorNode = colorAttributeNode->GetNode("color");
        const unsigned char *oldColor = colorNode->AsUnsignedCharArray();
        unsigned char newColor[4];
        newColor[0] = (unsigned char)color.red();
        newColor[1] = (unsigned char)color.green();
        newColor[2] = (unsigned char)color.blue();
        newColor[3] = oldColor[3];
        DataNode *newColorNode = new DataNode("color", newColor, 4);
        colorAttributeNode->RemoveNode(colorNode);
        colorAttributeNode->AddNode(newColorNode);
        atts->SetFromNode(&root);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::singleColorOpacityChanged(int opacity)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        DataNode root("root");
        atts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");
        DataNode *singleColorNode = multiCurveNode->GetNode("singleColor");
        DataNode *colorAttributeNode = singleColorNode->GetNode("ColorAttribute");
        DataNode *colorNode = colorAttributeNode->GetNode("color");
        const unsigned char *oldColor = colorNode->AsUnsignedCharArray();
        unsigned char newColor[4];
        newColor[0] = oldColor[0];
        newColor[1] = oldColor[1];
        newColor[2] = oldColor[2];
        newColor[3] = (unsigned char)opacity;
        DataNode *newColorNode = new DataNode("color", newColor, 4);
        colorAttributeNode->RemoveNode(colorNode);
        colorAttributeNode->AddNode(newColorNode);
        atts->SetFromNode(&root);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::multipleColorChanged(const QColor &color, int index)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        DataNode root("root");
        atts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");
        DataNode *multiColorNode = multiCurveNode->GetNode("multiColor");
        DataNode *calNode = multiColorNode->GetNode("ColorAttributeList");
        DataNode **children = calNode->GetChildren();

        DataNode *colorNode = children[index]->GetNode("color");
        const unsigned char *oldColor = colorNode->AsUnsignedCharArray();
        unsigned char newColor[4];
        newColor[0] = (unsigned char)color.red();
        newColor[1] = (unsigned char)color.green();
        newColor[2] = (unsigned char)color.blue();
        newColor[3] = oldColor[3];
        colorNode->SetUnsignedCharArray(newColor, 4);

        atts->SetFromNode(&root);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::opacityChanged(int opacity, int index)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        DataNode root("root");
        atts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");
        DataNode *multiColorNode = multiCurveNode->GetNode("multiColor");
        DataNode *calNode = multiColorNode->GetNode("ColorAttributeList");
        DataNode **children = calNode->GetChildren();

        DataNode *colorNode = children[index]->GetNode("color");
        const unsigned char *oldColor = colorNode->AsUnsignedCharArray();
        unsigned char newColor[4];
        newColor[0] = oldColor[0];
        newColor[1] = oldColor[1];
        newColor[2] = oldColor[2];
        newColor[3] = (unsigned char)opacity;
        colorNode->SetUnsignedCharArray(newColor, 4);

        atts->SetFromNode(&root);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::lineStyleChanged(int style)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("lineStyle", style);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::lineWidthChanged(int style)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("lineWidth", style);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::yAxisTitleFormatProcessText()
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        QString temp = yAxisTitleFormat->displayText();
        atts->SetValue("yAxisTitleFormat", temp.toStdString());
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::useYAxisTickSpacingChanged(bool val)
{
    if (val)
        yAxisTickSpacing->setEnabled(true);
    else
        yAxisTickSpacing->setEnabled(false);

    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("useYAxisTickSpacing", val);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::yAxisTickSpacingProcessText()
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(yAxisTickSpacing, val);
        if (okay)
        {
            atts->SetValue("yAxisTickSpacing", val);
            atts->Notify();
            emit multiCurveChanged(atts);
        }
    }
}

void
QvisAppearanceWidget::displayMarkersChanged(bool val)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("displayMarkers", val);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::displayIdsChanged(bool val)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("displayIds", val);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}

void
QvisAppearanceWidget::legendChanged(bool val)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *atts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(atts != 0)
    {
        atts->SetValue("legendFlag", val);
        atts->Notify();
        emit multiCurveChanged(atts);
    }
}
