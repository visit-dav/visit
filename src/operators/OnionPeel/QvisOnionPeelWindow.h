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

#ifndef QVISONIONPEELWINDOW_H
#define QVISONIONPEELWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class OnionPeelAttributes;
class QButtonGroup;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QvisSILSetSelector;

// ****************************************************************************
// Class: QvisOnionPeelWindow
//
// Purpose: 
//   Defines QvisOnionPeelWindow class.
//
// Notes:      This class was automatically generated!
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Added silUseSet and silAtts.
//   
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004 
//   Added useGlobalId checkbox, related methods and method
//   UpdateComboBoxesEnabledState. 
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group and 'seedTypeChanged' slot.
//
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced widgets/slots/var associated with Category/Set with
//   QvisSILSetSelector and its slots.
//
// ****************************************************************************

class QvisOnionPeelWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisOnionPeelWindow(const int type,
                        OnionPeelAttributes *subj,
                        const QString &caption = QString::null,
                        const QString &shortName = QString::null,
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisOnionPeelWindow();
  protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void adjacencyTypeChanged(int val);
    void seedTypeChanged(int val);
    void categoryChanged(const QString &);
    void subsetChanged(const QString &);
    void indexChanged();
    void useGlobalIdToggled(bool val);
    void requestedLayerChanged(int val);
    void delayedApply();
  private:
    QButtonGroup *adjacencyType;
    QButtonGroup *seedType;

    QvisSILSetSelector *silSet;
    QLineEdit *index;
    QSpinBox  *requestedLayer;
    QCheckBox *useGlobalId;

    OnionPeelAttributes *atts;
};



#endif
