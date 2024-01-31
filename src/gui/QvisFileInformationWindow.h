// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILE_INFORMATION_WINDOW_H
#define QVIS_FILE_INFORMATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class FileServerList;
class QTextEdit;

// ****************************************************************************
// Class: QvisFileInformationWindow
//
// Purpose:
//   Observes the file server and updates its contents when the open file
//   changes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:51:40 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:46:33 PDT 2008
//   QString for caption and shortName.
//
// ****************************************************************************

class GUI_API QvisFileInformationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFileInformationWindow(FileServerList *fs, const QString &caption = QString(),
        const QString &shortName = QString(), QvisNotepadArea *notepad = 0);
    virtual ~QvisFileInformationWindow();
    virtual void CreateWindowContents();
protected:
    virtual void UpdateWindow(bool doAll);
private:
    static const char *titleSeparator;
    QTextEdit    *outputText;
};

#endif
