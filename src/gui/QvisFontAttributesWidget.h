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
#ifndef QVIS_FONT_ATTRIBUTES_WIDGET_H
#define QVIS_FONT_ATTRIBUTES_WIDGET_H
#include <qframe.h>
#include <FontAttributes.h>

class QCheckBox;
class QComboBox;
class QNarrowLineEdit;
class QvisColorButton;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisFontAttributesWidget
//
// Purpose:
//   Widget class that makes it easier to show/update FontAttributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:54:19 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:56:16 PDT 2008
//   Changed fontHeight to fontScale. Added disableOpacity.
//
// ****************************************************************************

class QvisFontAttributesWidget : public QFrame
{
    Q_OBJECT
public:
    QvisFontAttributesWidget(QWidget *parent, const char *name=0);
    virtual ~QvisFontAttributesWidget();

    void setFontAttributes(const FontAttributes &);
    const FontAttributes &getFontAttributes();

    void disableOpacity();
signals:
    void fontChanged(const FontAttributes &);
private slots:
    void Apply();

    void textColorChanged(const QColor &);
    void textOpacityChanged(int);
    void fontFamilyChanged(int);
    void boldToggled(bool);
    void italicToggled(bool);
    void useForegroundColorToggled(bool);
private:
    void GetCurrentValues(FontAttributes &, int = -1);
    void Update(int = -1);
    bool               opacityEnabled;

    FontAttributes     atts;
    QNarrowLineEdit   *fontScale;
    QvisColorButton   *textColorButton;
    QvisOpacitySlider *textColorOpacity;
    QComboBox         *fontFamilyComboBox;
    QCheckBox         *boldCheckBox;
    QCheckBox         *italicCheckBox;
    QCheckBox         *useForegroundColorCheckBox;
};

#endif
