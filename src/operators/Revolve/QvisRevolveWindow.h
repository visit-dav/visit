#ifndef QVISREVOLVEWINDOW_H
#define QVISREVOLVEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class RevolveAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisRevolveWindow
//
// Purpose: 
//   Defines QvisRevolveWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Dec 11 14:17:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisRevolveWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisRevolveWindow(const int type,
                         RevolveAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisRevolveWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void axisProcessText();
    void startAngleProcessText();
    void stopAngleProcessText();
    void stepsProcessText();
  private:
    QLineEdit *axis;
    QLineEdit *startAngle;
    QLineEdit *stopAngle;
    QLineEdit *steps;

    RevolveAttributes *atts;
};



#endif
