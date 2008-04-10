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

#ifndef QVIS_GLOBALLINEOUT_WINDOW_H
#define QVIS_GLOBALLINEOUT_WINDOW_H

#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class GlobalLineoutAttributes;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;


// ****************************************************************************
// Class: QvisGlobalLineoutWindow
//
// Purpose: 
//   Defines QvisGlobalLineoutWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Nov 19 10:46:23 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Added widgets for new atts (colorOption, curveOption).  Modified
//   'dynamic' widget. 
//   
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Added freezeInTime widget.
//
//   Brad Whitlock, Wed Apr  9 11:47:09 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisGlobalLineoutWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisGlobalLineoutWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
  protected:
    void UpdateWindow(bool);
    void GetCurrentValues(int);
    void Apply(bool ignore = false);
  private slots:
    void dynamicChanged(bool);
    void freezeInTimeChanged(bool);
    void createWindowChanged(bool);
    void windowIdProcessText();
    void samplingOnChanged(bool);
    void numSamplesProcessText();
    void createReflineLabelsChanged(bool);
    void curveOptionsChanged(int);
    void colorOptionsChanged(int);
  private:
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QCheckBox *samplingOn;
    QLineEdit *numSamples;
    QCheckBox *createReflineLabels;
    QLabel *windowIdLabel;
    QLabel *numSamplesLabel;

#if QT_VERSION >= 0x030200
    QGroupBox *dynamic;
#else
    QCheckBox *dynamic;
    QGroupBox *dynamicGroup;
#endif
    QComboBox    *curveOptions;
    QLabel       *curveLabel;
    QComboBox    *colorOptions;
    QLabel       *colorLabel;
    QCheckBox    *freezeInTime;

    GlobalLineoutAttributes *atts;
};



#endif
