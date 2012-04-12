/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_TIMEQUERY_OPTIONSWIDGET_H
#define QVIS_TIMEQUERY_OPTIONSWIDGET_H

#include <gui_exports.h>

#include <QGroupBox>

#include <GUIBase.h>
#include <MapNode.h>

class QLabel;
class QSpinBox;
class PlotList;

// ****************************************************************************
// Class: QvisTimeQueryOptionsWidget
//
// Purpose: 
//   Defines QvisTimeQueryOptionsWidget class.
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011
//
// Modifications:
//    Kathleen Biagas, Wed Apr 11 19:10:38 PDT 2012
//    Removed GetDatabaseNStates, added UpdateState.
//
// ****************************************************************************

class GUI_API QvisTimeQueryOptionsWidget : public QGroupBox, public GUIBase
{
    Q_OBJECT
  public:
    QvisTimeQueryOptionsWidget(QWidget *parent = 0);
    QvisTimeQueryOptionsWidget(const QString &title, QWidget *parent = 0);
    virtual ~QvisTimeQueryOptionsWidget();

    bool GetTimeQueryOptions(MapNode &options);

    void UpdateState(PlotList *plotList);

    virtual void setCheckable(bool val);
 
  public slots:
    virtual void show();
    virtual void setEnabled(bool);

  private:
    void         CreateWindowContents();
    void         SetMax(const int val);

    QLabel      *msgLabel;
    QSpinBox    *startTime;
    QSpinBox    *endTime;
    QSpinBox    *stride;
    QLabel      *startLabel;
    QLabel      *endLabel;
    QLabel      *strideLabel;
    int maxTime;
};

#endif
