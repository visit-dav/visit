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

#include "QvisStripChartTabWidget.h"
#include "QvisStripChart.h"
#include <SimWidgetNames.h>
#include <qtimer.h>
#include <qlabel.h>
#include <math.h>
#include <qpen.h>
#include <qfont.h>
#include <qscrollview.h>
#include <qlayout.h>
#include <qsignal.h>

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
//    Shelly Prevost Thu Oct 18 16:47:10 PDT 2007
//    set the scroll area all the way to the left where the data will 
//    first be drawn.
//   
// ****************************************************************************


QvisStripChartTabWidget::QvisStripChartTabWidget( QWidget *parent, const char *name,QObject *mgr, int winX, int winY)
    : QTabWidget( parent, name, WStaticContents )
{
    // initialize vector array with strip chart data. This keeps all the associated data
    // together. It also make sure that the number of strip chart widgets is consistant.
    SC_Info.resize(maxStripCharts);
    SC_Info[0] = SC_NamesTabsIndex ( STRIP_CHART_1_WIDGET_NAME,STRIP_CHART_1_TAB_NAME,0);
    SC_Info[1] = SC_NamesTabsIndex ( STRIP_CHART_2_WIDGET_NAME,STRIP_CHART_2_TAB_NAME,1);
    SC_Info[2] = SC_NamesTabsIndex ( STRIP_CHART_3_WIDGET_NAME,STRIP_CHART_3_TAB_NAME,2);
    SC_Info[3] = SC_NamesTabsIndex ( STRIP_CHART_4_WIDGET_NAME,STRIP_CHART_4_TAB_NAME,3);
    SC_Info[4] = SC_NamesTabsIndex ( STRIP_CHART_5_WIDGET_NAME,STRIP_CHART_5_TAB_NAME,4);
    // create the strip charts
    for (size_t i =0; i< SC_Info.size(); i++) 
    {
        stripCharts[SC_Info[i].getIndex()] = new VisItSimStripChart(this,SC_Info[i].getName().ascii(),winX,winY);
        QScrollView *sc = new QScrollView(this,"StripChartScrollWindow");
        sc->addChild(stripCharts[SC_Info[i].getIndex()]);
        sc->setCaption(tr("VisIt Strip Chart"));
        // move the scroll area all the way to the right. That is where the 
        // data will start to be drawn.
        sc->center( winX, winY/2.0);
        sc->show();
        sc->updateContents();
        SC_Info[i].setScrollView(sc);
        addTab ( sc, stripCharts[SC_Info[i].getIndex()]->name() );
    }   

    // default to the first strip chart as current
    currentStripChart = 0;

    connect ( this, SIGNAL ( currentChanged(QWidget *)), this, SLOT( updateCurrentTabData()));
    if ( mgr != NULL)
        connect ( this, SIGNAL ( currentChanged(QWidget *)), mgr, SLOT( updateCurrentTabData()));


}

// ****************************************************************************
// Method: VisItSimStripChart::~QvisStripChartTabWidget
//
// Purpose: 
//   This is the destructor for QvisStripChartTabWidget
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

QvisStripChartTabWidget::~QvisStripChartTabWidget()
{
// cleanup
}

// ****************************************************************************
// Method: VisItSimStripChart::updateCurrentTabData
//
// Purpose: 
//   When the tab is change by clicking on another tab or
//   programatically the tab is changed all the associated widgets
//   in the manager must be updated with the current values for 
//   the new strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::updateCurrentTabData()
{
   // every time the tab ( strip chart ) is changed you have to 
   // update all the information needed to update the ui in VisIt
    currentStripChart = currentPageIndex();
    stripCharts[currentStripChart]->getMinMaxData( minData, maxData);
    stripCharts[currentStripChart]->getOutOfBandLimits( minYLimit, maxYLimit);
    enabled = stripCharts[currentStripChart]->getEnable();
    outOfBandLimitsEnabled = stripCharts[currentStripChart]->getEnableOutOfBandLimits();

}

// ****************************************************************************
// Method: VisItSimStripChart::getCurrentPageIndex
//
// Purpose: 
//   accessor for geting the index of the current tab page
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
int QvisStripChartTabWidget::getCurrentPageIndex()
{
    return currentStripChart;
}

// ****************************************************************************
// Method: VisItSimStripChart::getCurrentStripChart
//
// Purpose: 
//   accessor for geting the pointer to the current tab page
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************    
QWidget *QvisStripChartTabWidget::getCurrentStripChart()
{
    return currentPage();
}

// ****************************************************************************
// Method: VisItSimStripChart::setTabLabel
//
// Purpose: 
//   Tabs methos allows the tab label to be set programatically.
//
// Arguments:
//   tabIndex : index for the tab page to be changed
//   newLabel : Label to replace the existing label on the tap page.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************    
void QvisStripChartTabWidget::setTabLabel(int tabIndex, QString newLabel )
{
    changeTab( page(tabIndex), newLabel);
}   

