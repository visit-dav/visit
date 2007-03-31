#ifndef QVIS_TIME_SLIDER_INTERFACE_H
#define QVIS_TIME_SLIDER_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QCheckBox;
class QComboBox;
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
//   
// ****************************************************************************

class GUI_API QvisTimeSliderInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisTimeSliderInterface(QWidget *parent, const char *name = 0);
    virtual ~QvisTimeSliderInterface();

    virtual QString GetName() const { return "Time slider"; }

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void positionChanged(float, float);
    void widthChanged(int);
    void heightChanged(int);
    void labelChanged();
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
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QCheckBox              *useForegroundColorCheckBox;
    QLineEdit              *labelLineEdit;
    QComboBox              *timeDisplayComboBox;
    QCheckBox              *visibleCheckBox;
    QCheckBox              *roundedCheckBox;
    QCheckBox              *shadedCheckBox;
};

#endif
