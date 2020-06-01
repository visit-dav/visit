// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "VisItControlInterface_V2.h"
#include "VisItInterfaceTypes_V2P.h"
#include "SimV2Tracing.h"
#include "DeclareDataCallbacks.h"
#include "SimUI.h"
#include "VisIt_View2D.h"
#include "VisIt_View3D.h"
#include "VisIt_NameList.h"

#ifdef _WIN32
#if _MSC_VER < 1600
#define _WIN32_WINNT 0x0502
#endif
#include <winsock2.h>
#include <direct.h>
#include <sys/stat.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#ifndef VISIT_STATIC
#include <dlfcn.h>
#endif
#if defined(__APPLE__)
#include <malloc/malloc.h> // for mstat
#endif
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <visit-config.h> /* For HAVE_SOCKLEN_T */

#define CMD_MAX_STR_LEN 256

/*******************************************************************************
 * Simple dynamic string type
 ******************************************************************************/
typedef struct
{
    char   *str;
    size_t size;
    size_t buffer_size;
    size_t allocation_increment;
} visit_string;

void
visit_string_ctor(visit_string *obj, size_t incr)
{
    obj->str = NULL;
    obj->size = 0;
    obj->buffer_size = 0;
    obj->allocation_increment = incr;
}

void
visit_string_dtor(visit_string *obj)
{
    if(obj->str != NULL)
        free(obj->str);
    obj->str = NULL;
    obj->size = 0;
    obj->buffer_size = 0;
}

void
visit_string_reserve(visit_string *obj, int len)
{
    if(obj->buffer_size < len)
    {
        obj->str = (char *)realloc(obj->str, len);
        if(obj->str != NULL)
            obj->buffer_size = len;
    }
}

int
visit_string_append(visit_string *obj, const char *s, size_t len)
{
    size_t neededSize;

    if(obj == NULL || s == NULL || len == 0)
        return 0;

    neededSize = obj->size + len + 1;

    if(obj->buffer_size < neededSize)
    {
        /* Enlarge the buffer if needed. */
        while(obj->buffer_size < neededSize)
            obj->buffer_size += obj->allocation_increment;
        obj->str = (char *)realloc(obj->str, obj->buffer_size);
    }

    /* Append the new string. */
    memcpy(obj->str + obj->size, s, len);
    obj->size += len;
    obj->str[obj->size] = '\0';

    return 1;
}

int
visit_string_copy(visit_string *obj, const char *s)
{
    if(obj->str == s)
        return 1;

    obj->size = 0;
    if(s == NULL)
        return 1;

    return visit_string_append(obj, s, strlen(s));
}

void
visit_string_empty(visit_string *obj)
{
    obj->size = 0;
}

/* ****************************************************************************
 *  File:  VisItControlInterface.c
 *
 *  Purpose:
 *    Abstraction of VisIt Engine wrapper library.  Handles the
 *    grunt work of actually connecting to visit that must be done
 *    outside of the VisItEngine DLL, such as:
 *       1) setting up a listen socket
 *       2) writing a .sim file
 *       3) opening the VisItEngine .so and retrieving the functions from it
 *       4) accepting an incoming socket connection
 *       5) removing the .sim file when the program exits
 *
 *  Programmer:  Jeremy Meredith
 *  Creation:    May 5, 2005
 *
 *****************************************************************************/

#define INITIAL_PORT_NUMBER 5609

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define MAX_SIMULATION_FILENAME 1024

#define LIBSIM_MAX_STRING_SIZE      1024
#define LIBSIM_MAX_STRING_LIST_SIZE 100
#define LIBSIM_ENV_ALLOCATION_SIZE  10000

static int BroadcastInt(int *value, int sender);


/* VisIt Engine Library function pointers */
typedef struct
{
    void *(*get_engine)(void);
    int   (*get_descriptor)(void*);
    int   (*process_input)(void*);
    int   (*initialize)(void*,int,char**);
    int   (*initialize_batch)(void*,int,char**);
    int   (*connect_viewer)(void*,int,char**);
    void  (*time_step_changed)(void*);
    void  (*execute_command)(void*,const char*);
    void  (*disconnect)();
    void  (*set_slave_process_callback)(void(*)());
    void  (*set_command_callback)(void*,void(*)(const char*,const char*,void*),void*);
    int   (*save_window)(void*,const char *, int, int, int);
    void  (*debug_logs)(int,const char *);
    int   (*set_mpicomm)(void *);
    int   (*set_mpicomm_f)(int *);

    int   (*add_plot)(void *, const char *, const char *);
    int   (*add_operator)(void *, const char *, int);
    int   (*draw_plots)(void *);
    int   (*delete_active_plots)(void *);
    int   (*set_active_plots)(void *, const int *, int);
    int   (*change_plot_var)(void *, const char *, int);

    int   (*set_plot_options)(void *, const char *, int, void *, int);
    int   (*set_operator_options)(void *, const char *, int, void *, int);


    int   (*exportdatabase_with_options)(void *, const char *, const char *, visit_handle, visit_handle);
    int   (*restoresession)(void *, const char *);

    int   (*set_view2D)(void *, visit_handle);
    int   (*get_view2D)(void *, visit_handle);
    int   (*set_view3D)(void *, visit_handle);
    int   (*get_view3D)(void *, visit_handle);
} control_callback_t;

#define STRUCT_MEMBER(F, FR, FA)  void (*set_##F)(FR (*) FA, void*);

typedef struct
{
    DECLARE_DATA_CALLBACKS(STRUCT_MEMBER)
} data_callback_t;

typedef struct
{
    control_callback_t control;
    data_callback_t    data;
} visit_callback_t;

typedef struct
{
    int    id;
    void (*cb)(void*);
    void  *cbdata;
} SyncCallback;

static visit_callback_t *callbacks = NULL;

/* Internal Variables */
#ifdef _WIN32
#define VISIT_SOCKET         SOCKET
#define VISIT_INVALID_SOCKET INVALID_SOCKET
#ifndef VISIT_STATIC
static HMODULE     dl_handle;
#endif
static SOCKET      listenSocket = VISIT_INVALID_SOCKET;
static SOCKET      engineSocket = VISIT_INVALID_SOCKET;
#else
#define VISIT_SOCKET         int
#define VISIT_INVALID_SOCKET -1
#ifndef VISIT_STATIC
static void       *dl_handle = NULL;
#endif
static int         listenSocket = VISIT_INVALID_SOCKET;
static int         engineSocket = VISIT_INVALID_SOCKET;
#endif

static int         viewer_connected = 0;
static int         libsim_runtime_loaded = 0;

static int       (*BroadcastInt_internal)(int *value, int sender) = NULL;
static int       (*BroadcastInt_internal2)(int *value, int sender, void *) = NULL;
static void       *BroadcastInt_internal2_data = NULL;

static int       (*BroadcastString_internal)(char *str, int len, int sender) = NULL;
static int       (*BroadcastString_internal2)(char *str, int len, int sender, void *) = NULL;
static void       *BroadcastString_internal2_data = NULL;

static char       *visit_directory = NULL;
static char       *visit_options = NULL;
static void       *engine = NULL;
static int         engine_argc = 0;
static char      **engine_argv = NULL;
static char       *simulationFileName = NULL;
static char        securityKey[17];
static char        localhost[256];
static int         listenPort = -1;
struct sockaddr_in listenSockAddr;
static int         isParallel = FALSE;
static int         parallelRank = 0;
static char        lastError[1024] = "";
static int           visit_sync_enabled = 1;
static SyncCallback *visit_sync_callbacks = NULL;
static int           visit_sync_callbacks_size = 0;
static int           visit_sync_id = 1;
static void        (*visit_command_callback)(const char*,const char*,void*) = NULL;
static void         *visit_command_callback_data = NULL;
static void        (*visit_slave_process_callback)(void) = NULL;
static void        (*visit_slave_process_callback2)(void *) = NULL;
static void         *visit_slave_process_callback2_data = NULL;
static void         *visit_communicator = NULL;
static int           visit_batch_mode = 0;
static int          *visit_communicator_f = NULL;
static visit_string visit_env = {NULL, 0, 0, LIBSIM_ENV_ALLOCATION_SIZE};

/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
                               Internal Functions
 *******************************************************************************
 *******************************************************************************
 ******************************************************************************/

static char es_buffer[100];
static const char *es_VISIT_ERROR = "VISIT_ERROR";
static const char *es_VISIT_OKAY = "VISIT_OKAY";

static const char *
ErrorToString(int err)
{
    if(err == VISIT_ERROR)
        return es_VISIT_ERROR;
    else if(err == VISIT_OKAY)
        return es_VISIT_OKAY;
    else
        sprintf(es_buffer, "%d", err);
    return es_buffer;
}

#ifdef _WIN32
/*******************************************************************************
* UTILITY FUNCTIONS
*******************************************************************************/
static int
ReadKeyFromRoot(HKEY which_root, const char *ver, const char *key, char **keyval)
{
    const unsigned int maxStrSize = 500;

    int  readSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", ver);
    *keyval = (char *) malloc(maxStrSize * sizeof(char));
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD keyType, strSize = maxStrSize;
        if(RegQueryValueEx(hkey, key, NULL, &keyType,
                           (unsigned char *)*keyval, &strSize) == ERROR_SUCCESS)
        {
            readSuccess = 1;
        }

        RegCloseKey(hkey);
    }

    return readSuccess;
}

static int
ReadKey(const char *ver, const char *key, char **keyval)
{
    int retval = 0;

    if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, ver, key, keyval)) == 0)
        retval = ReadKeyFromRoot(HKEY_CURRENT_USER, ver, key, keyval);

    return retval;
}

static void
GetVisItDirectory(char *visitdir, int maxlen)
{
    if(visit_directory != NULL)
    {
        snprintf(visitdir, maxlen, "%s", visit_directory);
    }
    else
    {
        char *visitpath = 0;
        char visitversion[10];
        int major, minor, patch, beta;
        int haveVISITHOME = 0;

        /* Iterate through a few probable versions and keep the most up to date one */
        for(major = 2; major <= 4; ++major)
            for(minor = (major==2?2:0); minor <= 12; ++minor)
                for(patch = 0; patch < 5; ++patch)
                    for(beta = 0; beta < 2; ++beta)
                    {
                        char curversion[10], *path = NULL;
                        if(beta == 0)
                            snprintf(curversion, 10, "%d.%d.%d", major, minor, patch);
                        else
                            snprintf(curversion, 10, "%d.%d.%db", major, minor, patch);
                        if(ReadKey(curversion, "VISITHOME", &path))
                        {
                            strcpy(visitversion, curversion);
                            if(visitpath != NULL)
                                free(visitpath);
                            visitpath = path;
                            haveVISITHOME = 1;
                        }
                    }
        if(haveVISITHOME)
        {
            snprintf(visitdir, maxlen, "%s", visitpath);
            free(visitpath);
        }
        else
        {
            visitdir[0] = '\0';
        }
    }
}
#else
/* UNIX */
/*******************************************************************************
*
* Name: ReadEnvironmentFromCommand
*
* Purpose: Read the output of "visit -env" using the specified VisIt command.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Eric Brugger, Thu Sep 14 12:53:46 PDT 2006
*   Changed the routine to read at most ENV_BUF_SIZE bytes of output from
*   the execution of the command.
*
*   Brad Whitlock, Fri Jul 25 12:11:16 PDT 2008
*   Changed some types to remove warnings. Added trace information.
*
*   Brad Whitlock, Thu Jul  2 15:53:57 PDT 2015
*   Change to use visit_string.
*
*******************************************************************************/

static int ReadEnvironmentFromCommand(const char *visitpath, visit_string *output)
{
   /* VisIt will tell us what variables to set. */
   /* (redirect stderr so it won't complain if it can't find visit) */
   ssize_t n;
   char command[LIBSIM_MAX_STRING_SIZE], buf[LIBSIM_MAX_STRING_SIZE];
   FILE *file;

   LIBSIM_API_ENTER1(ReadEnvironmentFromCommand, "visitpath=%s", visitpath);

#ifdef VISIT_COMPILER
#define STR(s) STR2(s)
#define STR2(s) #s
   snprintf(command, LIBSIM_MAX_STRING_SIZE, "%s -compiler %s %s -env -engine 2>/dev/null",
           visitpath, STR(VISIT_COMPILER), visit_options ? visit_options : "");
#else
   snprintf(command, LIBSIM_MAX_STRING_SIZE, "%s %s -env -engine 2>/dev/null",
           visitpath, visit_options ? visit_options : "");
#endif

   LIBSIM_MESSAGE1("command=%s", command);

   file = popen(command, "r");
   visit_string_empty(output);
   while ((n = read(fileno(file), (void*)buf, LIBSIM_MAX_STRING_SIZE)) > 0)
   {
       visit_string_append(output, buf, (size_t)n);
   }

   LIBSIM_MESSAGE1("Output=%s", output->str);

   LIBSIM_API_LEAVE1(ReadEnvironmentFromCommand, "return %d", (int)output->size);
   return output->size;
}

#endif

/*******************************************************************************
*
* Name: GetEnvironment
*
* Purpose: Read the environment.
*
* Author: Brad Whitlock
*
* Modifications:
*
*******************************************************************************/

static void
GetEnvironment(visit_string *env)
{
/* We don't call this function on BG/Q or Win32. */
#if !defined(VISIT_BLUE_GENE_Q) && !defined(_WIN32)
    int done = 0;

    visit_string_empty(env);

    /* Try the one specified in by the visit_dir command first */
    if (visit_directory)
    {
        char path[LIBSIM_MAX_STRING_SIZE];
        sprintf(path, "%s/bin/visit", visit_directory);
        done = ReadEnvironmentFromCommand(path, env);
    }

    /* Try the one in their VISIT_HOME environment var next */
    if (!done)
    {
      if(getenv("VISIT_HOME") != NULL)
      {
        char path[LIBSIM_MAX_STRING_SIZE];
        sprintf(path, "%s/bin/visit", getenv("VISIT_HOME"));
        done = ReadEnvironmentFromCommand(path, env);
      }
    }

    /* Try the one in their path next */
    if (!done)
    {
        done = ReadEnvironmentFromCommand("visit", env);
    }

    /* If we still can't find it, try the LLNL path /usr/gapps/visit */
    if (!done)
    {
        done = ReadEnvironmentFromCommand("/usr/gapps/visit/bin/visit", env);
    }

    /* We didn't get good values, empty the environment string. */
    if(!done)
        visit_string_dtor(env);
#endif
}

static void
VisItMkdir(const char *dir, int permissions)
{
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, permissions);
#endif
}



/******************************************************************************
*
* Name: visit_get_sync
*
* Purpose: This function returns a slot in the sync table, creating one if needed.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*  Cyrus Harrison, Wed Nov 18 11:56:37 PST 2009
*  Declare "*ptr" at top of function for strict c compile.
*
******************************************************************************/

static SyncCallback *
visit_get_sync(void)
{
    SyncCallback *retval = NULL;
    SyncCallback *ptr = NULL;

    if(visit_sync_callbacks == NULL)
    {
        visit_sync_callbacks = (SyncCallback *)calloc(20, sizeof(SyncCallback));
        visit_sync_callbacks_size = 20;
        retval = visit_sync_callbacks;
    }
    else
    {
        /* return first empty slot. */
        int i;
        for(i = 0; i < visit_sync_callbacks_size; ++i)
        {
            if(visit_sync_callbacks[i].id == 0)
                return &visit_sync_callbacks[i];
        }

        /* resize */
        ptr = (SyncCallback *)calloc(visit_sync_callbacks_size+20, sizeof(SyncCallback));
        memcpy((void*)ptr, (void*)visit_sync_callbacks, visit_sync_callbacks_size * sizeof(SyncCallback));
        free(visit_sync_callbacks);
        visit_sync_callbacks = ptr;
        retval = &visit_sync_callbacks[visit_sync_callbacks_size];
        visit_sync_callbacks_size += 20;
    }
    return retval;
}

/******************************************************************************
*
* Name: visit_get_sync2
*
* Purpose: This function returns the slot with a given id.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*
******************************************************************************/

static SyncCallback *
visit_get_sync2(int id)
{
    int i;
    for(i = 0; i < visit_sync_callbacks_size; ++i)
    {
        if(visit_sync_callbacks[i].id == id)
            return &visit_sync_callbacks[i];
    }
    return NULL;
}

/******************************************************************************
*
* Name: visit_add_sync
*
* Purpose: This function adds a callback function to the sync table and sends
*          a sync message to VisIt. When we get that message back, via the
*          command callback mechanism, we execute the callback.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*  Cyrus Harrison, Wed Nov 18 11:56:37 PST 2009
*  Declare "cmd" at top of function for strict c compile.
*
******************************************************************************/

static void
visit_add_sync(void (*cb)(void*), void *cbdata)
{
    char cmd[30];

    if(cb != NULL && callbacks->control.execute_command != NULL)
    {
        SyncCallback *sync = visit_get_sync();
        sync->id = visit_sync_id++;
        sync->cb = cb;
        sync->cbdata = cbdata;
        sprintf(cmd, "INTERNALSYNC %d", sync->id);

        (*callbacks->control.execute_command)(engine, cmd);
    }
}

/******************************************************************************
*
* Name: visit_do_sync
*
* Purpose: This function executes a callback function for the given sync id
*          and then removes the callback from the sync table.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*
******************************************************************************/

static void
visit_do_sync(int id)
{
    SyncCallback *sync = visit_get_sync2(id);
    if(sync != NULL)
    {
        (*sync->cb)(sync->cbdata);
        sync->id = 0;
    }
}

/******************************************************************************
*
* Name: visit_handle_command_callback
*
* Purpose: This function gets installed as the engine's command callback and
*          it lets us intercept sync messages from the viewer. If we did not
*          get a sync message then we forward the command to the user-provided
*          command callback.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*   Brad Whitlock, Sun Feb 27 16:17:24 PST 2011
*   Handle UI commands.
*
******************************************************************************/

static void
visit_handle_command_callback(const char *cmd, const char *args, void *cbdata)
{
    if(strcmp(cmd, "INTERNALSYNC") == 0)
    {
        int id = -1;
        if(sscanf(args, "%d", &id) == 1)
            visit_do_sync(id);
    }
    else if(visit_command_callback != NULL)
    {
        if(strncmp(cmd, "UI;", 3) == 0)
        {
            char *cmd2, *args2;
            cmd2 = (char*) malloc((strlen(cmd) + 1 - 3) * sizeof(char));
            strcpy(cmd2, cmd + 3);
            args2 = cmd2;
            while(*args2 != ';')
                args2++;
            *args2++ = '\0';

            /* Try and call user-defined slot functions */
            if(sim_ui_handle(cmd2, args2) == 0)
            {
                /* As a backup, call the regular command function. */
                (*visit_command_callback)(cmd2, args2, visit_command_callback_data);
            }

            free(cmd2);
        }
        else
        {
            (*visit_command_callback)(cmd, args, visit_command_callback_data);
        }
    }
}

/******************************************************************************
*
* Name: visit_process_engine_command
*
* Purpose: This function processes commands from the viewer on all processors.
*          We use this function to help implement synchronization with the
*          viewer.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*   Brad Whitlock, Thu Jan 27 15:55:31 PST 2011
*   I added LEAVE trace statements to early returns so the trace logs don't
*   lool like recursion when this function is called repeatedly.
*
******************************************************************************/

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

