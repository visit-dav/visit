// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_MESH_PLOT_WINDOW_H
#define QVIS_MESH_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>

class MeshAttributes;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QvisLineWidthWidget;
class QvisColorButton;
class QvisPointControl;
class QvisOpacitySlider;

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
//   Brad Whitlock, Wed Jul 20 14:23:58 PST 2005
//   Added a new slot to handle a new signal from QvisPointControl.
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Jeremy Meredith, Fri Feb 20 17:28:17 EST 2009
//   Added per-plot alpha (opacity) support.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Biagas, Thu Apr 23 13:12:22 PDT 2015
//   Removed never-implemented outlineOnly and errorTolerance widgets.
// 
// ****************************************************************************

class QvisMeshPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisMeshPlotWindow(const int type, MeshAttributes *_meshAtts,
                       const QString &caption = QString(),
                       const QString &shortName = QString(),
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
    void lineWidthChanged(int newWidth);
    void legendToggled(bool val);
    void showInternalToggled(bool val);
    void meshColorChanged(const QColor &color);
    void opaqueModeChanged(int val);
    void opaqueColorChanged(const QColor &color);
    void meshColorClicked(int val);
    void opaqueColorClicked(int val);
    void smoothingLevelChanged(int index);

    void pointSizeChanged(double d);
    void pointSizePixelsChanged(int);
    void pointTypeChanged(int index);
    void pointSizeVarToggled(bool);
    void pointSizeVarChanged(const QString &);

    void changedOpacity(int opacity, const void *);

private:
    int                     plotType;
    MeshAttributes         *meshAtts;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *opaqueModeGroup;
    QCheckBox              *legendToggle;
    QCheckBox              *showInternalToggle;
    QButtonGroup           *meshColorButtons;
    QvisColorButton        *meshColor;
    QLabel                 *opaqueColorLabel;
    QButtonGroup           *opaqueColorButtons;
    QvisColorButton        *opaqueColor;
    QButtonGroup           *smoothingLevelButtons;
    QvisPointControl       *pointControl;
    QLabel                *opacityLabel;
    QvisOpacitySlider     *opacitySlider;
};

#endif
