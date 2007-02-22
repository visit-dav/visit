/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_GRID_WIDGET_H
#define QVIS_GRID_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

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
// ****************************************************************************

class GUI_API QvisGridWidget : public QWidget
{
    Q_OBJECT
public:
    QvisGridWidget(QWidget *parent = 0, const char *name = 0,
                        WFlags f = 0);
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

    void drawItemArray();
    virtual void drawItem(QPainter &paint, int index) = 0;
    QRegion drawHighlightedItem(QPainter *paint, int index);
    QRegion drawUnHighlightedItem(QPainter *paint, int index);
    QRegion drawSelectedItem(QPainter *paint, int index);

    void setIsPopup();
    virtual bool isValidIndex(int);
    virtual void emitSelection();

protected:
    void getItemRect(int index, int &x, int &y, int &w, int &h);
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
    QPixmap *drawPixmap;
    QTimer  *timer;

private:
    bool    isPopup;
};

#endif
