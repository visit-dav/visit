// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#define QVIS_FILLED_BOUNDARY_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class FilledBoundaryAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QvisColorButton;
class QvisColorSwatchListWidget;
class QvisColorTableWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisPointControl;

// ****************************************************************************
// Class: QvisFilledBoundaryPlotWindow
//
// Purpose: 
//   This class is an observer window that watches FilledBoundary plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisBoundaryPlotWindow with minor changes.
//
// Programmer: Jeremy Meredith
// Creation:   June 12, 2003
//
// Modifications:
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:07:08 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisFilledBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFilledBoundaryPlotWindow(const int type, FilledBoundaryAttributes *FilledBoundaryAtts_,
                           const QString &caption = QString(),
                           const QString &shortName = QString(),
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
    bool CompareItem(int i, const QString &name, const QColor &c) const;
    void GetCurrentValues(int which_widget);
private slots:
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
    void boundarySelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

    void mixedColorChanged(const QColor &color);
    void cleanZonesOnlyToggled(bool val);
    void drawInternalToggled(bool);
private:
    int                       plotType;
    FilledBoundaryAttributes *boundaryAtts;

    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendToggle;
    QGroupBox              *boundaryColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QLabel                 *multipleColorLabel;
    QvisColorSwatchListWidget *multipleColorList;
    QvisColorButton        *multipleColor;
    QvisOpacitySlider      *multipleColorOpacity;
    QvisColorTableWidget   *colorTableWidget;
    QvisOpacitySlider      *overallOpacity;
    QCheckBox              *wireframeToggle;
    QButtonGroup           *smoothingLevelButtons;
    QvisPointControl       *pointControl;
    QCheckBox              *cleanZonesOnlyToggle;
    QLabel                 *mixedColorLabel;
    QvisColorButton        *mixedColor;
    QCheckBox              *drawInternalToggle;
};

#endif
