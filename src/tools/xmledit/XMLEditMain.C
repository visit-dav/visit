#include <qapplication.h>
#include <XMLEdit.h>

#include <visitstream.h>
#include <stdlib.h>

#include <qfiledialog.h>
#include <qstring.h>

#ifndef __APPLE__
#include <qwindowsstyle.h>
#endif

// ****************************************************************************
//  Main Function: main()  
//
//  Purpose:
//    initialize and start the main window
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Aug 15 15:28:27 PST 2003
//    I prevented the style from being set on MacOS X so it looks like a
//    native Mac application.
//
// ****************************************************************************

int
main( int argc, char **argv )
{
    QApplication::setColorSpec(QApplication::ManyColor);
    QApplication *a = new QApplication(argc, argv);
#ifndef __APPLE__
    a->setStyle(new QWindowsStyle);
#endif
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
