/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <QvisFontAttributesWidget.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QNarrowLineEdit.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>

// ****************************************************************************
// Method: QvisFontAttributesWidget::QvisFontAttributesWidget
//
// Purpose: 
//   Constructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:47:24 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:51:59 PDT 2008
//   Changed height to scale.
//
// ****************************************************************************

QvisFontAttributesWidget::QvisFontAttributesWidget(QWidget *parent, const char *name) :
    QFrame(parent,name), atts()
{
    opacityEnabled = true;

    QGridLayout *gLayout = new QGridLayout(this, 2, 6);
    gLayout->setSpacing(5);
    int row = 0;

    // Add controls to set the font family.
    fontFamilyComboBox = new QComboBox(this, "fontFamilyComboBox");
    fontFamilyComboBox->insertItem("Arial", 0);
    fontFamilyComboBox->insertItem("Courier", 1);
    fontFamilyComboBox->insertItem("Times", 2);
    fontFamilyComboBox->setEditable(false);
    connect(fontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(fontFamilyChanged(int)));
    gLayout->addWidget(new QLabel(fontFamilyComboBox, "Font name", this), row, 0);
    gLayout->addWidget(fontFamilyComboBox, row, 1);

    // Add control for text font height
    fontScale = new QNarrowLineEdit(this, "fontScale");
    connect(fontScale, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    gLayout->addWidget(fontScale, row, 3);
    gLayout->addWidget(new QLabel(fontScale, "Font scale",
        this), row, 2);

    boldCheckBox = new QCheckBox("Bold", this, "boldCheckBox");
    connect(boldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(boldToggled(bool)));
    gLayout->addWidget(boldCheckBox, row, 4);

    italicCheckBox = new QCheckBox("Italic", this, "italicCheckBox");
    connect(italicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(italicToggled(bool)));
    gLayout->addWidget(italicCheckBox, row, 5);
    ++row;

    useForegroundColorCheckBox = new QCheckBox("Use foreground color", this,
        "useForegroundColorCheckBox");
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    gLayout->addMultiCellWidget(useForegroundColorCheckBox, row, row, 0, 1);

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    gLayout->addWidget(textColorButton, row, 2);
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "textColorOpacity");
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    gLayout->addMultiCellWidget(textColorOpacity, row, row, 3, 5);
    ++row;
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::~QvisFontAttributesWidget
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:48:15 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisFontAttributesWidget::~QvisFontAttributesWidget()
{
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::disableOpacity
//
// Purpose: 
//   Disables the opacity slider.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 09:58:47 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::disableOpacity()
{
    opacityEnabled = false;
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::getFontAttributes
//
// Purpose: 
//   Get the current font attributes and return them.
//
// Arguments:
//
// Returns:    The current font attributes.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:48:24 PST 2008
//
// Modifications:
//   
// ****************************************************************************

const FontAttributes &
QvisFontAttributesWidget::getFontAttributes()
{
    GetCurrentValues(atts, -1);
    return atts;
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::setFontAttributes
//
// Purpose: 
//   Set the current font attributes and update the widget.
//
// Arguments:
//   fa : The new font attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:48:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::setFontAttributes(const FontAttributes &fa)
{
    atts = fa;
    Update(-1);
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::Update
//
// Purpose: 
//   Called when we want to update the widgets with the FontAttributes values.
//
// Arguments:
//   which_widget : The widget to update or -1 for all.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:47:38 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:52:58 PDT 2008
//   Changed height to scale. Set the opacity enabled state accordingly.
//
// ****************************************************************************

void
QvisFontAttributesWidget::Update(int which_widget)
{
    bool doAll = which_widget == -1;

    if(doAll || which_widget == FontAttributes::ID_font)
    {
        fontFamilyComboBox->blockSignals(true);
        fontFamilyComboBox->setCurrentItem(int(atts.GetFont()));
        fontFamilyComboBox->blockSignals(false);
    }

    if(doAll || which_widget == FontAttributes::ID_scale)
    {
        fontScale->setText(QString().setNum(atts.GetScale()));
    }

    if(doAll || which_widget == FontAttributes::ID_useForegroundColor)
    {
        useForegroundColorCheckBox->blockSignals(true);
        useForegroundColorCheckBox->setChecked(atts.GetUseForegroundColor());
        useForegroundColorCheckBox->blockSignals(false);

        textColorOpacity->setEnabled(!atts.GetUseForegroundColor() && opacityEnabled);
        textColorButton->setEnabled(!atts.GetUseForegroundColor());
    }

    if(doAll || which_widget == FontAttributes::ID_color)
    {
        textColorOpacity->blockSignals(true);
        QColor tc(atts.GetColor().Red(),
                  atts.GetColor().Green(),
                  atts.GetColor().Blue());
        textColorButton->setButtonColor(tc);

        textColorOpacity->setGradientColor(tc);
        textColorOpacity->setValue(atts.GetColor().Alpha());
        textColorOpacity->blockSignals(false);
    }

    if(doAll || which_widget == FontAttributes::ID_bold)
    {
        boldCheckBox->blockSignals(true);
        boldCheckBox->setChecked(atts.GetBold());
        boldCheckBox->blockSignals(false);
    }

    if(doAll || which_widget == FontAttributes::ID_italic)
    {
        italicCheckBox->blockSignals(true);
        italicCheckBox->setChecked(atts.GetItalic());
        italicCheckBox->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::GetCurrentValues
//
// Purpose: 
//   Get the current values of widgets that don't necessarily update
//   cause signals when edited.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::GetCurrentValues(FontAttributes &fa, int which_widget)
{
    bool doAll = which_widget == -1;

    if(which_widget == FontAttributes::ID_scale || doAll)
    {
        bool okay;
        double val = fontScale->displayText().toDouble(&okay);
        if(okay)
            fa.SetScale(val);
    }
}

//
// Qt slot functions
//

void
QvisFontAttributesWidget::Apply()
{
    GetCurrentValues(atts, -1);
    emit fontChanged(atts);
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::textColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::textColorChanged(const QColor &c)
{
    int a = atts.GetColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    atts.SetColor(tc);
    Update(FontAttributes::ID_color);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::textOpacityChanged(int opacity)
{
    ColorAttribute tc(atts.GetColor());
    tc.SetAlpha(opacity);
    atts.SetColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::fontFamilyChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the font family is changed.
//
// Arguments:
//   value  : The new font.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::fontFamilyChanged(int value)
{
    atts.SetFont((FontAttributes::FontName)value);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::boldToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the bold checkbox is toggled.
//
// Arguments:
//   val : The new bold flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::boldToggled(bool val)
{
    atts.SetBold(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::italicToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the italic checkbox is toggled.
//
// Arguments:
//   val : The new italic flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::italicToggled(bool val)
{
    atts.SetItalic(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::useForegroundColorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the useForegroundColor
//   check box is clicked.
//
// Arguments:
//   val : The new setting for useForegroundColor
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   Brad Whitlock, Thu Mar 27 10:10:48 PDT 2008
//   Set the opacity enabled state accordingly.
//
// ****************************************************************************

void
QvisFontAttributesWidget::useForegroundColorToggled(bool val)
{
    atts.SetUseForegroundColor(val);
    textColorOpacity->setEnabled(!atts.GetUseForegroundColor() && opacityEnabled);
    textColorButton->setEnabled(!atts.GetUseForegroundColor());
    Apply();
}
