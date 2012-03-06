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

#include "QvisTimeQueryOptionsWidget.h"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::QvisTimeQueryOptionsWidget
//
// Purpose: 
//   Constructor
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011 
//
// Modifications:
//
// ****************************************************************************

QvisTimeQueryOptionsWidget::QvisTimeQueryOptionsWidget(QWidget *parent)
    : QGroupBox(parent), GUIBase()
{
    maxTime = 99;

    setCheckable(true);
    setChecked(false);
    setTitle(tr("Do Time Curve"));
    CreateWindowContents();
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::QvisTimeQueryOptionsWidget
//
// Purpose: 
//   Constructor
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011 
//
// Modifications:
//
// ****************************************************************************

QvisTimeQueryOptionsWidget::QvisTimeQueryOptionsWidget(const QString & title, 
    QWidget *parent) : QGroupBox(title, parent), GUIBase()
{
    maxTime = 99;

    setCheckable(true);
    setChecked(false);
    setTitle(title);
    CreateWindowContents();
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::CreateWindowContents
//
// Purpose: 
//   Creates the widgets this window uses.
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011 
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeQueryOptionsWidget::CreateWindowContents()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);    

    //
    // Message
    //
    QHBoxLayout *msgLayout = new QHBoxLayout();
    msgLabel = new QLabel(tr("Start and End are Time steps,\nnot cycles or times."));
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignHCenter);
    msgLayout->addWidget(msgLabel);

    topLayout->addLayout(msgLayout);

    QGridLayout *gLayout = new QGridLayout();
    topLayout->addLayout(gLayout);

    //
    // StartTime 
    //
    startLabel = new QLabel(tr("Starting timestep"));
    gLayout->addWidget(startLabel, 0, 0);
    QHBoxLayout *startLayout = new QHBoxLayout();
    startTime = new QSpinBox();
    startTime->setValue(0);
    startTime->setMinimum(0);
    startTime->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    gLayout->addWidget(startTime, 0, 1);

    //
    // EndTime 
    //
    endLabel   = new QLabel(tr("Ending timestep"));
    gLayout->addWidget(endLabel, 1, 0);
    endTime = new QSpinBox();
    endTime->setValue(0);
    endTime->setMinimum(0);
    endTime->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    gLayout->addWidget(endTime, 1, 1);

    //
    // Stride 
    //
    strideLabel = new QLabel(tr("Stride"));
    gLayout->addWidget(strideLabel, 2, 0);
    stride = new QSpinBox();
    stride->setMinimum(1);
    stride->setValue(1);
    stride->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    gLayout->addWidget(stride, 2, 1);
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::~QvisTimeQueryOptionsWidget
//
// Purpose: 
//   Destructor
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011
//
// Modifications:
//   
// ****************************************************************************

QvisTimeQueryOptionsWidget::~QvisTimeQueryOptionsWidget()
{
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::setCheckable
//
// Purpose: 
//   This is a Qt slot function that sets the checkable state of the widget.
//
// Arguments:
//   val : The new checkable state.
//
// Programmer: Kathleen Biagas 
// Creation:   August 9, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeQueryOptionsWidget::setCheckable(bool val)
{
    QGroupBox::setCheckable(val);
    if (val)
    {
        setTitle(tr("Do Time Query"));
        setChecked(false);
    }
    else 
    {
        setTitle(tr("Time Query Options"));
    }
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::setEnabled
//
// Purpose: 
//   This is a Qt slot function that sets the enabled state of the widget.
//   Also updates the Max for the widgets.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Kathleen Biagas 
// Creation:   August 9, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 15:53:01 MST 2012
//   Make enabled state also depend upon number of time states available.
//   
// ****************************************************************************

void
QvisTimeQueryOptionsWidget::setEnabled(bool val)
{
    int nStates = GetDatabaseNStates();
    QGroupBox::setEnabled(val && nStates > 1);
    if (isEnabled())
    {
        SetMax(nStates -1);
    }
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::show
//
// Purpose: 
//   This is a Qt slot function that shows the widget.
//   Also updates the Max for the widgets.
//
// Programmer: Kathleen Biagas 
// Creation:   August 9, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeQueryOptionsWidget::show()
{
    QGroupBox::show();
    SetMax(GetDatabaseNStates() -1);
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::SetMax
//
// Purpose: 
//   Sets the max value for the widgets.
//
// Arguments:
//   val : The new max.
//
// Programmer: Kathleen Biagas 
// Creation:   August 9, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisTimeQueryOptionsWidget::SetMax(const int val)
{
    if (val > 0)
    {
        maxTime = val;
        startTime->setMaximum(maxTime-1);
        endTime->setMaximum(maxTime);
        endTime->setValue(maxTime);
        if (maxTime > 1)
            stride->setMaximum(maxTime);
        else 
            stride->setMaximum(1);
    }
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::GetTimeQueryOptions
//
// Purpose: 
//   Gets values from the widgets and stores them in the MapNode.
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011 
//
// Modifications:
//   
// ****************************************************************************

bool
QvisTimeQueryOptionsWidget::GetTimeQueryOptions(MapNode &options)
{
    QString temp;
    options["start_time"] = startTime->value();
    options["end_time"] = endTime->value();
    options["stride"] = stride->value();
    return true;
}


// ****************************************************************************
// Method: QvisTimeQueryOptionsWidget::GetDatabaseNStates()
//
// Purpose: 
//   Gets number of states in active database.
//
// Programmer: Kathleen Biagas 
// Creation:   August 1, 2011 
//
// Modifications:
//   Kathleen Biagas, Wed Jan 25 15:56:29 MST 2012
//   Retrieve number of states from active time slider if available, instead
//   of active source.
//
// ****************************************************************************

int
QvisTimeQueryOptionsWidget::GetDatabaseNStates()
{
    WindowInformation *wi = GetViewerState()->GetWindowInformation();
    DatabaseCorrelationList *correlations = 
        GetViewerState()->GetDatabaseCorrelationList();

    // Get the number of states for the active time slider.
    int  ats = wi->GetActiveTimeSlider();
    int nStates = (ats == -1) ? 0 : 1;
    if(ats != -1)
    {
        DatabaseCorrelation c = correlations->GetCorrelations(ats);
        nStates = c.GetNumStates();
    }
    return nStates;
}

