// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VECTOR_WINDOW_H
#define QVIS_VECTOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QGroupBox;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QvisColorButton;
class QvisColorTableWidget;
class QvisLineWidthWidget;
class VectorAttributes;

// ****************************************************************************
// Class: QvisVectorPlotWindow
//
// Purpose:
//   This class is a postable window that watches vector plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Hank Childs & Brad Whitlock
// Creation:   Thu Mar 22 23:40:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 18:21:34 PST 2001
//   I added color table stuff.
//
//   Brad Whitlock, Fri Aug 29 11:34:52 PDT 2003
//   I grouped related fields into group boxes.
//
//   Jeremy Meredith, Fri Nov 21 12:06:46 PST 2003
//   Added radio buttons for vector origin.
//
//   Eric Brugger, Tue Nov 23 10:13:05 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004 
//   Added widgets for min/max and limitsSelection.
//
//   Jeremy Meredith, Mon Mar 19 16:24:08 EDT 2007
//   Added controls for lineStem, stemWidth, and geometryQuality.
//   Reorganized the window a bit.
//
//   Jeremy Meredith, Tue Jul  8 15:11:19 EDT 2008
//   Added ability to limit vectors to come from original cell only
//   (useful for material-selected vector plots).
//
//   Brad Whitlock, Tue Jul 29 11:07:34 PDT 2008
//   Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Dave Pugmire, Mon Jul 19 09:38:17 EDT 2010
//   Add ellipsoid glyphing.   
//
//   Hank Childs, Tue Aug 24 07:37:27 PDT 2010
//   Add option for glyph location.
//
//   Kathleen Bonnell, Mon Jan 17 18:02:39 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

class QvisVectorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisVectorPlotWindow(const int type, VectorAttributes *_vecAtts,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisVectorPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:

    void CreateVectorTab(QWidget *);
    void CreateDataTab(QWidget *);
    void CreateGlyphTab(QWidget *);
    void CreateExtrasTab(QWidget *);

    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

    void UpdateLineStem();

private slots:
    void lineWidthChanged(int newWidth);
    void vectorColorChanged(const QColor &color);
    void processScaleText();
    void scaleByMagnitudeToggled(bool);
    void autoScaleToggled(bool);
    void processHeadSizeText();
    void reduceMethodChanged(int index);
    void locationMethodChanged(int index);
    void processNVectorsText();
    void processStrideText();
    void legendToggled(bool);
    void drawHeadToggled(bool);
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
    void originTypeChanged(int);
    void glyphTypeChanged(int newType);
    void lineStemChanged(int newType);

    void minToggled(bool on);
    void maxToggled(bool on);
    void processMaxLimitText();
    void processMinLimitText();
    void limitsSelectChanged(int);

    void processStemWidthText();

    void limitToOrigToggled(bool);

    void geometryQualityChanged(int index);

private:
    int                  plotType;
    VectorAttributes     *vectorAtts;

    QvisColorButton      *vectorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableWidget *colorTableWidget;

    QLineEdit            *scaleLineEdit;
    QCheckBox            *scaleByMagnitudeToggle;
    QCheckBox            *autoScaleToggle;

    QButtonGroup         *locationButtonGroup;
    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nVectorsLineEdit;
    QLineEdit            *strideLineEdit;
    QRadioButton         *strideRB;

    QButtonGroup         *geometryQualityButtons;
    QCheckBox            *legendToggle;


    QvisLineWidthWidget  *lineWidth;
    QLabel               *glyphTypeLabel;
    QComboBox            *glyphType;
    QLabel               *lineStemLabel;
    QComboBox            *lineStem;
    QLabel               *lineWidthLabel;
    QLineEdit            *stemWidthEdit;
    QLabel               *stemWidthLabel;
    QLabel               *headSizeLabel;
    QLineEdit            *headSizeLineEdit;
    QCheckBox            *drawHeadToggle;

    QButtonGroup         *originButtonGroup;

    QGroupBox            *limitsGroup;
    QCheckBox            *minToggle;
    QCheckBox            *maxToggle;
    QComboBox            *limitsSelect;
    QLineEdit            *maxLineEdit;
    QLineEdit            *minLineEdit;
    QCheckBox            *limitToOrigToggle;
};

#endif
