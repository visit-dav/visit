// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FONT_ATTRIBUTES_WIDGET_H
#define QVIS_FONT_ATTRIBUTES_WIDGET_H
#include <gui_exports.h>
#include <QFrame>
#include <FontAttributes.h>

class QCheckBox;
class QComboBox;
class QNarrowLineEdit;
class QvisColorButton;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisFontAttributesWidget
//
// Purpose:
//   Widget class that makes it easier to show/update FontAttributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:54:19 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:56:16 PDT 2008
//   Changed fontHeight to fontScale. Added disableOpacity.
//
//   Brad Whitlock, Thu Jun  5 16:05:23 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Mon Apr 10 16:01:45 PDT 2017
//   Add ability to completely hide opacity widget.
//
// ****************************************************************************

class GUI_API QvisFontAttributesWidget : public QFrame
{
    Q_OBJECT
public:
    QvisFontAttributesWidget(QWidget *parent);
    virtual ~QvisFontAttributesWidget();

    void setFontAttributes(const FontAttributes &);
    const FontAttributes &getFontAttributes();

    void hideOpacity();
    void disableOpacity();
signals:
    void fontChanged(const FontAttributes &);
private slots:
    void Apply();

    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void fontFamilyChanged(int);
    void boldToggled(bool);
    void italicToggled(bool);
    void useForegroundColorToggled(bool);
private:
    void GetCurrentValues(FontAttributes &, int = -1);
    void Update(int = -1);
    bool               opacityEnabled;
    bool               opacityHidden;

    FontAttributes     atts;
    QNarrowLineEdit   *fontScale;
    QvisColorButton   *textColorButton;
    QvisOpacitySlider *textColorOpacity;
    QComboBox         *fontFamilyComboBox;
    QCheckBox         *boldCheckBox;
    QCheckBox         *italicCheckBox;
    QCheckBox         *useForegroundColorCheckBox;
};

#endif
