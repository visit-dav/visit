#ifndef QVISHISTOGRAMPLOTWINDOW_H
#define QVISHISTOGRAMPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class HistogramAttributes;
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
// Class: QvisHistogramPlotWindow
//
// Purpose: 
//   Defines QvisHistogramPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed May 24 09:22:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class QvisHistogramPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisHistogramPlotWindow(const int type,
                         HistogramAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisHistogramPlotWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void basedOnChanged(int val);
    void specifyRangeChanged(bool val);
    void minProcessText();
    void maxProcessText();
    void outputTypeChanged(int val);
    void numBinsProcessText();
    void twoDAmountChanged(int val);
    void domainProcessText();
    void zoneProcessText();
    void lineStyleChanged(int style);
    void lineWidthChanged(int style);
    void colorChanged(const QColor &color);
  private:
    int plotType;
    QButtonGroup *basedOn;
    QCheckBox *specifyRange;
    QLineEdit *min;
    QLineEdit *max;
    QButtonGroup *outputType;
    QLineEdit *numBins;
    QButtonGroup *twoDAmount;
    QLineEdit *domain;
    QLineEdit *zone;
    QvisLineStyleWidget *lineStyle;
    QvisLineWidthWidget *lineWidth;
    QvisColorButton *color;
    QLabel *basedOnLabel;
    QLabel *specifyRangeLabel;
    QLabel *minLabel;
    QLabel *maxLabel;
    QLabel *outputTypeLabel;
    QLabel *numBinsLabel;
    QLabel *twoDAmountLabel;
    QLabel *domainLabel;
    QLabel *zoneLabel;
    QLabel *lineStyleLabel;
    QLabel *lineWidthLabel;
    QLabel *colorLabel;

    HistogramAttributes *atts;
};



#endif
