#ifndef QVISRESAMPLEWINDOW_H
#define QVISRESAMPLEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ResamplePluginAttributes;
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
// Class: QvisResampleWindow
//
// Purpose: 
//   Defines QvisResampleWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Sep 30 09:09:25 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisResampleWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisResampleWindow(const int type,
                         ResamplePluginAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisResampleWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void startXProcessText();
    void endXProcessText();
    void samplesXProcessText();
    void startYProcessText();
    void endYProcessText();
    void samplesYProcessText();
    void is3DChanged(bool val);
    void startZProcessText();
    void endZProcessText();
    void samplesZProcessText();
    void tieResolverChanged(int val);
    void tieResolverVariableChanged(const QString &varName);
    void defaultValueProcessText();
    void distributedResampleChanged(bool val);
  private:
    QLineEdit *startX;
    QLineEdit *endX;
    QLineEdit *samplesX;
    QLineEdit *startY;
    QLineEdit *endY;
    QLineEdit *samplesY;
    QCheckBox *is3D;
    QLineEdit *startZ;
    QLineEdit *endZ;
    QLineEdit *samplesZ;
    QButtonGroup *tieResolver;
    QvisVariableButton *tieResolverVariable;
    QLineEdit *defaultValue;
    QCheckBox *distributedResample;
    QLabel *startXLabel;
    QLabel *endXLabel;
    QLabel *samplesXLabel;
    QLabel *startYLabel;
    QLabel *endYLabel;
    QLabel *samplesYLabel;
    QLabel *is3DLabel;
    QLabel *startZLabel;
    QLabel *endZLabel;
    QLabel *samplesZLabel;
    QLabel *tieResolverLabel;
    QLabel *tieResolverVariableLabel;
    QLabel *defaultValueLabel;
    QLabel *distributedResampleLabel;

    ResamplePluginAttributes *atts;
};



#endif
