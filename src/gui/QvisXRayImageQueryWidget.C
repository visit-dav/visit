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

#include <QvisXRayImageQueryWidget.h>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include <MapNode.h>

// ****************************************************************************
// Method: QvisXRayImageQueryWidget::QvisXRayImageQueryWidget
//
// Purpose: 
//   This is the constructor for the QvisXRayImageQueryWidget class.
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
// Creation:   June 17, 2011 
//
// Modifications:
//   Eric Brugger, Mon May 14 12:31:15 PDT 2012
//   I added the bov output image format.
//
// ****************************************************************************

QvisXRayImageQueryWidget::QvisXRayImageQueryWidget(QWidget *parent,
    Qt::WindowFlags f) : QWidget(parent, f)
{
    // 
    // Main layout for this widget
    // 
    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(10);
  
    // 
    // XRayImage type
    // 
    topLayout->addWidget(new QLabel(tr("Output Image Format")), 0, 0);
    imageFormat = new QComboBox();
    imageFormat->addItem(tr("bmp"));
    imageFormat->addItem(tr("jpeg"));
    imageFormat->addItem(tr("png"));
    imageFormat->addItem(tr("tiff"));
    imageFormat->addItem(tr("rawfloats"));
    imageFormat->addItem(tr("bov"));
    imageFormat->setCurrentIndex(2);
    topLayout->addWidget(imageFormat, 0, 1);

    divideFlag = new QCheckBox(tr("Divide Emis by Absorb"));
    divideFlag->setChecked(0);
    topLayout->addWidget(divideFlag, 1, 0, 1, 2);

    // 
    // origin
    // 
    topLayout->addWidget(new QLabel(tr("Origin")), 2, 0);
    origin = new QLineEdit();
    origin->setText("0 0 0");
    topLayout->addWidget(origin, 2, 1);

    // 
    // Theta
    // 
    topLayout->addWidget(new QLabel(tr("Theta")), 3, 0);
    theta= new QLineEdit();
    theta->setText("0");
    topLayout->addWidget(theta, 3, 1);

    // 
    // Phi
    // 
    topLayout->addWidget(new QLabel(tr("Phi")), 4, 0);
    phi = new QLineEdit();
    phi->setText("0");
    topLayout->addWidget(phi, 4, 1);

    // 
    // Width
    // 
    topLayout->addWidget(new QLabel(tr("Image Width")), 5, 0);
    width= new QLineEdit();
    width->setText("1");
    topLayout->addWidget(width, 5, 1);

    // 
    // Height
    // 
    topLayout->addWidget(new QLabel(tr("Image Height")), 6, 0);
    height = new QLineEdit();
    height->setText("1");
    topLayout->addWidget(height, 6, 1);

    // 
    // Pixel Size
    // 
    topLayout->addWidget(new QLabel(tr("Image Pixel Size")), 7, 0);
    imageSize = new QLineEdit();
    imageSize->setText("200 200");
    topLayout->addWidget(imageSize, 7, 1);
}

// ****************************************************************************
// Method: QvisXRayImageQueryWidget::~QvisXRayImageQueryWidget
//
// Purpose: 
//   This is the destructor for the QvisXRayImageQueryWidget class.
//
// Programmer: Kathleen Biagas
// Creation:   June 17, 2011
//
// Modifications:
//   
// ****************************************************************************

QvisXRayImageQueryWidget::~QvisXRayImageQueryWidget()
{
    // nothing here
}


// ****************************************************************************
// Method: QvisXRayImageQueryWidget::GetDoubleValues
//
// Purpose:
//   Gets double values from a text field.
//
// Arguments:
//   whichWidget     : The text field to parse.
//   n     : The number of values expected
//   pt    : The array in which the values will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 17, 2011
//
// Modifications:
//
// ****************************************************************************

bool 
QvisXRayImageQueryWidget::GetDoubleValues(int whichWidget, int n, double *pt)
{
    QString temp;
  
    if (whichWidget == 0) // Origin
    {
        temp = origin->displayText().simplified();
    }
    else if (whichWidget == 1) // Theta
    {
        temp = theta->displayText().simplified();
    }
    else if (whichWidget == 2) // Phi
    {
        temp = phi->displayText().simplified();
    }
    else if (whichWidget == 3) // Image Width
    {
        temp = width->displayText().simplified();
    }
    else if (whichWidget == 4) // Image Height
    {
        temp = height->displayText().simplified();
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
// Method: QvisXRayImageQueryWidget::GetIntValues
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
// Creation:   June 17, 2011
//
// ****************************************************************************

bool 
QvisXRayImageQueryWidget::GetIntValues(int whichWidget, int *pt)
{
    QString temp;

    if (whichWidget == 3) // Image Pixel Size
    {
        temp = imageSize->displayText().simplified();
    }

    bool okay = !temp.isEmpty();

    if(okay)
    {
        int numScanned = sscanf(temp.toStdString().c_str(), "%d %d",
                            &pt[0], &pt[1]);
        okay = (numScanned == 2 );
    }
    return okay;
}



// ****************************************************************************
// Method: QvisXRayImageQueryWidget::GetQueryParameters
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
// Creation:   June 17, 2011
//
// Modifications:
//    Kathleen Biagas, Wed Sep  7 08:40:22 PDT 2011
//    Return output_type as string instead of int.
// 
// ****************************************************************************
bool
QvisXRayImageQueryWidget::GetQueryParameters(MapNode &params)
{
    doubleVector origin(3);
    double       t, p, w, h;
    intVector    is(2);

    bool noerrors = true;

    if (!GetDoubleValues(0, 3, &origin[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(1, 1, &t))
        noerrors = false;

    if (noerrors && !GetDoubleValues(2, 1, &p))
        noerrors = false;

    if (noerrors && !GetDoubleValues(3, 1, &w))
        noerrors = false;

    if (noerrors && !GetDoubleValues(4, 1, &h))
        noerrors = false;

    if (noerrors && !GetIntValues(3, &is[0]))
        noerrors = false;

    if (noerrors)
    {
        params["output_type"] = imageFormat->currentText().toStdString();
        params["divide_emis_by_absorb"] = (int)divideFlag->isChecked();
        params["origin"] = origin;
        params["theta"] = t;
        params["phi"] = p;
        params["width"] = w;
        params["height"] = h;
        params["image_size"] = is;
    }
    return noerrors; 
}
