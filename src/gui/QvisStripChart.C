/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include "QvisStripChart.h"
#include <qtimer.h>
#include <math.h>
#include <qpen.h>

// ****************************************************************************
// Method: VisItSimStripChart::VisItSimStripChart
//
// Purpose: 
//   This is the constructor for the VisItSimStripChart and initializes the 
//   limits and window size.
// Arguments:
//   parent : the widget this stip chart is attached too.
//   name   : the name for this chart
//   winX   : the X dimension of the newly created window
//   winY   : the Y dimension of the newly created window
//
// Programmer: Shelly Prevost
// Creation:   Friday Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************


VisItSimStripChart::VisItSimStripChart( QWidget *parent, const char *name, int winX, int winY )
    : QWidget( parent, name, WStaticContents )
{
    setBackgroundColor( white );  // white background
    down = FALSE;
    delta = 0;
    vdelta = 0;
    winXSize = winX;
    winYSize = winY;
    QPainter paint( this );
    maxPoint = 1.0;
    minPoint =-1.0;
    minData = 0.0;
    maxData = 0.0;
    resize(winX,winY);
    // set the timeshift offset to start at the right side of the 
    // window.
    timeShift = width();
    setOutOfBandLimits( HUGE_VAL,-HUGE_VAL);
    // create in disabled mode
    enabled = false;
    outOfBandLimitsEnabled = 0;
    // Used to scale up and down the y axis in the strip chart
    zoom =1.0;
    center = FALSE;
}

// ****************************************************************************
// Method: VisItSimStripChart::~VisItSimStripChart
//
// Purpose: 
//   This is the destructor for VisItSimStripChart
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

VisItSimStripChart::~VisItSimStripChart()
{
// cleanup
}


// ****************************************************************************
// Method: VisItSimStripChart::paintEvent
//
// Purpose: 
//   This is main method to draw the strip chart. All drawing should
//   be called from this method. 
//
// Arguments:
//   QPaintEvent * : not used.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//   Shelly Prevost, Thu Mar 22 11:26:46 PDT 2007
//   Added zoom and focus controls. 
//   
// ****************************************************************************


void VisItSimStripChart::paintEvent( QPaintEvent * )
{
    int i =0;
    float h = height();
    float w = width();
    QPen penLimits;
    QPainter paint( this );
    paint.scale(1.0,zoom);
    if ( center )
    {
      paint.translate( 0,points.back().y());
      center = FALSE;
    }
    // start drawing the data n times step away from the right
    // hand edge of the window.
    timeShift =  int(w - ( (delta/2.0) * points.size()));
    // do we have data to draw and are we enabled?
    if ( points.empty()) return; 
    if ( !enabled ) return;

    // display the grid
    paintGrid( &paint);
    VisItPointD startOffset(points.front().x(),0); 
    Points::iterator it;
    // connect all points together to make the graph
    for( it = points.begin(); it != points.end(); )
    { 
        //  Set the pen width and color
        penLimits.setWidth(2);
        // set pen color to normal
        penLimits.setColor(darkGreen);
        VisItPointD startPoint(0,0);
        startPoint.setX( (*it).x() - startOffset.x());
        startPoint.setY( (*it).y() - startOffset.y());
        it++;
        if ( it == points.end()) break;
        VisItPointD endPoint(0,0);
        endPoint.setX((*it).x() - startOffset.x());;
        endPoint.setY((*it).y() - startOffset.y());;
        // check to see if it is out of limit
        if ( outOfBandLimitsEnabled )
        {
            if ( startPoint.y() > maxYLimit || startPoint.y() < minYLimit ||
                endPoint.y() > maxYLimit || endPoint.y() < minYLimit )
            {
                // set pen color to out of limits
                penLimits.setColor(red);
            }
        }
        paint.setPen( penLimits );

        QPoint startPoint_int;
        QPoint endPoint_int;
        // convert to screen space
        startPoint_int.setX ( int(startPoint.x()*delta +timeShift));
        startPoint_int.setY ( int ((h-((startPoint.y()-minPoint)*vdelta))));
        endPoint_int.setX ( int(endPoint.x()*delta + timeShift));
        endPoint_int.setY ( int((h-((endPoint.y()-minPoint)*vdelta)) ));

        paint.drawLine( startPoint_int, endPoint_int ); // draw line
        i++;
        if ( i%10 == 0 )
        {
            paint.drawText( startPoint_int.x(),int(h-10), QString::number((*it).x()));
            paint.drawText( startPoint_int.x(),int(middle*vdelta), QString::number((*it).x()));
            paint.drawText( startPoint_int.x(),int(10), QString::number((*it).x()));
        }
             
        // draw limit extents
        if ( outOfBandLimitsEnabled )
        {
            penLimits.setColor(red);
            penLimits.setWidth(3);
            paint.setPen( penLimits ); // set random pen color
            paint.drawLine( QPoint(int(0),int(h-((maxYLimit)-minPoint)*vdelta)), QPoint(int(w),int(h-((maxYLimit-minPoint)*vdelta)))); // draw line
            paint.drawLine( QPoint(int(0),int(h-((minYLimit)-minPoint)*vdelta)), QPoint(int(w),int(h-((minYLimit)-minPoint)*vdelta))); // draw line
        }
    }
}



