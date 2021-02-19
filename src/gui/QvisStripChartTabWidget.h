// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_STRIPCHART_TABWIDGET_H
#define QVIS_STRIPCHART_TABWIDGET_H

#include <QString>
#include <QVector>
#include <QTabBar>
#include <QTabWidget>


class QScrollArea;
class QvisStripChart;

#define MAX_STRIP_CHARTS 5

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
//
// ****************************************************************************

class SC_NamesTabsIndex
{
public:
    SC_NamesTabsIndex(){}
    SC_NamesTabsIndex(const char *n) : name(n) {}

    QString getName() const {return name;}

    void setScrollView(QScrollArea *sc) {scrollView = sc;}
    QScrollArea *getScrollView() {return scrollView;}
  
private:
     QString name;
     QScrollArea *scrollView;
};

// holds and keeps properly associate all the strip chart infomation.
typedef QVector<SC_NamesTabsIndex> SC_NamesVector;


class QvisTabBar : public QTabBar
{                                                          
    Q_OBJECT
public:
  QvisTabBar(QWidget *parent=0);

public slots:
  void mouseDoubleClickEvent(QMouseEvent *e);
};

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
//
// ****************************************************************************

class QvisStripChartTabWidget : public QTabWidget
{                                                          
    Q_OBJECT
public:
    QvisStripChartTabWidget( QWidget *parent=0, QObject *mgr=NULL,
                             int winX=4000, int winY=1000 );
    ~QvisStripChartTabWidget();
  
    int  nameToIndex(const QString &SC_Name) const;

    // void setEnableLogScale( bool enable );
    // bool getEnableLogScale();

    void setCurveTitle(const int unsigned curveIndex, const QString &newTitle);
    
    void clearAll     (const unsigned int tabIndex);
    void setTabLabel  (const unsigned int tabIndex, const QString &newLabel);
    void setCurveTitle(const unsigned int tabIndex,
                       const unsigned int curveIndex,
                       const QString &newTitle);
    
    void addDataPoint(const unsigned int tabIndex,
                      const unsigned int curveIndex,
                      const double x, const double y);

    void addDataPoints(const unsigned int tabIndex,
                       const unsigned int curveIndex,
                       const unsigned int npts,
                       const double *x, const double *y);

    unsigned int getCurrentStripChart() { return currentStripChart; };
                                                                        
public slots:
    void pick();
    void zoom();
    void reset();
    void clear();
    void clear( const unsigned int index );

    void updateCurrentTabData();

private:
    // index of the currently displayed strip chart
    unsigned int currentStripChart;

    // array of maxStripCharts
    QvisStripChart *stripCharts[MAX_STRIP_CHARTS];
    SC_NamesVector SC_Info;
};
#endif /* QVISSTRIPCHARTTABWIDGET */
