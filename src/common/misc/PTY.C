#include "PTY.h"

#ifdef USE_PTY

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#if !defined(TIOCGWINSZ) || defined(__APPLE__)
#include <sys/ioctl.h>   // 44BSD requires this too
#endif

// ****************************************************************************
//  Function:  pty_fork
//
//  Purpose:
//    Like fork(), but creates a PTY and returns its file descriptor/name.
//
//  Arguments:
//    fdm       : new file descriptor
//    slave_name: new pty path
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 22 15:00:37 PDT 2002
//    Added an optional signal child handler.  Since setting this must happen
//    before fork() and after grantpt(), it must be set in this function.
//
// ****************************************************************************
pid_t
pty_fork(int &fdm, void (*sigchld_handler)(int), char *slave_name)
{
    int         fds;
    pid_t       pid;
    char        pts_name[20];

    if ( (fdm = ptym_open(pts_name)) < 0)
        fprintf(stderr,"can't open master pty: %s", pts_name);

    if (slave_name != NULL)
        strcpy(slave_name, pts_name);   // return name of slave

    if (sigchld_handler != NULL)
        signal(SIGCHLD, sigchld_handler);

    if ( (pid = fork()) < 0)
        return(-1);

    else if (pid == 0) {                // child
        if (setsid() < 0)
            fprintf(stderr,"setsid error");

        // SVR4 acquires controlling terminal on open()
        if ( (fds = ptys_open(fdm, pts_name)) < 0)
            fprintf(stderr,"can't open slave pty: %d\n",fds);
        close(fdm);             // all done with master in child

#if     defined(TIOCSCTTY) && !defined(CIBAUD)
        // 44BSD way to acquire controlling terminal
        // !CIBAUD to avoid doing this under SunOS
        if (ioctl(fds, TIOCSCTTY, (char *) 0) < 0)
            fprintf(stderr,"TIOCSCTTY error");
#endif
        // slave becomes stdin/stdout/stderr of child
        if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
            fprintf(stderr,"dup2 error to stdin");
        if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
            fprintf(stderr,"dup2 error to stdout");
        if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
            fprintf(stderr,"dup2 error to stderr");
        if (fds > STDERR_FILENO)
            close(fds);
        return(0);              // child returns 0 just like fork()

    } else {            // parent
        return(pid);    // return pid of child
    }
}


#ifdef PTY_SYSV

#include        <stropts.h>

// ****************************************************************************
//  Method:  ptym_open  (SYSV)
//
//  Purpose:
//    Open the pty master.
//
//  Arguments:
//    pts_name:  output pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
// ****************************************************************************
#include <iostream.h>
int
ptym_open(char *pts_name)
{
    char *ptr;
    int   fdm;

    strcpy(pts_name, "/dev/ptmx");  // in case open fails
    if ( (fdm = open(pts_name, O_RDWR)) < 0)
        return(-1);
    if (grantpt(fdm) < 0) {         // grant access to slave
        close(fdm);
        return(-2);
    }
    if (unlockpt(fdm) < 0) {        // clear slave's lock flag
        close(fdm);
        return(-3);
    }
    if ( (ptr = ptsname(fdm)) == NULL) {    // get slave's name
        close(fdm);
        return(-4);
    }

    strcpy(pts_name, ptr);  // return name of slave
    return(fdm);            // return fd of master
}

// ****************************************************************************
//  Method:  ptys_open  (SYSV)
//
//  Purpose:
//    Open the pty slave
//
//  Arguments:
//    fdm      : the master file descriptor
//    pts_name : pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu May 17 11:32:26 PDT 2001
//    Removed a line of debug output.
//
// ****************************************************************************
int
ptys_open(int fdm, char *pts_name)
{
    int             fds;

    // following should allocate controlling terminal
    if ( (fds = open(pts_name, O_RDWR)) < 0) {
        close(fdm);
        return(-5);
    }
    if (ioctl(fds, I_PUSH, "ptem") < 0) {
        close(fdm);
        close(fds);
        return(-6);
    }
    if (ioctl(fds, I_PUSH, "ldterm") < 0) {
        close(fdm);
        close(fds);
        return(-7);
    }
    /*
    THIS IS UNSUPPORTED ON SEVERAL PLATFORMS AND GENERATES AN ERROR
    if (ioctl(fds, I_PUSH, "ttcompat") < 0) {
        close(fdm);
        close(fds);
        return(-8);
    }
    */

    return(fds);
}


#else

// ****************************************************************************
//  Method:  ptym_open  (BSD)
//
//  Purpose:
//    Open the pty master.
//
//  Arguments:
//    pts_name:  output pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
// ****************************************************************************
int
ptym_open(char *pts_name)
{
    int         fdm;
    char        *ptr1, *ptr2;

    strcpy(pts_name, "/dev/ptyXY");
    // array index: 0123456789 (for references in following code)
    for (ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++) {
        pts_name[8] = *ptr1;
        for (ptr2 = "0123456789abcdef"; *ptr2 != 0; ptr2++) {
            pts_name[9] = *ptr2;

            // try to open master
            if ( (fdm = open(pts_name, O_RDWR)) < 0) {
                if (errno == ENOENT)    // different from EIO
                    return(-1);         // out of pty devices
                else
                    continue;           // try next pty device
            }

            pts_name[5] = 't';  // change "pty" to "tty"
            return(fdm);        // got it, return fd of master
        }
    }
    return(-1);         // out of pty devices
}

// ****************************************************************************
//  Method:  ptys_open  (BSD)
//
//  Purpose:
//    Open the pty slave
//
//  Arguments:
//    fdm      : the master file descriptor
//    pts_name : pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
// ****************************************************************************
int
ptys_open(int fdm, char *pts_name)
{
    int fds;

    if ( (fds = open(pts_name, O_RDWR)) < 0) {
        close(fdm);
        return(-1);
    }
    return(fds);
}




#endif


#endif
