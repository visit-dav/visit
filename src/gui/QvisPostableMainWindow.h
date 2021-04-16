// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_POSTABLE_MAIN_WINDOW_H
#define QVIS_POSTABLE_MAIN_WINDOW_H
#include <QvisPostableWindow.h>

// ****************************************************************************
// Class: QvisPostableMainWindow
//
// Purpose:
//   This is a small window that we post the Main window's controls into when
//   we run on small screens.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 23 17:03:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class QvisPostableMainWindow : public QvisPostableWindow
{
public:
    QvisPostableMainWindow(const QString &captionString = QString(),
                           const QString &shortName = QString(),
                           QvisNotepadArea *n = 0);
    virtual ~QvisPostableMainWindow();

    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);

    QWidget *ContentsWidget();
    QVBoxLayout *ContentsLayout();
private:
    QWidget     *contentsWidget;
    QVBoxLayout *contentsLayout;
};

#endif
