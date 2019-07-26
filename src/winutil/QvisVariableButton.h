// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VARIABLE_BUTTON_H
#define QVIS_VARIABLE_BUTTON_H
#include <QPushButton>
#include <string>
#include <winutil_exports.h>

#ifdef DESIGNER_PLUGIN
class QMenu;
#else
class QvisVariableButtonHelper;
class QvisVariablePopupMenu;
class VariableMenuPopulator;
#endif

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
//   Brad Whitlock, Fri Apr 1 16:23:43 PST 2005
//   Added Label var support.
//
//   Hank Childs, Tue Jul 19 14:39:43 PDT 2005
//   Added Array var support.
//
//   Brad Whitlock, Tue Sep 26 15:39:40 PST 2006
//   Added another constructor and some properties so they can be exposed
//   in Qt designer.
//
//   Brad Whitlock, Thu Dec 20 12:42:30 PST 2007
//   Added methods to delete menu items.
//
//   Brad Whitlock, Fri May  9 12:04:07 PDT 2008
//   Qt 4.
//
//   Kathleen Bonnell, Tue Jun 24 11:18:13 PDT 2008 
//   Move setVarTypes implementation to C file. 
//
//   Brad Whitlock, Fri Jul 18 10:19:03 PDT 2008
//   Made activeSourceInfo and plotSourceInfo get allocated from the heap
//   so their destructors are called before the Qt library has completed
//   deleting its objects on exit. This prevents a crash with Qt 4.
//
//   Brad Whitlock, Thu Oct 16 14:50:21 PDT 2008
//   Added support for a stripped down version that can stand in for the 
//   real thing in designer.
//
//   Cyrus Harrison, Fri May 21 11:35:34 PDT 2010
//   Added onDefaultVar & onCreateExpr slots to reenable actions for the
//   'default var' & 'create new expr' top level menu items.
//
//   Hank Childs, Mon Aug  2 11:21:49 PDT 2010
//   Refactor class so that the menu can be explicitly set.
//
//   Mark C. Miller, Tue May  8 18:36:57 PDT 2018
//   Adjust default values for destroyMenuItems in methods where it is used.
// ****************************************************************************

class WINUTIL_API QvisBaseVariableButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int varTypes READ getVarTypes WRITE setVarTypes )
    Q_PROPERTY(QString variable READ getVariable WRITE setVariable )
    Q_PROPERTY(QString defaultVariable READ getDefaultVariable WRITE setDefaultVariable )
    Q_PROPERTY(bool addExpr READ getAddExpr WRITE setAddExpr )
    Q_PROPERTY(bool addDefault READ getAddDefault WRITE setAddDefault )
public:
    QvisBaseVariableButton(QWidget *parent);
    QvisBaseVariableButton(bool addDefault_, bool addExpr_, 
        int mask, QWidget *parent);
    virtual ~QvisBaseVariableButton();

    virtual void setText(const QString &);

    virtual void setDefaultVariable(const QString &);
    QString getDefaultVariable() const { return defaultVariable; }
    int  getVarTypes() const { return varTypes; }
    void setVarTypes(int t);

    bool getAddExpr() const;
    void setAddExpr(bool);

    bool getAddDefault() const;
    void setAddDefault(bool);

    const QString &getVariable() const;
    void setVariable(const QString &t);

    void setChangeTextOnVariableChange(bool);

#ifndef DESIGNER_PLUGIN
    //
    // Static methods that are used in connection with all variable menus.
    //
    static void ConnectExpressionCreation(QObject *, const char *);
#endif

    // Needed to deal with some inheritance issues.  Internal use only.
    virtual bool               DowncastToNormalButton(void) { return false; };

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
    static const int Labels;
    static const int Arrays;
signals:
    void activated(const QString &varName);
public slots:
    void changeVariable(int, const QString &);
private slots:
    void connectMenu();
    void disconnectMenu();
    void deferredDisconnectMenu();
    void onDefaultVar();
    void onCreateExpr();

protected:
    void UpdateMenu();
    void InitializeCategoryNames();
#ifdef DESIGNER_PLUGIN
    QMenu                    *menu;
#else
    struct VariablePopupInfo
    {
        VariablePopupInfo();
        virtual ~VariablePopupInfo();

        void Initialize();
        void UpdateMenus(VariableMenuPopulator *pop, bool destroyMenuItems=true);
        void CreateMenu(int);
        void DeleteMenu(int);
        void connect(QvisBaseVariableButton *);
        void disconnect();

        QvisVariableButtonHelper  *helper;
        QvisVariablePopupMenu    **varMenus;
    };

    static QList<QObject*>    instances;
    static QObject           *expressionCreator;
    static const char        *expressionSlot;

    virtual QvisBaseVariableButton::VariablePopupInfo *GetSourceInfo() = 0;

    QvisVariablePopupMenu    *menu;
#endif
    static QStringList       *categoryMenuNames;

    bool                      addDefault;
    bool                      addExpr;
    bool                      changeTextOnVarChange;
    int                       varTypes;
    QString                   variable;
    QString                   defaultVariable;
};

class WINUTIL_API QvisVariableButton : public QvisBaseVariableButton
{
  public:
    QvisVariableButton(QWidget *parent);
    QvisVariableButton(bool addDefault_, bool addExpr_, bool usePlot,
        int mask, QWidget *parent);
    virtual ~QvisVariableButton();

#ifndef DESIGNER_PLUGIN
    //
    // Static methods that are used in connection with all variable menus.
    //
    static void UpdateActiveSourceButtons(VariableMenuPopulator *pop,
        bool destroyMenuItems=true);
    static void UpdatePlotSourceButtons(VariableMenuPopulator *pop,
        bool destroyMenyItems=true);
#endif

  protected:
    bool                      usePlotSource;

    virtual bool              DowncastToNormalButton(void) { return true; };

#ifndef DESIGNER_PLUGIN
    static VariablePopupInfo  *activeSourceInfo;
    static VariablePopupInfo  *plotSourceInfo;
    virtual QvisBaseVariableButton::VariablePopupInfo *GetSourceInfo()
                 {return (usePlotSource ? plotSourceInfo : activeSourceInfo);};
#endif
};


class WINUTIL_API QvisCustomSourceVariableButton : public QvisBaseVariableButton
{
  public:
    QvisCustomSourceVariableButton(QWidget *parent);
    QvisCustomSourceVariableButton(bool addDefault_, bool addExpr_, 
                                   VariableMenuPopulator *pop, int mask, 
                                   QWidget *parent);
    virtual ~QvisCustomSourceVariableButton();

    void ResetPopulator(VariableMenuPopulator *);

  protected:
    VariablePopupInfo        *customSourceInfo;

    virtual QvisBaseVariableButton::VariablePopupInfo *GetSourceInfo()
                            { return customSourceInfo; };
};

#endif


