/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <Python.h>
#include <string.h>
#include <VisItException.h>

// For the VisIt module.
extern "C" void cli_initvisit(int, bool, int, char **, int, char **);
extern "C" void cli_runscript(const char *);
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
//   Brad Whitlock, Tue Jul 15 12:59:22 PDT 2003
//   I moved the code to run a script into the visit module.
//
//   Brad Whitlock, Mon Jul 28 16:51:22 PST 2003
//   Added code to prevent -sessionfile, -format, -framestep, and -output
//   from being passed on to the viewer.
//
//   Hank Childs, Thu Apr  1 08:57:11 PST 2004
//   Prevent -fps from being passed as well.
//
//   Brad Whitlock, Wed May 4 08:34:04 PDT 2005
//   I changed the code so argv[0] gets passed to the cli in argv2. I also
//   made Python initialize its threading.
//
//   Dave Bremer, Wed Mar 14 17:58:15 PDT 2007
//   Pass commandline arguments through to the main interpreter.
//
//   Jeremy Meredith, Wed Jun  6 16:41:45 EDT 2007
//   Add the -forceinteractivecli argument (equivalent to -i in python).
//
//   Brad Whitlock, Fri Jun 8 10:57:02 PDT 2007
//   Added support for saving the arguments after -s to their own tuple.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int  retval = 0;
    int  debugLevel = 0;
    bool verbose = false, s_found = false;
    char *runFile = 0, *loadFile = 0;
    char **argv2 = new char *[argc];
    char **argv_after_s = new char *[argc];
    int  argc2 = 0, argc_after_s = 0;

    // Parse the arguments
    for(int i = 0; i < argc; ++i)
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

            s_found = true;
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
        else if(strcmp(argv[i], "-output") == 0)
        {
            // Skip the argument that comes along with -output. Note that
            // the -output argument will be accessible in sys.argv but it
            // is not passed along in the args that are sent to the viewer.
            ++i;
        }
        else if(strcmp(argv[i], "-forceinteractivecli") == 0)
        {
            // Force the python interpreter to behave interactively
            // even if there is no TTY.  This lets clients hook
            // into visit's cli using pipes, and is the equivalent
            // of the "-i" flag in python.
            Py_InteractiveFlag++;
        }
        else if(strcmp(argv[i], "-format") == 0)
        {
            // Skip the argument that comes along with -format.
            ++i;
        }
        else if(strcmp(argv[i], "-framestep") == 0)
        {
            // Skip the argument that comes along with -framestep.
            ++i;
        }
        else if(strcmp(argv[i], "-sessionfile") == 0)
        {
            // Skip the argument that comes along with -sessionfile.
            ++i;
        }
        else if(strcmp(argv[i], "-fps") == 0)
        {
            // Skip the rate that comes along with -fps.
            ++i;
        }
        else
        {
            // Pass the array along to the visitmodule.
            argv2[argc2++] = argv[i];

            // This argument is after -s file.py so count it unless it's
            // the -dv argument.
            if(s_found && 
               strcmp(argv[i], "-dv") != 0)
            {
                argv_after_s[argc_after_s++] = argv[i];
            }
        }
    }

    TRY
    {
        // Initialize python
        Py_Initialize();
        PyEval_InitThreads();
        Py_SetProgramName(argv[0]);
        PySys_SetArgv(argc, argv);

        // Initialize the VisIt module.
        cli_initvisit(debugLevel, verbose, argc2, argv2,
                      argc_after_s, argv_after_s);

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
        cli_runscript(runFile);

        // Enter the python interpreter loop.
        //int argc3 = 1;
        //char *argv3 = argv[0];
        //retval = Py_Main(argc3, &argv3);
        
        char **argv3 = new char*[argc+1];
        int ii;
        for (ii = 1 ; ii < argc ; ii++)
        {
            argv3[ii+1] = argv[ii];
        }
        argv3[0] = argv[0];
        argv3[1] = "-";
        retval = Py_Main(argc+1, argv3);
        delete[] argv3;
        
    }
    CATCHALL(...)
    {
        retval = -1;
    }
    ENDTRY

    // Delete the argv2 array.
    delete [] argv2;
    delete [] argv_after_s;

    return retval;
}
