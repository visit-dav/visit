#ifndef ADIOS2_HELPER_FUNCS_H
#define ADIOS2_HELPER_FUNCS_H

#include <string>

std::string ADIOS2Helper_GetEngineName(const std::string &fname);
std::string ADIOS2Helper_GetFileName(const std::string &fname);
bool        ADIOS2Helper_IsStagingEngine(const std::string &engineName);

bool ADIOS2Helper_FileIsDirectory(const std::string& inName);


#endif
