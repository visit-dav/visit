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
#ifndef QVIS_STRIPCHART_H
#define QVIS_STRIPCHART_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#define HISTORY 50
#define MAX_STRIP_CHART_VARS 4

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

    void setCurveTitle( unsigned int index, const QString &newTitle);
    void addDataPoint( unsigned int index, double x, double y);

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
