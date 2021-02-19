// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class QvisColorTableWidget;
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
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 17:59:09 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisContourPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisContourPlotWindow(const int type, ContourAttributes *atts,
                          const QString &caption = QString(),
                          const QString &shortName = QString(),
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
    void lineWidthChanged(int newWidth);
    void singleColorChanged(const QColor &color);
    void singleColorOpacityChanged(int opacity);
    void multipleColorChanged(const QColor &color, int index);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
    void opacityChanged(int opacity, int index);
    void colorModeChanged(int index);
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();

    void wireframeToggled(bool val);
    void legendToggled(bool val);
private:
    int                     plotType;
    ContourAttributes      *contourAtts;

    QButtonGroup           *scalingButtons;

    QLabel                 *lineWidthLabel;
    QvisLineWidthWidget    *lineWidth;
    QButtonGroup           *colorModeButtons;
    QGroupBox              *contourColorGroup;
    QvisColorButton        *singleColor;
    QvisOpacitySlider      *singleColorOpacity;
    QvisColorManagerWidget *multipleColors;
    QvisColorTableWidget   *colorTableWidget;
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;

    QCheckBox              *wireframeToggle;
    QCheckBox              *legendToggle;
};

#endif
