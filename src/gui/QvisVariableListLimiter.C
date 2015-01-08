/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <QvisVariableListLimiter.h>

#include <QTimer>
#include <QVBoxLayout>


#include <QvisHistogramLimits.h>

QvisVariableListLimiter::QvisVariableListLimiter(QWidget *parent) :
    QScrollArea(parent), widgets()
{
    deleteObject = 0;

    central = new QWidget(this);
    setWidget(central);
    setWidgetResizable(true);
//central = this;

    vLayout = new QVBoxLayout(central);
}

QvisVariableListLimiter::~QvisVariableListLimiter()
{
    for(int i = 0; i < widgets.size(); ++i)
        delete widgets[i];
}

void
QvisVariableListLimiter::setNumVariables(int n)
{
    if(n > widgets.size())
    {
        // Make more histogramlimits widgets
        while(n > widgets.size())
        {
            QvisHistogramLimits *limits = new QvisHistogramLimits(0);
            limits->setCloseEnabled(true);
            connect(limits, SIGNAL(closeClicked()),
                    this, SLOT(closeVariable()));
            connect(limits, SIGNAL(selectedRangeChanged(const QString&,float,float)),
                    this, SIGNAL(selectedRangeChanged(const QString&,float,float)));
//qDebug("Adding new histogram widget");
            vLayout->addWidget(limits);
            widgets.append(limits);
        }
    }
    else if(n < widgets.size())
    {
        // Remove some widgets.
        while(n < widgets.size())
        {
            QvisHistogramLimits *limits = widgets.back();
            if(limits != 0)
            {
//qDebug("Deleting histogram widget");
                vLayout->removeWidget(limits);
                widgets.removeLast();
                delete limits;
            }
        }
    }
}

int
QvisVariableListLimiter::getNumVariables() const
{
    return widgets.size();
}

QvisHistogramLimits *
QvisVariableListLimiter::getVariable(int i)
{
    QvisHistogramLimits *retval = 0;
    if(i >= 0 && i < widgets.size())
        retval = widgets[i];
    return retval;
}

void
QvisVariableListLimiter::closeVariable()
{
    QvisHistogramLimits *obj = (QvisHistogramLimits *)sender();
    deleteObject = obj;
    QTimer::singleShot(10, this, SLOT(handleObjectDeletion()));
}

void
QvisVariableListLimiter::handleObjectDeletion()
{
    if(deleteObject != 0)
    {
        QString var(deleteObject->getVariable());

        vLayout->removeWidget(deleteObject);
        int index = widgets.indexOf(deleteObject);
        if(index >= 0)
        {
            widgets.removeAt(index);
            delete deleteObject;
        }
        deleteObject = 0;

        emit deleteVariable(var);
    }
}

