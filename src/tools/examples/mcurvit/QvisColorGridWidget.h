// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_GRID_WIDGET_H
#define QVIS_COLOR_GRID_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <QvisGridWidget.h>

class QPixmap;
class QPainter;

// ****************************************************************************
// Class: QvisColorGridWidget
//
// Purpose:
//   This widget contains an array of colors from which the user may choose.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 14:30:50 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 12 19:07:21 PST 2002
//   Added an internal method for drawing the button edges.
//
//   Brad Whitlock, Fri Apr 26 11:35:04 PDT 2002
//   I fixed a drawing error that cropped up on windows.
//
//   Brad Whitlock, Thu Nov 21 10:41:40 PDT 2002
//   I added more signals.
//
//   Brad Whitlock, Wed Feb 26 12:37:09 PDT 2003
//   I made it capable of having empty color slots.
//
//   Jeremy Meredith, Fri Aug 11 16:49:48 EDT 2006
//   Refactored most of this class to a new base QvisGridWidget.
//
//   Brad Whitlock, Tue Jun  3 10:26:51 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Wed Dec 31 15:27:54 EST 2008
//   Added support for showing hints such as the color index or an
//   element name (if we're working with an atomic color table).
//
// ****************************************************************************

class GUI_API QvisColorGridWidget : public QvisGridWidget
{
    Q_OBJECT
public:
    QvisColorGridWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisColorGridWidget();

    void setSelectedColor(const QColor &c);
    void setPaletteColors(const QColor *c, int nColors, int suggestedColumns=6);

    QColor selectedColor() const;
    QColor paletteColor(int index) const;
    bool   containsColor(const QColor &color) const;
    void   setPaletteColor(const QColor &c, int index);
    virtual void drawItem(QPainter &paint, int index);
    void   setShowIndexHints(bool val);

signals:
    void selectedColor(const QColor &c);
    void selectedColor(const QColor &c, int colorIndex);
    void selectedColor(const QColor &c, int row, int column);
    void activateMenu(const QColor &c, int row, int column, const QPoint &);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);

    virtual void emitSelection();
private:
    bool     showIndexHints;
    QColor  *paletteColors;
};

#endif
