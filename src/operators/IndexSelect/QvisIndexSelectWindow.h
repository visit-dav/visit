// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISINDEXSELECTWINDOW_H
#define QVISINDEXSELECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IndexSelectAttributes;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QSpinBox;
class QvisSILSetSelector;

// ****************************************************************************
// Class: QvisIndexSelectWindow
//
// Purpose: 
//   Defines QvisIndexSelectWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004
//   Changed Min/Max/Incr from LineEdit to SpinBox for usability, added
//   labels and group boxes for each dim.
//   
//   Kathleen Bonnell, Thu Jun  7 11:40:58 PDT 2007 
//   Added QvisSILSetSelector, removed domainIndex, groupIndex.
//   Changed whichData buttongroup to useWholeCollection checkbox.
// 
//   Cyrus Harrison, Tue Aug 19 16:23:19 PDT 2008
//   Qt4 Port. 
//
// ****************************************************************************

class QvisIndexSelectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIndexSelectWindow(const int type,
                         IndexSelectAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIndexSelectWindow();
  protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dimChanged(int val);
    void oneDMinChanged(int min);
    void oneDMaxChanged(int max = -1);
    void oneDIncrChanged(int incr);
    void oneDWrapToggled(bool val);
    void twoDMinChanged(int min);
    void twoDMaxChanged(int max = -1);
    void twoDIncrChanged(int incr);
    void twoDWrapToggled(bool val);
    void threeDMinChanged(int min);
    void threeDMaxChanged(int max = -1);
    void threeDIncrChanged(int incr);
    void threeDWrapToggled(bool val);
    void useWholeCollectionToggled(bool val);
    void categoryChanged(const QString &);
    void subsetChanged(const QString &);
  private:
    QButtonGroup *dim;

    QWidget      *oneDWidget;
    QLabel       *oneDLabel;
    QSpinBox     *oneDMin;
    QSpinBox     *oneDMax;
    QSpinBox     *oneDIncr;
    QCheckBox    *oneDWrap;

    QWidget      *twoDWidget;
    QLabel       *twoDLabel;
    QSpinBox     *twoDMin;
    QSpinBox     *twoDMax;
    QSpinBox     *twoDIncr;
    QCheckBox    *twoDWrap;

    QWidget      *threeDWidget;
    QLabel       *threeDLabel;
    QSpinBox     *threeDMin;
    QSpinBox     *threeDMax;
    QSpinBox     *threeDIncr;
    QCheckBox    *threeDWrap;

    QCheckBox *useWholeCollection;

    QvisSILSetSelector *silSet;
    IndexSelectAttributes *atts;
};

#endif
