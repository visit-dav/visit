#include <ctype.h>
#include <stdio.h>
#include <Python.h>
#include <string.h>
#include <VisItException.h>

// For the VisIt module.
extern "C" void cli_initvisit(int, bool, int, char **);
extern "C" int Py_Main(int, char **);

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the cli program.
//
// Notes:      Most of the program is in visitmodule.C which is also built as
//             a shared library.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 7 15:34:11 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 11 10:47:55 PDT 2001
//   I fixed the -debug, -s, -nowin arguments.
//
//   Brad Whitlock, Fri May 10 10:23:47 PDT 2002
//   I added the -o argument for opening databases.
//
//   Brad Whitlock, Mon Dec 16 13:43:20 PST 2002
//   I added the -verbose argument.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int  retval = 0;
    int  debugLevel = 0;
    bool verbose = false;
    char *runFile = 0, *loadFile = 0;
    char **argv2 = new char *[argc];
    int  argc2 = 0;

    // Parse the arguments
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-debug") == 0)
        {
            debugLevel = 1;
            if (i+1 < argc && isdigit(*(argv[i+1])))
               debugLevel = atoi(argv[++i]);
            else
               fprintf(stderr,"Warning: debug level not specified, assuming 1\n");
            if (debugLevel < 0)
            {
                debugLevel = 0;
                fprintf(stderr,"Warning: clamping debug level to 0\n");
            }
            if (debugLevel > 5)
            {
                debugLevel = 5;
                fprintf(stderr,"Warning: clamping debug level to 5\n");
            }
        }
        else if(strcmp(argv[i], "-s") == 0 && (i+1 < argc))
        {
            runFile = argv[i+1];
            ++i;
        }
        else if(strcmp(argv[i], "-o") == 0 && (i+1 < argc))
        {
            loadFile = argv[i+1];
            ++i;
        }
        else if(strcmp(argv[i], "-verbose") == 0)
        {
            verbose = true;
        }
        else
        {
            // Pass the array along to the visitmodule.
            argv2[argc2++] = argv[i];
        }
    }

    TRY
    {
        // Initialize python
        Py_Initialize();
        Py_SetProgramName(argv[0]);
        PySys_SetArgv(argc, argv);

        // Initialize the VisIt module.
        cli_initvisit(debugLevel, verbose, argc2, argv2);

        // Run some Python commands that import VisIt and launch the viewer.
        PyRun_SimpleString((char*)"from visit import *");
        PyRun_SimpleString((char*)"Launch()");

        // If a database was specified, load it.
        if(loadFile != 0)
        {
             char *command = new char[strlen(loadFile) + 1 + 16];
             sprintf(command, "OpenDatabase(\"%s\")", loadFile);
             PyRun_SimpleString(command);
             delete [] command;
        }

        // If there was a file to execute, do it.
        if(runFile != 0)
        {
            FILE *fp = fopen(runFile, "r");
            if(fp)
            {
                PyRun_SimpleFile(fp, runFile);
                fclose(fp);
            }
            else
            {
                fprintf(stderr, "The file %s could not be opened.\n", runFile);
            }
        }

        // Enter the python interpreter loop.
        int argc3 = 1;
        char *argv3 = argv[0];
        retval = Py_Main(argc3, &argv3);
    }
    CATCHALL(...)
    {
        retval = -1;
    }
    ENDTRY

    // Delete the argv2 array.
    delete [] argv2;

    return retval;
}
