#ifndef QVIS_EXPRESSIONS_WINDOW_H
#define QVIS_EXPRESSIONS_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class ExpressionList;
class QPoint;
class QVBoxLayout;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QListView;
class ExpressionTable;

// ****************************************************************************
// Class: QvisExpressionsWindow
//
// Purpose:
//   This class contains the widgets that manipulate expressions
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:59:13 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisExpressionsWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:

    QvisExpressionsWindow(ExpressionList * exprAtts_,
                          const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea * notepad = 0);
    virtual ~ QvisExpressionsWindow();
    virtual void CreateWindowContents();
  public  slots:
    virtual void apply();
  protected:
    void    UpdateWindow(bool doAll);
    void    Apply(bool forceUpdate = false);
  private slots:
    void    addExpression();
    void    deleteExpression(int);
  private:
    ExpressionList *exprList;

    // Widgets and layouts.
    QGroupBox *listGroup;
    QButton *newButton;
    QButton *editButton;
    QButton *deleteButton;
    ExpressionTable *table;
    QGroupBox *editGroup;
    QLineEdit *nameLineEdit;
    QLineEdit *defLineEdit;
    QCheckBox *hiddenToggle;

    // State information
};

#endif
