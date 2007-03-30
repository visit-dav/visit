#ifndef QVIS_CHECK_LIST_MANAGER_WIDGET_H
#define QVIS_CHECK_LIST_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

class QButtonGroup;
class QCheckBox;
class QGrid;
class QScrollView;

// ****************************************************************************
// Class: QvisCheckListManagerWidget
//
// Purpose:
//   This class contains a list of checkboxes and puts them in a scrolled
//   window so they can be arranged easily.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:21:14 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisCheckListManagerWidget : public QWidget
{
    Q_OBJECT
public:
    QvisCheckListManagerWidget(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisCheckListManagerWidget();

    void addCheckBox(const QString &name, bool checked);
    void removeLastEntry();
    void setCheckBox(int index, const QString &name, bool checked);

    int numCheckBoxes() const;
signals:
    void checked(int index, bool on);
private slots:
    void emitChecked(int index);
private:
    QScrollView    *scrollView;
    QGrid          *grid;
    QButtonGroup   *checkBoxGroup;

    int            nCheckBoxes;
    QCheckBox      **checkBoxes;
};

#endif
