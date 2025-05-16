// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisFontAttributesWidget.h>

#include <FontManager.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
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
//   Brad Whitlock, Tue Apr  8 14:10:36 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jun  4 16:57:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisFontAttributesWidget::QvisFontAttributesWidget(QWidget *parent) :
    QFrame(parent), atts()
{
    opacityEnabled = true;
    opacityHidden = false;

    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->setContentsMargins(0,0,0,0);
    gLayout->setSpacing(10);
    int row = 0;

    // Setup font information
    FontManager::instance().setupFonts();

    // Add controls to set the font family.
    // Make it so options appear in the list in their actual font
    fontFamilyComboBox = new QComboBox(this);
    QStandardItemModel *model = new QStandardItemModel(fontFamilyComboBox);
    FontManager::instance().setupItemModel(model);
    fontFamilyComboBox->setModel(model);
    fontFamilyComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    fontFamilyComboBox->setMinimumContentsLength(5);
    connect(fontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(fontFamilyChanged(int)));
    gLayout->addWidget(new QLabel(tr("Font name"), this), row, 0);
    gLayout->addWidget(fontFamilyComboBox, row, 1);

    // Add control for text font height
    fontScale = new QNarrowLineEdit(this);
    connect(fontScale, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    gLayout->addWidget(fontScale, row, 3);
    QLabel *fsLabel = new QLabel(tr("Font scale"), this);
    fsLabel->setBuddy(fontScale);
    gLayout->addWidget(fsLabel, row, 2);
    ++row;

    // Set up some preview text using the currently selected font
    // but increased slightly in size
    previewText = new QLabel(QString::fromUtf8(FontManager::sampleText()));
    int defaultPointSize = QApplication::font().pointSize();
    QFont font = fontFamilyComboBox->model()->index(0, 0).data(Qt::FontRole).value<QFont>();
    font.setPointSize(defaultPointSize+4);
    previewText->setFont(font);

    // Should really dispaly preview text on the selected background color.
#if 0
    int r = 255, g = 255, b = 255, a = 255;
    QString style = QString("background-color: rgba(%1, %2, %3, %4);").arg(r).arg(g).arg(b).arg(a);
    previewText->setStyleSheet(style);
#endif

    gLayout->addWidget(new QLabel(tr("Sample text"), this), row, 0);
    gLayout->addWidget(previewText, row, 1, 1, 4);
    ++row;

    boldCheckBox = new QCheckBox(tr("Bold"), this);
    connect(boldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(boldToggled(bool)));
    gLayout->addWidget(boldCheckBox, row, 0);

    italicCheckBox = new QCheckBox(tr("Italic"), this);
    connect(italicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(italicToggled(bool)));
    gLayout->addWidget(italicCheckBox, row, 1);

    shadowCheckBox = new QCheckBox(tr("Shadow"), this);
    connect(shadowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    gLayout->addWidget(shadowCheckBox, row, 2);
    shadowCheckBox->setEnabled(false); // disable until working
    ++row;

    useForegroundColorCheckBox = new QCheckBox(tr("Use foreground color"), this);
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    gLayout->addWidget(useForegroundColorCheckBox, row, 0, 1, 2);

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this);
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    gLayout->addWidget(textColorButton, row, 2);
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    gLayout->addWidget(textColorOpacity, row, 3, 1, 4);
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
// Method: QvisFontAttributesWidget::hideOpacity
//
// Purpose:
//   Hides the opacity slider.
//
// Programmer: Kathleen Biagas
// Creation:   April 10, 2017
//
// Modifications:
//
// ****************************************************************************

void
QvisFontAttributesWidget::hideOpacity()
{
    opacityHidden = true;
    textColorOpacity->hide();
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
        fontFamilyComboBox->setCurrentIndex(int(atts.GetFont()));
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

#if 0
    if(doAll || which_widget == FontAttributes::ID_shadow)
    {
        shadowCheckBox->blockSignals(true);
        shadowCheckBox->setChecked(atts.GetShadow());
        shadowCheckBox->blockSignals(false);
    }
#endif
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
    // We should probably also update the preview text appearence with color
    // changes. However, we should also display the preview text on the
    // current viewer background color and not the GUI's gray and until we do
    // that updating preview text color is probably not worthwhile.
    // Getting the currently selected background color involves getting state
    // from the viewer and this widget is not set up for that.

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
    QFont font = fontFamilyComboBox->model()->
        index(value, 0).data(Qt::FontRole).value<QFont>();
    int defaultPointSize = QApplication::font().pointSize();
    font.setPointSize(defaultPointSize+4);
    previewText->setFont(font);

    const FontManager::QtFontInfo& fi = FontManager::instance().fontInfo(font.family());

    boldCheckBox->blockSignals(true);
    boldCheckBox->setChecked(!fi.hasReg && fi.hasBold || font.bold());
    boldCheckBox->blockSignals(false);
    boldCheckBox->setEnabled(fi.hasReg && fi.hasBold);

    italicCheckBox->blockSignals(true);
    italicCheckBox->setChecked(!fi.hasReg && fi.hasItalic || font.italic());
    italicCheckBox->blockSignals(false);
    italicCheckBox->setEnabled(fi.hasReg && fi.hasItalic);

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
    // Make selected item appear with bold as specified
    QFont font = fontFamilyComboBox->model()->
        index(fontFamilyComboBox->currentIndex(), 0).data(Qt::FontRole).value<QFont>();
    int defaultPointSize = QApplication::font().pointSize();
    font.setPointSize(defaultPointSize+4);
    font.setBold(val);
    font.setItalic(italicCheckBox->isChecked());
    previewText->setFont(font);

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
// Programmer: Mark C. Miller, Thu May 15 18:17:17 PDT 2025
//   
// ****************************************************************************

void
QvisFontAttributesWidget::italicToggled(bool val)
{
    // Make selected item appear with bold as specified
    QFont font = fontFamilyComboBox->model()->
        index(fontFamilyComboBox->currentIndex(), 0).data(Qt::FontRole).value<QFont>();
    int defaultPointSize = QApplication::font().pointSize();
    font.setPointSize(defaultPointSize+4);
    font.setItalic(val);
    font.setBold(boldCheckBox->isChecked());
    previewText->setFont(font);

    atts.SetItalic(val);
    Apply();
}

// ****************************************************************************
// Method: QvisFontAttributesWidget::shadowToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the shadow checkbox is toggled.
//
// Arguments:
//   val : The new shadow flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 13:55:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFontAttributesWidget::shadowToggled(bool val)
{
    // Make selected item appear with bold as specified
    QFont font = fontFamilyComboBox->model()->
        index(fontFamilyComboBox->currentIndex(), 0).data(Qt::FontRole).value<QFont>();
    int defaultPointSize = QApplication::font().pointSize();
    font.setPointSize(defaultPointSize+4);
    //font.setShadow(val);
    previewText->setFont(font);

    //atts.SetShadow(val);
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
