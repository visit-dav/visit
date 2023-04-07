// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_GRID_WIDGET_H
#define QVIS_GRID_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

class QPixmap;
class QPainter;

// ****************************************************************************
// Class: QvisGridWidget
//
// Purpose:
//   This widget contains an grid of items from which the user may choose.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 14:30:50 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Aug 11 16:25:25 EDT 2006
//   Refactored most of QvisColorGridWidget into this new class.
//   Added some new virtual functions and capabilities to support
//   the new QvisPeriodicTableWidget.  Renamed most of the color-specific
//   members to refer to generic items.
//
//   Brad Whitlock, Mon Jun  2 16:27:18 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisGridWidget : public QWidget
{
    Q_OBJECT
public:
    QvisGridWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisGridWidget();
    virtual QSize sizeHint () const;
    virtual QSize minimumSize() const;

    void setActiveIndex(int index);
    void setSelectedIndex(int index);


    void setFrame(bool val);
    void setBoxSize(int val);
    void setBoxPadding(int val);

    int    rows() const; 
    int    columns() const;
    int    selectedIndex() const;
    int    boxSize() const;
    int    boxPadding() const;
    int    activeIndex() const;


public slots:
    virtual void show();
    virtual void hide();

protected:
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    void drawItemArray(QPainter &);
    virtual void drawItem(QPainter &paint, int index) = 0;
    QRegion getItemRegion(int index) const;
    QRegion drawHighlightedItem(QPainter &paint, int index);
    QRegion drawSelectedItem(QPainter &paint, int index);

    void setIsPopup();
    virtual bool isValidIndex(int) const;
    virtual void emitSelection();

protected:
    void getItemRect(int index, int &x, int &y, int &w, int &h) const;
    int  getIndexFromXY(int x, int y) const;
    int  getIndex(int row, int col) const;
    void getRowColumnFromIndex(int index, int &row, int &column) const;
    void drawBox(QPainter &p, const QRect &r,
                 const QColor &light, const QColor &dark, int lw = 2);
    void setIsPopup(bool);

    int     numGridSquares;
    int     numRows;
    int     numColumns;
    int     currentActiveItem;
    int     currentSelectedItem;
    bool    drawFrame;
    int     boxSizeValue;
    int     boxPaddingValue;
    QTimer  *timer;

private:
    bool    isPopup;
};

#endif
