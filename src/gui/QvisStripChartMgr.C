// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisStripChartMgr.h"
#include "QvisStripChartTabWidget.h"

#include <QvisSimulationWindow.h>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>

#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QMenu>

// ****************************************************************************
// Method: QvisStripChartMgr::QvisStripChartMgr
//
// Purpose: 
//   This is the constructor for the QvisStripChartMgr and initializes the 
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
QvisStripChartMgr::QvisStripChartMgr(QWidget *parent, ViewerProxy *theViewer,
     EngineList *engineList, int index, QvisNotepadArea *notepad2):
     QvisPostableWindow(tr("Strip charts"), tr("Strip charts"), notepad2)
{
    simulationWindow = (QvisSimulationWindow*) parent;
    viewer = theViewer;
    simIndex = index;
    engines = engineList;
    addLayoutStretch = false;
    CreateEntireWindow();
    isPosted = false;
    postEnabled = true;
    // activeVar = 0;
}

// ****************************************************************************
// Method: QvisStripChartMgr::~QvisStripChartMgr
//
// Purpose: 
//   This is the destructor for QvisStripChartMgr
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
// ****************************************************************************
QvisStripChartMgr::~QvisStripChartMgr()
{
}

// ****************************************************************************
// Method: QvisStripChartMgr::CreateEntireWindow
//
// Purpose: 
//   This creates the base window
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//   
// ****************************************************************************
void
QvisStripChartMgr::CreateEntireWindow()
{
    QvisPostableWindow::CreateEntireWindow();
    dismissButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisStripChartMgr::CreateWindowContents
//
// Purpose: 
//   This is called to actually create the window.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void 
QvisStripChartMgr::CreateWindowContents()
{
    stripChartTabWidget = new QvisStripChartTabWidget(central, this, 2000, 100);
    topLayout->addWidget( stripChartTabWidget);
    
    // Create the group box 
    QGroupBox *stripChartGroup = new QGroupBox(central);
    stripChartGroup->setTitle(tr("Strip Chart Information and Controls"));
    topLayout->addWidget( stripChartGroup);
    
    chartLayout = new QGridLayout(stripChartGroup);              
    chartLayout->setContentsMargins(10,10,10,10);
    chartLayout->setSpacing(10);
    
    // Pick and zoom buttons
    // Create the group box and generic buttons.
    QGridLayout *zoomLayout = new QGridLayout(0);
    chartLayout->addLayout(zoomLayout, 4, 0, 1, 6);

    pickButton = new QPushButton(tr("Pick"));
    pickButton->setEnabled(true);
    zoomLayout->addWidget(pickButton,0,0);
    connect(pickButton,SIGNAL(clicked()),this,SLOT(pick()));

    zoomButton = new QPushButton(tr("Pan/Zoom"));
    zoomButton->setEnabled(true);
    zoomLayout->addWidget(zoomButton,0,1);
    connect(zoomButton,SIGNAL(clicked()),this,SLOT(zoom()));

    resetButton = new QPushButton(tr("Reset"));
    resetButton->setEnabled(true);
    zoomLayout->addWidget(resetButton,0,2);
    connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

    clearButton = new QPushButton(tr("Clear"));
    clearButton->setEnabled(true);
    zoomLayout->addWidget(clearButton,0,3);
    connect(clearButton,SIGNAL(clicked()),this,SLOT(clear()));

    // Variable buttons
    // Create the group box and generic buttons.
    QGridLayout *menuLayout = new QGridLayout(0);
    chartLayout->addLayout(menuLayout, 5, 0, 1, 6);
        
    stripChartVarButton0 = new QPushButton(tr("Var0"));
    stripChartVarButton0->setEnabled(true);
    connect(stripChartVarButton0,SIGNAL(pressed()), this,
            SLOT(clickedStripChartVarButton0()));
    menuLayout->addWidget(stripChartVarButton0,0,0);

    stripChartVarButton1 = new QPushButton(tr("Var1"));
    stripChartVarButton1->setEnabled(true);
    connect(stripChartVarButton1,SIGNAL(pressed()), this,
            SLOT(clickedStripChartVarButton1()));
    menuLayout->addWidget(stripChartVarButton1,0,1);

    stripChartVarButton2 = new QPushButton(tr("Var2"));
    stripChartVarButton2->setEnabled(true);
    connect(stripChartVarButton2,SIGNAL(pressed()), this,
            SLOT(clickedStripChartVarButton2()));
    menuLayout->addWidget(stripChartVarButton2,0,2);

    stripChartVarButton3 = new QPushButton(tr("Var3"));
    stripChartVarButton3->setEnabled(true);
    connect(stripChartVarButton3,SIGNAL(pressed()), this,
            SLOT(clickedStripChartVarButton3()));
    menuLayout->addWidget(stripChartVarButton3,0,3);

    stripChartVarButton4 = new QPushButton(tr("Var4"));
    stripChartVarButton4->setEnabled(true);
    connect(stripChartVarButton4,SIGNAL(pressed()), this,
            SLOT(clickedStripChartVarButton4()));
    menuLayout->addWidget(stripChartVarButton4,0,4);

    // With Qt 5 and a C++11 compiler, the idiomatic to pass a value:    
    // connect(stripChartVar0Button, &QAction::triggered, this, [this]{ clickedStripChartVar(0); });
    // connect(stripChartVar1Button, &QAction::triggered, this, [this]{ clickedStripChartVar(1); });
    // connect(stripChartVar2Button, &QAction::triggered, this, [this]{ clickedStripChartVar(2); });
    // connect(stripChartVar3Button, &QAction::triggered, this, [this]{ clickedStripChartVar(3); });
    // connect(stripChartVar4Button, &QAction::triggered, this, [this]{ clickedStripChartVar(4); });

    stripChartVarMenu = new QMenu(this);
    
    connect(stripChartVarMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(stripChartVarMenuTriggered(QAction*)));

    clearMenu();

    // Use the same menu for each button. When a button is pressed its
    // id is recorded then matched up the current chart and variable
    // name.
    stripChartVarButton0->setMenu(stripChartVarMenu);
    stripChartVarButton1->setMenu(stripChartVarMenu);
    stripChartVarButton2->setMenu(stripChartVarMenu);
    stripChartVarButton3->setMenu(stripChartVarMenu);
    stripChartVarButton4->setMenu(stripChartVarMenu);
    
    stripChartGroup->adjustSize();
}

// ****************************************************************************
// Method: QvisStripChartMgr::clearAll
//
// Purpose: 
//   This is called to clear the strip charts
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::clearAll()
{
  clearMenu();
            
  for( unsigned int j=0; j<MAX_STRIP_CHARTS; ++j )
    stripChartTabWidget->clearAll( j );
}

// ****************************************************************************
// Method: QvisStripChartMgr::clearMenu
//
// Purpose: 
//   This is called to clear the strip chart menus
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::clearMenu()
{
    stripChartMenuMap.clear();
    stripChartActionMap.clear();

    stripChartVarMenu->clear();
    addMenuItem("None");
}

// ****************************************************************************
// Method: QvisStripChartMgr::addMenuItem
//
// Purpose: 
//   This is called to add an item to the strip chart menus
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::addMenuItem( std::string str )
{
    QAction *action;
    
    std::size_t pos = str.find_last_of("/");

    // Found a forward slash so a sub menu is needed.
    if( pos != std::string::npos )
    {
      std::string prefix = str.substr(0,pos);
      std::string var    = str.substr(pos+1);

      // Search for an existing sub menu.
      std::map< std::string, QMenu* >::iterator iter =
        stripChartMenuMap.find(prefix);

      QMenu *menu;

      // Found a sub menu so use it.
      if (iter != stripChartMenuMap.end())
        menu = iter->second;
      // No existing sub menu so add a sub menu
      else
        menu = addSubMenu( prefix );

      // Now add the menu item to the sub menu.
      action = menu->addAction(var.c_str());
    }
    // No forward slash so add the menu item directly.
    else
    {
      action = stripChartVarMenu->addAction(str.c_str());
    }

    // Store the menu action and the full string for later lookup when
    // the menu item is selected.
    stripChartActionMap[action] = str;
}

// ****************************************************************************
// Method: QvisStripChartMgr::addSubMenu
//
// Purpose: 
//   This is called to add a sub menu to the strip chart menus
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
QMenu *
QvisStripChartMgr::addSubMenu( std::string str )
{
  QMenu *menu;

  std::size_t pos = str.find_last_of("/");
  
  // Found a forward slash so a sub menu is needed.
  if( pos != std::string::npos )
  {
    std::string prefix = str.substr(0,pos);
    std::string var    = str.substr(pos+1);
    
    // Search for an existing sub menu.
    std::map< std::string, QMenu* >::iterator iter =
      stripChartMenuMap.find(prefix);

    QMenu *baseMenu;
    
    // Found a sub menu so use it.
    if (iter != stripChartMenuMap.end())
      baseMenu = iter->second;
    // No existing sub menu so add a sub menu
    else
      baseMenu = addSubMenu( prefix );

    // Now add the menu to the sub menu.
    menu = baseMenu->addMenu(var.c_str());
  }
  // No forward slash so add the menu directly.
  else
  {
    menu = stripChartVarMenu->addMenu(str.c_str());
  }

  // Store the string and the menu for later lookup when other menu
  // items are added.
  stripChartMenuMap[str] = menu;

  return menu;
}


// ****************************************************************************
// Method: QvisStripChartMgr::stripChartVarMenuTriggered
//
// Purpose: 
//   This is called when a menu item is triggered and communicates
//   back to the app the name of the menu item selcted.
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::stripChartVarMenuTriggered(QAction *action)
{
  QString varName;

  // Sending a null string resets the curve name.
  if( QString::compare(action->text(), "None" ) == 0 )
    varName.clear();
  // Get the complete variable name for the curve name
  else
    varName = QString( tr(stripChartActionMap[action].c_str()) );
  
  stripChartTabWidget->setCurveTitle(activeVar, varName);

  // Send a callback to the simulation with the chart, curve, and varName.
  std::stringstream tmpstr;
  tmpstr << stripChartTabWidget->getCurrentStripChart() << " | "
         << activeVar << " | "
         << stripChartActionMap[action];
  
  simulationWindow->setStripChartVar( tmpstr.str().c_str() );
}

// ****************************************************************************
// Method: QvisStripChartMgr::clickedStripChartVar0
//
// Purpose: 
//   This is called when a buttom menu item is pressed and is used with
//   stripChartVarMenuTriggered to communicate the variable.
//
// Programmer: Allen Sanderson
// Creation:   16 May 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::clickedStripChartVarButton( int button )
{
  activeVar = button;
}

void
QvisStripChartMgr::clickedStripChartVarButton0()
{
  activeVar = 0;
}

void
QvisStripChartMgr::clickedStripChartVarButton1()
{
  activeVar = 1;
}

void
QvisStripChartMgr::clickedStripChartVarButton2()
{
  activeVar = 2;
}

void
QvisStripChartMgr::clickedStripChartVarButton3()
{
  activeVar = 3;
}

void
QvisStripChartMgr::clickedStripChartVarButton4()
{
  activeVar = 4;
}

// ****************************************************************************
// Method: QvisStripChartMgr::updateCurrentTabData
//
// Purpose: 
//   This is called to update the Mgr widgets in the window.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
// ****************************************************************************
void 
QvisStripChartMgr::updateCurrentTabData()
{
    stripChartTabWidget->updateCurrentTabData();
}

// ****************************************************************************
// Method: QvisStripChartMgr::pick
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisStripChartMgr::pick()
{
    stripChartTabWidget->pick();
}

// ****************************************************************************
// Method: QvisStripChartMgr::zoom()
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
// ****************************************************************************
void
QvisStripChartMgr::zoom()
{
    stripChartTabWidget->zoom();
}

// ****************************************************************************
// Method: QvisStripChartMgr::reset()
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisStripChartMgr::reset()
{
    stripChartTabWidget->reset();
}

// ****************************************************************************
// Method: QvisStripChartMgr::clear()
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
// ****************************************************************************
void
QvisStripChartMgr::clear()
{
    stripChartTabWidget->clear();
}

// ****************************************************************************
// Method: QvisStripChartMgr::clear
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   index    :  index of the strip chart this label should be applied to.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
// ****************************************************************************
void
QvisStripChartMgr::clear( const unsigned int index )
{
    stripChartTabWidget->clear( index );
}

// ****************************************************************************
// Method: QvisStripChartMgr::setTabLabel
//
// Purpose: 
//   This is called to change the name displayed on the tab widget for 
//   the matching strip chart.
//
// Arguments:
//   index    :  index of the strip chart this label should be applied to.
//   newLabel :  the tab label for the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
// 
// ****************************************************************************
void
QvisStripChartMgr::setTabLabel( const unsigned int index, QString newLabel )
{
    stripChartTabWidget->setTabLabel(index, newLabel );
}

// ****************************************************************************
// Method: QvisStripChartMgr::setTabLabel
//
// Purpose: 
//   This is called to change the name displayed on the tab widget for 
//   the matching strip chart.
//
// Arguments:
//   tabIndex    :  index of the strip chart of the curve.
//   curveIndex  :  index of the curve this label should be applied to.
//   newLabel    :  the tab label for the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisStripChartMgr::setCurveTitle( const unsigned int tabIndex,
                                  const unsigned int curveIndex,
                                  QString newTitle )
{
    stripChartTabWidget->setCurveTitle(tabIndex, curveIndex, newTitle );
}

