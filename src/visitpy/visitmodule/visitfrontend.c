// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Python.h>

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <stdio.h>
#endif

#define MAX_ARGUMENTS 100

/*
 * Platform specific includes.
 */
#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define PLEASE_RESET_WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  ifdef PLEASE_RESET_WIN32_LEAN_AND_MEAN
#    undef WIN32_LEAN_AND_MEAN
#  endif
#  define pclose _pclose
#  define popen  _popen
typedef void* DynamicLibraryHandle;
#else
#include <dlfcn.h>
typedef void* DynamicLibraryHandle;
#endif
#if defined(__APPLE__)
#include <stdlib.h>
#endif

/*
 * Module data
 */
typedef struct
{
    long                  moduleDebugLevel;
    char                 *arguments[MAX_ARGUMENTS];
    char                 *moduleFile;
    DynamicLibraryHandle  moduleLibrary;
} VisItModuleState;

static VisItModuleState *moduleState = NULL;

/*
 * Define dynamic library access functions.
 */
#if defined(_WIN32)
void *
OpenDynamicLibrary(const char *filename)
{
    HINSTANCE lib = LoadLibrary(filename);
    if (!lib)
    {
        va_list *va = 0;
        char loadError[500];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, &loadError[0], 500 , va);
        fprintf(stderr, "LoadLibrary(%s) failed because: %s\n", filename, loadError);
    }
#ifdef DEBUG_PRINT
    fprintf(stderr, "LoadLibrary(%s) returned: %p\n", filename, lib);
#endif 
    return (void*)lib;
}

void*
GetDynamicFunction(const char *funcName)
{
    void *func = NULL;

    /* Need to open the library. */
    if(moduleState->moduleLibrary == NULL &&
       moduleState->moduleFile != NULL)
    {
        moduleState->moduleLibrary = OpenDynamicLibrary(moduleState->moduleFile);
    }

    if(moduleState->moduleLibrary != NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "Calling GetProcAddress function to get %s.\n", funcName);
#endif 
        func = (void*)GetProcAddress((HMODULE)moduleState->moduleLibrary, (char*)funcName);
    }
#ifdef DEBUG_PRINT
    else
    {
        fprintf(stderr, "Could not call GetProcAddress function because library could not be opened.\n");
    }
#endif 

    return func;

}
#else
void *
OpenDynamicLibrary(const char *filename)
{
    /* Open the library as global since it will allow the linker
     * to resolve the scripting plugins against the library that
     * we're opening.
     */
    void *h = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if(h == NULL)
    {
        fprintf(stderr, "dlopen(%s) failed because: %s\n", filename, dlerror());
    }
#ifdef DEBUG_PRINT
    fprintf(stderr, "dlopen(%s) returned: %p\n", filename, h);
#endif 
    return h;
}

void *
GetDynamicFunction(const char *funcName)
{
    void *func = NULL;

    /* Need to open the library. */
    if(moduleState->moduleLibrary == NULL &&
       moduleState->moduleFile != NULL)
    {
        moduleState->moduleLibrary = OpenDynamicLibrary(moduleState->moduleFile);
    }

    if(moduleState->moduleLibrary != NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "Calling dlsym function to get %s.\n", funcName);
#endif 
        func = dlsym(moduleState->moduleLibrary, (char*)funcName);
    }
#ifdef DEBUG_PRINT
    else
    {
        fprintf(stderr, "Could not call dlsym function because library could not be opened.\n");
    }
#endif 

    return func;
}
#endif

#ifdef _WIN32
#  ifdef visitfrontend_EXPORTS
#    define VISITFRONTEND_API __declspec(dllexport)
#  else
#    define VISITFRONTEND_API __declspec(dllimport)
#  endif
#else
#  define VISITFRONTEND_API 
#endif


/*
 * Make the initvisit function callable from C.
 */
#ifdef CPLUSPLUS
extern "C"
{
#endif
    VISITFRONTEND_API void initvisit(void);
#ifdef CPLUSPLUS
}
#endif

/*****************************************************************************
 * Function: cleanupvisit
 *
 * Purpose: 
 *   This is an internal function that is called when we need to clean up.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   
 * ***************************************************************************/

