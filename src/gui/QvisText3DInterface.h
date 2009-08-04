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

#ifndef QVIS_TEXT3D_INTERFACE_H
#define QVIS_TEXT3D_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisText3DInterface
//
// Purpose:
//   This class lets you set attributes for a 3D text annotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thurs Nov 8 10:56:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisText3DInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisText3DInterface(QWidget *parent);
    virtual ~QvisText3DInterface();

    virtual QString GetName() const { return "3D Text"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void textChanged();
    void positionChanged();
    void heightModeChanged(int);
    void relativeHeightChanged(int);
    void fixedHeightChanged();
    void facesCameraToggled(bool);
    void rotateZChanged(int);
    void rotateXChanged(int);
    void rotateYChanged(int);
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void useForegroundColorToggled(bool);
    void visibilityToggled(bool);
private:
    QLineEdit         *textLineEdit;
    QLineEdit         *positionEdit;
    QButtonGroup      *heightMode;
    QSpinBox          *relativeHeightSpinBox;
    QLineEdit         *fixedHeightEdit;
    QCheckBox         *facesCameraCheckBox;
    QSpinBox          *rotateZ;
    QSpinBox          *rotateX;
    QSpinBox          *rotateY;
    QvisColorButton   *textColorButton;
    QvisOpacitySlider *textColorOpacity;
    QCheckBox         *useForegroundColorCheckBox;
    QCheckBox         *visibleCheckBox;
};

#endif
