#ifndef QVISONIONPEELWINDOW_H
#define QVISONIONPEELWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class OnionPeelAttributes;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class SILRestrictionAttributes;

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
// ****************************************************************************

class QvisOnionPeelWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisOnionPeelWindow(const int type,
                         OnionPeelAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisOnionPeelWindow();
  protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void adjacencyTypeChanged(int val);
    void categoryNameChanged();
    void subsetNameChanged();
    void indexChanged();
    void useGlobalIdToggled(bool val);
    void requestedLayerChanged(int val);
    void delayedApply();
  private:
    void UpdateComboBoxes();
    void UpdateComboBoxesEnabledState(int which);
    void FillCategoryBox();
    void FillSubsetBox();

    QButtonGroup *adjacencyType;
    QLabel    *categoryLabel;
    QComboBox *categoryName;
    QLabel    *subsetLabel;
    QComboBox *subsetName;
    QLineEdit *index;
    QSpinBox  *requestedLayer;
    QCheckBox *useGlobalId;

    OnionPeelAttributes *atts;
    SILRestrictionAttributes *silAtts;
    QString defaultItem;
    
    int silTopSet;
    int silNumSets;
    int silNumCollections;
    unsignedCharVector silUseSet;
};



#endif
