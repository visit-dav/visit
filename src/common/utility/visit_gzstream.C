// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "visit_gzstream.h"

visit_ifstream::visit_ifstream(char const *name, char const *mode)
{
    std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
    if (strchr(mode,'r')) m |= std::ios_base::in;
    if (strchr(mode,'b')) m |= std::ios_base::binary;
    ifstrm = new std::ifstream(name,m);
    strm = ifstrm;
}


void
visit_ifstream::open(char const *name, char const *mode)
{
    if (ifstrm) delete ifstrm;
    std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
    if (strchr(mode,'r')) m |= std::ios_base::in;
    if (strchr(mode,'b')) m |= std::ios_base::binary;
    ifstrm = new std::ifstream(name,m);
    strm = ifstrm;
}


visit_ofstream::visit_ofstream(char const *name, char const *mode)
{
    std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
    if (strchr(mode,'a')) m |= std::ios_base::app;
    else if (strchr(mode,'t')) m |= std::ios_base::trunc;
    if (strchr(mode,'w')) m |= std::ios_base::out;
    if (strchr(mode,'b')) m |= std::ios_base::binary;
    strm = new std::ofstream(name,m);
}


void
visit_ofstream::open(char const *name, char const *mode)
{
    if (strm) delete strm;
    std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
    if (strchr(mode,'a')) m |= std::ios_base::app;
    else if (strchr(mode,'t')) m |= std::ios_base::trunc;
    if (strchr(mode,'w')) m |= std::ios_base::out;
    if (strchr(mode,'b')) m |= std::ios_base::binary;
    strm = new std::ofstream(name,m);
}
