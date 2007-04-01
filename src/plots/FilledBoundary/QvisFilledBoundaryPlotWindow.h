#ifndef QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#define QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class FilledBoundaryAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QListBox;
class QSlider;
class QvisColorButton;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisPointControl;

// ****************************************************************************
// Class: QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   This class is an observer window that watches boundary plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisMaterialPlotWindow with minor changes.
//
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
// Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//    Jeremy Meredith, Fri Jun 13 16:56:43 PDT 2003
//    Added clean zones only.
//
//    Jeremy Meredith, Tue Apr 13 16:42:55 PDT 2004
//    Added mixed color.
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Added pointControl and associated slots, also added GetCurrentValues.
//
// ****************************************************************************

class QvisFilledBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFilledBoundaryPlotWindow(const int type,
                                 FilledBoundaryAttributes *boundaryAtts_,
                                 const char *caption = 0,
                                 const char *shortName = 0,
                                 QvisNotepadArea *notepad = 0);
    virtual ~QvisFilledBoundaryPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    void SetMultipleColorWidgets(int index);
    void UpdateMultipleArea();
    void UpdateItem(int i);
    void GetCurrentValues(int which_widget);
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void drawInternalToggled(bool val);
    void cleanZonesOnlyToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void boundarySelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);
    void mixedColorChanged(const QColor &color);

    void pointSizeChanged(double d);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(QString &);

private:
    int                       plotType;
    FilledBoundaryAttributes *boundaryAtts;
    QLabel                   *lineStyleLabel;
    QvisLineStyleWidget      *lineStyle;
    QLabel                   *lineWidthLabel;
    QvisLineWidthWidget      *lineWidth;
    QButtonGroup             *colorModeButtons;
    QCheckBox                *legendCheckBox;
    QCheckBox                *cleanZonesOnlyCheckBox;
    QGroupBox                *boundaryColorGroup;
    QvisColorButton          *singleColor;
    QvisOpacitySlider        *singleColorOpacity;
    QLabel                   *multipleColorLabel;
    QListBox                 *multipleColorList;
    QvisColorButton          *multipleColor;
    QvisOpacitySlider        *multipleColorOpacity;
    QvisColorTableButton     *colorTableButton;
    QvisOpacitySlider        *overallOpacity;
    QLabel                   *mixedColorLabel;
    QvisColorButton          *mixedColor;
    QCheckBox                *wireframeCheckBox;
    QCheckBox                *drawInternalCheckBox;
    QButtonGroup             *smoothingLevelButtons;
    QvisPointControl         *pointControl;
};

#endif
