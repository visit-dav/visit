#ifndef QVIS_CONTOUR_PLOT_WINDOW_H
#define QVIS_CONTOUR_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ContourAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QvisColorButton;
class QvisColorManagerWidget;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisContourPlotWindow
//
// Purpose: 
//   This class is an observer window that watches material plot attributes
//   and always represents their current state.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 13:48:18 PST 2001
//
// Modifications:
//   Eric Brugger, Wed Mar 14 11:58:10 PST 2001
//   I added a plot type to the constructor for use with the viewer proxy.
//   
//   Jeremy Meredith, Wed Mar 13 10:20:32 PST 2002
//   Added a toggle for wireframe mode.
//
//   Brad Whitlock, Thu Aug 22 10:25:17 PDT 2002
//   I added a slot function for single color opacity.
//
//   Brad Whitlock, Tue Dec 3 08:52:43 PDT 2002
//   I added a color table coloring option.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Moved StringToDoubleList to QvisWindowBase 
// ****************************************************************************

class QvisContourPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisContourPlotWindow(const int type, ContourAttributes *materialAtts_,
                          const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisContourPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

    bool UpdateMultipleAreaColors();
    bool UpdateMultipleAreaNames();
    void UpdateSelectByText();

    QString LevelString(int i);
    void ProcessSelectByText();
private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void wireframeToggled(bool val);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color, int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void opacityChanged(int opacity, int index);
    void colorModeChanged(int index);
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
private:
    int                     plotType;
    ContourAttributes      *contourAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QCheckBox              *legendCheckBox;
    QGroupBox              *contourColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QvisColorManagerWidget *multipleColors;
    QvisColorTableButton   *colorTableButton;
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QButtonGroup           *scalingButtons;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QCheckBox              *wireframeCheckBox;
};

#endif
