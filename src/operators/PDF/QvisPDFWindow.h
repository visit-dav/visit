#ifndef QVISPDFWINDOW_H
#define QVISPDFWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class PDFAttributes;
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
// Class: QvisPDFWindow
//
// Purpose: 
//   Defines QvisPDFWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisPDFWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisPDFWindow(const int type,
                         PDFAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisPDFWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void var1ProcessText();
    void var1MinFlagChanged(bool val);
    void var1MaxFlagChanged(bool val);
    void var1MinProcessText();
    void var1MaxProcessText();
    void var1ScalingChanged(int val);
    void var1SkewFactorProcessText();
    void var1NumSamplesProcessText();
    void var2ProcessText();
    void var2MinFlagChanged(bool val);
    void var2MaxFlagChanged(bool val);
    void var2MinProcessText();
    void var2MaxProcessText();
    void var2ScalingChanged(int val);
    void var2SkewFactorProcessText();
    void var2NumSamplesProcessText();
    void numAxesChanged(int val);
    void var3ProcessText();
    void var3MinFlagChanged(bool val);
    void var3MaxFlagChanged(bool val);
    void var3MinProcessText();
    void var3MaxProcessText();
    void var3ScalingChanged(int val);
    void var3SkewFactorProcessText();
    void var3NumSamplesProcessText();
    void scaleCubeChanged(bool val);
    void densityTypeChanged(int val);
  private:
    QLineEdit *var1;
    QCheckBox *var1MinFlag;
    QCheckBox *var1MaxFlag;
    QLineEdit *var1Min;
    QLineEdit *var1Max;
    QButtonGroup *var1Scaling;
    QLineEdit *var1SkewFactor;
    QLineEdit *var1NumSamples;
    QLineEdit *var2;
    QCheckBox *var2MinFlag;
    QCheckBox *var2MaxFlag;
    QLineEdit *var2Min;
    QLineEdit *var2Max;
    QButtonGroup *var2Scaling;
    QLineEdit *var2SkewFactor;
    QLineEdit *var2NumSamples;
    QButtonGroup *numAxes;
    QLineEdit *var3;
    QCheckBox *var3MinFlag;
    QCheckBox *var3MaxFlag;
    QLineEdit *var3Min;
    QLineEdit *var3Max;
    QButtonGroup *var3Scaling;
    QLineEdit *var3SkewFactor;
    QLineEdit *var3NumSamples;
    QCheckBox *scaleCube;
    QButtonGroup *densityType;
    QLabel *var1Label;
    QLabel *var1MinFlagLabel;
    QLabel *var1MaxFlagLabel;
    QLabel *var1MinLabel;
    QLabel *var1MaxLabel;
    QLabel *var1ScalingLabel;
    QLabel *var1SkewFactorLabel;
    QLabel *var1NumSamplesLabel;
    QLabel *var2Label;
    QLabel *var2MinFlagLabel;
    QLabel *var2MaxFlagLabel;
    QLabel *var2MinLabel;
    QLabel *var2MaxLabel;
    QLabel *var2ScalingLabel;
    QLabel *var2SkewFactorLabel;
    QLabel *var2NumSamplesLabel;
    QLabel *numAxesLabel;
    QLabel *var3Label;
    QLabel *var3MinFlagLabel;
    QLabel *var3MaxFlagLabel;
    QLabel *var3MinLabel;
    QLabel *var3MaxLabel;
    QLabel *var3ScalingLabel;
    QLabel *var3SkewFactorLabel;
    QLabel *var3NumSamplesLabel;
    QLabel *scaleCubeLabel;
    QLabel *densityTypeLabel;

    PDFAttributes *atts;
};



#endif
