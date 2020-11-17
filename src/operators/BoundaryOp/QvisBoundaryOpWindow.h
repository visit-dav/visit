// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISBOUNDARYWINDOW_H
#define QVISBOUNDARYWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class BoundaryOpAttributes;
class QButtonGroup;

// ****************************************************************************
// Class: QvisBoundaryOpWindow
//
// Purpose: 
//   Defines QvisBoundaryOpWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class QvisBoundaryOpWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisBoundaryOpWindow(const int type,
                         BoundaryOpAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisBoundaryOpWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void smoothingLevelChanged(int index);

  private:
    QButtonGroup *smoothingLevelButtons;

    BoundaryOpAttributes *atts;
};



#endif
