/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <SimWidgetNames.h>
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

using std::string;

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
//    Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//    added pointSize to update font size. Also added a variable for
//    zoomOutLimit to prevent to small of zooms.
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
    isPosted = FALSE;
    postEnabled = TRUE;
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
//   
// ****************************************************************************
QvisStripChartMgr::~QvisStripChartMgr()
{
// cleanup
}

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
//   Shelly Prevost  Thu Oct 18 14:25:35 PDT 2007
//   added new widgets for cycle display and removed widgets that
//   do not fully function yet.
//  
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Jul  8 09:42:46 PDT 2008
//   Qt 4.
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
    
    chartLayout =  new QGridLayout(stripChartGroup);              
    chartLayout->setMargin(10);
    chartLayout->setSpacing(10);
    
    QLabel *limitLab = new QLabel(tr("Limit Bounds"), stripChartGroup);
    chartLayout->addWidget(limitLab,0,1); 
 
    QLabel *extremaLab = new QLabel(tr("Extrema"), stripChartGroup); 
    chartLayout->addWidget(extremaLab,0,3); 
    
    QLabel *currentLab = new QLabel(tr("Current"), stripChartGroup);
    chartLayout->addWidget(currentLab,0,5); 

    minLimitEdit = new QLineEdit(stripChartGroup);
    minLimitEdit->setObjectName(STRIP_MIN_LIMIT_WIDGET_NAME);
    minLimitEdit->setEnabled(false);
    minLimitLabel = new QLabel(tr("Min"), stripChartGroup);
    minLimitLabel->setBuddy(minLimitEdit);
    chartLayout->addWidget(minLimitLabel,1,0);
    chartLayout->addWidget(minLimitEdit,1,1);
    connect(minLimitEdit,SIGNAL(textChanged(const QString&)),this,SLOT(executeMinLimitStripChart()));
    
    maxLimitEdit = new QLineEdit(stripChartGroup);
    maxLimitEdit->setObjectName(STRIP_MAX_LIMIT_WIDGET_NAME);
    maxLimitEdit->setEnabled(false); 
    maxLimitLabel = new QLabel(tr("Max"), stripChartGroup);
    maxLimitLabel->setBuddy(maxLimitEdit);
    chartLayout->addWidget(maxLimitLabel,2,0);
    chartLayout->addWidget(maxLimitEdit,2,1);
    connect(maxLimitEdit,SIGNAL(textChanged(const QString&)),this,SLOT(executeMaxLimitStripChart()));

    minEdit = new QLineEdit(stripChartGroup);
    minEdit->setObjectName(STRIP_MIN_WIDGET_NAME);
    minEdit->setEnabled(false);
    minEdit->setText("0.0");
    minLabel = new QLabel(tr("Min"), stripChartGroup);
    minLabel->setBuddy(minEdit);
    chartLayout->addWidget(minLabel,1,2);
    chartLayout->addWidget(minEdit,1,3);    


    maxEdit = new QLineEdit(stripChartGroup);
    maxEdit->setObjectName(STRIP_MIN_WIDGET_NAME);
    maxEdit->setEnabled(false);
    maxEdit->setText("0.0");
    maxLabel = new QLabel(tr("Max"), stripChartGroup);
    maxLabel->setBuddy(maxEdit);
    chartLayout->addWidget(maxLabel,2,2);
    chartLayout->addWidget(maxEdit,2,3);
        
    curEdit = new QLineEdit(stripChartGroup);
    curEdit->setObjectName(STRIP_CUR_WIDGET_NAME);
    curEdit->setEnabled(false);
    curEdit->setText("0.0");
    curLabel = new QLabel(tr("Data"), stripChartGroup);
    curLabel->setBuddy(curEdit);
    chartLayout->addWidget(curLabel,1,4);
    chartLayout->addWidget(curEdit,1,5);
    
    cycleEdit = new QLineEdit(stripChartGroup);
    cycleEdit->setObjectName(STRIP_CYCLE_WIDGET_NAME);
    cycleEdit->setEnabled(false);
    cycleEdit->setText("0.0");
    cycleLabel = new QLabel(tr("Cycle"), stripChartGroup);
    cycleLabel->setBuddy(cycleEdit);
    chartLayout->addWidget(cycleLabel,2,4);
    chartLayout->addWidget(cycleEdit,2,5);
    
    enableStripChartLimits = new QCheckBox(stripChartGroup);
    enableStripChartLimits->setText(tr("Enable Limits"));
    connect(enableStripChartLimits, SIGNAL(stateChanged(int)),
            this, SLOT(executeEnableStripChartLimits()));
    chartLayout->addWidget(enableStripChartLimits,3, 0, 1, 3);
      
    //enableLogScale = new QCheckBox(stripChartGroup);
    //enableLogScale->setText("Log Scale");
    //connect(enableLogScale,SIGNAL(stateChanged(int)),this,SLOT(executeEnableLogScale()));
    //chartLayout->addMultiCellWidget(enableLogScale,4,2,1,1);
    
    // zoom and focus buttons
    // Create the group box and generic buttons.
    QGridLayout *zoomLayout = new QGridLayout(0);
    chartLayout->addLayout(zoomLayout, 4, 0, 1, 6);
    resetButton = new QPushButton(tr("Reset"));
    resetButton->setEnabled(true);
    zoomLayout->addWidget(resetButton,0,0);
    connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));
    plusButton = new QPushButton(tr("Zoom in"));
    plusButton->setEnabled(true);
    connect(plusButton,SIGNAL(clicked()),this,SLOT(zoomIn()));
    zoomLayout->addWidget(plusButton,0,1);
    minusButton = new QPushButton(tr("Zoom out"));
    minusButton->setEnabled(true);
    connect(minusButton,SIGNAL(clicked()),this,SLOT(zoomOut()));
    zoomLayout->addWidget(minusButton,0,2);
    focusButton = new QPushButton(tr("Focus"));
    focusButton->setEnabled(true);
    connect(focusButton,SIGNAL(clicked()),this,SLOT(focus()));
    zoomLayout->addWidget(focusButton,0,3);

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
//    Shelly Prevost  Thu Oct 18 14:25:35 PDT 2007
//    disabled widget that do not fully function yet.
//   
// ****************************************************************************
void 
QvisStripChartMgr::updateCurrentTabData()
{
    double minY, maxY;
    double minL, maxL;
 
    stripChartTabWidget->updateCurrentTabData();
    stripChartTabWidget->getMinMaxData(minY,maxY);
    setMinMaxStripChartDataDisplay(minY,maxY);
    stripChartTabWidget->getOutOfBandLimits(minL,maxL);
    setLimitStripChartDataDisplay(minL,maxL);
    setCurrentDataDisplay ( stripChartTabWidget->getCurrentData() );
    setCycleDisplay( stripChartTabWidget->getCurrentCycle() );
    enableStripChartLimits->setChecked(stripChartTabWidget->getEnableOutOfBandLimits());
    //enableLogScale->setChecked(stripChartTabWidget->getEnableLogScale());
}

