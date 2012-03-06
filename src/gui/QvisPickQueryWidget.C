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

#include <QvisPickQueryWidget.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

#include <MapNode.h>
using std::string;

// ****************************************************************************
// Method: QvisPickQueryWidget::QvisPickQueryWidget
//
// Purpose: 
//   This is the constructor for the QvisPickQueryWidget class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the instance.
//   f      : The window flags.
//
// Note:       This widget acts as a layout for its children since it 
//   positions the children manually and dispatches mouse events to them.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//
// ****************************************************************************

QvisPickQueryWidget::QvisPickQueryWidget(QWidget *parent,
    Qt::WindowFlags f) : QWidget(parent, f)
{
    // 
    // Main layout for this widget
    // 
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(10);
  
    // 
    // Pick type
    // 
    pickType = new QComboBox();
    pickType->addItem(tr("Pick using coordinate to determine zone")); 
    pickType->addItem(tr("Pick using coordinate to determine node")); 
    pickType->addItem(tr("Pick using domain and element Id")); 
    pickType->addItem(tr("Pick using global element Id")); 
    connect(pickType, SIGNAL(activated(int)),
            this, SLOT(pickTypeChanged(int)));
    pickType->setCurrentIndex(0);
    topLayout->addWidget(pickType);

    // 
    // coordinate
    // 
    coordinateLabel = new QLabel(tr("Coordinate"));
    coordinate = new QLineEdit();
    coordinate->setText("0 0 0");
    QGridLayout *cLayout = new QGridLayout(); 
    cLayout->addWidget(coordinateLabel, 0, 0);
    cLayout->addWidget(coordinate, 0, 1);
    topLayout->addLayout(cLayout);

    // 
    // element and domain
    // 
    elementType = new QButtonGroup();
    QRadioButton *node= new QRadioButton(tr("Node Id"));
    elementType->addButton(node, 0);
    node->setChecked(true);
    QRadioButton *zone= new QRadioButton(tr("Zone Id"));
    elementType->addButton(zone, 1);
    element = new QLineEdit();
    element->setText("0");

    domainLabel = new QLabel(tr("Domain Id") );
    domain = new QLineEdit();
    domain->setText("0");

    QGridLayout *elLayout = new QGridLayout();
    elLayout->addWidget(node, 0, 0);
    elLayout->addWidget(zone, 0, 1);
    elLayout->addWidget(element, 0, 2);
    elLayout->addWidget(domainLabel, 1, 1);
    elLayout->addWidget(domain, 1, 2);

    topLayout->addLayout(elLayout);

    // 
    // Time curve options
    // 
    tcbox = new QGroupBox();
    tcbox->setTitle(tr("Time Curve options:"));
    topLayout->addWidget(tcbox);
    QVBoxLayout *vtcLayout = new QVBoxLayout(tcbox);
    QGridLayout *gtcLayout = new QGridLayout();
    gtcLayout->setMargin(0);
    vtcLayout->addLayout(gtcLayout);

    timePreserveType = new QButtonGroup(tcbox);
    QRadioButton *pc = new QRadioButton(tr("Preserve Picked Coordinate"));
    timePreserveType->addButton(pc, 0);
    pc->setChecked(true);
    QRadioButton *pe = new QRadioButton(tr("Preserve Picked Element Id"));
    timePreserveType->addButton(pe, 1);
    gtcLayout->addWidget(pc, 0, 0);
    gtcLayout->addWidget(pe, 1, 0);


    // 
    // Multiple-variable Time curve options
    // 

    mcbox = new QGroupBox();
    mcbox->setTitle(tr("Multiple-variable Time Curve options:"));
    topLayout->addWidget(mcbox);
    QVBoxLayout *vmcLayout = new QVBoxLayout(mcbox);
    QGridLayout *gmcLayout = new QGridLayout();
    gmcLayout->setMargin(0);
    vmcLayout->addLayout(gmcLayout);

    plotType = new QButtonGroup(mcbox);
    QRadioButton *sa = new QRadioButton(tr("Create Single Y-Axis plot"));
    plotType->addButton(sa, 0);
    sa->setChecked(true);
    QRadioButton *ma = new QRadioButton(tr("Create Multiple Y-Axes plot"));
    plotType->addButton(ma, 1);
    gmcLayout->addWidget(sa, 0, 0);
    gmcLayout->addWidget(ma, 1, 0);

    UpdateControls();
}

// ****************************************************************************
// Method: QvisPickQueryWidget::~QvisPickQueryWidget
//
// Purpose: 
//   This is the destructor for the QvisPickQueryWidget class.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//   
// ****************************************************************************

QvisPickQueryWidget::~QvisPickQueryWidget()
{
    // nothing here
}



// ****************************************************************************
// Method: QvisPickQueryWidget::UpdateControls
//
// Purpose:
//   Sets enabled state of controls.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//
// ****************************************************************************

void
QvisPickQueryWidget::UpdateControls()
{
    switch (pickType->currentIndex())
    {
      case 0: // Pick by zone coordinate
      case 1: // Pick by node coordinate
         coordinate->setEnabled(true);
         coordinate->show();
         coordinateLabel->setEnabled(true);
         coordinateLabel->show();
         elementType->button(0)->setEnabled(false);
         elementType->button(1)->setEnabled(false);
         elementType->button(0)->hide();
         elementType->button(1)->hide();
         domain->setEnabled(false);
         domain->hide();
         domainLabel->setEnabled(false);
         domainLabel->hide();
         element->setEnabled(false);
         element->hide();
         break;
       
      case 2: // Pick by domain/element
         coordinate->setEnabled(false);
         coordinate->hide();
         coordinateLabel->setEnabled(false);
         coordinateLabel->hide();
         elementType->button(0)->setEnabled(true);
         elementType->button(1)->setEnabled(true);
         elementType->button(0)->show();
         elementType->button(1)->show();
         domain->setEnabled(true);
         domain->show();
         domainLabel->setEnabled(true);
         domainLabel->show();
         element->setEnabled(true);
         element->show();
         break;

      case 3: // Pick by domain/element
         coordinate->setEnabled(false);
         coordinate->hide();
         coordinateLabel->setEnabled(false);
         coordinateLabel->hide();
         elementType->button(0)->setEnabled(true);
         elementType->button(1)->setEnabled(true);
         elementType->button(0)->show();
         elementType->button(1)->show();
         domain->setEnabled(false);
         domain->hide();
         domainLabel->setEnabled(false);
         domainLabel->hide();
         element->setEnabled(true);
         element->show();
         break;

      default:
         break;
    }
}

