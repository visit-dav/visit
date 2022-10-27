// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>
#include <qapplication.h>
#include <XMLEdit.h>

#include <iostream>
#include <stdlib.h>

#include <QFileDialog>

#include <QString>
#include <QTextStream>

using std::cerr;
using std::endl;

QTextStream cOut(stdout), cErr(stderr);
QString Endl("\n");


// ****************************************************************************
//  Main Function: XMLEditMain()
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

int
XMLEditMain( int argc, char **argv )
{
    QApplication *a = new QApplication(argc, argv);
    XMLEdit *w;
    if (argc > 1)
        w = new XMLEdit(argv[1], NULL);
    else
    {
        w = new XMLEdit("untitled.xml", NULL);
    }

    w->show();

    try
    {
        return a->exec();
    }
    catch (const char *s)
    {
        cerr << "ERROR: " << s << endl;
    }
    catch (const QString &s)
    {
        cerr << "ERROR: " << s.toStdString() << endl;
    }
    return -1;
}

// ****************************************************************************
// Method: main/WinMain
//
// Purpose:
//   The program entry point function.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 23 13:15:31 PST 2011
//
// Modifications:
//
// ****************************************************************************

#if defined(_WIN32) && defined(VISIT_WINDOWS_APPLICATION)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI
WinMain(HINSTANCE hInstance,     // handle to the current instance
        HINSTANCE hPrevInstance, // handle to the previous instance
        LPSTR lpCmdLine,         // pointer to the command line
        int nCmdShow             // show state of window
)
{
    return XMLEditMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return XMLEditMain(argc, argv);
}
#endif
