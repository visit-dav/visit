#ifndef QVIS_MESH_PLOT_WINDOW_H
#define QVIS_MESH_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisColorButton;
class QvisPointControl;
class Subject;
class MeshAttributes;

// ****************************************************************************
// Class: QvisMeshPlotWindow
//
// Purpose:
//   This class is a postable window that watches mesh plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:31:31 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  5 16:20:52 PDT 2001
//   Added color choice for opaque mode.
//   
//   Kathleen Bonnell, Wed Sep 26 10:06:05 PDT 2001 
//   Added checkboxes for foreground and background. 
//   
//   Jeremy Meredith, Tue Dec 10 10:22:40 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 23:14:39 PDT 2003
//   Added support for different types of point glyphs.
//
//   Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003 
//   Changed opaqueToggle check box to opaqueMode button group.
//   Renamed signal opaqueToggled to opaqueModeChanged.
//
//   Kathleen Bonnell, Thu Feb  5 11:51:39 PST 2004 
//   Added showInternalToggle, signal showInternalToggled.
//
//   Kathleen Bonnell, Fri Nov 12 10:51:59 PST 2004 
//   Replaced point-related control widgets (and associated slots) 
//   with QvisPointControl. 
//
// ****************************************************************************

class QvisMeshPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisMeshPlotWindow(const int type, MeshAttributes *_meshAtts,
                       const char *caption = 0,
                       const char *shortName = 0,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisMeshPlotWindow();
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
    void showInternalToggled(bool val);
    void meshColorChanged(const QColor &color);
    void outlineOnlyToggled(bool on);
    void opaqueModeChanged(int val);
    void processErrorToleranceText();
    void opaqueColorChanged(const QColor &color);
    void backgroundToggled(bool on);
    void foregroundToggled(bool on);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool);
    void pointSizeVarChanged(const QString &);

private:
    int                     plotType;
    MeshAttributes         *meshAtts;
    QLabel                 *lineStyleLabel;
    QvisLineStyleWidget    *lineStyle;
    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QLabel                 *meshColorLabel;
    QvisColorButton        *meshColor;
    QCheckBox              *outlineOnlyToggle;
    QLabel                 *errorToleranceLabel;
    QLineEdit              *errorToleranceLineEdit;
    QButtonGroup           *opaqueMode;
    QCheckBox              *legendToggle;
    QCheckBox              *showInternalToggle;
    QLabel                 *opaqueColorLabel;
    QvisColorButton        *opaqueColor;
    QCheckBox              *backgroundToggle;
    QCheckBox              *foregroundToggle;
    QButtonGroup           *smoothingLevelButtons;
    QvisPointControl       *pointControl;
};

#endif
