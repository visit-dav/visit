#ifndef QVIS_POSTABLE_WINDOW
#define QVIS_POSTABLE_WINDOW
#include <gui_exports.h>
#include <qstring.h>
#include <QvisWindowBase.h>

class DataNode;
class QvisNotepadArea;
class QPushButton;
class QVBoxLayout;

// ****************************************************************************
// Class: QvisPostableWindow
//
// Purpose:
//   This is a base class for postable windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:28:28 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:40:41 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Wed May 2 11:09:42 PDT 2001
//   Removed interpreter member.
//
//   Brad Whitlock, Tue Sep 25 12:28:53 PDT 2001
//   Made post() and unpost() virtual.
//
//   Brad Whitlock, Fri Feb 15 11:16:41 PDT 2002
//   Made CreateEntireWindow public.
//
// ****************************************************************************

class GUI_API QvisPostableWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisPostableWindow(const char *captionString = 0,
                       const char *shortName = 0,
                       QvisNotepadArea *n = 0);
    virtual ~QvisPostableWindow();
    virtual void CreateWindowContents() = 0;
    QWidget *GetCentralWidget();
    const QString &GetShortCaption();
    bool posted();
    virtual void CreateEntireWindow();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
protected:
    virtual void UpdateWindow(bool doAll);
public slots:
    virtual void raise();
    virtual void show();
    virtual void hide();
protected slots:
    virtual void post();
    virtual void unpost();
protected:
    bool               isCreated;
    bool               isPosted;
    QString            shortCaption;
    QWidget            *central;
    QVBoxLayout        *topLayout;
    QPushButton        *postButton;
    QvisNotepadArea    *notepad;
};

#endif
