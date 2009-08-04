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
#ifndef QVIS_AXIS_ATTRIBUTES_WIDGET_H
#define QVIS_AXIS_ATTRIBUTES_WIDGET_H
#include <QWidget>
#include <GUIBase.h>
#include <AxisAttributes.h>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QNarrowLineEdit;
class QSpinBox;
class QvisFontAttributesWidget;

// ****************************************************************************
// Class: QvisAxisAttributesWidget
//
// Purpose:
//   Widget that makes it easier to show/update AxisAttributes objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:55:10 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:59:31 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Jan 16 11:12:48 EST 2009
//   Allow clients to not expose the "ShowGrid" and custom title/units settings
//
// ****************************************************************************

class QvisAxisAttributesWidget : public QWidget, public GUIBase
{
    Q_OBJECT
public:
    QvisAxisAttributesWidget(QWidget *parent, bool ticksCheckEnabled=true,
                             bool titleCheckEnabled=true,
                             bool showGridEnabled=true,
                             bool customTitleAndUnitsEnabled=true);
    virtual ~QvisAxisAttributesWidget();

    void setAxisAttributes(const AxisAttributes &);
    const AxisAttributes &getAxisAttributes();

    void setAutoScaling(bool val);
    void setAutoTickMarks(bool val);
signals:
    void axisChanged(const AxisAttributes &);
private slots:
    void Apply();

    void titleToggled(bool);
    void customTitleToggled(bool);
    void customUnitsToggled(bool);
    void titleFontChanged(const FontAttributes &);

    void labelToggled(bool);
    void labelScalingChanged(int);
    void labelFontChanged(const FontAttributes &);

    void tickToggled(bool);

    void gridToggled(bool);
private:
    void Update(int,int);
    void GetCurrentValues(AxisAttributes &, int = -1, int = -1);
    void ForceSpinBoxUpdate(QSpinBox *sb);
    bool GetDouble(double &val, QLineEdit *le, const QString &name);

    AxisAttributes           atts;
    bool                     autoScaling;
    bool                     autoTickMarks;

    // Title widgets
    QGroupBox                *titleGroup;
    QCheckBox                *customTitleToggle;
    QLineEdit                *customTitle;
    QCheckBox                *customUnitsToggle;
    QLineEdit                *customUnits;
    QvisFontAttributesWidget *titleFont;

    // Label widgets
    QGroupBox                *labelGroup;
    QLabel                   *labelScalingLabel;
    QSpinBox                 *labelScaling;
    QvisFontAttributesWidget *labelFont;

    // Tick widgets
    QGroupBox                *tickGroup;
    QLabel                   *majorMinimumLabel;
    QNarrowLineEdit          *majorMinimum;
    QLabel                   *majorMaximumLabel;
    QNarrowLineEdit          *majorMaximum;
    QLabel                   *minorSpacingLabel;
    QNarrowLineEdit          *minorSpacing;
    QLabel                   *majorSpacingLabel;
    QNarrowLineEdit          *majorSpacing;

    // Grid widgets
    QCheckBox                *grid;
};

#endif