// ****************************************************************************
// Method: QvisStripChartTabWidget::nameToIndex
//
// Purpose: 
//    This method attemps to match the input string SC_Name with 
//    the name of one of the strip charts. If it finds a match
//    it returns the index to that strip chart, if not then it
//    returns -1.
//   
// Arguments:
//   SC_Name : the name of the strip chart widget you are looking for. This will
//             be one of the standard names found in the SimWidgetNames.h file.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

int QvisStripChartTabWidget::nameToIndex(QString SC_Name)
{
    int ST_Index = -1;
    for (size_t i = 0; i < SC_Info.size(); i++)
    {
        if (SC_Name == stripCharts[i]->name())
            ST_Index = i;
    }      
   return ST_Index;
}

// ****************************************************************************
// Method: VisItSimStripChart::nameToTabIndex
//
// Purpose: 
//    This method looks up the tab name and returns the index of  the
//    tab page it is on. If the name is not found -1 is returned.
//   
// Arguments:
//   name : the name of the strip chart tab you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 12:18:40 PDT 2008
//   Use QString == operator.
//   
// ****************************************************************************

int
QvisStripChartTabWidget::nameToTabIndex(QString Tab_Name)
{
    for (size_t i = 0; i < SC_Info.size(); i++)
    {
        if(SC_Info[i].getTabName() == Tab_Name) 
            return i;
    }      
    return -1;
}

// ****************************************************************************
// Method: VisItSimStripChart::isStripChartWidget
//
// Purpose: 
//    This method looks up name to see if it matches one of the known
//    strip charts. If it does true is returned else false.
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::isStripChartWidget( QString name )
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
        return true;
    return false;
}

// ****************************************************************************
// Method: VisItSimStripChart::isStripChartTabLabel
//
// Purpose: 
//    If the name is found to match one of the known strip chart label name 
//    then true is returned else false.
//   
// Arguments:
//   name : the name of the strip chart tab widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::isStripChartTabLabel( QString name )
{
    int ST_Index = nameToTabIndex(name);
    if (ST_Index >= 0 )
        return true;
    return false;
}

// ****************************************************************************
// Method: VisItSimStripChart::setEnable
//
// Purpose: 
//    Allows the strip chart to accept value changes via the user
//   
// Arguments:
//   name   : the name of the strip chart widget you are looking for. This will
//            be one of the standard names found in the SimWidgetNames.h file. 
//   enable : enables the strip chart to accept values from the user.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::setEnable( QString name, bool enable )
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
        stripCharts[ST_Index]->setEnable( enable );
}

// ****************************************************************************
// Method: VisItSimStripChart::getEnable
//
// Purpose: 
//    returns the current state for the strip chart named named.
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::getEnable( QString name )
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
        return stripCharts[ST_Index]->getEnable();
    else return false;
}

// ****************************************************************************
// Method: VisItSimStripChart::setEnableLogScale
//
// Purpose: 
//    Enables the log plot of data for the current strip chart.
//   
// Arguments:
//   enable :  sets the state of the log plotting for the current strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::setEnableLogScale( bool enable )
{
    stripCharts[currentStripChart]->setEnableLogScale( enable );
}

// ****************************************************************************
// Method: VisItSimStripChart::getCurrentData
//
// Purpose: 
//   Returns the last data updated for the currently displayed strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
double 
QvisStripChartTabWidget::getCurrentData( )
{
    return stripCharts[currentStripChart]->getCurrentData();
}

// ****************************************************************************
// Method: VisItSimStripChart::getCurrentCycle
//
// Purpose: 
//   Returns the last data updated for the currently displayed strip chart.
//
// Programmer: Shelly Prevost 
// Creation:   Thu Oct 18 14:25:35 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
int 
QvisStripChartTabWidget::getCurrentCycle( )
{
    return stripCharts[currentStripChart]->getCurrentCycle();
}

// ****************************************************************************
// Method: VisItSimStripChart::getEnableLogScale
//
// Purpose: 
//   Returns the state for the currently displayed strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::getEnableLogScale()
{
    return stripCharts[currentStripChart]->getEnableLogScale();
}

// ****************************************************************************
// Method: VisItSimStripChart::addDataPoint
//
// Purpose: 
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file.
//    x   : data x value, usually the cycle
//    y   : data y value, usually the current value of the variable being plotted.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//    Shelly Prevost,  Thu Oct 18 16:36:59 PDT 2007
//    fixed return type to pass on out of bounds information.
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::addDataPoint( QString name,double x, double y)
{
    bool outOfBounds = TRUE;
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
       outOfBounds = stripCharts[ST_Index]->addDataPoint(x,y);
    updateCurrentTabData();
    return outOfBounds;
}


