// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisStripChartTabWidget.h>
#include <QvisStripChart.h>

#include <QScrollArea>
#include <QInputDialog>
#include <QtGui/QMouseEvent>

#include <sstream>
#include <iostream>

QvisTabBar::QvisTabBar(QWidget *parent) :
    QTabBar(parent)
{
}

void QvisTabBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button () != Qt::LeftButton) {
        QTabBar::mouseDoubleClickEvent (e);
        return;
    }

    int idx = currentIndex ();
    bool ok = true;
    QString newName = QInputDialog::getText (
                this, tr ("Change Name"),
                tr ("Insert New Tab Name"),
                QLineEdit::Normal,
                tabText (idx),
                &ok);

    if (ok) {
        setTabText (idx, newName);
    }
}

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
    QvisTabBar* myTab = new QvisTabBar;
    setTabBar(myTab);

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
// Method: VisItSimStripChart::setCurveTitle
//
// Purpose:
//   Tabs method allows the curve name to be set programatically.
//
// Arguments:
//   curveIndex : index of the curve to be changed
//   newLabel   : Label to replace the existing label on the tap page.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartTabWidget::setCurveTitle(const unsigned int curveIndex,
                                       const QString &newTitle)
{
    stripCharts[currentStripChart]->setCurveTitle(curveIndex, newTitle);
}

// ****************************************************************************
// Method: VisItSimStripChart::setTabLabel
//
// Purpose:
//   Tabs method allows the tab label to be set programatically.
//
// Arguments:
//   tabIndex : index of the tab page to be changed
//   newLabel : Label to replace the existing label on the tap page.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartTabWidget::clearAll(const unsigned int tabIndex)
{
    // Clear the plot
    stripCharts[tabIndex]->clear();

    // Reset the plot name
    std::ostringstream label;
    label << "StripChart_" << tabIndex;
    setTabText(tabIndex, label.str().c_str());
    stripCharts[tabIndex]->setTitle( "History" );

    // Clear the curves.
    for( unsigned int i=0; i<MAX_STRIP_CHART_VARS; ++i )
      stripCharts[tabIndex]->setCurveTitle( i, "" );
}

// ****************************************************************************
// Method: VisItSimStripChart::setTabLabel
//
// Purpose:
//   Tabs method allows the tab label to be set programatically.
//
// Arguments:
//   tabIndex : index of the tab page to be changed
//   newLabel : Label to replace the existing label on the tap page.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartTabWidget::setTabLabel(const unsigned int tabIndex,
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
// Method: VisItSimStripChart::setCurveTitle
//
// Purpose:
//   Tabs method allows the curve name to be set programatically.
//
// Arguments:
//   tabIndex   : index of the tab page
//   curveIndex : index of the curve to be changed
//   newLabel   : Label to replace the existing label on the tap page.
//
// Programmer: Shelly Prevost
// Creation:   Mon Oct 15 14:27:29 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartTabWidget::setCurveTitle(const unsigned int tabIndex,
                                       const unsigned int curveIndex,
                                       const QString &newTitle)
{
    stripCharts[tabIndex]->setCurveTitle(curveIndex, newTitle);
}

// ****************************************************************************
// Method: VisItSimStripChart::addDataPoint
//
// Purpose:
//   This is a pass through method that call the function with the
//   same name in the strip chart.
//
// Arguments:
//   tabIndex   : index of the tab page
//   curveIndex : index of the curve
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
QvisStripChartTabWidget::addDataPoint( const unsigned int tabIndex,
                                       const unsigned int curveIndex,
                                       const double x, const double y )
{
    stripCharts[tabIndex]->addDataPoint(curveIndex, x, y);
}

// ****************************************************************************
// Method: VisItSimStripChart::addDataPoints
//
// Purpose:
//   This is a pass through method that call the function with the
//   same name in the strip chart.
//
// Arguments:
//   tabIndex   : index of the tab page
//   curveIndex : index of the curve
//   npts : number of points
//   x    : data x value, i.e. the cycle
//   y    : data y value, i.e. the current value of the variable being plotted.
//
// Programmer: Allen Sanderson
// Creation:   29 May 2020
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartTabWidget::addDataPoints( const unsigned int tabIndex,
                                        const unsigned int curveIndex,
                                        const unsigned int npts,
                                        const double *x, const double *y )
{
    stripCharts[tabIndex]->addDataPoints(curveIndex, npts, x, y);
}
