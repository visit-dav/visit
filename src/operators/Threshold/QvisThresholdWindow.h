/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVISTHRESHOLDWINDOW_H
#define QVISTHRESHOLDWINDOW_H

#include <QvisOperatorWindow.h>
#include <ThresholdAttributes.h>

class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisThresholdWindow
//
// Purpose: 
//   Defines QvisThresholdWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//
//   Brad Whitlock, Fri Dec 10 09:39:57 PDT 2004
//   Added a variable button.
//
//   Hank Childs, Thu Sep 15 15:31:34 PDT 2005
//   Added windowing for creating point meshes.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Certain Threshold changes are
//   preserved when viewer sends attributes that do not know about the changes.
//   (See full explanation in header of RestoreAppropriateUnappliedAttributes.)
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
// ****************************************************************************

class QvisThresholdWindow : public QvisOperatorWindow
{
    Q_OBJECT

public:
    QvisThresholdWindow(const int type,
                        ThresholdAttributes *subj,
                        const char *caption = 0,
                        const char *shortName = 0,
                        QvisNotepadArea *notepad = 0);
    virtual            ~QvisThresholdWindow();
    virtual void        CreateWindowContents();

protected:
    void                UpdateWindow(bool doAll);
    virtual void        GetCurrentValues(int which_widget);

private slots:
    void                apply();
    void                outputMeshTypeChanged(int buttonID);
    void                zonePortionChanged(int buttonID);
    void                lowerBoundChanged();
    void                upperBoundChanged();
    void                prevVarClicked();
    void                nextVarClicked();
    void                variableAdded(const QString &variableToAdd);
    void                variableDeleted(const QString &variableToDelete);
    void                variableSwapped(const QString &variableToSwapIn);

private:
    void                UpdateShownFields();

/* No longer used --- see update comment above for 9/21/06
    void                RestoreAppropriateUnappliedAttributes();
*/

    QButtonGroup        *outputMeshType;
    QButtonGroup        *zonePortion;
    QLabel              *zonePortionLabel;
    QLineEdit           *lowerBound;
    QLabel              *lowerBoundLabel;
    QLineEdit           *upperBound;
    QLabel              *upperBoundLabel;
    QLabel              *shownVariable;
    QPushButton         *showPrevVariable;
    QPushButton         *showNextVariable;
    QvisVariableButton  *addVariable;
    QvisVariableButton  *deleteVariable;
    QvisVariableButton  *swapVariable;

    const QBitmap       *leftArrowBitmap;
    const QBitmap       *rightArrowBitmap;

    ThresholdAttributes *atts;

/* No longer used --- see update comment above for 9/21/06
    ThresholdAttributes *guiVarAtts;
*/
};

#endif
