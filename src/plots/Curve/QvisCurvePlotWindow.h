// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISCURVEPLOTWINDOW_H
#define QVISCURVEPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class CurveAttributes;
class QLabel;
class QNarrowLineEdit;
class QCheckBox;
class QComboBox;
class QButtonGroup;
class QRadioButton;
class QSpinBox;
class QvisColorButton;
class QvisLineWidthWidget;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisCurvePlotWindow
//
// Purpose: 
//   Defines QvisCurvePlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 23 13:27:22 PST 2003
//   Added showPoints, pointSize and pointSizeLabel widgets. 
//   Added showPointChanged and processPointSizeText slots.
//
//   Kathleen Bonnell, Thu Oct 27 15:35:11 PDT 2005 
//   Added showLegend.
//   
//   Kathleen Bonnell, Mon Oct 31 17:05:35 PST 2005
//   Added cycleColors, colorLabel.
//
//   Brad Whitlock, Mon Nov 20 13:25:30 PST 2006
//   Added controls to control symbol-based rendering.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add cue for the current location.
//
//   Brad Whitlock, Fri Jul  5 16:54:02 PDT 2013
//   Add fill color.
//
//   Kathleen Biagas, Wed Sep 11 17:17:42 PDT 2013
//   Added polarToggle and useDegreesToggle, added widgets for saving tabs.
//
// ****************************************************************************

class QvisCurvePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisCurvePlotWindow(const int type,
                         CurveAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCurvePlotWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void CreateDataTab(QWidget *);
    void CreateGeometryTab(QWidget *);
    void CreateExtrasTab(QWidget *);

    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
 
  private slots:
    void showLinesChanged(bool val);
    void lineWidthChanged(int style);
    void labelsToggled(bool val);
    void legendToggled(bool val);
    void showPointsChanged(bool val);
    void processPointSizeText();
    void pointStrideChanged(int);

    void curveColorClicked(int val);
    void curveColorChanged(const QColor &color);

    void fillModeChanged(int);

    void fillColor1Changed(const QColor &color);
    void fillColor1OpacityChanged(int opacity, const void*);

    void fillColor2Changed(const QColor &color);
    void fillColor2OpacityChanged(int opacity, const void*);

    void symbolTypeChanged(int);
    void symbolDensityChanged(int);
    void symbolFillModeChanged(int);

    void doBallTimeCueChanged(bool val);
    void ballTimeCueColorChanged(const QColor &color);
    void timeCueBallSizeProcessText();
    void doLineTimeCueChanged(bool val);
    void lineTimeCueColorChanged(const QColor &color);
    void lineTimeCueWidthChanged(int);
    void doCropTimeCueChanged(bool val);
    void timeForTimeCueProcessText();

    void polarToggled(bool val);
    void polarOrderChanged(int);
    void angleUnitsChanged(int);

  private:
    int plotType;
    QCheckBox           *showLines;
    QvisLineWidthWidget *lineWidth;
    QLabel              *lineWidthLabel;

    QCheckBox           *cycleColors;
    QButtonGroup        *curveColorButtons;
    QvisColorButton     *curveColor;

    QComboBox           *fillMode;

    QLabel              *fillLabel1;
    QvisColorButton     *fillColor1;
    QvisOpacitySlider   *fillOpacity1;

    QLabel              *fillLabel2;
    QvisColorButton     *fillColor2;
    QvisOpacitySlider   *fillOpacity2;

    QCheckBox           *labelsToggle;
    QCheckBox           *legendToggle;

    QCheckBox           *showPoints;

    QNarrowLineEdit     *pointSize;
    QLabel              *pointSizeLabel;

    QSpinBox            *pointStride;
    QLabel              *pointStrideLabel;

    QButtonGroup        *fillModeGroup;
    QRadioButton        *staticButton;
    QRadioButton        *dynamicButton;   

    QComboBox           *symbolType;
    QLabel              *symbolTypeLabel;
    QSpinBox            *symbolDensity;
    QLabel              *symbolDensityLabel;

    QCheckBox           *doBallTimeCue;
    QvisColorButton     *ballTimeCueColor;
    QLineEdit           *timeCueBallSize;
    QCheckBox           *doLineTimeCue;
    QvisColorButton     *lineTimeCueColor;
    QvisLineWidthWidget *lineTimeCueWidth;
    QCheckBox           *doCropTimeCue;
    QLineEdit           *timeForTimeCue;
    QLabel              *timeCueBallSizeLabel;
    QLabel              *lineTimeCueWidthLabel;
    QLabel              *timeForTimeCueLabel;

    QCheckBox           *polarToggle;
    QComboBox           *polarOrder;
    QLabel              *polarOrderLabel;
    QComboBox           *angleUnits;
    QLabel              *angleUnitsLabel;

    CurveAttributes *atts;
};



#endif
