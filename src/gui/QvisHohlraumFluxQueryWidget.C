/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#include <QvisHohlraumFluxQueryWidget.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <MapNode.h>
using std::string;

// ****************************************************************************
// Method: QvisHohlraumFluxQueryWidget::QvisHohlraumFluxQueryWidget
//
// Purpose: 
//   This is the constructor for the QvisHohlraumFluxQueryWidget class.
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
// Creation:   June 20, 2011 
//
// Modifications:
//    Kathleen Biagas, Tue Jul 26 09:03:30 PDT 2011
//    Changed Number of Lines to match default in the actual query.
//
// ****************************************************************************

QvisHohlraumFluxQueryWidget::QvisHohlraumFluxQueryWidget(QWidget *parent,
    Qt::WindowFlags f) : QWidget(parent, f)
{
    // 
    // Main layout for this widget
    // 
    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(10);
  
    // 
    // numLines
    // 
    topLayout->addWidget(new QLabel(tr("Number of Lines")), 0, 0);
    numLines = new QLineEdit();
    numLines->setText("1000");
    topLayout->addWidget(numLines, 0, 1);

    // 
    // DivideEmisByAbsorb
    // 
    divideFlag = new QCheckBox(tr("Divide Emis by Absorb"));
    divideFlag->setChecked(0);
    topLayout->addWidget(divideFlag, 1, 0, 1, 2);

    // 
    // Ray Center
    // 
    topLayout->addWidget(new QLabel(tr("Ray Center")), 2, 0);
    rayCenter = new QLineEdit();
    rayCenter->setText("0 0 0");
    topLayout->addWidget(rayCenter, 2, 1);

    // 
    // Radius
    // 
    topLayout->addWidget(new QLabel(tr("Radius")), 3, 0);
    radius = new QLineEdit();
    radius->setText("1");
    topLayout->addWidget(radius, 3, 1);

    // 
    // Theta
    // 
    topLayout->addWidget(new QLabel(tr("Theta")), 4, 0);
    theta = new QLineEdit();
    theta->setText("0");
    topLayout->addWidget(theta, 4, 1);

    // 
    // Phi
    // 
    topLayout->addWidget(new QLabel(tr("Phi")), 5, 0);
    phi = new QLineEdit();
    phi->setText("0");
    topLayout->addWidget(phi, 5, 1);
}

// ****************************************************************************
// Method: QvisHohlraumFluxQueryWidget::~QvisHohlraumFluxQueryWidget
//
// Purpose: 
//   This is the destructor for the QvisHohlraumFluxQueryWidget class.
//
// Programmer: Kathleen Biagas
// Creation:   June 20, 2011
//
// Modifications:
//   
// ****************************************************************************

QvisHohlraumFluxQueryWidget::~QvisHohlraumFluxQueryWidget()
{
    // nothing here
}


// ****************************************************************************
// Method: QvisHohlraumFluxQueryWidget::GetOrigin
//
// Purpose:
//   Gets a point from the origin text field.
//
// Arguments:
//   pt    : The array in which the point will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 20, 2011
//
// Modifications:
//
// ****************************************************************************

bool 
QvisHohlraumFluxQueryWidget::GetDoubleValues(int whichWidget, int n, double *pt)
{
    QString temp;
  
    if (whichWidget == 2) // RayCenter
    {
        temp = rayCenter->displayText().simplified();
    }
    else if (whichWidget == 3) // Radius
    {
        temp = radius->displayText().simplified();
    }
    else if (whichWidget == 4) // Theta
    {
        temp = theta->displayText().simplified();
    }
    else if (whichWidget == 5) // Phi
    {
        temp = phi->displayText().simplified();
    }
    bool okay = !temp.isEmpty();

    if(okay)
    {
        int numScanned = -1;
        if (n == 1)
        {
            numScanned = sscanf(temp.toStdString().c_str(), "%lg", &pt[0]);
        }
        if (n == 2)
        {
            numScanned = sscanf(temp.toStdString().c_str(), "%lg %lg",
                        &pt[0], &pt[1]);
        }
        else if (n == 3) 
        {
            numScanned = sscanf(temp.toStdString().c_str(), "%lg %lg %lg",
                        &pt[0], &pt[1], &pt[2]);
        }
        okay = (numScanned == n);
    }
    return okay;
}

// ****************************************************************************
// Method: QvisHohlraumFluxQueryWidget::GetIntValues
//
// Purpose:
//   Gets a point from the text fiels containing ints.
//
// Arguments:
//   whichWidget : which widget to retrieve from
//   pt    : The array in which the point will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 20, 2011
//
// ****************************************************************************

bool 
QvisHohlraumFluxQueryWidget::GetIntValues(int whichWidget, int *pt)
{
    QString temp;

    if (whichWidget == 0) // NumLines
    {
        temp = numLines->displayText().simplified();
    }

    bool okay = !temp.isEmpty();

    if(okay)
    {
        int numScanned = sscanf(temp.toStdString().c_str(), "%d", &pt[0]);
        okay = (numScanned == 1 );
    }
    return okay;
}



// ****************************************************************************
// Method: QvisHohlraumFluxQueryWidget::GetQueryParameters
//
// Purpose:
//   Retrieves values from this form and stores them in the MapNode in 
//   a manner in keeping with this query.
//
// Arguments:
//   params :  The MapNode in which to store the parameters.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 20, 2011
//
// Modifications:
//
// ****************************************************************************

bool
QvisHohlraumFluxQueryWidget::GetQueryParameters(MapNode &params)
{
    doubleVector rc(3);
    double       rad, t, p;
    int          nl;

    bool noerrors = true;

    if (!GetIntValues(0, &nl))
        noerrors = false;

    if (!GetDoubleValues(2, 3, &rc[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(3, 1, &rad))
        noerrors = false;

    if (noerrors && !GetDoubleValues(4, 1, &t))
        noerrors = false;

    if (noerrors && !GetDoubleValues(5, 1, &p))
        noerrors = false;

    if (noerrors)
    {
        params["num_lines"] = nl;
        params["divide_emis_by_absorb"] = divideFlag->isChecked();
        params["ray_center"] = rc;
        params["radius"] = rad;
        params["theta"] = t;
        params["phi"] = p;
    }
    return noerrors; 
}
