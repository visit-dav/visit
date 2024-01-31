// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisStripChart.h"

#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_canvas.h>
#include <QPainter>

#include <DebugStream.h>

#include <sstream>
#include <iostream>
#include <limits>

//****************************************************************************
// Class: TimeScaleDraw
//
// Purpose:
//    Implements a class for drawing the x axis based on a value.
//
//    Note this class is not used but is present as a demonstration
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************

// class TimeScaleDraw: public QwtScaleDraw
// {
// public:
//     TimeScaleDraw( const QTime &base ):
//         baseTime( base )
//     {
//         // Pass the base time to init the system.
//     }
//
//     virtual QwtText label( double v ) const
//     {
//         // Add the current time to the base time and return it as a string.
//         QTime upTime = baseTime.addSecs( static_cast<int>( v ) );
//         return upTime.toString();
//     }
// private:
//     QTime baseTime;
// };

//****************************************************************************
// Class: Background
//
// Purpose:
//    Implements a class for drawing the background as a series gray bands.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************

class Background: public QwtPlotItem
{
public:
    Background()
    {
        nBands = 10;
        height = 0.1;
        start = 0;
        setZ( 0.0 );
    }

    virtual int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    virtual void draw( QPainter *painter,
                       const QwtScaleMap &,
                       const QwtScaleMap &yMap,
                       const QRectF &canvasRect ) const
    {
        QColor c( Qt::darkGray );
        QRectF r = canvasRect;

        // Create a series of gradated bands to give a relative scale.
        for( unsigned int n=0; n<nBands; ++n )
        {
            r.setBottom( yMap.transform( start+(double)(n  )*height ) );
            r.setTop(    yMap.transform( start+(double)(n+1)*height ) );
            painter->fillRect( r, c );

            c = c.lighter( 107 );
        }
    }

    unsigned int nBands; // Number of bands.
    double height;       // Interval between bands.
    double start;        // Location of the first band.
};


//****************************************************************************
// Class: Zoomer
//
// Purpose:
//    Implements a class for allowing zooming.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer( int xAxis, int yAxis, QWidget *canvas ):
        QwtPlotZoomer( xAxis, yAxis, canvas )
    {
        setTrackerMode( QwtPicker::AlwaysOff );
        setRubberBand( QwtPicker::NoRubberBand );

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        // setMousePattern( QwtEventPattern::MouseSelect2,
        //     Qt::RightButton, Qt::ControlModifier );
        // setMousePattern( QwtEventPattern::MouseSelect3,
        //     Qt::RightButton );
        setMousePattern( QwtEventPattern::MouseSelect2,
            Qt::LeftButton, Qt::ControlModifier );
        setMousePattern( QwtEventPattern::MouseSelect3,
            Qt::LeftButton, Qt::AltModifier );
    }
};

//****************************************************************************
// Class: QvisStripChart::QvisStripChart
//
// Purpose:
//    Base class for the QvisStripChart
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
QvisStripChart::QvisStripChart( QWidget *parent ):
  QwtPlot( parent ), dataCount( 0 )
{
    setAutoReplot( false );

    // Create and set the canvas to be used.
    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setBorderRadius( 10 );
    setCanvas( canvas );

    plotLayout()->setAlignCanvasToScales( true );

    // Add the picker, panning, and zooming functions to the canvas
    d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                  QwtPlotPicker::CrossRubberBand,
                                  QwtPicker::AlwaysOn, canvas );

    d_picker->setStateMachine( new QwtPickerDragPointMachine() );
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::white ) );

    d_panner = new QwtPlotPanner( canvas );
    d_panner->setMouseButton( Qt::LeftButton, Qt::ShiftModifier );

    d_zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas );
    d_zoomer[0]->setRubberBand( QwtPicker::RectRubberBand );
    d_zoomer[0]->setRubberBandPen( QColor( Qt::green ) );
    d_zoomer[0]->setTrackerMode( QwtPicker::ActiveOnly );
    d_zoomer[0]->setTrackerPen( QColor( Qt::white ) );

    d_zoomer[1] = new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas );

    toggleDisplayMode( false );

    // Create and add the background to the plot
    bg = new Background();
    bg->attach( this );

    // Create and add the legend and the scale
    QwtLegend *legend = new QwtLegend;
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    insertLegend( legend, QwtPlot::RightLegend );

    setAxisScale( QwtPlot::yLeft, bg->start, bg->nBands*bg->height );

    setAxisTitle( QwtPlot::xBottom, "Cycle" );
    setAxisScale( QwtPlot::xBottom, 0, HISTORY );

    // This call is not used but is left as demostration when on wants
    // to offset from a base value.
