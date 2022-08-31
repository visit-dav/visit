// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtPVTKFileReader.h
// ****************************************************************************

#ifndef AVT_PVTK_FILE_READER_H
#define AVT_PVTK_FILE_READER_H

#include <avtVTKMultiBlockFileReader.h>


// ****************************************************************************
//  Class: avtPVTKFileReader
//
//  Purpose:
//      Handles files of the .pvt? file format.
//
//  Notes: Originally part of the avtVTKFileReader class.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

class avtPVTKFileReader : public avtVTKMultiBlockFileReader
{
  public:
    avtPVTKFileReader(const char *, const DBOptionsAttributes *);
    ~avtPVTKFileReader();


  protected:
    virtual void  ReadInFile(int _domain=-1);
    std::string   fileExtension;
};


#endif


