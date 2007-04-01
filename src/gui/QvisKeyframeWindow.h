#ifndef QVISKEYFRAMEWINDOW_H
#define QVISKEYFRAMEWINDOW_H

#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <AttributeSubject.h>
#include <vector>
#include <map>

class KeyframeAttributes;
class WindowInformation;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QPopupMenu;


class KFListView;
class KFListViewItem;
class KFTimeSlider;
class Plot;
class PlotList;

// ****************************************************************************
//  Class:  QvisKeyframeWindow
//
//  Purpose:
//    Keyframing animation editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 10 13:34:27 PST 2002
//    Added api.
//
//    Jeremy Meredith, Fri Jan 31 16:06:04 PST 2003
//    Added database state keyframing methods.
//
//    Jeremy Meredith, Tue Feb  4 17:47:43 PST 2003
//    Added the view keyframing item.
//
//    Brad Whitlock, Fri Jan 23 17:35:00 PST 2004
//    I made it observe WindowInformation since that's where the view
//    keyframes are now.
//
// ****************************************************************************

class GUI_API QvisKeyframeWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
  public:
    QvisKeyframeWindow(KeyframeAttributes *subj,
                       const char *caption = 0,
                       const char *shortName = 0,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisKeyframeWindow();
    virtual void CreateWindowContents();
    virtual void ConnectAttributes(AttributeSubject *subj, const char *name=NULL);
    virtual void ConnectPlotAttributes(AttributeSubject *subj, int index);
    virtual void ConnectWindowInformation(WindowInformation *subj);
    virtual void ConnectPlotList(PlotList *subj);
    virtual void UpdateWindowInformation();
    virtual void UpdatePlotList();
    void SubjectRemoved(Subject*);

  public slots:
    virtual void apply();
    void timeChanged(int);
    void newSelection();
    void stateKFClicked();

  protected:
    void UpdateWindow(bool doAll);
    void UpdateWindowSensitivity();
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    KFListViewItem *AddSubjectToWindow(AttributeSubject*,
                                       const char* =NULL,
                                       KFListViewItem* =NULL);
    KFListViewItem *AddPlotToWindow(AttributeSubject*, const char*,
                                    KFListViewItem*);
    int GetCurrentFrame() const;

  private slots:
    void nFramesProcessText();
    void keyframeEnabledToggled(bool);

  private:
    QLineEdit *nFrames;
    QCheckBox *snapToFrameCheck;
    QCheckBox *keyframeEnabledCheck;
    QLineEdit *dbStateLineEdit;
    QPushButton *dbStateButton;

    KFListView   *lv;
    KFTimeSlider *ts;

    WindowInformation *windowInfo;
    PlotList *plotList;
    std::map<int, KFListViewItem*> plotMap;
    std::map<int, AttributeSubject*> plotAtts;
    KFListViewItem *viewItem;

    KeyframeAttributes *kfAtts;

    std::vector<AttributeSubject*> atts;
    std::map<AttributeSubject*, std::string> nameMap;
    std::map<AttributeSubject*, KFListViewItem*> itemMap;
    std::vector<KFListViewItem*> fileItems;
};



#endif
