#ifndef QVIS_EXPRESSIONS_WINDOW_H
#define QVIS_EXPRESSIONS_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class ExpressionList;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QMultiLineEdit;
class QLabel;
class QListBox;
class QComboBox;

// ****************************************************************************
// Class: QvisExpressionsWindow
//
// Purpose:
//   This class contains the widgets that manipulate expressions
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//    Jeremy Meredith, Mon Oct 25 11:32:14 PDT 2004
//    Reversed the sense of the "hidden" button.  Added a list-box-index to
//    expresion-list-index map so we didn't have to index expressions by name.
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
    void    BlockAllSignals(bool);
  private slots:
    void    addExpression();
    void    delExpression();
    void    nameTextChanged(const QString&);
    void    definitionTextChanged();
    void    typeChanged(int);
    void    notHiddenChanged();
    void    displayAllVarsChanged();
    void    insertFunction(int);

    void    UpdateWindowSingleItem();
    void    UpdateWindowSensitivity();

  private:
    // Widgets and layouts.
    QListBox *exprListBox;

    QLabel *nameEditLabel;
    QLabel *definitionEditLabel;
    QLabel *typeLabel;

    QLineEdit *nameEdit;
    QComboBox *typeList;
    QCheckBox *notHidden;
    QMultiLineEdit *definitionEdit;

    QPushButton *newButton;
    QPushButton *delButton;

    QPushButton *insertFunctionButton;
    QPopupMenu *insertFunctionMenu;

    QCheckBox *displayAllVars;

    // State information
    ExpressionList *exprList;
    std::map<int,int> indexMap;
};

#endif
