#ifndef QVIS_PSEUDOCOLOR_WINDOW_H
#define QVIS_PSEUDOCOLOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QvisOpacitySlider;
class QvisColorTableButton;
class QvisPointControl;

class Subject;
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
// ****************************************************************************

class QvisPseudocolorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPseudocolorPlotWindow(const int type, PseudocolorAttributes *_pcAtts,
                              const char *caption = 0,
                              const char *shortName = 0,
                              QvisNotepadArea *notepad = 0);
    virtual ~QvisPseudocolorPlotWindow();
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
    void centeringClicked(int button);
    void legendToggled(bool on);
    void lightingToggled(bool on);
    void minToggled(bool on);
    void maxToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();
    void processSkewText();
    void scaleClicked(int scale);
    void changedOpacity(int opacity, const void *);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void limitsSelectChanged(int);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool on);
    void pointSizeVarChanged(const QString &);

private:
    int                   plotType;
    PseudocolorAttributes *pcAtts;
    QButtonGroup          *centeringButtons;
    QCheckBox             *legendToggle;
    QCheckBox             *lightingToggle;
    QCheckBox             *minToggle;
    QCheckBox             *maxToggle;
    QComboBox             *limitsSelect;
    QLineEdit             *maxLineEdit;
    QLineEdit             *minLineEdit;
    QButtonGroup          *scalingButtons;
    QLabel                *skewLabel;
    QLineEdit             *skewLineEdit;
    QvisOpacitySlider     *opacitySlider;
    QvisColorTableButton  *colorTableButton;
    QButtonGroup          *smoothingLevelButtons;
    QvisPointControl      *pointControl;
};

#endif
