/*****************************************************************************
*
* Copyright (c) 2000 - 2016, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <QvisStripChartTabWidget.h>
#include <QvisStripChart.h>

#include <QScrollArea>

#include <sstream>
#include <iostream>

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
// ****************************************************************************
QvisStripChartTabWidget::QvisStripChartTabWidget( QWidget *parent,
                                                  QObject *mgr,
                                                  int winX, int winY )
    : QTabWidget(parent)
{
    // initialize vector array with strip chart data. This keeps all
    // the associated data together. It also make sure that the number
    // of strip chart widgets is consistant.
    SC_Info.resize(MAX_STRIP_CHARTS);

    // create the strip charts
    for( unsigned int i=0; i<SC_Info.size(); ++i )
    {
        std::stringstream str;
        str << "StripChart_" << i;

        SC_Info[i] = SC_NamesTabsIndex( str.str().c_str() );

        stripCharts[i] = new QvisStripChart( this );
        stripCharts[i]->setTitle( "History" );

        const unsigned int margin = 5;
        stripCharts[i]->setContentsMargins( margin, margin, margin, margin );

        QScrollArea *sc = new QScrollArea(this);
        sc->setWidget(stripCharts[i]);
        sc->setWidgetResizable(true);

        // Move the scroll area all the way to the right. That is
        // where the data will start to be drawn.
        sc->ensureVisible(winX, winY/2);
        SC_Info[i].setScrollView(sc);

        addTab(sc, SC_Info[i].getName());
    }   

    // default to the first strip chart as current
    currentStripChart = 0;

    connect( this, SIGNAL(currentChanged(int)), this,
             SLOT(updateCurrentTabData()) );

    if( mgr != NULL )
        connect( this, SIGNAL(currentChanged(int)), mgr,
                 SLOT(updateCurrentTabData()) );
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
// ****************************************************************************
QvisStripChartTabWidget::~QvisStripChartTabWidget()
{
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
// ****************************************************************************
void 
QvisStripChartTabWidget::updateCurrentTabData()
{
    currentStripChart = currentIndex();
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
//   SC_Name : the name of the strip chart widget wanted.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//   
// ****************************************************************************
int
QvisStripChartTabWidget::nameToIndex(const QString &SC_Name) const
{
    int ST_Index = -1;
    
    for (unsigned int i=0; i<SC_Info.size(); ++i)
    {
        if (SC_Name == tabText(i))
            ST_Index = i;
    }
    
    return ST_Index;
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
// ****************************************************************************
// void 
// QvisStripChartTabWidget::setEnableLogScale( bool enable )
// {
//     stripCharts[currentStripChart]->setEnableLogScale( enable );
// }

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
// ****************************************************************************
// bool 
// QvisStripChartTabWidget::getEnableLogScale()
// {
//     return stripCharts[currentStripChart]->getEnableLogScale();
// }

// ****************************************************************************
// Method: VisItSimStripChart::pick()
//
// Purpose: 
//    Pass through method to call the pick method for the current 
//    strip chart. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//   
// **************************************************************************** 
void
QvisStripChartTabWidget::pick()
{
    stripCharts[currentStripChart]->toggleDisplayMode(false);
}

// ****************************************************************************
// Method: VisItSimStripChart::zoom
//
// Purpose: 
//    Pass through method to call the zoom method for the current
//    strip chart. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//   
// **************************************************************************** 
void
QvisStripChartTabWidget::zoom()
{
    stripCharts[currentStripChart]->toggleDisplayMode(true);
}

// ****************************************************************************
// Method: VisItSimStripChart::reset()
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
// **************************************************************************** 
void
QvisStripChartTabWidget::reset()
{
    stripCharts[currentStripChart]->reset();
}

// ****************************************************************************
// Method: VisItSimStripChart::clear()
//
// Purpose: 
//    Pass through method to call the clear method for the current 
//    strip chart. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//  
// **************************************************************************** 
void
QvisStripChartTabWidget::clear()
{
    stripCharts[currentStripChart]->clear();
}

// ****************************************************************************
// Method: VisItSimStripChart::clear
//
// Purpose: 
//    Pass through method to call the clear method for the specific 
//    strip chart. 
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//   
// **************************************************************************** 
void
QvisStripChartTabWidget::clear( const unsigned int index )
{
    stripCharts[index]->clear();
}

// ****************************************************************************
// Method: VisItSimStripChart::setTabLabel
//
// Purpose: 
//   Tabs method allows the tab label to be set programatically.
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
// ****************************************************************************
void
QvisStripChartTabWidget::setTabLabel(unsigned int tabIndex,
                                     const QString &newLabel)
{
    // If no labe use a default.
    if( newLabel.isEmpty() )
    {
        std::ostringstream label;
        label << "StripChart_" << tabIndex;
        setTabText(tabIndex, label.str().c_str());
        stripCharts[tabIndex]->setTitle( "History" );
    }
    else
    {
        setTabText(tabIndex, newLabel);
        stripCharts[tabIndex]->setTitle( newLabel );
    }
}   

// ****************************************************************************
// Method: VisItSimStripChart::setVarLabel
//
// Purpose: 
//   Tabs method allows the tab label to be set programatically.
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
// ****************************************************************************
void
QvisStripChartTabWidget::setCurveTitle(unsigned int tabIndex,
                                       unsigned int curveIndex,
                                       const QString &newTitle)
{
    stripCharts[tabIndex]->setCurveTitle(curveIndex, newTitle);
}

// ****************************************************************************
// Method: VisItSimStripChart::addDataPoint
//
// Purpose: 
//   
// Arguments:
//   name : the name of the strip chart widget wanted.
//   var  : The name of the strip chart curve wanted.
//   x    : data x value, i.e. the cycle
//   y    : data y value, i.e. the current value of the variable being plotted.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void 
QvisStripChartTabWidget::addDataPoint( unsigned int tabIndex,
                                       unsigned int curveIndex,
                                       double x, double y )
{
    stripCharts[tabIndex]->addDataPoint(curveIndex, x, y);
}
