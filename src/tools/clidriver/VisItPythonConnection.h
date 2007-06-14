#ifndef VISIT_PYTHON_CONNECTION
#define VISIT_PYTHON_CONNECTION

#include <string>
#include <sys/types.h>

// ****************************************************************************
//  Class:  VisItPythonConnection
//
//  Purpose:
//    Raw connection to VisIt's Python command line interface.
//    Allows opening, closing and sending raw strings.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 12, 2007
//
// ****************************************************************************
class VisItPythonConnection
{
  public:
                VisItPythonConnection();
               ~VisItPythonConnection();

    // Open the connection; uses VISITHOME or PATH to find visit.
    bool        Open();
    // Close the connection.  Sends EOF to the Python CLI as well.
    bool        Close();
    // Send a raw command.  This method appends the newline for you.
    bool        SendCommand(const char *buff);
    // Return true if we have a valid open connection.
    bool        IsOpen();
    // When a function returns an error (false), get the error string.
    std::string GetLastError();

  protected:
    bool        WriteString(const char *buff);
    bool        WaitForPrompt();
  private:
    int         to_cli;
    int         from_cli;

    int         readbufferlen;
    char       *readbuffer;

    pid_t       visitpid;
    std::string error;
};

#endif
