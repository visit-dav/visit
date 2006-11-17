/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

class GUI_API QvisVariableButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int varTypes READ getVarTypes WRITE setVarTypes )
    Q_PROPERTY(QString variable READ getVariable WRITE setVariable )
    Q_PROPERTY(QString defaultVariable READ getDefaultVariable WRITE setDefaultVariable )
public:
    QvisVariableButton(QWidget *parent, const char *name = 0);
    QvisVariableButton(bool addDefault_, bool addExpr_, bool usePlot,
        int mask, QWidget *parent, const char *name = 0);
    virtual ~QvisVariableButton();

    virtual void setText(const QString &);

    virtual void setDefaultVariable(const QString &);
    QString getDefaultVariable() const { return defaultVariable; }
    int  getVarTypes() const { return varTypes; }
    void setVarTypes(int t) { varTypes = t; }

    const QString &getVariable() const;
    void setVariable(const QString &t);

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
    QString                   variable;
    QString                   defaultVariable;
    QvisVariablePopupMenu    *menu;
};

#endif
