/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_COLOR_GRID_WIDGET_H
#define QVIS_COLOR_GRID_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>
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
// ****************************************************************************

class GUI_API QvisColorGridWidget : public QvisGridWidget
{
    Q_OBJECT
public:
    QvisColorGridWidget(QWidget *parent = 0, const char *name = 0,
                        WFlags f = 0);
    virtual ~QvisColorGridWidget();

    void setSelectedColor(const QColor &c);
    void setPaletteColors(const QColor *c, int nColors, int suggestedColumns=6);

    QColor selectedColor() const;
    QColor paletteColor(int index) const;
    bool   containsColor(const QColor &color) const;
    void   setPaletteColor(const QColor &c, int index);
    virtual void drawItem(QPainter &paint, int index);

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
    QColor  *paletteColors;
};

#endif
