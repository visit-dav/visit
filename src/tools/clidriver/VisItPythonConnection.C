#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

#include "VisItPythonConnection.h"


// ****************************************************************************
//  Constructor:  VisItPythonConnection::VisItPythonConnection
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
VisItPythonConnection::VisItPythonConnection()
{
    visitpid = -1;

    // Create a buffer for reading
    readbufferlen = 100000;
    readbuffer = new char[readbufferlen];
}

// ****************************************************************************
//  Destructor:  VisItPythonConnection::VisItPythonConnection
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
VisItPythonConnection::~VisItPythonConnection()
{
    Close();

    delete[] readbuffer;
    readbuffer = NULL;
}


// ****************************************************************************
//  Method:  WriteString
//
//  Purpose:
//    Internal method to write a string the the CLI's input file descriptor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::WriteString(const char *buff)
{
    // write isn't guaranteed to send the whole buffer; force it
    size_t nleft = strlen(buff);
    const char *ptr = (const char*)buff;
    while (nleft > 0)
    {
        size_t nwritten;
        if ((nwritten = write(to_cli, ptr, nleft)) <= 0)
            break;

        nleft -= nwritten;
        ptr   += nwritten;
    }

    // if we couldn't write the whole thing, there was an error
    if (nleft > 0)
    {
        error = "could not write to CLI";
        return false;
    }

    return true;
}

// ****************************************************************************
//  Method:  WaitForPrompt
//
//  Purpose:
//    Internal method to read output from the CLI until we get a prompt.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::WaitForPrompt()
{
    char *pbuf = readbuffer;
    int   len  = 0;
    while (true)
    {
        // Read some text
        int nread = read(from_cli, pbuf, readbufferlen - len);
        if (nread <= 0)
        {
            error = "read from CLI failed";
            return false;
        }

        // Enable for debugging
        //write(fileno(stderr), pbuf, nread);

        // Advance pbuf and keep track of the total length read
        len  += nread;
        pbuf += nread;
        *pbuf = '\0';

        // Check for the prompt string.  We could probably just check the
        // last four characters, not search the whole string, but this is
        // easy and might be more robust.
        if (strstr(readbuffer, ">>> "))
        {
            return true;
        }

        // To avoid buffer overruns and speed up searches for the prompt,
        // cycle around.  Avoid the situation where the prompt is partially
        // printed at the end of the read buffer.
        if (readbuffer[len-1] != '>')
        {
            pbuf = readbuffer;
            len  = 0;
        }
    }
}

// ****************************************************************************
//  Method:  Open
//
//  Purpose:
//    Start VisIt's CLI.  Finds visit either in $VISITHOME/bin/visit or through
//    the user's PATH.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::Open()
{
    // Make sure it's not already open
    if (visitpid != -1)
    {
        error = "error opening CLI; already open";
        return false;
    }

    // Create a couple pipes for I/O to the CLI
    int p1[2], p2[2];
    if (pipe(p1)<0 || pipe(p2)<0)
    {
        error = "setting up pipes failed";
        return false;
    }

    // Set them nonblocking.  This appears to be unnecessary
    //fcntl(p1[0], F_SETFL, fcntl(p1[0], F_GETFL) & ~O_NONBLOCK);
    //fcntl(p1[1], F_SETFL, fcntl(p1[1], F_GETFL) & ~O_NONBLOCK);
    //fcntl(p2[0], F_SETFL, fcntl(p2[0], F_GETFL) & ~O_NONBLOCK);
    //fcntl(p2[1], F_SETFL, fcntl(p2[1], F_GETFL) & ~O_NONBLOCK);

    // descriptors for the child
    int from_sim  = p1[0];
    int to_sim    = p2[1];

    // descriptors for the parent
    to_cli   = p1[1];
    from_cli = p2[0];

    // fork
    visitpid = fork();
    if (visitpid < 0)
    {
        error = "fork failed";
        return false;
    }

    if (visitpid == 0)
    {
        // We're the child; close the to/from_cli descriptors
        close(from_cli);
        close(to_cli);

        // Use the to/from_sim descriptors for stdin/out/err
        dup2(from_sim, fileno(stdin));
        dup2(to_sim, fileno(stdout));
        dup2(to_sim, fileno(stderr));

        // And close the originals
        close(from_sim);
        close(to_sim);

        char visitpath[1024] = "visit";
        char *visithome = getenv("VISITHOME");
        if (visithome)
        {
            sprintf(visitpath, "%s/bin/visit", visithome);
        }
        execlp(visitpath, visitpath, "-forceinteractivecli", "-cli", NULL);
        // exec of visit failed
        exit(1);
    }

    // We're the simulation; we'll use the to/from_cli descriptors
    // and close the to/from_sim ones for the CLI to use.
    close(from_sim);
    close(to_sim);

    // Wait for that first prompt.  This is the best time to make sure
    // visit got launched successfully.
    if (!WaitForPrompt())
    {
        error = "launch of visit failed";
        return false;
    }

    // Success!
    return true;
}

// ****************************************************************************
//  Method:  Close
//
//  Purpose:
//    Close the connection to the CLI.  Also closes file descriptors, which
//    should tell VisIt's Python interface to exit.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::Close()
{
    // don't try to close if we're not open
    if (visitpid == -1)
    {
        // okay, maybe flagging this as an error is excessive, but still....
        error = "close failed; CLI was not launched";
        return false;
    }

    close(to_cli);
    close(from_cli);

    // Probably unnecessary to kill and wait for the child
    //kill(visitpid, SIGHUP);
    //waitpid(visitpid, NULL, 0) << endl;

    to_cli   = -1;
    from_cli = -1;
    visitpid = -1;
    return true;
}

// ****************************************************************************
//  Method:  SendCommand
//
//  Purpose:
//    Send a command, then send a newline, then wait for the prompt to return.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::SendCommand(const char *buff)
{
    // Make sure we've opened visit's CLI before trying this
    if (visitpid == -1)
    {
        error = "send command failed; CLI was not launched";
        return false;
    }

    // Stupid signal.  We can detect errors ourselves, thankyouverymuch.
    // signal(SIGPIPE, SIG_IGN);
    // Alas, GDB interferes with signals, so trying to SIG_IGN this isn't
    // a good idea.  Instead, we assume the CLI is behaving normally and
    // we should never have anything to read on the to_cli socket.
    struct timeval zerotimeout = {0,0};
    fd_set errorset;
    FD_ZERO(&errorset);
    FD_SET(to_cli, &errorset);
    if (select(to_cli+1,&errorset, NULL, NULL, &zerotimeout))
    {
        error = "data sent from CLI on wrong pipe; write error assumed";
        return false;
    }

    // Write the string
    if (!WriteString(buff))
        return false;

    // Write a newline
    if (!WriteString("\n"))
        return false;

    // Wait for the prompt to return
    if (!WaitForPrompt())
        return false;

    // Enable for debugging
    //fprintf(stderr, "Sent command %s\n",buff);

    // Success!
    return true;
}

// ****************************************************************************
//  Method:  GetLastError
//
//  Purpose:
//    When one of the public functions returns false, it sets an internal
//    error string.  This method retrieves the error string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
std::string VisItPythonConnection::GetLastError()
{
    // Clear the error.
    std::string retval = error;
    error = "";
    return retval;
}

// ****************************************************************************
//  Method:  IsOpen
//
//  Purpose:
//    Return true of the connection is open and valid.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
bool VisItPythonConnection::IsOpen()
{
    return (visitpid > 0);
}
