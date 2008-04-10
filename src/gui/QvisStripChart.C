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

#include "QvisStripChart.h"
#include <qtimer.h>
#include <math.h>
#include <qpen.h>
#include <qfont.h>

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
//    Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//    added pointSize to update font size. Also added a variable for
//    zoomOutLimit to prevent to small of zooms.
//   
//    Shelly Prevost Fri Oct 12 15:43:38 PDT 2007
//    added enableLogScale flag initialization
//
//    Shelly Prevost Wed Oct 10 11:27:08 PDT 2007 
//    added support for current cycle display.
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
    minData = HUGE_VAL;
    maxData = -HUGE_VAL;
    resize(winX,winY);
    enableLogScale = FALSE;
     
    // set the timeshift offset to start at the right side of the 
    // window.
    timeShift = width();
    setOutOfBandLimits( -HUGE_VAL,HUGE_VAL);
    // create in disabled mode
    enabled = false;
    outOfBandLimitsEnabled = 0;
    // Used to scale up and down the y axis in the strip chart
    zoom =1.0;
    center = FALSE;
    // controls maximum amount you can zoom out.
    zoomOutLimit = 0.001;
    pointSize = 14;
    gridFont = new QFont("Helvetica",pointSize);
    setFont(*gridFont);
    currentData =0;
    currentCycle =0;
    currentScaledY=0;
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
//   Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//   added setPointSize call to update font size. 
// ****************************************************************************