static int
visit_process_engine_command(void)
{
    int command;
    LIBSIM_API_ENTER(visit_process_engine_command);
    if(isParallel)
    {
        if (parallelRank == 0)
        {
            int success = VisItProcessEngineCommand();

            if (success)
            {
                command = VISIT_COMMAND_SUCCESS;
                BroadcastInt(&command, 0);
                LIBSIM_API_LEAVE1(visit_process_engine_command, "return %d", 1);
                return 1;
            }
            else
            {
                command = VISIT_COMMAND_FAILURE;
                BroadcastInt(&command, 0);
                LIBSIM_API_LEAVE1(visit_process_engine_command, "return %d", 0);
                return 0;
            }
        }
        else
        {
            /* Note: only through the SlaveProcessCallback callback
             * above can the rank 0 process send a VISIT_COMMAND_PROCESS
             * instruction to the non-rank 0 processes. */
            while (1)
            {
                BroadcastInt(&command, 0);
                switch (command)
                {
                case VISIT_COMMAND_PROCESS:
                    VisItProcessEngineCommand();
                    break;
                case VISIT_COMMAND_SUCCESS:
                    LIBSIM_API_LEAVE1(visit_process_engine_command, "return %d", 1);
                    return 1;
                case VISIT_COMMAND_FAILURE:
                    LIBSIM_API_LEAVE1(visit_process_engine_command, "return %d", 0);
                    return 0;
                }
            }
        }
    }

    command = VisItProcessEngineCommand() ? 1 : 0;
    LIBSIM_API_LEAVE1(visit_process_engine_command, "return %d", command);
    return command;
}

/*******************************************************************************
*
* Name: CreateRandomSecurityKey
*
* Purpose: Create a random key that clients will need to connect to us.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 15:19:25 PDT 2008
*   Trace information.
*
*******************************************************************************/
static void CreateRandomSecurityKey(void)
{
    int len = 8;
    int i;
    securityKey[0] = '\0';

    LIBSIM_API_ENTER(CreateRandomSecurityKey);

#if defined(_WIN32)
    srand((unsigned)time(0));
#else
    srand48((long)(time(0)));
#endif
    for (i=0; i<len; i++)
    {
        char str[3];
#if defined(_WIN32)
        double d = (double)(rand()) / (double)(RAND_MAX);
        sprintf(str, "%02x", (int)(d * 255.));
#else
        sprintf(str, "%02x", (int)(lrand48() % 256));
#endif
        strcat(securityKey, str);
    }

    LIBSIM_API_LEAVE1(CreateRandomSecurityKey, "securityKey=%s", securityKey);
}

/*******************************************************************************
*
* Name: ReceiveSingleLineFromSocket
*
* Purpose: Receive a single line character transmission from the socket.
*          Note that this assumes it is not part of a larger transmission.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 11:54:12 PDT 2008
*   Changed some assignments to NULL to remove warnings.
*
*******************************************************************************/
static void ReceiveSingleLineFromSocket(char *buffer, size_t maxlen, VISIT_SOCKET desc)
{
    char *buf = buffer;
    char *ptr = buffer;
    char *tmp = NULL;
    int n;

    LIBSIM_API_ENTER2(ReceiveSingleLineFromSocket,
                      "maxlen=%d, desc=%d",
                      (int)maxlen,desc);

    strcpy(buffer, "");
    tmp = strstr(buf, "\n");
    while (!tmp)
    {
        n = recv(desc, (void*)ptr, maxlen, 0);
        ptr += n;
        *ptr = '\0';
        tmp = strstr(buf, "\n");
    }
    *tmp = '\0';

    LIBSIM_API_LEAVE1(ReceiveSingleLineFromSocket,
                     "buffer=%s", buffer);
}

/*******************************************************************************
*
* Name: ReceiveContinuousLineFromSocket
*
* Purpose: Receive a single line as part of a larger transmission.  Note that
*          it assumes buffer is initialized for the first call to the empty
*          string, and that it retains its values as an intermediate buffer
*          between calls.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 11:55:05 PDT 2008
*   Changed some assignments to NULL to remove warnings.
*
*******************************************************************************/
static char *ReceiveContinuousLineFromSocket(char *buffer, size_t maxlen, VISIT_SOCKET desc)
{
    char *buf = buffer;
    char *ptr = buffer;
    char *tmp = NULL;
    int n;

    LIBSIM_API_ENTER2(ReceiveContinuousLineFromSocket,
                      "maxlen=%d, desc=%d",
                      (int)maxlen,desc);

    tmp = strstr(buf, "\n");
    while (!tmp)
    {
        n = recv(desc, (void*)ptr, maxlen, 0);
        ptr += n;
        *ptr = '\0';
        tmp = strstr(buf, "\n");
    }
    *tmp = '\0';

    LIBSIM_API_LEAVE1(ReceiveContinuousLineFromSocket,
                     "return %s", buffer);
    return tmp+1;
}

/*******************************************************************************
*
* Name: SendStringOverSocket
*
* Purpose: Send a single null-terminated string over a socket.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:14:54 PDT 2008
*  Trace information.
*
*******************************************************************************/
static int SendStringOverSocket(char *buffer, VISIT_SOCKET desc)
{
    size_t      nleft, nwritten;
    const char *sptr;

    LIBSIM_API_ENTER2(SendStringOverSocket, "buffer=%s, desc=%d", buffer,desc);
    /* Send it! */
    sptr = (const char*)buffer;
    nleft = strlen(buffer);
    while (nleft >= 1)
    {
        if ((nwritten = send(desc, (const char *)sptr, nleft, 0)) == 0)
        {
            LIBSIM_API_LEAVE1(SendStringOverSocket,
                             "send() returned 0. return %d", TRUE);
            return FALSE;
        }
        nleft -= nwritten;
        sptr  += nwritten;
    }
    LIBSIM_API_LEAVE1(SendStringOverSocket, "return %d", TRUE);

    return TRUE;
}

/*******************************************************************************
*
* Name: BroadcastInt
*
* Purpose: Call the broadcast int callback function to broadcast an int among
*          processors.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Tue Apr 17 10:33:00 PDT 2012
*   Add support for a callback with data.
*
*******************************************************************************/

static int
BroadcastInt(int *value, int sender)
{
    int retval = 0;

    if(sender==parallelRank)
    {
        LIBSIM_API_ENTER2(BroadcastInt, "value=%d, sender=%d", *value, sender);
    }
    else
    {
        LIBSIM_API_ENTER1(BroadcastInt, "sender=%d", sender);
    }

    if(BroadcastInt_internal2 != NULL)
        retval = (*BroadcastInt_internal2)(value, sender, BroadcastInt_internal2_data);
    else if(BroadcastInt_internal != NULL)
        retval = (*BroadcastInt_internal)(value, sender);
    else
    {
        LIBSIM_MESSAGE("BroadcastInt function not set.");
    }

    LIBSIM_API_LEAVE1(BroadcastInt, "return %s", ErrorToString(retval));
    return retval;
}

/*******************************************************************************
*
* Name: BroadcastString
*
* Purpose: Call the broadcast string callback function to broadcast a string
*          among processors.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Tue Apr 17 10:33:00 PDT 2012
*   Add support for a callback with data.
*
*******************************************************************************/

static int
BroadcastString(char *str, int len, int sender)
{
    int retval = 0;

    if(sender==parallelRank)
    {
        LIBSIM_API_ENTER3(BroadcastString, "str=%s, len=%d, sender=%d",
                          str, len, sender);
    }
    else
    {
        LIBSIM_API_ENTER2(BroadcastString, "len=%d, sender=%d", len, sender);
    }

    if(BroadcastString_internal2 != NULL)
        retval = (*BroadcastString_internal2)(str, len, sender, BroadcastString_internal2_data);
    else if(BroadcastString_internal != NULL)
        retval = (*BroadcastString_internal)(str, len, sender);
    else
    {
        LIBSIM_MESSAGE("BroadcastString function not set.");
    }

    LIBSIM_API_LEAVE1(BroadcastString, "return %s", ErrorToString(retval));
    return retval;
}

static int
BroadcastVisItString(visit_string *obj, int sender)
{
    int retval, len = obj->size + 1;
    LIBSIM_API_ENTER(BroadcastVisItString);

    /* Send the length of the buffer (including null terminator).*/
    BroadcastInt(&len, sender);

    /* If the destination string is not large enough, reserve sufficient space. */
    visit_string_reserve(obj, len);

    /* Broadcast the string contents. */
    retval = BroadcastString(obj->str, len, sender);
    obj->size = len-1;

    LIBSIM_API_LEAVE1(BroadcastVisItString, "return %s", ErrorToString(retval));
    return retval;
}

/*******************************************************************************
*
* Name: VerifySecurityKeys
*
* Purpose: Receive a security key over the socket and compare it to ours,
*          sending the result of the comparison back to the client.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 11:57:40 PDT 2008
*  Added casts to eliminate warnings. Added trace information.
*
*******************************************************************************/
static int VerifySecurityKeys(VISIT_SOCKET desc)
{
   int securityKeyLen;
   int offeredKeyLen;
   char offeredKey[VISIT_SOCKET_BUFFER_SIZE] = "";

   LIBSIM_API_ENTER1(VerifySecurityKeys, "desc=%d", desc);

   if (parallelRank == 0)
   {
      /* The first thing the VCL sends is the key */
      ReceiveSingleLineFromSocket(offeredKey, VISIT_SOCKET_BUFFER_SIZE, desc);

      if (isParallel)
      {
         /* Broadcast the known key */
         securityKeyLen = (int)strlen(securityKey);
         BroadcastInt(&securityKeyLen, 0);
         BroadcastString(securityKey,  securityKeyLen+1, 0);

         /* Broadcast the received key */
         offeredKeyLen = (int)strlen(offeredKey);
         BroadcastInt(&offeredKeyLen, 0);
         BroadcastString(offeredKey,  offeredKeyLen+1, 0);
      }

      /* Make sure the keys match, and send a response */
      if (strcmp(securityKey, offeredKey) != 0)
      {
         SendStringOverSocket("failure\n", desc);
         LIBSIM_API_LEAVE1(VerifySecurityKeys, "return %d", FALSE);
         return FALSE;
      }
      else
      {
         SendStringOverSocket("success\n", desc);
      }
   }
   else
   {
      /* Receive the security keys and make sure they match */
      BroadcastInt(&securityKeyLen, 0);
      BroadcastString(securityKey, securityKeyLen+1, 0);
      BroadcastInt(&offeredKeyLen, 0);
      BroadcastString(offeredKey, offeredKeyLen+1, 0);

      if (strcmp(securityKey, offeredKey) != 0)
      {
         /* Error: keys didn't match */
         LIBSIM_API_LEAVE1(VerifySecurityKeys, "return %d", FALSE);
         return FALSE;
      }
   }

   LIBSIM_API_LEAVE1(VerifySecurityKeys, "return %d", TRUE);
   return TRUE;
}

/*******************************************************************************
*
* Name: GetConnectionParameters
*
* Purpose: Receive the command line to be used to initialize the VisIt engine.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 15:11:11 PDT 2008
*   Trace information.
*
*******************************************************************************/
static int GetConnectionParameters(VISIT_SOCKET desc)
{
   char buf[VISIT_SOCKET_BUFFER_SIZE] = "";
   char *tmpbuf;
   char *nxtbuf;
   int i;

   LIBSIM_API_ENTER1(GetConnectionParameters, "desc=%d", desc);

   engine_argv = (char**) malloc(100 * sizeof(char*));

   if (parallelRank == 0)
   {
      /* Receive the ARGV over the socket */
      engine_argc = 0;

      tmpbuf = buf;
      while (1)
      {
         nxtbuf = ReceiveContinuousLineFromSocket(tmpbuf, VISIT_SOCKET_BUFFER_SIZE, desc);

         if (strlen(tmpbuf) == 0)
            break;

         engine_argv[engine_argc] = strdup(tmpbuf);
         engine_argc++;
         tmpbuf = nxtbuf;
      }

      /* Broadcast them to the other processors if needed */
      if (isParallel)
      {
         BroadcastInt(&engine_argc, 0);
         for (i = 0 ; i < engine_argc; i++)
         {
            int len = strlen(engine_argv[i]);
            BroadcastInt(&len, 0);
            BroadcastString(engine_argv[i], len+1, 0);
         }
      }
   }
   else
   {
      /* Receive the ARGV */
      BroadcastInt(&engine_argc, 0);
      for (i = 0 ; i < engine_argc; i++)
      {
         int len;
         BroadcastInt(&len, 0);
         BroadcastString(buf, len+1, 0);
         engine_argv[i] = strdup(buf);
      }
   }

   LIBSIM_API_LEAVE1(GetConnectionParameters, "return %d", TRUE);

   return TRUE;
}

/*******************************************************************************
*
* Name: CreateEngineAndConnectToViewer
*
* Purpose: Intialize the engine and connect to the viewer.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 14:33:37 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Fri Aug 26 10:38:11 PDT 2011
*   Set the communicator that the engine will use.
*
*   Brad Whitlock, Fri Jul 20 10:59:19 PDT 2012
*   Set the communicator after we have the engine but before it ends up in
*   functions that call PAR_Init. This lets PAR_Init skip duplication of the
*   MPI_COMM_WORLD communicator since we already set our custom communicator.
*
*   Brad Whitlock, Fri Sep 28 12:15:32 PDT 2012
*   I split up the function into 2 functions. Allow for this function being
*   called repeatedly.
*
*   Brad Whitlock, Thu Sep 18 16:12:37 PDT 2014
*   I made it call a different initialization function for batch.
*
*   Brad Whitlock, Tue Nov 11 17:32:10 PST 2014
*   Get batch mode options from visit_options string.
*
*   Brad Whitlock, Thu Feb  7 18:01:53 PST 2019
*   Fix buffer size allocation to prevent a crash.
*
*******************************************************************************/

static int CreateEngine(int batch)
{
    int status = VISIT_ERROR;

    LIBSIM_API_ENTER(CreateEngine);

    if(callbacks != NULL)
    {
        status = VISIT_OKAY;
        if(engine == NULL)
        {
            /* get the engine */
            LIBSIM_MESSAGE("  Calling simv2_get_engine");
            engine = (*callbacks->control.get_engine)();
            if (!engine)
            {
                LIBSIM_API_LEAVE1(CreateEngine,
                                 "engine could not be allocated. return %d",
                                 VISIT_ERROR);
                return VISIT_ERROR;
            }

            /* If there are no engine args, as would be the case if we've
             * started creating the engine without a viewer connection then
             * we must create some engine arguments.
             */
            if(engine_argc == 0)
            {
                size_t argv_size = LIBSIM_MAX_STRING_LIST_SIZE * sizeof(char *);
                engine_argc = 1;
                engine_argv = (char **)malloc(argv_size);
                memset(engine_argv, 0, argv_size);
                engine_argv[0] = strdup("/usr/gapps/visit/bin/visit");
#if 1
                if(visit_options != NULL)
                {
                    char *start, *end, *tmpstr;
                    start = end = visit_options;
                    tmpstr = (char *)malloc(argv_size);
                    while(start != NULL)
                    {
                        int len = 0;
                        end = start;
                        while(*end != ' ' && *end != '\0')
                            ++end;
                        len = end - start;
                        if(len > LIBSIM_MAX_STRING_SIZE-1)
                            len = LIBSIM_MAX_STRING_SIZE-1;
                        memcpy(tmpstr, start, len);
                        tmpstr[len] = '\0';
                        if(len > 0 && engine_argc < LIBSIM_MAX_STRING_LIST_SIZE)
                            engine_argv[engine_argc++] = strdup(tmpstr);
                        if(*end == '\0')
                            start = NULL;
                        else
                            start = end + 1;
                    }
                    free(tmpstr);
                }
#else
                engine_argv[1] = strdup("-debug");
                engine_argv[2] = strdup("5");
                engine_argv[3] = strdup("-clobber_vlogs");
                engine_argc = 4;
#endif
            }

            if(visit_communicator != NULL)
            {
                VisItSetMPICommunicator(visit_communicator);
            }
            else if(visit_communicator_f != NULL)
            {
                VisItSetMPICommunicator_f(visit_communicator_f);
            }

            if(batch && callbacks->control.initialize_batch != NULL)
            {
                LIBSIM_MESSAGE_STRINGLIST("Calling simv2_initialize_batch: argv=",
                                          engine_argc, engine_argv);
                if (!(*callbacks->control.initialize_batch)(engine, engine_argc, engine_argv))
                {
                    VisItDisconnect();
                    LIBSIM_API_LEAVE1(CreateEngine,
                                      "simv2_initialize_batch failed. return %s",
                                      ErrorToString(VISIT_ERROR));
                    return VISIT_ERROR;
                }

                visit_batch_mode = 1;
            }
            else
            {
                LIBSIM_MESSAGE_STRINGLIST("Calling simv2_initialize: argv=",
                                          engine_argc, engine_argv);
                if (!(*callbacks->control.initialize)(engine, engine_argc, engine_argv))
                {
                    VisItDisconnect();
                    LIBSIM_API_LEAVE1(CreateEngine,
                                      "simv2_initialize failed. return %s",
                                      ErrorToString(VISIT_ERROR));
                    return VISIT_ERROR;
                }
            }
        }
    }

    LIBSIM_API_LEAVE1(CreateEngine,"return %s", ErrorToString(status));
    return status;
}

/*******************************************************************************
*
* Name: ConnectToViewer
*
* Purpose: Connect the engine to the viewer.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 14:33:37 PDT 2008
*   Trace information.
*
*******************************************************************************/

static int ConnectToViewer(void)
{
    LIBSIM_API_ENTER(ConnectToViewer);

    LIBSIM_MESSAGE_STRINGLIST("Calling simv2_connect_viewer: argv=",
                              engine_argc, engine_argv);
    if (!(*callbacks->control.connect_viewer)(engine, engine_argc, engine_argv))
    {
        VisItDisconnect();
        LIBSIM_API_LEAVE1(ConnectToViewer,
                         "simv2_connect_viewer failed. return %s",
                         ErrorToString(VISIT_ERROR));
        return VISIT_ERROR;
    }

    viewer_connected = 1;

    LIBSIM_API_LEAVE1(ConnectToViewer,"return %s", ErrorToString(VISIT_OKAY));
    return VISIT_OKAY;
}

/*******************************************************************************
*
* Name: GetLocalhostName
*
* Purpose: Determine the true local host name.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 14:41:30 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Fri Nov 12 23:40:23 PST 2010
*  I added an extra layer of gethostbyname to match RemoteProcess.
*
*******************************************************************************/
static int GetLocalhostName(void)
{
    char localhostStr[256];
    struct hostent *localhostEnt = NULL;

    LIBSIM_API_ENTER(GetLocalhostName);

    LIBSIM_MESSAGE("  Calling gethostname");
    if (gethostname(localhostStr, 256) == -1)
    {
        /* Couldn't get the hostname, it's probably invalid */
        LIBSIM_API_LEAVE1(GetLocalhostName,
                         "gethostname failed. return=%d", FALSE);
        return FALSE;
    }
    LIBSIM_MESSAGE1("gethostname returned %s\n", localhostStr);

    LIBSIM_MESSAGE("  Calling gethostbyname");
    localhostEnt = gethostbyname(localhostStr);
    if (localhostEnt == NULL)
    {
        /* Couldn't get the full host entry; it's probably invalid */
        LIBSIM_MESSAGE("gethostbyname failed. call gethostbyname(localhost)");

        strcpy(localhostStr, "localhost");
        localhostEnt = gethostbyname(localhostStr);
        if(localhostEnt != NULL)
        {
            sprintf(localhost, "%s", localhostEnt->h_name);
            LIBSIM_MESSAGE1("Using return value of gethostbyname: %s", localhost);
        }
        else
        {
            LIBSIM_MESSAGE("Use \"localhost\" as the host name.");
            strcpy(localhost, "localhost");
        }
    }
    else
    {
        sprintf(localhost, "%s", localhostEnt->h_name);
        LIBSIM_MESSAGE1("Using return value of gethostbyname: %s", localhost);
    }
    LIBSIM_API_LEAVE1(GetLocalhostName, "return %s", localhost);

    return TRUE;
}

