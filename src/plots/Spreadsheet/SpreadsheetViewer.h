// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPREADSHEET_VIEWER_H
#define SPREADSHEET_VIEWER_H

#include <QMainWindow>
#include <Observer.h>
#include <SpreadsheetAttributes.h>
#include <VariableMenuPopulator.h>
#include <vector>

#include <vtkDataSet.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QEnterEvent;
class QGroupBox;
class QHBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QMenu;
class QSlider;
class QTable;
class QTabWidget;

class QvisColorTableButton;
class QvisVariableButton;
class VariableMenuPopulator;

class SpreadsheetTable;
class SpreadsheetTabWidget;
class avtLookupTable;

class ViewerPlot;

// ****************************************************************************
// Class: SpreadsheetViewer
//
// Purpose:
//   This widget can display a VTK dataset in spreadsheet form.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 8 09:53:46 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 18:48:43 PST 2007
//   Override closeEvent.
//
//   Gunther H. Weber (with help from Hank Childs & Brad Whitlock), Mon Sep 10 18:31:13 PDT 2007
//   Show picks in spreadsheet 
//
//   Gunther H. Weber, Tue Oct 16 20:41:26 PDT 2007
//   Toggle tracer plane and patch outline independently
//
//   Gunther H. Weber, Wed Nov 28 15:20:13 PST 2007
//   Added toggle for current cell outline
//
//   Brad Whitlock, Thu Aug 28 14:41:18 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Tue May 26 11:09:08 PDT 2009
//   I added lineout operations.
//
//   Kathleen Biagas, Wed Apr 19 14:56:09 PDT 2023
//   Use QEnterEvent as arg to enterEvent.
//
// ****************************************************************************

class SpreadsheetViewer : public QMainWindow, public Observer
{
    Q_OBJECT
public:
    SpreadsheetViewer(ViewerPlot *p, QWidget *parent = 0);
    virtual ~SpreadsheetViewer();

    void setAllowRender(bool);
    void render(vtkDataSet *ds);
    void clear();
    bool setColorTable(const char *);

    virtual void Update(Subject *);
protected:
    virtual void enterEvent(QEnterEvent *);
    virtual void closeEvent(QCloseEvent *e);
private slots:
    void formatChanged();
    void sliderChanged(int);
    void sliderPressed();
    void sliderReleased();
    void tabChanged(int);
    void minClicked();
    void maxClicked();
    void colorTableCheckBoxToggled(bool);
    void tracerCheckBoxToggled(bool);
    void outlineCheckBoxToggled(bool);
    void showCurrentCellOutlineCheckBoxToggled(bool);
    void normalChanged(int);
    void selectedColorTable(bool, const QString &);
    void postNotify();
    void changedVariable(const QString &);
    void tableSelectionChanged();

    // Menu related slots
    void saveAsText();
    void copySelectionToClipboard();
    void selectAll();
    void selectNone();
    void operationSum();
    void operationAverage();
    void operationCurveX(int);
    void operationCurveY(int);
    void operationCurveX0();
    void operationCurveY0();
    void operationCurveX1();
    void operationCurveY1();
private:
    void updateSpreadsheet();
    bool moveSliceToCurrentPick();
    void selectPickPoints();
    void displayStructuredGrid(int dims[3]);
    void displayUnstructuredGrid();
    void setNumberOfTabs(int, int, bool);
    void beginMinMax(double);
    void minMax(double);
    void endMinMax();
    void calculateMinMaxCells(int meshDims[3], bool structured);
    void updateMinMaxButtons();
    void updateSliderLabel();
    void updateVariableMenus();
    void updateMenuEnabledState(int);
    int  GetCell(double, double, double);
    bool PickPointsChanged() const;
    void GetBaseIndexFromMetaData(int *base_index) const;
    void GetPickIJK(int pickId, int pickType, int *ijk) const;

    void DisplayCurve(const double *vals, int nvals);
    bool GetDataVsCoordinate(double *curve, const vtkIdType *, int nvals, int coord) const;

    // Cached plot attributes that are used to see if the Qt display needs
    // to update when the Update() method is called.
    SpreadsheetAttributes cachedAtts;

    // Indicates whether the window will update in response to a render
    // request. This flag enables the Qt display to remain fixed even when
    // the OpenGL vis window needs to redraw. This way, the Qt display
    // does not update each time the vis window needs to update, though
    // other plots might want to do just that.
    bool                  allowRender;
 
    // Pointer to the ViewerPlot object with which this object is associated.
    ViewerPlot           *plot;

    // Pointer to the VTK data that needs to be rendered.
    vtkDataSet           *input;

    // Used by SpreadsheetTable objects to map data values to color. The
    // LUT is owned by this object because this object will stay around
    // as long as the ViewerPlot exists.
    avtLookupTable       *colorLUT;

    VariableMenuPopulator menuPopulator;

    // Contains table cell location for min,max values.
    int                   minCell[3];
    double                minValue;
    int                   maxCell[3];
    double                maxValue;

    // Widgets
    QGroupBox            *controls3D;
    QLabel               *kLabel;
    QSlider              *kSlider;
    bool                  sliding;
    QCheckBox            *tracerCheckBox;
    QCheckBox            *patchOutlineCheckBox;
    QCheckBox            *currentCellOutlineCheckBox;
    QLabel               *normalLabel;
    QButtonGroup         *normalButtonGroup;
    QWidget              *normalRadioButtons;

    // Widgets that we'll use all the time.
    SpreadsheetTabWidget *zTabs;
    SpreadsheetTable    **tables;
    int                   nTables;
    int                   nTablesForSlider;

    QLabel               *formatLabel;
    QLineEdit            *formatLineEdit;

    QCheckBox            *colorTableCheckBox;
    QvisColorTableButton *colorTableButton;

    QLabel               *varLabel;
    QvisVariableButton   *varButton;

    QPushButton          *minButton;
    QPushButton          *maxButton;

    // Menu related members.
    QMenu                *fileMenu;
    QMenu                *editMenu;
#if defined(Q_OS_MAC)
    QPushButton          *opButton;
#endif
    QMenu                *operationsMenu;
    QAction              *fileMenu_SaveText;
    QAction              *editMenu_Copy;
};

#endif
