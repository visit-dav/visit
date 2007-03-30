#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <gui_exports.h>

#if 0
#include <stdio.h>
#include <string>
#include <map>
#endif

// *******************************************************************
// Class: Interpreter
//
// Purpose:
//   This is a veneer over the Python interpreter. It encapsulates
//   writing the command string to a file and recording macros.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 15:32:50 PST 2000
//
// Modifications:
//   
// *******************************************************************

class GUI_API Interpreter
{
public:
    Interpreter(const char *filename, int argc, char *argv[]);
    ~Interpreter();
    void ExecuteString(const char *str);
    void ExecuteFile(const char *filename);
    void ExecuteMacro(const char *macro);
    void BeginMacro(const char *macro);
    void EndMacro();
    void RedefineMacro(const char *macro, const char *body);
    void RemoveMacro(const char *macro);
    void WriteLog(const char *str);
private:
#if 0
    FILE                               *logFile;
    bool                               recordingMacro;
    std::string                        macroName, macroString;
    std::map<std::string, std::string> macroList;
#endif
};

#endif
