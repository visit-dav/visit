#include "Scanner.h"
#include "ExprParser.h"
#include "ParseException.h"

// ****************************************************************************
//  Function:  main
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
int
main(int argc, char *argv[])
{
    if (argc<2) {cerr<<"needs an argument\n"; exit(-1);}

    ExprParser parser;

    for (int i=1; i<argc; i++)
    {
        cout << "\n----\n";
        cout << "PARSING '"<<argv[i]<<"'"<<endl;
        cout << "----\n\n";

        ExprNode *node = parser.Parse(argv[i]);
        if (node)
            node->Print(cout);
        else
            cout << "ERROR\n";
    }

    return 0;
}
