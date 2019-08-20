// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VARIABLE_POPUP_MENU_H
#define QVIS_VARIABLE_POPUP_MENU_H
#include <winutil_exports.h>

#include <QAction>
#include <QActionGroup>
#include <QMenu>

// ****************************************************************************
// Class: QvisVariablePopupMenu
//
// Purpose:
//   This is a popup menu that knows what kind of plot it is for. This allows
//   us to use fewer signals, etc. and support plugins inside the plot manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 17:57:49 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 10:29:38 PDT 2002
//   Added a methods to return/set the plot type.
//
//   Brad Whitlock, Mon Mar 17 15:12:48 PST 2003
//   I changed the API.
//
//   Brad Whitlock, Fri May  9 10:29:44 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class WINUTIL_API QvisVariablePopupMenu : public QMenu
{
    Q_OBJECT
public:
    QvisVariablePopupMenu(int plotType_, QWidget *parent);
    virtual ~QvisVariablePopupMenu();
    void setPlotType(int p) { plotType = p; };
    int getPlotType() const { return plotType; };
    void setVarPath(const QString &str) { varPath = str; };
    QString getVarPath() const { return varPath; };
    unsigned int getHashVal() const { return myHashVal; };
    void setHashVal(unsigned int hv) { myHashVal = hv; };

    QAction *addVar(const QString &, bool valid = true);
    int count() const;
signals:
    void activated(int plotType_, const QString &varName);
private slots:
    void caughtTriggered(QAction *);
private:
    QActionGroup *actions;
    QString       varPath;
    int           plotType;
    unsigned int  myHashVal;
};

#endif
