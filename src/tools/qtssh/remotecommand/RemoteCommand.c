#ifndef AUTO_WINSOCK
#include <winsock2.h>
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define PUTTY_DO_GLOBALS               /* actually _define_ globals */
#include "putty.h"
#include "puttystorage.h"
#include "tree234.h"
    
#include <RemoteCommand.h>

/*
 * Type definitions.
 */
struct input_data {
    DWORD len;
    char buffer[4096];
    HANDLE event, eventback;
};

/*
 * External functions.
 */
extern SOCKET first_socket(enum234 *), next_socket(enum234 *);
extern int select_result(WPARAM, LPARAM);

/*
 * GLOBAL VARIABLES!
 */
static const char *host_g = NULL;
static int initSockets_g = 1;
static passwordCallback *passwordCB = NULL;
static WSAEVENT netevent;

/*
 * Prototypes.
 */
static int InitializeWinsock();
static DWORD WINAPI stdin_read_thread(void *param);
static int get_password(const char *prompt, char *str, int maxlen);
void fatalbox (char *p, ...);
void connection_fatal (char *p, ...);
void logevent(char *string);
void begin_session(void);
void from_backend(int is_stderr, char *data, int len);
char *do_select(SOCKET skt, int startup);
void verify_ssh_host_key(char *host, int port, char *keytype,
                         char *keystr, char *fingerprint);

/******************************************************************************
 *
 * Purpose: This is the only function that is exposed from the DLL. It is
 *          pretty much just a modified version of Simon Tatham's plink main
 *          function. I changed the interface so I could make it into an SSH
 *          library for running remote commands and so I could make it have
 *          a user-defined way to supply the password.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Aug 29 14:28:59 PST 2002
 *
 * Input Arguments:
 *   username    : The user login name for the specified host.
 *   host        : The name of the host that we want to run the command.
 *   port        : The port to use when connecting to the host.
 *   commands    : A list of strings where the first element specifies the name
 *                 of the remote program to run.
 *   nCommands   : The number of strings in the commands array.
 *   cb          : A callback function for getting the password.
 *   initSockets : Whether to initialize Windows sockets.
 *
 * Returns: This function returns -1 if there was an error or 0 if it all
 *          worked.
 *
 * Modifications:
 *   Brad Whitlock, Fri Oct 10 14:21:53 PST 2003
 *   Added the port argument so we can use whatever port we want.
 *
 *****************************************************************************/

