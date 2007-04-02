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

#ifndef QVIS_COMMAND_WINDOW_H
#define QVIS_COMMAND_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;
class QTextEdit;

// ****************************************************************************
// Class: QvisCommandWindow
//
// Purpose:
//   This class implements a window that lets you type commands to be
//   interpreted.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:20:44 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jan 6 13:35:40 PST 2006
//   Added new buttons for recording macros.
//
// ****************************************************************************

class QvisCommandWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisCommandWindow(const char *captionString = 0,
                      const char *shortName = 0,
                      QvisNotepadArea *n = 0);
    virtual ~QvisCommandWindow();
    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
signals:
    void runCommand(const QString &);
public slots:
    void acceptRecordedMacro(const QString &);
private slots:
    void executeClicked(int);
    void clearClicked(int);

    void macroRecordClicked();
    void macroPauseClicked();
    void macroEndClicked();
    void macroAppendClicked(bool);
    void macroStorageActivated(int);

    void textChanged0();
    void textChanged1();
    void textChanged2();
    void textChanged3();
    void textChanged4();
    void textChanged5();
    void textChanged6();
    void textChanged7();
private:
    QString fileName(int index) const;
    void LoadScripts();
    void SaveScripts();
    void UpdateMacroCheckBoxes();

    QTabWidget      *tabWidget;
    QButtonGroup    *executeButtonsGroup;
    QPushButton    **executeButtons;
    QButtonGroup    *clearButtonsGroup;
    QPushButton    **clearButtons;
    QTextEdit      **lineEdits;

    QPushButton     *macroRecord;
    QPushButton     *macroPause;
    QPushButton     *macroEnd;
    QCheckBox       *macroAppendCheckBox;
    QComboBox       *macroStorageComboBox;
    
    int              macroStorageMode;
    bool             macroAppend;
};

#endif
