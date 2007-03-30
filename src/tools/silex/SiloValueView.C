#include "SiloValueView.h"
#include <SiloFile.h>
#include <qlabel.h>
#include <iostream.h>

// ----------------------------------------------------------------------------
//                            Value View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor:  SiloValueViewWindow::SiloValueViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloValueViewWindow::SiloValueViewWindow(SiloFile *s, const QString &n, QWidget *p)
    : QMainWindow(p, n), silo(s), name(n)
{
    setCaption(QString("Value: ")+name);

    QLabel *l = new QLabel(this, "ValueList");
    setCentralWidget(l);

    void *var = silo->GetVar(name);
    if (!var)
    {
        cerr << "SiloValueViewWindow::SiloValueViewWindow -- not an array var\n";
        throw;
    }

    int type = silo->GetVarType(name);
    int len  = silo->GetVarLength(name);
    char str[4096];
    switch (type)
    {
      case DB_INT:
        sprintf(str, "int: %d", *((int*)var));
        
        break;
      case DB_SHORT:
        sprintf(str, "short: %d", *((short*)var));
        break;
      case DB_LONG:
        sprintf(str, "long: %d", *((long*)var));
        break;
      case DB_FLOAT:
        sprintf(str, "float: %g", *((float*)var));
        break;
      case DB_DOUBLE:
        sprintf(str, "double: %g", *((double*)var));
        break;
      case DB_CHAR:
        if (len == 1)
            sprintf(str, "char: %c", *((char*)var));
        else
            sprintf(str, "string: %s", ((char*)var));
        break;
      case DB_NOTYPE:
        sprintf(str, "NOTYPE: ???");
        break;
      default:
        sprintf(str, "???: ???");
        break;
    }
    l->setText(str);
}

