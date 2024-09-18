// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_STRIPCHART_H
#define QVIS_STRIPCHART_H

#define MAX_STRIP_CHART_VARS 5

#ifdef HAVE_QWT
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QPen>

#define HISTORY 50

class Background;

class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;

//****************************************************************************
// Class: StripChartCurve
//
// Purpose:
//    Implements a class to hold the strip chart curve.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************

class StripChartCurve: public QwtPlotCurve
{
public:
    StripChartCurve( const QString &title ):
        QwtPlotCurve( title )
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );
    }

    void setColor( const QColor &color )
    {
        QColor c = color;
        c.setAlpha( 150 );

        setPen( QPen( Qt::NoPen ) );
        setBrush( c );
    }
};

//****************************************************************************
// Class: StripChartCurve
//
// Purpose:
//    Implements a class to hold the strip chart plot.
//
// Programmer: Allen Sanderson
// Creation:   1 May 2016
//
// Modifications:
//
//****************************************************************************

class QvisStripChart : public QwtPlot
{
    Q_OBJECT

public:
    QvisStripChart( QWidget * = 0 );

    // void setEnable( bool enable ) {};
    // bool getEnable() { return true; };
  
    // void setEnableLogScale( bool enable ) {};
    // bool getEnableLogScale() { return false; };

    void toggleDisplayMode( bool mode );
    void reset();
    void clear();

    void setCurveTitle( const unsigned int index, const QString &newTitle);
    void addDataPoint ( const unsigned int index,
                        const double x, const double y);
    void addDataPoints( const unsigned int index,
                        const unsigned int npts,
                        const double *x, const double *y);

private Q_SLOTS:
    void legendChecked( const QVariant &, bool on );

private:
    void showCurve( QwtPlotItem *item, bool on );
    void hideCurve( QwtPlotItem *item );

    void setupTimeAxis( double time, double timeStep );
    void advanceDataCount( double time );

    void updateSamples();
    void updateAxis();
    void AdjustLabelsComputeRange( double range[2], 
                                   double &interval,
                                   unsigned int &nTicks );
    unsigned int dataCount;

    double timeData[HISTORY];

    Background *bg;

    QwtPlotZoomer *d_zoomer[2];
    QwtPlotPicker *d_picker;
    QwtPlotPanner *d_panner;

    struct
    {
        double varData[HISTORY];

        StripChartCurve *curve;
    }
    vars[MAX_STRIP_CHART_VARS];
};
#endif
#endif
