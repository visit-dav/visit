#include <ADIOS2HelperFuncs.h>

std::string ADIOS2Helper_GetEngineName(const std::string &fname)
{
    if (fname.find(".bp.sst") != std::string::npos)
        return "SST";
    if (fname.find(".bp.ssc") != std::string::npos)
        return "WDM";
    if (fname.find(".h5") != std::string::npos)
        return "HDF5";
    else
        return "BPFile";
}

std::string ADIOS2Helper_GetFileName(const std::string &fname)
{
    if (fname.find(".bp.sst") != std::string::npos)
        return fname.substr(0, fname.size()-4);
    else if (fname.find(".bp.ssc") != std::string::npos)
        return fname.substr(0, fname.size()-4);

    return fname;
}

bool ADIOS2Helper_IsStagingEngine(const std::string &engineName)
{
    if (engineName == "SST" || engineName == "WDM" )
        return true;
    else
        return false;
}

