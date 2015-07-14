/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <QvisLine3DInterface.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>

#include <AnnotationObject.h>

#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <QvisLineStyleWidget.h>
#include <QvisOpacitySlider.h>

// ****************************************************************************
// Method: QvisLine3DInterface::QvisLine3DInterface
//
// Purpose: 
//   Constructor for the QvisLine3DInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:39:07 PDT 2015
//   Add support for arrow and tube style lines.
//
// ****************************************************************************

QvisLine3DInterface::QvisLine3DInterface(QWidget *parent) :
    QvisAnnotationObjectInterface(parent)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(0);
    topLayout->addLayout(cLayout);
    cLayout->setSpacing(10);

    int row = 0;
    // Add controls for point1
    point1Edit = new QLineEdit(this);
    connect(point1Edit, SIGNAL(returnPressed()),
            this, SLOT(point1Changed()));
    QLabel *startLabel = new QLabel(tr("Start"), this);
    cLayout->addWidget(point1Edit, row, 1, 1, 3);
    cLayout->addWidget(startLabel, row, 0);
    ++row;

    // Add controls for point2 
    point2Edit = new QLineEdit(this);
    connect(point2Edit, SIGNAL(returnPressed()),
            this, SLOT(point2Changed()));
    QLabel *endLabel = new QLabel(tr("End"), this);
    cLayout->addWidget(point2Edit, row, 1, 1, 3);
    cLayout->addWidget(endLabel, row, 0);
    ++row;
  
    // Add controls for line type.
    lineType = new QComboBox(this);
    lineType->addItem(tr("Line"));
    lineType->addItem(tr("Tube"));
    connect(lineType, SIGNAL(activated(int)),
            this, SLOT(lineTypeChanged(int)));
    cLayout->addWidget(lineType, row, 1, 1, 2);
    cLayout->addWidget(new QLabel(tr("Line type"), this), row, 0);
    ++row;

    // Add controls for line width.
    widthWidget = new QvisLineWidthWidget(0, this);
    connect(widthWidget, SIGNAL(lineWidthChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthWidget, row, 1);
    widthLabel = new QLabel(tr("Line Width"), this);
    cLayout->addWidget(widthLabel, row, 0);
    //++row;

    // Add controls for line style.
    styleWidget = new QvisLineStyleWidget(0, this);
    connect(styleWidget, SIGNAL(lineStyleChanged(int)),
            this, SLOT(styleChanged(int)));
    cLayout->addWidget(styleWidget, row, 3);
    styleLabel = new QLabel(tr("Line Style"), this);
    cLayout->addWidget(styleLabel, row, 2);
    ++row;

    // Add controls for tube quality.
    tubeQuality = new QComboBox(this);
    tubeQuality->addItem(tr("Low"));
    tubeQuality->addItem(tr("Medium"));
    tubeQuality->addItem(tr("High"));
    tubeQuality->setCurrentIndex(1);
    connect(tubeQuality, SIGNAL(activated(int)),
            this, SLOT(tubeQualityChanged(int)));
    cLayout->addWidget(tubeQuality, row, 1);
    tubeQualLabel = new QLabel(tr("Tube Quality"), this);
    cLayout->addWidget(tubeQualLabel, row, 0);
    //++row;

    // Add controls for tube radius.
    tubeRadius = new QLineEdit(this);
    connect(tubeRadius, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusChanged()));
    cLayout->addWidget(tubeRadius, row, 3);
    tubeRadLabel = new QLabel(tr("Tube Radius"), this);
    cLayout->addWidget(tubeRadLabel, row, 2);
    ++row;

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox(tr("Use foreground color"), this);
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addWidget(useForegroundColorCheckBox, row, 0, 1, 4);
    ++row;

    // Add controls for the line color.
    colorLabel = new QLabel(tr("Line color"), this);
    cLayout->addWidget(colorLabel, row, 0, Qt::AlignLeft);

    colorButton = new QvisColorButton(this);
    connect(colorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    cLayout->addWidget(colorButton, row, 1);

    // Add controls for the line opacity.
    opacitySlider = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(opacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(opacityChanged(int)));
    cLayout->addWidget(opacitySlider, row, 2, 1, 3);
    ++row;

    beginArrow = new QCheckBox(tr("Begin Arrow"), this);
    connect(beginArrow, SIGNAL(toggled(bool)),
            this, SLOT(beginArrowToggled(bool)));
    cLayout->addWidget(beginArrow, row, 0, Qt::AlignLeft);

    arrow1Resolution = new QSpinBox(this);
    arrow1Resolution->setMinimum(1);
    arrow1Resolution->setMaximum(20);
    arrow1Resolution->setButtonSymbols(QSpinBox::PlusMinus);
    connect(arrow1Resolution, SIGNAL(valueChanged(int)),
            this, SLOT(arrow1ResolutionChanged(int)));
    cLayout->addWidget(arrow1Resolution, row, 2);
    res1Label = new QLabel(tr("Resolution"), this);
    cLayout->addWidget(res1Label, row, 1, Qt::AlignRight);
   
    arrow1Radius = new QLineEdit(this);
    connect(arrow1Radius, SIGNAL(returnPressed()),
            this, SLOT(arrow1RadiusChanged()));
    cLayout->addWidget(arrow1Radius, row, 4);
    rad1Label = new QLabel(tr("Radius"), this);
    cLayout->addWidget(rad1Label, row, 3, Qt::AlignRight);
    ++row;

    endArrow = new QCheckBox(tr("End Arrow"), this);
    connect(endArrow, SIGNAL(toggled(bool)),
            this, SLOT(endArrowToggled(bool)));
    cLayout->addWidget(endArrow, row, 0, Qt::AlignLeft);

    arrow2Resolution = new QSpinBox(this);
    arrow2Resolution->setMinimum(1);
    arrow2Resolution->setMaximum(20);
    arrow2Resolution->setButtonSymbols(QSpinBox::PlusMinus);
    connect(arrow2Resolution, SIGNAL(valueChanged(int)),
            this, SLOT(arrow2ResolutionChanged(int)));
    cLayout->addWidget(arrow2Resolution, row, 2);
    res2Label = new QLabel(tr("Resolution"), this);
    cLayout->addWidget(res2Label, row, 1, Qt::AlignRight);

    arrow2Radius = new QLineEdit(this);
    connect(arrow2Radius, SIGNAL(returnPressed()),
            this, SLOT(arrow2RadiusChanged()));
    cLayout->addWidget(arrow2Radius, row, 4);
    rad2Label = new QLabel(tr("Radius"), this);
    cLayout->addWidget(rad2Label, row, 3, Qt::AlignRight);
    ++row;
    

    // Added a visibility toggle
    visibleCheckBox = new QCheckBox(tr("Visible"), this);
    connect(visibleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(visibilityToggled(bool)));
    cLayout->addWidget(visibleCheckBox, row, 0);
}