// ****************************************************************************
// Method: QvisPickQueryWidget::GetPoint
//
// Purpose:
//   Gets a point from the coordinate text field.
//
// Arguments:
//   pt    : The array in which the point will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//
// ****************************************************************************

bool 
QvisPickQueryWidget::GetPoint(double *pt)
{
    QString temp(coordinate->displayText().simplified());
    bool okay = !temp.isEmpty();
    if(okay)
    {
        pt[2] = 0.;
        int numScanned = sscanf(temp.toStdString().c_str(), "%lg %lg %lg",
                        &pt[0], &pt[1], &pt[2]);
        okay = (numScanned == 2 || numScanned == 3);
    }
    return okay;
}

// ****************************************************************************
// Method: QvisPickQueryWidget::GetElementType
//
// Purpose: 
//   Retrieves the elementType
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//
// ****************************************************************************

int 
QvisPickQueryWidget::GetPickType(void)
{
    return pickType->currentIndex();
}


// ****************************************************************************
// Method: QvisPickQueryWidget::GetDomain
//
// Purpose: 
//   Retrieves the domain number from the text field.
//
// Arguments:
//   num   : The integer in which the number will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

bool
QvisPickQueryWidget::GetDomain(int *num)
{
    QString temp(domain->displayText().simplified());
    bool okay = !temp.isEmpty();
    if (okay)
    {
        okay = (sscanf(temp.toStdString().c_str(), "%d", num) == 1);
    }
    return okay;
}


// ****************************************************************************
// Method: QvisPickQueryWidget::GetElement
//
// Purpose: 
//   Retrieves the element number from the text field.
//
// Arguments:
//   num   : The integer in which the number will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

bool 
QvisPickQueryWidget::GetElement(int *num)
{
    QString temp(element->displayText().simplified());
    bool okay = !temp.isEmpty();
    if (okay)
    {
        okay = (sscanf(temp.toStdString().c_str(), "%d", num) == 1);
    }
    return okay;
}


// ****************************************************************************
// Method: QvisPickQueryWidget::GetElementType
//
// Purpose: 
//   Retrieves the elementType
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

int 
QvisPickQueryWidget::GetElementType(void)
{
    return elementType->checkedId();
}

// ****************************************************************************
// Method: QvisPickQueryWidget::GetPlotType
//
// Purpose: 
//   Retrieves the plotType
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

int 
QvisPickQueryWidget::GetPlotType(void)
{
    return plotType->checkedId();
}
 

// ****************************************************************************
// Method: QvisPickQueryWidget::GetTimePreservesCoord
//
// Purpose: 
//   Retrieves the time curve type.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

bool 
QvisPickQueryWidget::GetTimePreservesCoord(void)
{
    return timePreserveType->checkedId() == 0;
}


bool
QvisPickQueryWidget::GetQueryParameters(MapNode &params)
{
    doubleVector p(3);
    bool noerrors = true;
    int  curvePlotType = GetPlotType();
    int preserveCoord = (int) GetTimePreservesCoord();
    int dom = 0, el = 0;
    switch (pickType->currentIndex())
    {
      case 0: // Pick by zone coordinate
          if (!GetPoint(&p[0]))
              noerrors = false;
          if (noerrors)
          {
              params["pick_type"] = string("Zone");
              params["coord"] = p;
          }
          break;
      case 1: // Pick by node coordinate
          if (!GetPoint(&p[0]))
              noerrors = false;
          if (noerrors)
          {
              params["pick_type"] = string("Node");
              params["coord"] = p;
          }
          break;
      case 2: // Pick by domain and element
          if (!GetElement(&el))
              noerrors = false;
          if (!GetDomain(&dom))
              noerrors = false;
          if (noerrors)
          {
              if (GetElementType() == 0)
                  params["pick_type"] = string("DomainNode");
              else 
                  params["pick_type"] = string("DomainZone");
              params["element"] = el;
              params["domain"] = dom;
          }
          break;
      case 3: // Pick by global element 
          if (!GetElement(&el))
              noerrors = false;
          if (noerrors)
          { 
              if (GetElementType() == 0)
                  params["pick_type"] = string("DomainNode");
              else 
                  params["pick_type"] = string("DomainZone");
              params["element"] = el;
              params["use_global_id"] = 1;
          }
          break;
      default:
          break;
    } 

    if (noerrors)
    {
        params["curve_plot_type"] = curvePlotType;
        params["preserve_coord"] = preserveCoord;
    }
    return noerrors; 
}


//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisPickQueryWidget::pickTypeChanged
//
// Purpose: 
//   Updates the controls displayed.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisPickQueryWidget::pickTypeChanged(int)
{
    UpdateControls();
}

// ****************************************************************************
// Method: QvisPickQueryWidget::timeOptionsToggled
//
// Purpose: 
//   Updates the controls displayed.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisPickQueryWidget::timeOptionsToggled(bool val)
{
    tcbox->setEnabled(val);
    mcbox->setEnabled(val);
}

