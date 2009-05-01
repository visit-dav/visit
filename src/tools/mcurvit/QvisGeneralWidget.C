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
#include <QvisGeneralWidget.h>

#include <QLabel>
#include <QListWidget>
#include <QSpinBox>
#include <QVBoxLayout>

#include <VisItViewer.h>

#include <AttributeSubject.h>
#include <DataNode.h>

// ****************************************************************************
// Method: QvisGeneralWidget::QvisGeneralWidget
//
// Purpose: 
//   Constructor.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//
// ****************************************************************************

QvisGeneralWidget::QvisGeneralWidget(QWidget *parent, VisItViewer *v) :
    QWidget(parent)
{
    viewer = v;

    CreateWindow();
}

// ****************************************************************************
// Method: QvisGeneralWidget::~QvisGeneralWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

QvisGeneralWidget::~QvisGeneralWidget()
{
}

// ****************************************************************************
// Method: QvisGeneralWidget::Update
//
// Purpose: 
//   Update the widgets associated with the specified subject.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisGeneralWidget::Update(Subject *subject)
{
    if (subject == indexSelectAtts)
        UpdateIndexSelectWidgets();
}

// ****************************************************************************
// Method: QvisGeneralWidget::SubjectRemoved
//
// Purpose: 
//   Tell the window that the subject being observed is no longer valid.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisGeneralWidget::SubjectRemoved(Subject *subject)
{
    if (subject == indexSelectAtts)
       indexSelectAtts = 0;
}

// ****************************************************************************
// Method: QvisGeneralWidget::ConnectIndexSelectAttributes
//
// Purpose:
//   Makes the window observe the index select attributes.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisGeneralWidget::ConnectIndexSelectAttributes(AttributeSubject *i)
{
    indexSelectAtts = i;
    i->Attach(this);
}

// ****************************************************************************
// Method: QvisGeneralWidget::CreateWindow
//
// Purpose:
//   Create the contents of the window.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisGeneralWidget::CreateWindow()
{
    QVBoxLayout *gLayout = new QVBoxLayout(this);
    gLayout->setMargin(5);
    gLayout->setSpacing(10);

    // Create the variable list.
    variableLabel = new QLabel(tr("Variable"), this);
    gLayout->addWidget(variableLabel);

    variableList = new QListWidget(this);
    gLayout->addWidget(variableList);
    connect(variableList, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(variableChanged(const QString &)));

    // Create a layout for the remaining widgets.
    QGridLayout *mainLayout = new QGridLayout(0);
    gLayout->addLayout(mainLayout);

    // Add the domain widgets.
    domainLabel = new QLabel(tr("Domain"), this);
    mainLayout->addWidget(domainLabel, 0,0);

    domainMinLabel = new QLabel(tr("min"), this);
    mainLayout->addWidget(domainMinLabel, 0,1);

    domainMin = new QSpinBox(this);
    domainMin->setRange(0, 10000);
    domainMin->setSingleStep(1);
    connect(domainMin, SIGNAL(valueChanged(int)),
            this, SLOT(domainMinChanged(int)));
    mainLayout->addWidget(domainMin, 0,2);

    domainMaxLabel = new QLabel(tr("max"), this);
    mainLayout->addWidget(domainMaxLabel, 0,3);

    domainMax = new QSpinBox(this);
    domainMax->setRange(-1, 10000);
    domainMax->setSingleStep(1);
    domainMax->setSpecialValueText(tr("max"));
    domainMax->setValue(-1);
    connect(domainMax, SIGNAL(valueChanged(int)),
            this, SLOT(domainMaxChanged(int)));
    mainLayout->addWidget(domainMax, 0,4);

    // Add the range widgets.
    rangeLabel = new QLabel(tr("Range"), this);
    mainLayout->addWidget(rangeLabel,1,0);

    rangeMinLabel = new QLabel(tr("min"), this);
    mainLayout->addWidget(rangeMinLabel,1,1);

    rangeMin = new QSpinBox(this);
    rangeMin->setRange(0, 10000);
    rangeMin->setSingleStep(1);
    connect(rangeMin, SIGNAL(valueChanged(int)),
            this, SLOT(rangeMinChanged(int)));
    mainLayout->addWidget(rangeMin, 1,2);

    rangeMaxLabel = new QLabel(tr("max"), this);
    mainLayout->addWidget(rangeMaxLabel,1,3);

    rangeMax = new QSpinBox(this);
    rangeMax->setRange(-1, 10000);
    rangeMax->setSingleStep(1);
    rangeMax->setSpecialValueText(tr("max"));
    rangeMax->setValue(-1);
    connect(rangeMax, SIGNAL(valueChanged(int)),
            this, SLOT(rangeMaxChanged(int)));
    mainLayout->addWidget(rangeMax, 1,4);
}