// ****************************************************************************
// Method: QvisLine3DInterface::~QvisLine3DInterface
//
// Purpose: 
//   Destructor for the QvisLine3DInterface class.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

QvisLine3DInterface::~QvisLine3DInterface()
{
}


// ****************************************************************************
// Method: QvisLine3DInterface::GetMenuText
//
// Purpose: 
//   Returns the text to use in the annotation list box.
//
// Arguments:
//   annot : The annotation object to use for extra information.
//
// Returns:    The text to use in the annotation list box.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//
// ****************************************************************************

QString
QvisLine3DInterface::GetMenuText(const AnnotationObject &annot) const
{
    QString retval;
    if(annot.GetText().size() > 0)
        retval = QString("%1 - %2").arg(GetName()).arg(annot.GetText()[0].c_str());
    else
        retval = GetName();

    return retval;
}


// ****************************************************************************
// Method: QvisLine3DInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:39:07 PDT 2015
//   Add support for arrow and tube style lines.
//
// ****************************************************************************

void
QvisLine3DInterface::UpdateControls()
{
    // Set the start position.
    QString pos;
    pos.sprintf("%lg %lg %lg",
        annot->GetPosition()[0],
        annot->GetPosition()[1],
        annot->GetPosition()[2]);
    point1Edit->setText(pos);
    
    // Set the end position.
    pos.sprintf("%lg %lg %lg",
        annot->GetPosition2()[0],
        annot->GetPosition2()[1],
        annot->GetPosition2()[2]);
    point2Edit->setText(pos);

    lineType->blockSignals(true);
    lineType->setCurrentIndex(annot->GetIntAttribute3());
    lineType->blockSignals(false);

    // Set the values for the width and style 
    widthWidget->blockSignals(true);
    widthWidget->SetLineWidth(annot->GetIntAttribute1());
    widthWidget->setEnabled(lineType->currentIndex() == 0);
    widthLabel->setEnabled(lineType->currentIndex() == 0);
    widthWidget->blockSignals(false);

    styleWidget->blockSignals(true);
    styleWidget->SetLineStyle(annot->GetIntAttribute2());
    styleWidget->setEnabled(lineType->currentIndex() == 0);
    styleLabel->setEnabled(lineType->currentIndex() == 0);
    styleWidget->blockSignals(false);

    doubleVector dv = annot->GetDoubleVector1();
    tubeQuality->blockSignals(true);
    tubeQuality->setCurrentIndex((int)dv[2]);
    tubeQuality->setEnabled(lineType->currentIndex() == 1);
    tubeQualLabel->setEnabled(lineType->currentIndex() == 1);
    tubeQuality->blockSignals(false);

    tubeRadius->blockSignals(true);
    pos.sprintf("%lg", dv[3]);
    tubeRadius->setText(pos);
    tubeRadius->setEnabled(lineType->currentIndex() == 1);
    tubeRadLabel->setEnabled(lineType->currentIndex() == 1);
    tubeRadius->blockSignals(false);

    // Set the use foreground color check box.
    useForegroundColorCheckBox->blockSignals(true);
    useForegroundColorCheckBox->setChecked(annot->GetUseForegroundForTextColor());
    useForegroundColorCheckBox->blockSignals(false);

    // Change color and opacity.
    colorButton->blockSignals(true);
    opacitySlider->blockSignals(true);

    if (annot->GetUseForegroundForTextColor())
    {    
        QColor tmp(255,255,255);
        colorButton->setButtonColor(tmp);
        colorLabel->setEnabled(false);
        colorButton->setEnabled(false);
        opacitySlider->setGradientColor(tmp);
    }
    else
    {
        QColor tc(annot->GetColor1().Red(),
                  annot->GetColor1().Green(),
                  annot->GetColor1().Blue());
        colorButton->setButtonColor(tc);
        colorLabel->setEnabled(true);
        colorButton->setEnabled(true);
        opacitySlider->setGradientColor(tc);
        opacitySlider->setValue(annot->GetColor1().Alpha());
    }
    opacitySlider->blockSignals(false);
    colorButton->blockSignals(false);

    ColorAttribute ca = annot->GetColor2();

    beginArrow->blockSignals(true);
    beginArrow->setChecked((bool)(ca.Red()));
    beginArrow->blockSignals(false);

    arrow1Resolution->blockSignals(true);
    arrow1Resolution->setValue((int)ca.Blue());
    arrow1Resolution->setEnabled(beginArrow->isChecked());
    res1Label->setEnabled(beginArrow->isChecked());
    arrow1Resolution->blockSignals(false);

  
    arrow1Radius->blockSignals(true);
    pos.sprintf("%lg", dv[0]);
    arrow1Radius->setText(pos);
    arrow1Radius->setEnabled(beginArrow->isChecked());
    rad1Label->setEnabled(beginArrow->isChecked());
    arrow1Radius->blockSignals(false);
  
    endArrow->blockSignals(true);
    endArrow->setChecked((bool)(ca.Green()));
    endArrow->blockSignals(false);

    arrow2Resolution->blockSignals(true);
    arrow2Resolution->setValue((int)ca.Alpha());
    arrow2Resolution->setEnabled(endArrow->isChecked());
    res2Label->setEnabled(endArrow->isChecked());
    arrow2Resolution->blockSignals(false);

    arrow2Radius->blockSignals(true);
    pos.sprintf("%lg", dv[1]);
    arrow2Radius->setText(pos);
    arrow2Radius->setEnabled(endArrow->isChecked());
    rad2Label->setEnabled(endArrow->isChecked());
    arrow2Radius->blockSignals(false);

    // Set the visible check box.
    visibleCheckBox->blockSignals(true);
    visibleCheckBox->setChecked(annot->GetVisible());
    visibleCheckBox->blockSignals(false);
}


