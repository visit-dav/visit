#ifndef QVIS_SUBSET_PLOT_WINDOW_H
#define QVIS_SUBSET_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class SubsetAttributes;
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
//   Brad Whitlock, Wed Jan 30 11:04:17 PDT 2002
//   Rewrote the way multiple colors are assigned.
//
//   Jeremy Meredith, Tue Mar 12 17:15:49 PST 2002
//   Added a toggle for wireframe mode.
//
//   Jeremy Meredith, Thu Mar 14 17:49:58 PST 2002
//   Added a toggle for internal surfaces.
//
//   Brad Whitlock, Mon Nov 25 17:04:48 PST 2002
//   I added a color table button.
//
//   Jeremy Meredith, Tue Dec 10 10:22:40 PST 2002
//   Added smoothing level.
//
// ****************************************************************************

class QvisSubsetPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSubsetPlotWindow(const int type, SubsetAttributes *subsetAtts_,
                           const char *caption = 0,
                           const char *shortName = 0,
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
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void drawInternalToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void subsetSelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);
private:
    int                     plotType;
    SubsetAttributes       *subsetAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendCheckBox;
    QGroupBox              *subsetColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QLabel                 *multipleColorLabel;
    QListBox               *multipleColorList;
    QvisColorButton        *multipleColor;
    QvisOpacitySlider      *multipleColorOpacity;
    QvisColorTableButton   *colorTableButton;
    QvisOpacitySlider      *overallOpacity;
    QCheckBox              *wireframeCheckBox;
    QCheckBox              *drawInternalCheckBox;
    QButtonGroup           *smoothingLevelButtons;
};

#endif