// ****************************************************************************
// Method: VisItSimStripChart::update
//
// Purpose: 
//   Calls the strip chart that matches the name and executes the update method 
//   for it.
//
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::update(QString name)
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
        stripCharts[ST_Index]->update();
}

// ****************************************************************************
// Method: VisItSimStripChart::getMinMaxData
//
// Purpose: 
//     Returns the min and the max for the strip chart named "name". 
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//   minY : the minimum y data value in the strip chart named "name"
//   maxY : the maximum y data value in the strip chart named "name" 
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::getMinMaxData( QString name, double &minY, double &maxY)
{
    
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
        stripCharts[ST_Index]->getMinMaxData( minY, maxY);
}

// ****************************************************************************
// Method: VisItSimStripChart::getMinMaxData
//
// Purpose: 
//     Returns the min and the max for the current strip chart. 
//   
// Arguments:
//   minY : the minimum y data value in the current strip chart.
//   maxY : the maximum y data value in the current strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::getMinMaxData( double &minY, double &maxY)
{
    stripCharts[currentStripChart]->getMinMaxData( minY, maxY);
}

// ****************************************************************************
// Method: VisItSimStripChart::enableOutOfBandLimit
//
// Purpose: 
//    Sets the enabled state for out of band checking for the strip chart named "name"
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file.
//   enabled : sets the enable state for out of band processing for the named strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::enableOutOfBandLimits(QString name, bool enabled)
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )
       stripCharts[ST_Index]->enableOutOfBandLimits(enabled);
}

// ****************************************************************************
// Method: VisItSimStripChart::enableOutOfBandLimit
//
// Purpose: 
//    Sets the enabled state for out of band checking for the current strip chart.
//
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file.
//   tabIndex : 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::enableOutOfBandLimits(bool enabled)
{
    stripCharts[currentStripChart]->enableOutOfBandLimits(enabled);
}

// ****************************************************************************
// Method: VisItSimStripChart::setOutOfBandLimits
//
// Purpose: 
//    Set the values to be checked against for out of band checking. 
//   
// Arguments:
//   name : the name of the strip chart widget you are looking for. This will
//          be one of the standard names found in the SimWidgetNames.h file. 
//   min  : The minimum value that will trigger and out of bound alert.
//   max  : The maximum value that will trigger and out of bound alert.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::setOutOfBandLimits(QString name,double min, double max)
{
    int ST_Index = nameToIndex(name);
    if (ST_Index >= 0 )   stripCharts[ST_Index]->setOutOfBandLimits(min,max);
}

// ****************************************************************************
// Method: VisItSimStripChart::setOutOfBandLimits
//
// Purpose: 
//    Set the values to be checked against for out of band checking for the current 
//    strip chart.
//
// Arguments:
//   min  : The minimum value that will trigger and out of bound alert.
//   max  : The maximum value that will trigger and out of bound alert. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::setOutOfBandLimits(double min, double max)
{
    stripCharts[currentStripChart]->setOutOfBandLimits(min,max);
}

// ****************************************************************************
// Method: VisItSimStripChart::getOutOfBandLimits
//
// Purpose: 
//    Get the values to be checked against for out of band checking for the current 
//    strip chart.
//
// Arguments:
//   min  : The minimum value that will trigger and out of bound alert.
//   max  : The maximum value that will trigger and out of bound alert. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::getOutOfBandLimits(double &min, double &max)
{
    stripCharts[currentStripChart]->getOutOfBandLimits(min,max);
}

// ****************************************************************************
// Method: VisItSimStripChart::getEnableOutOfBandLimits
//
// Purpose: 
//    Returns the out of band enabled state for the current 
//    strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartTabWidget::getEnableOutOfBandLimits()
{
    return stripCharts[currentStripChart]->getEnableOutOfBandLimits();
}

// ****************************************************************************
// Method: VisItSimStripChart::zoomIn
//
// Purpose: 
//    Pass through method to call the zoomIn method for the current 
//    strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::zoomIn()
{
    stripCharts[currentStripChart]->zoomIn();
}

// ****************************************************************************
// Method: VisItSimStripChart::reset
//
// Purpose: 
//    Pass through method to call the reset method for the current 
//    strip chart. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// **************************************************************************** 
void QvisStripChartTabWidget::reset()
{
    stripCharts[currentStripChart]->reset();
}

// ****************************************************************************
// Method: VisItSimStripChart::zoomOut
//
// Purpose: 
//    Pass through method to call the zoomOut method for the current 
//    strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::zoomOut()
{
    stripCharts[currentStripChart]->zoomOut();
}

// ****************************************************************************
// Method: VisItSimStripChart::focus
//
// Purpose: 
//    Pass through method to call the focus method for the current 
//    strip chart.   
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartTabWidget::focus()
{
    stripCharts[currentStripChart]->focus(SC_Info[currentStripChart].getScrollView());
}