int
RunRemoteCommand(const char *username, const char *host, int port,
    const char *commands[], int nCommands, passwordCallback *cb,
    int initSockets)
{
    WSAEVENT stdinevent;
    HANDLE handles[2];
    DWORD threadid;
    struct input_data idata;
    int sending;
    int portnumber = -1;
    SOCKET *sklist = NULL;
    int skcount = 0, sksize = 0;
    int connopen;

    if(username == NULL || host == NULL || commands == NULL ||
       nCommands == 0)
    {
       return -1;
    }

    /* Store a global flag that indicates if we should initialize
     * sockets when calling this function.
     */
    initSockets_g = initSockets;

    /* Store the callback function to get the password. */
    passwordCB = cb;

    ssh_get_password = get_password;

    flags = FLAG_STDERR;
    
    /*
     * Set some default values into the config.
     */
    do_defaults(NULL, &cfg);

    /*
     * Force SSH
     */
    default_protocol = cfg.protocol = PROT_SSH;
    default_port = cfg.port = port;
    back = backends[0].backend;

    /*
     * Copy the supplied username into the config.
     */
    strncpy(cfg.username, username, sizeof(cfg.username));
    cfg.username[sizeof(cfg.username)-1] = '\0';

    /*
     * Copy the hostname into the config.
     */
    host_g = host;
    strncpy(cfg.host, host, sizeof(cfg.host)-1);
    cfg.host[sizeof(cfg.host)-1] = '\0';

    /*
     * Verbose for now...
     */
    //flags |= FLAG_VERBOSE;

    /*
     * Copy the remote commands into the config.
     */
    memset(cfg.remote_cmd, 0, sizeof(cfg.remote_cmd));
    {
        char *p = cfg.remote_cmd;
        int i, available = sizeof(cfg.remote_cmd);
        for(i = 0; i < nCommands; ++i)
        {
            int len = strlen(commands[i]);
            if(len > available)
               len = available - 1;
            strncpy(p, commands[i], len);
            p += len;
            if(i < nCommands - 1)
                *p++ = ' ';
            else
                *p++ = '\0';
        }
    }
    cfg.nopty = TRUE;      /* command => no terminal */
    cfg.ldisc_term = TRUE; /* use stdin like a line buffer */

    /*
     * Initialize WinSock.
     */
    if(initSockets_g)
    {
        if(InitializeWinsock())
            return -1;
    }
    sk_init();

    /*
     * Start up the connection.
     */
    netevent = CreateEvent(NULL, FALSE, FALSE, NULL);
    {
        char *error;
        char *realhost;

        error = back->init(cfg.host, cfg.port, &realhost);
        if(error)
        {
            fprintf(stderr, "Unable to open connection:\n%s", error);
            return -1;
        }
    }
    connopen = 1;

    stdinevent = CreateEvent(NULL, FALSE, FALSE, NULL);
  
    /*
     * Turn off ECHO and LINE input modes. We don't care if this
     * call fails, because we know we aren't necessarily running in
     * a console.
     */
    handles[0] = netevent;
    handles[1] = stdinevent;
    sending = FALSE;
    while (1)
    {
        int n;

        if(!sending && back->sendok())
        {
            /*
             * Create a separate thread to read from stdin. This is
             * a total pain, but I can't find another way to do it:
             *
             *  - an overlapped ReadFile or ReadFileEx just doesn't
             *    happen; we get failure from ReadFileEx, and
             *    ReadFile blocks despite being given an OVERLAPPED
             *    structure. Perhaps we can't do overlapped reads
             *    on consoles. WHY THE HELL NOT?
             * 
             *  - WaitForMultipleObjects(netevent, console) doesn't
             *    work, because it signals the console when
             *    _anything_ happens, including mouse motions and
             *    other things that don't cause data to be readable
             *    - so we're back to ReadFile blocking.
             */
            idata.event = stdinevent;
            idata.eventback = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (!CreateThread(NULL, 0, stdin_read_thread,
                              &idata, 0, &threadid))
            {
                fprintf(stderr, "Unable to create second thread\n");
                return -1;
            }
            sending = TRUE;
        }

        n = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (n == 0)
        {
            WSANETWORKEVENTS things;
            enum234 e;
            SOCKET socket;
            int i;

            /*
             * We must not call select_result() for any socket
             * until we have finished enumerating within the tree.
             * This is because select_result() may close the socket
             * and modify the tree.
             */
            /* Count the active sockets. */
            i = 0;
            for (socket = first_socket(&e);
                 socket != INVALID_SOCKET;
                 socket = next_socket(&e))
            {
                i++;
            }

            /* Expand the buffer if necessary. */
            if (i > sksize)
            {
                sksize = i + 16;
                sklist = srealloc(sklist, sksize * sizeof(*sklist));
            }

            /* Retrieve the sockets into sklist. */
            skcount = 0;
            for (socket = first_socket(&e);
                 socket != INVALID_SOCKET;
                 socket = next_socket(&e))
            {
                sklist[skcount++] = socket;
            }

            /* Now we're done enumerating; go through the list. */
            for (i = 0; i < skcount; i++)
            {
                WPARAM wp;
                socket = sklist[i];
                wp = (WPARAM)socket;
                if (!WSAEnumNetworkEvents(socket, netevent, &things))
                {
                    noise_ultralight(socket);
                    noise_ultralight(things.lNetworkEvents);

                    if (things.lNetworkEvents & FD_READ)
                        connopen &= select_result(wp, (LPARAM)FD_READ);
                    if (things.lNetworkEvents & FD_CLOSE)
                        connopen &= select_result(wp, (LPARAM)FD_CLOSE);
                    if (things.lNetworkEvents & FD_OOB)
                        connopen &= select_result(wp, (LPARAM)FD_OOB);
                    if (things.lNetworkEvents & FD_WRITE)
                        connopen &= select_result(wp, (LPARAM)FD_WRITE);
                }
            }
        }
        else if (n == 1)
        {
            noise_ultralight(idata.len);
            if (idata.len > 0)
            {
                back->send(idata.buffer, idata.len);
            }
            else
            {
                back->special(TS_EOF);
            }
            SetEvent(idata.eventback);
        }

        /* we closed the connection */
        if (!connopen || back->socket() == NULL)
            break;
    }

    if(initSockets_g)
        WSACleanup();

    return 0;
}

