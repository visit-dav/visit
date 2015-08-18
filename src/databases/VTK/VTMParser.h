/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

    void                      SetFileName(const char *);
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
