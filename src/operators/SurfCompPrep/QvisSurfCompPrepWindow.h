#ifndef QVISSURFCOMPPREPWINDOW_H
#define QVISSURFCOMPPREPWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class SurfCompPrepAttributes;
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
// Class: QvisSurfCompPrepWindow
//
// Purpose: 
//   Defines QvisSurfCompPrepWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jul 1 11:03:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisSurfCompPrepWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisSurfCompPrepWindow(const int type,
                         SurfCompPrepAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSurfCompPrepWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void surfaceTypeChanged(int val);
    void coordSystemChanged(int val);
    void thetaStartProcessText();
    void thetaStopProcessText();
    void thetaStepsProcessText();
    void phiStartProcessText();
    void phiStopProcessText();
    void phiStepsProcessText();
    void startRadiusProcessText();
    void endRadiusProcessText();
    void radiusStepsProcessText();
    void xStartProcessText();
    void xStopProcessText();
    void xStepsProcessText();
    void yStartProcessText();
    void yStopProcessText();
    void yStepsProcessText();
    void zStartProcessText();
    void zStopProcessText();
    void zStepsProcessText();
  private:
    QButtonGroup *surfaceType;
    QButtonGroup *coordSystem;
    QLineEdit *thetaStart;
    QLineEdit *thetaStop;
    QLineEdit *thetaSteps;
    QLineEdit *phiStart;
    QLineEdit *phiStop;
    QLineEdit *phiSteps;
    QLineEdit *startRadius;
    QLineEdit *endRadius;
    QLineEdit *radiusSteps;
    QLineEdit *xStart;
    QLineEdit *xStop;
    QLineEdit *xSteps;
    QLineEdit *yStart;
    QLineEdit *yStop;
    QLineEdit *ySteps;
    QLineEdit *zStart;
    QLineEdit *zStop;
    QLineEdit *zSteps;

    SurfCompPrepAttributes *atts;
};



#endif
