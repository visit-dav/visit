#ifndef QVIS_SCREEN_POSITIONER_H
#define QVIS_SCREEN_POSITIONER_H
#include <gui_exports.h>
#include <qframe.h>

// ****************************************************************************
// Class: QvisScreenPositioner
//
// Purpose:
//   This widget allows the user to position a crosshairs that specifies a
//   screen position for annotations or other objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:10:17 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisScreenPositioner : public QFrame
{
    Q_OBJECT
public:
    QvisScreenPositioner(QWidget *parent = 0, const char *name = 0, WFlags = 0);
    virtual ~QvisScreenPositioner();
    virtual QSize sizeHint () const;
    virtual QSize minimumSize() const;

    void setScreenSize(int xs, int ys);
    int  screenWidth() const       { return xScreenSize; };
    int  screenHeight() const      { return yScreenSize; };
    void setScreenPosition(int xp, int yp);
    void setScreenPosition(float xp, float yp);
    int  screenX() const           { return xPosition; }
    int  screenY() const           { return yPosition; }
    int  pageIncrement() const     { return pageIncrement_; }
    void setPageIncrement(int val) { pageIncrement_ = val; }    
signals:
    void screenPositionChanged(int newX, int newY);
    void screenPositionChanged(float newX, float newY);
    void intermediateScreenPositionChanged(int newX, int newY);
    void intermediateScreenPositionChanged(float newX, float newY);
public slots:
    void popupShow();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void drawContents(QPainter *);

    void sendNewScreenPosition();
    void setTempPositionFromWidgetCoords(int wx, int wy);

    static const int minXScreenSize;
    static const int minYScreenSize;

    int xPosition;
    int yPosition;
    int xTempPosition;
    int yTempPosition;
    int xScreenSize;
    int yScreenSize;    
    int pageIncrement_;
    bool dragging;
    bool paging;
};

#endif
