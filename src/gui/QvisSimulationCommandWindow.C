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
#include <QvisSimulationCommandWindow.h>

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

#define MAX_CMD_BUTTONS 6

QvisSimulationCommandWindow::QvisSimulationCommandWindow(
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindow(caption, shortName, notepad)
{
    commandButtonIndex = 5;
    commandGroup = 0;
    addLayoutStretch = false;
    CreateEntireWindow();
}

QvisSimulationCommandWindow::~QvisSimulationCommandWindow()
{
}

void
QvisSimulationCommandWindow::CreateEntireWindow()
{
    QvisPostableWindow::CreateEntireWindow();
    dismissButton->setEnabled(false);
}

void
QvisSimulationCommandWindow::CreateWindowContents()
{
    // Create the group box and generic buttons.
    QGroupBox *cmdGroup = new QGroupBox(tr("Commands"), central);
    topLayout->addWidget(cmdGroup);
    QGridLayout *buttonLayout2 = new QGridLayout(cmdGroup);
    commandGroup = new QButtonGroup(cmdGroup);
    connect(commandGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(handleCommandButton(int)));
    for (int r=0; r<2; r++)
    {
        for (int c=0; c<3; c++)
        {
            cmdButtons[r*3+c] = new QPushButton("", cmdGroup);
            buttonLayout2->addWidget(cmdButtons[r*3+c],r,c);
            commandGroup->addButton(cmdButtons[r*3+c], r*3+c);
            cmdButtons[r*3+c]->hide();
        }
    }

    // Create time controls.
    timeGroup = new QGroupBox(tr("Enable time ranging"), central);
    timeGroup->setCheckable(true);
    connect(timeGroup, SIGNAL(toggled(bool)),
            this, SLOT(handleTimeRanging(bool)));
    topLayout->addWidget(timeGroup);
    
    QGridLayout *timeLayout = new QGridLayout(timeGroup);
    startCycle = new QLineEdit(timeGroup);
    startLabel = new QLabel(timeGroup);
    startLabel->setText(tr("Start"));
    timeLayout->addWidget(startLabel,0,0);
    timeLayout->addWidget(startCycle,0,1);
    connect(startCycle,SIGNAL(returnPressed()),this,SLOT(handleStart()));

    stepCycle = new QLineEdit(timeGroup);
    stepLabel = new QLabel(timeGroup);
    stepLabel->setText(tr("Step"));
    timeLayout->addWidget(stepLabel,0,2);
    timeLayout->addWidget(stepCycle,0,3);
    connect(stepCycle,SIGNAL(returnPressed()),this,SLOT(handleStep()));
    
    stopCycle = new QLineEdit(timeGroup);
    stopLabel = new QLabel(timeGroup);
    stopLabel->setText(tr("Stop"));
    timeLayout->addWidget(stopLabel,0,4);
    timeLayout->addWidget(stopCycle,0,5);
    connect(stopCycle,SIGNAL(returnPressed()),this,SLOT(handleStop()));
}

void
QvisSimulationCommandWindow::setButtonCommand(int index, const QString &cmd)
{
    if(index >= 0 && index < MAX_CMD_BUTTONS)
        cmdButtons[index]->setText(cmd);
}

void
QvisSimulationCommandWindow::setButtonEnabled(int index, bool enabled)
{
    if(index >= 0 && index < MAX_CMD_BUTTONS)
    {
        if(enabled)
            cmdButtons[index]->show();
        else
            cmdButtons[index]->hide();
    }
}

void
QvisSimulationCommandWindow::setCustomButton(int index)
{
    if(index >= 0 && index < MAX_CMD_BUTTONS)
    {
        commandButtonIndex = index;
        cmdButtons[index]->setText(tr("Custom . . ."));
    }
}

void
QvisSimulationCommandWindow::setTimeValues(bool timeRanging, 
    const QString &start, const QString &stop, const QString &step)
{
    timeGroup->setChecked(timeRanging);
    startCycle->setText(start);
    stopCycle->setText(stop);
    stepCycle->setText(step);
}

//
// Qt slots
//

void
QvisSimulationCommandWindow::handleCommandButton(int btn)
{
    if(btn == commandButtonIndex)
        emit showCommandWindow();
    else
        emit executeButtonCommand(commandGroup->button(btn)->text());
}

void
QvisSimulationCommandWindow::handleTimeRanging(bool b)
{
    if(b)
    {
        QString value(startCycle->text().trimmed());
        if(!value.isEmpty())
            emit timeRangingToggled(value);
    }
}

void
QvisSimulationCommandWindow::handleStart()
{
    QString value(startCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStart(value);
}

void
QvisSimulationCommandWindow::handleStop()
{
    QString value(stopCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStop(value);
}

void
QvisSimulationCommandWindow::handleStep()
{
    QString value(stepCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStep(value);
}

void 
QvisSimulationCommandWindow::unpost()
{
    QvisPostableWindow::unpost();
}

void 
QvisSimulationCommandWindow::post()
{
    QvisPostableWindow::post();
}
