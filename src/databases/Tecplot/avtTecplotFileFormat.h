/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtTecplotFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Tecplot_FILE_FORMAT_H
#define AVT_Tecplot_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>
#include <visitstream.h>
#include <ExpressionList.h>

class vtkFloatArray;
class vtkPoints;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtTecplotFileFormat
//
//  Purpose:
//      Reads in Tecplot files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Mar 17 09:37:26 PST 2005
//    Fixed memory leak.
//
//    Brad Whitlock, Tue Jul 26 14:57:47 PST 2005
//    Added expressions.
//
//    Jeremy Meredith, Wed Oct 15 12:07:59 EDT 2008
//    Added support for cell-centered vars (through VARLOCATION).
//    Renamed ParseNodes* to ParseArrays* to reflect this capability.
//
// ****************************************************************************

class avtTecplotFileFormat : public avtSTMDFileFormat
{
  public:
                       avtTecplotFileFormat(const char *);
    virtual           ~avtTecplotFileFormat();

    virtual const char    *GetType(void)   { return "Tecplot"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    std::string GetNextToken();
    void        ReadFile();
    void        ParseFEBLOCK(int numNodes, int numElements,const std::string&);
    void        ParseFEPOINT(int numNodes, int numElements,const std::string&);
    void        ParseBLOCK(int numI, int numJ, int numK);
    void        ParsePOINT(int numI, int numJ, int numK);
    void        PushBackToken(const std::string&);
    vtkPoints  *ParseArraysBlock(int numNodes, int numElements);
    vtkPoints  *ParseArraysPoint(int numNodes, int numElements);
    vtkUnstructuredGrid *ParseElements(int numElements, const std::string&);

  protected:
    ifstream file;
    std::string saved_token;
    bool file_read;
    char next_char;
    bool next_char_valid;
    bool next_char_eof;
    bool next_char_eol;
    bool token_was_string;
    std::string filename;
    ExpressionList expressions;

    int Xindex;
    int Yindex;
    int Zindex;
    int spatialDimension;
    int topologicalDimension;

    std::string title;
    int                       numTotalVars;
    std::vector<std::string>  variableNames;
    std::vector<std::string>  allVariableNames;
    std::vector<int>          variableCellCentered;
    std::vector<std::string>  curveNames;
    std::map<std::string,int> curveIndices;
    std::vector<int>          curveDomains;
    std::vector<int>          curveFirstVar;
    std::vector<int>          curveSecondVar;
    std::vector<std::string>  zoneTitles;

    std::vector<vtkDataSet*> meshes;
    std::map<std::string, std::vector<vtkFloatArray*> > vars;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    avtMeshType            DetermineAVTMeshType() const;
};


#endif