/*******************************************************************************
*
* Name: StartListening
*
* Purpose: Find a port and start listening.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/
static int StartListening(void)
{
    int portFound = FALSE;
    int on = 1;
    int err;

    LIBSIM_API_ENTER(StartListening);
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
    {
        /* Cannot open a socket. */
        LIBSIM_API_LEAVE1(StartListening, "socket() failed. return=%d", FALSE);
        return FALSE;
    }

    /*
     * Look for a port that can be used.
     */
    LIBSIM_MESSAGE("Looking for a listening port");
    listenSockAddr.sin_family = AF_INET;
    listenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenPort = INITIAL_PORT_NUMBER;
    while (!portFound && listenPort < 32767)
    {
        listenSockAddr.sin_port = htons(listenPort);
#if !defined(_WIN32)
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

        err = bind(listenSocket, (struct sockaddr *)&listenSockAddr,
                   sizeof(listenSockAddr));
        if (err)
        {
            listenPort++;
        }
        else
        {
            portFound = TRUE;
        }
    }
    if (!portFound)
    {
        /* Cannot find unused port. */
       listenSocket = VISIT_INVALID_SOCKET;
       LIBSIM_API_LEAVE1(StartListening, "port not found. return %d", FALSE);
       return FALSE;
    }

    LIBSIM_MESSAGE("  Calling listen() on socket");
    err = listen(listenSocket, 5);
    if (err)
    {
       listenSocket = VISIT_INVALID_SOCKET;
       LIBSIM_API_LEAVE1(StartListening, "listen() failed. return %d", FALSE);
       return FALSE;
    }

    LIBSIM_API_LEAVE1(StartListening, "return %d", TRUE);
    return TRUE;
}

/*******************************************************************************
*
* Name: AcceptConnection
*
* Purpose: Perform the accept on the listen socket.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 14:47:55 PDT 2008
*  Trace information.
*
*******************************************************************************/
static VISIT_SOCKET AcceptConnection(void)
{
    VISIT_SOCKET desc = VISIT_INVALID_SOCKET;
    int opt = 1;

    LIBSIM_API_ENTER(AcceptConnection);

    /* Wait for the socket to become available on the other side. */
    do
    {
#ifdef HAVE_SOCKLEN_T
        socklen_t len;
#else
#ifdef __APPLE__
        unsigned int len;
#else
        int len;
#endif
#endif
        len = sizeof(struct sockaddr);
        LIBSIM_MESSAGE("  Calling accept()");
        desc = accept(listenSocket, (struct sockaddr *)&listenSockAddr, &len);
    }
    while (desc == VISIT_INVALID_SOCKET
#ifndef _WIN32
           && errno != EWOULDBLOCK
#endif
           );

    /* Disable Nagle algorithm. */
#if defined(_WIN32)
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY,
               (const char FAR*)&opt, sizeof(int));
#else
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    LIBSIM_API_LEAVE1(AcceptConnection, "desc=%d", desc);
    return desc;
}

/*******************************************************************************
*
* Name: GetHomeDirectory
*
* Purpose: Return the true home directory path.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Mon Jan 17 18:31:23 PST 2011
*   I added a Windows implementation.
*
*   Brad Whitlock, Wed Apr 18 13:16:24 PDT 2012
*   I added Jean's fix for determining the home directory. We now use the
*   HOME environment variable if it exists and then we back up to the old
*   method.
*
*   Brad Whitlock, Wed Aug 12 16:46:44 PDT 2015
*   On BG/Q, getpwuid doesn't work reliably so we avoid calling it and return
*   NULL instead. The BG/Q job will not have HOME set unless the batch scheduler
*   set it. On cobalt, the user must pass --env HOME=$HOME to qsub.
*
*******************************************************************************/
static const char *GetHomeDirectory(void)
{
#ifdef _WIN32
    char visituserpath[MAX_PATH], expvisituserpath[MAX_PATH];
    static char *returnpath = NULL;
    int haveVISITUSERHOME=0, pathlen = 0;
    TCHAR szPath[MAX_PATH];

    LIBSIM_API_ENTER(GetHomeDirectory);

    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,
                             SHGFP_TYPE_CURRENT, szPath)))
    {
        snprintf(visituserpath, 512, "%s", szPath);
        haveVISITUSERHOME = 1;
    }

    if (haveVISITUSERHOME)
        ExpandEnvironmentStrings(visituserpath, expvisituserpath, MAX_PATH);
    else
        strcpy(expvisituserpath, "C:\\Users");

    if(returnpath != NULL)
        free(returnpath);
    pathlen = 1 + strlen(expvisituserpath);
    returnpath = (char*) malloc(pathlen * sizeof(char));

    strcpy(returnpath, expvisituserpath);

    LIBSIM_API_LEAVE1(GetHomeDirectory, "homedir=%s", returnpath);

    return returnpath;
#else
    const char *home = NULL;

    LIBSIM_API_ENTER(GetHomeDirectory);
    home = getenv("HOME");
#if !defined(VISIT_BLUE_GENE_Q)
    if(home == NULL)
    {
        struct passwd *users_passwd_entry = NULL;
        users_passwd_entry = getpwuid(getuid());
        home = users_passwd_entry->pw_dir;
    }
#endif
    LIBSIM_API_LEAVE1(GetHomeDirectory, "homedir=%s", (home!=NULL) ? home : "NULL");

    return home;
#endif
}

/*******************************************************************************
*
* Name: EnsureSimulationDirectoryExists
*
* Purpose: Make the simulations directory.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 14:56:47 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Wed Aug 12 16:49:48 PDT 2015
*   Work around unset home directory.
*
*******************************************************************************/
static int EnsureSimulationDirectoryExists(void)
{
    int retval = 0;
    char str[1024];
    const char *home = NULL;
    LIBSIM_API_ENTER(EnsureSimulationDirectoryExists);

    if((home = GetHomeDirectory()) != NULL)
    {
#ifdef _WIN32
        snprintf(str, 1024, "%s/Simulations", home);
        VisItMkdir(str, 7*64 + 7*8 + 7);
        LIBSIM_MESSAGE1("mkdir %s", str);
#else
        snprintf(str, 1024, "%s/.visit", home);
        VisItMkdir(str, 7*64 + 7*8 + 7);
        LIBSIM_MESSAGE1("mkdir %s", str);

        snprintf(str, 1024, "%s/.visit/simulations", home);
        VisItMkdir(str, 7*64 + 7*8 + 7);
        LIBSIM_MESSAGE1("mkdir %s", str);
#endif
        retval = 1;
    }

    LIBSIM_API_LEAVE(EnsureSimulationDirectoryExists);
    return retval;
}

/*******************************************************************************
*
* Name: RemoveSimFile
*
* Purpose: This will delete the sim file from the file system.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 14:57:51 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Thu Nov 10 10:35:11 PST 2011
*  Make simulationFileName dynamic.
*
*******************************************************************************/
static void RemoveSimFile(void)
{
    LIBSIM_API_ENTER(RemoveSimFile);
    if(simulationFileName != NULL)
    {
        LIBSIM_MESSAGE1("unlink(%s)", simulationFileName);
        unlink(simulationFileName);
    }
    LIBSIM_API_LEAVE(RemoveSimFile);
}

/*******************************************************************************
*
* Name: visit_get_runtime_function
*
* Purpose: Get the function pointer from the runtime library
*
* Author: Brad Whitlock
*
* Modifications:
*   Brad Whitlock, Thu Nov 25 23:26:23 PST 2010
*   Ported to Windows.
*
*   Brad Whitlock, Mon Oct 24 09:45:42 PDT 2011
*   Wrap it with VISIT_STATIC since we don't use them in that case.
*
*******************************************************************************/
#ifndef VISIT_STATIC
void *
visit_get_runtime_function(const char *name)
{
    void *f = NULL;

    LIBSIM_API_ENTER2(visit_get_runtime_function, "handle=%p, name=%s", dl_handle, name);
#ifdef _WIN32
    f = GetProcAddress(dl_handle, name);
#else
    f = dlsym(dl_handle, name);
#endif
    if(f == NULL)
    {
        sprintf(lastError, "Function %s could not be located in the runtime.\n", name);
    }
    LIBSIM_API_LEAVE1(visit_get_runtime_function, "func=%p", (void*)f);
    return f;
}
#endif

/*******************************************************************************
*
* Name: LoadVisItLibrary_XXX
*
* Purpose: Loads the Visit runtime library, setting dl_handle and lastError.
*
* Author: Brad Whitlock
*
* Modifications:
*   Brad Whitlock, Mon Oct 24 09:44:38 PDT 2011
*   Wrap it all with VISIT_STATIC since we don't use them in that case.
*
*   Cyrus Harrison, Tue Nov 26 10:53:52 PST 2013
*   Support new osmesa setup for visit post 2.7.0
*
*******************************************************************************/
#ifndef VISIT_STATIC
#ifdef _WIN32
static int LoadVisItLibrary_Windows(void)
{
    char visitpath[1024], lib[1024];

    LIBSIM_API_ENTER(LoadVisItLibrary_Windows);
    GetVisItDirectory(visitpath, 1024);

    /* load library */
    if (isParallel)
    {
        snprintf(lib, 256, "%s\\simV2runtime_par.dll", visitpath);
    }
    else
    {
        snprintf(lib, 256, "%s\\simV2runtime_ser.dll", visitpath);
    }

    LIBSIM_MESSAGE1("Loading runtime: %s", lib);
    SetErrorMode(0);
    dl_handle = LoadLibrary(lib);

#if 0
    /* TODO: Make the error reporting work. */
    if (dl_handle == NULL)
    {
        WCHAR msg[1024];
        va_list *va = 0;
        LIBSIM_MESSAGE1("Error: %d", GetLastError());
        LIBSIM_MESSAGE1("Error: %x", GetLastError());
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0,
                      msg, 1024, va);
        wprintf(L"Formatted message: %s\n", msg);

        /*snprintf(lastError, 1024, "Failed to open the VisIt library: %s\n", msg);*/
    }
#endif

    LIBSIM_API_LEAVE(LoadVisItLibrary_Windows);

    return (dl_handle != NULL) ? VISIT_OKAY : VISIT_ERROR;
}
#else
/*******************************************************************************
*
* Name: Preload_OSMesaGL
*
* Purpose: Loads OSMesa (from VISIT_MESA_LIB) as libGL.
*
* Author: Cyrus Harrison
*
* Modifications:
*   Burlen Loring, Sat May  3 10:52:13 PDT 2014
*   fix bug, return a value. report if the env var is not found
*
*******************************************************************************/
int Preload_OSMesaGL(void)
{
    /* load library */
    void *gl_dl_handle = NULL;
    if(getenv("VISIT_MESA_LIB") != NULL)
    {
        char *osmesa_lib_path = getenv("VISIT_MESA_LIB");
        LIBSIM_MESSAGE1("Attempting to preload osmesa as libGL -- calling dlopen(%s)", osmesa_lib_path);
        gl_dl_handle = dlopen(osmesa_lib_path, RTLD_LAZY | RTLD_GLOBAL);
        if(gl_dl_handle == NULL)
        {
            LIBSIM_MESSAGE1("dlopen error: %s", dlerror());
            LIBSIM_MESSAGE1("failed to preload osmesa from VISIT_MESA_LIB: %s",osmesa_lib_path);
            return -1;
        }
        return 0;
    }
    LIBSIM_MESSAGE("Not preloading osmesa, VISIT_MESA_LIB was not set");
    return -1;
}

static int LoadVisItLibrary_UNIX(void)
{
    char lib[256];

    /* load library */
#ifdef __APPLE__
    const char *extension = "dylib";
    const char *LD_LIBRARY_PATH = "DYLD_LIBRARY_PATH";
#else
    const char *extension = "so";
    const char *LD_LIBRARY_PATH = "LD_LIBRARY_PATH";
#endif
    /*
       If VISIT_MESA_LIB is set we want to use osmesa instead of system gl.
       For this case: load osmesa as libGL before the rest of libsim.
    */
    if(getenv("VISIT_MESA_LIB") != NULL)
    {
        Preload_OSMesaGL();
    }

    if (isParallel)
    {
        sprintf(lib, "libsimV2runtime_par.%s", extension);
    }
    else
    {
        sprintf(lib, "libsimV2runtime_ser.%s", extension);
    }

    LIBSIM_MESSAGE1("Calling dlopen(%s)", lib);
    dl_handle = dlopen(lib, RTLD_LAZY | RTLD_GLOBAL);

    if (dl_handle == NULL)
    {
        LIBSIM_MESSAGE1("Failed to open the VisIt library: %s\n", dlerror())
        LIBSIM_MESSAGE1("Calling getenv(%s)", LD_LIBRARY_PATH);

        if(getenv(LD_LIBRARY_PATH) != NULL)
        {
            char *libpath = NULL, *start = NULL, *ptr = NULL;
            libpath = start = strdup(getenv(LD_LIBRARY_PATH));
            LIBSIM_MESSAGE1("getenv returned: %s", getenv(LD_LIBRARY_PATH));

            /* Iterate over all paths in the LD_LIBRARY_PATH and try each one
             * until we get a library to open.
             */
            do
            {
                /* If there is a separator then null it out*/
                ptr = strstr(libpath, ":");
                if (ptr)
                    *ptr = '\0';

                /* Try to use the current libpath to open the library */
                if (isParallel)
                    sprintf(lib, "%s/libsimV2runtime_par.%s", libpath, extension);
                else
                    sprintf(lib, "%s/libsimV2runtime_ser.%s", libpath, extension);
                LIBSIM_MESSAGE1("Calling dlopen(%s)", lib);
                dl_handle = dlopen(lib, RTLD_LAZY | RTLD_GLOBAL);
                if(dl_handle == NULL)
                {
                     LIBSIM_MESSAGE1("dlopen error: %s", dlerror());
                }
                if(ptr != NULL)
                    libpath = ptr + 1;
                else
                    libpath += strlen(libpath);
            } while(dl_handle == NULL && *libpath != '\0');
            free(start);
        }
    }

    if (dl_handle == NULL)
        sprintf(lastError, "Failed to open the VisIt library: %s\n", dlerror());

    return (dl_handle != NULL) ? VISIT_OKAY : VISIT_ERROR;
}
#endif
#endif

/*******************************************************************************
*
* Name: CloseVisItLibrary
*
* Purpose: Closes the Visit runtime library, setting dl_handle.
*
* Author: Brad Whitlock
*
* Modifications:
*   Brad Whitlock, Mon Oct 24 09:47:15 PDT 2011
*   Do nothing if we're building statically.
*
*******************************************************************************/

static void CloseVisItLibrary(void)
{
#ifndef VISIT_STATIC
#ifdef _WIN32
    if(dl_handle != NULL)
    {
        FreeLibrary(dl_handle);
        dl_handle = NULL;
    }
#else
    /* Call dlclose(dl_handle) ???*/
    dl_handle = NULL;
#endif
#endif

    libsim_runtime_loaded = 0;
}

/*******************************************************************************
*
* Name: LoadVisItLibrary
*
* Purpose: Load the DLL/SO of the VisIt Engine and get the needed function
*          pointers from it.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Thu Jan 25 14:58:26 PST 2007
*   Added update plots and execute_command.
*
*   Jeremy Meredith, Fri Nov  2 18:06:42 EDT 2007
*   Use dylib as the extension for OSX.
*
*   Brad Whitlock, Fri Feb 13 09:54:47 PST 2009
*   I added code to get data callback setting functions from the library.
*
*   Brad Whitlock, Tue Feb 16 15:09:52 PST 2010
*   Try a list of libraries in the library path.
*
*   Brad Whitlock, Thu Nov 25 23:03:23 PST 2010
*   I moved the code to load libraries into helper functions to separate out
*   some Windows-only logic.
*
*   Brad Whitlock, Fri Aug 26 09:48:22 PDT 2011
*   Initialize set communicator function.
*
*   Brad Whitlock, Mon Oct 24 09:43:36 PDT 2011
*   Add case for static builds where we just access the function that we want
*   instead of opening it dynamically.
*
*   Brad Whitlock, Thu Nov 10 09:53:29 PST 2011
*   Add control functions for setting up plots.
*
*   Brad Whitlock, Fri Sep 19 17:44:43 PDT 2014
*   Added exportdatabase, restoresession, changed plot setup functions.
*
*******************************************************************************/

