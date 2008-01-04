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

#ifndef QVISANIMATIONWINDOW_H
#define QVISANIMATIONWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AnimationAttributes;
class QButtonGroup;
class QCheckBox;
class QSlider;

// ****************************************************************************
// Class: QvisAnimationWindow
//
// Purpose: 
//   This class creates a window that has animation controls.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 13:47:37 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:31:55 PDT 2002
//   Added controls for the animation playback speed.
//
//   Brad Whitlock, Mon Oct 6 16:19:59 PST 2003
//   Added a controls to let the users set the animation style.
//
// ****************************************************************************

class GUI_API QvisAnimationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAnimationWindow(AnimationAttributes *subj,
                        const char *caption = 0,
                        const char *shortName = 0,
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisAnimationWindow();
    virtual void CreateWindowContents();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
private slots:
    virtual void apply();
    void pipelineCachingToggled(bool val);
    void playbackModeChanged(int val);
    void timeoutChanged(int);
private:
    AnimationAttributes *animationAtts;

    QCheckBox    *pipelineCachingToggle;
    QSlider      *timeoutSlider;
    QButtonGroup *playbackModeButtonGroup;
};

#endif