/******************************************************************************
 *
 * Purpose: This is the callback function for getting the password. It calls
 *          the callback function that was passed to RunRemoteCommand.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Aug 29 14:24:24 PST 2002
 *
 * Modifications:
 *
 *****************************************************************************/

static int get_password(const char *prompt, char *str, int maxlen)
{
    int okay = 1;
    const char *password = NULL;

    /* Call the user-supplied password gathering function. */
    password = passwordCB(host_g, &okay);

    /* Copy the password into the return buffer. */
    if(okay)
    {
        strncpy(str, password, maxlen);
        str[maxlen-1] = '\0';
    }

    return okay ? 1 : 0;
}

static int
InitializeWinsock()
{
    WSADATA wsadata;
    WORD winsock_ver;

    winsock_ver = MAKEWORD(2, 0);
    if(WSAStartup(winsock_ver, &wsadata))
    {
           return 1;
    }
    if(LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 0)
    {
        WSACleanup();
        return 1;
    }

    return 0;
}

static DWORD WINAPI stdin_read_thread(void *param)
{
    struct input_data *idata = (struct input_data *)param;
    HANDLE inhandle;

    inhandle = GetStdHandle(STD_INPUT_HANDLE);

    while (ReadFile(inhandle, idata->buffer, sizeof(idata->buffer),
                    &idata->len, NULL) && idata->len > 0)
    {
        SetEvent(idata->event);
        WaitForSingleObject(idata->eventback, INFINITE);
    }

    idata->len = 0;
    SetEvent(idata->event);

    return 0;
}

void fatalbox (char *p, ...)
{
    va_list ap;
    fprintf(stderr, "FATAL ERROR: ");
    va_start(ap, p);
    vfprintf(stderr, p, ap);
    va_end(ap);
    fputc('\n', stderr);
    if(initSockets_g)
       WSACleanup();
    exit(1);
}
void connection_fatal (char *p, ...)
{
    va_list ap;
    fprintf(stderr, "FATAL ERROR: ");
    va_start(ap, p);
    vfprintf(stderr, p, ap);
    va_end(ap);
    fputc('\n', stderr);
    if(initSockets_g)
        WSACleanup();
    exit(1);
}

void
logevent(char *string)
{
}

void
begin_session(void)
{
}

void
from_backend(int is_stderr, char *data, int len)
{
    int pos;
    DWORD ret;
    HANDLE h;

    if(is_stderr)
        h = GetStdHandle(STD_ERROR_HANDLE);
    else
        h = GetStdHandle(STD_OUTPUT_HANDLE);

    pos = 0;
    while (pos < len)
    {
        if (!WriteFile(h, data+pos, len-pos, &ret, NULL))
            return;                    /* give up in panic */
        pos += ret;
    }
}

char *
do_select(SOCKET skt, int startup)
{
    int events;
    if (startup)
        events = FD_READ | FD_WRITE | FD_OOB | FD_CLOSE;
    else
        events = 0;

    if (WSAEventSelect (skt, netevent, events) == SOCKET_ERROR)
    {
        switch (WSAGetLastError())
        {
          case WSAENETDOWN:
              return "Network is down";
          default:
              return "WSAAsyncSelect(): unknown error";
        }
    }

    return NULL;
}

void
verify_ssh_host_key(char *host, int port, char *keytype,
    char *keystr, char *fingerprint)
{
    int ret;

    /*
     * Verify the key against the registry.
     */
    ret = verify_host_key(host, port, keytype, keystr);

    if (ret > 0)
    {   /* key was different or absent, store the new key. */
        store_host_key(host, port, keytype, keystr);
    }
}
