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

#include <QvisText2DInterface.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>

// ****************************************************************************
// Method: QvisText2DInterface::QvisText2DInterface
//
// Purpose: 
//   Constructor for the QvisText2DInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:46:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisText2DInterface::QvisText2DInterface(QWidget *parent,
    const char *name) : QvisAnnotationObjectInterface(parent, name)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(topLayout, 8, 4);
    cLayout->setSpacing(10);

    // Add controls for the position
    positionEdit = new QvisScreenPositionEdit(this, "positionEdit");
    connect(positionEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionChanged(double, double)));
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
#if 0
    heightSpinBox = new QSpinBox(1, 100, 1, this, "heightSpinBox");
    heightSpinBox->setSuffix("%");
    heightSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(heightChanged(int)));
    cLayout->addWidget(heightSpinBox, 1, 3);
    cLayout->addWidget(new QLabel(widthSpinBox, "Height",
        this), 1, 2);
#endif

    // Add controls for entering the text
    textLineEdit = new QLineEdit(this, "textLineEdit");
    connect(textLineEdit, SIGNAL(returnPressed()),
            this, SLOT(textChanged()));
    cLayout->addMultiCellWidget(textLineEdit, 2, 2, 1, 3);
    cLayout->addWidget(new QLabel(textLineEdit, "Text",
        this), 2, 0);

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(textColorButton, "Text color", this),
        3, 0, Qt::AlignLeft);
    cLayout->addWidget(textColorButton, 3, 1);
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "textColorOpacity");
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    cLayout->addMultiCellWidget(textColorOpacity, 3, 3, 2, 3);

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox("Use foreground color", this,
        "useForegroundColorCheckBox");
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addMultiCellWidget(useForegroundColorCheckBox, 4, 4, 0, 3);

    // Add controls to set the font family.
    fontFamilyComboBox = new QComboBox(this, "fontFamilyComboBox");
    fontFamilyComboBox->insertItem("Arial", 0);
    fontFamilyComboBox->insertItem("Courier", 1);
    fontFamilyComboBox->insertItem("Times", 2);
    fontFamilyComboBox->setEditable(false);
    connect(fontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(fontFamilyChanged(int)));
    cLayout->addMultiCellWidget(fontFamilyComboBox, 5, 5, 1, 3);
    cLayout->addWidget(new QLabel("Font family", this), 5, 0);

    // Add controls for font properties.
    boldCheckBox = new QCheckBox("Bold", this, "boldCheckBox");
    connect(boldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(boldToggled(bool)));
    cLayout->addWidget(boldCheckBox, 6, 0);

    italicCheckBox = new QCheckBox("Italic", this, "italicCheckBox");
    connect(italicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(italicToggled(bool)));
    cLayout->addWidget(italicCheckBox, 6, 1);

    shadowCheckBox = new QCheckBox("Shadow", this, "shadowCheckBox");
    connect(shadowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    cLayout->addWidget(shadowCheckBox, 6, 2);

    // Added a visibility toggle
    visibleCheckBox = new QCheckBox("Visible", this, "visibleCheckBox");
    connect(visibleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(visibilityToggled(bool)));
    cLayout->addWidget(visibleCheckBox, 7, 0);
}

// ****************************************************************************
// Method: QvisText2DInterface::~QvisText2DInterface
//
// Purpose: 
//   Destructor for the QvisText2DInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:47:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisText2DInterface::~QvisText2DInterface()
{
}

// ****************************************************************************
// Method: QvisText2DInterface::GetMenuText
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
// Creation:   Wed Nov 5 16:06:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QString
QvisText2DInterface::GetMenuText(const AnnotationObject &annot) const
{
    QString retval;
    if(annot.GetText().size() > 0)
        retval.sprintf("%s - %s", GetName().latin1(), annot.GetText()[0].c_str());
    else
        retval = GetName();

    return retval;
}