static int LoadVisItLibrary(void)
{
#ifdef VISIT_STATIC
/* Static */
#define CONTROL_DLSYM(N, FR, FA) \
    extern FR simv2_##N FA; \
    callbacks->control.N = simv2_##N;

#define CONTROL_DLSYM_OPTIONAL(N, FR, FA) \
    extern FR simv2_##N FA; \
    callbacks->control.N = simv2_##N;

#define DATA_DLSYM(N, FR, FA) \
    extern void simv2_set_##N(FR (*) FA, void *); \
    callbacks->data.set_##N = simv2_set_##N;

    LIBSIM_API_ENTER(LoadVisItLibrary);
    int status = VISIT_OKAY;
    if(callbacks == NULL)
    {
#else
/* Dynamic */
#define SAFE_DLSYM(D, NAME, SIMV2NAME, FR, FA, CHECK) \
    callbacks->D.NAME = (FR (*) FA)visit_get_runtime_function(#SIMV2NAME); \
    CHECK(\
    if (!callbacks->D.NAME) \
    { \
        CloseVisItLibrary();\
        LIBSIM_API_LEAVE2(LoadVisItLibrary, "%s: return %d", lastError, FALSE); \
        return FALSE; \
    })

#define CHECKIT(A) A
#define NOCHECKIT(A)

#define CONTROL_DLSYM(N, FR, FA)          SAFE_DLSYM(control, N, simv2_##N, FR, FA, CHECKIT)
#define CONTROL_DLSYM_OPTIONAL(N, FR, FA) SAFE_DLSYM(control, N, simv2_##N, FR, FA, NOCHECKIT)
#define DATA_DLSYM(N, FR, FA)             SAFE_DLSYM(data,    set_##N, simv2_set_##N, void, (FR (*) FA, void*), CHECKIT)

    int status = VISIT_OKAY;
    LIBSIM_API_ENTER(LoadVisItLibrary);
    if(callbacks == NULL)
    {
        /* Load the library */
#ifdef _WIN32
        status = LoadVisItLibrary_Windows();
#else
        status = LoadVisItLibrary_UNIX();
#endif

        if (status == VISIT_ERROR)
        {
            LIBSIM_API_LEAVE2(LoadVisItLibrary, "%s: return %d", lastError, status);
            return status;
        }
#endif /* VISIT_STATIC */

        callbacks = (visit_callback_t *) malloc(sizeof(visit_callback_t));
        memset(callbacks, 0, sizeof(visit_callback_t));

        CONTROL_DLSYM(get_engine,                 void *, (void));
        CONTROL_DLSYM(get_descriptor,             int,    (void *));
        CONTROL_DLSYM(process_input,              int,    (void *));
        CONTROL_DLSYM(initialize,                 int,    (void *, int, char **));
        CONTROL_DLSYM(connect_viewer,             int,    (void *, int, char **));
        CONTROL_DLSYM(time_step_changed,          void,   (void *));
        CONTROL_DLSYM(execute_command,            void,   (void *,const char*));
        CONTROL_DLSYM(disconnect,                 void,   ());
        CONTROL_DLSYM(set_slave_process_callback, void,   (void (*)()));
        CONTROL_DLSYM(set_command_callback,       void,   (void*,void (*)(const char*,const char*,void*),void*));
        CONTROL_DLSYM(save_window,                int,    (void*,const char *,int,int,int));
        CONTROL_DLSYM(debug_logs,                 void,   (int,const char *));
        CONTROL_DLSYM(set_mpicomm,                int,    (void *));
        CONTROL_DLSYM(set_mpicomm_f,              int,    (int *));

        CONTROL_DLSYM_OPTIONAL(add_plot,             int,    (void *, const char *, const char *));
        CONTROL_DLSYM_OPTIONAL(add_operator,         int,    (void *, const char *, int));
        CONTROL_DLSYM_OPTIONAL(draw_plots,           int,    (void *));
        CONTROL_DLSYM_OPTIONAL(delete_active_plots,  int,    (void *));
        CONTROL_DLSYM_OPTIONAL(set_active_plots,     int,    (void *, const int *, int));
        CONTROL_DLSYM_OPTIONAL(change_plot_var,      int,    (void *, const char *, int));

        CONTROL_DLSYM_OPTIONAL(set_plot_options,     int,    (void *, const char *, int, void *, int));
        CONTROL_DLSYM_OPTIONAL(set_operator_options, int,    (void *, const char *, int, void *, int));

        CONTROL_DLSYM_OPTIONAL(initialize_batch,     int,    (void *, int, char **));
        CONTROL_DLSYM_OPTIONAL(exportdatabase_with_options, int, (void *, const char *, const char *, visit_handle, visit_handle));
        CONTROL_DLSYM_OPTIONAL(restoresession,       int,    (void *, const char *));

        CONTROL_DLSYM_OPTIONAL(set_view2D,           int,    (void *, visit_handle));
        CONTROL_DLSYM_OPTIONAL(get_view2D,           int,    (void *, visit_handle));
        CONTROL_DLSYM_OPTIONAL(set_view3D,           int,    (void *, visit_handle));
        CONTROL_DLSYM_OPTIONAL(get_view3D,           int,    (void *, visit_handle));

        /* Get the data functions from the library. */
        DECLARE_DATA_CALLBACKS(DATA_DLSYM)
    }

    LIBSIM_API_LEAVE1(LoadVisItLibrary, "return %d", status);
    return status;
}

/*******************************************************************************
*
* Name: InitializeRuntime
*
* Purpose: Load the runtime library and create an engine from it. After calling
*          this function, we can make calls on the engine to do work.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Thu Sep 18 16:13:39 PDT 2014
*   Add batch flag.
*
*******************************************************************************/

static int
InitializeRuntime(int batch)
{
    /* load the library */
    if (LoadVisItLibrary() == VISIT_ERROR)
    {
        LIBSIM_API_LEAVE1(InitializeRuntime,
                          "LoadVisItLibrary failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    /* Create the engine object. */
    if (CreateEngine(batch) == VISIT_ERROR)
    {
        LIBSIM_API_LEAVE1(InitializeRuntime,
                          "CreateEngine failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    libsim_runtime_loaded = 1;

    return VISIT_OKAY;
}

/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
                             Public API Functions
 *******************************************************************************
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
*
* Name: VisItSetBroadcastIntFunction
*
* Purpose: Set the callback to broadcast an integer.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 15:33:27 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Tue Apr 17 10:41:41 PDT 2012
*   Set version2 pointers to NULL.
*
*******************************************************************************/
void  VisItSetBroadcastIntFunction(int (*bi)(int *, int))
{
    LIBSIM_API_ENTER1(VisItSetBroadcastIntFunction, "func=%p", (void*)bi);
    BroadcastInt_internal = bi;

    BroadcastInt_internal2 = NULL;
    BroadcastInt_internal2_data = NULL;

    LIBSIM_API_LEAVE(VisItSetBroadcastIntFunction);
}

void  VisItSetBroadcastIntFunction2(int (*bi)(int *, int, void *), void *bidata)
{
    LIBSIM_API_ENTER1(VisItSetBroadcastIntFunction2, "func=%p", (void*)bi);
    BroadcastInt_internal = NULL;

    BroadcastInt_internal2 = bi;
    BroadcastInt_internal2_data = bidata;

    LIBSIM_API_LEAVE(VisItSetBroadcastIntFunction2);
}

/*******************************************************************************
*
* Name: VisItSetBroadcastStringFunction
*
* Purpose: Set the callback to broadcast a string.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:33:37 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Tue Apr 17 10:41:41 PDT 2012
*  Set version2 pointers to NULL.
*
*******************************************************************************/
void  VisItSetBroadcastStringFunction(int (*bs)(char *, int, int))
{
    LIBSIM_API_ENTER1(VisItSetBroadcastStringFunction, "func=%p", (void*)bs);
    BroadcastString_internal = bs;

    BroadcastString_internal2 = NULL;
    BroadcastString_internal2_data = NULL;

    LIBSIM_API_LEAVE(VisItSetBroadcastStringFunction);
}

void  VisItSetBroadcastStringFunction2(int (*bs)(char *, int, int, void *), void *bsdata)
{
    LIBSIM_API_ENTER1(VisItSetBroadcastStringFunction2, "func=%p", (void*)bs);
    BroadcastString_internal = NULL;

    BroadcastString_internal2 = bs;
    BroadcastString_internal2_data = bsdata;

    LIBSIM_API_LEAVE(VisItSetBroadcastStringFunction2);
}

/*******************************************************************************
*
* Name: VisItSetParallel
*
* Purpose: Set whether or not we have to work in parallel.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:33:37 PDT 2008
*  Trace information.
*
*******************************************************************************/
void  VisItSetParallel(int ispar)
{
   LIBSIM_API_ENTER1(VisItSetParallel, "isparallel=%d", ispar);
   isParallel = ispar;
   LIBSIM_API_LEAVE(VisItSetParallel);
}

/*******************************************************************************
*
* Name: VisItSetParallelRank
*
* Purpose: Set the rank of this processor.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:33:37 PDT 2008
*  Trace information.
*
*******************************************************************************/
void  VisItSetParallelRank(int pr)
{
   LIBSIM_API_ENTER1(VisItSetParallelRank, "rank=%d", pr);
   parallelRank = pr;
   LIBSIM_API_LEAVE(VisItSetParallelRank);
}

/*******************************************************************************
*
* Name: VisItSetDirectory
*
* Purpose: Set the top-level directory for VisIt.  This can either be a
*          installed or development version.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:33:37 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Mon Aug  8 10:37:58 PDT 2011
*  NULL checks.
*
*******************************************************************************/
void VisItSetDirectory(char *d)
{
    LIBSIM_API_ENTER1(VisItSetDirectory, "dir=%s", ((d!=NULL)?d:"NULL"));
    if(d != NULL)
    {
        if (visit_directory == NULL)
            visit_directory = (char*)(malloc(1000 * sizeof(char)));

        if(visit_directory != NULL)
            strcpy(visit_directory, d);
    }
    else
    {
        if(visit_directory != NULL)
            free(visit_directory);
        visit_directory = NULL;
    }
    LIBSIM_API_LEAVE(VisItSetDirectory);
}

/*******************************************************************************
*
* Name: VisItSetOptions
*
* Purpose: Set command-line options needed to pick up the right VisIt.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Mon Aug  8 10:37:58 PDT 2011
*  NULL checks.
*
*******************************************************************************/
void VisItSetOptions(char *o)
{
    LIBSIM_API_ENTER1(VisItSetOptions, "options=%s", ((o!=NULL)?o:"NULL"));
    if(o != NULL)
    {
        if (visit_options == NULL)
            visit_options = (char*)(malloc(1000 * sizeof(char)));

        if(visit_options != NULL)
            strcpy(visit_options, o);
    }
    else
    {
        if(visit_options != NULL)
            free(visit_options);
        visit_options = NULL;
    }
    LIBSIM_API_LEAVE(VisItSetOptions);
}

/*******************************************************************************
*
* Name: VisItGetEnvironment
*
* Purpose: Try to determine the environment variables that the VisIt Engine
*          needs to run and return a string containing those values.  The
*          VisIt script can tell us this.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Thu Jul  2 14:48:53 PDT 2015
*   I moved code to a helper function. Return a copy of the environment.
*
*******************************************************************************/
char *VisItGetEnvironment(void)
{
#ifdef _WIN32
    LIBSIM_API_ENTER(VisItGetEnvironment);
    LIBSIM_API_LEAVE1(VisItGetEnvironment, "return %s", "NULL");
    return NULL;
#else
    GetEnvironment(&visit_env);

    LIBSIM_API_LEAVE1(VisItGetEnvironment, "return %s",
                     (visit_env.str ? visit_env.str : "NULL"));

    return visit_env.str ? strdup(visit_env.str) : NULL;
#endif
}

/*******************************************************************************
*
* Name: VisItSetupEnvironment
*
* Purpose: Try to determine the environment variables that the VisIt Engine
*          needs to run.  The VisIt script can tell us this.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Eric Brugger, Thu Sep 14 12:53:46 PDT 2006
*   Changed the routine to allocate the buffer new_env to be ENV_BUF_SIZE
*   bytes in size instead of 10000.
*
*   Brad Whitlock, Fri Jul 25 15:37:05 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Thu Nov 25 23:40:34 PST 2010
*   Windows port.
*
*   Brad Whitlock, Wed Mar 23 11:06:27 PDT 2011
*   I moved the guts to VisItSetupEnvironment2.
*
*******************************************************************************/
int VisItSetupEnvironment(void)
{
    int r;
    LIBSIM_API_ENTER(VisItSetupEnvironment);
    /* Pass NULL for the environment so we'll read it in the most appropriate way.*/
    r = VisItSetupEnvironment2(NULL);
    LIBSIM_API_LEAVE1(VisItSetupEnvironment, "return %d", r);
    return r;
}

/*******************************************************************************
*
* Name: VisItSetupEnvironment2
*
* Purpose: Try to determine the environment variables that the VisIt Engine
*          needs to run.  The VisIt script can tell us this.
*
* Author: Jeremy Meredith / Brad Whitlock, B Division,
*         Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Tue Apr 17 10:35:33 PDT 2012
*  Add test for 2nd broadcast callback.
*
*  Brad Whitlock, Thu Jul  2 15:39:33 PDT 2015
*  Changed how the environment gets read to support long environments.
*
*******************************************************************************/

int VisItSetupEnvironment2(char *env)
{
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    char visitpath[LIBSIM_MAX_STRING_SIZE], tmp[LIBSIM_MAX_STRING_SIZE];

    LIBSIM_API_ENTER(VisItSetupEnvironment2);
    GetVisItDirectory(visitpath, LIBSIM_MAX_STRING_SIZE);

    /* Tell Windows that we want to get DLLs from this path. We DO need this
     * in order for dependent DLLs to be located.
     */
    SetDllDirectory(visitpath);

    /* Set the VisIt home dir. */
    snprintf(tmp, LIBSIM_MAX_STRING_SIZE, "VISITHOME=%s", visitpath);
    LIBSIM_MESSAGE(tmp);
    putenv(tmp);

    /* Set the plugin dir. */
    snprintf(tmp, LIBSIM_MAX_STRING_SIZE, "VISITPLUGINDIR=%s", visitpath);
    LIBSIM_MESSAGE(tmp);
    putenv(tmp);

    /* Initiate the use of a Winsock DLL (WS2_32.DLL), necessary for sockets. */
    wVersionRequested = MAKEWORD(2,2);
    WSAStartup(wVersionRequested, &wsaData);
#else
   int done = 0, canBroadcast = 0, mustReadEnv = 1;
   char *ptr;

   LIBSIM_API_ENTER(VisItSetupEnvironment2);
#if !defined(VISIT_BLUE_GENE_Q)
   /* Make a copy of the input string & store into visit_env. */
   done = visit_string_copy(&visit_env, env);

   /* Determine whether we can broadcast strings */
   canBroadcast = isParallel &&
                 (BroadcastString_internal != NULL ||
                  BroadcastString_internal2 != NULL);

   /* Determine whether we must read the environment. */
   if(canBroadcast)
       mustReadEnv = (parallelRank == 0) && (env == NULL);
   else
       mustReadEnv = (env == NULL);

   /* Read the environment. */
   if(mustReadEnv)
   {
       done = 0;
       GetEnvironment(&visit_env);
       if(visit_env.size > 0)
           done = 1;
   }

   /* Use broadcast if we can. */
   if(canBroadcast)
   {
       /* Send the string to other processors */
       BroadcastVisItString(&visit_env, 0);
       LIBSIM_MESSAGE1("VisItSetupEnvironment2: After broadcast: %s\n", visit_env.str);

       /* We're done if the string was not empty. */
       done = (visit_env.str != NULL && visit_env.str[0] != '\0');
   }

   if (!done)
   {
       /* We're not using new_env for putenv so we can free it. */
       visit_string_dtor(&visit_env);
       LIBSIM_API_LEAVE1(VisItSetupEnvironment2, "return %d", FALSE);
       return FALSE;
   }

   /* Do a bunch of putenv calls; it should already be formatted correctly */
   ptr = visit_env.str;
   while (ptr != NULL && ptr[0]!='\0')
   {
      int i = 0;
      while (ptr[i]!='\n')
         i++;
      ptr[i] = '\0';

      LIBSIM_MESSAGE1("putenv(%s)", ptr);
      putenv(ptr);

      ptr += i+1;
   }
#endif

#endif
   LIBSIM_API_LEAVE1(VisItSetupEnvironment2, "return %d", TRUE);
   return TRUE;
}

/*******************************************************************************
*
* Name: VisItInitializeSocketAndDumpSimFile
*
* Purpose: Start listening, and write the file telling clients how to connect.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Shelly Prevost, Wed Jan 25 08:50:44 PST 2006
*   Added the guifile argument.
*
*   Brad Whitlock, Fri Jul 25 15:40:03 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Fri Sep 28 14:29:00 PDT 2012
*   Make sim file storage dynamic.
*
*******************************************************************************/
int VisItInitializeSocketAndDumpSimFile(const char *name,
                                        const char *comment,
                                        const char *path,
                                        const char *inputfile,
                                        const char *guifile,
                                        const char *absoluteFilename)
{
    FILE *file = NULL;
    LIBSIM_API_ENTER(VisItInitializeSocketAndDumpSimFile)
    LIBSIM_MESSAGE1("name=%s", name);
    LIBSIM_MESSAGE1("comment=%s", comment);
    LIBSIM_MESSAGE1("path=%s", path);
    LIBSIM_MESSAGE1("inputfile=%s", inputfile);
    LIBSIM_MESSAGE1("guifile=%s", guifile);
    LIBSIM_MESSAGE1("absoluteFilename=%s", absoluteFilename);

    CreateRandomSecurityKey();

    if(simulationFileName == NULL)
    {
        simulationFileName = (char *) malloc(MAX_SIMULATION_FILENAME * sizeof(char));
        memset(simulationFileName, 0, MAX_SIMULATION_FILENAME * sizeof(char));
    }

    if ( !absoluteFilename )
    {
        if(EnsureSimulationDirectoryExists())
        {
            snprintf(simulationFileName, MAX_SIMULATION_FILENAME,
#ifdef _WIN32
                "%s/Simulations/%012d.%s.sim2",
#else
                "%s/.visit/simulations/%012d.%s.sim2",
#endif
                GetHomeDirectory(), (int)time(NULL), name);
        }
        else
        {
            snprintf(simulationFileName, MAX_SIMULATION_FILENAME,
                "%012d.%s.sim2", (int)time(NULL), name);
        }
    }
    else
    {
        snprintf(simulationFileName, MAX_SIMULATION_FILENAME, "%s", absoluteFilename);
    }

    if (!GetLocalhostName())
    {
        LIBSIM_API_LEAVE1(VisItInitializeSocketAndDumpSimFile, "return %d", FALSE);
        return FALSE;
    }

    if (!StartListening())
    {
        LIBSIM_API_LEAVE1(VisItInitializeSocketAndDumpSimFile, "return %d", FALSE);
        return FALSE;
    }

    LIBSIM_MESSAGE1("Opening sim file %s", simulationFileName);
    file = fopen(simulationFileName, "wt");
    if (!file)
    {
        LIBSIM_API_LEAVE1(VisItInitializeSocketAndDumpSimFile, "return %d", FALSE);
        return FALSE;
    }

    atexit(RemoveSimFile);

    fprintf(file, "host %s\n", localhost);
    fprintf(file, "port %d\n", listenPort);
    fprintf(file, "key %s\n", securityKey);
    if (path)
        fprintf(file, "path %s\n", path);
    if (inputfile)
        fprintf(file, "inputfile %s\n", inputfile);
    if (comment)
        fprintf(file, "comment %s\n", comment);
     if ( guifile )
       fprintf(file, "uiFile %s\n", guifile);

    fclose(file);

    LIBSIM_API_LEAVE1(VisItInitializeSocketAndDumpSimFile, "return %d", TRUE);
    return TRUE;
}

/*******************************************************************************
*
* Name: VisItDetectInput
*
* Purpose: Determine who needs attention: the listen socket, the client
*          socket, or the console.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Wed Jun 25 16:28:02 PDT 2008
*   Change 500000us (1/2 second) timeout to zero. The 500000 must have been
*   left in during an old debugging exercise.
*
*   Brad Whitlock, Fri Mar 18 13:52:19 PDT 2011
*   Fix for Windows VisItDetectInputWithTimeout so it returns the same value
*   over and over again until the user calls the routine that handles the
*   event that we detected.
*
*******************************************************************************/

int
VisItDetectInput(int blocking, int consoleFileDescriptor)
{
    return VisItDetectInputWithTimeout(blocking, 0, consoleFileDescriptor);
}

#ifdef _WIN32
/*
 * Win32 implementation of VisItDetectInputWithTimeout.
 */
static int VisItDetectInput_return_value = 0;
static int selectThreadStarted = 0;
static int consoleThreadStarted = 0;
static WSAEVENT listenevent = 0;
static HANDLE engineevent = 0;
static HANDLE listeneventCB = 0;
static HANDLE engineeventCB = 0;

static void
VLogWindowsSocketError()
{
    switch(WSAGetLastError())
    {
    case WSANOTINITIALISED:
        fprintf(stderr,"WSAENOTINITIALISED: WSAStartup() must be called before using this API.");
        break;
    case WSAENETDOWN:
        fprintf(stderr,"WSAENETDOWN: The network subsystem or the associated service provider has failed.");
        break;
    case WSAEAFNOSUPPORT:
        fprintf(stderr,"WSAEAFNOSUPPORT: The specified address family is not supported.");
        break;
    case WSAEINPROGRESS:
        fprintf(stderr,"WSAEINPROGRESS: A blocking Winsock 1.1 call is in progress, or the service provider is still processing a callback function.");
        break;
    case WSAEFAULT:
        fprintf(stderr,"WSAEFAULT:");
        break;
    case WSAEINTR:
        fprintf(stderr,"WSAEINTR: A blocking WinSock 1.1 call was canceled via WSACancelBlockingCall.");
        break;
    case WSAEMFILE:
        fprintf(stderr,"WSAEMFILE: No more socket descriptors are available.");
        break;
    case WSAENOBUFS:
        fprintf(stderr,"WSAENOBUFS: No buffer space is available. The socket cannot be created.");
        break;
    case WSAEPROTONOSUPPORT:
        fprintf(stderr,"WSAEPROTONOSUPPORT: The specified protocol is not supported.");
        break;
    case WSAEPROTOTYPE:
        fprintf(stderr,"WSAEPROTOTYPE: The specified protocol is the wrong type for this socket.");
        break;
    case WSAESOCKTNOSUPPORT:
        fprintf(stderr,"WSAESOCKTNOSUPPORT: The specified socket type is not supported in this address family.");
        break;
    case WSAHOST_NOT_FOUND:
        fprintf(stderr,"WSAHOST_NOT_FOUND: Authoratiative Answer Host not found.");
        break;
    case WSATRY_AGAIN:
        fprintf(stderr,"WSATRY_AGAIN: Non-Authoratative Host not found, or server failure.");
        break;
    case WSANO_RECOVERY:
        fprintf(stderr,"WSANO_RECOVERY: Non-recoverable error occurred.");
        break;
    case WSANO_DATA:
        fprintf(stderr,"WSANO_DATA: Valid name, no data record of requested type.");
        break;
    case WSAEINVAL:
        fprintf(stderr,"WSAEINVAL: See documentation for: ");
        break;
    case WSAENETRESET:
        fprintf(stderr,"WSAENETRESET: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.");
        break;
    case WSAENOPROTOOPT:
        fprintf(stderr,"WSAENOPROTOOPT: The option is unknown or unsupported for the specified provider or socket.");
        break;
    case WSAENOTCONN:
        fprintf(stderr,"WSAENOTCONN: Connection has been reset when SO_KEEPALIVE is set.");
        break;
    case WSAENOTSOCK:
        fprintf(stderr,"WSAENOTSOCK: The descriptor is not a socket.");
        break;
    case WSAEADDRINUSE:
        fprintf(stderr,"WSAEADDRINUSE: The socket's local address space is already in use and the socket was not marked to allow address reuse.");
        break;
    case WSAEALREADY:
        fprintf(stderr,"WSAEALREADY: A non-blocking connect() call is in progress or the service provider is still processing a callback function.");
        break;
    case WSAEADDRNOTAVAIL:
        fprintf(stderr,"WSAADDRNOTAVAIL: The remote address is not valid (e.g. ADDR_ANY).");
        break;
    case WSAECONNREFUSED:
        fprintf(stderr,"WSAECONNREFUSED: The attempt to connect was forcefully rejected.");
        break;
    case WSAEISCONN:
        fprintf(stderr,"WSAEISCONN: The socket is already connected.");
        break;
    case WSAENETUNREACH:
        fprintf(stderr,"WSAENETUNREACH: The network can't be reached from this host at this time.");
        break;
    case WSAETIMEDOUT:
        fprintf(stderr,"WSAETIMEDOUT: Attempt to connect timed out without establishing a connection.");
        break;
    case WSAEWOULDBLOCK:
        fprintf(stderr,"WSAEWOULDBLOCK: The socket it marked as non-blocking and the connection cannot be completed immediately.");
        break;
    case WSAEACCES:
        fprintf(stderr,"WSAEACCES: Attempt to connect datagram socket to broadcast address failed.");
        break;
    case WSAEOPNOTSUPP:
        fprintf(stderr,"WSAENOTSUPP: The referenced socket is not of a type that supports listen().");
        break;
    default:
        fprintf(stderr, "WSA error: %d", WSAGetLastError());
    }
    fprintf(stderr, "\n");
}

/*
 * We run this function on another thread so select can block and then we
 * send events to the main thread which is stuck in WaitForMultipleObjects
 * inside its VisItDetectInputWithTimeout function. We use this multiple
 * thread approach so we can wait for both socket input and console input,
 * which is central to VisItDetectInput.
 */

DWORD WINAPI
select_thread(LPVOID param)
{
    while(engineSocket != VISIT_INVALID_SOCKET ||
          listenSocket != VISIT_INVALID_SOCKET)
    {
        fd_set readSet;
        int    ignored = 0, status = 0;

        FD_ZERO(&readSet);

        /* If we're connected, select on the control socket */
        if(engineSocket != VISIT_INVALID_SOCKET)
        {
            FD_SET(engineSocket, &readSet);
            /*fprintf(stderr, "select_thread: selecting engine control socket\n");*/
        }

        /* If we're connected, do *not* select on the listen socket */
        /* This forces us to have only one client at a time. */
        if (engineSocket == VISIT_INVALID_SOCKET &&
            listenSocket != VISIT_INVALID_SOCKET)
        {
            FD_SET(listenSocket, &readSet);
/*            fprintf(stderr, "select_thread: selecting listen socket for inbound connections\n");*/
        }

/*        fprintf(stderr, "select_thread: calling select\n");*/
        status = select(ignored, &readSet, (fd_set*)NULL,
                        (fd_set*)NULL, NULL);

        if(status == SOCKET_ERROR)
        {
            VLogWindowsSocketError();
        }
        else if(status > 0)
        {
             if (listenSocket != VISIT_INVALID_SOCKET &&
                 FD_ISSET(listenSocket, &readSet))
             {
                 SetEvent(listenevent);

                 /* wait for it to be done. */
                 WaitForSingleObject(listeneventCB, INFINITE);
             }
             else if (engineSocket != VISIT_INVALID_SOCKET &&
                      FD_ISSET(engineSocket, &readSet))
             {
                 SetEvent(engineevent);

                 /* wait for it to be done. */
                 WaitForSingleObject(engineeventCB, INFINITE);
             }
        }
    }

    return 0;
}

/*
 * Wait for input from the sockets or console on a specified timeout interval.
 */
int
VisItDetectInputWithTimeout(int blocking, int timeoutVal,
    int consoleFileDescriptor)
{
    HANDLE handles[3];
    int n0, n, msec;

    LIBSIM_API_ENTER2(VisItDetectInput, "blocking=%d, consoleFile=%d",
                      blocking, consoleFileDescriptor);

    msec = timeoutVal / 1000;

    if(!selectThreadStarted)
    {
        DWORD select_threadid;

        listenevent = CreateEvent(NULL, FALSE, FALSE, NULL);
        engineevent = CreateEvent(NULL, FALSE, FALSE, NULL);
        listeneventCB = CreateEvent(NULL, FALSE, FALSE, NULL);
        engineeventCB = CreateEvent(NULL, FALSE, FALSE, NULL);

        /*fprintf(stderr, "Creating socket input thread\n");*/
        if(!CreateThread(NULL, 0, select_thread,
                         NULL, 0, &select_threadid))
        {
            LIBSIM_API_LEAVE1(VisItDetectInput,
                              "Unable to create socket input thread. return %d",
                              -4);
            return -4;
        }

        selectThreadStarted = 1;
    }

    /* If the last call to VisItDetectInput returned 1 or 2 then we need to
     * return those values again if VisItDetectInput_return_value is still
     * set to those values. It means that the user called VisItDetectInput
     * again without first calling the routines that clear this flag
     * (VisItAttemptToCompleteConnection, VisItProcessEngineCommand).
     */
    if(VisItDetectInput_return_value == 1 ||
       VisItDetectInput_return_value == 2)
    {
        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "return previous value %d",
                          VisItDetectInput_return_value);
        return VisItDetectInput_return_value;
    }

    /* Wait for any of these events to occur. */
waitforevents:
    n0 = 2;
    handles[0] = listenevent;
    handles[1] = engineevent;
    if(consoleFileDescriptor >= 0)
    {
        handles[2] = GetStdHandle(STD_INPUT_HANDLE);
        n0++;
    }

    n = WaitForMultipleObjects(n0, handles, FALSE, blocking ? INFINITE : msec);

    if(n == WAIT_TIMEOUT)
    {
        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "Okay - Timed out. return %d",
                          0);
        VisItDetectInput_return_value = 0;
    }
    else if(n == WAIT_OBJECT_0)
    {
        /* We received an event on the listen socket. If it's an event
         * indicating that there's input then tell the user it's okay
         * to read from the listen socket.
         */
        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "WAIT_OBJECT_0: Listen socket input. return %d",
                          1);
        VisItDetectInput_return_value = 1;
    }
    else if(n == WAIT_OBJECT_0+1)
    {
        /* we received an event for the engine socket. If it's an event
         * indicating that there's input then tell the user it's okay
         * to read from the engine socket.
         */
        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "WAIT_OBJECT_0+1: Engine socket input. return %d",
                          2);
        VisItDetectInput_return_value = 2;
    }
    else if(n == WAIT_OBJECT_0+2)
    {
        /* We received a stdin event but it might not be a key event.
         * Peek at the events to look for key presses. If we find some,
         * tell the client that it's okay to read from the console.
         */
        DWORD nEvents = 0;
        INPUT_RECORD input[100];
        int retval = 0; /* no input timeout */
        if(PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE),
           input, 100, &nEvents))
        {
            DWORD j;
            for(j = 0; j < nEvents; ++j)
            {
                if(input[j].EventType == KEY_EVENT)
                {
                    retval = 3; /* read console input */
                    break;
                }
            }
        }
        /* We did not have any real key events so go back to waiting for them. */
        if(retval != 3)
        {
            /* Gobble up the junk events so we don't keep getting them. */
            DWORD maxevents = nEvents;
            ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE),
                             input, maxevents, &nEvents);
            goto waitforevents;
        }

        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "WAIT_OBJECT_0+2: Console input. return %d",
                          3);
        VisItDetectInput_return_value = 3;
    }
    else
    {
        VisItDetectInput_return_value = -5;

        LIBSIM_API_LEAVE1(VisItDetectInput,
                          "Unspecified error. return %d",
                          -5);
    }

    return VisItDetectInput_return_value;
}
#else

