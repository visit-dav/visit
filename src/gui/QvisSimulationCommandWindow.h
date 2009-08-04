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
#ifndef QVIS_SIMULATION_COMMAND_WINDOW_H
#define QVIS_SIMULATION_COMMAND_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

// ****************************************************************************
// Class: QvisSimulationCommandWindow
//
// Purpose:
//   Shows messages in the simulation window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul  8 13:26:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisSimulationCommandWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisSimulationCommandWindow(const QString &caption = QString::null,
                                const QString &shortName = QString::null,
                                QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationCommandWindow();
    virtual void CreateEntireWindow();

    void setButtonCommand(int index, const QString &cmd);
    void setButtonEnabled(int index, bool);
    void setCustomButton(int index);
    void setTimeValues(bool timeRanging, 
                       const QString &start, 
                       const QString &stop,
                       const QString &step);

signals:
    void executeButtonCommand(const QString &cmd);
    void showCommandWindow();
    void timeRangingToggled(const QString &);
    void executeStart(const QString &);
    void executeStop(const QString &);
    void executeStep(const QString &);
public slots:
    void post();
    void unpost();
private slots:
    void handleTimeRanging(bool);
    void handleStart();
    void handleStop();
    void handleStep();
    void handleCommandButton(int);
protected:
    virtual void CreateWindowContents();
private:
    QGroupBox          *timeGroup;
    QLineEdit          *startCycle;
    QLineEdit          *stopCycle;
    QLineEdit          *stepCycle;
    QLabel             *startLabel;
    QLabel             *stopLabel;
    QLabel             *stepLabel;

    int                 commandButtonIndex;
    QButtonGroup       *commandGroup;
    QPushButton        *cmdButtons[6];
};

#endif
