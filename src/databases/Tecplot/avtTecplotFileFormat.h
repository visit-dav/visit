// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTecplotFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Tecplot_FILE_FORMAT_H
#define AVT_Tecplot_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>
#include <ExpressionList.h>
#include <DBOptionsAttributes.h>

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
//    Mark C. Miller, Tue Jan 12 17:35:54 PST 2010
//    Added solTime data member and GetTime() method.
//
//    Jeremy Meredith, Fri Oct 21 10:18:56 EDT 2011
//    Support VARSHARELIST.
//
//    Jeremy Meredith, Tue Oct 25 12:37:42 EDT 2011
//    Allow user manual override of coordinate axis variables (via options).
//
// ****************************************************************************

class avtTecplotFileFormat : public avtSTMDFileFormat
{
  public:
                       avtTecplotFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtTecplotFileFormat();

    virtual const char    *GetType(void)   { return "Tecplot"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    double                 GetTime() { return solTime;} ;

  protected:
    std::string GetNextToken();
    void        ReadFile();
    void        ParseFEBLOCK(int numNodes, int numElements,const std::string&, int connectivitycopy);
    void        ParseFEPOINT(int numNodes, int numElements,const std::string&, int connectivitycopy);
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
    int  currentZoneIndex;

    bool userSpecifiedAxisVars;
    int userSpecifiedX;
    int userSpecifiedY;
    int userSpecifiedZ;

    int Xindex;
    int Yindex;
    int Zindex;
    int spatialDimension;
    int topologicalDimension;
    double solTime;

    std::string title;
    int                       numTotalVars;
    std::vector<std::string>  variableNames;
    std::vector<int>          variableCellCentered;
    std::vector<int>          variableShareMap;
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
