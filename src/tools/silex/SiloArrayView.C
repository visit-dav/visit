#include "SiloArrayView.h"
#include <SiloFile.h>
#include <qlistbox.h>
#include <iostream.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
//                            Array View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloArrayViewWindow::SiloArrayViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloArrayViewWindow::SiloArrayViewWindow(SiloFile *s, const QString &n, QWidget *p)
    : QMainWindow(p, n), silo(s), name(n)
{
    setCaption(QString("Array: ")+name);

    lb = new QListBox(this, "ArrayList");
    setCentralWidget(lb);

    void *var = silo->GetVar(name);
    if (!var)
    {
        cerr << "SiloArrayViewWindow::SiloArrayViewWindow -- not an array var\n";
        throw;
    }

    int type = silo->GetVarType(name);
    int len  = silo->GetVarLength(name);

    if (type != DB_CHAR)
    {
        for (int i=0; i<len; i++)
        {
            char str[256];
            switch (type)
            {
              case DB_INT:
                sprintf(str, "%-4d: %d", i, ((int*)var)[i]);
                break;
              case DB_SHORT:
                sprintf(str, "%-4d: %d", i, ((short*)var)[i]);
                break;
              case DB_LONG:
                sprintf(str, "%-4d: %ld", i, ((long*)var)[i]);
                break;
              case DB_FLOAT:
                sprintf(str, "%-4d: %g", i, ((float*)var)[i]);
                break;
              case DB_DOUBLE:
                sprintf(str, "%-4d: %g", i, ((double*)var)[i]);
                break;
              case DB_CHAR:
                sprintf(str, "%-4d: %c", i, ((char*)var)[i]);
                break;
              case DB_NOTYPE:
                sprintf(str, "%-4d: NOTYPE", i);
                break;
              default:
                sprintf(str, "%-4d: type ???", i);
                break;
            }
            lb->insertItem(str);
        }
    }
    else
    {
        char str[1024];
        char *p = str;
        for (int i=0; i<len; i++)
        {
            char c = ((char*)var)[i];
            if (c == ';')
            {
                *p = '\0';
                if (strlen(str) > 0)
                    lb->insertItem(str);
                p = str;
            }
            else
                *(p++)=c;
        }
        *p = '\0';
        if (strlen(str) > 0)
            lb->insertItem(str);
    }
}

// ****************************************************************************
//  Method:  SiloArrayViewWindow::sizeHint
//
//  Purpose:
//    Suggest a good size for the view
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
QSize
SiloArrayViewWindow::sizeHint() const
{
    QSize size = QMainWindow::sizeHint();
    if (lb->count() == 0)
        return size;

    size.setHeight(QMIN(QMAX(size.height(),
                             lb->itemHeight() * (lb->count()+2)),
                        QApplication::desktop()->height() * 7/8));
    if (!size.isValid())
        size.setWidth(150);
    else
        size.setWidth(QMAX(150, size.width()));

    return size;
}