static void 
cleanupvisit(void)
{
    if(moduleState != NULL)
    {
        int i;
        for(i = 0; i < MAX_ARGUMENTS; ++i)
        {
            if(moduleState->arguments[i] != NULL)
                free(moduleState->arguments[i]);
        }

        if(moduleState->moduleFile != NULL)
            free(moduleState->moduleFile);

        free(moduleState);
        moduleState = NULL;
    }
}

/*****************************************************************************
 * Function: ReadVisItEnvironment
 *
 * Purpose: 
 *   Reads LIBPATH that would be set up by the VisIt script that
 *   is passed into this function.
 *
 * Arguments:
 *   visitProgram : The fully qualified filename to the visit script that
 *                  we want to run.
 *
 * Returns:    A new string containing the contents of LIBPATH or
 *             NULL if there was a problem.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Nov 22 15:17:56 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Wed May 26 15:49:37 PST 2010
 *   I changed the function so it also reads VISITPLUGINDIR.
 *
 * ***************************************************************************/

int
ReadVisItEnvironment(const char *visitProgram, char **LIBPATH,
    char **VISITPLUGINDIR)
{
    FILE *p = NULL;
    char line[2000];
    char *command =  NULL;
    int lpLen = 0, vpdLen = 0, ldlpLen = 0;
    const char *lp = "LIBPATH=";
    const char *vpd = "VISITPLUGINDIR=";
    lpLen = strlen(lp);
    vpdLen = strlen(vpd);

    *LIBPATH = NULL;
    *VISITPLUGINDIR = NULL;

    command = (char*)malloc(strlen(visitProgram) + 1 + strlen(" -env"));
    if(command == NULL)
        return 0;
    sprintf(command, "%s -env", visitProgram);
    p = popen(command, "r");
    if(p == NULL)
    {
        free(command);
        return 0;
    }

    while(!feof(p))
    {
        fgets(line, 2000, p);
        if(strncmp(line, lp, lpLen) == 0)
        {
            char *value = NULL, *end = NULL;
            int len;
            value = line + lpLen;
            len = strlen(value);
            /* Trim off the newlines and colons at the end.*/
            end = value + len;
            while(*end == '\0' || *end == '\n' || *end == ':')
                *end-- = '\0';
            /* Copy the string. */
            *LIBPATH=(char *)malloc(len + 1);
            strcpy(*LIBPATH, value);
        }
        else if(strncmp(line, vpd, vpdLen) == 0)
        {
            char *value = NULL, *end = NULL;
            int len;
            value = line;
            len = strlen(value);
            /* Trim off the newlines and colons at the end.*/
            end = value + len;
            while(*end == '\0' || *end == '\n' || *end == ':')
                *end-- = '\0';
            /* Copy the string. */
            *VISITPLUGINDIR=(char *)malloc(len + 1);
            strcpy(*VISITPLUGINDIR, value);
        }
    }

    pclose(p);
    free(command);

    return 1;
}

/*****************************************************************************
 * Function: visit_frontend_Launch
 *
 * Purpose: 
 *   This is a Python callable method that determines which VisIt executable
 *   to run and gets the name of the appropriate visitmodule shared library.
 *   This function then dynamically opens that shared library and calls
 *   its "initvisit" function so we can begin loading the Python interface
 *   for that version of VisIt. We then call into that library to set up
 *   some arguments and then we call into the library to tell the version of
 *   the Python interface to launch its viewer.
 *
 * Returns:    NULL on error. 0 on failure. 1 on success.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   Brad Whitlock, Wed May 26 15:50:34 PST 2010
 *   Use provided arguments to get the right version of VisIt. Also set 
 *   VISITPLUGINDIR.
 *
 *   Kathleen Bonnell, Tue May 3 15:17:09 MST 2011
 *   Change module extenstion to .pyd on Windows.
 *
 * ***************************************************************************/

