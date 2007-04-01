#ifndef QVISELEVATEWINDOW_H
#define QVISELEVATEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ElevateAttributes;
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
class QvisVariableButton;

// ****************************************************************************
// Class: QvisElevateWindow
//
// Purpose: 
//   Defines QvisElevateWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Feb 1 11:37:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisElevateWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisElevateWindow(const int type,
                         ElevateAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisElevateWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void useXYLimitsChanged(bool val);
    void limitsModeChanged(int val);
    void scalingChanged(int val);
    void skewFactorProcessText();
    void minFlagChanged(bool val);
    void minProcessText();
    void maxFlagChanged(bool val);
    void maxProcessText();
    void variableChanged(const QString &varName);
  private:
    QCheckBox *useXYLimits;
    QButtonGroup *limitsMode;
    QButtonGroup *scaling;
    QLineEdit *skewFactor;
    QCheckBox *minFlag;
    QLineEdit *min;
    QCheckBox *maxFlag;
    QLineEdit *max;
    QvisVariableButton *variable;
    QLabel *useXYLimitsLabel;
    QLabel *limitsModeLabel;
    QLabel *scalingLabel;
    QLabel *skewFactorLabel;
    QLabel *minFlagLabel;
    QLabel *minLabel;
    QLabel *maxFlagLabel;
    QLabel *maxLabel;
    QLabel *variableLabel;

    ElevateAttributes *atts;
};



#endif
