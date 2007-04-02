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

#include <QvisLegendAttributesInterface.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <QNarrowLineEdit.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>
#include <Plot.h>
#include <PlotList.h>
#include <ViewerState.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>

#define WIDTH_HEIGHT_PRECISION 100.

// Allow this when text opacity is supported.
//#define TEXT_OPACITY_SUPPORTED

// ****************************************************************************
// Method: QvisLegendAttributesInterface::QvisLegendAttributesInterface
//
// Purpose: 
//   Constructor for the QvisLegendAttributesInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 15:31:44 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisLegendAttributesInterface::QvisLegendAttributesInterface(QWidget *parent,
    const char *name) : QvisAnnotationObjectInterface(parent, name)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    int row = 0;
    QGridLayout *cLayout = new QGridLayout(topLayout, 11, 4);
    cLayout->setSpacing(10);

    // Add controls for the layout management.
    manageLayout = new QCheckBox("Let VisIt manage legend position", this, "manageLayout");
    connect(manageLayout, SIGNAL(toggled(bool)),
            this, SLOT(layoutChanged(bool)));
    cLayout->addMultiCellWidget(manageLayout, row, row, 0, 3);
    ++row;

    // Add controls for the position
    positionEdit = new QvisScreenPositionEdit(this, "positionEdit");
    connect(positionEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionChanged(double, double)));
    cLayout->addMultiCellWidget(positionEdit, row, row, 1, 3);
    positionLabel = new QLabel(positionEdit, "Legend position", this);
    cLayout->addWidget(positionLabel, row, 0);
    ++row;

    // Add controls for position2
    widthSpinBox = new QSpinBox(1, int(WIDTH_HEIGHT_PRECISION * 5), 1, this, "widthSpinBox");
    widthSpinBox->setSuffix("%");
    widthSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthSpinBox, row, 1);
    cLayout->addWidget(new QLabel(widthSpinBox, "X-scale",
        this), row, 0);

    heightSpinBox = new QSpinBox(1, int(WIDTH_HEIGHT_PRECISION * 5), 1, this, "heightSpinBox");
    heightSpinBox->setSuffix("%");
    heightSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(heightChanged(int)));
    cLayout->addWidget(heightSpinBox, row, 3);
    cLayout->addWidget(new QLabel(widthSpinBox, "Y-scale",
        this), row, 2);
    ++row;

    QFrame *splitter1 = new QFrame(this, "splitter");
    splitter1->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter1, row, row, 0, 3);  
    ++row;

    // Add controls for the text color.
    drawBoundingBoxCheckBox = new QCheckBox("Bounding box", this,
        "drawBoundingBoxCheckBox");
    connect(drawBoundingBoxCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawBoundingBoxToggled(bool)));
    cLayout->addWidget(drawBoundingBoxCheckBox, row, 0);

    boundingBoxColorButton = new QvisColorButton(this, "boundingBoxColorButton");
    connect(boundingBoxColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(boundingBoxColorChanged(const QColor &)));
    cLayout->addWidget(boundingBoxColorButton, row, 1);
    boundingBoxOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "boundingBoxOpacity");
    connect(boundingBoxOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(boundingBoxOpacityChanged(int)));
    cLayout->addMultiCellWidget(boundingBoxOpacity, row, row, 2, 3);
    ++row;


    drawLabelsCheckBox = new QCheckBox("Draw labels", this, "drawLabelsCheckBox");
    connect(drawLabelsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawLabelsToggled(bool)));
    cLayout->addWidget(drawLabelsCheckBox, row, 0);
    ++row;

    QFrame *splitter2 = new QFrame(this, "splitter");
    splitter2->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter2, row, row, 0, 3);  
    ++row;

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    textColorLabel = new QLabel(textColorButton, "Text color", this);
    cLayout->addWidget(textColorLabel, row, 2, Qt::AlignLeft);
    cLayout->addWidget(textColorButton, row, 3);
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "textColorOpacity");
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    cLayout->addMultiCellWidget(textColorOpacity, row, row, 2, 3);
    ++row;
