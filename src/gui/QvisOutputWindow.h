#ifndef QVIS_OUTPUT_WINDOW_H
#define QVIS_OUTPUT_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// Forward declarations.
class QString;
class QMultiLineEdit;
class MessageAttributes;

// ****************************************************************************
// Class: QvisOutputWindow
//
// Purpose:
//   This class describes an output window. An output window is
//   basically a dialog box that has a large, scrollable, multi-line
//   edit text field and a dismiss button. It is used to display
//   big messages or the contents of a file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:25:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 12:09:37 PDT 2001
//   Made it inherit from QvisPostableWindowObserver.
//
// ****************************************************************************

class GUI_API QvisOutputWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisOutputWindow(MessageAttributes *subj,
                     const char *caption = 0,
                     const char *shortName = 0,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisOutputWindow();
    virtual void CreateWindowContents();
signals:
    void unreadOutput(bool);
protected:
    virtual void UpdateWindow(bool doAll);
private:
    QMultiLineEdit *outputText;
};

#endif