#ifdef VISIT_BLUE_GENE_Q
static int VisItDetectInputWithTimeout_bgq_ncalls = 0;

int
VisItDetectInputWithTimeout(int blocking, int timeoutVal,
    int consoleFileDescriptor)
{
    int retval = 1; /* Listen socket input */
    if(++VisItDetectInputWithTimeout_bgq_ncalls > 1)
        retval = 2; /* Engine socket input */
    return retval;
}
#else
int
VisItDetectInputWithTimeout(int blocking, int timeoutVal,
    int consoleFileDescriptor)
{
   /*  RETURN CODES:
      -5: Logic error (fell through all cases)
      -4: Logic error (no descriptors but blocking)
      -3: Logic error (a socket was selected but not one we set)
      -2: Unknown error in select
      -1: Interrupted by EINTR in select
       0: Okay - Timed out
       1: Listen  socket input
       2: Engine  socket input
       3: Console socket input
   */
   int maxDescriptor = MAX(MAX(consoleFileDescriptor,
                               engineSocket),
                               listenSocket);

   fd_set readSet;
   int status = 0;
   struct timeval TimeoutValue = {0,0};
   struct timeval *timeout = (blocking ? NULL : &TimeoutValue);
   TimeoutValue.tv_sec = timeoutVal / 1000000;
   TimeoutValue.tv_usec = timeoutVal - (TimeoutValue.tv_sec * 1000000);

   LIBSIM_API_ENTER2(VisItDetectInput, "blocking=%d, consoleFile=%d",
                     blocking, consoleFileDescriptor);

   if (consoleFileDescriptor < 0 &&
       engineSocket <= VISIT_INVALID_SOCKET &&
       listenSocket <= VISIT_INVALID_SOCKET)
   {
      if (blocking)
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Logic error (no descriptors but blocking). return %d",
                           -4);
         return -4;
      }
      else
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Okay - Timed out. return %d",
                           0);
         return 0;
      }
   }

   FD_ZERO(&readSet);

   /* If we were told to, select on the console socket */
   if (consoleFileDescriptor >= 0)
      FD_SET(consoleFileDescriptor, &readSet);

   /* If we're connected, select on the control socket */
   if (engineSocket >= 0)
      FD_SET(engineSocket, &readSet);

   /* If we're connected, do *not* select on the listen socket */
   /* This forces us to have only one client at a time. */
   if (engineSocket < 0 &&
       listenSocket >= 0)
      FD_SET(listenSocket, &readSet);

   status = select(maxDescriptor+1,
                   &readSet, (fd_set*)NULL, (fd_set*)NULL, timeout);

   if (status < 0)
   {
      if (errno == EINTR)
      {
         /* Interruption will likely cause a program exit anyway */
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Interrupted by EINTR in select. return %d",
                           -1);
         return -1;
      }
      else if (errno == EBADF)
      {
         /* Interruption will likely cause a program exit anyway */
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Invalid file descriptor given to select. return %d",
                           -1);
         return -1;
      }
      else
      {
         /* This should never happen; internal error */
         LIBSIM_API_LEAVE2(VisItDetectInput,
                           "Error \"%s\" in select. return %d",
                           strerror(errno), -2);
         return -2;
      }
   }
   else if (status == 0)
   {
      /* We timed out */
      LIBSIM_API_LEAVE1(VisItDetectInput,
                        "Okay - Timed out. return %d",
                        0);
      return 0;
   }
   else
   {
      /* Determine if it's a new connection attempt or
         commands from an existing one */
      if (listenSocket >= 0 &&
          FD_ISSET(listenSocket, &readSet))
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Listen  socket input. return %d",
                           1);
         return 1;
      }
      else if (engineSocket >= 0 &&
               FD_ISSET(engineSocket, &readSet))
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Engine  socket input. return %d",
                           2);
         return 2;
      }
      else if (consoleFileDescriptor >= 0 &&
               FD_ISSET(consoleFileDescriptor, &readSet))
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Console socket input. return %d",
                           3);
         return 3;
      }
      else
      {
         LIBSIM_API_LEAVE1(VisItDetectInput,
                           "Logic error (a socket was selected but not one we set). return %d",
                           -3);
         return -3;
      }
   }
   /*return -5; Compilers complain because this line cannot be reached */
}
#endif
#endif

/*******************************************************************************
*
* Name: VisItReadConsole
*
* Purpose: Read at most maxlen characters from the console.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/

int
VisItReadConsole(int maxlen, char *buffer)
{
    int retval = VISIT_OKAY;

    LIBSIM_API_ENTER(VisItReadConsole);

    if(buffer != NULL && maxlen > 0)
    {
#ifdef _WIN32
        DWORD mlen = maxlen, nread = 0, nEvents = 0;
        memset(buffer, 0, maxlen);

        /* This blocks so we may need to switch at some point to a ReadConsoleInput*/
        if(ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buffer, mlen, &nread, NULL))
        {
            /* Strip end of line */
            if(nread > 0)
            {
                buffer[nread-1] = '\0';
                buffer[nread-2] = '\0';
            }
        }
        else
            retval = VISIT_ERROR;
#else
        if(fgets(buffer, 1000, stdin) == NULL)
            retval = VISIT_ERROR;
        else
        {
            /* Strip end of line */
            int len = strlen(buffer);
            if(len > 0 && buffer[len-1] == '\n')
                buffer[len-1] = '\0';
        }
#endif
    }

    LIBSIM_API_LEAVE(VisItReadConsole);
    return retval;
}

#ifndef _WIN32
/*******************************************************************************
*
* Name: VisItGetSockets
*
* Purpose: Get the sockets that we should be select()'ing on.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/
int
VisItGetSockets(VISIT_SOCKET *lSocket, VISIT_SOCKET *cSocket)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItGetSockets);

    if(lSocket != NULL && cSocket != NULL)
    {
        /* If we're connected, select on the control socket */
        *cSocket = (engineSocket >= 0) ? engineSocket : VISIT_INVALID_SOCKET;

        /* If we're connected, do *not* select on the listen socket */
        /* This forces us to have only one client at a time. */
        if (engineSocket < 0 && listenSocket >= 0)
            *lSocket = listenSocket;
        else
            *lSocket = VISIT_INVALID_SOCKET;

        retval = VISIT_OKAY;
    }

    LIBSIM_API_LEAVE(VisItGetSockets);

    return retval;
}
#endif

/*******************************************************************************
*
* Name: VisItAttemptToCompleteConnection
*
* Purpose: Accept the socket, verify security keys, get the connection
*          parameters from the client, load the VisIt engine library,
*          create the Engine and connect to the viewer.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 15:54:14 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Thu Mar 26 13:38:50 PDT 2009
*   Install our command callback handler.
*
*   Brad Whitlock, Fri Nov 26 00:23:34 PDT 2010
*   Add Windows code
*
*   Brad Whitlock, Fri Mar 18 13:49:01 PDT 2011
*   Fix for Windows VisItDetectInput.
*
*   Brad Whitlock, Fri Sep 28 14:30:44 PDT 2012
*   I split engine creation and viewer connection. Then I grouped runtime
*   library loading and engine creation.
*
*******************************************************************************/
int VisItAttemptToCompleteConnection(void)
{
    VISIT_SOCKET socket = -1; //TODO: verify initialization is safe

    LIBSIM_API_ENTER(VisItAttemptToCompleteConnection);

    /* wait for a connection -- only process 0 does this */
    if (parallelRank == 0)
    {
        socket = AcceptConnection();

        if (socket < 0)
        {
            LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection,
                              "socket<0, return %d", VISIT_ERROR);
            return VISIT_ERROR;
        }
    }

    /* verify security keys */
    if (!VerifySecurityKeys(socket))
    {
        LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection,
                          "VerifySecurityKeys failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    /* get the connection parameters */
    if (!GetConnectionParameters(socket))
    {
        LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection,
                          "GetConnectionParameters failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    /* Initialize engine */
    if (InitializeRuntime(0) == VISIT_ERROR)
    {
        LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection,
                          "InitializeRuntime failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    /* connect to the viewer. */
    if (ConnectToViewer() == VISIT_ERROR)
    {
        LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection,
                          "ConnectToViewer failed. return %d", VISIT_ERROR);
        return VISIT_ERROR;
    }

    /* get the socket for listening from the viewer */
    if (parallelRank == 0)
    {
        LIBSIM_MESSAGE("  Calling simv2_get_descriptor");
        engineSocket = callbacks->control.get_descriptor(engine);
        LIBSIM_MESSAGE1("simv2_get_descriptor returned %d", (int)engineSocket);
#ifdef _WIN32
        /* Clear the value from VisItDetectInput so it can return a new value. */
        if(VisItDetectInput_return_value == 1)
            VisItDetectInput_return_value = 0;

        /* Send an event back to the select_thread telling it that it is okay
         * to proceed.
         */
        SetEvent(listeneventCB);
#endif
    }

    /* Install our local command callback handler so we can monitor the
     * callbacks before handing them off to the user's command callback. This
     * lets us implement synchronization for the user.
     */
    if(callbacks->control.set_command_callback != NULL)
        (*callbacks->control.set_command_callback)(engine, visit_handle_command_callback, NULL);

    LIBSIM_API_LEAVE1(VisItAttemptToCompleteConnection, "return %d", VISIT_OKAY);
    return VISIT_OKAY;
}

/*******************************************************************************
*
* Name: VisItSetSlaveProcessCallback
*
* Purpose: Set the callback to inform slave processes that they should
*          call VisItProcessEngineCommand.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Fri Jul 25 15:55:53 PDT 2008
*   Trace information.
*
*   Brad Whitlock, Thu Jan 27 15:13:25 PST 2011
*   Store off the slave process callback because we'll need to install a new
*   one during synchronization in case the user has a funky callback.
*
*******************************************************************************/
void VisItSetSlaveProcessCallback(void (*spic)(void))
{
    LIBSIM_API_ENTER1(VisItSetSlaveProcessCallback, "spic=%p", (void*)spic);
    LIBSIM_MESSAGE("  Calling simv2_set_slave_process_callback");
    if(callbacks != NULL && callbacks->control.set_slave_process_callback)
    {
        visit_slave_process_callback = spic;
        visit_slave_process_callback2 = NULL;
        visit_slave_process_callback2_data = NULL;
        (*callbacks->control.set_slave_process_callback)(spic);
    }
    LIBSIM_API_LEAVE(VisItSetSlaveProcessCallback);
}

/*******************************************************************************
*
* Name: VisItSetSlaveProcessCallback2
*
* Purpose: Set the callback to inform slave processes that they should
*          call VisItProcessEngineCommand. This version lets us pass callback
*          function data.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/

static void
visit_slave_process_callback2_thunk(void)
{
    if(visit_slave_process_callback2 != NULL)
        (*visit_slave_process_callback2)(visit_slave_process_callback2_data);
}

void VisItSetSlaveProcessCallback2(void (*spic)(void *), void *spicdata)
{
    LIBSIM_API_ENTER1(VisItSetSlaveProcessCallback2, "spic=%p", (void*)spic);
    LIBSIM_MESSAGE("  Calling simv2_set_slave_process_callback");
    if(callbacks != NULL && callbacks->control.set_slave_process_callback)
    {
        visit_slave_process_callback = NULL;
        visit_slave_process_callback2 = spic;
        visit_slave_process_callback2_data = spicdata;
        (*callbacks->control.set_slave_process_callback)(visit_slave_process_callback2_thunk);
    }
    LIBSIM_API_LEAVE(VisItSetSlaveProcessCallback2);
}

/*******************************************************************************
*
* Name: VisItSetCommandCallback
*
* Purpose: Set the callback for processing control commands.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:56:41 PDT 2008
*  Trace information.
*
*******************************************************************************/
void VisItSetCommandCallback(void (*scc)(const char*,const char*,void*),
    void *sccdata)
{
    LIBSIM_API_ENTER1(VisItSetCommandCallback, "scc=%p", (void*)scc);
    LIBSIM_MESSAGE("  Calling simv2_set_command_callback");

    visit_command_callback = scc;
    visit_command_callback_data = sccdata;

    LIBSIM_API_LEAVE(VisItSetCommandCallback);
}

/*******************************************************************************
*
* Name: VisItProcessEngineCommand
*
* Purpose: Process requests coming from the client.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 15:56:41 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Fri Nov 26 00:25:24 PDT 2010
*  Add Windows code.
*
*  Brad Whitlock, Fri Mar 18 13:50:23 PDT 2011
*  Fix for Windows VisItDetectInput.
*
*******************************************************************************/
int VisItProcessEngineCommand(void)
{
    int retval = VISIT_ERROR;
    LIBSIM_API_ENTER(VisItProcessEngineCommand);
    if (!engine)
    {
        LIBSIM_MESSAGE("engine == NULL");
        retval = VISIT_OKAY;
    }
    else if (callbacks != NULL)
    {
        LIBSIM_MESSAGE("  Calling simv2_process_input");
#ifdef _WIN32
        if(parallelRank == 0)
        {
            /* Clear the value from VisItDetectInput so it can return a new value. */
            if(VisItDetectInput_return_value == 2)
                VisItDetectInput_return_value = 0;

            /* Send an event back to select_thread telling it that it is okay to
             * proceed
             */
            SetEvent(engineeventCB);
        }
#endif
        retval = ((*callbacks->control.process_input)(engine) == 1) ?
            VISIT_OKAY : VISIT_ERROR;
        LIBSIM_MESSAGE1("simv2_process_input returned: %d", retval);
    }

    LIBSIM_API_LEAVE1(VisItProcessEngineCommand, "return %s", ErrorToString(retval));
    return retval;
}

/*******************************************************************************
*
* Name: VisItTimeStepChanged
*
* Purpose: Tell VisIt a new time step is ready.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 16:00:32 PDT 2008
*  Trace information.
*
*******************************************************************************/
void VisItTimeStepChanged(void)
{
    LIBSIM_API_ENTER(VisItTimeStepChanged);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.time_step_changed)
    {
        LIBSIM_MESSAGE("  Calling simv2_time_step_changed");
        (*callbacks->control.time_step_changed)(engine);
    }
    LIBSIM_API_LEAVE(VisItTimeStepChanged);
}

