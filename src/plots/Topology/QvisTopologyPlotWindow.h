// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISTOPOLOGYPLOTWINDOW_H
#define QVISTOPOLOGYPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class TopologyAttributes;
class QLabel;
class QLineEdit;
class QvisOpacitySlider;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisTopologyPlotWindow
//
// Purpose:
//    Defines QvisTopologyPlotWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisTopologyPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisTopologyPlotWindow(const int type,
                         TopologyAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTopologyPlotWindow();
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
    void lineWidthChanged(int style);
    //writeHeaderCallback unknown for att (variable multiColor)
    void minOpacityChanged(int opacity, const void*);
    void minPlateauOpacityChanged(int opacity, const void*);
    void maxPlateauOpacityChanged(int opacity, const void*);
    void maxOpacityChanged(int opacity, const void*);
    void toleranceProcessText();
    void hitpercentProcessText();
  private:
    int plotType;
    QvisLineWidthWidget *lineWidth;
    //writeHeaderData unknown for att (variable multiColor)
    QvisOpacitySlider *minOpacity;
    QvisOpacitySlider *minPlateauOpacity;
    QvisOpacitySlider *maxPlateauOpacity;
    QvisOpacitySlider *maxOpacity;
    QLineEdit *tolerance;
    QLineEdit *hitpercent;
    QLabel *lineWidthLabel;
    QLabel *multiColorLabel;
    QLabel *minOpacityLabel;
    QLabel *minPlateauOpacityLabel;
    QLabel *maxPlateauOpacityLabel;
    QLabel *maxOpacityLabel;
    QLabel *toleranceLabel;
    QLabel *hitpercentLabel;

    TopologyAttributes *atts;
};



#endif
