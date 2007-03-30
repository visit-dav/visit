#ifndef QVIS_FILE_INFORMATION_WINDOW_H
#define QVIS_FILE_INFORMATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class FileServerList;
class QMultiLineEdit;

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
//   
// ****************************************************************************

class GUI_API QvisFileInformationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisFileInformationWindow(FileServerList *fs, const char *caption = 0,
        const char *shortName = 0, QvisNotepadArea *notepad = 0);
    virtual ~QvisFileInformationWindow();
    virtual void CreateWindowContents();
protected:
    virtual void UpdateWindow(bool doAll);
private:
    static const char *titleSeparator;
    QMultiLineEdit    *outputText;
};

#endif