/*******************************************************************************
*
* Name: VisItUpdatePlots
*
* Purpose: Tell VisIt to update its plots.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 16:00:32 PDT 2008
*  Trace information.
*
*******************************************************************************/
void VisItUpdatePlots(void)
{
    LIBSIM_API_ENTER(VisItUpdatePlots);

    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        LIBSIM_MESSAGE("  Calling simv2_execute_command: UpdatePlots");
        (*callbacks->control.execute_command)(engine, "UpdatePlots");

        if(visit_sync_enabled)
            VisItSynchronize();
    }
    LIBSIM_API_LEAVE(VisItUpdatePlots);
}

/*******************************************************************************
*
* Name: VisItExecuteCommand
*
* Purpose: Tell VisIt to execute a command.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 16:00:32 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Mon Aug  8 10:51:11 PDT 2011
*  NULL checks.
*
*******************************************************************************/
void VisItExecuteCommand(const char *command)
{
    LIBSIM_API_ENTER(VisItExecuteCommand);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command &&
        command != NULL)
    {
        LIBSIM_MESSAGE("  Calling simv2_execute_command");
        char *cmd2 = (char *) malloc((strlen(command) + 1 + 10) * sizeof(char));
        if(cmd2 != NULL)
        {
            sprintf(cmd2, "Interpret:%s", command);
            (*callbacks->control.execute_command)(engine, cmd2);
            free(cmd2);
        }

        if(visit_sync_enabled)
            VisItSynchronize();
    }
    LIBSIM_API_LEAVE(VisItExecuteCommand);
}

/*******************************************************************************
*
* Name: VisItDisconnect
*
* Purpose: Disconnect from the client.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 16:00:32 PDT 2008
*  Trace information.
*
*  Brad Whitlock, Fri Feb 13 09:45:14 PST 2009
*  Delete the callbacks structure and set it to NULL.
*
*******************************************************************************/
void VisItDisconnect(void)
{
    LIBSIM_API_ENTER(VisItDisconnect);
    LIBSIM_MESSAGE("  Calling simv2_disconnect");
    if(callbacks != NULL)
    {
        if(callbacks->control.disconnect)
            (*callbacks->control.disconnect)();

        free(callbacks);
        callbacks = NULL;
    }
    engineSocket = VISIT_INVALID_SOCKET;
    engine = NULL;
#ifdef _WIN32
    selectThreadStarted = 0;
#endif
#ifdef VISIT_BLUE_GENE_Q
    VisItDetectInputWithTimeout_bgq_ncalls = 0;
#endif
    viewer_connected = 0;

    CloseVisItLibrary();
    LIBSIM_API_LEAVE(VisItDisconnect);
}

/*******************************************************************************
*
* Name: VisItIsConnected
*
* Purpose: Returns whether VisIt is connected
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*   Brad Whitlock, Thu Nov 10 23:38:27 PST 2011
*   Rewrote.
*
*******************************************************************************/
int VisItIsConnected(void)
{
    return viewer_connected;
}

/*******************************************************************************
*
* Name: VisItIsRuntimeLoaded
*
* Purpose: Returns whether the Libsim runtime is loaded.
*
* Author: Brad Whitlock
*
* Modifications:
*
*******************************************************************************/
int VisItIsRuntimeLoaded(void)
{
    return libsim_runtime_loaded;
}

/*******************************************************************************
*
* Name: VisItSaveImage
*
* Purpose: Tell VisIt to save the active network as an image.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/
int VisItSaveWindow(const char *filename, int w, int h, int format)
{
    int ret = VISIT_ERROR;
    LIBSIM_API_ENTER(VisItSaveWindow);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.save_window)
    {
        LIBSIM_MESSAGE("  Calling simv2_save_window");
        ret = (*callbacks->control.save_window)(engine, filename, w, h, format);
        /* Synchronize in case we we're connected interactively. */
        if(visit_batch_mode == 0 && visit_sync_enabled)
            VisItSynchronize();
    }
    LIBSIM_API_LEAVE1(VisItSaveWindow, "return %s", ErrorToString(ret));
    return ret;
}

/*******************************************************************************
*
* Name: VisItGetLastError
*
* Purpose: Returns the last error message generated.
*
* Author: Jeremy Meredith, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*  Brad Whitlock, Fri Jul 25 16:00:32 PDT 2008
*  Trace information.
*
*******************************************************************************/
char *VisItGetLastError()
{
    char *err = NULL;

    LIBSIM_API_ENTER(VisItGetLastError);
    err = strdup(lastError);
    strcpy(lastError, "");
    LIBSIM_API_LEAVE1(VisItGetLastError, "return %s", err);
    return err;
}

/******************************************************************************
*
* Name: VisItOpenTraceFile
*
* Purpose: Opens the trace file that libsim will use.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  9 10:01:08 PST 2009
*
* Modifications:
*
******************************************************************************/
void VisItOpenTraceFile(const char *filename)
{
    LIBSIM_API_ENTER(VisItOpenTraceFile);
    if(filename != NULL)
        simv2_set_trace_file(fopen(filename, "wt"));
    LIBSIM_API_LEAVE(VisItOpenTraceFile);
}

/******************************************************************************
*
* Name: VisItCloseTraceFile
*
* Purpose: Opens the trace file that libsim will use.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  9 10:01:08 PST 2009
*
* Modifications:
*
******************************************************************************/
void VisItCloseTraceFile(void)
{
    LIBSIM_API_ENTER(VisItCloseTraceFile);
    simv2_set_trace_file(NULL);
    LIBSIM_API_LEAVE(VisItCloseTraceFile);
}

/******************************************************************************
*
* Name: VisItSet*
*
* Purpose: Define the bodies of all of the functions that let the user
*          register their libsim callback function.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  9 10:01:08 PST 2009
*
* Modifications:
*
******************************************************************************/

#define VISIT_SET_CALLBACK_BODY(F, FR, FA) \
int VisItSet##F(FR (*cb) FA, void *cbdata) \
{ \
    int retval = VISIT_ERROR; \
    LIBSIM_API_ENTER(VisItSet##F);\
    if (engine && callbacks != NULL && callbacks->data.set_##F)\
    {\
        LIBSIM_MESSAGE("  Calling simv2_set_"#F);\
        (*callbacks->data.set_##F)(cb, cbdata);\
        retval = VISIT_OKAY; \
    }\
    else\
        fprintf(stderr, "VisItSet"#F" should not be called until VisIt connects to the simulation.\n");\
    LIBSIM_API_LEAVE(VisItSet##F); \
    return retval;\
}

DECLARE_DATA_CALLBACKS(VISIT_SET_CALLBACK_BODY)

/******************************************************************************
*
* Name: VisItSynchronize
*
* Purpose: Sends a synchronize message to VisIt if we're connected to VisIt
*          and spawns a sub-eventloop to wait for the sync message to be
*          returned from VisIt. This permits us to write code in the sim that
*          blocks until it's done and still processes events from VisIt needed
*          to get us to the point where the message gets read.
*
* Note:    This function has not been tried for sims that use polling.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*   Brad Whitlock, Thu Jan 27 15:22:00 PST 2011
*   The visit_process_engine_command function we use internally assumes that
*   messages that tell slave processes what to do are ints. We must install
*   our own int-based slave process callback to ensure that we're broadcasting
*   ints since the sim's slave process callback can do whatever it wants.
*
*   Brad Whitlock, Tue Apr 17 11:05:08 PDT 2012
*   Account for different style slave process callback.
*
******************************************************************************/

static void
visit_sync_helper(void *cbdata)
{
    int *syncing = (int *)cbdata;
    *syncing = 0;
}

static void
visit_sync_slave_process_callback(void)
{
    int command = VISIT_COMMAND_PROCESS;
    BroadcastInt(&command, 0);
}

int
VisItSynchronize(void)
{
    int blocking = 1;
    int syncing = 1;
    int visitstate = 0, err = 0;
    void (*sim_spc)(void) = visit_slave_process_callback;
    void (*sim_spc2)(void*) = visit_slave_process_callback2;
    void *sim_spc2data = visit_slave_process_callback2_data;

    LIBSIM_API_ENTER(VisItSynchronize);

    if(!VisItIsConnected())
    {
        LIBSIM_API_LEAVE(VisItSynchronize);
        return VISIT_OKAY;
    }

    /* Send a sync to the viewer. When we get it back the loop will end. */
    visit_add_sync(visit_sync_helper, &syncing);

    /* Save the sim's slave process callback and install a new one temporarily. */
    VisItSetSlaveProcessCallback(visit_sync_slave_process_callback);

    do
    {
        /* Get input from VisIt. */
        if(parallelRank == 0)
            visitstate = VisItDetectInput(blocking, -1);
        BroadcastInt(&visitstate, 0);

        /* Do different things depending on the output from VisItDetectInput. */
        if(visitstate >= -5 && visitstate <= -1)
        {
            fprintf(stderr, "Can't recover from error!\n");
            err = 1;
        }
        else if(visitstate == 0)
        {
            /* There was no input from VisIt. We're blocking so this should not happen. */
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to the sim. We're already connected
             * so this can't happen.
             */
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            if(!visit_process_engine_command())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                err = 1;
            }
        }
        else if(visitstate == 3)
        {
            /* We're not trapping for console input. */
        }
    } while(syncing && err == 0);

    /* Restore the sim's slave process callback. */
    if(sim_spc != NULL)
        VisItSetSlaveProcessCallback(sim_spc);
    else
        VisItSetSlaveProcessCallback2(sim_spc2, sim_spc2data);

    LIBSIM_API_LEAVE(VisItSynchronize);
    return (err==0) ? VISIT_OKAY : VISIT_ERROR;
}

/******************************************************************************
*
* Name: VisItEnableSynchronize
*
* Purpose: This method lets us turn off automatic synchronization for functions
*          that communicate with VisIt's viewer.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*
******************************************************************************/

void
VisItEnableSynchronize(int mode)
{
    visit_sync_enabled = mode ? 1 : 0;
}

/******************************************************************************
*
* Name: VisItDebug*
*
* Purpose: Let the user write to the VisIt debug logs.
*
* Programmer: Brad Whitlock
* Date:       Thu Mar 26 13:28:11 PDT 2009
*
* Modifications:
*
******************************************************************************/

#define DBG_BUFFER_SIZE 1000
#define VISIT_DEBUG_FUNCTION(N)\
void \
VisItDebug##N(const char *format, ...)\
{\
    char buffer[DBG_BUFFER_SIZE]; \
    va_list args; \
    va_start(args, format); \
    vsnprintf(buffer, DBG_BUFFER_SIZE, format, args);\
    va_end(args);\
    if(engine && callbacks != NULL && callbacks->control.debug_logs != NULL)\
        (*callbacks->control.debug_logs)(N, buffer);\
}

VISIT_DEBUG_FUNCTION(1)
VISIT_DEBUG_FUNCTION(2)
VISIT_DEBUG_FUNCTION(3)
VISIT_DEBUG_FUNCTION(4)
VISIT_DEBUG_FUNCTION(5)

/******************************************************************************
*
* Name: VisItSetMPICommunicator
*
* Purpose: Let the user set the MPI communicator that VisIt will use.
*
* Programmer: Brad Whitlock
* Date:       Fri Aug 26 09:47:53 PDT 2011
*
* Modifications:
*
******************************************************************************/

int
VisItSetMPICommunicator(void *comm)
{
    int retval = VISIT_OKAY;
    LIBSIM_API_ENTER(VisItSetMPICommunicator);
    visit_communicator = comm;
    if(engine && callbacks != NULL && callbacks->control.set_mpicomm != NULL)
    {
        LIBSIM_MESSAGE("  Calling simv2_set_mpicomm");
        retval = (*callbacks->control.set_mpicomm)(comm);
    }
    LIBSIM_API_LEAVE1(VisItSetMPICommunicator, "return %s", ErrorToString(retval));
    return retval;
}

/******************************************************************************
*
* Name: VisItSetMPICommunicator_f
*
* Purpose: Let the Fortran user set the MPI communicator that VisIt will use.
*
* Programmer: William T. Jones
* Date:       Fri Jul 12 18:13:24 EDT 2013
*
* Modifications:
*
******************************************************************************/

int
VisItSetMPICommunicator_f(int *comm)
{
    int retval = VISIT_OKAY;
    LIBSIM_API_ENTER(VisItSetMPICommunicator_f);
    visit_communicator_f = comm;
    if(engine && callbacks != NULL && callbacks->control.set_mpicomm_f != NULL)
    {
        LIBSIM_MESSAGE("  Calling simv2_set_mpicomm_f");
        retval = (*callbacks->control.set_mpicomm_f)(comm);
    }
    LIBSIM_API_LEAVE1(VisItSetMPICommunicator_f, "return %s", ErrorToString(retval));
    return retval;
}

/******************************************************************************
*
* Name: VisItInitializeRuntime
*
* Purpose: Explicitly load the VisIt runtime library.
*
* Programmer: Brad Whitlock
* Date:       2013
*
* Modifications:
*
******************************************************************************/

int
VisItInitializeRuntime(void)
{
    int retval = VISIT_ERROR;
    LIBSIM_API_ENTER(VisItInitializeRuntime);
    retval = InitializeRuntime(1);
    LIBSIM_API_LEAVE1(VisItInitializeRuntime, "return %s", ErrorToString(retval));
    return retval;
}

/******************************************************************************
*
* Name: VisItGetMemory
*
* Purpose: Measure the memory being used on the current processor.
*
* Programmer: Satheesh Maheswaran
* Date:       2012
*
* Modifications:
*
******************************************************************************/

int
VisItGetMemory(double *m_size, double *m_rss)
{
  int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItGetMemory);
    /* Make sure the function exists before using it. - not sure if this is required, need to talk to
     Brad */
    /*
    if (engine && callbacks != NULL && callbacks->control.delete_plot)
    {
        retval = (*callbacks->control.delete_plot)(engine, plotID);
    }
    */
    *m_size = 0.0;
    *m_rss = 0.0;
#if defined(__APPLE__)
    struct mstats m = mstats();
    *m_size = (unsigned long)m.bytes_used; // The bytes used out of the bytes_total.
    *m_rss = (unsigned long)m.bytes_total; // not quite accurate but this should be the total
                                           // amount allocated by malloc.
#elif !defined(_WIN32)
    unsigned long tmp1, tmp2;
    FILE *file = fopen("/proc/self/statm", "r");
    if (file == NULL)
    {
        return retval;
    }
    int count = fscanf(file, "%lu%lu", &tmp1, &tmp2);
    *m_size = (double)tmp1;
    *m_rss = (double)tmp2;
    if (count != 2)
    {
        fclose(file);
        return retval;
    }
    *m_size *= (double)getpagesize();
    *m_rss  *= (double)getpagesize();
    fclose(file);
#endif
    //Convert to megabytes
    *m_size /= 1048576.0;
    *m_rss /=  1048576.0;
    LIBSIM_API_LEAVE(VisItGetMemory)
    return retval;
}


/***************************************************************************

                                  UI CODE

****************************************************************************/
int
VisItUI_clicked(const char *name, void (*cb)(void*), void *cbdata)
{
    int retval = VISIT_ERROR;
    sim_ui_element *e = NULL;
    LIBSIM_API_ENTER(VisItUI_clicked);
    if((e = sim_ui_get(name)) != NULL)
    {
        e->slot_clicked = cb;
        e->slot_clicked_data = cbdata;
        retval = VISIT_OKAY;
    }
    LIBSIM_API_LEAVE(VisItUI_clicked);
    return retval;
}

int
VisItUI_stateChanged(const char *name, void (*cb)(int,void*), void *cbdata)
{
    int retval = VISIT_ERROR;
    sim_ui_element *e = NULL;
    LIBSIM_API_ENTER(VisItUI_stateChanged);
    if((e = sim_ui_get(name)) != NULL)
    {
        e->slot_stateChanged = cb;
        e->slot_stateChanged_data = cbdata;
        retval = VISIT_OKAY;
    }
    LIBSIM_API_LEAVE(VisItUI_stateChanged);
    return retval;
}

int
VisItUI_valueChanged(const char *name, void (*cb)(int,void*), void *cbdata)
{
    int retval = VISIT_ERROR;
    sim_ui_element *e = NULL;
    LIBSIM_API_ENTER(VisItUI_valueChanged);
    if((e = sim_ui_get(name)) != NULL)
    {
        e->slot_valueChanged = cb;
        e->slot_valueChanged_data = cbdata;
        retval = VISIT_OKAY;
    }
    LIBSIM_API_LEAVE(VisItUI_valueChanged);
    return retval;
}

int
VisItUI_textChanged(const char *name, void (*cb)(char*,void*), void *cbdata)
{
    int retval = VISIT_ERROR;
    sim_ui_element *e = NULL;
    LIBSIM_API_ENTER(VisItUI_textChanged);
    if((e = sim_ui_get(name)) != NULL)
    {
        e->slot_textChanged = cb;
        e->slot_textChanged_data = cbdata;
        retval = VISIT_OKAY;
    }
    LIBSIM_API_LEAVE(VisItUI_textChanged);
    return retval;
}

int
VisItUI_cellChanged(const char *name, void (*cb)(char*,void*), void *cbdata)
{
    int retval = VISIT_ERROR;
    sim_ui_element *e = NULL;
    LIBSIM_API_ENTER(VisItUI_cellChanged);
    if((e = sim_ui_get(name)) != NULL)
    {
        e->slot_cellChanged = cb;
        e->slot_cellChanged_data = cbdata;
        retval = VISIT_OKAY;
    }
    LIBSIM_API_LEAVE(VisItUI_cellChanged);
    return retval;
}

int
VisItUI_setValueI(const char *name, int value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setValueI);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d:%d", name, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setValueI)
    return retval;
}

int
VisItUI_setValueD(const char *name, double value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setValueD);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%f:%d", name, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setValueD)
    return retval;
}

int
VisItUI_setValueV(const char *name, double x, double y, double z, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setValueV);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%f,%f,%f:%d", name, x, y, z, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setValueV)
    return retval;
}

int
VisItUI_setValueS(const char *name, const char *value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setValueS);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + strlen( value ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%s:%d", name, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setValueS)
    return retval;
}


int
VisItUI_setListValueI(const char *name,
                      int row, int value, int editable, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setListValueI);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %d :%d",
                name, row, value, editable?1:0, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setListValueI)
    return retval;
}

