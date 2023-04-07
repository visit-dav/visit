// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PERIODIC_TABLE_WIDGET_H
#define QVIS_PERIODIC_TABLE_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <QvisGridWidget.h>
#include <vector>

class QPixmap;
class QPainter;

// ****************************************************************************
// Class: QvisPeriodicTableWidget
//
// Purpose:
//   This widget contains an array of colors from which the user may choose.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   July  6, 2006
//
// Modifications:
//    Jeremy Meredith, Tue Feb 12 14:01:52 EST 2008
//    Added support for hinting some elements to the user, e.g. to highlight
//    the elements that are actually in the database.
//
//    Brad Whitlock, Tue Jun  3 14:21:26 PDT 2008
//    QT 4.
//
// ****************************************************************************

class GUI_API QvisPeriodicTableWidget : public QvisGridWidget
{
    Q_OBJECT
public:
    QvisPeriodicTableWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisPeriodicTableWidget();

    void setSelectedElement(int e);
    void setHintedElements(const std::vector<int>&);

signals:
    void   selectedElement(int element);
protected:
    virtual void keyPressEvent(QKeyEvent *e);

    virtual void drawItem(QPainter &paint, int index);

private:
    int  indexToElement(int) const;
    virtual bool isValidIndex(int) const;
    virtual void emitSelection();

    bool *hintedElements;
};

#endif
