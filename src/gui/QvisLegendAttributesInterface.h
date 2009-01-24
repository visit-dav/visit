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

#ifndef QVIS_LEGEND_ATTRIBUTES_INTERFACE_H
#define QVIS_LEGEND_ATTRIBUTES_INTERFACE_H
#include <QvisAnnotationObjectInterface.h>

// Forward declarations
class QComboBox;
class QCheckBox;
class QLabel;
class QNarrowLineEdit;
class QSpinBox;
class QvisColorButton;
class QvisOpacitySlider;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisLegendAttributesInterface
//
// Purpose:
//   This class lets you set attributes for a legend.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 09:18:40 PDT 2007
//
// Modifications:
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Added orientationComboBox
//   
//    Hank Childs, Fri Jan 23 15:19:28 PST 2009
//    Add support for whether or not to draw the "min/max" portion of the
//    legend.
//
// ****************************************************************************

class GUI_API QvisLegendAttributesInterface : public QvisAnnotationObjectInterface
{
    Q_OBJECT
public:
    QvisLegendAttributesInterface(QWidget *parent);
    virtual ~QvisLegendAttributesInterface();

    virtual QString GetName() const { return "Legend"; }
    virtual QString GetMenuText(const AnnotationObject &) const;

    virtual void GetCurrentValues(int which);
    virtual bool AllowInstantiation() const { return false; }
protected:
    virtual void UpdateControls();

    bool GetBool(int bit) const;
    void SetBool(int bit, bool val);

private slots:
    void layoutChanged(bool);
    void positionChanged(double, double);
    void widthChanged(int);
    void heightChanged(int);
    void orientationChanged(int);
    void textChanged();
    void fontHeightChanged();
    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void fontFamilyChanged(int);
    void boldToggled(bool);
    void italicToggled(bool);
    void shadowToggled(bool);
    void useForegroundColorToggled(bool);
    void drawLabelsToggled(bool);
    void drawMinmaxToggled(bool);
    void drawBoundingBoxToggled(bool);
    void boundingBoxColorChanged(const QColor &);
    void boundingBoxOpacityChanged(int);
    void drawTitleToggled(bool);
private:
    QCheckBox              *manageLayout;
    QvisScreenPositionEdit *positionEdit;
    QLabel                 *positionLabel;
    QSpinBox               *widthSpinBox;
    QSpinBox               *heightSpinBox;
    QComboBox              *orientationComboBox;
    QNarrowLineEdit        *formatString;
    QNarrowLineEdit        *fontHeight;
    QLabel                 *textColorLabel;
    QvisColorButton        *textColorButton;
    QvisOpacitySlider      *textColorOpacity;
    QComboBox              *fontFamilyComboBox;
    QCheckBox              *boldCheckBox;
    QCheckBox              *italicCheckBox;
    QCheckBox              *shadowCheckBox;
    QCheckBox              *useForegroundColorCheckBox;

    QCheckBox              *drawLabelsCheckBox;
    QCheckBox              *drawMinmaxCheckBox;
    QCheckBox              *drawTitleCheckBox;

    QCheckBox              *drawBoundingBoxCheckBox;
    QvisColorButton        *boundingBoxColorButton;
    QvisOpacitySlider      *boundingBoxOpacity;
};

#endif