// ****************************************************************************
// Method: QvisLine3DInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:39:07 PDT 2015
//   Add support for arrow and tube style lines.
//
// ****************************************************************************

void
QvisLine3DInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    
    if (doAll || which_widget == 0)
    {
        double v[3];
        if(LineEditGetDoubles(point1Edit, v, 3))
            annot->SetPosition(v);
        else
        {
            QString msg = tr("The start point must be specified as a 3D "
               " coordinate. Resetting to the last good value of %1.").
            arg(DoublesToQString(annot->GetPosition(), 3));
            Error(msg);
            annot->SetPosition(annot->GetPosition());
        }  
    }  
    if (doAll || which_widget == 1)
    {
        double v[3];
        if(LineEditGetDoubles(point2Edit, v, 3))
            annot->SetPosition2(v);
        else
        {
            QString msg = tr("The end point must be specified as a 3D "
               " coordinate. Resetting to the last good value of %1.").
            arg(DoublesToQString(annot->GetPosition2(), 3));
            Error(msg);
            annot->SetPosition2(annot->GetPosition2());
        }
    }  
    if (doAll || which_widget == 2)
    {
        double v;
        doubleVector dv = annot->GetDoubleVector1();
        if(LineEditGetDouble(arrow1Radius, v))
        {
            dv[0] = v;
            annot->SetDoubleVector1(dv);
        }
        else
        {
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(dv[0]));
            Error(msg);
            annot->SetDoubleVector1(dv);
        }
    }
    if (doAll || which_widget == 3)
    {
        double v;
        doubleVector dv = annot->GetDoubleVector1();
        if(LineEditGetDouble(arrow2Radius, v))
        {
            dv[1] = v;
            annot->SetDoubleVector1(dv);
        }
        else
        {
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(dv[1]));
            Error(msg);
            annot->SetDoubleVector1(dv);
        }
    }
    if (doAll || which_widget == 4)
    {
        double v;
        doubleVector dv = annot->GetDoubleVector1();
        if(LineEditGetDouble(tubeRadius, v))
        {
            dv[3] = v;
            annot->SetDoubleVector1(dv);
        }
        else
        {
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(dv[3]));
            Error(msg);
            annot->SetDoubleVector1(dv);
        }
    }
}


