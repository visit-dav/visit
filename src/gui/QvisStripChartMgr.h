/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#ifndef QVIS_STRIPCHART_MGR
#define QVIS_STRIPCHART_MGR
#include <qwidget.h>
#include <qgroupbox.h>
#include <QvisPostableWindow.h>

// ****************************************************************************
// Class: QvisStripChartTabWidget
//
// Purpose:
//    Implements a container widget to hold a set of QvisStripChartTabWidget
//
// Notes:      
//
// Programmer: Shelly Prevost, 
// Creation:   Wed Aug  1 15:11:06 PDT 2007
//
// Modifications:

// ****************************************************************************
class QvisStripChartTabWidget;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QListView;
class QString;
class QSpinBox;
class QCheckBox;
class QGridLayout;
class QScrollView;
class QColor;

class EngineList;
class ViewerProxy;

class QvisStripChartMgr : public QvisPostableWindow
{                                                          
    Q_OBJECT
    
public: 
//    QvisStripChartMgr(QWidget *parent, const char *name, ViewerProxy *theViewer,
//    EngineList *engineList, int index );
    QvisStripChartMgr(QWidget *parent, const char *name, ViewerProxy *theViewer,
    EngineList *engineList, int index , QvisNotepadArea *notepad2 );
   ~QvisStripChartMgr ();
    bool  isStripChartWidget( QString name );
    bool  isStripChartTabLabel( QString name );
    
    void setEnable( QString name, bool enable );
    bool getEnable(QString name);

    bool addDataPoint ( QString name,double x, double y);
    void update(QString name);
    void getMinMaxData( QString name, double &minY, double &maxY);
    void enableOutOfBandLimits(QString name, bool enabled);
    void setOutOfBandLimits(QString name,double min, double max);
    void setOutOfBandLimits(double min, double max);
    void setLimitStripChartDataDisplay(double min, double max);
    void setCurrentDataDisplay(double currentData );
    void setCycleDisplay (int currentCycle);
    int  sendCMD(QString sig, const QObject *ui, QString value);
    int  sendCMD(QString cmd);
    void CreateWindowContents();
    void setTabLabel(QString tabName, QString newLabel );
    
public slots:
    void reset();
    void zoomIn();
    void zoomOut();
    void focus();
    void updateCurrentTabData();
    void post();
    void unpost();
    
protected slots:
    void executeMaxLimitStripChart();
    void executeMinLimitStripChart();
    void setMinMaxStripChartDataDisplay (double minY, double maxY);
    void executeEnableStripChartLimits();
    void executeEnableLogScale();
    
private:
    QvisStripChartTabWidget *stripChartTabWidget;
    QCheckBox          *enableStripChartLimits;
    QLineEdit          *maxLimitEdit;
    QLabel             *maxLimitLabel;
    QLineEdit          *minLimitEdit;
    QLabel             *minLimitLabel;
    QLineEdit          *maxEdit;
    QLabel             *maxLabel;
    QLineEdit          *minEdit;
    QLabel             *minLabel;
    QLineEdit          *curEdit;
    QLabel             *curLabel;
    QLineEdit          *cycleEdit;
    QLabel             *cycleLabel;
    QGridLayout        *chartLayout;
    QPushButton        *resetButton;
    QPushButton        *plusButton;
    QPushButton        *minusButton;
    QPushButton        *focusButton;
    QScrollView        *sc;
    QCheckBox          *enableLogScale;
    
    bool                posted;
    
    EngineList  *engines;
    ViewerProxy *viewer;
    int          simIndex;
};
#endif /* QVIS_STRIPCHART_MGR */
