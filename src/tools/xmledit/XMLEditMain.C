#include <qapplication.h>
#include <XMLEdit.h>

#include <iostream.h>
#include <stdlib.h>

#include <qfiledialog.h>
#include <qstring.h>
#include <qwindowsstyle.h>

// ****************************************************************************
//  Main Function: main()  
//
//  Purpose:
//    initialize and start the main window
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2002
//
// ****************************************************************************
int main( int argc, char **argv )
{
    QApplication::setColorSpec(QApplication::ManyColor);
    QApplication *a = new QApplication(argc, argv);
    a->setStyle(new QWindowsStyle);
    XMLEdit *w;
    if (argc > 1)
        w = new XMLEdit(argv[1], NULL, "XML Editor");
    else
    {
        w = new XMLEdit("untitled.xml", NULL, "XML Editor");
    }

    a->setMainWidget(w);
    w->show();

    try
    {
        return a->exec();
    }
    catch (const char *s)
    {
        cerr << "ERROR: " << s << endl;
        exit(-1);
    }
    catch (const QString &s)
    {
        cerr << "ERROR: " << s << endl;
        exit(-1);
    }
}
