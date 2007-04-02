#ifndef QVIS_LINE2D_INTERFACE_H
#define QVIS_LINE2D_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
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
//   
// ****************************************************************************

class GUI_API QvisLine2DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisLine2DInterface(QWidget *parent, const char *name = 0);
    virtual ~QvisLine2DInterface();

    virtual QString GetName() const { return "2D Line"; }
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

private:
    QvisScreenPositionEdit *positionStartEdit;
    QvisScreenPositionEdit *positionEndEdit;
    QComboBox              *beginArrowComboBox;
    QComboBox              *endArrowComboBox;
    QSpinBox               *widthSpinBox;
    QvisColorButton        *colorButton;
    QvisOpacitySlider      *opacitySlider;
    QCheckBox              *visibleCheckBox;
};

#endif