//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisLine3DInterface::point1Changed
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   point1 line edit.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::point1Changed()
{
    GetCurrentValues(0);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::point2Changed
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   point2 line edit.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::point2Changed()
{
    GetCurrentValues(1);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::widthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value of the width
//   widget changes.
//
// Arguments:
//   w : The new width.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::widthChanged(int w)
{
    annot->SetIntAttribute1(w);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::styleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value of the style
//   widget changes.
//
// Arguments:
//   s : The new style.
//
// Programmer: Kathleen Biagas
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::styleChanged(int s)
{
    annot->SetIntAttribute2(s);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::colorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::colorChanged(const QColor &c)
{
    int a = annot->GetColor1().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetColor1(tc);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::opacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::opacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetColor1());
    tc.SetAlpha(opacity);
    annot->SetColor1(tc);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::visibilityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the visibility toggle is
//   changed.
//
// Arguments:
//   val : The visibility flag.
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::visibilityToggled(bool val)
{
    annot->SetVisible(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::useForegroundColorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the useForegroundColor
//   check box is clicked.
//
// Arguments:
//   val : The new setting for useForegroundColor
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::beginArrowToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the beginArrow
//   check box is clicked.
//
// Arguments:
//   val : The new setting for beginArrow
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::beginArrowToggled(bool val)
{
    ColorAttribute ca = annot->GetColor2();
    ca.SetRed(val?1:0);
    annot->SetColor2(ca);
    Apply();
}

// ****************************************************************************
// Method: QvisLine3DInterface::arrow1ResolutionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value in the
//   arrow1Resolution spin box changes.
//
// Arguments:
//   r:  the new resolution
//
// Programmer: Kathleen Biagas
// Creation:   July 14, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::arrow1ResolutionChanged(int r)
{
    ColorAttribute ca = annot->GetColor2();
    ca.SetBlue(r);
    annot->SetColor2(ca);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::arrow1RadiusChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   arrow1Radius line edit.
//
// Programmer: Kathleen Biagas
// Creation:   July 14, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::arrow1RadiusChanged()
{
    GetCurrentValues(2);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::endArrowToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the endArrow
//   check box is clicked.
//
// Arguments:
//   val : The new setting for endArrow
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::endArrowToggled(bool val)
{
    ColorAttribute ca = annot->GetColor2();
    ca.SetGreen(val?1:0);
    annot->SetColor2(ca);
    Apply();
}

// ****************************************************************************
// Method: QvisLine3DInterface::arrow2ResolutionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value in the
//   arrow2Resolution spin box changes.
//
// Arguments:
//   r:  the new resolution
//
// Programmer: Kathleen Biagas
// Creation:   July 14, 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::arrow2ResolutionChanged(int r)
{
    ColorAttribute ca = annot->GetColor2();
    ca.SetAlpha(r);
    annot->SetColor2(ca);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::arrow2RadiusChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   arrow2Radius line edit.
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::arrow2RadiusChanged()
{
    GetCurrentValues(3);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::lineTypeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when value in lineType combo
//   box changes
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::lineTypeChanged(int val)
{
    annot->SetIntAttribute3(val);
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::tubeQualityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when value in tubeQuality
//   combo box changes
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::tubeQualityChanged(int val)
{
    doubleVector dv = annot->GetDoubleVector1();
    dv[2] = (double) val;
    annot->SetDoubleVector1(dv); 
    Apply();
}


// ****************************************************************************
// Method: QvisLine3DInterface::tubeRadiusChanged
//
// Purpose: 
//   This is a Qt slot function that is called when value in tubeRadius
//   line edit changes
//
// Programmer: Kathleen Biagas 
// Creation:   July 14, 2015 
//
// Modifications:
//   
// ****************************************************************************

void
QvisLine3DInterface::tubeRadiusChanged()
{
    GetCurrentValues(4);
    Apply();
}


