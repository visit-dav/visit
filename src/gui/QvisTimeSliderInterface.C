#include <QvisTimeSliderInterface.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>

#define ROUNDED_MASK 1
#define SHADED_MASK  2

// ****************************************************************************
// Method: QvisTimeSliderInterface::QvisTimeSliderInterface
//
// Purpose: 
//   Constructor for the QvisTimeSliderInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:46:42 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatLineEdit.
//   
// ****************************************************************************

QvisTimeSliderInterface::QvisTimeSliderInterface(QWidget *parent,
    const char *name) : QvisAnnotationObjectInterface(parent, name)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(topLayout, 10, 4);
    cLayout->setSpacing(10);

    // Add controls for the position
    positionEdit = new QvisScreenPositionEdit(this, "positionEdit");
    connect(positionEdit, SIGNAL(screenPositionChanged(float,float)),
            this, SLOT(positionChanged(float,float)));
    cLayout->addMultiCellWidget(positionEdit, 0, 0, 1, 3);
    cLayout->addWidget(new QLabel(positionEdit, "Lower left",
        this), 0, 0);

    // Add controls for position2
    widthSpinBox = new QSpinBox(1, 100, 1, this, "widthSpinBox");
    widthSpinBox->setSuffix("%");
    widthSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthSpinBox, 1, 1);
    cLayout->addWidget(new QLabel(widthSpinBox, "Width",
        this), 1, 0);
    heightSpinBox = new QSpinBox(1, 100, 1, this, "heightSpinBox");
    heightSpinBox->setSuffix("%");
    heightSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(heightChanged(int)));
    cLayout->addWidget(heightSpinBox, 1, 3);
    cLayout->addWidget(new QLabel(widthSpinBox, "Height",
        this), 1, 2);

    // Add controls for time label
    labelLineEdit = new QLineEdit(this, "labelLineEdit");
    connect(labelLineEdit, SIGNAL(returnPressed()),
            this, SLOT(labelChanged()));
    cLayout->addMultiCellWidget(labelLineEdit, 2, 2, 1, 3);
    cLayout->addWidget(new QLabel(labelLineEdit, "Text label",
        this), 2, 0);

    // Add controls for timeFormat
    timeFormatLineEdit = new QLineEdit(this, "timeFormatLineEdit");
    connect(timeFormatLineEdit, SIGNAL(returnPressed()),
            this, SLOT(timeFormatChanged()));
    cLayout->addMultiCellWidget(timeFormatLineEdit, 3, 3, 1, 3);
    cLayout->addWidget(new QLabel(timeFormatLineEdit, "Time format",
        this), 3, 0);

    // Add controls for the start color.
    startColorButton = new QvisColorButton(this, "startColorButton");
    connect(startColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(startColorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(startColorButton, "Start color", this),
        4, 0, Qt::AlignLeft);
    cLayout->addWidget(startColorButton, 4, 1);
    startColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "startColorOpacity");
    connect(startColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(startOpacityChanged(int)));
    cLayout->addMultiCellWidget(startColorOpacity, 4, 4, 2, 3);

    // Add controls for the end color.
    endColorButton = new QvisColorButton(this, "endColorButton");
    connect(endColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(endColorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(endColorButton, "End color", this),
        5, 0, Qt::AlignLeft);
    cLayout->addWidget(endColorButton, 5, 1);
    endColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "endColorOpacity");
    connect(endColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(endOpacityChanged(int)));
    cLayout->addMultiCellWidget(endColorOpacity, 5, 5, 2, 3);

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(textColorButton, "Text color", this),
        6, 0, Qt::AlignLeft);
    cLayout->addWidget(textColorButton, 6, 1);
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "textColorOpacity");
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    cLayout->addMultiCellWidget(textColorOpacity, 6, 6, 2, 3);

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox("Use foreground color", this,
        "useForegroundColorCheckBox");
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addMultiCellWidget(useForegroundColorCheckBox, 7, 7, 0, 3);

    // Add a time display combobox.
    timeDisplayComboBox = new QComboBox(this, "timeDisplayComboBox");
    timeDisplayComboBox->insertItem("All frames", 0);
    timeDisplayComboBox->insertItem("Frames for plot", 1);
    timeDisplayComboBox->insertItem("States for plot", 2);
    timeDisplayComboBox->setEditable(false);
    connect(timeDisplayComboBox, SIGNAL(activated(int)),
            this, SLOT(timeDisplayChanged(int)));
    cLayout->addMultiCellWidget(timeDisplayComboBox, 8, 8, 1, 3);
    cLayout->addWidget(new QLabel("Time source", this), 8, 0);

    // Add a visibility toggle
    visibleCheckBox = new QCheckBox("Visible", this, "visibleCheckBox");
    connect(visibleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(visibilityToggled(bool)));
    cLayout->addWidget(visibleCheckBox, 9, 0);

    // Add a rounded toggle
    roundedCheckBox = new QCheckBox("Rounded", this, "roundedCheckBox");
    connect(roundedCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(roundedToggled(bool)));
    cLayout->addWidget(roundedCheckBox, 9, 1);

    // Add a shaded toggle
    shadedCheckBox = new QCheckBox("Shaded", this, "shadedCheckBox");
    connect(shadedCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(shadedToggled(bool)));
    cLayout->addMultiCellWidget(shadedCheckBox, 9, 9, 2, 3);
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::~QvisTimeSliderInterface
//
// Purpose: 
//   Destructor for the QvisTimeSliderInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:47:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisTimeSliderInterface::~QvisTimeSliderInterface()
{
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:48:15 PDT 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 25 14:52:12 PDT 2004
//   Modify the float to percent conversion for the width and height to
//   avoid numeric issues where the percent value would be one too small.
//   
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatLineEdit.
//
//   Brad Whitlock, Thu Feb 24 16:35:05 PST 2005
//   Changed internal implementation for time format.
//
// ****************************************************************************

void
QvisTimeSliderInterface::UpdateControls()
{
    // Set the start position.
    positionEdit->setPosition(annot->GetPosition()[0], annot->GetPosition()[1]);

    // Set the spinbox values for the width and height.  The 0.5 is added
    // to avoid numeric issues converting back and forth between float and
    // integer.
    int w(int(annot->GetPosition2()[0] * 100.f + 0.5f));
    int h(int(annot->GetPosition2()[1] * 100.f + 0.5f));
    widthSpinBox->blockSignals(true);
    widthSpinBox->setValue(w);
    widthSpinBox->blockSignals(false);
    heightSpinBox->blockSignals(true);
    heightSpinBox->setValue(h);
    heightSpinBox->blockSignals(false);

    // set the start color.
    QColor c1(annot->GetColor1().Red(),
              annot->GetColor1().Green(),
              annot->GetColor1().Blue());
    startColorButton->setButtonColor(c1);
    startColorOpacity->blockSignals(true);
    startColorOpacity->setValue(annot->GetColor1().Alpha());
    startColorOpacity->setGradientColor(c1);
    startColorOpacity->blockSignals(false);

    // Set the end color.
    QColor c2(annot->GetColor2().Red(),
              annot->GetColor2().Green(),
              annot->GetColor2().Blue());
    endColorButton->setButtonColor(c2);
    endColorOpacity->blockSignals(true);
    endColorOpacity->setValue(annot->GetColor2().Alpha());
    endColorOpacity->setGradientColor(c2);
    endColorOpacity->blockSignals(false);

    //
    // Set the text color. If we're using the foreground color for the text
    // color then make the button be white and only let the user change the 
    // opacity.
    //
    textColorOpacity->blockSignals(true);
    if(annot->GetUseForegroundForTextColor())
    {
        QColor tmp(255,255,255);
        textColorButton->setButtonColor(tmp);
        textColorButton->setEnabled(false);
        textColorOpacity->setGradientColor(tmp);
    }
    else
    {
        QColor tc(annot->GetTextColor().Red(),
                  annot->GetTextColor().Green(),
                  annot->GetTextColor().Blue());
        textColorButton->setButtonColor(tc);
        textColorButton->setEnabled(true);
        textColorOpacity->setGradientColor(tc);
    }
    textColorOpacity->setValue(annot->GetTextColor().Alpha());
    textColorOpacity->blockSignals(false);

    // Set the use foreground color check box.
    useForegroundColorCheckBox->blockSignals(true);
    useForegroundColorCheckBox->setChecked(annot->GetUseForegroundForTextColor());
    useForegroundColorCheckBox->blockSignals(false);

    // Set the label text.
    const stringVector &annotText = annot->GetText();
    if(annotText.size() > 1)
    {
        labelLineEdit->setText(annotText[0].c_str());
        timeFormatLineEdit->setText(annotText[1].c_str());
    }
    else
    {
        labelLineEdit->setText("");
        timeFormatLineEdit->setText("");
    }

    // Set the time display combo box.
    timeDisplayComboBox->blockSignals(true);
    int timeDisplay = ((annot->GetIntAttribute1() >> 2) & 3);
    timeDisplayComboBox->setCurrentItem(timeDisplay);
    timeDisplayComboBox->blockSignals(false);

    // Set the visible check box.
    visibleCheckBox->blockSignals(true);
    visibleCheckBox->setChecked(annot->GetVisible());
    visibleCheckBox->blockSignals(false);

    // Set the rounded check box.
    roundedCheckBox->blockSignals(true);
    roundedCheckBox->setChecked(annot->GetIntAttribute1() & ROUNDED_MASK);
    roundedCheckBox->blockSignals(false);

    // Set the shaded check box.
    shadedCheckBox->blockSignals(true);
    shadedCheckBox->setChecked(annot->GetIntAttribute1() & SHADED_MASK);
    shadedCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:08 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatLineEdit.
//
//   Brad Whitlock, Thu Feb 24 16:37:01 PST 2005
//   I changed how time format gets put into the object.
//
// ****************************************************************************

void
QvisTimeSliderInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        // Get the new position
        GetScreenPosition(positionEdit, "Lower left");
    }

    if(which_widget == 1 || doAll)
    {
        stringVector sv(annot->GetText());
        if(sv.size() > 1)
        {
            sv[0] = labelLineEdit->text().latin1();
            sv[1] = timeFormatLineEdit->text().latin1();
        }
        else
        {
            sv.clear();
            sv.push_back(labelLineEdit->text().latin1());
            sv.push_back(timeFormatLineEdit->text().latin1());
        }

        annot->SetText(sv);
    }
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisTimeSliderInterface::positionChanged
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
QvisTimeSliderInterface::positionChanged(float x, float y)
{
    float pos[3] = {x, y, 0.f};
    annot->SetPosition(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::widthChanged
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
QvisTimeSliderInterface::widthChanged(int w)
{
    float pos2[3];
    pos2[0] = float(w) * 0.01f;
    pos2[1] = annot->GetPosition2()[1];
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::heightChanged
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
QvisTimeSliderInterface::heightChanged(int h)
{
    float pos2[3];
    pos2[0] = annot->GetPosition2()[0];
    pos2[1] = float(h) * 0.01f;
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::labelChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   either of the label line edits.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::labelChanged()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::timeFormatChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   time format line edit.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 12, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Feb 24 16:37:32 PST 2005
//   I made it use the same value as for setting the label.
//
// ****************************************************************************

void
QvisTimeSliderInterface::timeFormatChanged()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::startColorChanged
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
QvisTimeSliderInterface::startColorChanged(const QColor &c)
{
    int a = annot->GetColor1().Alpha();
    ColorAttribute c1(c.red(), c.green(), c.blue(), a);
    annot->SetColor1(c1);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::startOpacityChanged
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
QvisTimeSliderInterface::startOpacityChanged(int opacity)
{
    ColorAttribute c1(annot->GetColor1());
    c1.SetAlpha(opacity);
    annot->SetColor1(c1);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::endColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new end color is
//   selected.
//
// Arguments:
//   c : The new end color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::endColorChanged(const QColor &c)
{
    int a = annot->GetColor2().Alpha();
    ColorAttribute c2(c.red(), c.green(), c.blue(), a);
    annot->SetColor2(c2);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::endOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new end opacity is
//   selected.
//
// Arguments:
//   opacity : The new end opacity.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::endOpacityChanged(int opacity)
{
    ColorAttribute c2(annot->GetColor2());
    c2.SetAlpha(opacity);
    annot->SetColor2(c2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::timeDisplayChanged
//
// Purpose: 
//   This is a Qt slot function that is called when we change the time display.
//
// Arguments:
//   timeDisplay : The new time display method.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:02:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::timeDisplayChanged(int timeDisplay)
{
    // Store the timeDisplay int in bits 2,3 of IntAttribute1.
    int val = (annot->GetIntAttribute1() & (~12)) | ((timeDisplay & 3) << 2);
    annot->SetIntAttribute1(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::textColorChanged
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
QvisTimeSliderInterface::textColorChanged(const QColor &c)
{
    int a = annot->GetTextColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetTextColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::textOpacityChanged
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
QvisTimeSliderInterface::textOpacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetTextColor());
    tc.SetAlpha(opacity);
    annot->SetTextColor(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::useForegroundColorToggled
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
QvisTimeSliderInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::visibilityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the visibility toggle is
//   changed.
//
// Arguments:
//   val : The visibility flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::visibilityToggled(bool val)
{
    annot->SetVisible(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::roundedToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the rounded toggle is
//   changed.
//
// Arguments:
//   val : The new rounded flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::roundedToggled(bool val)
{
    int v = (annot->GetIntAttribute1() & (~1)) | (val?1:0);
    annot->SetIntAttribute1(v);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTimeSliderInterface::shadedToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the shaded toggle is
//   changed.
//
// Arguments:
//   val : The new shaded flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:49:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeSliderInterface::shadedToggled(bool val)
{
    int v = (annot->GetIntAttribute1() & (~2)) | ((val?1:0) << 1);
    annot->SetIntAttribute1(v);
    SetUpdate(false);
    Apply();
}
