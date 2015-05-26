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
//   Kathleen Biagas, Wed Oct 17 12:12:29 PDT 2012
//   Added upVector.
//
//   Eric Brugger, Fri May 22 15:52:32 PDT 2015
//   I updated the window to use the new view description and support the
//   recently added background intensity parameter.
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

    //
    // Divide emissivity by absorptivity
    //
    divideFlag = new QCheckBox(tr("Divide Emis by Absorb"));
    divideFlag->setChecked(0);
    topLayout->addWidget(divideFlag, 1, 0, 1, 2);

    // 
    // Background intensities
    // 
    topLayout->addWidget(new QLabel(tr("background intensities")), 2, 0);
    backgroundIntensities = new QLineEdit();
    backgroundIntensities->setText("0");
    topLayout->addWidget(backgroundIntensities, 2, 1);

    // 
    // Normal
    // 
    topLayout->addWidget(new QLabel(tr("Normal")), 3, 0);
    normal = new QLineEdit();
    normal->setText("0 0 1");
    topLayout->addWidget(normal, 3, 1);

    // 
    // Focus
    // 
    topLayout->addWidget(new QLabel(tr("Focus")), 4, 0);
    focus = new QLineEdit();
    focus->setText("0 0 0");
    topLayout->addWidget(focus, 4, 1);

    // 
    // View up
    // 
    topLayout->addWidget(new QLabel(tr("View up")), 5, 0);
    viewUp = new QLineEdit();
    viewUp->setText("0 1 0");
    topLayout->addWidget(viewUp, 5, 1);

    // 
    // View angle
    // 
    topLayout->addWidget(new QLabel(tr("View angle")), 6, 0);
    viewAngle = new QLineEdit();
    viewAngle->setText("30");
    topLayout->addWidget(viewAngle, 6, 1);

    // 
    // Parallel scale
    // 
    topLayout->addWidget(new QLabel(tr("Parallel scale")), 7, 0);
    parallelScale = new QLineEdit();
    parallelScale->setText("10");
    topLayout->addWidget(parallelScale, 7, 1);

    // 
    // Near plane
    // 
    topLayout->addWidget(new QLabel(tr("Near plane")), 8, 0);
    nearPlane = new QLineEdit();
    nearPlane->setText("-20");
    topLayout->addWidget(nearPlane, 8, 1);

    // 
    // Far plane
    // 
    topLayout->addWidget(new QLabel(tr("Far plane")), 9, 0);
    farPlane = new QLineEdit();
    farPlane->setText("20");
    topLayout->addWidget(farPlane, 9, 1);

    // 
    // Image pan
    // 
    topLayout->addWidget(new QLabel(tr("Image pan")), 10, 0);
    imagePan = new QLineEdit();
    imagePan->setText("0 0");
    topLayout->addWidget(imagePan, 10, 1);

    // 
    // Image zoom
    // 
    topLayout->addWidget(new QLabel(tr("Image zoom")), 11, 0);
    imageZoom = new QLineEdit();
    imageZoom->setText("1");
    topLayout->addWidget(imageZoom, 11, 1);

    //
    // Perspective
    //
    perspective = new QCheckBox(tr("Perspective"));
    perspective->setChecked(0);
    topLayout->addWidget(perspective, 12, 0, 1, 2);

    // 
    // Image size
    // 
    topLayout->addWidget(new QLabel(tr("Image Size")), 13, 0);
    imageSize = new QLineEdit();
    imageSize->setText("500 500");
    topLayout->addWidget(imageSize, 13, 1);
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
//   pt    : The array in which the values will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Eric Brugger
// Creation:   May 22, 2015
//
// Modifications:
//
// ****************************************************************************

