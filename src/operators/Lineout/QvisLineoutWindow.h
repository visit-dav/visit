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

#ifndef QVISLINEOUTWINDOW_H
#define QVISLINEOUTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class LineoutAttributes;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;

// ****************************************************************************
// Class: QvisLineoutWindow
//
// Purpose: 
//   Defines QvisLineoutWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Nov 19 11:39:48 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Dec 21 11:46:18 PDT 2004
//   Added version-specific Qt coding so we can use pre-3.2 versions.
//
// ****************************************************************************

class QvisLineoutWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisLineoutWindow(const int type,
                         LineoutAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLineoutWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void point1ProcessText();
    void point2ProcessText();
    void interactiveChanged(bool val);
    void ignoreGlobalChanged(bool val);
    void samplingOnChanged(bool val);
    void numberOfSamplePointsProcessText();
    void reflineLabelsChanged(bool val);
  private:
    QLineEdit *point1;
    QLineEdit *point2;
    QCheckBox *interactive;
#if QT_VERSION >= 0x030200
    QGroupBox *ignoreGlobal;
#else
    QCheckBox *ignoreGlobal;
    QGroupBox *ignoreGlobalGroup;
#endif
    QCheckBox *samplingOn;
    QLineEdit *numberOfSamplePoints;
    QCheckBox *reflineLabels;
    QLabel    *numberOfSamplePointsLabel;

    LineoutAttributes *atts;
};



#endif
