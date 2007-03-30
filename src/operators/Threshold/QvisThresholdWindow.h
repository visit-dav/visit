#ifndef QVISTHRESHOLDWINDOW_H
#define QVISTHRESHOLDWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ThresholdAttributes;
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
// Class: QvisThresholdWindow
//
// Purpose: 
//   Defines QvisThresholdWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisThresholdWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisThresholdWindow(const int type,
                         ThresholdAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisThresholdWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void amountChanged(int val);
    void lboundProcessText();
    void uboundProcessText();
    void variableProcessText();
  private:
    QButtonGroup *amount;
    QLineEdit *lbound;
    QLineEdit *ubound;
    QLineEdit *variable;

    ThresholdAttributes *atts;
};



#endif