bool 
QvisXRayImageQueryWidget::GetDoubleValues(int whichWidget, doubleVector &pt)
{
    QString temp;
  
    if (whichWidget == 0) // Background intensities
    {
        temp = backgroundIntensities->displayText().simplified();
    }
    bool okay = !temp.isEmpty();

    if(okay)
    {
        QStringList s = temp.split(" ", QString::SkipEmptyParts);
        for (int i = 0; okay && i < s.size(); ++i)
        {
            double val = s[i].toDouble(&okay);
            if (okay) pt.push_back(val);
        }
    }

    return okay;
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
//   Kathleen Biagas, Wed Oct 17 12:12:29 PDT 2012
//   Added upVector.
//
//   Eric Brugger, Fri May 22 15:52:32 PDT 2015
//   I updated the window to use the new view description and support the
//   recently added background intensity parameter.
//
// ****************************************************************************

bool 
QvisXRayImageQueryWidget::GetDoubleValues(int whichWidget, int n, double *pt)
{
    QString temp;
  
    if (whichWidget == 1) // Normal
    {
        temp = normal->displayText().simplified();
    }
    else if (whichWidget == 2) // Focus
    {
        temp = focus->displayText().simplified();
    }
    else if (whichWidget == 3) // View up
    {
        temp = viewUp->displayText().simplified();
    }
    else if (whichWidget == 4) // View angle
    {
        temp = viewAngle->displayText().simplified();
    }
    else if (whichWidget == 5) // Parallel scale
    {
        temp = parallelScale->displayText().simplified();
    }
    else if (whichWidget == 6) // Near plane
    {
        temp = nearPlane->displayText().simplified();
    }
    else if (whichWidget == 7) // Far plane
    {
        temp = farPlane->displayText().simplified();
    }
    else if (whichWidget == 8) // Image pan
    {
        temp = imagePan->displayText().simplified();
    }
    else if (whichWidget == 9) // Image zoom
    {
        temp = imageZoom->displayText().simplified();
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
//   Gets a point from the text fields containing ints.
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
// Modifications:
//   Eric Brugger, Fri May 22 15:52:32 PDT 2015
//   I updated the window to use the new view description and support the
//   recently added background intensity parameter.
//
// ****************************************************************************

bool 
QvisXRayImageQueryWidget::GetIntValues(int whichWidget, int *pt)
{
    QString temp;

    if (whichWidget == 10) // Image size
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
//   Kathleen Biagas, Wed Sep  7 08:40:22 PDT 2011
//   Return output_type as string instead of int.
// 
//   Kathleen Biagas, Wed Oct 17 12:12:29 PDT 2012
//   Added upVector.
//
//   Eric Brugger, Fri May 22 15:52:32 PDT 2015
//   I updated the window to use the new view description and support the
//   recently added background intensity parameter.
//
// ****************************************************************************
bool
QvisXRayImageQueryWidget::GetQueryParameters(MapNode &params)
{
    doubleVector backgroundIntensities;
    doubleVector normal(3);
    doubleVector focus(3);
    doubleVector viewUp(3);
    double       viewAngle, parallelScale, nearPlane, farPlane;
    doubleVector imagePan(2);
    double       imageZoom;
    intVector    imageSize(2);

    bool noerrors = true;

    if (!GetDoubleValues(0, backgroundIntensities))
        noerrors = false;
    if (backgroundIntensities.size() == 0)
        noerrors = false;

    if (noerrors && !GetDoubleValues(1, 3, &normal[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(2, 3, &focus[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(3, 3, &viewUp[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(4, 1, &viewAngle))
        noerrors = false;

    if (noerrors && !GetDoubleValues(5, 1, &parallelScale))
        noerrors = false;

    if (noerrors && !GetDoubleValues(6, 1, &nearPlane))
        noerrors = false;

    if (noerrors && !GetDoubleValues(7, 1, &farPlane))
        noerrors = false;

    if (noerrors && !GetDoubleValues(8, 2, &imagePan[0]))
        noerrors = false;

    if (noerrors && !GetDoubleValues(9, 1, &imageZoom))
        noerrors = false;

    if (noerrors && !GetIntValues(10, &imageSize[0]))
        noerrors = false;

    if (noerrors)
    {
        params["output_type"] = imageFormat->currentText().toStdString();
        params["divide_emis_by_absorb"] = (int)divideFlag->isChecked();
        if (backgroundIntensities.size() == 1)
            params["background_intensity"] = backgroundIntensities[0];
        else
            params["background_intensities"] = backgroundIntensities;
        params["normal"] = normal;
        params["focus"] = focus;
        params["view_up"] = viewUp;
        params["view_angle"] = viewAngle;
        params["parallel_scale"] = parallelScale;
        params["near_plane"] = nearPlane;
        params["far_plane"] = farPlane;
        params["image_pan"] = imagePan;
        params["image_zoom"] = imageZoom;
        params["perspective"] = (int)perspective->isChecked();;
        params["image_size"] = imageSize;
    }
    return noerrors; 
}