// ****************************************************************************
// Method: VisItSimStripChart::paintGrid
//
// Purpose: 
//   This method draws the grid for the stripchart.
//
// Arguments:
//   paint : Qpainter object used to draw the stip chart on.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//    Shelly Prevost, Wed Mar 21 16:35:30 PDT 2007.
//    Added support for smaller grids. 
//   
// ****************************************************************************

void VisItSimStripChart::paintGrid(QPainter *paint)
{
    float w = width();
    float h = height();
    float range = maxPoint -minPoint;
    delta = 10.0;
    vdelta = (h/range);
    middle = (range/2.0)+minPoint;
    if ( points.empty()) return;
    float last = points.back().x();
    for ( int i=int(0); i<last*delta; i+= int(delta*10)) 
    {   
        // draw verticle lines
        paint->setPen( darkGray ); // set pen color
        paint->drawLine( QPoint(int(i+timeShift),int(0)), QPoint(i+timeShift,int(h) )); // draw line
    }    

    for ( float i=-range; i<=range*2.0; i+=range/10.0 ) 
    {// draw horizontal lines
        paint->setPen( black ); // set pen color
        paint->drawLine( QPoint(int(0),int(i*vdelta)), QPoint(int(w),int(i*vdelta))); // draw line
        //qDebug("i  %f,i_screen Cord  %f, maxPoint %f, maxPoint-i %f\n",i,i*vdelta,maxPoint,maxPoint-i); 
        paint->drawText( int(0),int(h-(i*vdelta )), QString::number(i+minPoint));
        paint->drawText( int(w-25),int(h-(i*vdelta)), QString::number(i+minPoint));
        paint->drawText( timeShift,int(h-(i*vdelta)), QString::number(i+minPoint));
        for ( int t=int(0); t<int(last*delta); t+=int(delta)) 
        {
            // draw verticle tick lines
            paint->setPen( darkGray ); // set pen color
            paint->drawLine( QPoint(t+timeShift,int(-delta+i*vdelta)), QPoint(t+timeShift,int(delta+i*vdelta ))); // draw line
        }
    }    
    setCaption( "VisIt Strip Chart");

}



// ****************************************************************************
// Method: VisItSimStripChart::mousePressEvent
//
// Purpose: 
//   This method is called on a mouse down event.
//
// Arguments:
//   QMouseEvent * : not used.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//   Shelly Prevost, Thu Mar 22 11:26:46 PDT 2007
//   don't use erase() function yet. This event handler
//   will be flushed out more. 
//  
//   
// ****************************************************************************


void VisItSimStripChart::mousePressEvent( QMouseEvent * )
{
    down = TRUE;
    // erase widget contents
    //erase();
}

// ****************************************************************************
// Method: VisItSimStripChart::addDataPoint
//
// Purpose: 
//   This allows data to be added to the data array.
//
// Arguments:
//   x    : data point x coordinate
//   y    : data point y coordinate
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//    Shelly Prevost, Wed Mar 21 18:31:17 PDT 2007
//    Modified to use doubles for data type.
//  
//   
// ****************************************************************************


