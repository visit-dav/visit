// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//    GEOSXParser.h
// ************************************************************************* //

#ifndef GEOSX_PARSER_H
#define GEOSX_PARSER_H

#include <set>
#include <string>
#include <vector>

class vtkXMLDataElement;
class vtkXMLDataParser;

// ****************************************************************************
//  Class: GEOSXParser
//
//  Purpose:
//    Parses GEOSX style .vtm (vtk multi block) files.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 11, 2022
//
//  Modifications:
//
// ****************************************************************************

class GEOSXParser
{
  public:
                              GEOSXParser();
    virtual                  ~GEOSXParser();

    void                      SetFileName(const std::string &);

    bool                      Parse();

    std::string              &GetErrorMessage()
                                { return errorMessage; }

    int                       GetNumberOfDataSets();
    std::set<std::string>    &GetMeshNames();
    int                       GetNumberOfBlocks(const std::string &meshName);
    std::string              &GetBlockFileName(const std::string &meshName,
                                               const int blockNum);
    int                       GetDataSetIndex(const std::string &meshName,
                                              const int blockNum);
    std::vector<std::string>  &GetMeshGroupNames(const std::string &meshName);
    std::vector<int>          &GetMeshGroupIds(const std::string &meshName);
    std::vector<int>          &GetMeshBlockPerGroup(const std::string &meshName);
    std::vector<std::string>  &GetMeshBlockNames(const std::string &meshName);

  private:

    std::string               fileName;
    std::string               baseDir;
    std::string               blockExtension;
    std::string               blockName;

    std::string               errorMessage;
    std::string               temp;

    GEOSXParser(const GEOSXParser &) = delete;
    void operator=(const GEOSXParser &) = delete;
};

#endif
