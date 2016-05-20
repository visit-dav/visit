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
#ifndef QVIS_STRIPCHART_MGR
#define QVIS_STRIPCHART_MGR

#include <QWidget>
#include <QGroupBox>
#include <QvisPostableWindow.h>

class QvisStripChartTabWidget;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QTreeView;
class QString;
class QSpinBox;
class QCheckBox;
class QGridLayout;
class QScrollArea;
class QColor;

class EngineList;
class ViewerProxy;

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
//   Brad Whitlock, Tue Jul  8 09:05:14 PDT 2008
//   Qt 4.
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

    void addDataPoint( const QString name,
                       const QString var,
                       double x, double y );

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

protected:
    virtual void CreateWindowContents();

private:
    QvisStripChartTabWidget *stripChartTabWidget;

    QGridLayout        *chartLayout;
    QPushButton        *pickButton;
    QPushButton        *zoomButton;
    QPushButton        *resetButton;
    QPushButton        *clearButton;

    QScrollArea        *sc;
    QCheckBox          *enableLogScale;
    
    bool               posted;
    
    EngineList         *engines;
    ViewerProxy        *viewer;
    int                simIndex;
};
#endif /* QVIS_STRIPCHART_MGR */
