// ************************************************************************* //
//                                    Init.C                                 //
// ************************************************************************* //

#include <Init.h>

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <process.h>
#include <winsock2.h>
#else
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#endif
#include <new.h>

#include <DebugStream.h>
#include <TimingsManager.h>


static void RemovePrependedDirs(const char *, char *); 
static char executableName[256];
static char componentName[256];
static ErrorFunction errorFunction = NULL;
static void *errorFunctionArgs = NULL;


// ****************************************************************************
//  Function: striparg
//
//  Purpose:
//      Given an argc and an argv, strip off the arguments from i to i+n.
//
//  Arguments:
//      i           The first argument to strip      (i)
//      n           The number of arguments to strip (i)
//      argc        The number of total arguments    (io)
//      argv        The array of arguments           (io)
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  1 16:05:05 PST 2002
//    Fixed a bug where it was copying beyond the end of the array.
//
// ****************************************************************************
static void
striparg(int i, int n, int &argc, char *argv[])
{
    for ( ; i+n<argc; i++)
        argv[i] = argv[i+n];

    argc -= n;
}

// ****************************************************************************
//  Function: NewHandler
//
//  Purpose:
//      Called when the component runs out of memory.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 20 13:07:51 PST 2001
//    Added an abort.
//
// ****************************************************************************
static void
NewHandler(void)
{
    debug1 << "This component has run out of memory." << endl;
    abort();
}

// ****************************************************************************
//  Function: Init::Initialize
//
//  Purpose:
//      Parse any common commandline arguments (e.g. "-debug N") and do
//      common initialization.
//
//  Arguments:
//      argc        The number of total arguments                       (io)
//      argv        The array of arguments                              (io)
//      r           The rank of this process                            (i)
//      n           The total number of processes                       (i)
//      strip       Whether or not to strip the argument from the list. (i)
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Nov 27 16:25:07 PST 2000
//    I added a new flag that indicates whether or not to strip the argument
//    from the list after processing it.
//
//    Hank Childs, Sat Mar 10 15:35:36 PST 2001
//    Start timings manager too.
//
//    Hank Childs, Mon Aug 13 13:43:21 PDT 2001
//    Remove the directory before the program name to allow debug stream to
//    go to current directory.
//
//    Hank Childs, Tue Aug 21 13:05:28 PDT 2001
//    Registered a new handler.
//
//    Jeremy Meredith, Fri Feb  1 16:05:36 PST 2002
//    Added an 'else' to the if test when checking arguments, and added a
//    '--i' when stripping the arg.
//
//    Brad Whitlock, Thu Mar 14 12:43:34 PDT 2002
//    Made it work on Windows. Added code to initialize sockets.
//
//    Jeremy Meredith, Wed Aug  7 13:17:01 PDT 2002
//    Made it clamp the debug level to 0 through 5.
//
//    Brad Whitlock, Wed May 21 17:03:56 PST 2003
//    I made it write the command line arguments to the debug logs.
//
//    Brad Whitlock, Wed Jun 18 11:15:22 PDT 2003
//    I made it understand the -timing argument.
//
//    Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//    Removed atexit call, since that is buggy with gcc.
//
// ****************************************************************************

void
Init::Initialize(int &argc, char *argv[], int r, int n, bool strip)
{
    int i, debuglevel = 0;
#if defined(_WIN32)
    bool usePid = true;
#else
    bool usePid = false;
#endif

    bool enableTimings = false;
    for (i=1; i<argc; i++)
    {
        if (strcmp("-debug",argv[i]) == 0)
        {
            debuglevel = 1;
            if (i+1 < argc && isdigit(*(argv[i+1])))
               debuglevel = atoi(argv[i+1]);
            else
               cerr << "Warning: debug level not specified, assuming 1\n";
            if (debuglevel > 5)
            {
                cerr << "Warning: clamping debug level to 5\n";
                debuglevel = 5;
            }
            if (debuglevel < 0)
            {
                cerr << "Warning: clamping debug level to 0\n";
                debuglevel = 0;
            }
            if(strip)
            {
                striparg(i,2, argc,argv);
                --i;
            }
            else
            {
                ++i;
            }
        }
        else if (strcmp("-pid", argv[i]) == 0)
        {
            usePid = true;
        }
        else if (strcmp("-timing",  argv[i]) == 0 ||
                 strcmp("-timings", argv[i]) == 0)
        {
            enableTimings = true;
        }
    }

    char progname_wo_dir[256];
    RemovePrependedDirs(argv[0], progname_wo_dir);
    strcpy(executableName, progname_wo_dir);
    strcpy(componentName, progname_wo_dir);
    char progname[256];
    if (n > 1)
    {
        sprintf(progname, "%s.%03d", progname_wo_dir, r);
    }
    else
    {
        strcpy(progname, progname_wo_dir);
    }
    if (usePid)
    {
#if defined(_WIN32)
        int pid = _getpid();
#else
        int pid = (int) getpid();
#endif
        char progname2[256];
        sprintf(progname2, "%s.%d", progname, pid);
        strcpy(progname, progname2);
    }

    // Initialize the debug streams and also add the command line arguments
    // to the debug logs.
    DebugStream::Initialize(progname, debuglevel);
    for(i = 0; i < argc; ++i)
        debug1 << argv[i] << " ";
    debug1 << endl;

    TimingsManager::Initialize(progname);
    if(enableTimings)
        visitTimer->Enable();

#if !defined(_WIN32)
    set_new_handler(NewHandler);
#endif

#if defined(_WIN32)
    // Windows specific code
    WORD wVersionRequested;
    WSADATA wsaData;

    // Initiate the use of a Winsock DLL (WS2_32.DLL), necessary for sockets.
    wVersionRequested = MAKEWORD(2,2);
    WSAStartup(wVersionRequested, &wsaData);
#endif
}

