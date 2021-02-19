// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISONIONPEELWINDOW_H
#define QVISONIONPEELWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class OnionPeelAttributes;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QvisSILSetSelector;

// ****************************************************************************
// Class: QvisOnionPeelWindow
//
// Purpose: 
//   Defines QvisOnionPeelWindow class.
//
// Notes:      This class was automatically generated!
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Added silUseSet and silAtts.
//   
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004 
//   Added useGlobalId checkbox, related methods and method
//   UpdateComboBoxesEnabledState. 
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group and 'seedTypeChanged' slot.
//
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced widgets/slots/var associated with Category/Set with
//   QvisSILSetSelector and its slots.
//
//   Kathleen Biagas, Tue Jul 22 19:32:57 MST 2014
//   Added honorOriginalMesh comboBox.
//
// ****************************************************************************

class QvisOnionPeelWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisOnionPeelWindow(const int type,
                        OnionPeelAttributes *subj,
                        const QString &caption = QString(),
                        const QString &shortName = QString(),
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisOnionPeelWindow();
  protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void adjacencyTypeChanged(int val);
    void seedTypeChanged(int val);
    void categoryChanged(const QString &);
    void subsetChanged(const QString &);
    void indexChanged();
    void useGlobalIdToggled(bool val);
    void requestedLayerChanged(int val);
    void honorOriginalMeshChanged(int);
    void delayedApply();
  private:
    QButtonGroup *adjacencyType;
    QButtonGroup *seedType;

    QvisSILSetSelector *silSet;
    QLineEdit *index;
    QSpinBox  *requestedLayer;
    QCheckBox *useGlobalId;
    QComboBox *honorOriginalMesh;

    OnionPeelAttributes *atts;
};



#endif
