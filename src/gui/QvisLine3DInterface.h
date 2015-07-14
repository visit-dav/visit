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

#ifndef QVIS_LINE3D_INTERFACE_H
#define QVIS_LINE3D_INTERFACE_H

#include <QvisAnnotationObjectInterface.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisLineWidthWidget;
class QvisLineStyleWidget;

// ****************************************************************************
// Class: QvisLine3DInterface
//
// Purpose:
//   This class lets you set attributes for a line annotation.
//
// Notes:      
//
// Programmer: Kathleen Biagas 
// Creation:   July 13, 2015
//
// Modifications:
//   Kathleen Biagas, Tue Jul 14 16:39:07 PDT 2015
//   Add support for arrow and tube style lines.
//
// ****************************************************************************

class GUI_API QvisLine3DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT

public:
    QvisLine3DInterface(QWidget *parent);
    virtual ~QvisLine3DInterface();

    virtual QString GetName() const { return "3D Line"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);

protected:
    virtual void UpdateControls();

private slots:
    void point1Changed();
    void point2Changed();
    void widthChanged(int);
    void styleChanged(int);
    void colorChanged(const QColor &);
    void opacityChanged(int);
    void visibilityToggled(bool);
    void useForegroundColorToggled(bool);
    void beginArrowToggled(bool);
    void endArrowToggled(bool);
    void arrow1ResolutionChanged(int);
    void arrow2ResolutionChanged(int);
    void arrow1RadiusChanged();
    void arrow2RadiusChanged();
    void lineTypeChanged(int);
    void tubeQualityChanged(int);
    void tubeRadiusChanged();
   

private:
    QLineEdit              *point1Edit;
    QLineEdit              *point2Edit;
    QComboBox              *lineType;
    QvisLineWidthWidget    *widthWidget;
    QLabel                 *widthLabel;
    QvisLineStyleWidget    *styleWidget;
    QLabel                 *styleLabel;
    QComboBox              *tubeQuality;
    QLabel                 *tubeQualLabel;
    QLineEdit              *tubeRadius;
    QLabel                 *tubeRadLabel;
    QCheckBox              *useForegroundColorCheckBox;
    QLabel                 *colorLabel;
    QvisColorButton        *colorButton;
    QvisOpacitySlider      *opacitySlider;

    QCheckBox              *beginArrow;
    QLabel                 *res1Label;
    QSpinBox               *arrow1Resolution;
    QLabel                 *rad1Label;
    QLineEdit              *arrow1Radius;
    QCheckBox              *endArrow;
    QLabel                 *res2Label;
    QSpinBox               *arrow2Resolution;
    QLabel                 *rad2Label;
    QLineEdit              *arrow2Radius;

    QCheckBox              *visibleCheckBox;
};

#endif
