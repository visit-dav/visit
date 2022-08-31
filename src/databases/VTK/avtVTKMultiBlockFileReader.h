// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTKMultiBlockFileReader.h
// ****************************************************************************

#ifndef AVT_VTKMultiBlock_FILE_READER_H
#define AVT_VTKMultiBlock_FILE_READER_H

#include <avtVTKFileReaderBase.h>

#include <array>
#include <string>
#include <vector>


// ****************************************************************************
//  Class: avtVTKMultiBlockFileReader
//
//  Purpose:
//      Base class for vtk multi block file readers.
//
//  Notes: Originally part of the avtVTKFileReader class.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

class avtVTKMultiBlockFileReader : public avtVTKFileReaderBase
{
  public:
    avtVTKMultiBlockFileReader(const char *, const DBOptionsAttributes *);
    ~avtVTKMultiBlockFileReader();

    int           GetNumberOfDomains();

    vtkDataSet   *GetMesh(int, const char *);
    vtkDataArray *GetVar(int, const char *);
    void         *GetAuxiliaryData(const char *var, int,
                                   const char *type, void *, DestructorFunction &df);

    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *);

    bool          IsEmpty();

    virtual void   FreeUpResources(void);

    double        GetTime(void);
    int           GetCycle(void);

  protected:

    std::string              filename;

    bool                     haveReadFile;
    bool                     haveReadDataset;

    int                      nblocks;
    std::string              blockPieceName;
    std::vector<std::string> blockNames;

    std::vector<std::string> pieceFileNames;
    std::vector<vtkDataSet *> pieceDatasets;
    std::vector<std::array<int,6> > pieceExtents;

    virtual void          ReadInFile(int _domain=-1) = 0;
    void                  ReadInDataset(int domain);
};


#endif


