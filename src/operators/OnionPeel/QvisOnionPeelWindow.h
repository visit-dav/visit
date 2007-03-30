#ifndef QVISONIONPEELWINDOW_H
#define QVISONIONPEELWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class OnionPeelAttributes;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QButtonGroup;

// ****************************************************************************
// Class: QvisOnionPeelWindow
//
// Purpose: 
//   Defines QvisOnionPeelWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
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
    void requestedLayerChanged(int val);
    void delayedApply();
  private:
    void UpdateComboBoxes();
    void FillCategoryBox();
    void FillSubsetBox();

    QButtonGroup *adjacencyType;
    QComboBox *categoryName;
    QComboBox *subsetName;
    QLineEdit *index;
    QSpinBox *requestedLayer;

    OnionPeelAttributes *atts;
    QString defaultItem;
    int silTopSet;
    int silNumSets;
    int silNumCollections;
};



#endif
