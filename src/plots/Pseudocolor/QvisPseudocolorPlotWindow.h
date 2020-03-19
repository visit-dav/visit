// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PSEUDOCOLOR_WINDOW_H
#define QVIS_PSEUDOCOLOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

class QGroupBox;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QSpinBox;
class QvisOpacitySlider;
class QvisColorTableWidget;
class QvisPointControl;
class QvisLineWidthWidget;
class QvisVariableButton;
class QvisColorButton;
class QvisCollapsibleLayout;

class PseudocolorAttributes;


// ****************************************************************************
// Class: QvisPseudocolorPlotWindow
//
// Purpose:
//   This class is a postable window that watches pseudocolot plot
//   attributes and always represents their current state.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:16:38 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Thu Dec 14 17:04:25 PST 2000
//   Added support for setting opacity.
//
//   Eric Brugger, Wed Mar 14 06:59:25 PST 2001
//   I added a plot type to the constructor for use with the viewer
//   proxy.
//
//   Brad Whitlock, Sat Jun 16 15:17:12 PST 2001
//   I added a color table button.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//   I added a limits combo box.
//
//   Brad Whitlock, Fri Feb 15 10:27:55 PDT 2002
//   Removed a method.
//
//   Jeremy Meredith, Tue Dec 10 10:22:40 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004
//   Replace individual point-size related widgets and associated slots
//   with QvisPointControl
//
//   Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//   Added a new slot to handle a new signal from QvisPointControl.
//
//   Jeremy Meredith, Wed Nov 26 11:28:24 EST 2008
//   Added line style/width controls.
//
//   Jeremy Meredith, Fri Feb 20 15:14:29 EST 2009
//   Added support for using per-color alpha values from a color table
//   (instead of just a single global opacity for the whole plot).
//   There's a new toggle for this, and it overrides the whole-plot opacity.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:02:39 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
//   Eric Brugger, Wed Oct 26 09:12:16 PDT 2016
//   I modified the plot to support independently setting the point style
//   for the two end points of lines.
//
//   Cyrus Harrison, Wed Nov  2 19:09:51 PDT 2016
//   Remove tubeRadiusVarLabel, the check box used for this includes a label.
//
//   Kathleen Biagas, Wed Dec 26 13:08:45 PST 2018
//   Add belowMinColor, aboveMaxColor, belowMinToggle, aboveMaxToggle.
//
//   Kathleen Biagas, Tue Aug 27 09:15:38 PDT 2019
//   Added lineSettings and endPointSettings methods which allow changing the
//   visibility and/or enabled state of certain controls based on the rendering
//   type(s) in effect.
//
// ****************************************************************************

class QvisPseudocolorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPseudocolorPlotWindow(const int type, PseudocolorAttributes *_pcAtts,
                              const QString &caption = QString::null,
                              const QString &shortName = QString::null,
                              QvisNotepadArea *notepad = 0);
    virtual ~QvisPseudocolorPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:

    void CreateGeometryTab(QWidget *);
    void CreateDataTab(QWidget *);
    void CreateExtrasTab(QWidget *);

    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
private slots:

    void scaleClicked(int scale);
    void processSkewText();

    void limitsSelectChanged(int);
    void minToggled(bool on);
    void maxToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();
    void belowMinToggled(bool);
    void aboveMaxToggled(bool);
    void belowMinColorChanged(const QColor &);
    void aboveMaxColorChanged(const QColor &);

    void centeringClicked(int button);

    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);

    void opacityTypeChanged(int val);
    void opacityVariableChanged(const QString &var);
    void opacityChanged(int opacity, const void*);

    void opacityMinToggled(bool);
    void opacityMaxToggled(bool);
    void processOpacityVarMin();
    void processOpacityVarMax();

    void pointTypeChanged(int index);
    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

    void lineTypeChanged(int newType);
    void lineWidthChanged(int newWidth);

    void tubeRadiusSizeTypeChanged(int v);
    void tubeRadiusProcessText();
    void tubeRadiusVarToggled(bool val);
    void tubeRadiusVarChanged(const QString &var);
    void tubeRadiusVarRatioProcessText();
    void tubeResolutionChanged(int val);

    void tailStyleChanged(int newStyle);
    void headStyleChanged(int newStyle);

    void endPointRadiusProcessText();
    void endPointRadiusSizeTypeChanged(int v);
    void endPointRatioProcessText();
    void endPointRadiusVarToggled(bool val);
    void endPointRadiusVarChanged(const QString &var);
    void endPointRadiusVarRatioProcessText();
    void endPointResolutionChanged(int val);

    void smoothingLevelChanged(int index);
    void renderSurfacesChanged(bool);
    void renderWireframeChanged(bool);
    void wireframeColorChanged(const QColor &color);
    void renderPointsChanged(bool);
    void pointColorChanged(const QColor &color);

    void legendToggled(bool on);
    void lightingToggled(bool on);

