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

#include "QvisStripChartMgr.h"
#include "QvisStripChartTabWidget.h"

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton> 
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>

// ****************************************************************************
// Method: VisItSimStripChart::QvisStripChartMgr
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
    viewer = theViewer;
    simIndex = index;
    engines = engineList;
    addLayoutStretch = false;
    CreateEntireWindow();
    isPosted = false;
    postEnabled = true;
}

// ****************************************************************************
// Method: VisItSimStripChart::~QvisStripChartMgr
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
// Method: VisItSimStripChart::CreateEntireWindow
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
// Method: VisItSimStripChart::CreateWindowContents
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
    chartLayout->setMargin(10);
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

    stripChartGroup->adjustSize();
}

// ****************************************************************************
// Method: VisItSimStripChart::updateCurrentTabData
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
// Method: VisItSimStripChart::pick
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
// Method: VisItSimStripChart::zoom()
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
// Method: VisItSimStripChart::reset()
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
// Method: VisItSimStripChart::clear()
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
// Method: VisItSimStripChart::clear
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
// Method: VisItSimStripChart::setTabLabel
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
// Method: VisItSimStripChart::setTabLabel
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
// Method: VisItSimStripChart::addDataPoint
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
// Method: VisItSimStripChart::unpost()
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
// Method: VisItSimStripChart::post()
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
