// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PARALLEL_COORDINATES_PLOTWINDOW_H
#define QVIS_PARALLEL_COORDINATES_PLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ParallelCoordinatesAttributes;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QSlider;
class QvisColorButton;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisParallelCoordinatesPlotWindow
//
// Purpose: 
//   Defines QvisParallelCoordinatesPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2007
//
// Notes: initial implementation taken from the ParallelAxis plot window.
//
// Modifications:
//    Jeremy Meredith, Fri Feb  8 12:34:19 EST 2008
//    Added ability to unify extents across all axes.
//   
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//    Added min/max extents columns for each axis, and a button to reset them.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
//    Jeremy Meredith, Wed Feb 25 12:54:37 EST 2009
//    Added number of line partitions settings since histograms are now also
//    used to draw the lines.  Allow user to force into the mode using
//    individual data point lines for the focus instead of using a histogram.
//
//    Jeremy Meredith, Thu Mar 12 13:22:44 EDT 2009
//    Qt4 port of new additions.
//
//    Jeremy Meredith, Tue Oct 27 11:01:51 EDT 2009
//    Added ability to set axis values.
//
// ****************************************************************************

class QvisParallelCoordinatesPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisParallelCoordinatesPlotWindow(const int type,
                         ParallelCoordinatesAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisParallelCoordinatesPlotWindow();
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
    void drawLinesChanged(bool val);
    void linesColorChanged(const QColor &color);
    void linesNumPartitionsProcessText();
    void linesNumPartitionsSliderChanged(int val);
    void linesNumPartitionsSliderReleased();
    void drawContextChanged(bool val);
    void contextGammaProcessText();
    void contextGammaSliderChanged(int val);
    void contextGammaSliderReleased();
    void contextNumPartitionsProcessText();
    void contextNumPartitionsSliderChanged(int val);
    void contextNumPartitionsSliderReleased();
    void contextColorChanged(const QColor &color);
    void axisSelected(QTreeWidgetItem*);
    void addAxis(const QString &axisToAdd);
    void delAxis();
    void moveAxisUp();
    void moveAxisDown();
    void resetAxisExtents();
    void linesOnlyIfExtentsToggled(bool);
    void unifyAxisExtentsToggled(bool);
    void focusGammaProcessText();
    void focusGammaSliderChanged(int val);
    void focusGammaSliderReleased();
    void drawFocusAsChanged(int);
    void axisMinValChanged(const QString &val);
    void axisMaxValChanged(const QString &val);
    void axisMinOrMaxValProcessText();
  private:
    int GetSelectedAxisIndex();
    int plotType;

    QGroupBox          *axisGroup;
    QTreeWidget        *axisTree;
    QvisVariableButton *axisNewButton;
    QPushButton        *axisDelButton;
    QPushButton        *axisUpButton;
    QPushButton        *axisDownButton;
    QPushButton        *axisResetExtentsButton;

    QLabel             *axisMinValLabel;
    QLineEdit          *axisMinVal;
    QLabel             *axisMaxValLabel;
    QLineEdit          *axisMaxVal;

    QGroupBox          *drawLines;
    QCheckBox          *linesOnlyIfExtents;
    QComboBox          *drawFocusAs;
    QLabel             *drawFocusAsLabel;
    QvisColorButton    *linesColor;
    QLineEdit          *linesNumPartitions;
    QSlider            *linesNumPartitionsSlider;
    QLineEdit          *focusGamma;
    QSlider            *focusGammaSlider;

    QGroupBox          *drawContext;
    QLineEdit          *contextGamma;
    QSlider            *contextGammaSlider;
    QLineEdit          *contextNumPartitions;
    QSlider            *contextNumPartitionsSlider;
    QvisColorButton    *contextColor;

    QLabel             *linesColorLabel;
    QLabel             *linesNumPartitionsLabel;
    QLabel             *contextGammaLabel;
    QLabel             *contextNumPartitionsLabel;
    QLabel             *contextColorLabel;
    QLabel             *focusGammaLabel;

    QCheckBox          *unifyAxisExtents;


    ParallelCoordinatesAttributes *atts;
};



#endif
