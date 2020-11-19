// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SUBSET_PLOT_WINDOW_H
#define QVIS_SUBSET_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class SubsetAttributes;
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
// Class: QvisSubsetPlotWindow
//
// Purpose: 
//   This class is an observer window that watches subset plot attributes
//   and always represents their current state.
//
// Notes:  Copied from QvisMaterialPlotWindow with minor changes.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 16, 2001 
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 15:44:03 PDT 2008
//   Adapted from Boundary plot window after that windows was ported to Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:14:12 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisSubsetPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSubsetPlotWindow(const int type, SubsetAttributes *SubsetAtts_,
                           const QString &caption = QString(),
                           const QString &shortName = QString(),
                           QvisNotepadArea *notepad = 0);
    virtual ~QvisSubsetPlotWindow();
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
    void subsetSelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

    void drawInternalToggled(bool);
private:
    int                     plotType;
    SubsetAttributes       *subsetAtts;

    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendToggle;
    QGroupBox              *subsetColorGroup;
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
    QCheckBox              *drawInternalToggle;
};

#endif
