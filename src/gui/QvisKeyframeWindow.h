// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISKEYFRAMEWINDOW_H
#define QVISKEYFRAMEWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <AttributeSubject.h>

class KeyframeAttributes;
class PlotList;
class WindowInformation;

class QCheckBox;
class QLineEdit;
class QTreeView;

class KeyframeDataModel;

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
//    Brad Whitlock, Wed Apr 7 00:15:44 PDT 2004
//    I added an internal slot function.
//
//    Brad Whitlock, Wed Apr  9 11:09:07 PDT 2008
//    QString for caption, shortName.
//
//    Brad Whitlock, Thu Oct 23 15:02:53 PDT 2008
//    Rewrote for Qt 4.
//
// ****************************************************************************

class GUI_API QvisKeyframeWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisKeyframeWindow(KeyframeAttributes *subj,
                       const QString &caption = QString(),
                       const QString &shortName = QString(),
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisKeyframeWindow();

    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject*);

    void ConnectWindowInformation(WindowInformation *subj);
    void ConnectPlotList(PlotList *subj);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void UpdateWindowInformation();
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

    int GetCurrentFrame() const;

private slots:
    void nFramesProcessText();
    void keyframeEnabledToggled(bool);
    void userSetNFrames(const QString &);
    void newSelection();
    void stateKFClicked();
    void addViewKeyframe();
    void useViewKFClicked(bool);
private:
    QLineEdit   *nFrames;
    QCheckBox   *keyframeEnabledCheck;
    QLineEdit   *dbStateLineEdit;
    QPushButton *dbStateButton;
    QPushButton *viewButton;
    QTreeView   *kv;
    QCheckBox   *useViewKeyframes;

    WindowInformation  *windowInfo;
    PlotList           *plotList;
    KeyframeAttributes *kfAtts;
    KeyframeDataModel  *model;
};



#endif
