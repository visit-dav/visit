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

#ifndef QVIS_PARALLEL_AXIS_WIDGET_H
#define QVIS_PARALLEL_AXIS_WIDGET_H

#include <qwidget.h>

#include <vectortypes.h>

#define AXIS_LEFT_MARGIN        0.125
#define AXIS_RIGHT_MARGIN       0.125
#define AXIS_BOTTOM_MARGIN      0.30
#define AXIS_TOP_MARGIN         0.05

#define TICKS_PER_AXIS          9
#define AXIS_AND_TICK_WIDTH     2       // pixels
#define TICK_HALF_LENGTH        3       // pixels

#define DASHES_PER_AXIS         15
#define DASH_GAP_FRACTION       0.4


// ****************************************************************************
// Class: QvisParallelAxisWidget
//
// Purpose:
//   This widget displays a simple thumbnail rendering of a ParallelAxis plot.
//   This thumbnail is used in that plot's wizard to aid the user in selecting
//   the initial set of axes that will appear in the plot.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class QvisParallelAxisWidget : public QWidget
{
    Q_OBJECT
public:
    QvisParallelAxisWidget(QWidget *parent, const char *name=0);

    virtual     ~QvisParallelAxisWidget();
    virtual      QSize sizeHint() const;
    virtual      QSizePolicy sizePolicy() const;
    
    void         setNumberOfAxes(int axisCount_);
    void         setAxisTitles(const std::string axisTitles_[]);
    void         redrawAllAxes(bool rightAxisNamed);

public slots:
    virtual void show();
    virtual void hide();
    
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void         redrawScene(QPainter *painter);
    void         deleteBackingPixmap();

    QPixmap      *pixmap;
    bool         pixmapDirty;
    int          animationProgress;        // Might be useful eventually to
    bool         animationCountPositive;   // axis currently being selected.
    
    int          axisCount;
    bool         namedRightAxis;
    stringVector axisTitles;

private:
    void         drawAxes(QPainter *painter);
    void         drawAxisTitles(QPainter *painter);
    void         drawDataCurves(QPainter *painter);

    int          axisBottomY;
    int          axisTopY;
    
    intVector    axesXPos;
    intVector    ticksYPos;
    intVector    dashesTopYPos;
    intVector    dashesBotYPos;
};

#endif
