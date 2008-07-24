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

#ifndef QVIS_FILE_PANEL
#define QVIS_FILE_PANEL
#include <gui_exports.h>
#include <qwidget.h>
#include <SimpleObserver.h>
#include <GUIBase.h>
#include <QualifiedFilename.h>
#include <TimeFormat.h>
#include <map>

// Forward declarations.
class QComboBox;
class QLabel;
class QListView;
class QListViewItem;
class QPushButton;
class QLineEdit;
class QPixmap;
class QvisAnimationSlider;
class QvisListViewFileItem;
class QvisVCRControl;

class avtDatabaseMetaData;
class FileServerList;
class WindowInformation;
class ViewerProxy;

// ****************************************************************************
// Class: QvisFilePanel
//
// Purpose:
//   This class contains all the objects and logic for the top panel
//   of the VisIt main window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 24 14:47:44 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 25 16:51:59 PST 2000
//   Made it inherit from GUIBase too.
//
//   Brad Whitlock, Wed Mar 21 14:58:59 PST 2001
//   Added a new pixmap.
//
//   Brad Whitlock, Tue Aug 21 16:54:23 PST 2001
//   Added UpdateTimeFieldText method.
//
//   Brad Whitlock, Thu Feb 28 13:00:18 PST 2002
//   Added infrastructure for keeping track of expanded files.
//
//   Brad Whitlock, Wed Mar 6 16:17:52 PST 2002
//   Made OpenFile return a bool.
//
//   Brad Whitlock, Mon Aug 19 13:36:19 PST 2002
//   I added a private slot function to update the width of the list view.
//
//   Brad Whitlock, Fri Feb 28 08:47:04 PDT 2003
//   I added a bool argument to OpenFile.
//
//   Brad Whitlock, Thu May 15 09:51:24 PDT 2003
//   I added ExpandDatabaseItem and changed some method prototypes.
//
//   Brad Whitlock, Wed May 21 15:51:32 PST 2003
//   Added method definitions for FileDisplayInformation since it didn't
//   work the way I had it on SGI.
//
//   Brad Whitlock, Wed Jul 30 16:48:59 PST 2003
//   Added reopenOnNextFrame signal.
//
//   Brad Whitlock, Mon Oct 13 15:30:20 PST 2003
//   Changed so it supports displaying times as well as cycles.
//
//   Brad Whitlock, Wed Oct 15 15:18:43 PST 2003
//   Added an optional timeState argument to ReplaceFile.
//
//   Brad Whitlock, Fri Oct 24 14:40:58 PST 2003
//   Added an internal AnimationSetFrame method.
//
//   Brad Whitlock, Fri Dec 19 17:00:07 PST 2003
//   I added a slot to help me update the file list in case we encounter
//   virtual files that have been added but have multiple time states
//   in each file. I also added some internal convenience methods.
//
//   Brad Whitlock, Tue Dec 30 14:33:25 PST 2003
//   I made it use QvisAnimationSlider.
//
//   Brad Whitlock, Sat Jan 24 23:48:13 PST 2004
//   I made it observe WindowInformation instead of GlobalAttributes
//   because I rewrote both of those state objects to better support time
//   and file selection and now much of the needed information resides in
//   WindowAttributes. I also made it possible to not display the selected
//   files list.
//
//   Brad Whitlock, Tue Apr 6 14:06:25 PST 2004
//   I added allowFileSelectionChange.
//
//   Brad Whitlock, Fri Aug 6 12:19:22 PDT 2004
//   I added updateHeaderForLongName.
//
//   Brad Whitlock, Mon Dec 20 12:18:36 PDT 2004
//   I added UpdateReplaceButtonEnabledState
//
//   Brad Whitlock, Mon Jun 27 14:54:27 PST 2005
//   Added updateOpenButtonState
//
//   Brad Whitlock, Thu Jul 24 09:17:05 PDT 2008
//   Made it possible to overlay a file at a given state.
//
// ****************************************************************************

class GUI_API QvisFilePanel : public QWidget, public SimpleObserver, public GUIBase
{
    Q_OBJECT

