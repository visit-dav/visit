// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                PVTKParser.h                               //
// ************************************************************************* //

#ifndef PVTK_PARSER_H
#define PVTK_PARSER_H

#include <string>
#include <vector>

class vtkXMLDataParser;

// ****************************************************************************
//  Class: PVTKParser
//
//  Purpose:
//    Parses .pvtk (partitioned legacy vtk) files.
//
//  Programmer: Kathleen Biagas
//  Creation:   September 21, 2017
//
//  Modifications:
//
// ****************************************************************************

class PVTKParser
{
  public:
                              PVTKParser();
    virtual                  ~PVTKParser();

    void                      SetFileName(const char *);
    bool                      Parse();

    bool                      HasExtents() { return hasExtents; }

    size_t                    GetNumberOfPieces();
    std::string              &GetPieceFileName(size_t index);
    std::vector< int>        &GetPieceExtent(size_t index);

    std::string              &GetErrorMessage()
                                { return errorMessage; }

  private:

    std::string               fileName;
    std::string               baseDir;
    std::string               pieceName;
    std::vector<std::string>  pieceFileNames;
    std::vector< int >        pieceExtent;
    std::vector< std::vector <int> >  pieceExtents;
    bool                      hasExtents;

    vtkXMLDataParser         *parser;

    std::string               errorMessage;

    PVTKParser(const PVTKParser &);      // Not implemented
    void operator=(const PVTKParser &); // Not implemented
};

#endif