bool VisItSimStripChart::addDataPoint( double x, double y )
{
    float additionalMargin;
    bool outOfBounds = FALSE;
    if (points.size() < 2)
        additionalMargin =0.3f;
        else
            additionalMargin = 0.2f;

    if (maxData < y) maxData = y;
    if (minData > y) minData = y;
    
    if (maxPoint < y)
    {
        maxPoint = y + fabs(y) *additionalMargin;
        maxPoint = (int(maxPoint + 9.99) / 10) *10;
    }
    if (minPoint > y)
    {
        minPoint = y - fabs(y)*additionalMargin;
        minPoint = (int(minPoint - 9.99) / 10) *10;
    }

    // add point
    VisItPointD data(x,y);
    points.push_back(data);

     // check to see if it is out of limit
     if ( outOfBandLimitsEnabled )
     {
         if (data.y() > maxYLimit || data.y() < minYLimit )
         outOfBounds = TRUE;
     }
    return outOfBounds;
}


// ****************************************************************************
// Method: VisItSimStripChart::mouseReleaseEvent
//
// Purpose: 
//   This method is not used at this time.
//
// Arguments:
//   QMouseEvent * : not used.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************


void VisItSimStripChart::mouseReleaseEvent( QMouseEvent * )
{

}

// ****************************************************************************
// Method: VisItSimStripChart::setOutOfBandLimits
//
// Purpose: 
//   This method sets the range that data should remain inside of.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************
void VisItSimStripChart::setOutOfBandLimits( double maxY, double minY )
{
   minYLimit = minY;
   maxYLimit = maxY;
}

// ****************************************************************************
// Method: VisItSimStripChart::getMinMaxData
//
// Purpose:
//   This method returns the smallest and largest data in the data set.
//
// Programmer: Shelly Prevost
// Creation:   Fri Mar 16 16:33:51 PDT 2007
//
// Modifications:
//
//
// ****************************************************************************
void VisItSimStripChart::getMinMaxData( double &minY, double &maxY )
{
   minY = minData;
   maxY = maxData;
}
// ****************************************************************************
// Method: VisItSimStripChart::enableOutOfBandLimits
//
// Purpose: 
//   This method enables out of bounds checking and limit line drawing.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 18:15:11 PST 2006
//
// Modifications:
//  
//   
// ****************************************************************************
void VisItSimStripChart::enableOutOfBandLimits( bool enable )
{
   outOfBandLimitsEnabled = enable;
}


// ****************************************************************************
// Method: VisItSimStripChart::mouseMoveEvent
//
// Purpose: 
//   This function is called to respond to mouseMoveEvent.
//
// Arguments:
//   QMouseEvent * : not used.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************


void VisItSimStripChart::mouseMoveEvent( QMouseEvent *e )
{
    QPainter paint( this );

}

// ****************************************************************************
// Method: VisItSimStripChart::getEnable
//
// Purpose: 
//   This function is an accessor to get the enable flag.
//
// Arguments:
//   enable  : flag returned to show if drawing of the strip chart will be done.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

bool VisItSimStripChart::getEnable( )
{
    return enabled;

}

// ****************************************************************************
// Method: VisItSimStripChart::setEnable
//
// Purpose: 
//   This function is an accessor to set/unset the enable flag.
//
// Arguments:
//   enable  : flag used to enable/disable drawing of the strip chart
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

void VisItSimStripChart::setEnable( bool enable )
{
    enabled = enable;

}

// ****************************************************************************
// Method: VisItSimStripChart::ZoomIn
//
// Purpose:
//   This function is to zoom out on the strip chart.
//
// Arguments:
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:35:30 PDT 2007
//
// Modifications:
//
//
// ****************************************************************************

void VisItSimStripChart::zoomIn()
{
    zoom += 0.25;
}

// ****************************************************************************
// Method: VisItSimStripChart::ZoomIn
//
// Purpose:
//   This function is to zoom in on the strip chart.
//
// Arguments:
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:35:30 PDT 2007
//
// Modifications:
//
//
// ****************************************************************************

void VisItSimStripChart::zoomOut()
{
    zoom -= 0.25;
    if (zoom == 0 ) zoom = 0.1;
}

// ****************************************************************************
// Method: VisItSimStripChart::ZoomIn
//
// Purpose:
//   This function is to center on the last data point.
//
// Arguments:
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:35:30 PDT 2007.
//
// Modifications:
//
//
// ****************************************************************************

void VisItSimStripChart::focus(QScrollView *sc)
{
    sc->horizontalScrollBar()->setValue(sc->horizontalScrollBar()->maxValue());
    float scVMax = sc->verticalScrollBar()->maxValue();
    float dataPoint =  fabs(( points.back().y()*vdelta)/(float(height())*zoom));
    scVMax *= (1.0 - dataPoint);
    sc->verticalScrollBar()->setValue(scVMax);
    sc->updateContents();
}

