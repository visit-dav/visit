#ifndef QVIS_COLOR_BUTTON_H
#define QVIS_COLOR_BUTTON_H
#include <gui_exports.h>
#include <qcolor.h>
#include <qbutton.h>
#include <vector>

class QPainter;
class QPopupMenu;
class QvisColorSelectionWidget;

// ****************************************************************************
// Class: QvisColorButton
//
// Purpose:
//   This class represents a color button widget that can be used to select
//   colors for materials, isocontours, and other items in the gui.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 09:49:14 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisColorButton : public QButton
{
    Q_OBJECT

    typedef std::vector<QvisColorButton *> ColorButtonVector;
public:
    QvisColorButton(QWidget *parent = 0, const char *name = 0,
                    const void *userData = 0);
    virtual ~QvisColorButton();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy () const;

    void setButtonColor(const QColor &c);
    const QColor &buttonColor() const;

    void setUserData(const void *data);
    const void *getUserData() const;
signals:
    void selectedColor(const QColor &c);
    void selectedColor(const QColor &c, const void *data);
protected:
    virtual void  drawButton(QPainter *);
    virtual void  drawButtonLabel(QPainter *);
private slots:
    void popupPressed();
    void colorSelected(const QColor &c);
private:
    QColor                           color;
    const void                      *userData;

    static QvisColorSelectionWidget *popup;
    static ColorButtonVector         buttons;
};

#endif
