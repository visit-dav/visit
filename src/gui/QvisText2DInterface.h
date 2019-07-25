// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TEXT2D_INTERFACE_H
#define QVIS_TEXT2D_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisText2DInterface
//
// Purpose:
//   This class lets you set attributes for a time slider annotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 12:47:34 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 16 16:12:59 PDT 2011
//   Remove width and use height.
//
//   Kathleen Biagas, Mon Jul 13 13:01:18 PDT 2015
//   Add textColorLabel.
//
// ****************************************************************************

class GUI_API QvisText2DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisText2DInterface(QWidget *parent);
    virtual ~QvisText2DInterface();

    virtual QString GetName() const { return "Text"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void positionChanged(double, double);
    void heightChanged(int);
    void textChanged();
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void fontFamilyChanged(int);
    void boldToggled(bool);
    void italicToggled(bool);
    void shadowToggled(bool);
    void useForegroundColorToggled(bool);
    void visibilityToggled(bool);
private:
    QvisScreenPositionEdit *positionEdit;
    QSpinBox               *heightSpinBox;
    QLineEdit              *textLineEdit;
    QLabel                 *textColorLabel;
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QComboBox              *fontFamilyComboBox;
    QCheckBox              *boldCheckBox;
    QCheckBox              *italicCheckBox;
    QCheckBox              *shadowCheckBox;
    QCheckBox              *useForegroundColorCheckBox;
    QCheckBox              *visibleCheckBox;
};

#endif
