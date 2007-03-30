#ifndef QVIS_COMMAND_LINE_WINDOW_H
#define QVIS_COMMAND_LINE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindow.h>

class QLineEdit;

// ****************************************************************************
// Class: QvisCommandLineWindow
//
// Purpose:
//   Command line interface window in Qt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:38:14 PST 2000
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisCommandLineWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisCommandLineWindow(const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea *notepad = 0);
    ~QvisCommandLineWindow();
public slots:
    void processText();
protected:
    virtual void CreateWindowContents();
private:
    QLineEdit *commandLineEdit;
};

#endif
