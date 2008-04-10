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

#ifndef QVISKEYFRAMEWINDOW_H
#define QVISKEYFRAMEWINDOW_H

#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <AttributeSubject.h>
#include <vector>
#include <map>

class KeyframeAttributes;
class WindowInformation;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QPopupMenu;


class KFListView;
class KFListViewItem;
class KFTimeSlider;
class Plot;
class PlotList;

// ****************************************************************************
//  Class:  QvisKeyframeWindow
//
//  Purpose:
//    Keyframing animation editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 10 13:34:27 PST 2002
//    Added api.
//
//    Jeremy Meredith, Fri Jan 31 16:06:04 PST 2003
//    Added database state keyframing methods.
//
//    Jeremy Meredith, Tue Feb  4 17:47:43 PST 2003
//    Added the view keyframing item.
//
//    Brad Whitlock, Fri Jan 23 17:35:00 PST 2004
//    I made it observe WindowInformation since that's where the view
//    keyframes are now.
//
//    Brad Whitlock, Wed Apr 7 00:15:44 PDT 2004
//    I added an internal slot function.
//
//    Brad Whitlock, Wed Apr  9 11:09:07 PDT 2008
//    QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisKeyframeWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
  public:
    QvisKeyframeWindow(KeyframeAttributes *subj,
                       const QString &caption = QString::null,
                       const QString &shortName = QString::null,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisKeyframeWindow();
    virtual void CreateWindowContents();
    virtual void ConnectAttributes(AttributeSubject *subj, const char *name=NULL);
    virtual void ConnectPlotAttributes(AttributeSubject *subj, int index);
    virtual void ConnectWindowInformation(WindowInformation *subj);
    virtual void ConnectPlotList(PlotList *subj);
    virtual void UpdateWindowInformation();
    virtual void UpdatePlotList();
    void SubjectRemoved(Subject*);

  public slots:
    virtual void apply();
    void timeChanged(int);
    void newSelection();
    void stateKFClicked();

  protected:
    void UpdateWindow(bool doAll);
    void UpdateWindowSensitivity();
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    KFListViewItem *AddSubjectToWindow(AttributeSubject*,
                                       const char* =NULL,
                                       KFListViewItem* =NULL);
    KFListViewItem *AddPlotToWindow(AttributeSubject*, const char*,
                                    KFListViewItem*);
    int GetCurrentFrame() const;

  private slots:
    void nFramesProcessText();
    void keyframeEnabledToggled(bool);
    void userSetNFrames(const QString &);
  private:
    QLineEdit *nFrames;
    QCheckBox *snapToFrameCheck;
    QCheckBox *keyframeEnabledCheck;
    QLineEdit *dbStateLineEdit;
    QPushButton *dbStateButton;

    KFListView   *lv;
    KFTimeSlider *ts;

    WindowInformation *windowInfo;
    PlotList *plotList;
    std::map<int, KFListViewItem*> plotMap;
    std::map<int, AttributeSubject*> plotAtts;
    KFListViewItem *viewItem;

    KeyframeAttributes *kfAtts;

    std::vector<AttributeSubject*> atts;
    std::map<AttributeSubject*, std::string> nameMap;
    std::map<AttributeSubject*, KFListViewItem*> itemMap;
    std::vector<KFListViewItem*> fileItems;
};



#endif