private:
    int                   plotType;
    PseudocolorAttributes *pcAtts;

    QvisCollapsibleLayout *propertyLayout;

    QButtonGroup          *scalingButtons;
    QLineEdit             *skewLineEdit;

    QComboBox             *limitsSelect;
    QCheckBox             *minToggle;
    QCheckBox             *maxToggle;
    QLineEdit             *maxLineEdit;
    QLineEdit             *minLineEdit;
    QCheckBox             *belowMinToggle;
    QvisColorButton       *belowMinColor;
    QCheckBox             *aboveMaxToggle;
    QvisColorButton       *aboveMaxColor;

    QButtonGroup          *centeringButtons;

    QvisColorTableWidget  *colorTableWidget;

    QComboBox *opacityType;
    QLabel    *opacityVarLabel;
    QvisVariableButton *opacityVar;
    QvisOpacitySlider *opacitySlider;
    QGroupBox *opacityMinMaxGroup;
    QCheckBox *opacityMinToggle;
    QCheckBox *opacityMaxToggle;
    QLineEdit *opacityVarMin;
    QLineEdit *opacityVarMax;

    // QButtonGroup          *opacityButtons;
    // QLabel                *opacitySliderLabel;
    // QvisOpacitySlider     *opacitySlider;

    QvisPointControl      *pointControl;

    QLabel                *lineTypeLabel;
    QComboBox             *lineType;

    QLabel                *lineWidthLabel;
    QvisLineWidthWidget   *lineWidth;


    QLabel             *tubeRadiusLabel;
    QLineEdit          *tubeRadius;
    QComboBox          *tubeRadiusSizeType;

    QCheckBox          *tubeRadiusVarEnabled;
    QvisVariableButton *tubeRadiusVar;
    QLabel             *tubeRadiusVarRatioLabel;
    QLineEdit          *tubeRadiusVarRatio;

    QLabel             *tubeResolutionLabel;
    QSpinBox           *tubeResolution;

    QLabel    *tailStyleLabel;
    QComboBox *tailStyle;
    QLabel    *headStyleLabel;
    QComboBox *headStyle;

    QLabel    *endPointRadiusLabel;
    QLineEdit *endPointRadius;
    QComboBox *endPointRadiusSizeType;
    QLabel    *endPointRatioLabel;
    QLineEdit *endPointRatio;

    QCheckBox          *endPointRadiusVarEnabled;
    QvisVariableButton *endPointRadiusVar;
    QLabel             *endPointRadiusVarRatioLabel;
    QLineEdit          *endPointRadiusVarRatio;

    QLabel   *endPointResolutionLabel;
    QSpinBox *endPointResolution;

    QLabel                *renderLabel;
    QCheckBox             *renderSurfaces;
    QCheckBox             *renderWireframe;
    QvisColorButton       *wireframeRenderColor;
    QCheckBox             *renderPoints;
    QvisColorButton       *pointsRenderColor;
    QButtonGroup          *smoothingLevelButtons;

    QCheckBox             *legendToggle;
    QCheckBox             *lightingToggle;

    void lineSettings(void);
    void endPointSettings(void);
};
#endif
