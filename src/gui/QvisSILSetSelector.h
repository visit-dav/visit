// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SIL_SET_SELECTOR_H
#define QVIS_SIL_SET_SELECTOR_H

#include <gui_exports.h>
#include <QWidget>
#include <SimpleObserver.h>
#include <GUIBase.h>
#include <vectortypes.h>

class QComboBox;
class QCompleter;
class QStringListModel;
class QLabel;
class SILRestrictionAttributes;

// ****************************************************************************
// Class: QvisSILSetSelector
//
// Purpose:
//   Defines QvisSILSetSelector class.
//
// Programmer: Kathleen Bonnell
// Creation:   June 6, 2007
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:18:47 PDT 2007
//   Added userCategory, userSubset, so that options stored in sesisonfiles
//   can be restored.
//
//   Brad Whitlock, Fri Jul 18 08:35:26 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Monday Oct 21, 2024
//   Added QCompleter and QStringListModel for use with subsetName ComboBox.
//
// ****************************************************************************

class GUI_API QvisSILSetSelector : public QWidget,
                                   public SimpleObserver,
                                   public GUIBase
{
    Q_OBJECT
  public:
    QvisSILSetSelector(QWidget *parent,
            SILRestrictionAttributes *, intVector &);
    virtual ~QvisSILSetSelector();

    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void SetCategoryName(const QString &name);
    QString GetCategoryName() const;
    void SetSubsetName(const QString &name);
    QString GetSubsetName() const;


  signals:
    void categoryChanged(const QString &);
    void subsetChanged(const QString &);

  private slots:
    void categoryNameChanged();
    void subsetNameChanged();

  private:
    void UpdateComboBoxes();
    void FillCategoryBox();
    void FillSubsetBox();

    QLabel    *categoryLabel;
    QComboBox *categoryName;
    QLabel    *subsetLabel;
    QComboBox *subsetName;
    QCompleter *subsetNameCompleter;
    QStringListModel *subsetNameModel;

    SILRestrictionAttributes *silAtts;
    QString defaultItem;
    QString lastGoodCategory;
    QString lastGoodSubset;
    QString userCategory;
    QString userSubset;
    int silTopSet;
    int silNumSets;
    int silNumCollections;
    unsignedCharVector silUseSet;
    intVector allowedCategories;
};


#endif
