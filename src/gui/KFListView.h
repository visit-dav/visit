#ifndef KF_LIST_VIEW_H
#define KF_LIST_VIEW_H

#include <GUIBase.h>
#include <qlistview.h>
class KFListViewItem;
class QPopupMenu;

// ****************************************************************************
//  Class:  KFListView
//
//  Purpose:
//    The main listview for the keyframe editor.  (the big grid)
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:37:17 PST 2003
//    Added some infrastructure necessary for keyframing.
//
// ****************************************************************************
class KFListView : public QListView, public GUIBase
{
    Q_OBJECT
  public:
    KFListView(QWidget *p, const char *n);
    double kfstep();
    double kfwidth();
    double kfstart();
    virtual void paintEmptyArea(QPainter *p, const QRect &rect);
    double x2time(int x);
    double x2i(int x);
    int time2i(double t);
    double i2time(int i);
    int i2x(int i);
    int GetCurrentIndex();
    bool GetSnap();
    int GetNFrames();
    void SetNFrames(int);
    void SelectNewItem(int x, int y, bool hint, bool del);
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
  public slots:
    void timeChanged(int);
    void snapToFrameToggled(bool);
  private:
    int nframes;
    bool snap;
    KFListViewItem *selecteditem;
    int     currentIndex;
    QPopupMenu *interpMenu;
    bool mousedown;
};


#endif
