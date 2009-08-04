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

#ifndef QVISQUERYOVERTIMEWINDOW_H
#define QVISQUERYOVERTIMEWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class QueryOverTimeAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;

// ****************************************************************************
// Class: QvisQueryOverTimeWindow
//
// Purpose: 
//   Defines QvisQueryOverTimeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:32:17 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class QvisQueryOverTimeWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisQueryOverTimeWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void timeTypeChanged(int val);
    void startTimeFlagChanged(bool val);
    void startTimeProcessText();
    void endTimeFlagChanged(bool val);
    void endTimeProcessText();
    void strideProcessText();
    void createWindowChanged(bool val);
    void windowIdProcessText();
  private:
    QButtonGroup *timeType;
    QCheckBox *startTimeFlag;
    QLineEdit *startTime;
    QCheckBox *endTimeFlag;
    QLineEdit *endTime;
    QLineEdit *stride;
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QLabel *strideLabel;
    QLabel *createWindowLabel;
    QLabel *windowIdLabel;

    QueryOverTimeAttributes *atts;
};



#endif
