#ifndef LOGGING_H
#define LOGGING_H
#include <string>

// Macro functions
void Macro_SetString(const std::string &s);
const std::string &Macro_GetString();
void Macro_SetRecord(bool val);
bool Macro_GetRecord();

// LogFile functions
bool LogFile_Open(const char *filename);
void LogFile_Close();
void LogFile_SetEnabled(bool val);
bool LogFile_GetEnabled();
void LogFile_IncreaseLevel();
void LogFile_DecreaseLevel();
void LogFile_Write(const char *str);

// Callback functions
void LogRPCs(Subject *subj, void *);
void SS_log_ViewCurve(const std::string &s);
void SS_log_View2D(const std::string &s);
void SS_log_View3D(const std::string &s);

#endif
