/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef QVIS_TIME_SLIDER_INTERFACE_H
#define QVIS_TIME_SLIDER_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisTimeSliderInterface
//
// Purpose:
//   This class lets you set attributes for a time slider annotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 12:47:34 PDT 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Jan 13 08:39:30 PST 2005
//   Added timeFormatLinEdit and timeFormatChanged slot.
//
//   Brad Whitlock, Fri Jul 18 16:20:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisTimeSliderInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisTimeSliderInterface(QWidget *parent);
    virtual ~QvisTimeSliderInterface();

    virtual QString GetName() const { return "Time slider"; }

    virtual void GetCurrentValues(int which);
protected:
    virtual void UpdateControls();
private slots:
    void positionChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void labelChanged();
    void timeFormatChanged();
    void startColorChanged(const QColor &);
    void startOpacityChanged(int);
    void endColorChanged(const QColor &);
    void endOpacityChanged(int);
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void visibilityToggled(bool);
    void roundedToggled(bool);
    void shadedToggled(bool);
    void timeDisplayChanged(int);
    void useForegroundColorToggled(bool);
private:
    QvisScreenPositionEdit *positionEdit;
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QvisColorButton        *startColorButton;
    QvisOpacitySlider      *startColorOpacity;
    QvisColorButton        *endColorButton;
    QvisOpacitySlider      *endColorOpacity;
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QCheckBox              *useForegroundColorCheckBox;
    QLineEdit              *labelLineEdit;
    QLineEdit              *timeFormatLineEdit;
    QComboBox              *timeDisplayComboBox;
    QCheckBox              *visibleCheckBox;
    QCheckBox              *roundedCheckBox;
    QCheckBox              *shadedCheckBox;
};

#endif
