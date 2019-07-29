// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_LINE2D_INTERFACE_H
#define QVIS_LINE2D_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QvisColorButton;
class QvisOpacitySlider;
class QvisLineWidthWidget;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisLine2DInterface
//
// Purpose:
//   This class lets you set attributes for a line annotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 12:47:34 PDT 2003
//
// Modifications:
//   Kathleen Biagas, Mon Jul 13 13:01:18 PDT 2015
//   Add useForegroundColor, colorLabel.
//
// ****************************************************************************

class GUI_API QvisLine2DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisLine2DInterface(QWidget *parent);
    virtual ~QvisLine2DInterface();

    virtual QString GetName() const { return "2D line"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);

protected:
    virtual void UpdateControls();

private slots:
    void positionStartChanged(double, double);
    void positionEndChanged(double, double); 
    void beginArrowChanged(int);
    void endArrowChanged(int);
    void widthChanged(int);
    void colorChanged(const QColor &);
    void opacityChanged(int);
    void visibilityToggled(bool);
    void useForegroundColorToggled(bool);

private:
    QvisScreenPositionEdit *positionStartEdit;
    QvisScreenPositionEdit *positionEndEdit;
    QComboBox              *beginArrowComboBox;
    QComboBox              *endArrowComboBox;
    QvisLineWidthWidget    *widthWidget;
    QLabel                 *colorLabel;
    QvisColorButton        *colorButton;
    QvisOpacitySlider      *opacitySlider;
    QCheckBox              *visibleCheckBox;
    QCheckBox              *useForegroundColorCheckBox;
};

#endif