//  setAxisScaleDraw( QwtPlot::xBottom, new TimeScaleDraw( cpuStat.upTime() ) );

    setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );

    // In situations, when there is a label at the most right position
    // of the scale, additional space is needed to display the
    // overlapping part of the label would be taken by reducing the
    // width of scale and canvas. To avoid this "jumping canvas"
    // effect, we add a permanent margin. We don't need to do the same
    // for the left border, because there is enough space for the
    // overlapping label below the left scale.

    QwtScaleWidget *scaleWidget = axisWidget( QwtPlot::xBottom );
    const int fmh = QFontMetrics( scaleWidget->font() ).height();
    scaleWidget->setMinBorderDist( 0, fmh / 2 );

    // Set up the time array that holds the cycle values.
    for( unsigned int i=0; i<HISTORY; ++i )
      timeData[i] = 0;

    // Set up the StripChartCurve.
    Qt::GlobalColor colors[MAX_STRIP_CHART_VARS] =
      { Qt::red, Qt::blue, Qt::black, Qt::darkCyan, Qt::green };

    for( unsigned int c=0; c<MAX_STRIP_CHART_VARS; ++c )
    {
        std::ostringstream var;
        var << "VAR_" << c;

        StripChartCurve *curve = new StripChartCurve( var.str().c_str() );
        curve->setColor( colors[c] );
        curve->setZ( curve->z() - c );
        curve->attach( this );
        vars[c].curve = curve;

        for( unsigned int i=0; i<HISTORY; ++i )
            vars[c].varData[i] = 0.0;

        // Hide the strip chart as it is automatically added to the
        // legend when it is created.
        hideCurve( vars[c].curve );
    }

    connect( legend,
             SIGNAL( checked( const QVariant &, bool, int ) ),
             SLOT( legendChecked( const QVariant &, bool ) ) );
}

//****************************************************************************
// Class: QvisStripChart::setCurveTitle
//
// Purpose:
//    Set the curve title
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void
QvisStripChart::setCurveTitle( const unsigned int index,
                               const QString &newTitle)
{
    bool clearData = false;

    if(index >= MAX_STRIP_CHART_VARS)
    {
        debug1 << "The curve index is above the maximum (MAX_STRIP_CHART_VARS)"
               << endl;
        return;
    }

    // If blank use a default name.
    if( newTitle.isEmpty() )
    {
      std::ostringstream title;
      title << "VAR_" << index;

      vars[index].curve->setTitle( title.str().c_str() );
      hideCurve( vars[index].curve );

      clearData = true;
    }
    else if( vars[index].curve->title() != newTitle )
    {
      vars[index].curve->setTitle( newTitle );
      showCurve( vars[index].curve, 1 );

      clearData = true;
    }

    // Clear the data.
    if( clearData )
    {
      for( unsigned int i=0; i<HISTORY; ++i )
        vars[index].varData[i] = 0.0;
    }
}

//****************************************************************************
// Class: QvisStripChart::showCurve
//
// Purpose:
//    Show the curve on the legend and either hides/shows the curve.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void
QvisStripChart::showCurve( QwtPlotItem *item, bool on )
{
    item->setVisible( on );

    QwtLegend *lgd = qobject_cast<QwtLegend *>( legend() );

    QList<QWidget *> legendWidgets = lgd->legendWidgets( itemToInfo( item ) );

    if( legendWidgets.size() == 1 )
    {
        QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>( legendWidgets[0] );

        if( legendLabel )
        {
            legendLabel->setVisible( true );
            legendLabel->setChecked( on );
        }
    }

    replot();
}

//****************************************************************************
// Class: QvisStripChart::hideCurve
//
// Purpose:
//    Hide the curve on the legend and plot.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void
QvisStripChart::hideCurve( QwtPlotItem *item )
{
    item->setVisible( false );

    QwtLegend *lgd = qobject_cast<QwtLegend *>( legend() );

    QList<QWidget *> legendWidgets = lgd->legendWidgets( itemToInfo( item ) );

    if( legendWidgets.size() == 1 )
    {
        QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>( legendWidgets[0] );

        if( legendLabel )
            legendLabel->setVisible( false );
    }

    replot();
}

//****************************************************************************
// Class: QvisStripChart::legendChecked
//
// Purpose:
//    Hides/shows the curve and update the plots.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void
QvisStripChart::legendChecked( const QVariant &itemInfo, bool on )
{
    QwtPlotItem *plotItem = infoToItem( itemInfo );

    if( plotItem )
        showCurve( plotItem, on );
}

