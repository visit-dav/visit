
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
#ifndef QVISSTRIPCHARTTABWIDGET
#define QVISSTRIPCHARTTABWIDGET
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <stdlib.h>
#include <qvaluevector.h>
class VisItSimStripChart;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QListView;
class QSpinBox;
class QCheckBox;
class QGridLayout;
class QScrollView;
class QColor;
class QSignal;
class QObject;

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

// ****************************************************************************

class SC_NamesTabsIndex
{
public:
    SC_NamesTabsIndex(){}
    SC_NamesTabsIndex(char *n, char *tab, int i):name(n),tabName(tab), index(i){}
    QString getName() {return name;}
    void setScrollView(QScrollView *sc) {scrollView = sc;}
    QScrollView *getScrollView() {return scrollView;}
    QString getTabName() {return tabName;}
    int getIndex() {return index;}
private:
     QString name;
     QString tabName;
     int  index;
     VisItSimStripChart *StripChart;
     QScrollView *scrollView;
     
};

// holds and keeps properly associate all the strip chart infomation.
typedef QValueVector<SC_NamesTabsIndex> SC_NamesVector;

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

// ****************************************************************************

class QvisStripChartTabWidget : public QTabWidget
{                                                          
    Q_OBJECT
    
public:
    QvisStripChartTabWidget( QWidget *parent=0, const char *name=0, QObject *mgr=NULL, int winX=4000, int winY=1000 );
    ~QvisStripChartTabWidget();
    void setEnable( QString name, bool enable );
    bool getEnable( QString name );
    bool addDataPoint ( QString name,double x, double y);
    void update(QString name);
    void getMinMaxData( QString name, double &minY, double &maxY);
    void getMinMaxData(  double &minY, double &maxY);
 
    enum numStripCharts { maxStripCharts = 5 };
    int  nameToIndex(QString SC_Name);
    int  nameToTabIndex(QString Tab_Name);
    bool isStripChartWidget( QString name );
    bool isStripChartTabLabel( QString name );
    int  getCurrentPageIndex();
    void executeEnableStripChartLimits();
    QWidget *getCurrentStripChart();
    void getOutOfBandLimits(double &min, double &max);
    bool getEnableOutOfBandLimits();
    void setEnableLogScale( bool enable );
    bool getEnableLogScale();
    void setTabLabel(int tabIndex, QString newLabel );
    double getCurrentData();
    int  getCurrentCycle();
    
public slots:
    void reset();
    void zoomIn();
    void zoomOut();
    void focus();
    void updateCurrentTabData();
    void enableOutOfBandLimits(QString name, bool enabled);
    void enableOutOfBandLimits( bool enabled);
    void setOutOfBandLimits(QString name,double min, double max);
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
