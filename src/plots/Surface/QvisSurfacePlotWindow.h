// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class QLineEdit;
class QvisColorButton;
class QvisColorTableWidget;
class QvisLineWidthWidget;

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
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 17:54:48 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisSurfacePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSurfacePlotWindow(const int type, SurfaceAttributes *surfaceAtts_,
                          const QString &caption = QString(),
                          const QString &shortName = QString(),
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
    void invertColorTableToggled(bool val);
    void limitsSelectChanged(int);
private:
    int                     plotType;
    SurfaceAttributes      *surfaceAtts;

    // Surface controls
    QGroupBox              *surfaceGroup;
    QButtonGroup           *colorModeButtons;
    QvisColorButton        *surfaceColor;
    QvisColorTableWidget   *colorTableWidget;

    // Wireframe controls
    QGroupBox              *wireframeGroup;
    QvisLineWidthWidget    *lineWidth;
    QvisColorButton        *wireframeColor;

    // Scale controls
    QButtonGroup           *scalingButtons;
    QLineEdit              *skewLineEdit;

    // Limits controls
    QComboBox              *limitsSelect;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;

    QCheckBox              *legendToggle;
    QCheckBox              *lightingToggle;
};

#endif
