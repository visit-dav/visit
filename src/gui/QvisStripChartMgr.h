// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_STRIPCHART_MGR
#define QVIS_STRIPCHART_MGR

#include <QWidget>
#include <QvisPostableWindow.h>

class QvisSimulationWindow;
class QvisStripChartTabWidget;

class QLabel;
class QPushButton;
class QString;
class QCheckBox;
class QGridLayout;
class QScrollArea;
class QMenu;

class EngineList;
class ViewerProxy;

#include <map>

// ****************************************************************************
// Class: QvisStripChartMgr
//
// Purpose:
//    Implements a container widget to hold a set of QvisStripChartMgr
//
// Notes:      
//
// Programmer: Shelly Prevost, 
// Creation:   Wed Aug  1 15:11:06 PDT 2007
//
// Modifications:
//
// ****************************************************************************

class QvisStripChartMgr : public QvisPostableWindow
{                                                          
    Q_OBJECT
    
public: 
    QvisStripChartMgr(QWidget *parent, ViewerProxy *theViewer,
                      EngineList *engineList, int index,
                      QvisNotepadArea *notepad2);
  
    virtual ~QvisStripChartMgr();

    void setTabLabel( const unsigned int index, QString newLabel );
    void setCurveTitle( const unsigned int tabIndex,
                        const unsigned int curveIndex, QString newTitle );

    void clearAll();
    void clearMenu();
    void addMenuItem( std::string str );
    QMenu *addSubMenu( std::string str );

    void addDataPoint( const unsigned int tabIndex,
                       const unsigned int curveIndex,
                       double x, double y );

    void addDataPoints( const unsigned int tabIndex,
                        const unsigned int curveIndex,
                        const unsigned int npts,
                        double *x, double *y );

    virtual void CreateEntireWindow();

public slots:
    void pick();
    void zoom();
    void reset();
    void clear();
    void clear( const unsigned int index );

    void post();
    void unpost();
        
    void updateCurrentTabData();

    void clickedStripChartVarButton( int button );  
    void clickedStripChartVarButton0();
    void clickedStripChartVarButton1();
    void clickedStripChartVarButton2();
    void clickedStripChartVarButton3();
    void clickedStripChartVarButton4();
    void stripChartVarMenuTriggered(QAction *action);
  
protected:
    virtual void CreateWindowContents();

private:
    QvisSimulationWindow    *simulationWindow;
    QvisStripChartTabWidget *stripChartTabWidget;

    QGridLayout        *chartLayout;
    QPushButton        *pickButton;
    QPushButton        *zoomButton;
    QPushButton        *resetButton;
    QPushButton        *clearButton;

    QMenu              *stripChartVarMenu;
    QPushButton        *stripChartVarButton0;
    QPushButton        *stripChartVarButton1;
    QPushButton        *stripChartVarButton2;
    QPushButton        *stripChartVarButton3;
    QPushButton        *stripChartVarButton4;

    QScrollArea        *sc;
    QCheckBox          *enableLogScale;
    
    bool               posted;
    
    EngineList         *engines;
    ViewerProxy        *viewer;
    int                simIndex;

    int                activeVar;

    // A map associating sub menu names with Qt menus
    std::map< std::string, QMenu* > stripChartMenuMap;
    // A map associating menu actions with the complete variable name
    std::map< QAction*, std::string > stripChartActionMap;
};
#endif /* QVIS_STRIPCHART_MGR */
