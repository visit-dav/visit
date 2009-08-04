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

#ifndef QVIS_POINT_CONTROLS_H
#define QVIS_POINT_CONTROLS_H
#include <gui_exports.h>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisPointControl
//
// Purpose:
//   This is a widget that encapsulates the individual point control
//   buttons into a Point controls widget.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:01:35 PST 2004
//   I replaced the line edit for the variable with a variable button.
//
//   Brad Whitlock, Wed Jul 20 13:44:39 PST 2005
//   Added a control for setting the point size in terms of pixels for
//   when we render the points as points.
//
//   Brad Whitlock, Thu Aug 25 09:29:56 PDT 2005
//   I changed the point size from a button group to a combo box.
//
//   Cyrus Harrison, Tue Jul  8 09:58:45 PDT 2008
//   Initial Qt4 Port
//
// ****************************************************************************

class GUI_API QvisPointControl : public QWidget
{
    Q_OBJECT

public:
    QvisPointControl(QWidget * parent=0);
    ~QvisPointControl();

    void SetPointSize(double);
    void SetPointSizePixels(int);
    void SetPointSizeVarChecked(bool);
    void SetPointSizeVar(QString &);
    void SetPointType(int);

    double   GetPointSize();
    int      GetPointSizePixels();
    bool     GetPointSizeVarChecked() const;
    QString  &GetPointSizeVar();
    int      GetPointType() const;

signals:
    void pointSizeChanged(double);
    void pointSizePixelsChanged(int);
    void pointSizeVarToggled(bool val);
    void pointSizeVarChanged(const QString &);
    void pointTypeChanged(int);

private slots:
    void processSizeText();
    void sizeVarChanged(const QString &);
    void sizeVarToggled(bool on);
    void typeComboBoxChanged(int);

private:
    void UpdateSizeText();
    void UpdatePointType();
    bool ProcessSizeText(int pointType);

    QLabel                 *sizeLabel;
    QLineEdit              *sizeLineEdit;
    QCheckBox              *sizeVarToggle;
    QvisVariableButton     *sizeVarButton;
    QComboBox              *typeComboBox;

    int                     lastGoodPointType;
    double                  lastGoodSize;
    int                     lastGoodSizePixels;
    QString                 lastGoodVar;
};

#endif
