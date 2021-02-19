// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISHISTOGRAMPLOTWINDOW_H
#define QVISHISTOGRAMPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class HistogramAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class QvisColorButton;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisHistogramPlotWindow
//
// Purpose: 
//   Defines QvisHistogramPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: Cyrus Harrison - generated using xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//
//    Dave Pugmire, Thu Nov 01 12:39:07 EDT 2007
//    Support for log, sqrt scaling.
//   
//    Hank Childs, Tue Dec 11 19:58:39 PST 2007
//    Add support for scaling by an arbitrary variable.
//
//    Kevin Bensema, Mon Nov 11 12:42 2013
//    Added support for computing CDFs and normalizing histograms.
//
// ****************************************************************************

class QvisHistogramPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisHistogramPlotWindow(const int type,
                         HistogramAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
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
    void histogramTypeChanged(int val);
    void weightTypeChanged(int val);
    void weightVariableChanged(const QString &varName);
    void dataScaleChanged(int val);
    void binScaleChanged(int val);
    void limitsSelectChanged(int);
    void minToggled(bool on);
    void maxToggled(bool on);
    void minProcessText();
    void maxProcessText();
    void numBinsChanged(int val);
    void domainProcessText();
    void zoneProcessText();
    void useBinWidthsChanged(bool val);
    void outputTypeChanged(int val);
    void lineWidthChanged(int style);
    void colorChanged(const QColor &color);
    void normalizeChanged(bool val);
    void computeCdfChanged(bool val);
  private:
    int plotType;
    QButtonGroup        *basedOnGroup;
    QWidget             *histogramTypeWidget;
    QButtonGroup        *histogramTypeGroup;
    QWidget             *weightTypeWidget;
    QButtonGroup        *weightTypeGroup;
    QvisVariableButton  *weightVariable;
    QButtonGroup        *dataScaleGroup;
    QButtonGroup        *binScaleGroup;
    QComboBox           *limitsSelect;
    QCheckBox           *minToggle;
    QLineEdit           *minLineEdit;
    QCheckBox           *maxToggle;
    QLineEdit           *maxLineEdit;
    QSpinBox            *numBins;
    QLineEdit           *domain;
    QLineEdit           *zone;
    QCheckBox           *useBinWidths;
    QButtonGroup        *outputTypeGroup;
    QvisLineWidthWidget *lineWidth;
    QvisColorButton     *color;
    QCheckBox           *normalizeHistogram;
    QCheckBox           *computeCDF;

    QLabel              *basedOnLabel;
    QLabel              *histogramTypeLabel;
    QLabel              *weightTypeLabel;
    QLabel              *weightVariableLabel;
    QLabel              *minLabel;
    QLabel              *maxLabel;
    QLabel              *numBinsLabel;
    QLabel              *domainLabel;
    QLabel              *zoneLabel;
    QLabel              *outputTypeLabel;
    QLabel              *lineWidthLabel;
    QLabel              *colorLabel;

    QGroupBox           *histGroupBox;
    QGroupBox           *barGroupBox;
    QGroupBox           *styleGroupBox;

    HistogramAttributes *atts;
};



#endif