// ****************************************************************************
// Method: QvisStripChartMgr::addDataPoint
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget and then update the Mgr
//   widgets
//
// Arguments:
//   y:  new Y data value
//   x:  new x data value
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::addDataPoint( const unsigned int tabIndex,
                                 const unsigned int curveIndex,
                                 double x, double y )
{
    stripChartTabWidget->addDataPoint(tabIndex, curveIndex, x, y);
}

// ****************************************************************************
// Method: QvisStripChartMgr::addDataPoints
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget and then update the Mgr
//   widgets
//
// Arguments:
//   y:  new Y data value
//   x:  new x data value
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//
// ****************************************************************************
void
QvisStripChartMgr::addDataPoints( const unsigned int tabIndex,
                                  const unsigned int curveIndex,
                                  const unsigned int npts,
                                  double *x, double *y )
{
    stripChartTabWidget->addDataPoints(tabIndex, curveIndex, npts, x, y);
}

// ****************************************************************************
// Method: QvisStripChartMgr::unpost()
//
// Purpose: 
//   This method unpost this window.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void 
QvisStripChartMgr::unpost()
{
   QvisPostableWindow::unpost();
}

// ****************************************************************************
// Method: QvisStripChartMgr::post()
//
// Purpose: 
//   This method will post this window back into the simulation window.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void 
QvisStripChartMgr::post()
{
    QvisPostableWindow::post();
}
