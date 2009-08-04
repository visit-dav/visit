/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVIS_SCREEN_POSITIONER_H
#define QVIS_SCREEN_POSITIONER_H
#include <gui_exports.h>
#include <QWidget>

// ****************************************************************************
// Class: QvisScreenPositioner
//
// Purpose:
//   This widget allows the user to position a crosshairs that specifies a
//   screen position for annotations or other objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:10:17 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:09:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisScreenPositioner : public QWidget
{
    Q_OBJECT
public:
    QvisScreenPositioner(QWidget *parent = 0, Qt::WindowFlags = 0);
    virtual ~QvisScreenPositioner();
    virtual QSize sizeHint () const;
    virtual QSize minimumSize() const;

    void setScreenSize(int xs, int ys);
    int  screenWidth() const       { return xScreenSize; };
    int  screenHeight() const      { return yScreenSize; };
    void setScreenPosition(int xp, int yp);
    void setScreenPosition(double xp, double yp);
    int  screenX() const           { return xPosition; }
    int  screenY() const           { return yPosition; }
    int  pageIncrement() const     { return pageIncrement_; }
    void setPageIncrement(int val) { pageIncrement_ = val; }    
signals:
    void screenPositionChanged(int newX, int newY);
    void screenPositionChanged(double newX, double newY);
    void intermediateScreenPositionChanged(int newX, int newY);
    void intermediateScreenPositionChanged(double newX, double newY);
public slots:
    void popupShow();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *);

    void sendNewScreenPosition();
    void setTempPositionFromWidgetCoords(int wx, int wy);
    void drawBox(QPainter &paint, const QRect &r,
                 const QColor &light, const QColor &dark, int lw);
    void drawLines(QPainter &paint);

    static const int minXScreenSize;
    static const int minYScreenSize;

    int xPosition;
    int yPosition;
    int xTempPosition;
    int yTempPosition;
    int xScreenSize;
    int yScreenSize;    
    int pageIncrement_;
    bool dragging;
    bool paging;
    bool drawFrame;
};

#endif
