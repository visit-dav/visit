#ifndef QVIS_BOUNDARY_PLOT_WINDOW_H
#define QVIS_BOUNDARY_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class BoundaryAttributes;
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
// ****************************************************************************

class QvisBoundaryPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisBoundaryPlotWindow(const int type, BoundaryAttributes *boundaryAtts_,
                           const char *caption = 0,
                           const char *shortName = 0,
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
    void GetCurrentValues(int which_widget);
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color);
    void multipleColorOpacityChanged(int opacity);
    void colorModeChanged(int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void boundarySelectionChanged();
    void overallOpacityChanged(int opacity);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int size);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

private:
    int                     plotType;
    BoundaryAttributes     *boundaryAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendCheckBox;
    QGroupBox              *boundaryColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QLabel                 *multipleColorLabel;
    QListBox               *multipleColorList;
    QvisColorButton        *multipleColor;
    QvisOpacitySlider      *multipleColorOpacity;
    QvisColorTableButton   *colorTableButton;
    QvisOpacitySlider      *overallOpacity;
    QCheckBox              *wireframeCheckBox;
    QButtonGroup           *smoothingLevelButtons;
    QvisPointControl       *pointControl;
};

#endif