// ****************************************************************************
//  Function: Init::SetComponentName
//
//  Purpose: Sets the name of the component
//
// ****************************************************************************

void
Init::SetComponentName(const char *cname)
{
   int len;

   if (cname != 0 && (len = strlen(cname)) > 0)
   {
      len = len < sizeof(componentName) ? len : sizeof(componentName) - 1;
      strncpy(componentName, cname, len);
      componentName[len]='\0';
   }
}

// ****************************************************************************
//  Function: Init::GetComponentName
//
//  Purpose: Gets the name of the component. Defaults to name of the
//  executable of it was not set.
//
// ****************************************************************************

const char *
Init::GetComponentName(void)
{
   return (const char *) componentName;
}

// ****************************************************************************
//  Function: Init::IsComponent
//
//  Purpose: Tests name of component against name passed as argument 
//
// ****************************************************************************
bool
Init::IsComponent(const char *compName)
{
   if (strcmp(compName, Init::GetComponentName()) == 0)
       return true;
   else
       return false;
}

// ****************************************************************************
//  Function: Init::GetExecutableName
//
//  Purpose: Gets the name of the executable 
//
// ****************************************************************************

const char *
Init::GetExecutableName(void)
{
   return (const char *) executableName;
}

// ****************************************************************************
//  Function: Init::ComponentRegisterErrorFunction
//
//  Purpose:
//      Registers an error function.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
// ****************************************************************************

void
Init::ComponentRegisterErrorFunction(ErrorFunction ef, void *args)
{
    errorFunctionArgs = args;
    errorFunction = ef;
}


// ****************************************************************************
//  Function: Init::ComponentIssueError
//
//  Purpose:
//      Issues an error message on that component.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
// ****************************************************************************

void
Init::ComponentIssueError(const char *msg)
{
    debug1 << "Error issued: " << msg << endl;
    if (errorFunction != NULL)
    {
        errorFunction(errorFunctionArgs, msg);
    }
    else
    {
        debug1 << "Not able to issue error because no function was registered."
               << endl;
    }
}


// ****************************************************************************
// Method: Init::Finalize
//
// Purpose: 
//   Calls cleanup functions before the application exits.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 19 16:12:37 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Jun 18 11:14:50 PDT 2003
//   Added code to dump timings.
//
//   Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//   Made the method be associated with the Init namespace.
//
// ****************************************************************************

void
Init::Finalize(void)
{
    // Dump timings.
    if(visitTimer)
        visitTimer->DumpTimings();

#if defined(_WIN32)
    // Terminates use of the WS2_32.DLL, the Winsock DLL.
    WSACleanup();
#endif
}

// ****************************************************************************
//  Function: RemovePrependedDirs
//
//  Purpose:
//      Removes prepended directories from a program name.
//
//  Arguments:
//      path      The input.  A program name with a path.
//      name      The output.  Only the name at the end.
//
//  Programmer:   Hank Childs
//  Creation:     August 13, 2001
//
// ****************************************************************************

static void
RemovePrependedDirs(const char *path, char *name)
{
    //
    // Find the last slash by going to the end and working backwards.
    //
    int  len = strlen(path);
    int lastSlash;
    for (lastSlash=len ; path[lastSlash]!='/' && lastSlash>=0 ; lastSlash--)
    {
        continue;
    }

    //
    // Last slash can get to be less than zero if there are no slashes.
    //
    if (lastSlash < 0)
    {
        lastSlash = 0;
    }

    //
    //
    if (path[lastSlash] == '/')
    {
        strcpy(name, path + lastSlash + 1);
    }
    else
    {
        strcpy(name, path + lastSlash);
    }
}
