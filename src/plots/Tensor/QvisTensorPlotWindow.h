// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_TENSOR_WINDOW_H
#define QVIS_TENSOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QGroupBox;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QvisColorButton;
class QvisColorTableWidget;
class QvisLineWidthWidget;

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
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTensorPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();

protected:
    void CreateSamplingTab(QWidget *);
    void CreateDataTab(QWidget *);
    void CreateGeometryTab(QWidget *);


    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

private slots:
    void locationMethodChanged(int index);
    void reduceMethodChanged(int index);
    void processNTensorsText();
    void processStrideText();
    void limitToOrigToggled(bool);

    void limitsSelectChanged(int);
    void minToggled(bool on);
    void maxToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();

    void tensorColorChanged(const QColor &color);
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
    void legendToggled(bool);

    void processScaleText();
    void scaleByMagnitudeToggled(bool);
    void autoScaleToggled(bool);

private:
    int                  plotType;
    TensorAttributes     *tensorAtts;

    QButtonGroup         *locationButtonGroup;
    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nTensorsLineEdit;
    QLineEdit            *strideLineEdit;
    QRadioButton         *strideRB;
    QCheckBox            *limitToOrigToggle;

    QGroupBox            *limitsGroup;
    QCheckBox            *minToggle;
    QCheckBox            *maxToggle;
    QComboBox            *limitsSelect;
    QLineEdit            *maxLineEdit;
    QLineEdit            *minLineEdit;

    QvisColorButton      *tensorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableWidget *colorTableWidget;
    QCheckBox            *legendToggle;

    QLineEdit            *scaleLineEdit;
    QCheckBox            *scaleByMagnitudeToggle;
    QCheckBox            *autoScaleToggle;
};

#endif
