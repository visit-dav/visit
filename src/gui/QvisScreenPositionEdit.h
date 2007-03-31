#ifndef QVIS_SCREEN_POSITION_EDIT_H
#define QVIS_SCREEN_POSITION_EDIT_H
#include <gui_exports.h>
#include <qlineedit.h>

class QTimer;
class QvisScreenPositioner;
class QvisTurnDownButton;

// ****************************************************************************
// Class: QvisScreenPositionEdit
//
// Purpose:
//   Special purpose line edit widget that provides a popup to position the
//   coordinates.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:17:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisScreenPositionEdit : public QWidget
{
    Q_OBJECT
public:
    QvisScreenPositionEdit(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisScreenPositionEdit();

    void setPosition(float, float);
    bool getPosition(float &, float &);
signals:
    void screenPositionChanged(float, float);
protected slots:
    void closePopup();
    void newScreenPosition(float x, float y);
    void popup();
    void updateText(float, float);
    void returnPressed();
protected:
    bool getCurrentValues(float *, float *);

    float                screenX, screenY;
    QLineEdit            *lineEdit;
    QvisTurnDownButton   *turnDown;
    QvisScreenPositioner *screenPositionPopup;
    QTimer               *popupTimer;
};

#endif
