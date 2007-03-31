#ifndef QVIS_TURNDOWN_BUTTON_H
#define QVIS_TURNDOWN_BUTTON_H
#include <gui_exports.h>
#include <qpushbutton.h>

// ****************************************************************************
// Class: QvisTurnDownButton
//
// Purpose:
//   This class creates a turn down button.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:59:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisTurnDownButton : public QPushButton
{
    Q_OBJECT
public:
    QvisTurnDownButton(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisTurnDownButton();
    virtual QSize sizeHint() const { return QSize(15,15); }
protected:
    virtual void drawButtonLabel(QPainter *);
};

#endif
