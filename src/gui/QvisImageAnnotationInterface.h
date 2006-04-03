#ifndef QVIS_IMAGE_ANNOTATION_INTERFACE_H
#define QVIS_IMAGE_ANNOTATION_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>

#include <qstring.h>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

class QvisColorButton;
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
//   
// ****************************************************************************

class GUI_API QvisImageAnnotationInterface:
  public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisImageAnnotationInterface(QWidget *parent, const char *name = 0);
    virtual ~QvisImageAnnotationInterface();

    virtual QString GetName() const { return "Image"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);

protected:
    virtual void UpdateControls();

private slots:
    void imageSourceChanged(const QString &);
    void imageSourceEdit(); 
    void positionStartChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void maintainAspectRatio(bool);
    void toggleOpacityColor(bool);
    void opacityColorChanged(const QColor &);
    void opacityChanged(int);
    void visibilityToggled(bool);

private:
    QString                 initialDir;
    QLineEdit              *imageSource;
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
