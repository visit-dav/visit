// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                VTMParser.h                               //
// ************************************************************************* //

#ifndef VTM_PARSER_H
#define VTM_PARSER_H

#include <string>
#include <vector>

class vtkXMLDataElement;
class vtkXMLDataParser;

// ****************************************************************************
//  Class: VTMParser
//
//  Purpose:
//    Parses .vtm (vtk multi block) files.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 12, 2015
//
//  Modifications:
//
// ****************************************************************************

class VTMParser
{
  public:
                              VTMParser();
    virtual                  ~VTMParser();

    void                      SetFileName(const std::string &);
    std::string              &GetBlockFileName(int index);
    std::string              &GetBlockExtension()
                                { return blockExtension; }

    int                       GetNumberOfGroups()
                                { return nGroups; }
    std::string              &GetGroupPieceName()
                                { return groupPieceName; }
    std::vector<std::string> &GetGroupNames()
                                { return groupNames; }

    int                       GetNumberOfBlocks();
    std::string              &GetBlockPieceName()
                                { return blockPieceName; }
    std::vector<std::string> &GetBlockNames()
                                { return blockNames; }
    std::vector<int>         &GetGroupIds()
                                { return groupIds; }

    bool                      Parse();

    std::string              &GetErrorMessage()
                                { return errorMessage; }

  private:

    std::string               fileName;
    std::string               baseDir;
    std::string               blockExtension;
    std::vector<std::string>  blockFileNames;

    std::string               groupPieceName;
    std::vector<std::string>  groupNames;

    std::string               blockPieceName;
    std::vector<std::string>  blockNames;
    std::vector<int>          groupIds;

    vtkXMLDataParser         *parser;
    int                       nGroups;

    std::string               blockName;
    void                      ParseForDS(vtkXMLDataElement *el, int &nb);

    std::string               errorMessage;

    VTMParser(const VTMParser &);      // Not implemented
    void operator=(const VTMParser &); // Not implemented
};

#endif
