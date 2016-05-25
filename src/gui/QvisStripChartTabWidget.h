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
#ifndef QVIS_STRIPCHART_TABWIDGET_H
#define QVIS_STRIPCHART_TABWIDGET_H

#include <QString>
#include <QVector>
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

    void setTabLabel(unsigned int tabIndex, const QString &newLabel);
    void setCurveTitle(int unsigned tabIndex, int unsigned cruveIndex,
                       const QString &newTitle);
    
    void addDataPoint(int unsigned tabIndex, int unsigned cruveIndex,
                      double x, double y);
  
public slots:
    void pick();
    void zoom();
    void reset();
    void clear();
    void clear( const unsigned int index );

    void updateCurrentTabData();

private:
    // index of the currently displayed strip chart
    int currentStripChart;

    // array of maxStripCharts
    QvisStripChart *stripCharts[MAX_STRIP_CHARTS];
    SC_NamesVector SC_Info;
};
#endif /* QVISSTRIPCHARTTABWIDGET */
