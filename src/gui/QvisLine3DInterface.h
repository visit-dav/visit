// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_LINE3D_INTERFACE_H
#define QVIS_LINE3D_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisLine3DInterface
//
// Purpose:
//   This class lets you set attributes for a line annotation.
//
// Notes:      
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:39:07 PDT 2015
//   Add support for arrow and tube style lines.
//
// ****************************************************************************

class GUI_API QvisLine3DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisLine3DInterface(QWidget *parent);
    virtual ~QvisLine3DInterface();

    virtual QString GetName() const { return "3D line"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);

protected:
    virtual void UpdateControls();

private slots:
    void point1Changed();
    void point2Changed();
    void widthChanged(int);
    void colorChanged(const QColor &);
    void opacityChanged(int);
    void visibilityToggled(bool);
    void useForegroundColorToggled(bool);
    void beginArrowToggled(bool);
    void endArrowToggled(bool);
    void arrow1ResolutionChanged(int);
    void arrow2ResolutionChanged(int);
    void arrow1RadiusChanged();
    void arrow2RadiusChanged();
    void arrow1HeightChanged();
    void arrow2HeightChanged();
    void lineTypeChanged(int);
    void tubeQualityChanged(int);
    void tubeRadiusChanged();
   

private:
    QLineEdit              *point1Edit;
    QLineEdit              *point2Edit;
    QComboBox              *lineType;
    QvisLineWidthWidget    *widthWidget;
    QLabel                 *widthLabel;
    QComboBox              *tubeQuality;
    QLabel                 *tubeQualLabel;
    QLineEdit              *tubeRadius;
    QLabel                 *tubeRadLabel;
    QCheckBox              *useForegroundColorCheckBox;
    QLabel                 *colorLabel;
    QvisColorButton        *colorButton;
    QvisOpacitySlider      *opacitySlider;

    QCheckBox              *beginArrow;
    QLabel                 *res1Label;
    QSpinBox               *arrow1Resolution;
    QLabel                 *rad1Label;
    QLineEdit              *arrow1Radius;
    QLabel                 *height1Label;
    QLineEdit              *arrow1Height;
    QCheckBox              *endArrow;
    QLabel                 *res2Label;
    QSpinBox               *arrow2Resolution;
    QLabel                 *rad2Label;
    QLineEdit              *arrow2Radius;
    QLabel                 *height2Label;
    QLineEdit              *arrow2Height;

    QCheckBox              *visibleCheckBox;
};

#endif
