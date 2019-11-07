#include <ADIOS2HelperFuncs.h>

std::string ADIOS2Helper_GetEngineName(const std::string &fname)
{
    if (fname.find(".bp.sst") != std::string::npos)
        return "SST";
    if (fname.find(".bp.ssc") != std::string::npos)
        return "WDM";
    if (fname.find(".h5") != std::string::npos)
        return "HDF5";
    if (fname.find(".bp/md.idx") != std::string::npos)
        return "BPFile";
    if (fname.find(".bp/md.0") != std::string::npos)
        return "BPFile";

	if (ADIOS2Helper_FileIsDirectory(fname))
	    return "BP4";
    else
        return "BPFile";
}

std::string ADIOS2Helper_GetFileName(const std::string &fname)
{
    if (fname.find(".bp.sst") != std::string::npos)
        return fname.substr(0, fname.size()-4);
    else if (fname.find(".bp.ssc") != std::string::npos)
        return fname.substr(0, fname.size()-4);
    if (fname.find(".bp/md.idx") != std::string::npos)
        return fname.substr(0, fname.size()-7);
    if (fname.find(".bp/md.0") != std::string::npos)
        return fname.substr(0, fname.size()-5);

    return fname;
}

bool ADIOS2Helper_IsStagingEngine(const std::string &engineName)
{
    if (engineName == "SST" || engineName == "WDM")
        return true;
    else
        return false;
}


// Windows API.
#if defined(_WIN32)

#include <windows.h>
#include <winioctl.h>
#include <string.h>
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
#if defined(_MSC_VER) && _MSC_VER >= 1800
#define KWSYS_WINDOWS_DEPRECATED_GetVersionEx
#endif

#elif defined(__CYGWIN__)

#include <windows.h>
#include <string.h>
#undef _WIN32

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#endif // _WIN32 or __CYGWIN__ or else



bool ADIOS2Helper_FileIsDirectory(const std::string& inName)
{
  if (inName.empty()) {
    return false;
  }
  size_t length = inName.size();
  const char* name = inName.c_str();

  // Remove any trailing slash from the name except in a root component.
  char local_buffer[256];
  std::string string_buffer;
  size_t last = length - 1;
  if (last > 0 && (name[last] == '/' || name[last] == '\\') &&
      strcmp(name, "/") != 0 && name[last - 1] != ':') {
    if (last < sizeof(local_buffer)) {
      memcpy(local_buffer, name, last);
      local_buffer[last] = '\0';
      name = local_buffer;
    } else {
      string_buffer.append(name, last);
      name = string_buffer.c_str();
    }
  }

// Now check the file node type.
#if defined(_WIN32)
  DWORD attr =
    GetFileAttributesW(Encoding::ToWindowsExtendedPath(name).c_str());
  if (attr != INVALID_FILE_ATTRIBUTES) {
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  struct stat fs;
  if (stat(name, &fs) == 0) {
    return S_ISDIR(fs.st_mode);
#endif
  } else {
    return false;
  }
}
