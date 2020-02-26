// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_ZIPSTREAM_H
#define VISIT_ZIPSTREAM_H

#include "visit-config.h"

// standard C++ with new header file names and std:: namespace
#include <iostream>
#include <fstream>
#include <cstring>

#include "utility_exports.h"

class UTILITY_API visit_ifstream {
public:
   /** Simple factory to create ifstream or other streams depending on whether
       named file appears to be a compressed file. */
    visit_ifstream() : strm(0),ifstrm(0) {};
    visit_ifstream(char const *name, char const *mode = "r") {
        std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
        if (strchr(mode,'r')) m |= std::ios_base::in;
        if (strchr(mode,'b')) m |= std::ios_base::binary;
        ifstrm = new std::ifstream(name,m);
        strm = ifstrm;
    };
    void open(char const *name, char const *mode = "rb") {
        if (ifstrm) delete ifstrm;
        std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
        if (strchr(mode,'r')) m |= std::ios_base::in;
        if (strchr(mode,'b')) m |= std::ios_base::binary;
        ifstrm = new std::ifstream(name,m);
        strm = ifstrm;
    };
    std::istream &operator()(void) const { return *strm; };
    int peek(void)
    {
        if (ifstrm) return ifstrm->peek();
        return 0;
   };
    void close(void)
    {
        if (ifstrm) ifstrm->close();
    };
   ~visit_ifstream() { delete strm; };
private:
   visit_ifstream(const visit_ifstream&);
   std::istream *strm;
   std::ifstream *ifstrm;
   static bool maybe_gz(const char *fn) {
       unsigned short hdr = 0x0000;
       std::ifstream strm(fn,std::ios_base::binary|std::ios_base::in);
       strm.read(reinterpret_cast<char*>(&hdr),2);
       if (hdr==0x1f8b||hdr==0x8b1f) return true;
       return false;}
};

class UTILITY_API visit_ofstream {
public:
   /** Simple factory to create ofstream or zipp'd ofstream depending on mode.
       The mode chars are as in gzopen() with additional caveat that presence of a 'z'
       character indicates desire to compress and absence
       indicates a desire to use std::ofstream.
       https://refspecs.linuxbase.org/LSB_3.0.0/LSB-PDA/LSB-PDA/zlib-gzopen-1.html.
       Default mode is "zwb6". */
    visit_ofstream() : strm(0) {};
    visit_ofstream(char const *name, char const *mode = "w") {
        std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
        if (strchr(mode,'a')) m |= std::ios_base::app;
        else if (strchr(mode,'t')) m |= std::ios_base::trunc;
        if (strchr(mode,'w')) m |= std::ios_base::out;
        if (strchr(mode,'b')) m |= std::ios_base::binary;
        strm = new std::ofstream(name,m);
    };
    void open(char const *name, char const *mode = "w") {
        if (strm) delete strm;
        std::ios_base::openmode m = (std::ios_base::openmode) 0x0;
        if (strchr(mode,'a')) m |= std::ios_base::app;
        else if (strchr(mode,'t')) m |= std::ios_base::trunc;
        if (strchr(mode,'w')) m |= std::ios_base::out;
        if (strchr(mode,'b')) m |= std::ios_base::binary;
        strm = new std::ofstream(name,m);
    };
    std::ostream &operator()(void) const { return *strm; };
   ~visit_ofstream() { delete strm; };
private:
   visit_ofstream(const visit_ofstream &);
   std::ostream *strm;
};

#endif
