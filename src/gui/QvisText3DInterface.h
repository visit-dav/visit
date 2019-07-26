// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TEXT3D_INTERFACE_H
#define QVIS_TEXT3D_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisText3DInterface
//
// Purpose:
//   This class lets you set attributes for a 3D text annotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thurs Nov 8 10:56:34 PDT 2007
//
// Modifications:
//   Kathleen Biagas, Mon Jul 13 13:01:18 PDT 2015
//   Add textColorLabel.
//
// ****************************************************************************

class GUI_API QvisText3DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisText3DInterface(QWidget *parent);
    virtual ~QvisText3DInterface();

    virtual QString GetName() const { return "3D text"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void textChanged();
    void positionChanged();
    void heightModeChanged(int);
    void relativeHeightChanged(int);
    void fixedHeightChanged();
    void facesCameraToggled(bool);
    void rotateZChanged(int);
    void rotateXChanged(int);
    void rotateYChanged(int);
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void useForegroundColorToggled(bool);
    void visibilityToggled(bool);
private:
    QLineEdit         *textLineEdit;
    QLineEdit         *positionEdit;
    QButtonGroup      *heightMode;
    QSpinBox          *relativeHeightSpinBox;
    QLineEdit         *fixedHeightEdit;
    QCheckBox         *facesCameraCheckBox;
    QSpinBox          *rotateZ;
    QSpinBox          *rotateX;
    QSpinBox          *rotateY;
    QLabel            *textColorLabel;
    QvisColorButton   *textColorButton;
    QvisOpacitySlider *textColorOpacity;
    QCheckBox         *useForegroundColorCheckBox;
    QCheckBox         *visibleCheckBox;
};

#endif