void VisItSimStripChart::paintEvent( QPaintEvent * )
{
    int i =0;
    float h = height();
    float w = width();
    QPen penLimits;
    QPainter paint( this );
    gridFont->setPointSize(pointSize);
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
        currentScaledY = ((h-((endPoint.y()-minPoint)*vdelta)) );
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
            paint.setPen( penLimits ); 
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
//    Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//    added gridRes variable and dynamic grid resolution base on zoom
// ****************************************************************************

void VisItSimStripChart::paintGrid(QPainter *paint)
{
    float w = width();
    float h = height();
    float gridRes = 15.0*zoom;
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
        paint->drawLine( QPoint(int(i+timeShift),int(0)), QPoint(i+timeShift,int(h*5.0) )); // draw line
    }    

    for ( float i=-range; i<=range*5.0; i+=range/gridRes)
    {// draw horizontal lines
        paint->setPen( black ); // set pen color
        paint->drawLine( QPoint(int(0),int(h-(i*vdelta))), QPoint(int(w),int(h-(i*vdelta)))); // draw line
        paint->drawText( int(0),   int(h-(i*vdelta)), QString::number(i+minPoint));
        // be careful not to draw over the other numbers.
        if ( (w-timeShift) > 200)
           paint->drawText( int(w-80),int(h-(i*vdelta)),QString::number(i+minPoint));
        paint->drawText( timeShift,int(h-(i*vdelta)), QString::number(i+minPoint));
        for ( int t=int(0); t<int(last*delta); t+=int(delta)) 
        {
            // draw verticle tick lines
            paint->setPen( darkGray ); // set pen color
            paint->drawLine( QPoint(t+timeShift,int(h-(-delta+i*vdelta))), QPoint(t+timeShift,int(h-(delta+i*vdelta )))); // draw line
        }
    }    
    setCaption(tr("Strip Chart"));
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
//    Shelly Prevost Fri Oct 12 15:43:38 PDT 2007
//    added additional start up scaling to reduce big differences
//    due to abnormal start up values.
// 
// ****************************************************************************


bool VisItSimStripChart::addDataPoint( double x, double y )
{
    float additionalMargin;
    bool outOfBounds = FALSE;
    currentData = y;
    currentCycle = x;
    // special startup processing
    if (points.size() < 2)
    {
        additionalMargin =0.3f;
        maxPoint = maxData = y + ( fabs(y) * 0.1);
        minPoint = minData = y - ( fabs(y) * 0.1);
    }
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
//    Shelly Prevost Fri Oct 12 15:43:38 PDT 2007
//    made parameter positions consitent with other functions
//   
// ****************************************************************************
void VisItSimStripChart::setOutOfBandLimits( double minY, double maxY )
{
   minYLimit = minY;
   maxYLimit = maxY;
}

// ****************************************************************************
// Method: VisItSimStripChart::getOutOfBandLimits
//
// Purpose: 
//   This method gets the range that data should remain inside of.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//    Shelly Prevost Fri Oct 12 15:43:38 PDT 2007
//    made parameter positions consitent with other functions
//   
// ****************************************************************************
void VisItSimStripChart::getOutOfBandLimits( double &minY, double &maxY )
{
   minY = minYLimit;
   maxY = maxYLimit;
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
// Method: VisItSimStripChart::getCurrentData
//
// Purpose:
//   This method returns the last value to be added to the strip chart.
//
// Programmer: Shelly Prevost
// Creation:  Wed Oct 10 11:27:08 PDT 2007 
//
// Modifications:
//
//
// ****************************************************************************
double VisItSimStripChart::getCurrentData( )
{
   return currentData;
}

// ****************************************************************************
// Method: VisItSimStripChart::getCurrentCycle
//
// Purpose:
//   This method returns the current cycle for the strip chart.
//
// Programmer: Shelly Prevost
// Creation:  Wed Oct 10 11:27:08 PDT 2007 
//
// Modifications:
//
//
// ****************************************************************************
int VisItSimStripChart::getCurrentCycle( )
{
   return currentCycle;
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
// Method: VisItSimStripChart::getEnableOutOfBandLimits
//
// Purpose: 
//   This method gets the enables out of bounds checking flag.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 18:15:11 PST 2006
//
// Modifications:
//  
//   
// ****************************************************************************
bool VisItSimStripChart::getEnableOutOfBandLimits()
{
    return outOfBandLimitsEnabled;
}

// ****************************************************************************
// Method: VisItSimStripChart::enableLogScale
//
// Purpose: 
//   This method enables enableLogScale drawing.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 18:15:11 PST 2006
//
// Modifications:
//  
//   
// ****************************************************************************
void VisItSimStripChart::setEnableLogScale( bool enable )
{
   enableLogScale = enable;
}

// ****************************************************************************
// Method: VisItSimStripChart::getEnableLogScale
//
// Purpose: 
//   This method gets the enables enableLogScale flag.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 18:15:11 PST 2006
//
// Modifications:
//  
//   
// ****************************************************************************
bool VisItSimStripChart::getEnableLogScale()
{
    return enableLogScale;
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
//    Shelly Prevost  Tue Mar 27 16:15:47 PDT 2007
//    Limited zoom level to greater than zero.
//
//    Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//    added setFont function to update font size. Also added constant for
//    zoomOutLimit check.
//
// ****************************************************************************

void VisItSimStripChart::zoomIn()
{
    zoom += 0.25;
    if(zoom > 3.0 )  zoom =3.0;
    if (zoom <= zoomOutLimit) zoom = zoomOutLimit;
    setFontSize();
}

// ****************************************************************************
// Method: VisItSimStripChart::setFontSize
//
// Purpose:
//   This function is used to keep the font large enough to be readable at all
//   zoom settings.
//
// Arguments:
//
// Programmer: Shelly Prevost
// Creation:   Fri Apr 13 14:03:03 PDT 2007
//
//
// ****************************************************************************

void VisItSimStripChart::setFontSize()
{
    if ( zoom <= 0.35)  pointSize = 20;
    if ( zoom  > 0.35)  pointSize = 16;
    if ( zoom  > 0.5 )  pointSize = 14;
    if ( zoom  > 1.0 )  pointSize = 10;
    if ( zoom  > 1.25)  pointSize = 6;
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
//    Shelly Prevost  Tue Mar 27 16:15:47 PDT 2007
//    Limited zoom level to greater than zero
//
//    Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//    added setFont function to update font size. Also added constant for
//    zoomOutLimit check.
// ****************************************************************************

void VisItSimStripChart::zoomOut()
{
    if (zoom > 0.5) zoom = zoom -= 0.25;
    else  zoom = zoom/2.0;
    if (zoom <= zoomOutLimit ) zoom = zoomOutLimit;
    setFontSize();

}

// ****************************************************************************
// Method: VisItSimStripChart::reset
//
// Purpose:
//   This function is to reset the strip chart back to
//   it's original state.
//
// Arguments:
//
// Programmer: Shelly Prevost
// Creation:   Wed Oct 10 11:27:08 PDT 2007
//
// Modifications:
//  
// ****************************************************************************
void VisItSimStripChart::reset()
{

    delta = 0;
    vdelta = 0;
    QPainter paint( this );
    maxPoint = 1.0;
    minPoint =-1.0;
    minData = HUGE_VAL;
    maxData = -HUGE_VAL;
    currentData = 0.0;
    currentCycle = 0;
    currentScaledY=0;
    
    //enableLogScale = FALSE;
   
    // set the timeshift offset to start at the right side of the 
    // window.
    timeShift = width();
    
    setOutOfBandLimits( -HUGE_VAL,HUGE_VAL);
    points.clear();
    // create in disabled mode
    enabled = false;
    outOfBandLimitsEnabled = 0;
    // Used to scale up and down the y axis in the strip chart
    zoom =1.0;
    center = FALSE;
    // controls maximum amount you can zoom out.
    zoomOutLimit = 0.001;
    pointSize = 14;
    gridFont = new QFont("Helvetica",pointSize);
    setFont(*gridFont);
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
//    Shelly Prevost  Tue Mar 27 16:15:47 PDT 2007
//    Fixed the focus function to work properly under different
//    zoom levels.
//
// ****************************************************************************

void VisItSimStripChart::focus(QScrollView *sc)
{
    
    sc->center( width(), currentScaledY);
    sc->updateContents();
}

