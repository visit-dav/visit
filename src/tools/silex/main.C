#include <qapplication.h>
#include <Explorer.h>

#include <iostream.h>
#include <stdlib.h>

#include <qwindowsstyle.h>
#include <qfiledialog.h>
#include <qstring.h>

// ****************************************************************************
//  Main Function: main()  
//
//  Purpose:
//    initialize and start the main window
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
int main( int argc, char **argv )
{
    QApplication::setColorSpec(QApplication::ManyColor);
    QApplication a(argc, argv);
    a.setStyle(new QWindowsStyle);
    Explorer *w;
    if (argc > 1)
        w = new Explorer(argv[1], NULL, "Explorer");
    else
    {
        QString file =
            QFileDialog::getOpenFileName(QString(),
                                         "Silo files (*.silo *.root *.pdb);;"
                                         "All files (*)",
                                         NULL, "SiloOpen", "Open file...");
        if (file.isNull())
            return 0;
        w = new Explorer(file, NULL, "Explorer");
    }

    a.setMainWidget(w);
    w->show();

    return a.exec();
}