// ****************************************************************************
// Method: QvisGeneralWidget::UpdateIndexSelectWidgets
//
// Purpose:
//   Update the widgets associated with the index select attributes.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 10:01:23 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisGeneralWidget::UpdateIndexSelectWidgets()
{
    // Get a data node representation of the index select attributes.
    DataNode root("root");
    indexSelectAtts->CreateNode(&root, true, false);
    DataNode *indexSelectNode = root.GetNode("IndexSelectAttributes");
    
    // Update the domain minimum.
    DataNode *xMinNode = indexSelectNode->GetNode("xMin");
    domainMin->blockSignals(true);
    domainMin->setValue(xMinNode->AsInt());
    domainMin->blockSignals(false);

    // Update the domain maximum.
    DataNode *xMaxNode = indexSelectNode->GetNode("xMax");
    domainMax->blockSignals(true);
    domainMax->setValue(xMaxNode->AsInt());
    domainMax->blockSignals(false);

    // Update the range minimum.
    DataNode *yMinNode = indexSelectNode->GetNode("yMin");
    rangeMin->blockSignals(true);
    rangeMin->setValue(yMinNode->AsInt());
    rangeMin->blockSignals(false);

    // Update the range maximum.
    DataNode *yMaxNode = indexSelectNode->GetNode("yMax");
    rangeMax->blockSignals(true);
    rangeMax->setValue(yMaxNode->AsInt());
    rangeMax->blockSignals(false);
}

void
QvisGeneralWidget::ClearVariableList()
{
    variableList->clear();
}

void
QvisGeneralWidget::AddVariable(const char *var)
{
    variableList->addItem(var);
}

void
QvisGeneralWidget::SetCurrentVariable(const int iVar)
{
    variableList->setCurrentRow(iVar);
}

int
QvisGeneralWidget::GetCurrentVariable()
{
    return variableList->currentRow();
}

QString
QvisGeneralWidget::GetCurrentVariableName()
{
    return variableList->currentItem()->text();
}

//
// Qt slot functions
//

void
QvisGeneralWidget::variableChanged(const QString &var)
{
    emit variableTextChanged(var);
}

void
QvisGeneralWidget::domainMinChanged(int min)
{
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    AttributeSubject *atts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(atts != 0)
    {
        atts->SetValue("xMin", min);
        atts->Notify();
        emit indexSelectChanged(atts);
    }
}

void
QvisGeneralWidget::domainMaxChanged(int max)
{
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    AttributeSubject *atts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(atts != 0)
    {
        atts->SetValue("xMax", max);
        atts->Notify();
        emit indexSelectChanged(atts);
    }
}

void
QvisGeneralWidget::rangeMinChanged(int min)
{
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    AttributeSubject *atts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(atts != 0)
    {
        atts->SetValue("yMin", min);
        atts->Notify();
        emit indexSelectChanged(atts);
    }
}

void
QvisGeneralWidget::rangeMaxChanged(int max)
{
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    AttributeSubject *atts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(atts != 0)
    {
        atts->SetValue("yMax", max);
        atts->Notify();
        emit indexSelectChanged(atts);
    }
}
