#if 0
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <Python.h>
#endif

#include <Interpreter.h>

// *******************************************************************
// Method: Interpreter::Interpreter
//
// Purpose: 
//   Constructor for the Interpreter class. It initializes the Python
//   interpreter and writes program usage information to the log file.
//
// Arguments:
//   filename : The name of the log file to which the commands will
//              be written.
//   argc     : The number of command line arguments for the program.
//   argv     : A list of strings containing the command line args.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:17:48 PST 2000
//
// Modifications:
//   
// *******************************************************************

Interpreter::Interpreter(const char *filename, int argc, char *argv[])
#if 0
 : macroList(), macroName(), macroString()
#endif
{
#if 0
    recordingMacro = false;

    // Open the log file.
    logFile = fopen(filename, "wb");

    // Write the usage information to the log file.
    if(logFile && argv)
    {
        std::string str;
#ifndef WIN32
        char        buf[200];
        std::string host;

        // Get the host name.
        host = (gethostname(buf, 200) == -1) ? "unknown" : buf;
        str = "# " + std::string(argv[0]) + " run on " + host + "\n";
        WriteLog(str.c_str());
#endif
        // Write the command line arguments to the log
        str = "# Command line: ";
        for(int i = 0; i < argc; ++i)
        {
            str += argv[i];
            str += " ";
        }
        str += "\n";
        WriteLog(str.c_str());
    }

    // Initialize the Python interpreter.
    Py_SetProgramName(argv[0]);
    Py_Initialize();

    // Set up true, false until I know whether or not Python has boolean.
    ExecuteString("true=1");
    ExecuteString("false=0");
#endif
}

// *******************************************************************
// Method: Interpreter::~Interpreter
//
// Purpose: 
//   Destructor for the Interpreter class. It closes the log file if
//   it is still open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:17:01 PST 2000
//
// Modifications:
//   
// *******************************************************************

Interpreter::~Interpreter()
{
#if 0
    // Close the log file if it is open.
    if(logFile != NULL)
        fclose(logFile);

    Py_Finalize();
#endif
}

// *******************************************************************
// Method: Interpreter::WriteLog
//
// Purpose: 
//   Writes a string to the log file.
//
// Arguments:
//   str : The string that we want to write to the log.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:15:36 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::WriteLog(const char *str)
{
#if 0
    std::string command = std::string(str);

    // Add a newline character to the command if it does not have one.
    if(command[command.length() - 1] != '\n')
        command += "\n";

    fprintf(logFile, "%s", command.c_str());
#endif
}

// *******************************************************************
// Method: Interpreter::ExecuteString
//
// Purpose: 
//   Sends a command string to the interpreter, logfile, and macro
//   string.
//
// Arguments:
//    str : The command that we're going to execute.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:14:30 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::ExecuteString(const char *str)
{
#if 0
    // If a log file is open, write the string to the log.
    if(logFile)
        WriteLog(str);

    // Add a newline character to the command if it does not have one.
    std::string command = std::string(str);
    if(command[command.length() - 1] != '\n')
        command += "\n";

    // If we're recording a macro, add the string to the macro.
    if(recordingMacro)
    {
        macroString += command;
    }

    // Send the string to the Python interpreter.
    PyRun_SimpleString((char *)command.c_str());
#endif
}

// *******************************************************************
// Method: Interpreter::ExecuteFile
//
// Purpose: 
//   Calls the interpreter for all lines in the file. This should be
//   finished later... It's contents should be added to the log.
//
// Arguments:
//   filename : The name of the file to execute.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:11:24 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::ExecuteFile(const char *filename)
{
#if 0
    FILE *fp = fopen(filename, "rb");
    if(fp != NULL)
    {
        PyRun_SimpleFile(fp, (char *)filename);
        fclose(fp);
    }
#endif
}

// *******************************************************************
// Method: Interpreter::ExecuteMacro
//
// Purpose: 
//   Executes the named macro if it exists.
//
// Arguments:
//   macro : The name of the macro to execute.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:10:36 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::ExecuteMacro(const char *macro)
{
#if 0
    std::map<std::string, std::string>::iterator pos;
    std::string name(macro);

    //
    // If the macro name is in the list, execute the macro body.
    //
    if((pos = macroList.find(name)) != macroList.end())
    {
        ExecuteString(pos->second.c_str());
    }
#endif
}

// *******************************************************************
// Method: Interpreter::BeginMacro
//
// Purpose: 
//   Starts recording a macro for the macro list.
//
// Arguments:
//    macro : The name of the new macro.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:09:04 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::BeginMacro(const char *macro)
{
#if 0
    recordingMacro = true;
    macroName = std::string(macro);
    macroString.erase();
#endif
}

// *******************************************************************
// Method: Interpreter::EndMacro
//
// Purpose: 
//   Stops recording the macro and puts the macro in the macro list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:08:24 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::EndMacro()
{
#if 0
    recordingMacro = false;

    //
    // If the macroString is not empty, then add the macro to
    // the macro list.
    //
    if(macroString.length() > 0)
    {
        macroList[macroName] = macroString;
        macroString.erase();
    }
#endif
}

// *******************************************************************
// Method: Interpreter::RedefineMacro
//
// Purpose: 
//   Redefines the macro body for the specified macro.
//
// Arguments:
//   macro : The name of the macro to redefine.
//   body  : The new commands that make up the macro.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:07:19 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::RedefineMacro(const char *macro, const char *body)
{
#if 0
    macroList[std::string(macro)] = std::string(body);
#endif
}

// *******************************************************************
// Method: Interpreter::RemoveMacro
//
// Purpose: 
//   Removes a macro from the macro list.
//
// Arguments:
//    macro : The name of the macro to remove.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:06:39 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Interpreter::RemoveMacro(const char *macro)
{
#if 0
    std::map<std::string, std::string>::iterator pos;

    //
    // If the macro specified by name is in the macro list, then
    // remove it.
    //
    if((pos = macroList.find(std::string(macro))) != macroList.end())
    {
        macroList.erase(pos);
    }
#endif
}



