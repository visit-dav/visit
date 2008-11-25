
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
#ifndef QVIS_STRIPCHART_TABWIDGET_H
#define QVIS_STRIPCHART_TABWIDGET_H
#include <QPainter>
#include <QString>
#include <QTabWidget>
#include <QVector>
#include <QWidget>

class QCheckBox;
class QColor;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QObject;
class QPushButton;
class QScrollArea;
class QSignal;
class QSpinBox;
class QTreeWidget;
class QVBoxLayout;
class VisItSimStripChart;


// ****************************************************************************
// Class: SC_NamesTabsIndex
//
// Purpose:
//    Implements a class to hold all the name and indexes of the strip charts.
//
// Notes:      
//
// Programmer: Shelly Prevost, 
// Creation:   Wed Aug  1 15:11:06 PDT 2007
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 15:41:55 EDT 2008
//    Use const char*'s when possible to allow string literals.
//
// ****************************************************************************

class SC_NamesTabsIndex
{
public:
    SC_NamesTabsIndex(){}
    SC_NamesTabsIndex(const char *n, const char *tab, int i):name(n),tabName(tab), index(i){}
    QString getName() const {return name;}
    void setScrollView(QScrollArea *sc) {scrollView = sc;}
    QScrollArea *getScrollView() {return scrollView;}
    QString getTabName() const {return tabName;}
    int getIndex() const {return index;}
private:
     QString name;
     QString tabName;
     int  index;
     VisItSimStripChart *StripChart;
     QScrollArea *scrollView;
};

// holds and keeps properly associate all the strip chart infomation.
typedef QVector<SC_NamesTabsIndex> SC_NamesVector;

// ****************************************************************************
// Class: QvisStripChartTabWidget
//
// Purpose:
//    Implements a container widget to hold a set of VisitSimStripCharts.
//
// Notes:      
//
// Programmer: Shelly Prevost, 
// Creation:   Wed Aug  1 15:11:06 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Jul  8 09:33:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisStripChartTabWidget : public QTabWidget
{                                                          
    Q_OBJECT
public:
    QvisStripChartTabWidget( QWidget *parent=0, QObject *mgr=NULL, int winX=4000, int winY=1000 );
    ~QvisStripChartTabWidget();
    void setEnable(const QString &name, bool enable );
    bool getEnable(const QString &name) const;
    bool addDataPoint (const QString &name,double x, double y);
    void update(const QString &name);
    void getMinMaxData(const QString &name, double &minY, double &maxY);
    void getMinMaxData(double &minY, double &maxY);
 
    enum numStripCharts { maxStripCharts = 5 };
    int  nameToIndex(const QString &SC_Name) const;
    int  nameToTabIndex(const QString &Tab_Name) const;
    bool isStripChartWidget(const QString &name) const;
    bool isStripChartTabLabel(const QString &name) const;
    int  getCurrentPageIndex() const;
    void executeEnableStripChartLimits();
    QWidget *getCurrentStripChart();
    void getOutOfBandLimits(double &min, double &max);
    bool getEnableOutOfBandLimits();
    void setEnableLogScale( bool enable );
    bool getEnableLogScale();
    void setTabLabel(int tabIndex, const QString &newLabel);
    double getCurrentData();
    int  getCurrentCycle();
    
public slots:
    void reset();
    void zoomIn();
    void zoomOut();
    void focus();
    void updateCurrentTabData();
    void enableOutOfBandLimits(const QString &name, bool enabled);
    void enableOutOfBandLimits( bool enabled);
    void setOutOfBandLimits(const QString &name,double min, double max);
    void setOutOfBandLimits(double min, double max);

private:

    int     timeShift;    // how far left to points
    bool    down;         // TRUE if mouse down
    float   delta;
    float   vdelta;
    float   middle;
    float   maxPoint;
    float   minPoint;
    double  minYLimit;
    double  maxYLimit;
    double  minData;
    double  maxData;
    int     winXSize;

    int     winYSize;
    bool    enabled;
    bool    outOfBandLimitsEnabled;
    float   zoom;
    bool    center;
    float   zoomOutLimit;
    QFont   *gridFont;
    int     pointSize;
    
    
    // index of the currently displayed strip chart
    int currentStripChart;
    // array of maxStripCharts
    VisItSimStripChart *stripCharts[maxStripCharts];
    SC_NamesVector SC_Info;
};

#endif /* QVISSTRIPCHARTTABWIDGET */
