// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