PyObject *
visit_frontend_Launch(PyObject *self, PyObject *args)
{
    char *visitProgram = NULL, *visitProgramWithArgs = NULL,
         *LIBPATH = NULL, *VISITPLUGINDIR = NULL;
    int i, slen = 0;
    PyCFunction func = NULL;
#if defined(_WIN32)
    static char *visitProgramDefault = "visit.exe";
    static const char *moduleFile = "/visitmodule.pyd";
#else
    static char *visitProgramDefault = "visit";
    static const char *moduleFile = "/visitmodule.so";
#endif
#if defined(__APPLE__)
    int length;
    char *envcommand = NULL, *VISITHOME = NULL;
#endif

    /* Return in the unlikely event that moduleState is NULL. */
    if(moduleState == NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "moduleState is NULL!\n");
#endif
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s", &visitProgram))
    {
        visitProgram = visitProgramDefault;
        PyErr_Clear();
    }

#ifdef DEBUG_PRINT
    fprintf(stderr, "visitProgram = %s\n", visitProgram);
#endif

    /* Decorate visitProgram with any arguments we have added. */
    slen = strlen(visitProgram) + 1;
    for(i = 0; moduleState->arguments[i] != NULL; i++)
        slen += strlen(moduleState->arguments[i]) + 1 + 1;
    visitProgramWithArgs = (char *)malloc(slen);
    strcpy(visitProgramWithArgs, visitProgram);
    slen = strlen(visitProgramWithArgs);
    for(i = 0; moduleState->arguments[i] != NULL; i++)
    {
        sprintf(&visitProgramWithArgs[slen], " %s", moduleState->arguments[i]);
        slen += (strlen(moduleState->arguments[i]) + 1);
    }
#ifdef DEBUG_PRINT
    fprintf(stderr, "visitProgramWithArgs = %s\n", visitProgramWithArgs);
#endif

    /* Read the LIBPATH that would be used by the specified VisIt program
     * so we know where to look for visitmodule.so
     */
    ReadVisItEnvironment(visitProgramWithArgs, &LIBPATH, &VISITPLUGINDIR);
    free(visitProgramWithArgs);
    visitProgramWithArgs = NULL;
    if(LIBPATH == 0)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "LIBPATH = NULL\n");
#endif
        return NULL;
    }
#ifdef DEBUG_PRINT
    else
    {
        fprintf(stderr, "LIBPATH = %s\n", LIBPATH);
    }
#endif
    if(VISITPLUGINDIR == 0)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "VISITPLUGINDIR = NULL\n");
#endif
        return NULL;
    }
#ifdef DEBUG_PRINT
    else
    {
        fprintf(stderr, "VISITPLUGINDIR = %s\n", VISITPLUGINDIR);
    }
#endif

#if defined(__APPLE__)
    /* use LIBPATH but truncate the "/lib" */
    length=strlen(LIBPATH) - 4;
    VISITHOME=(char *)malloc(length);
    strncpy(VISITHOME, LIBPATH, length);
#ifdef DEBUG_PRINT
    fprintf(stderr, "VISITHOME = %s\n", VISITHOME);
#endif
    envcommand = (char*)malloc(
      strlen("DYLD_LIBRARY_PATH=") + strlen(VISITHOME) + strlen("/lib:") +
      strlen(VISITHOME) + strlen("/plugins/operators:") +
      strlen(VISITHOME) + strlen("/plugins/plots") + 1);
    sprintf(envcommand, 
      "DYLD_LIBRARY_PATH=%s/lib:%s/plugins/operators:%s/plugins/plots",
       VISITHOME, VISITHOME, VISITHOME);
#ifdef DEBUG_PRINT
    fprintf(stderr, "envcommand = %s\n", envcommand);
#endif
    putenv(envcommand);
    free(VISITHOME);
    free(envcommand);
#else
    /* UNIX */

    /* Set the plugin dir */
    putenv(VISITPLUGINDIR);
#endif

    /* Save off the name of the module file that we will have to load
     * in order to access functions from the VisIt module.
     */
    if(moduleState->moduleFile != NULL)
    {
        free(moduleState->moduleFile);
        moduleState->moduleFile = NULL;
    }
    moduleState->moduleFile = (char*)malloc(strlen(LIBPATH) + 
        strlen(moduleFile) + 1);
    sprintf(moduleState->moduleFile, "%s%s", LIBPATH, moduleFile);
    free(LIBPATH);
