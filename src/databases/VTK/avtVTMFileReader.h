// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTMFileReader.h
// ****************************************************************************

#ifndef AVT_VTM_FILE_READER_H
#define AVT_VTM_FILE_READER_H

#include <avtVTKMultiGroupFileReader.h>


// ****************************************************************************
//  Class: avtVTMFileReader
//
//  Purpose:
//      Handles files of the .vtm file format.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

class avtVTMFileReader : public avtVTKMultiGroupFileReader
{
  public:
    avtVTMFileReader(const char *, const DBOptionsAttributes *);
    ~avtVTMFileReader();

  protected:
    void          ReadInFile(int _domain=-1) override;
};


#endif


