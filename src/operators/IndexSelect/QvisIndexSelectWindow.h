/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef QVISINDEXSELECTWINDOW_H
#define QVISINDEXSELECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IndexSelectAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QSpinBox;
class QvisSILSetSelector;

// ****************************************************************************
// Class: QvisIndexSelectWindow
//
// Purpose: 
//   Defines QvisIndexSelectWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004
//   Changed Min/Max/Incr from LineEdit to SpinBox for usability, added
//   labels and group boxes for each dim.
//   
//   Kathleen Bonnell, Thu Jun  7 11:40:58 PDT 2007 
//   Added QvisSILSetSelector, removed domainIndex, groupIndex.
//   Changed whichData buttongroup to useWholeCollection checkbox.
// 
// ****************************************************************************

class QvisIndexSelectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIndexSelectWindow(const int type,
                         IndexSelectAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIndexSelectWindow();
  protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dimChanged(int val);
    void oneDMinChanged(int min);
    void oneDMaxChanged(int max);
    void oneDIncrChanged(int incr);
    void twoDMinChanged(int min);
    void twoDMaxChanged(int max);
    void twoDIncrChanged(int incr);
    void threeDMinChanged(int min);
    void threeDMaxChanged(int max);
    void threeDIncrChanged(int incr);
    void useWholeCollectionToggled(bool val);
    void categoryChanged(const QString &);
    void subsetChanged(const QString &);
  private:
    QButtonGroup *dim;

    QGroupBox    *oneDWidgetGroup;
    QLabel       *oneDLabel;
    QSpinBox     *oneDMin;
    QSpinBox     *oneDMax;
    QSpinBox     *oneDIncr;

    QGroupBox    *twoDWidgetGroup;
    QLabel       *twoDLabel;
    QSpinBox     *twoDMin;
    QSpinBox     *twoDMax;
    QSpinBox     *twoDIncr;

    QGroupBox    *threeDWidgetGroup;
    QLabel       *threeDLabel;
    QSpinBox     *threeDMin;
    QSpinBox     *threeDMax;
    QSpinBox     *threeDIncr;

    QCheckBox *useWholeCollection;

    QvisSILSetSelector *silSet;
    IndexSelectAttributes *atts;
};



#endif
