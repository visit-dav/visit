#ifndef QVIS_FILE_PANEL
#define QVIS_FILE_PANEL
#include <gui_exports.h>
#include <qwidget.h>
#include <SimpleObserver.h>
#include <GUIBase.h>
#include <QualifiedFilename.h>
#include <map>

// Forward declarations.
class QListView;
class QListViewItem;
class QPushButton;
class QSlider;
class QLineEdit;
class QPixmap;
class QvisListViewFileItem;
class QvisVCRControl;

class FileServerList;
class GlobalAttributes;
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
// ****************************************************************************

class GUI_API QvisFilePanel : public QWidget, public SimpleObserver, public GUIBase
{
    Q_OBJECT

    struct FileDisplayInformation
    {
        FileDisplayInformation()
        {
            expanded = true; correctCycles = false;
        }
        FileDisplayInformation(const FileDisplayInformation &obj)
        {
            expanded = obj.expanded;
            correctCycles = obj.correctCycles;
        }
        ~FileDisplayInformation() { }
        void operator = (const FileDisplayInformation &obj)
        {
            expanded = obj.expanded;
            correctCycles = obj.correctCycles;
        }

        bool expanded;
        bool correctCycles;
    };

    typedef std::map<QualifiedFilename, FileDisplayInformation>
            FileDisplayInformationMap;

public:
    QvisFilePanel(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisFilePanel();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void ConnectFileServer(FileServerList *);
    void ConnectGlobalAttributes(GlobalAttributes *);

    bool HaveFileInformation(const QualifiedFilename &filename) const;
    void AddExpandedFile(const QualifiedFilename &filename);
signals:
    void reopenOnNextFrame();
private:
    void UpdateFileList(bool doAll);
    void UpdateAnimationControls(bool doAll);
    void UpdateFileSelection();
    void UpdateTimeFieldText(int timeState);
    bool OpenFile(const QualifiedFilename &filename, int timeState,
                  bool reOpen);
    void ReplaceFile(const QualifiedFilename &filename);
    void OverlayFile(const QualifiedFilename &filename);
    void ExpandDatabases();
    void ExpandDatabaseItem(QvisListViewFileItem *item);
    void RemoveExpandedFile(const QualifiedFilename &filename);
    void SetFileExpanded(const QualifiedFilename &filename, bool);
    bool FileIsExpanded(const QualifiedFilename &filename);
    bool FileShowsCorrectCycles(const QualifiedFilename &filename);
    void SetFileShowsCorrectCycles(const QualifiedFilename &filename, bool);
private slots:
    void prevFrame();
    void reversePlay();
    void stop();
    void play();
    void nextFrame();
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
private:
    QListView        *fileListView;
    QPushButton      *openButton;
    QPushButton      *replaceButton;
    QPushButton      *overlayButton;
    QSlider          *animationPosition;
    QLineEdit        *timeField;
    QvisVCRControl   *vcrControls;
    QPixmap          *computerPixmap;
    QPixmap          *databasePixmap;
    QPixmap          *folderPixmap;

    GlobalAttributes *globalAtts;

    bool                      sliderDown;
    int                       sliderVal;
    FileDisplayInformationMap displayInfo;
};

#endif