// ****************************************************************************
// Method: QvisText2DInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 11:48:15 PDT 2003
//
// Modifications:
//   Eric Brugger, Fri Sep 17 13:54:07 PDT 2004
//   Modify the float to percent conversion for the width and height to
//   avoid numeric issues where the percent value would be one too small.
//   
// ****************************************************************************

void
QvisText2DInterface::UpdateControls()
{
    // Set the start position.
    positionEdit->setPosition(annot->GetPosition()[0], annot->GetPosition()[1]);

    // Set the spinbox values for the width and height.  The 0.5 is added
    // to avoid numeric issues converting back and forth between float and
    // integer.
    int w(int(annot->GetPosition2()[0] * 100.f + 0.5f));
    widthSpinBox->blockSignals(true);
    widthSpinBox->setValue(w);
    widthSpinBox->blockSignals(false);
#if 0
    int h(int(annot->GetPosition2()[1] * 100.f + 0.5f));
    heightSpinBox->blockSignals(true);
    heightSpinBox->setValue(h);
    heightSpinBox->blockSignals(false);
#endif
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

    // Set the start and end labels.
    const stringVector &annotText = annot->GetText();
    if(annotText.size() > 0)
        textLineEdit->setText(annotText[0].c_str());
    else
        textLineEdit->setText("");

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

    // Set the visible check box.
    visibleCheckBox->blockSignals(true);
    visibleCheckBox->setChecked(annot->GetVisible());
    visibleCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisText2DInterface::GetCurrentValues
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
//   Brad Whitlock, Mon Mar 6 14:36:03 PST 2006
//   Added code to grab the width.
//
// ****************************************************************************

void
QvisText2DInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        // Get the new position
        GetScreenPosition(positionEdit, "Lower left");
    }

    if(which_widget == 1 || doAll)
    {
        stringVector sv;
        sv.push_back(textLineEdit->text().latin1());
        annot->SetText(sv);
    }

    if(which_widget == 2 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(widthSpinBox);
        int w = widthSpinBox->value();
        double pos2[3];
        pos2[0] = double(w) * 0.01;
        pos2[1] = annot->GetPosition2()[1];
        pos2[2] = annot->GetPosition2()[2];
        annot->SetPosition2(pos2);
    }
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisText2DInterface::positionChanged
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
QvisText2DInterface::positionChanged(double x, double y)
{
    double pos[] = {x, y, 0.};
    annot->SetPosition(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::widthChanged
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
QvisText2DInterface::widthChanged(int w)
{
    double pos2[3];
    pos2[0] = double(w) * 0.01;
    pos2[1] = annot->GetPosition2()[1];
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::heightChanged
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
QvisText2DInterface::heightChanged(int h)
{
    double pos2[3];
    pos2[0] = annot->GetPosition2()[0];
    pos2[1] = double(h) * 0.01;
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::textChanged
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
QvisText2DInterface::textChanged()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::textColorChanged
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
QvisText2DInterface::textColorChanged(const QColor &c)
{
    int a = annot->GetTextColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetTextColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::textOpacityChanged
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
QvisText2DInterface::textOpacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetTextColor());
    tc.SetAlpha(opacity);
    annot->SetTextColor(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::fontFamilyChanged
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
QvisText2DInterface::fontFamilyChanged(int family)
{
    annot->SetFontFamily((AnnotationObject::FontFamily)family);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::boldToggled
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
QvisText2DInterface::boldToggled(bool val)
{
    annot->SetFontBold(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::italicToggled
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
QvisText2DInterface::italicToggled(bool val)
{
    annot->SetFontItalic(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::shadowToggled
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
QvisText2DInterface::shadowToggled(bool val)
{
    annot->SetFontShadow(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::visibilityToggled
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
QvisText2DInterface::visibilityToggled(bool val)
{
    annot->SetVisible(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText2DInterface::useForegroundColorToggled
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
QvisText2DInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}