//****************************************************************************
// Class: QvisStripChart::toggleDisplayMode
//
// Purpose:
//     Toogles between pick and pan/zoom mode.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::toggleDisplayMode( bool mode )
{
    d_picker->setEnabled( !mode );

    d_panner->setEnabled( mode );

    d_zoomer[0]->setEnabled( mode );
    d_zoomer[1]->setEnabled( mode );
}

//****************************************************************************
// Class: QvisStripChart::reset
//
// Purpose:
//     Reset the view to the full extents.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::reset()
{
    updateAxis();

    replot();
}

//****************************************************************************
// Class: QvisStripChart::clear
//
// Purpose:
//     Clear all of the strip chart data.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::clear()
{
    dataCount = 0;

    for( unsigned int c=0; c<MAX_STRIP_CHART_VARS; ++c )
    {
        for( unsigned int i=0; i<HISTORY; ++i )
            vars[c].varData[i] = 0.0;
    }

    // Update the samples for all curves.
    updateSamples();

    updateAxis();

    replot();

    toggleDisplayMode(false);
}

//****************************************************************************
// Class: QvisStripChart::setupTimeAxis
//
// Purpose:
//     Sets up the time axis.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::setupTimeAxis( double time, double timeStep )
{
    // Given a time and an time step go backwards in time to create a
    // history axis.
    for( unsigned int i=0; i<HISTORY; ++i )
      timeData[i] = time - (double) i * timeStep;
}

//****************************************************************************
// Class: QvisStripChart::advanceDataCount
//
// Purpose:
//     Advance the time and data histories. Which what moves the curves.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::advanceDataCount( double time )
{
    // If a single samples set the full time axis so the time
    // axis is consistant.
    if( dataCount == 0 )
      setupTimeAxis( time, 1 );

    // Once there are two samples set the full time axis so the time
    // axis is consistant.
    else if( dataCount == 1 )
      setupTimeAxis( time, time - timeData[0] );

    // Advance all of the time values.
    for( int i=HISTORY-1; i>0; --i )
        timeData[i] = timeData[i-1];

    // For each curve advance all of the of the data values.
    for( unsigned int c=0; c<MAX_STRIP_CHART_VARS; ++c )
    {
        for( unsigned int i=dataCount; i>0; --i )
        {
            if( i < HISTORY )
                vars[c].varData[i] = vars[c].varData[i-1];
        }
    }

    // Update the data count.
    if( dataCount < HISTORY )
        ++dataCount;

    timeData[0] = time;
}

//****************************************************************************
// Class: QvisStripChart::addDataPoint
//
// Purpose:
//     Add the next point to a curve.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::addDataPoint( const unsigned int index,
                                   const double x, const double y)
{
    if(index >= MAX_STRIP_CHART_VARS)
    {
        debug1 << "The curve index is above the maximum (MAX_STRIP_CHART_VARS)"
               << endl;
        return;
    }

    // If a new time advance the data count which also sets
    // the new time.
    if( timeData[0] != x )
      advanceDataCount( x );

    // Set the new Y value.
    vars[index].varData[0] = y;

    // Update the samples for all curves.
    updateSamples();

    updateAxis();

    replot();
}

//****************************************************************************
// Class: QvisStripChart::addDataPoints
//
// Purpose:
//     Add a series of points to a curve.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2020
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::addDataPoints( const unsigned int index,
                                    const unsigned int npts,
                                    const double *x, const double *y)
{
    if(index >= MAX_STRIP_CHART_VARS)
    {
        debug1 << "The curve index is above the maximum (MAX_STRIP_CHART_VARS)"
               << endl;
        return;
    }

    // If a new time advance the data count which also sets
    // the new time.
    if( timeData[0] != x[npts-1] )
    {
      advanceDataCount( x[npts-1] );

      // Now fill in the times which may already be there.
      for( unsigned int i=0; i<npts; ++i )
        timeData[i] = x[npts-1-i];
    }

    // Set the new Y value.
    for( unsigned int i=0; i<npts && i<HISTORY; ++i )
    {
      vars[index].varData[i] = y[npts-1-i];

      // Update the data count.
      if( dataCount < HISTORY )
        ++dataCount;
    }

    for( unsigned int i=npts; i<HISTORY; ++i )
    {
      vars[index].varData[i] = 0;
    }

    // Update the samples for all curves.
    updateSamples();

    updateAxis();

    replot();
}

//****************************************************************************
// Class: QvisStripChart::updateSamples
//
// Purpose:
//     Update the samples
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::updateSamples()
{
    for( unsigned int c=0; c<MAX_STRIP_CHART_VARS; ++c )
        vars[c].curve->setRawSamples( timeData, vars[c].varData, dataCount );
}


