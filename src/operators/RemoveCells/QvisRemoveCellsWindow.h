#ifndef QVISREMOVECELLSWINDOW_H
#define QVISREMOVECELLSWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class RemoveCellsAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QListBox;
class QListBoxItem;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisRemoveCellsWindow
//
// Purpose: 
//   Defines QvisRemoveCellsWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisRemoveCellsWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisRemoveCellsWindow(const int type,
                         RemoveCellsAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisRemoveCellsWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void cellProcessText();
    void domainProcessText();
    void addButtonClicked();
    void removeButtonClicked();
    void changeButtonClicked();
    void listSelected(int index);
  private:
    QLineEdit *cell;
    QLineEdit *domain;

    QListBox  *cellList;

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *changeButton;

    RemoveCellsAttributes *atts;

    bool buildingList;
};



#endif
