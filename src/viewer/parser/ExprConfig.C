#include "ExprGrammar.h"
#include <fstream.h>

// ****************************************************************************
//  Method:  ExprGrammar::Initialize
//
//  Purpose:
//    Dummy function so we don't get link errors.  This should not be called
//    when only configuring the grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ExprGrammar::Initialize()
{
    cerr << "Should never be called in configuration program.\n";
    return false;
}

// ****************************************************************************
//  Function:  main
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
int main(int argc, char *argv[])
{
    if (argc < 2)
        cerr << "Expecting output file name as argument.  Testing only.\n";

    Grammar *G = new ExprGrammar;

    G->SetPrinter(&cout);

    if (!G->Configure())
    {
        cerr << "--------------------------\n";
        cerr << "  Error in configuration  \n";
        cerr << "--------------------------\n";
        cerr << endl;
        return -1;
    }

    if (argc >= 2)
    {
        ofstream output(argv[1], ios::out);
        if (!output)
        {
            cerr << "---------------------------\n";
            cerr << "    Error creating file   \n";
            cerr << "---------------------------\n";
            cerr << endl;
            exit(-1);
        }
        cerr << "Writing to file: " << argv[1] << endl;
        G->WriteStateInitialization("ExprGrammar", output);
        output.close();
    }

    return 0;
}