int
VisItUI_setListValueD(const char *name,
                      int row, double value, int editable, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setListValueD);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %f | %d :%d",
                name, row, value, editable?1:0, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setListValueD)
    return retval;
}

int
VisItUI_setListValueV(const char *name,
                      int row, double x, double y, double z, int editable, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setListValueV);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %f,%f,%f | %d :%d",
                name, row, x, y, z, editable?1:0, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setListValueV)
    return retval;
}

int
VisItUI_setListValueS(const char *name,
                      int row, const char *value, int editable, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setListValueS);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + strlen( value ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %s | %d :%d",
                name, row, value, editable?1:0, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setListValueS)
    return retval;
}


int
VisItUI_setTableValueI(const char *name,
                       int row, int column, int value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setTableValueI);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %d :%d",
                name, row, column, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setTableValueI)
    return retval;
}

int
VisItUI_setTableValueD(const char *name,
                       int row, int column, double value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setTableValueD);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %f :%d",
                name, row, column, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setTableValueD)
    return retval;
}

int
VisItUI_setTableValueV(const char *name,
                       int row, int column, double x, double y, double z, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setTableValueV);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %f,%f,%f :%d",
                name, row, column, x, y, z, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setTableValueV)
    return retval;
}

int
VisItUI_setTableValueS(const char *name,
                       int row, int column, const char *value, int enabled)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_setTableValueS);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const unsigned int nChars = strlen( name ) + strlen( value ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %s :%d",
                name, row, column, value, enabled?1:0);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_setTableValueS)
    return retval;
}

int
VisItUI_addStripChartPoint(int chart, int curve, int cycle, double value)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_addStripChartPoint);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const char *name = "STRIP_CHART_ADD_POINT";
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %d | %f :%d",
                name, chart, curve, cycle, value, 1);
        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_addStripChartPoint)
    return retval;
}

int
VisItUI_addStripChartPoints(int chart, int curve, int npts, int *cycles, double *values)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItUI_addStripChartPoints);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.execute_command)
    {
        const char *name = "STRIP_CHART_ADD_POINTS";
        const unsigned int nChars = strlen( name ) + CMD_MAX_STR_LEN;
        char *cmd = (char*) malloc(nChars * sizeof(char));

        sprintf(cmd, "SetUI:s:%s:%d | %d | %d",
                name, chart, curve, npts);

        char subcmd[CMD_MAX_STR_LEN];

        int i;
        for(i = 0; i < npts; ++i)
        {
          sprintf(subcmd, " | %d | %f", cycles[i], values[i]);
          cmd = realloc(cmd, strlen(cmd) + strlen(subcmd) + 1);
          strcat(cmd, subcmd);
        }

        sprintf(subcmd, " :%d", 1);
        cmd = realloc(cmd, strlen(cmd) + strlen(subcmd) + 1);
        strcat(cmd, subcmd);

        (*callbacks->control.execute_command)(engine, cmd);
        retval = VISIT_OKAY;

        free( cmd );
    }
    LIBSIM_API_LEAVE(VisItUI_addStripChartPoints)
    return retval;
}

/***************************************************************************

                        EXPERIMENTAL PLOTTING CODE

****************************************************************************/

/******************************************************************************
*
* Name: VisItAddPlot
*
* Purpose: Add a new plot.
*
* Programmer: Brad Whitlock
* Date:       Fri Sep 19 17:33:35 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItAddPlot(const char *plotType, const char *var)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItAddPlot);

    if(plotType == NULL)
    {
        LIBSIM_API_LEAVE0(VisItAddPlot,
                         "VisItAddPlot: NULL was passed for the plot type.");
        return VISIT_ERROR;
    }

    if(var == NULL)
    {
        LIBSIM_API_LEAVE0(VisItAddPlot,
                         "VisItAddPlot: NULL was passed for the variable.");
        return VISIT_ERROR;
    }

    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.add_plot)
    {
        LIBSIM_MESSAGE("  Calling simv2_add_plot");
        retval = (*callbacks->control.add_plot)(engine, plotType, var);
    }

    LIBSIM_API_LEAVE1(VisItAddPlot, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItAddOperator
*
* Purpose: Add a new operator.
*
* Programmer: Brad Whitlock
* Date:       Fri Sep 19 17:33:35 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItAddOperator(const char *operatorType, int applyToAll)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItAddOperator);

    if(operatorType == NULL)
    {
        LIBSIM_API_LEAVE0(VisItAddOperator,
                         "VisItAddOperator: NULL was passed for the operator type.");
        return VISIT_ERROR;
    }

    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.add_operator)
    {
        LIBSIM_MESSAGE("  Calling simv2_add_operator");
        retval = (*callbacks->control.add_operator)(engine, operatorType, applyToAll);
    }
    LIBSIM_API_LEAVE1(VisItAddOperator, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItDrawPlots
*
* Purpose: Draw the plots in the plot list.
*
* Programmer: Brad Whitlock
* Date:       Fri Sep 19 17:33:35 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItDrawPlots(void)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItDrawPlots);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.draw_plots)
    {
        LIBSIM_MESSAGE("  Calling simv2_draw_plots");
        retval = (*callbacks->control.draw_plots)(engine);
    }
    LIBSIM_API_LEAVE1(VisItDrawPlots, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItDeleteActivePlots
*
* Purpose: Delete the active plots in the plot list.
*
* Programmer: Brad Whitlock
* Date:       Fri Sep 19 17:33:35 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItDeleteActivePlots(void)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItDeleteActivePlots);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.delete_active_plots)
    {
        LIBSIM_MESSAGE("  Calling simv2_delete_active_plots");
        retval = (*callbacks->control.delete_active_plots)(engine);
    }
    LIBSIM_API_LEAVE1(VisItDeleteActivePlots, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItSetActivePlots
*
* Purpose: Set the active plots in the plot list.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  2 13:44:02 PST 2015
*
* Modifications:
*
******************************************************************************/

int
VisItSetActivePlots(const int *ids, int nids)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItSetActivePlots);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.set_active_plots)
    {
        LIBSIM_MESSAGE("  Calling simv2_set_active_plots");
        retval = (*callbacks->control.set_active_plots)(engine, ids, nids);
    }
    LIBSIM_API_LEAVE1(VisItSetActivePlots, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItChangePlotVar
*
* Purpose: Changes the plot variables.
*
* Programmer: Brad Whitlock
* Date:       Thu Dec 14 14:43:29 PST 2017
*
* Modifications:
*
******************************************************************************/

int
VisItChangePlotVar(const char *var, int all)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItChangePlotVar);

    if(var == NULL)
    {
        LIBSIM_API_LEAVE0(VisItChangePlotVar,
                         "VisItChangePlotVar: NULL was passed for the variable.");
        return VISIT_ERROR;
    }

    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.change_plot_var)
    {
        LIBSIM_MESSAGE("  Calling simv2_change_plot_var");
        retval = (*callbacks->control.change_plot_var)(engine, var, all);
    }

    LIBSIM_API_LEAVE1(VisItChangePlotVar, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItSetPlotOptions*
*
* Purpose: Sets the operator options for the active operator.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  2 13:55:43 PST 2015
*
* Modifications:
*
******************************************************************************/

static int
PlotOpt(const char *fieldName, int fieldType, void *fieldVal, int fieldLen)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItSetPlotOptions);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.set_plot_options)
    {
        LIBSIM_MESSAGE("  Calling simv2_plot_options");
        retval = (*callbacks->control.set_plot_options)(engine, fieldName, fieldType, fieldVal, fieldLen);
    }
    LIBSIM_API_LEAVE1(VisItSetPlotOptions, "return %s", ErrorToString(retval))
    return retval;
}

int VisItSetPlotOptionsC(const char *name,char v){ return PlotOpt(name,VISIT_FIELDTYPE_CHAR,(void*)&v,1); }
int VisItSetPlotOptionsUC(const char *name,unsigned char v){ return PlotOpt(name,VISIT_FIELDTYPE_UNSIGNED_CHAR,(void*)&v,1); }
int VisItSetPlotOptionsB(const char *name,int v){ return PlotOpt(name,VISIT_FIELDTYPE_INT,(void*)&v,1); }
int VisItSetPlotOptionsI(const char *name,int v){ return PlotOpt(name,VISIT_FIELDTYPE_INT,(void*)&v,1); }
int VisItSetPlotOptionsL(const char *name,long v){ return PlotOpt(name,VISIT_FIELDTYPE_LONG,(void*)&v,1); }
int VisItSetPlotOptionsF(const char *name,float v){ return PlotOpt(name,VISIT_FIELDTYPE_FLOAT,(void*)&v,1); }
int VisItSetPlotOptionsD(const char *name,double v){ return PlotOpt(name,VISIT_FIELDTYPE_DOUBLE,(void*)&v,1); }
int VisItSetPlotOptionsS(const char *name,const char *v){ return PlotOpt(name,VISIT_FIELDTYPE_STRING,(void*)v,1); }

int VisItSetPlotOptionsCv(const char *name,const char *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_CHAR_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsUCv(const char *name,const unsigned char *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_UNSIGNED_CHAR_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsIv(const char *name,const int *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_INT_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsLv(const char *name,const long *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_LONG_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsFv(const char *name,const float *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_FLOAT_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsDv(const char *name,const double *v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_DOUBLE_ARRAY,(void*)v,L); }
int VisItSetPlotOptionsSv(const char *name,const char **v,int L){ return PlotOpt(name,VISIT_FIELDTYPE_STRING_ARRAY,(void*)v,L); }

/******************************************************************************
*
* Name: VisItSetOperatorOptions*
*
* Purpose: Sets the operator options for the active operator.
*
* Programmer: Brad Whitlock
* Date:       Mon Feb  2 13:55:43 PST 2015
*
* Modifications:
*
******************************************************************************/
static int
OperatorOpt(const char *fieldName, int fieldType, void *fieldVal, int fieldLen)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItSetOperatorOptions);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.set_operator_options)
    {
        LIBSIM_MESSAGE("  Calling simv2_operator_options");
        retval = (*callbacks->control.set_operator_options)(engine, fieldName, fieldType, fieldVal, fieldLen);
    }
    LIBSIM_API_LEAVE1(VisItSetOperatorOptions, "return %s", ErrorToString(retval))
    return retval;
}

int VisItSetOperatorOptionsC(const char *name,char v){ return OperatorOpt(name,VISIT_FIELDTYPE_CHAR,(void*)&v,1); }
int VisItSetOperatorOptionsUC(const char *name,unsigned char v){ return OperatorOpt(name,VISIT_FIELDTYPE_UNSIGNED_CHAR,(void*)&v,1); }
int VisItSetOperatorOptionsB(const char *name,int v){ return OperatorOpt(name,VISIT_FIELDTYPE_INT,(void*)&v,1); }
int VisItSetOperatorOptionsI(const char *name,int v){ return OperatorOpt(name,VISIT_FIELDTYPE_INT,(void*)&v,1); }
int VisItSetOperatorOptionsL(const char *name,long v){ return OperatorOpt(name,VISIT_FIELDTYPE_LONG,(void*)&v,1); }
int VisItSetOperatorOptionsF(const char *name,float v){ return OperatorOpt(name,VISIT_FIELDTYPE_FLOAT,(void*)&v,1); }
int VisItSetOperatorOptionsD(const char *name,double v){ return OperatorOpt(name,VISIT_FIELDTYPE_DOUBLE,(void*)&v,1); }
int VisItSetOperatorOptionsS(const char *name,const char *v){ return OperatorOpt(name,VISIT_FIELDTYPE_STRING,(void*)v,1); }

int VisItSetOperatorOptionsCv(const char *name,const char *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_CHAR_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsUCv(const char *name,const unsigned char *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_UNSIGNED_CHAR_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsIv(const char *name,const int *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_INT_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsLv(const char *name,const long *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_LONG_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsFv(const char *name,const float *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_FLOAT_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsDv(const char *name,const double *v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_DOUBLE_ARRAY,(void*)v,L); }
int VisItSetOperatorOptionsSv(const char *name,const char **v,int L){ return OperatorOpt(name,VISIT_FIELDTYPE_STRING_ARRAY,(void*)v,L); }

/******************************************************************************
*
* Name: VisItExportDatabase / VisItExportDatabaseWithOptions
*
* Purpose: Export the current plots to a database file.
*
* Programmer: Brad Whitlock
* Date:       Thu Sep 18 10:32:29 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItExportDatabaseWithOptions(const char *filename, const char *format,
    visit_handle varNames, visit_handle options)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItExportDatabaseWithOptions);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.exportdatabase_with_options)
    {
        LIBSIM_MESSAGE("  Calling simv2_exportdatabase_with_options");
        retval = (*callbacks->control.exportdatabase_with_options)(engine, filename, format, varNames, options);
        /* Synchronize in case we we're connected interactively. */
        if(visit_batch_mode == 0 && visit_sync_enabled)
            VisItSynchronize();
    }
    LIBSIM_API_LEAVE1(VisItExportDatabaseWithOptions, "return %s", ErrorToString(retval))
    return retval;
}

int
VisItExportDatabase(const char *filename, const char *format, visit_handle varNames)
{
    return VisItExportDatabaseWithOptions(filename, format, varNames, VISIT_INVALID_HANDLE);
}

/******************************************************************************
*
* Name: VisItRestoreSession
*
* Purpose: Export the current plots to a database file.
*
* Programmer: Brad Whitlock
* Date:       Thu Sep 18 10:32:29 PDT 2014
*
* Modifications:
*
******************************************************************************/

int
VisItRestoreSession(const char *filename)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItRestoreSession);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.restoresession)
    {
        LIBSIM_MESSAGE("  Calling simv2_restoresession");
        retval = (*callbacks->control.restoresession)(engine, filename);
    }
    LIBSIM_API_LEAVE1(VisItRestoreSession, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItSetView2D
*
* Purpose: Set the 2D view.
*
* Programmer: Brad Whitlock
* Date:       Thu Jun  1 15:54:47 PDT 2017
*
* Modifications:
*
******************************************************************************/

int
VisItSetView2D(visit_handle v)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItSetView2D);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.set_view2D)
    {
        LIBSIM_MESSAGE("  Calling simv2_set_view2D");
        retval = (*callbacks->control.set_view2D)(engine, v);
    }
    LIBSIM_API_LEAVE1(VisItSetView2D, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItGetView2D
*
* Purpose: Get the 2D view.
*
* Programmer: Brad Whitlock
* Date:       Thu Jun  1 15:54:47 PDT 2017
*
* Modifications:
*
******************************************************************************/

int
VisItGetView2D(visit_handle v)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItGetView2D);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.get_view2D)
    {
        LIBSIM_MESSAGE("  Calling simv2_get_view2D");
        retval = (*callbacks->control.get_view2D)(engine, v);
    }
    LIBSIM_API_LEAVE1(VisItGetView2D, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItSetView3D
*
* Purpose: Set the 3D view.
*
* Programmer: Brad Whitlock
* Date:       Thu Jun  1 15:54:47 PDT 2017
*
* Modifications:
*
******************************************************************************/

int
VisItSetView3D(visit_handle v)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItSetView3D);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.set_view3D)
    {
        LIBSIM_MESSAGE("  Calling simv2_set_view3D");
        retval = (*callbacks->control.set_view3D)(engine, v);
    }
    LIBSIM_API_LEAVE1(VisItSetView3D, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItGetView3D
*
* Purpose: Get the 3D view.
*
* Programmer: Brad Whitlock
* Date:       Thu Jun  1 15:54:47 PDT 2017
*
* Modifications:
*
******************************************************************************/

int
VisItGetView3D(visit_handle v)
{
    int retval = VISIT_ERROR;

    LIBSIM_API_ENTER(VisItGetView3D);
    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.get_view3D)
    {
        LIBSIM_MESSAGE("  Calling simv2_get_view3D");
        retval = (*callbacks->control.get_view3D)(engine, v);
    }
    LIBSIM_API_LEAVE1(VisItGetView3D, "return %s", ErrorToString(retval))
    return retval;
}

/******************************************************************************
*
* Name: VisItSaveCinema
*
* Purpose: Save a Cinema database.
*
* Programmer: Brad Whitlock
* Date:       Thu Jun  1 15:54:47 PDT 2017
*
* Modifications:
*
******************************************************************************/
static void spherical_to_cartesian(double p[3], double theta, double phi)
{
    p[0] = cos(theta) * cos(phi);
    p[1] = sin(theta);
    p[2] = cos(theta) * sin(phi);
}

static int ideg(double rad)
{
    return (int)((360. * rad) / (2. * M_PI));
}

typedef struct
{
    visit_handle h;
    char *file_cdb;
    int spec;
    int composite;
    int imgFormat;
    int width;
    int height;
    int cameraType;
    int nphi;
    int ntheta;
    visit_handle varnames;

    double *phi;
    double *theta;
    double *times;
    int     times_n;
    int     times_size;
} cinema_t;

void
cinema_t_create(cinema_t *obj)
{
    obj->h = VISIT_INVALID_HANDLE;
    obj->file_cdb = strdup("visit.cdb");
    obj->spec = VISIT_CINEMA_SPEC_A;
    obj->composite = 0;
    obj->width = 600;
    obj->height = 600;
    obj->imgFormat = VISIT_IMAGEFORMAT_PNG;
    obj->cameraType = VISIT_CINEMA_CAMERA_PHI_THETA;
    obj->nphi = 12;
    obj->ntheta = 7;
    obj->varnames = VISIT_INVALID_HANDLE;

    obj->phi = NULL;
    obj->theta = NULL;
    obj->times_n = 0;
    obj->times_size = 1000;
    obj->times = (double *)malloc(obj->times_size * sizeof(double));
}

void
cinema_t_destroy(cinema_t *obj)
{
    if(obj == NULL)
        return;
    if(obj->file_cdb != NULL)
    {
        free(obj->file_cdb);
        obj->file_cdb = NULL;
    }
    if(obj->phi != NULL)
    {
        free(obj->phi);
        obj->phi = NULL;
    }
    if(obj->theta != NULL)
    {
        free(obj->theta);
        obj->theta = NULL;
    }
    if(obj->times != NULL)
    {
        obj->times_n = 0;
        free(obj->times);
        obj->times = NULL;
    }
}

