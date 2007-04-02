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

    void setPosition(double, double);
    bool getPosition(double &, double &);
signals:
    void screenPositionChanged(double, double);
protected slots:
    void closePopup();
    void newScreenPosition(double x, double y);
    void popup();
    void updateText(double, double);
    void returnPressed();
protected:
    bool getCurrentValues(double *, double *);

    double                screenX, screenY;
    QLineEdit            *lineEdit;
    QvisTurnDownButton   *turnDown;
    QvisScreenPositioner *screenPositionPopup;
    QTimer               *popupTimer;
};

#endif