#endif

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox("Use foreground color", this,
        "useForegroundColorCheckBox");
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addMultiCellWidget(useForegroundColorCheckBox, row, row, 0, 1);
    ++row;

    // Add controls for text format string.
    formatString = new QNarrowLineEdit(this, "formatString");
    connect(formatString, SIGNAL(returnPressed()),
            this, SLOT(textChanged()));
    cLayout->addWidget(formatString, row, 3);
    cLayout->addWidget(new QLabel(formatString, "Number format",
        this), row, 2);
    // Add control for text font height
    fontHeight = new QNarrowLineEdit(this, "fontHeight");
    connect(fontHeight, SIGNAL(returnPressed()),
            this, SLOT(fontHeightChanged()));
    cLayout->addWidget(fontHeight, row, 1);
    cLayout->addWidget(new QLabel(fontHeight, "Font height",
        this), row, 0);
    ++row;

    // Add controls to set the font family.
    fontFamilyComboBox = new QComboBox(this, "fontFamilyComboBox");
    fontFamilyComboBox->insertItem("Arial", 0);
    fontFamilyComboBox->insertItem("Courier", 1);
    fontFamilyComboBox->insertItem("Times", 2);
    fontFamilyComboBox->setEditable(false);
    connect(fontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(fontFamilyChanged(int)));
    cLayout->addMultiCellWidget(fontFamilyComboBox, row, row, 1, 3);
    cLayout->addWidget(new QLabel("Font family", this), row, 0);
    ++row;

    // Add controls for font properties.
    boldCheckBox = new QCheckBox("Bold", this, "boldCheckBox");
    connect(boldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(boldToggled(bool)));
    cLayout->addWidget(boldCheckBox, row, 0);

    italicCheckBox = new QCheckBox("Italic", this, "italicCheckBox");
    connect(italicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(italicToggled(bool)));
    cLayout->addWidget(italicCheckBox, row, 1);

    shadowCheckBox = new QCheckBox("Shadow", this, "shadowCheckBox");
    connect(shadowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    cLayout->addWidget(shadowCheckBox, row, 2);
    shadowCheckBox->setEnabled(false); // Until this works in the legend.
    ++row;
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::~QvisLegendAttributesInterface
//
// Purpose: 
//   Destructor for the QvisLegendAttributesInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:47:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisLegendAttributesInterface::~QvisLegendAttributesInterface()
{
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::GetMenuText
//
// Purpose: 
//   Returns the text to use in the annotation list box.
//
// Arguments:
//   annot : The annotation object to use for extra information.
//
// Returns:    The text to use in the annotation list box.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 14:16:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QString
QvisLegendAttributesInterface::GetMenuText(const AnnotationObject &annot) const
{
    // Look for the name of a plot in the plot list that has the same name as 
    // the annotation object. There should be a match because of how plots are
    // created.
    QString retval;
    bool match = false;
    PlotPluginManager *pMgr = PlotPluginManager::Instance();
    OperatorPluginManager *oMgr = OperatorPluginManager::Instance();
    for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
    {
        const Plot &plot = GetViewerState()->GetPlotList()->GetPlots(i);
        if(plot.GetPlotName() == annot.GetObjectName())
        {
            match = true;
            retval = GetName();
            retval += ":";

            CommonPlotPluginInfo *plotInfo = pMgr->GetCommonPluginInfo(
                pMgr->GetEnabledID(plot.GetPlotType()));
            retval += QString(plotInfo->GetName());
            retval += " - ";
            for(int j = 0; j < plot.GetNumOperators(); ++j)
            {
                CommonOperatorPluginInfo *info = oMgr->GetCommonPluginInfo(
                oMgr->GetEnabledID(plot.GetOperator(plot.GetNumOperators() - 1 - j)));
                retval += info->GetName();
                retval += "(";
            }
            retval += plot.GetPlotVar().c_str();
            for(int j = 0; j < plot.GetNumOperators(); ++j)
                retval += ")";

            match = true;
            break;
        }
    }

    if(!match)
        retval.sprintf("%s - %s", GetName().latin1(), annot.GetObjectName().c_str());

    return retval;
}


bool
QvisLegendAttributesInterface::GetBool(int bit) const
{
    return (annot->GetIntAttribute1() & (1 << bit)) != 0;
}

void
QvisLegendAttributesInterface::SetBool(int bit, bool val)
{
    int shifted = (1 << bit);
    int mask = ~shifted;
    int data = annot->GetIntAttribute1();
    annot->SetIntAttribute1((data & mask) | (val ? shifted : 0));
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 15:58:02 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::UpdateControls()
{
    // Set the manage layout bit.
    manageLayout->blockSignals(true);
    manageLayout->setChecked(GetBool(LEGEND_MANAGE_POSITION));
    manageLayout->blockSignals(false);

    // Set the start position.
    positionEdit->setPosition(annot->GetPosition()[0], annot->GetPosition()[1]);
    positionEdit->setEnabled(!GetBool(LEGEND_MANAGE_POSITION));
    positionLabel->setEnabled(!GetBool(LEGEND_MANAGE_POSITION));

    // Set the spinbox values for the width and height.  The 0.5 is added
    // to avoid numeric issues converting back and forth between float and
    // integer.
    int w(int(annot->GetPosition2()[0] * WIDTH_HEIGHT_PRECISION + 0.5f));
    widthSpinBox->blockSignals(true);
    widthSpinBox->setValue(w);
    widthSpinBox->blockSignals(false);

    int h(int(annot->GetPosition2()[1] * WIDTH_HEIGHT_PRECISION + 0.5f));
    heightSpinBox->blockSignals(true);
    heightSpinBox->setValue(h);
    heightSpinBox->blockSignals(false);

    //
    // Set the text color. If we're using the foreground color for the text
    // color then make the button be white and only let the user change the 
    // opacity.
    //
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity->blockSignals(true);
#endif
    QColor tc(annot->GetTextColor().Red(),
              annot->GetTextColor().Green(),
              annot->GetTextColor().Blue());
    textColorButton->setButtonColor(tc);
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity->setGradientColor(tc);
    textColorOpacity->setValue(annot->GetTextColor().Alpha());
    textColorOpacity->blockSignals(false);
    textColorOpacity->setEnabled(!annot->GetUseForegroundForTextColor());
#endif
    textColorButton->setEnabled(!annot->GetUseForegroundForTextColor());
    textColorLabel->setEnabled(!annot->GetUseForegroundForTextColor());

    // Set the bounding box color.
    drawBoundingBoxCheckBox->blockSignals(true);
    drawBoundingBoxCheckBox->setChecked(GetBool(LEGEND_DRAW_BOX));
    drawBoundingBoxCheckBox->blockSignals(false);
    boundingBoxOpacity->blockSignals(true);
    QColor bbc(annot->GetColor1().Red(),
               annot->GetColor1().Green(),
               annot->GetColor1().Blue());
    boundingBoxColorButton->setButtonColor(bbc);
    boundingBoxColorButton->setEnabled(GetBool(LEGEND_DRAW_BOX));
    boundingBoxOpacity->setEnabled(GetBool(LEGEND_DRAW_BOX));
    boundingBoxOpacity->setGradientColor(bbc);
    boundingBoxOpacity->setValue(annot->GetColor1().Alpha());
    boundingBoxOpacity->blockSignals(false);

    // Set the format string
    const stringVector &annotText = annot->GetText();
    if(annotText.size() > 0)
        formatString->setText(annotText[0].c_str());
    else
        formatString->setText("");

    // Set the "draw labels" box.
    drawLabelsCheckBox->blockSignals(true);
    drawLabelsCheckBox->setChecked(GetBool(LEGEND_DRAW_LABELS));
    drawLabelsCheckBox->blockSignals(false);

    // Set the font height
    fontHeight->setText(QString().sprintf("%g", annot->GetDoubleAttribute1()));

    // Set the use foreground color check box.
    useForegroundColorCheckBox->blockSignals(true);
    useForegroundColorCheckBox->setChecked(annot->GetUseForegroundForTextColor());
    useForegroundColorCheckBox->blockSignals(false);

    // Set the font family
    fontFamilyComboBox->blockSignals(true);
    fontFamilyComboBox->setCurrentItem(int(annot->GetFontFamily()));
    fontFamilyComboBox->blockSignals(false);

    // Set the bold check box.
    boldCheckBox->blockSignals(true);
    boldCheckBox->setChecked(annot->GetFontBold());
    boldCheckBox->blockSignals(false);

    // Set the italic check box.
    italicCheckBox->blockSignals(true);
    italicCheckBox->setChecked(annot->GetFontItalic());
    italicCheckBox->blockSignals(false);

    // Set the shadow check box.
    shadowCheckBox->blockSignals(true);
    shadowCheckBox->setChecked(annot->GetFontShadow());
    shadowCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 16:00:25 PST 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisLegendAttributesInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        // Get the new position
        GetScreenPosition(positionEdit, "Lower left");
    }

    if(which_widget == 1 || doAll)
    {
        QString fmt(formatString->text());
        int count = 0;
        for(int i = 0; i < fmt.length(); ++i)
            if(fmt[i] == '%')
                ++count;
        if(count == 0)
        {
            Error("The format string for the legend was not used because it "
                  "does not contain a '%' character.");
        }
        else if(count > 1)
        {
            Error("The format string for the legend was not used because it "
                  "contains multiple '%' characters.");
        }
        else
        {
            stringVector sv;
            sv.push_back(formatString->text().latin1());
            annot->SetText(sv);
        }
    }

    if(which_widget == 2 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(widthSpinBox);
        int w = widthSpinBox->value();
        double pos2[3];
        pos2[0] = double(w) * (1. / WIDTH_HEIGHT_PRECISION);
        pos2[1] = annot->GetPosition2()[1];
        pos2[2] = annot->GetPosition2()[2];
        annot->SetPosition2(pos2);
    }

    if(which_widget == 3 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(heightSpinBox);
        int h = heightSpinBox->value();
        double pos2[3];
        pos2[0] = annot->GetPosition2()[0];
        pos2[1] = double(h) * (1. / WIDTH_HEIGHT_PRECISION);
        pos2[2] = annot->GetPosition2()[2];
        annot->SetPosition2(pos2);
    }

    if(which_widget == 4 || doAll)
    {
        bool okay;
        double val = fontHeight->text().toDouble(&okay);
        if(okay)
            annot->SetDoubleAttribute1(val);
    }
}

//
// Qt Slot functions
//

void
QvisLegendAttributesInterface::layoutChanged(bool val)
{
    SetBool(LEGEND_MANAGE_POSITION, val);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::positionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   position line edit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::positionChanged(double x, double y)
{
    double pos[] = {x, y, 0.};
    annot->SetPosition(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::widthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value of the width
//   spin box changes.
//
// Arguments:
//   w : The new width in percent.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::widthChanged(int w)
{
    double pos2[3];
    pos2[0] = double(w) * (1. / WIDTH_HEIGHT_PRECISION);
    pos2[1] = annot->GetPosition2()[1];
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::heightChanged
//
// Purpose: 
//   This a Qt slot function that is called when the value of the height spin
//   box changes.
//
// Arguments:
//   h : The new height in percent.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:50:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::heightChanged(int h)
{
    double pos2[3];
    pos2[0] = annot->GetPosition2()[0];
    pos2[1] = double(h) * (1. / WIDTH_HEIGHT_PRECISION);
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   text line edit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textChanged()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Class: QvisLegendAttributesInterface::fontHeightChanged
//
// Purpose:
//   This is a Qt slot function that is called when return is pressed in the 
//   fontHeight line edit.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 09:27:24 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::fontHeightChanged()
{
    GetCurrentValues(4);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textColorChanged(const QColor &c)
{
    int a = annot->GetTextColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetTextColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textOpacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetTextColor());
    tc.SetAlpha(opacity);
    annot->SetTextColor(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: drawBoundingBoxToggled
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 15:34:13 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::drawBoundingBoxToggled(bool val)
{
    SetBool(LEGEND_DRAW_BOX, val);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::boundingBoxColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new bounding box color is
//   selected.
//
// Arguments:
//   c : The new color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boundingBoxColorChanged(const QColor &c)
{
    int a = annot->GetColor1().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetColor1(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new bounding box opacity
//   is selected.
//
// Arguments:
//   opacity : The new bbox opacity.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boundingBoxOpacityChanged(int opacity)
{
    ColorAttribute c(annot->GetColor1());
    c.SetAlpha(opacity);
    annot->SetColor1(c);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::fontFamilyChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the font family is changed.
//
// Arguments:
//   family  : The new font family.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 16:00:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::fontFamilyChanged(int family)
{
    annot->SetFontFamily((AnnotationObject::FontFamily)family);
    SetUpdate(false);
    Apply();
}

void
QvisLegendAttributesInterface::drawLabelsToggled(bool val)
{
    SetBool(LEGEND_DRAW_LABELS, val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::boldToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the bold checkbox is toggled.
//
// Arguments:
//   val : The new bold flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 16:01:03 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boldToggled(bool val)
{
    annot->SetFontBold(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::italicToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the italic checkbox is toggled.
//
// Arguments:
//   val : The new italic flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 16:01:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::italicToggled(bool val)
{
    annot->SetFontItalic(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::shadowToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the shadow checkbox is
//   toggled.
//
// Arguments:
//   val : The new shadow setting.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 16:02:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::shadowToggled(bool val)
{
    annot->SetFontShadow(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::useForegroundColorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the useForegroundColor
//   check box is clicked.
//
// Arguments:
//   val : The new setting for useForegroundColor
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:34:48 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}
