// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TIME_SLIDER_INTERFACE_H
#define QVIS_TIME_SLIDER_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisTimeSliderInterface
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
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatLinEdit and timeFormatChanged slot.
//
//   Brad Whitlock, Fri Jul 18 16:20:40 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Mon Jul 13 13:01:18 PDT 2015
//   Add textColorLabel.
//
// ****************************************************************************

class GUI_API QvisTimeSliderInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisTimeSliderInterface(QWidget *parent);
    virtual ~QvisTimeSliderInterface();

    virtual QString GetName() const { return "Time slider"; }

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void positionChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void labelChanged();
    void timeFormatChanged();
    void startColorChanged(const QColor &);
    void startOpacityChanged(int);
    void endColorChanged(const QColor &);
    void endOpacityChanged(int);
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void visibilityToggled(bool);
    void roundedToggled(bool);
    void shadedToggled(bool);
    void timeDisplayChanged(int);
    void useForegroundColorToggled(bool);
private:
    QvisScreenPositionEdit *positionEdit;
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QvisColorButton        *startColorButton;
    QvisOpacitySlider      *startColorOpacity;
    QvisColorButton        *endColorButton;
    QvisOpacitySlider      *endColorOpacity;
    QLabel                 *textColorLabel;
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QCheckBox              *useForegroundColorCheckBox;
    QLineEdit              *labelLineEdit;
    QLineEdit              *timeFormatLineEdit;
    QComboBox              *timeDisplayComboBox;
    QCheckBox              *visibleCheckBox;
    QCheckBox              *roundedCheckBox;
    QCheckBox              *shadedCheckBox;
};

#endif
