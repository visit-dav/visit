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
    void meshColorChanged(const QColor &color);
    void outlineOnlyToggled(bool on);
    void opaqueToggled(bool on);
    void processErrorToleranceText();
    void processPointSizeText();
    void opaqueColorChanged(const QColor &color);
    void backgroundToggled(bool on);
    void foregroundToggled(bool on);
    void smoothingLevelChanged(int index);
    void processPointSizeVarText();
    void pointSizeVarToggled(bool on);
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
    QCheckBox              *opaqueToggle;
    QCheckBox              *legendToggle;
    QLabel                 *pointSizeLabel;
    QLineEdit              *pointSizeLineEdit;
    QCheckBox              *pointSizeVarToggle;
    QLineEdit              *pointSizeVarLineEdit;
    QLabel                 *opaqueColorLabel;
    QvisColorButton        *opaqueColor;
    QCheckBox              *backgroundToggle;
    QCheckBox              *foregroundToggle;
    QButtonGroup           *smoothingLevelButtons;
};

#endif
