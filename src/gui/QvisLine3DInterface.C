// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Kathleen Biagas, Mon Jul 30 16:43:01 MST 2018
//   Change layout for arrows, to allow more room for radius.
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
    cLayout->addWidget(point1Edit, row, 1);
    cLayout->addWidget(startLabel, row, 0);

    // Add controls for point2
    point2Edit = new QLineEdit(this);
    connect(point2Edit, SIGNAL(returnPressed()),
            this, SLOT(point2Changed()));
    QLabel *endLabel = new QLabel(tr("End"), this);
    cLayout->addWidget(point2Edit, row, 4);
    cLayout->addWidget(endLabel, row, 3);
    ++row;

    // Add controls for line type.
    lineType = new QComboBox(this);
    lineType->addItem(tr("Line"));
    lineType->addItem(tr("Tube"));
    connect(lineType, SIGNAL(activated(int)),
            this, SLOT(lineTypeChanged(int)));
    cLayout->addWidget(lineType, row, 1, 1, 1);
    cLayout->addWidget(new QLabel(tr("Line type"), this), row, 0);

    // Add controls for line width.
    widthWidget = new QvisLineWidthWidget(0, this);
    connect(widthWidget, SIGNAL(lineWidthChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthWidget, row, 4);
    widthLabel = new QLabel(tr("Line Width"), this);
    cLayout->addWidget(widthLabel, row, 3);
    //++row;

    // Add controls for tube quality.
    tubeQuality = new QComboBox(this);
    tubeQuality->addItem(tr("Low"));
    tubeQuality->addItem(tr("Medium"));
    tubeQuality->addItem(tr("High"));
    tubeQuality->setCurrentIndex(1);
    connect(tubeQuality, SIGNAL(activated(int)),
            this, SLOT(tubeQualityChanged(int)));
    cLayout->addWidget(tubeQuality, row, 4);
    tubeQualLabel = new QLabel(tr("Tube Quality"), this);
    cLayout->addWidget(tubeQualLabel, row, 3);
    ++row;

    // Add controls for tube radius.
    tubeRadius = new QLineEdit(this);
    connect(tubeRadius, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusChanged()));
    connect(tubeRadius, SIGNAL(editingFinished()),
            this, SLOT(tubeRadiusChanged()));
    cLayout->addWidget(tubeRadius, row, 4);
    tubeRadLabel = new QLabel(tr("Tube Radius"), this);
    cLayout->addWidget(tubeRadLabel, row, 3);
    ++row;

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox(tr("Use foreground color"), this);
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addWidget(useForegroundColorCheckBox, row, 0, 1, 4);
    ++row;

    // Add controls for the line color.
    colorLabel = new QLabel(tr("Line color"), this);
    cLayout->addWidget(colorLabel, row, 0, Qt::AlignRight);

    colorButton = new QvisColorButton(this);
    connect(colorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    cLayout->addWidget(colorButton, row, 1);

    // Add controls for the line opacity.
    opacitySlider = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(opacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(opacityChanged(int)));
    cLayout->addWidget(opacitySlider, row, 3, 1, 3);
    ++row;

    // Add controls for arrow 1 (Begin arrow)
    beginArrow = new QCheckBox(tr("Begin Arrow"), this);
    connect(beginArrow, SIGNAL(toggled(bool)),
            this, SLOT(beginArrowToggled(bool)));
    cLayout->addWidget(beginArrow, row, 0, Qt::AlignLeft);
    ++row;

    arrow1Resolution = new QSpinBox(this);
    arrow1Resolution->setMinimum(1);
    arrow1Resolution->setMaximum(20);
    arrow1Resolution->setButtonSymbols(QSpinBox::PlusMinus);
    connect(arrow1Resolution, SIGNAL(valueChanged(int)),
            this, SLOT(arrow1ResolutionChanged(int)));
    cLayout->addWidget(arrow1Resolution, row, 1);
    res1Label = new QLabel(tr("Resolution"), this);
    cLayout->addWidget(res1Label, row, 0, Qt::AlignRight);

    arrow1Radius = new QLineEdit(this);
    connect(arrow1Radius, SIGNAL(returnPressed()),
            this, SLOT(arrow1RadiusChanged()));
    connect(arrow1Radius, SIGNAL(editingFinished()),
            this, SLOT(arrow1RadiusChanged()));
    cLayout->addWidget(arrow1Radius, row, 3);
    rad1Label = new QLabel(tr("Radius"), this);
    cLayout->addWidget(rad1Label, row, 2, Qt::AlignRight);

    arrow1Height = new QLineEdit(this);
    connect(arrow1Height, SIGNAL(returnPressed()),
            this, SLOT(arrow1HeightChanged()));
    connect(arrow1Height, SIGNAL(editingFinished()),
            this, SLOT(arrow1HeightChanged()));
    cLayout->addWidget(arrow1Height, row, 5);
    height1Label = new QLabel(tr("Height"), this);
    cLayout->addWidget(height1Label, row, 4, Qt::AlignRight);
    ++row;

    // Add controls for arrow 2 (End arrow)
    endArrow = new QCheckBox(tr("End Arrow"), this);
    connect(endArrow, SIGNAL(toggled(bool)),
            this, SLOT(endArrowToggled(bool)));
    cLayout->addWidget(endArrow, row, 0, Qt::AlignLeft);
    ++row;

    arrow2Resolution = new QSpinBox(this);
    arrow2Resolution->setMinimum(1);
    arrow2Resolution->setMaximum(20);
    arrow2Resolution->setButtonSymbols(QSpinBox::PlusMinus);
    connect(arrow2Resolution, SIGNAL(valueChanged(int)),
            this, SLOT(arrow2ResolutionChanged(int)));
    cLayout->addWidget(arrow2Resolution, row, 1);
    res2Label = new QLabel(tr("Resolution"), this);
    cLayout->addWidget(res2Label, row, 0, Qt::AlignRight);

    arrow2Radius = new QLineEdit(this);
    connect(arrow2Radius, SIGNAL(returnPressed()),
            this, SLOT(arrow2RadiusChanged()));
    connect(arrow2Radius, SIGNAL(editingFinished()),
            this, SLOT(arrow2RadiusChanged()));
    cLayout->addWidget(arrow2Radius, row, 3);
    rad2Label = new QLabel(tr("Radius"), this);
    cLayout->addWidget(rad2Label, row, 2, Qt::AlignRight);

    arrow2Height = new QLineEdit(this);
    connect(arrow2Height, SIGNAL(returnPressed()),
            this, SLOT(arrow2HeightChanged()));
    connect(arrow2Height, SIGNAL(editingFinished()),
            this, SLOT(arrow2HeightChanged()));
    cLayout->addWidget(arrow2Height, row, 5);
    height2Label = new QLabel(tr("Height"), this);
    cLayout->addWidget(height2Label, row, 4, Qt::AlignRight);
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
//   Kathleen Biagas, Thu Jan 21, 2021
//   Replace QString.asprintf with QString.arg, and QString.setNum
//   for simpler cases.
//
// ****************************************************************************

void
QvisLine3DInterface::UpdateControls()
{
    // Set the start position.
    const MapNode &opts = annot->GetOptions();
    QString pos = QString("%1 %2 %3")
        .arg(annot->GetPosition()[0])
        .arg(annot->GetPosition()[1])
        .arg(annot->GetPosition()[2]);
    point1Edit->setText(pos);

    // Set the end position.
    pos = QString("%1 %2 %3")
        .arg(annot->GetPosition2()[0])
        .arg(annot->GetPosition2()[1])
        .arg(annot->GetPosition2()[2]);
    point2Edit->setText(pos);

    lineType->blockSignals(true);
    lineType->setCurrentIndex(opts.GetEntry("lineType")->AsInt());
    lineType->blockSignals(false);

    // Set the width
    widthWidget->blockSignals(true);
    widthWidget->SetLineWidth(opts.GetEntry("width")->AsInt());
    widthWidget->setEnabled(lineType->currentIndex() == 0);
    widthWidget->setVisible(lineType->currentIndex() == 0);
    widthLabel->setEnabled(lineType->currentIndex() == 0);
    widthLabel->setVisible(lineType->currentIndex() == 0);
    widthWidget->blockSignals(false);

    tubeQuality->blockSignals(true);
    tubeQuality->setCurrentIndex(opts.GetEntry("tubeQuality")->AsInt());
    tubeQuality->setEnabled(lineType->currentIndex() == 1);
    tubeQuality->setVisible(lineType->currentIndex() == 1);
    tubeQualLabel->setEnabled(lineType->currentIndex() == 1);
    tubeQualLabel->setVisible(lineType->currentIndex() == 1);
    tubeQuality->blockSignals(false);

    tubeRadius->blockSignals(true);
    pos.setNum(opts.GetEntry("tubeRadius")->AsDouble());
    tubeRadius->setText(pos);
    tubeRadius->setEnabled(lineType->currentIndex() == 1);
    tubeRadius->setVisible(lineType->currentIndex() == 1);
    tubeRadLabel->setEnabled(lineType->currentIndex() == 1);
    tubeRadLabel->setVisible(lineType->currentIndex() == 1);
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


    beginArrow->blockSignals(true);
    beginArrow->setChecked(opts.GetEntry("arrow1")->AsBool());
    beginArrow->blockSignals(false);

    arrow1Resolution->blockSignals(true);
    arrow1Resolution->setValue(opts.GetEntry("arrow1Resolution")->AsInt());
    arrow1Resolution->setEnabled(beginArrow->isChecked());
    res1Label->setEnabled(beginArrow->isChecked());
    arrow1Resolution->blockSignals(false);


    arrow1Radius->blockSignals(true);
    pos.setNum(opts.GetEntry("arrow1Radius")->AsDouble());
    arrow1Radius->setText(pos);
    arrow1Radius->setEnabled(beginArrow->isChecked());
    rad1Label->setEnabled(beginArrow->isChecked());
    arrow1Radius->blockSignals(false);

    arrow1Height->blockSignals(true);
    pos.setNum(opts.GetEntry("arrow1Height")->AsDouble());
    arrow1Height->setText(pos);
    arrow1Height->setEnabled(beginArrow->isChecked());
    height1Label->setEnabled(beginArrow->isChecked());
    arrow1Height->blockSignals(false);

    endArrow->blockSignals(true);
    endArrow->setChecked((bool)(opts.GetEntry("arrow2")->AsBool()));
    endArrow->blockSignals(false);

    arrow2Resolution->blockSignals(true);
    arrow2Resolution->setValue(opts.GetEntry("arrow2Resolution")->AsInt());
    arrow2Resolution->setEnabled(endArrow->isChecked());
    res2Label->setEnabled(endArrow->isChecked());
    arrow2Resolution->blockSignals(false);

    arrow2Radius->blockSignals(true);
    pos.setNum(opts.GetEntry("arrow2Radius")->AsDouble());
    arrow2Radius->setText(pos);
    arrow2Radius->setEnabled(endArrow->isChecked());
    rad2Label->setEnabled(endArrow->isChecked());
    arrow2Radius->blockSignals(false);

    arrow2Height->blockSignals(true);
    if (opts.HasEntry("arrow2Height"))
    {
        pos.setNum(opts.GetEntry("arrow2Height")->AsDouble());
        arrow2Height->setText(pos);
    }
    else
    {
        arrow2Height->setText("-1");
    }
    arrow2Height->setEnabled(endArrow->isChecked());
    height2Label->setEnabled(endArrow->isChecked());
    arrow2Height->blockSignals(false);

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

    // point 1
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
    // point 2
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
    // tube radius
    if (doAll || which_widget == 2)
    {
        double v;
        if(LineEditGetDouble(tubeRadius, v))
        {
            annot->GetOptions().GetEntry("tubeRadius")->SetValue(v);
        }
        else
        {
            v = annot->GetOptions().GetEntry("tubeRadius")->AsDouble();
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(v));
            Error(msg);
        }
    }
    // arrow 1 radius
    if (doAll || which_widget == 3)
    {
        double v;
        if(LineEditGetDouble(arrow1Radius, v))
        {
            annot->GetOptions().GetEntry("arrow1Radius")->SetValue(v);
        }
        else
        {
            v = annot->GetOptions().GetEntry("arrow1Radius")->AsDouble();
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(v));
            Error(msg);
        }
    }
    // arrow 1 height
    if (doAll || which_widget == 4)
    {
        double v;
        if(LineEditGetDouble(arrow1Height, v))
        {
            annot->GetOptions().GetEntry("arrow1Height")->SetValue(v);
        }
        else
        {
            v = annot->GetOptions().GetEntry("arrow1Height")->AsDouble();
            QString msg = tr("The height must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(v));
            Error(msg);
        }
    }
    // arrow 2 radius
    if (doAll || which_widget == 5)
    {
        double v;
        if(LineEditGetDouble(arrow2Radius, v))
        {
            annot->GetOptions().GetEntry("arrow2Radius")->SetValue(v);
        }
        else
        {
            v = annot->GetOptions().GetEntry("arrow2Radius")->AsDouble();
            QString msg = tr("The radius must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(v));
            Error(msg);
        }
    }
    // arrow 2 height
    if (doAll || which_widget == 6)
    {
        double v;
        if(LineEditGetDouble(arrow2Height, v))
        {
            annot->GetOptions().GetEntry("arrow2Height")->SetValue(v);
        }
        else
        {
            v = annot->GetOptions().GetEntry("arrow2Height")->AsDouble();
            QString msg = tr("The height must be specified as a floating point "
               "value. Resetting to the last good value of %1.").
               arg(DoubleToQString(v));
            Error(msg);
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
    annot->GetOptions().GetEntry("width")->SetValue(w);
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
    annot->GetOptions().GetEntry("arrow1")->SetValue(val);
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
    annot->GetOptions().GetEntry("arrow1Resolution")->SetValue(r);
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
    GetCurrentValues(3);
}


// ****************************************************************************
// Method: QvisLine3DInterface::arrow1HeightChanged
//
// Purpose:
//   This is a Qt slot function that is called when return is pressed in the
//   arrow1Height line edit.
//
// Programmer: Kathleen Biagas
// Creation:   November 28, 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisLine3DInterface::arrow1HeightChanged()
{
    GetCurrentValues(4);
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
    annot->GetOptions().GetEntry("arrow2")->SetValue(val);
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
    annot->GetOptions().GetEntry("arrow2Resolution")->SetValue(r);
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
    GetCurrentValues(5);
}

// ****************************************************************************
// Method: QvisLine3DInterface::arrow2HeightChanged
//
// Purpose:
//   This is a Qt slot function that is called when return is pressed in the
//   arrow2Height line edit.
//
// Programmer: Kathleen Biagas
// Creation:   November 28, 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisLine3DInterface::arrow2HeightChanged()
{
    GetCurrentValues(6);
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
    annot->GetOptions().GetEntry("lineType")->SetValue(val);
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
    annot->GetOptions().GetEntry("tubeQuality")->SetValue(val);
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
    GetCurrentValues(2);
}


