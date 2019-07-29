// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TENSOR_WINDOW_H
#define QVIS_TENSOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QLineEdit;
class QvisColorButton;
class QvisColorTableWidget;
class TensorAttributes;

// ****************************************************************************
// Class: QvisTensorPlotWindow
//
// Purpose:
//   This class is a postable window that watches tensor plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Eric Brugger, Wed Nov 24 11:39:58 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:17:26 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisTensorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisTensorPlotWindow(const int type, TensorAttributes *_vecAtts,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTensorPlotWindow();
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
    void tensorColorChanged(const QColor &color);
    void processScaleText();
    void scaleByMagnitudeToggled(bool on);
    void autoScaleToggled(bool on);
    void reduceMethodChanged(int index);
    void processNTensorsText();
    void processStrideText();
    void legendToggled(bool on);
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
private:
    int                  plotType;
    TensorAttributes     *tensorAtts;

    QvisColorButton      *tensorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableWidget *colorTableWidget;

    QLineEdit            *scaleLineEdit;
    QCheckBox            *scaleByMagnitudeToggle;
    QCheckBox            *autoScaleToggle;

    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nTensorsLineEdit;
    QLineEdit            *strideLineEdit;

    QCheckBox            *legendToggle;
};

#endif