    struct FileDisplayInformation
    {
        FileDisplayInformation()
        {
            expanded = true; correctData = false;
        }
        FileDisplayInformation(const FileDisplayInformation &obj)
        {
            expanded = obj.expanded;
            correctData = obj.correctData;
        }
        ~FileDisplayInformation() { }
        void operator = (const FileDisplayInformation &obj)
        {
            expanded = obj.expanded;
            correctData = obj.correctData;
        }

        bool expanded;
        bool correctData;
    };

    typedef std::map<std::string, FileDisplayInformation>
            FileDisplayInformationMap;

public:
    QvisFilePanel(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisFilePanel();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void ConnectFileServer(FileServerList *);
    void ConnectWindowInformation(WindowInformation *);

    bool HaveFileInformation(const QualifiedFilename &filename) const;
    void AddExpandedFile(const QualifiedFilename &filename);
    void SetTimeStateFormat(const TimeFormat &fmt);
    const TimeFormat &GetTimeStateFormat() const;

    bool GetShowSelectedFiles() const;
    void SetShowSelectedFiles(bool);
    bool GetAllowFileSelectionChange() const;
    void SetAllowFileSelectionChange(bool);

    void UpdateOpenButtonState();
signals:
    void reopenOnNextFrame();
private:
    void UpdateFileList(bool doAll);
    void RepopulateFileList();
    void UpdateAnimationControls(bool doAll);
    void UpdateFileSelection();
    void UpdateTimeFieldText(int timeState);
    void UpdateAnimationControlsEnabledState();
    bool UpdateReplaceButtonEnabledState();
    void UpdateOpenButtonState(QvisListViewFileItem *fileItem);

    bool OpenFile(const QualifiedFilename &filename, int timeState,
                  bool reOpen);
    void ReplaceFile(const QualifiedFilename &filename, int timeState=0);
    void OverlayFile(const QualifiedFilename &filename, int timeState=0);

    void ExpandDatabases();
    void ExpandDatabaseItem(QvisListViewFileItem *item);
    void ExpandDatabaseItemUsingMetaData(QvisListViewFileItem *item);
    void ExpandDatabaseItemUsingVirtualDBDefinition(QvisListViewFileItem *item);
    void RemoveExpandedFile(const QualifiedFilename &filename);
    void SetFileExpanded(const QualifiedFilename &filename, bool);
    bool FileIsExpanded(const QualifiedFilename &filename) const;
    bool FileShowsCorrectData(const QualifiedFilename &filename);
    void SetFileShowsCorrectData(const QualifiedFilename &filename, bool);
    bool HighlightedItemIsInvalid() const;

    QString CreateItemLabel(const avtDatabaseMetaData *md, int ts, bool);
    QString FormattedCycleString(const int cycle) const;
    QString FormattedTimeString(const double d, bool accurate) const;
    bool DisplayVirtualDBInformation(const QualifiedFilename &file) const;

    void SetTimeSliderState(int);
private slots:
    void changeActiveTimeSlider(int);
    void backwardStep();
    void reversePlay();
    void stop();
    void play();
    void forwardStep();
    void sliderStart();
    void sliderMove(int val);
    void sliderEnd();
    void sliderChange(int val);
    void processTimeText();

    void fileCollapsed(QListViewItem *);
    void fileExpanded(QListViewItem *);
    void highlightFile(QListViewItem *);
    void openFile();
    void openFileDblClick(QListViewItem *);
    void replaceFile();
    void overlayFile();
    void updateHeaderWidth();
    void updateHeaderWidthForLongName();
    void internalUpdateFileList();
private:
    bool                     showSelectedFiles;

    QListView                *fileListView;
    QComboBox                *activeTimeSlider;
    QLabel                   *activeTimeSliderLabel;
    QPushButton              *openButton;
    QPushButton              *replaceButton;
    QPushButton              *overlayButton;
    QvisAnimationSlider      *animationPosition;
    QLineEdit                *timeField;
    QvisVCRControl           *vcrControls;
    QPixmap                  *computerPixmap;
    QPixmap                  *databasePixmap;
    QPixmap                  *folderPixmap;

    WindowInformation        *windowInfo;

    bool                      allowFileSelectionChange;
    int                       sliderVal;
    FileDisplayInformationMap displayInfo;
    TimeFormat                timeStateFormat;
};

#endif
