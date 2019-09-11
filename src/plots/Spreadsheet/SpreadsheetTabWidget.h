// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPREADSHEET_TAB_WIDGET_H
#define SPREADSHEET_TAB_WIDGET_H
#include <QTabWidget>

// ****************************************************************************
// Class: SpreadsheetTabWidget
//
// Purpose:
//   Does the same thing as QTabWidget except that it can draw the active
//   tab's label in a highlighted color.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:41:32 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 15:23:58 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class SpreadsheetTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    SpreadsheetTabWidget(QWidget *parent);
    virtual ~SpreadsheetTabWidget();
    void setHighlightColor(const QColor &c);
    const QColor &highlightColor() const;
protected slots:
    void changeTabColors(int index);
private:
    QColor highlight;
};

#endif
