#ifndef QVIS_TEXT2D_INTERFACE_H
#define QVIS_TEXT2D_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QComboBox;
class QCheckBox;
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
//   
// ****************************************************************************

class GUI_API QvisText2DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisText2DInterface(QWidget *parent, const char *name = 0);
    virtual ~QvisText2DInterface();

    virtual QString GetName() const { return "Text"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void positionChanged(float, float);
    void widthChanged(int);
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
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QLineEdit              *textLineEdit;
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