//****************************************************************************
// Class: QvisStripChart::updateAxis
//
// Purpose:
//     Up date the y axis based on all of the curves.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::updateAxis()
{
    bool haveData = false;

    // Get the min/max for all of the curves.
    double c_min =  std::numeric_limits<float>::max();
    double c_max = -std::numeric_limits<float>::max();

    for( unsigned int c=0; c<MAX_STRIP_CHART_VARS; ++c )
    {
        if( vars[c].curve->isVisible() )
        {
            for( unsigned int i=0; i<dataCount; ++i )
            {
                if( c_min > vars[c].varData[i] )
                    c_min = vars[c].varData[i];

                if( c_max < vars[c].varData[i] )
                    c_max = vars[c].varData[i];

                haveData = true;
            }
        }
    }

    if( !haveData )
      return;

    // Make sure there is a range.
    if( c_max - c_min < std::numeric_limits<float>::min() )
        c_max = c_min + 1.0;

    double range[2] = {c_min,c_max};

    // Find some reasonable bounds the includes the min/max.
    double interval;
    unsigned int nTicks;
    AdjustLabelsComputeRange( range, interval, nTicks );

    // Set the parameters for the background.
    bg->nBands = nTicks;
    bg->height = interval;
    bg->start = range[0];

    // Set the axii.
    setAxisScale( QwtPlot::xBottom, timeData[HISTORY-1], timeData[0] );
    setAxisScale( QwtPlot::yLeft, range[0], range[1] );
}

//****************************************************************************
// Function: ffix
//
// Purpose:
//     Put in a tolerance to stop numerical precision errors from creating
//     jumpy behavior.
//
// Programmer: Hank Childs
// Creation:   Fri Sep 27 13:46:14 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
inline double ffix(double value)
{
  int ivalue = (int)(value);
  double v = (value - ivalue);

  if (v > 0.9999)
    ivalue++;

  return (double) ivalue;
}

//****************************************************************************
// Function: fsign
//
// Purpose:
//     Assure the sign of two values are the same
//
// Programmer: Hank Childs
// Creation:   Fri Sep 27 13:46:14 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
inline double fsign(double value, double sign)
{
  value = fabs(value);

  if (sign < 0.)
    value *= -1.;

  return value;
}

//****************************************************************************
// Class: QvisStripChart::AdjustLabelsComputeRange
//
// Purpose:
//     Gets an even range for labeling axis.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************
void QvisStripChart::AdjustLabelsComputeRange( double range[2],
                                               double &interval,
                                               unsigned int &nTicks )
{
  double sortedRange[2] = { (range[0] < range[1] ? range[0] : range[1]),
                            (range[0] > range[1] ? range[0] : range[1]) };

  double diff = sortedRange[1] - sortedRange[0];

  // Find the integral points.
  double pow10 = log10(diff);

  // Build in numerical tolerance
  if (pow10 != 0.)
  {
      double eps = 10.0e-10;
      pow10 = fsign((fabs(pow10) + eps), pow10);
  }

  // ffix moves you in the wrong direction if pow10 is negative.
  if (pow10 < 0.)
  {
      pow10 = pow10 - 1.;
  }

  double fxt = pow(10., ffix(pow10));

  // Find the number of integral points in the interval.
  double fnt  = diff / fxt;
  fnt  = ffix(fnt);
  double frac = fnt;
  nTicks = (frac <= 0.5 ? (int)ffix(fnt) : ((int)ffix(fnt) + 1));

  double div = 1.;

  if (nTicks < 5)
      div = 2.;
  if (nTicks <= 2)
      div = 5.;

  // If there aren't enough tick points in this decade, use the next
  // decade.
  interval = fxt;

  if (div != 1.)
      interval /= div;

  // Figure out the first tick locations, relative to the start of the
  // axis.
  double start;

  if (sortedRange[0] < 0.)
      start = interval*(ffix(sortedRange[0]*(1./interval)) + 0.);
  else
      start = interval*(ffix(sortedRange[0]*(1./interval)) + 1.);

  // Create all of the ticks.
  nTicks = 0;

  // The first tick starts one interval back so to fully include all
  // values.
  range[0] = start - interval;
  range[1] = range[0];

  // Add ticks until the full range is reach.
  while (range[1] < sortedRange[1])
  {
      range[1] += interval;
      ++nTicks;
  }

  // The last tick continues one interval forward so to fully include
  // all values.
  range[1] += interval;
  ++nTicks;
}
