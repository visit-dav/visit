#ifndef QVIS_SURFACE_PLOT_WINDOW_H
#define QVIS_SURFACE_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class SurfaceAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QvisColorButton;
class QvisColorManagerWidget;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisSurfacePlotWindow
//
// Purpose: 
//   This class is an observer window that watches material plot attributes
//   and always represents their current state.
//
// Notes:
//
// Programmer: Kathleen Bonnell 
// Creation:   March 06, 2001 
//
// Modifications:
//   Eric Brugger, Fri Mar 16 14:42:45 PST 2001
//   I added a plot type to the constructor for use with the viewer
//   proxy.
//
//   Brad Whitlock, Sat Jun 16 18:21:34 PST 2001
//   I added color table stuff.
//
//   Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001
//   Changed limits (min/max) related members to reflect distinction between
//   limits used for scaling and limits used for coloring.   
//   Added limitsSelect
//
//   Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//   Revert back to using one min/max for both scaling and coloring purposes. 
//   
// ****************************************************************************

class QvisSurfacePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSurfacePlotWindow(const int type, SurfaceAttributes *surfaceAtts_,
                          const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisSurfacePlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

private slots:
    void lineStyleChanged(int newStyle);
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void lightingToggled(bool val);
    void scaleClicked(int button);
    void surfaceToggled(bool val);
    void wireframeToggled(bool val);
    void surfaceColorChanged(const QColor &color);
    void wireframeColorChanged(const QColor &color);
    void colorModeChanged(int ); 
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
    void processSkewText();
    void colorTableClicked(bool useDefault, const QString &ctName);
    void limitsSelectChanged(int);
private:
    int                     plotType;
    SurfaceAttributes      *surfaceAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QCheckBox              *legendToggle;
    QCheckBox              *lightingToggle;
    QButtonGroup           *scalingButtons;
    QCheckBox              *surfaceToggle;
    QCheckBox              *wireframeToggle;
    QGroupBox              *colorModeGroup;
    QButtonGroup           *colorModeButtons;
    QvisColorButton        *surfaceColor;
    QvisColorButton        *wireframeColor;
    QLabel                 *wireframeLabel;
    QComboBox              *limitsSelect;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QLabel                 *skewLabel;
    QLineEdit              *skewLineEdit;
    QvisColorTableButton   *colorTableButton;
};

#endif
