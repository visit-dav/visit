#ifndef QVIS_MESSAGE_WINDOW_H
#define QVIS_MESSAGE_WINDOW_H
#include <gui_exports.h>
#include <QvisWindowBase.h>
#include <Observer.h>

class QLabel;
class QMultiLineEdit;

// *******************************************************************
// Class: QvisMessageWindow
//
// Purpose:
//   This window observes a MessageAttributes state object and prints
//   the message in the state object into the window when it is
//   updated.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:09:22 PST 2000
//
// Modifications:
//   
// *******************************************************************

class GUI_API QvisMessageWindow : public QvisWindowBase, public Observer
{
    Q_OBJECT
public:
    QvisMessageWindow(MessageAttributes *msgAttr,
                      const char *captionString = 0);
    virtual ~QvisMessageWindow();
    virtual void Update(Subject *);
private:
    QLabel         *severityLabel;
    QMultiLineEdit *messageText;
};

#endif
