#ifndef QVISSMOOTHWINDOW_H
#define QVISSMOOTHWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class SmoothOperatorAttributes;
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
// Class: QvisSmoothWindow
//
// Purpose: 
//   Defines QvisSmoothWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Sun Aug 14 11:59:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisSmoothWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisSmoothWindow(const int type,
                         SmoothOperatorAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSmoothWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void numIterationsProcessText();
    void relaxationFactorProcessText();
    void convergenceProcessText();
    void maintainFeaturesChanged(bool val);
    void featureAngleProcessText();
    void edgeAngleProcessText();
    void smoothBoundariesChanged(bool val);
  private:
    QLineEdit *numIterations;
    QLineEdit *relaxationFactor;
    QLineEdit *convergence;
    QCheckBox *maintainFeatures;
    QLineEdit *featureAngle;
    QLineEdit *edgeAngle;
    QCheckBox *smoothBoundaries;
    QLabel *numIterationsLabel;
    QLabel *relaxationFactorLabel;
    QLabel *convergenceLabel;
    QLabel *maintainFeaturesLabel;
    QLabel *featureAngleLabel;
    QLabel *edgeAngleLabel;
    QLabel *smoothBoundariesLabel;

    SmoothOperatorAttributes *atts;
};



#endif