// ****************************************************************************
// Method: VisItSimStripChart::isStripChartWidget
//
// Purpose: 
//   This is called to determine if the parameter name matches one of
//   the strip chart names
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************    
bool 
QvisStripChartMgr::isStripChartWidget( QString name )
{
    return stripChartTabWidget->isStripChartWidget(name);
}

// ****************************************************************************
// Method: VisItSimStripChart::CreateWindowContents
//
// Purpose: 
//   This is called to determine if the parameter name matches one of
//   the strip chart tab label widget names
//
// Arguments:
//   name    :  name of the strip chart this flag should be set for.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
bool 
QvisStripChartMgr::isStripChartTabLabel( QString name )
{
    return stripChartTabWidget->isStripChartTabLabel(name);
}

// ****************************************************************************
// Method: VisItSimStripChart::zoomIn
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
//   
// ****************************************************************************
void 
QvisStripChartMgr::zoomIn()
{
    stripChartTabWidget->zoomIn();
}

// ****************************************************************************
// Method: VisItSimStripChart::zoomOut
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
//   
// ****************************************************************************
void
QvisStripChartMgr::zoomOut()
{
    stripChartTabWidget->zoomOut();
}

// ****************************************************************************
// Method: VisItSimStripChart::reset()
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget and update the Mgr widgets
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void
QvisStripChartMgr::reset()
{
    stripChartTabWidget->reset();
    updateCurrentTabData();
}

// ****************************************************************************
// Method: VisItSimStripChart::setTabLabel
//
// Purpose: 
//   This is called to change the name displayed on the tab widget for 
//   the matching strip chart.
//
// Arguments:
//   tabName    :  name of the strip chart this lable should be applied to.
//   newLabel   :  the tab label for the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void
QvisStripChartMgr::setTabLabel(QString tabName, QString newLabel )
{
    int tabIndex = stripChartTabWidget->nameToTabIndex(tabName);
    stripChartTabWidget->setTabLabel(tabIndex,newLabel );
}

