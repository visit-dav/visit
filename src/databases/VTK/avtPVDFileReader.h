// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//   avtPVDFileReader.h
// ************************************************************************* //

#ifndef AVT_PVD_FILE_READER_H
#define AVT_PVD_FILE_READER_H

#include <avtVTKMultiGroupFileReader.h>
#include <array>
#include <map>
#include <string>
#include <vector>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtPVDFileReader
//
//  Purpose:
//      Parses files of the .pvd file format.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 13, 2021
//
//  Modifications:
//    Kathleen Biagas, June 22, 2022
//    Change inheritance from avtVTKFilerReader to avtVTKMultiGroupFileReader.
//
// ****************************************************************************

class avtPVDFileReader : public avtVTKMultiGroupFileReader
{
  public:
            avtPVDFileReader(const char *, const DBOptionsAttributes *);
           ~avtPVDFileReader();

    // Override avtVTKMultiGroupFileReader
    void    FreeUpResources(void) override;
    void    PopulateDatabaseMetaData(avtDatabaseMetaData *) override;
    void    ActivateTimestep(int ts);

    // Info obtained from parsing the PVD file
    int     GetNTimes(void);
    int     GetNBlocks(int ts=0);
    void    GetTimes(std::vector<double> &_times) { _times = times; }

  protected:

    // Override avtVTKMultiGroupFileReader
    // Throws an exception if it enounters a problem indicating
    // the pvdFile may not be formatted as valid pvd.
    void    ReadInFile(int _domain=-1) override;

  private:
    std::vector<int>          dataFilePartIds;
    std::vector<double>       dataFileTimes;
    std::vector<std::string>  dataFileNames;
    std::map<double, std::vector<std::string> >  timeBlockMap;
    std::map<double, std::vector<std::array<int,6> > >  timePieceMap;
    std::vector<std::array<int,6> >  allPieceExtents;

    std::vector<double>       times;

    int                       currentTS;
};

#endif

