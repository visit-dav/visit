// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_IMAGE_ANNOTATION_INTERFACE_H
#define QVIS_IMAGE_ANNOTATION_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>
#include <QString>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

class QvisColorButton;
class QvisDialogLineEdit;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisImageAnnotationInterface
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
//   Brad Whitlock, Thu Jun 26 14:57:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisImageAnnotationInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisImageAnnotationInterface(QWidget *parent);
    virtual ~QvisImageAnnotationInterface();

    virtual QString GetName() const { return "Image"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);

protected:
    virtual void UpdateControls();

private slots:
    void imageSourceChanged(const QString &);
    void positionStartChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void maintainAspectRatio(bool);
    void toggleOpacityColor(bool);
    void opacityColorChanged(const QColor &);
    void opacityChanged(int);
    void visibilityToggled(bool);

private:
    QvisDialogLineEdit     *imageSource;
    QvisScreenPositionEdit *positionStartEdit;
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QCheckBox              *opacityCheck;
    QvisColorButton        *opacityColorButton;
    QvisOpacitySlider      *opacitySlider;
    QCheckBox              *visibleCheckBox;
    QCheckBox              *linkedWH;
};

#endif