// ****************************************************************************
// Method: VisItSimStripChart::focus()
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
//   
// ****************************************************************************
void 
QvisStripChartMgr::focus()
{
    stripChartTabWidget->focus();
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
//   
// ****************************************************************************
void 
QvisStripChartMgr::post()
{
    QvisPostableWindow::post();
}

// ****************************************************************************
// Method: VisItSimStripChart::getEnable
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   name    :  name of the strip chart this flag should be set for.
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

bool QvisStripChartMgr::getEnable(QString name )
{
    return stripChartTabWidget->getEnable(name);

}

// ****************************************************************************
// Method: VisItSimStripChart::setEnable
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   name    :  name of the strip chart this flag should be set for.
//   enable  : flag used to enable/disable drawing of the strip chart
//
// Programmer: Shelly Prevost
// Creation:   Oct. 27, 2006
//
// Modifications:
//  
//   
// ****************************************************************************

void QvisStripChartMgr::setEnable( QString name, bool enable )
{
    stripChartTabWidget->setEnable(name, enable);

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
//    Shelly Prevost,  Thu Oct 18 16:36:59 PDT 2007
//    fixed return type to pass on out of bounds information.
//   
// ****************************************************************************
bool 
QvisStripChartMgr::addDataPoint ( QString name,double x, double y)
{
    bool outOfBound = stripChartTabWidget->addDataPoint(name,x,y);
    updateCurrentTabData();
    return outOfBound;
}

// ****************************************************************************
// Method: VisItSimStripChart::update()
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
//   
// ****************************************************************************
void 
QvisStripChartMgr::update(QString name)
{
    stripChartTabWidget->update(name);
}

// ****************************************************************************
// Method: VisItSimStripChart::getMinMaxData
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   minY:  minimum Y value
//   minX:  minimum x value
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void 
QvisStripChartMgr::getMinMaxData( QString name, double &minY, double &maxY)
{
    stripChartTabWidget->getMinMaxData( name,minY, maxY);
}

void 
QvisStripChartMgr::enableOutOfBandLimits(QString name, bool enabled)
{
    stripChartTabWidget->enableOutOfBandLimits(name, enabled);
}

// ****************************************************************************
// Method: VisItSimStripChart::setOutOfBandLimits
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   min:  minimum Y value
//   min:  minimum x value
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartMgr::setOutOfBandLimits(QString name,double min, double max)
{
    stripChartTabWidget->setOutOfBandLimits(name,min,max);
}

// ****************************************************************************
// Method: VisItSimStripChart::setOutOfBandLimits
//
// Purpose: 
//   This is a pass through method that call the function with the
//   same name in the strip chat tab widget.
//
// Arguments:
//   min:  minimum Y value
//   min:  minimum x value

// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
// Modifications:
//  
//   
// ****************************************************************************
void QvisStripChartMgr::setOutOfBandLimits(double min, double max)
{
    stripChartTabWidget->setOutOfBandLimits(min,max);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeMaxLimitStripChart()
//
// Purpose:
//   This method is called when the when the user changes the value
//   in the limit strip chart line edit widget. It set the limits 
//   for the strip chart and sends them to the simulation.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 17:21:39 PST 2006
//
// Modifications:
//
//
// ****************************************************************************
void 
QvisStripChartMgr::executeMaxLimitStripChart()
{
    QString cmd2 = minLimitEdit->text();
    QString cmd3 = maxLimitEdit->text();
    stripChartTabWidget->setOutOfBandLimits( cmd2.toDouble(), cmd3.toDouble());
    cmd2 = "returnedPressed();MaxLimitEdit;QLineEdit;Simulations;" + cmd3;
    sendCMD(cmd2);
}

// ****************************************************************************
// Method: VisItSimStripChart::executeEnableLogScale
//
// Purpose: 
//   This is called to enable log ploting of the strip chart data.
//
// Programmer: Shelly Prevost
// Creation:   Wed Sep 26 16:16:23 PDT 2007
//
//    Shelly Prevost  Thu Oct 18 14:25:35 PDT 2007
//    disabled widgets because it is not fully function yet. 
//   
// ****************************************************************************
void 
QvisStripChartMgr::executeEnableLogScale()
{
    //QString cmd = "returnedPressed();EnableLogScale;QLineEdit;Simulations;" + //enableLogScale->isChecked();
    //stripChartTabWidget->setEnableLogScale(enableLogScale->isChecked());
    //sendCMD(cmd);
}


// ****************************************************************************
// Method: QvisSimulationWindow::setMinMaxStripChartDataDisplaye
//
// Purpose:
//   Sets the managed widget that display the min/max of the data so far.
//
// Arguments:
//   minY:  minimum Y value
//   minX:  minimum x value
//
// Programmer: Shelly Prevost
// Creation:   Tue Nov 28 17:12:04 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisStripChartMgr::setMinMaxStripChartDataDisplay (double minY, double maxY)
{
  minEdit->setText(QString::number(minY));
  maxEdit->setText(QString::number(maxY));

}

// ****************************************************************************
// Method: QvisSimulationWindow::setMinMaxStripChartDataDisplaye
//
// Purpose:
//   Sets the managed widget that display the min/max limits to the data.
//
// Arguments:
//   minY:  minimum Y value
//   minX:  minimum x value
//
// Programmer: Shelly Prevost
// Creation:   Tue Nov 28 17:12:04 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisStripChartMgr::setLimitStripChartDataDisplay (double minY, double maxY)
{
  minLimitEdit->setText(QString::number(minY));
  maxLimitEdit->setText(QString::number(maxY));

}

// ****************************************************************************
// Method: QvisSimulationWindow::setCurrentDataDisplay
//
// Purpose:
//   Updates the current data display to the last data value updated.
//
// Arguments:
//   currentData: the last data y value update.
//
// Programmer: Shelly Prevost
// Creation:   Tue Nov 28 17:12:04 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisStripChartMgr::setCurrentDataDisplay (double currentData)
{
  curEdit->setText(QString::number(currentData));
}

// ****************************************************************************
// Method: QvisSimulationWindow::setCurrentDataDisplay
//
// Purpose:
//   Updates the cycle display to the last cycle value dispalyed in the strip chart.
//
// Arguments:
//   currentData: the last data y value update.
//
// Programmer: Shelly Prevost
// Creation:   Thu Oct 18 14:25:35 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisStripChartMgr::setCycleDisplay (int currentCycle)
{
  cycleEdit->setText(QString::number(currentCycle));
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeEnableStripChartLimits()
//
// Purpose:
//   This method is called when the user clicks on the enable button
//   for the stip chart ui. It will set the upper and lower bounds for
//   checking simiulation data for out of band problems.
//
// Programmer: Shelly Prevost
// Creation:   Fri Dec  1 10:36:07 PST 2006  
//
// Modifications:
//
//
// ****************************************************************************
void
QvisStripChartMgr::executeEnableStripChartLimits()
{
    QString cmd = enableStripChartLimits->text();
    cmd = "clicked();EnableStripChartLimits;QCheckBox;Simulations;" + cmd;
    sendCMD( cmd);
    bool enabled = enableStripChartLimits->isChecked();
    maxLimitEdit->setEnabled(enabled);       
    minLimitEdit->setEnabled(enabled); 
    stripChartTabWidget->enableOutOfBandLimits(enabled);      

}


// ****************************************************************************
// Method: QvisSimulationWindow::executeMinLimitStripChart()
//
// Purpose:
//   This method is called when the when the user changes the value
//   in the limit strip chart line edit widget. It set the limits 
//   for the strip chart and sends them to the simulation.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 17:21:39 PST 2006
//
// Modifications:
//
//
// ****************************************************************************
void 
QvisStripChartMgr::executeMinLimitStripChart()
{
    QString cmd2 = minLimitEdit->text();
    QString cmd3 = maxLimitEdit->text();
    stripChartTabWidget->setOutOfBandLimits( cmd2.toDouble(), cmd3.toDouble());
    cmd2 = "returnedPressed();MinLimitEdit;QLineEdit;Simulations;" + cmd2;
    sendCMD( cmd2);
}


// ****************************************************************************
// Method: SimCommandSlots::SendCMD
//
// Purpose:
//   This is the main function called to send a Visit UI signal to
//   the remote running simulation program.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//   Brad Whitlock, Tue Jul  8 09:49:26 PDT 2008
//   Qt 4.
//
// ****************************************************************************

int QvisStripChartMgr::sendCMD(QString sig, const QObject *ui, QString value)
{
    if (!ui)
         QMessageBox::warning(0, "VisIt", tr("Invalid ui component"),
                             tr("Ok"), 0, 0, 0, 1 );
    if (simIndex < 0)
    {
        QMessageBox::warning(0, "VisIt",
                             tr("Invalid index encountered for Sim engine access"),
                             tr("Ok"), 0, 0, 0, 1 );
        return -1;
    }

    // check that there is at least one engine
    const stringVector &s = engines->GetEngines();
    if ( s.size() < 1) return 0;
    string host = engines->GetEngines()[simIndex];
    string sim  = engines->GetSimulationName()[simIndex];

    QString cmd = sig + ";" + ui->objectName() + ";" + ui->metaObject()->className() + ";" +
                  ui->parent()->objectName() + ";" + value;
    viewer->GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
    return 0;
}

// ****************************************************************************
// Method: SimCommandSlots::SendCMD
//
// Purpose:
//   This is the main function called to send a Visit UI signal to
//   the remote running simulation program.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

int QvisStripChartMgr::sendCMD(QString cmd)
{
    if (simIndex < 0)
    {
        QMessageBox::warning(0, "VisIt",
                             tr("Invalid index encountered for Sim engine access"),
                             tr("Ok"), 0, 0, 0, 1 );
        return -1;
    }
    // check that there is at least one engine
    const stringVector &s = engines->GetEngines();
    if ( s.size() < 1) return 0;
    string host = engines->GetEngines()[simIndex];
    string sim  = engines->GetSimulationName()[simIndex];
    viewer->GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
    return 0;
}

