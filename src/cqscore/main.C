#include <qapplication.h>
#include <qstring.h>

#include <visitstream.h>

#include "CQScore.h"

// ****************************************************************************
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2005
// ****************************************************************************
int main( int argc, char **argv )
{
    QApplication::setColorSpec(QApplication::ManyColor);
    QApplication *a = new QApplication(argc, argv);

    CQScore *w;
    w = new CQScore(NULL, "CQScore");

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