#ifdef DEBUG_PRINT
    fprintf(stderr, "moduleFile = %s\n", moduleState->moduleFile);
#endif

    if(moduleState->moduleLibrary == NULL)
    {
        void *init = NULL;
#ifdef DEBUG_PRINT
        fprintf(stderr, "Getting initvisit2 function.\n");
#endif
        /* First try to get the initvisit2 function from the module since
         * it is a little more likely to do what we want if the user
         * has issued "from visit import *".
         */
        if((init = GetDynamicFunction("initvisit2")) == NULL)
        {
#ifdef DEBUG_PRINT
            fprintf(stderr, "Getting initvisit function.\n");
#endif
            init = GetDynamicFunction("initvisit");
        }
        if(init != NULL)
        {
            /* Initialize the VisIt module. This will overwrite this
             * VisIt module's functions with functions from the new
             * VisIt module, allowing us to pick up its interface.
             */
#ifdef DEBUG_PRINT
            fprintf(stderr, "Calling initvisit function.\n");
#endif      
            ((void(*)(void))init)();
        }
#ifdef DEBUG_PRINT
        else
        {
            fprintf(stderr, "Could not get initvisit function.\n");
        }
#endif      

        /* Call the SetDebugLevel function in the loaded library. */
        if(moduleState->moduleDebugLevel > 0)
        {
            func = (PyCFunction)GetDynamicFunction("visit_SetDebugLevel");
            if(func != NULL)
            {
                PyObject *obj = NULL, *debugLevel = NULL, *tuple = NULL;
                /* Create an argument tuple. */
                tuple = PyTuple_New(1);
                debugLevel = PyInt_FromLong(moduleState->moduleDebugLevel);
                PyTuple_SET_ITEM(tuple, 0, debugLevel);
#ifdef DEBUG_PRINT
                fprintf(stderr, "Calling SetDebugLevel function.\n");
#endif 
                obj = (func)(self, tuple);
                Py_DECREF(tuple);
                if(obj != NULL)
                {
                    Py_DECREF(obj);
                }
#ifdef DEBUG_PRINT
                else
                {
                    fprintf(stderr, "SetDebugLevel function returned NULL.\n");
                }
#endif 
            }
        }

        /* Call functions to set the arguments that the user provided. */
        if(moduleState->arguments[0] != NULL)
        {
            func = (PyCFunction)GetDynamicFunction("visit_AddArgument");
            if(func != NULL)
            {
                int i = 0;
                while(moduleState->arguments[i] != NULL)
                {
                    PyObject *obj = NULL, *a = NULL, *tuple = NULL;
                    tuple = PyTuple_New(1);
                    a = PyString_FromString(moduleState->arguments[i]);
                    PyTuple_SET_ITEM(tuple, 0, a);
#ifdef DEBUG_PRINT
                    fprintf(stderr, "Calling AddArgument function.\n");
#endif 
                    obj = (func)(self, tuple);
                    Py_DECREF(tuple);
                    if(obj != NULL)
                    {
                        Py_DECREF(obj);
                    }
#ifdef DEBUG_PRINT
                    else
                    {
                        fprintf(stderr, "AddArgument function returned NULL.\n");
                    }
#endif 
                    ++i;
                }
            }
        }
    }

    /* Call the launch function in the loaded library. */
    func = (PyCFunction)GetDynamicFunction("visit_Launch");
    if(func != NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "Calling Launch function.\n");
#endif
        /* Since we've been able to call the initvisit function in
         * the dynamically loaded module if we've made it here, we won't
         * be in this module any more since its methods will be replaced
         * by those from the new VisIt module. Therefore, we should be
         * able to free memory that we've allocated.
         */
        cleanupvisit();

        /* Call the new VisIt module's Launch method. */
        return (func)(self, args);
    }

#ifdef DEBUG_PRINT
    fprintf(stderr, "Could not call Launch function.\n");
#endif 

    return PyInt_FromLong(0);
}