void
cinema_t_addtime(cinema_t *obj, double t)
{
    if(obj->times_n+1 >= obj->times_size)
    {
        obj->times_size += 1000;
        obj->times = (double *)realloc(obj->times, obj->times_size * sizeof(double));
    }
    obj->times[obj->times_n++] = t;
}

void
cinema_t_compute_phi_theta(cinema_t *obj)
{
    int itheta, iphi;
    double tt, tp;
    obj->theta = (double *) malloc(obj->ntheta * sizeof(double));
    obj->phi   = (double *) malloc(obj->nphi * sizeof(double));
    for(itheta = 0; itheta < obj->ntheta; ++itheta)
    {
        if(obj->ntheta > 1)
            tt = (float)(itheta) / (float)(obj->ntheta-1);
        else
            tt = 0.;
        obj->theta[itheta] = ((1.-tt) * -M_PI/2.) + (tt*M_PI/2.);
    }
    for(iphi = 0; iphi < obj->nphi; ++iphi)
    {
        if(obj->nphi > 1)
            tp = (float)(iphi) / (float)(obj->nphi-1);
        else
            tp = 0.;
        obj->phi[iphi] = ((1.-tp) * -M_PI) + (tp*M_PI);
    }
}

static cinema_t *cinema_map = NULL;
static int       cinema_map_n = 0;
static int       cinema_map_size = 0;
static int       cinema_map_id = 0;
static const char *cinema_ext[] = {".bmp", ".jpeg", ".png", ".povray", ".ppm", ".rgb", ".tiff", ".exr"};

cinema_t *cinema_create(const char *file_cdb)
{
    if(cinema_map == NULL)
    {
        cinema_map_size = 10;
        cinema_map_n = 1;
        cinema_map = (cinema_t *) malloc(cinema_map_size * sizeof(cinema_t));

        cinema_t_create(&cinema_map[0]);
        free(cinema_map[0].file_cdb);
        cinema_map[0].file_cdb = strdup(file_cdb);
        cinema_map[0].h = cinema_map_id++;

        return cinema_map;
    }
    else
    {
        int i;
        for(i = 0; i < cinema_map_n; ++i)
        {
            if(strcmp(cinema_map[i].file_cdb, file_cdb) == 0)
                return &cinema_map[i];
        }
        if(cinema_map_n+1 >= cinema_map_size)
        {
            cinema_map_size += 10;
            cinema_map = (cinema_t*)realloc(cinema_map, cinema_map_size*sizeof(cinema_t));
        }

        cinema_t_create(&cinema_map[cinema_map_n]);
        free(cinema_map[cinema_map_n].file_cdb);
        cinema_map[cinema_map_n].file_cdb = strdup(file_cdb);
        cinema_map[cinema_map_n].h = cinema_map_id++;

        return &cinema_map[cinema_map_n++];
    }
}

int cinema_destroy(visit_handle h)
{
    int i, j;
    for(i = 0; i < cinema_map_n; ++i)
    {
        if(h == cinema_map[i].h)
        {
            cinema_t_destroy(&cinema_map[i]);
            for(j = i; j < cinema_map_n-1; ++j)
                memcpy(&cinema_map[j], &cinema_map[j+1], sizeof(cinema_t));
            cinema_map_n--;
            return 1;
        }
    }
    return 0;
}

cinema_t *cinema_get(visit_handle h)
{
    if(cinema_map != NULL)
    {
        int i;
        for(i = 0; i < cinema_map_n; ++i)
        {
            if(cinema_map[i].h == h)
                return &cinema_map[i];
        }
    }
    return NULL;
}

int
dir_join_size(char **dirnames, int ndirnames)
{
    int i, len = 0;
    for(i = 0; i < ndirnames; ++i)
    {
       size_t s = strlen(dirnames[i]);
       if(s < 100) /* pad the size in case the dirnames change size a little. */
           s = 100;
       len += s + 2;
    }
    return len;
}

int
dir_exists(const char *dirname)
{
    struct stat s;
    return (stat(dirname, &s) == 0) ? 1 : 0;
}

int
dir_create(char **dirnames, int ndirnames)
{
    int len;
    char *joinname;
    if(parallelRank == 0)
    {
        len = dir_join_size(dirnames, ndirnames);
        joinname = (char *) malloc(len * sizeof(char));
        if(joinname != NULL)
        {
            int i, len = 0;
            for(i = 0; i < ndirnames; ++i)
            {
                len += sprintf(joinname+len, "%s/", dirnames[i]);

                if(!dir_exists(joinname))
                {
                    VisItMkdir(joinname, 7*64 + 7*8 + 7);
                }
            }

            free(joinname);
        }
    }

    return 1;
}

void
dir_join(char *joinname, char **dirnames, int ndirnames)
{
    int i, len = 0;
    for(i = 0; i < ndirnames; ++i)
    {
        len += sprintf(joinname+len, "%s/", dirnames[i]);
    }
}

int
cinema_t_hasvars(cinema_t *obj, int *nvars)
{
    int changevars = 0;
    if(obj == NULL)
    {
        *nvars = 0;
        return 0;
    }

    if(obj == NULL)
        *nvars = 0;
    else if(obj->varnames == VISIT_INVALID_HANDLE)
    {
        *nvars = 1;
    }
    else
    {
        if(VisIt_NameList_getNumName(obj->varnames, nvars) == VISIT_OKAY)
        {
            if(*nvars < 1)
                *nvars = 1;
            else
                changevars = 1;
        }
    }
    return changevars;
}

int
cinema_t_static_image(cinema_t *obj)
{
    FILE *f = NULL;
    int ret = VISIT_ERROR;
    char filebase[1024], *path = NULL;
    char *dirnames[2];
    dirnames[0] = obj->file_cdb;
    dirnames[1] = "image";

    if(dir_create(dirnames, 2))
    {
        int i, nvars, changevars, len;

        len = dir_join_size(dirnames, 2) + 1024;
        path = (char *) malloc(len * sizeof(char));

        /* Determine whether we have variables to switch. */
        changevars = cinema_t_hasvars(obj, &nvars);

        /* Loop through the variables. */
        for(i = 0; i < nvars; ++i)
        {
            if(changevars)
            {
                char *var = NULL;
                if(VisIt_NameList_getName(obj->varnames, i, &var) == VISIT_OKAY)
                {
                    VisItChangePlotVar(var, 1);
                    sprintf(filebase, "%s_%1.6e", var, obj->times[obj->times_n-1]);
                    free(var);
                }
            }
            else
            {
                sprintf(filebase, "time_%1.6e", obj->times[obj->times_n-1]);
            }

            /* Make the image filename and save. */
            dir_join(path, dirnames, 2);
            strcat(path, filebase);
            strcat(path, cinema_ext[obj->imgFormat]);

            ret = VisItSaveWindow(path, obj->width, obj->height, obj->imgFormat);
        }

        /* Make the info.json filename. */
        dir_join(path, dirnames, 2);
        strcat(path, "info.json");
        if((f = fopen(path, "wt")) != NULL)
        {
            fprintf(f, "{\n");
            fprintf(f, "  \"type\" : \"simple\",\n");
            fprintf(f, "  \"version\": \"1.1\",\n");
            fprintf(f, "  \"metadata\":{\"type\":\"parametric-image-stack\"},\n");
            if(changevars)
                fprintf(f, "  \"name_pattern\":\"{var}_{time}%s\",\n", cinema_ext[obj->imgFormat]);
            else
                fprintf(f, "  \"name_pattern\":\"time_{time}%s\",\n", cinema_ext[obj->imgFormat]);
            fprintf(f, "  \"arguments\":{\n");
            if(changevars)
            {
                fprintf(f, "    \"var\": {\n");
                fprintf(f, "       \"label\":\"Variable\",\n");
                fprintf(f, "       \"type\":\"range\",\n");
                fprintf(f, "       \"values\":[");
                for(i = 0; i < nvars; ++i)
                {
                    char *var = NULL;
                    VisIt_NameList_getName(obj->varnames, i, &var);
                    fprintf(f, "\"%s\"", (var == NULL) ? "" : var);
                    if(i < nvars-1)
                        fprintf(f, ", ");
                    if(var != NULL)
                        free(var);
                }
                fprintf(f, "]\n");
                fprintf(f, "    },\n");
            }
            fprintf(f, "    \"time\": {\n");
            fprintf(f, "       \"default\":\"%1.6e\",\n", obj->times[0]);
            fprintf(f, "       \"label\":\"Time\",\n");
            fprintf(f, "       \"type\":\"range\",\n");
            fprintf(f, "       \"values\":[");
            for(i = 0; i < obj->times_n; ++i)
            {
                fprintf(f, "\"%1.6e\"", obj->times[i]);
                if(i < obj->times_n-1)
                    fprintf(f, ", ");
            }
            fprintf(f, "]\n");
            fprintf(f, "    }\n");
            fprintf(f, "  }\n");
            fprintf(f, "}\n");
            fclose(f);
            ret = VISIT_OKAY;
        }
    }

    return ret;
}

int
cinema_t_static_composite(cinema_t *obj)
{
    fprintf(stderr, "cinema_t_static_composite: not implemented");
    return VISIT_ERROR;
}

int
cinema_t_phitheta_image(cinema_t *obj)
{
    FILE *f = NULL;
    char timedir[100], phidir[100], thetadir[100], *path = NULL;
    char *dirnames[5];
    double pOffset, normal[3], up[3];
    int i, itheta, iphi, changevars, nvars, ret = VISIT_ERROR;
    visit_handle origview, view;

    sprintf(timedir, "time_%1.6e", obj->times[obj->times_n-1]);
    dirnames[0] = obj->file_cdb;
    dirnames[1] = "image";
    dirnames[2] = timedir;
    dirnames[3] = phidir;
    dirnames[4] = thetadir;

    /* Get the camera. */
    VisIt_View3D_alloc(&origview);
    VisIt_View3D_alloc(&view);
    VisItGetView3D(origview);

    /* Determine whether we have variables to switch. */
    changevars = cinema_t_hasvars(obj, &nvars);

    /* Loop through the variables. */
    for(i = 0; i < nvars; ++i)
    {
        if(changevars)
        {
            char *var = NULL;
            if(VisIt_NameList_getName(obj->varnames, i, &var) == VISIT_OKAY)
            {
                VisItChangePlotVar(var, 1);
                free(var);
            }
        }

        /* Vary the camera and save images. */
        for(itheta = 0; itheta < obj->ntheta; ++itheta)
        {
            sprintf(thetadir, "theta_%d", ideg(obj->theta[itheta]));
            for(iphi = 0; iphi < obj->nphi; ++iphi)
            {
                sprintf(phidir, "phi_%d", ideg(obj->phi[iphi]));

                if(path == NULL)
                {
                    int len = dir_join_size(dirnames, 5) + 1024;
                    path = (char *) malloc(len * sizeof(char));
                }

                /* Compensate to match PV.*/
                pOffset = -M_PI/2.;

                /* Change the view.*/
                spherical_to_cartesian(normal, obj->theta[itheta], obj->phi[iphi] + pOffset);
                spherical_to_cartesian(up,     obj->theta[itheta]+M_PI/4., obj->phi[iphi] + pOffset);

#if 0
                printf("theta=%lg, phi=%lg, normal={%lg,%lg,%lg}, up={%lg,%lg,%lg}\n",
                    theta[itheta], phi[iphi], normal[0], normal[1], normal[2],
                    up[0], up[1], up[2]);
#endif
                /* Override the normal and up vectors. */
                VisIt_View3D_copy(view, origview);
                VisIt_View3D_setViewNormal(view, normal);
                VisIt_View3D_setViewUp(view, up);
                VisItSetView3D(view);

                /* Create the output directories. */
                dir_create(dirnames, 5);

                /* Make the filename. */
                dir_join(path, dirnames, 5);
                if(changevars)
                {
                    char *var = NULL;
                    if(VisIt_NameList_getName(obj->varnames, i, &var) == VISIT_OKAY)
                    {
                        strcat(path, var);
                        free(var);
                    }
                    else
                        strcat(path, "image");
                }
                else
                {
                    strcat(path, "image");
                }
                strcat(path, cinema_ext[obj->imgFormat]);

                /* Save the image*/
                ret = VisItSaveWindow(path, obj->width, obj->height, obj->imgFormat);
            }
        }
    }

    VisIt_View3D_free(view);
    VisIt_View3D_free(origview);

    /* Make the Cinema database. */
    if(parallelRank == 0)
    {
        dir_join(path, dirnames, 2);
        if(obj->spec == VISIT_CINEMA_SPEC_A)
        {
            strcat(path, "info.json");
            if((f = fopen(path, "wt")) != NULL)
            {
                fprintf(f, "{\n");
                fprintf(f, "  \"type\" : \"simple\",\n");
                fprintf(f, "  \"version\" : \"1.1\",\n");
                fprintf(f, "  \"metadata\": {\n");
                fprintf(f, "    \"type\": \"parametric-image-stack\"\n");
                fprintf(f, "  },\n");
                if(changevars)
                    fprintf(f, "  \"name_pattern\": \"time_{time}/phi_{phi}/theta_{theta}/{var}%s\",\n", cinema_ext[obj->imgFormat]);
                else
                    fprintf(f, "  \"name_pattern\": \"time_{time}/phi_{phi}/theta_{theta}/image%s\",\n", cinema_ext[obj->imgFormat]);
                fprintf(f, "  \"arguments\": {\n");

                fprintf(f, "    \"time\": {");
                fprintf(f, "\"default\":\"%1.6e\",", obj->times[0]);
                fprintf(f, " \"label\":\"Time\",");
                fprintf(f, " \"type\":\"range\",");
                fprintf(f, " \"values\":[");
                for(i = 0; i < obj->times_n; ++i)
                {
                    fprintf(f, "\"%1.6e\"", obj->times[i]);
                    if(i < obj->times_n-1)
                        fprintf(f, ", ");
                }
                fprintf(f, "]},\n");

                fprintf(f, "    \"theta\": {\"default\":%d, \"label\":\"theta\", \"type\":\"range\", \"values\":[", ideg(obj->theta[obj->ntheta/2]));
                for(i = 0; i < obj->ntheta; ++i)
                {
                    fprintf(f, "%d", ideg(obj->theta[i]));
                    if(i < obj->ntheta-1)
                        fprintf(f, ",");
                }
                fprintf(f, "]},\n");

                fprintf(f, "    \"phi\": {\"default\":%d, \"label\":\"phi\", \"type\":\"range\", \"values\":[", ideg(obj->phi[0]));
                for(i = 0; i < obj->nphi; ++i)
                {
                    fprintf(f, "%d", ideg(obj->phi[i]));
                    if(i < obj->nphi-1)
                        fprintf(f, ",");
                }
                fprintf(f, "]}\n");

                if(changevars)
                {
                    fprintf(f, "    ,\"var\": {");
                    fprintf(f, "\"label\":\"Variable\",");
                    fprintf(f, " \"type\":\"range\",");
                    fprintf(f, " \"values\":[");
                    for(i = 0; i < nvars; ++i)
                    {
                        char *var = NULL;
                        VisIt_NameList_getName(obj->varnames, i, &var);
                        fprintf(f, "\"%s\"", (var == NULL) ? "" : var);
                        if(i < nvars-1)
                            fprintf(f, ", ");
                        if(var != NULL)
                            free(var);
                    }
                    fprintf(f, "]");
                    fprintf(f, "}\n");
                }

                fprintf(f, "  }\n");
                fprintf(f, "}\n");
                fclose(f);
            }
        }
    }

    free(path);

    LIBSIM_API_LEAVE1(VisItSaveCinema, "return %s", ErrorToString(ret));
    return ret;
}

int
cinema_t_phitheta_composite(cinema_t *obj)
{
    fprintf(stderr, "cinema_t_phitheta_composite: not implemented");
    return VISIT_ERROR;
}

visit_handle
VisItBeginCinema(visit_handle *h,
    const char *file_cdb, int dbspec, int composite,
    int imgFormat, int width, int height,
    int cameraType, int nphi, int ntheta,
    visit_handle varnames)
{
    int ret = VISIT_ERROR;
    cinema_t *obj = NULL;
    LIBSIM_API_ENTER(VisItBeginCinema);
    if((file_cdb != NULL) &&
       (composite == 0 || (composite == 1 && dbspec == VISIT_CINEMA_SPEC_C)) &&
       (width > 0 && height > 0) &&
       (imgFormat == VISIT_IMAGEFORMAT_BMP || imgFormat == VISIT_IMAGEFORMAT_JPEG || imgFormat == VISIT_IMAGEFORMAT_PNG || imgFormat == VISIT_IMAGEFORMAT_TIFF) &&
       (cameraType == VISIT_CINEMA_CAMERA_STATIC || cameraType == VISIT_CINEMA_CAMERA_PHI_THETA) &&
       (nphi > 0 && ntheta > 0)
      )
    {
        if(dbspec == VISIT_CINEMA_SPEC_C) /* For now */
        {
            fprintf(stderr, "VisItBeginCinema does not yet support spec C.\n");
        }
        else if(dbspec == VISIT_CINEMA_SPEC_D)
        {
            fprintf(stderr, "VisItBeginCinema does not yet support spec D.\n");
        }
        else if(dbspec == VISIT_CINEMA_SPEC_A)
        {
            obj = cinema_create(file_cdb);
            if(obj != NULL)
            {
                *h = obj->h;

                obj->spec = dbspec;
                obj->composite = composite;
                obj->imgFormat = imgFormat;
                obj->width = width;
                obj->height = height;
                obj->cameraType = cameraType;
                obj->nphi = nphi;
                obj->ntheta = ntheta;
                obj->varnames = varnames;
                cinema_t_compute_phi_theta(obj);
            }
        }
    }

    /* Create the containing directory ? */

    ret = (obj != NULL) ? VISIT_OKAY : VISIT_ERROR;
    LIBSIM_API_LEAVE1(VisItBeginCinema, "return %s", ErrorToString(ret));
    return ret;
}

int
VisItSaveCinema(visit_handle h, double time)
{
    int ret = VISIT_ERROR;
    LIBSIM_API_ENTER(VisItSaveCinema);

    /* Make sure the function exists before using it. */
    if (engine && callbacks != NULL && callbacks->control.save_window)
    {
        cinema_t *obj = cinema_get(h);
        if(obj != NULL)
        {
            cinema_t_addtime(obj, time);
            if(obj->composite)
            {
                if(obj->cameraType == VISIT_CINEMA_CAMERA_STATIC)
                    ret = cinema_t_static_composite(obj);
                else if(obj->cameraType == VISIT_CINEMA_CAMERA_PHI_THETA)
                    ret = cinema_t_phitheta_composite(obj);
            }
            else
            {
                if(obj->cameraType == VISIT_CINEMA_CAMERA_STATIC)
                    ret = cinema_t_static_image(obj);
                else if(obj->cameraType == VISIT_CINEMA_CAMERA_PHI_THETA)
                    ret = cinema_t_phitheta_image(obj);
            }
        }
    }
    LIBSIM_API_LEAVE1(VisItSaveCinema, "return %s", ErrorToString(ret));
    return ret;
}

int
VisItEndCinema(visit_handle h)
{
    int ret = VISIT_ERROR;
    LIBSIM_API_ENTER(VisItEndCinema);
    /* We can remove a cinema object. */
    ret = cinema_destroy(h) ? VISIT_OKAY : VISIT_ERROR;
    LIBSIM_API_LEAVE1(VisItEndCinema, "return %s", ErrorToString(ret));
    return ret;
}
