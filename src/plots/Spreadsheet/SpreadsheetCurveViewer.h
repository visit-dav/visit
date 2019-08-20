// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPREADSHEET_CURVE_VIEWER_H
#define SPREADSHEET_CURVE_VIEWER_H

#include <QMainWindow>

class ViewerPlot;

class QMenu;
class QTextEdit;

// ****************************************************************************
// Class: SpreadsheetCurveViewer
//
// Purpose:
//   This widget can display a data for a curve file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 15:32:22 PDT 2009
//
// Modifications:
//
// ****************************************************************************

class SpreadsheetCurveViewer : public QMainWindow
{
    Q_OBJECT
public:
    SpreadsheetCurveViewer(ViewerPlot *p, QWidget *parent);
    virtual ~SpreadsheetCurveViewer();

    void setData(const double *vals, int nvals);
private slots:
    void saveCurve();
    void copyToClipboard();
    void operationPlot();
private:
    void saveCurveFile(const QString &name);

    ViewerPlot *plot;
    QTextEdit  *curveText;
    QMenu      *fileMenu;
    QMenu      *editMenu;
    QMenu      *operationsMenu;

    QString     filename;
    static int  counter;
};

#endif
