// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_BOUNDARY_PLOT_WINDOW_H
#define QVIS_BOUNDARY_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class BoundaryAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QvisColorButton;
class QvisColorSwatchListWidget;
class QvisColorTableWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisBoundaryPlotWindow
//
// Purpose: 
//   This class is an observer window that watches boundary plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisMaterialPlotWindow with minor changes.
//
// Programmer: Jeremy Meredith
// Creation:   June 12, 2003
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 12 10:17:58 PST 2004
//   Added QvisPointControl widget and appropriate methods and slots.
//
//   Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//   Added a new slot to handle a new signal from QvisPointControl.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 17:47:48 MST 2011
//   Changed colorTableButton to colorTableWidget to gain invert toggle.
//
//   Kathleen Biagas, Tue Dec 20 14:11:42 PST 2016
//   Remove point control and related slots, GetCurrentValues.
//
// ****************************************************************************

class QvisBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisBoundaryPlotWindow(const int type, BoundaryAttributes *boundaryAtts_,
                           const QString &caption = QString(),
                           const QString &shortName = QString(),
                           QvisNotepadArea *notepad = 0);
    virtual ~QvisBoundaryPlotWindow();
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

private:
    int                     plotType;
    BoundaryAttributes     *boundaryAtts;
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
};

#endif