/*****************************************************************************
 * Function: visit_frontend_AddArgument
 *
 * Purpose: 
 *   This is a Python callable method that appends an argument to the list
 *   of arguments that will be passed to the viewer.
 *
 * Returns:    Py_None.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   
 * ***************************************************************************/

PyObject *
visit_frontend_AddArgument(PyObject *self, PyObject *args)
{
    int i = 0;
    char *argument = 0;

    /* Return in the unlikely event that moduleState is NULL. */
    if(moduleState == NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "moduleState is NULL!\n");
#endif
        return NULL;
    }

    /* Parse the arguments. */
    if(!PyArg_ParseTuple(args, "s", &argument))
        return NULL;

    while(moduleState->arguments[i] != NULL && i < MAX_ARGUMENTS)
        ++i;

    if(i < MAX_ARGUMENTS)
    {
        moduleState->arguments[i] = (char*)malloc(strlen(argument)+1);
        strcpy(moduleState->arguments[i], argument);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************
 * Function: visit_frontend_SetDebugLevel
 *
 * Purpose: 
 *   This is a Python callable method that sets the debug level that will be
 *   used to invoke the viewer.
 *
 * Returns:    Py_None.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   
 * ***************************************************************************/

PyObject *
visit_frontend_SetDebugLevel(PyObject *self, PyObject *args)
{
    int dLevel;
    /* Return in the unlikely event that moduleState is NULL. */
    if(moduleState == NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "moduleState is NULL!\n");
#endif
        return NULL;
    }

    /* Parse the arguments. */
    if(!PyArg_ParseTuple(args, "i", &dLevel))
        return NULL;

    moduleState->moduleDebugLevel = dLevel;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************
 * Function: visit_frontend_GetDebugLevel
 *
 * Purpose: 
 *   This is a Python callable method that returns the debug level used to
 *   invoke the viewer.
 *
 * Returns:    The debug level.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   
 * ***************************************************************************/

PyObject *
visit_frontend_GetDebugLevel(PyObject *self, PyObject *args)
{
    /* Return in the unlikely event that moduleState is NULL. */
    if(moduleState == NULL)
    {
#ifdef DEBUG_PRINT
        fprintf(stderr, "moduleState is NULL!\n");
#endif
        return NULL;
    }

    return PyInt_FromLong(moduleState->moduleDebugLevel);
}

/* 
 * The methods that are available until Launch is called.
 */
static PyMethodDef visit_methods[] = {
{"Launch", visit_frontend_Launch, METH_VARARGS, NULL},
{"SetDebugLevel", visit_frontend_SetDebugLevel, METH_VARARGS, NULL},
{"GetDebugLevel", visit_frontend_GetDebugLevel, METH_VARARGS, NULL},
{"AddArgument", visit_frontend_AddArgument, METH_VARARGS, NULL},
{NULL, NULL, METH_VARARGS, NULL}
};

/*****************************************************************************
 * Function: initvisit
 *
 * Purpose: 
 *   This is the entry point to the module.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Dec 13 10:45:58 PDT 2006
 *
 * Modifications:
 *   Brad Whitlock, Web Feb 3 10:21:23 PDT 2010
 *   Always make the initvisit function visible.
 *
 *   Brad Whitlock, Wed Jun  2 13:32:10 PDT 2010
 *   Make a call to init threads since that creates the global interpreter
 *   lock that we need later for callbacks to work.
 *
 * ***************************************************************************/

void 
#if __GNUC__ >= 4
/* Ensure this function is visible even if -fvisibility=hidden was passed */
__attribute__ ((visibility("default")))
#endif 
initvisit(void)
{
    if(moduleState == NULL)
    {
        moduleState = (VisItModuleState*)malloc(sizeof(VisItModuleState));
        memset(moduleState, 0, sizeof(VisItModuleState));
    }

    /* Make sure that threads are init'd */
    PyEval_InitThreads();

    /* Add the VisIt module to Python. Note that we're passing just a
     * couple of methods and then the interface that we load will
     * get the rest of the functions.
     */
    Py_InitModule("visit", visit_methods);
}


