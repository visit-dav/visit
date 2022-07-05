// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTKMultiGroupFileReader.h
// ****************************************************************************

#ifndef AVT_VTK_MULTIGROUP_FILE_READER_H
#define AVT_VTK_MULTIGROUP_FILE_READER_H

#include <avtVTKMultiBlockFileReader.h>


// ****************************************************************************
//  Class: avtVTKMultiGroupFileReader
//
//  Purpose:
//    A base class for multi-group VTK file readers.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

class avtVTKMultiGroupFileReader : public avtVTKMultiBlockFileReader
{
  public:
    avtVTKMultiGroupFileReader(const char *, const DBOptionsAttributes *);
    ~avtVTKMultiGroupFileReader();

    void   PopulateDatabaseMetaData(avtDatabaseMetaData *md) override;

  protected:
    int                      ngroups;
    std::string              groupPieceName;
    std::vector<std::string> groupNames;
    std::vector<int>         groupIds;

    void          ReadInFile(int _domain=-1) override = 0;
};

#endif


