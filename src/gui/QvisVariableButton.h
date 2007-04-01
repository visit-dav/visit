#ifndef QVIS_VARIABLE_BUTTON_H
#define QVIS_VARIABLE_BUTTON_H
#include <qpushbutton.h>
#include <qobjectlist.h>
#include <string>
#include <gui_exports.h>

class QvisVariableButtonHelper;
class QvisVariablePopupMenu;
class VariableMenuPopulator;

// ****************************************************************************
// Class: QvisVariableButton
//
// Purpose:
//   This button pops up a variable menu based on the active source or the
//   selected plot's database.
//
// Notes:      This class uses 2 static variable menus that are shared for
//   all instances of the class. There is one menu for variables from the
//   active source and another menu for variables from the current plot
//   source. Menus are shared across instances of this class to reduce the
//   number of menus that have to be updated when the variable lists change.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 3 10:20:56 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisVariableButton : public QPushButton
{
    Q_OBJECT
public:
    QvisVariableButton(bool addDefault_, bool addExpr_, bool usePlot,
        int mask, QWidget *parent, const char *name = 0);
    virtual ~QvisVariableButton();

    virtual void setText(const QString &);
    virtual void setDefaultVariable(const QString &);

    void setChangeTextOnVariableChange(bool);

    //
    // Static methods that are used in connection with all variable menus.
    //
    static void UpdateActiveSourceButtons(VariableMenuPopulator *pop);
    static void UpdatePlotSourceButtons(VariableMenuPopulator *pop);
    static void ConnectExpressionCreation(QObject *, const char *);

    //
    // Const values for which menus to show. Or them together to get
    // multiple variable menus in the button.
    //
    static const int Scalars;
    static const int Vectors;
    static const int Meshes;
    static const int Materials;
    static const int Subsets;
    static const int Species;
    static const int Curves;
    static const int Tensors;
    static const int SymmetricTensors;
signals:
    void activated(const QString &varName);
public slots:
    void changeVariable(int, const QString &);
private slots:
    void connectMenu();
    void disconnectMenu();
    void deferredDisconnectMenu();
private:
    void UpdateMenu();

    struct VariablePopupInfo
    {
        VariablePopupInfo();
        virtual ~VariablePopupInfo();

        void Initialize();
        void UpdateMenus(VariableMenuPopulator *pop);
        void connect(QvisVariableButton *);
        void disconnect();

        QvisVariableButtonHelper  *helper;
        QvisVariablePopupMenu    **varMenus;
    };

    static QObjectList        instances;
    static VariablePopupInfo  activeSourceInfo;
    static VariablePopupInfo  plotSourceInfo;
    static QObject           *expressionCreator;
    static const char        *expressionSlot;

    bool                      addDefault;
    bool                      addExpr;
    bool                      usePlotSource;
    bool                      changeTextOnVarChange;
    int                       varTypes;
    QString                   defaultVariable;
    QvisVariablePopupMenu    *menu;
};

#endif
