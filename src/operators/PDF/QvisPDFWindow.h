// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISPDFWINDOW_H
#define QVISPDFWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class PDFAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;

// ****************************************************************************
// Class: QvisPDFWindow
//
// Purpose:
//    Defines QvisPDFWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
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
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
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
    QWidget      *var1Scaling;
    QButtonGroup *var1ScalingButtonGroup;
    QLineEdit *var1SkewFactor;
    QLineEdit *var1NumSamples;
    QLineEdit *var2;
    QCheckBox *var2MinFlag;
    QCheckBox *var2MaxFlag;
    QLineEdit *var2Min;
    QLineEdit *var2Max;
    QWidget      *var2Scaling;
    QButtonGroup *var2ScalingButtonGroup;
    QLineEdit *var2SkewFactor;
    QLineEdit *var2NumSamples;
    QWidget      *numAxes;
    QButtonGroup *numAxesButtonGroup;
    QLineEdit *var3;
    QCheckBox *var3MinFlag;
    QCheckBox *var3MaxFlag;
    QLineEdit *var3Min;
    QLineEdit *var3Max;
    QWidget      *var3Scaling;
    QButtonGroup *var3ScalingButtonGroup;
    QLineEdit *var3SkewFactor;
    QLineEdit *var3NumSamples;
    QCheckBox *scaleCube;
    QWidget      *densityType;
    QButtonGroup *densityTypeButtonGroup;
    QLabel *var1Label;
    QLabel *var1MinLabel;
    QLabel *var1MaxLabel;
    QLabel *var1ScalingLabel;
    QLabel *var1SkewFactorLabel;
    QLabel *var1NumSamplesLabel;
    QLabel *var2Label;
    QLabel *var2MinLabel;
    QLabel *var2MaxLabel;
    QLabel *var2ScalingLabel;
    QLabel *var2SkewFactorLabel;
    QLabel *var2NumSamplesLabel;
    QLabel *numAxesLabel;
    QLabel *var3Label;
    QLabel *var3MinLabel;
    QLabel *var3MaxLabel;
    QLabel *var3ScalingLabel;
    QLabel *var3SkewFactorLabel;
    QLabel *var3NumSamplesLabel;
    QLabel *densityTypeLabel;

    PDFAttributes *atts;
};



#endif
