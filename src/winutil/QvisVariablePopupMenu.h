#ifndef QVIS_VARIABLE_POPUP_MENU_H
#define QVIS_VARIABLE_POPUP_MENU_H
#include <winutil_exports.h>
#include <qpopupmenu.h>

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
// ****************************************************************************

class WINUTIL_API QvisVariablePopupMenu : public QPopupMenu
{
    Q_OBJECT
public:
    QvisVariablePopupMenu(int plotType_, QWidget *parent, const char *name);
    virtual ~QvisVariablePopupMenu();
    int getPlotType() const { return plotType; };
    void setPlotType(int p) { plotType = p; };
    void setVarPath(const QString &str) { varPath = str; };
signals:
    void activated(int plotType_, const QString &varName);
private slots:
    void activatedCaught(int index);
private:
    QString varPath;
    int     plotType;
};

#endif
