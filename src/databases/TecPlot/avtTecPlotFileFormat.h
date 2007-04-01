// ************************************************************************* //
//                            avtTecPlotFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TecPlot_FILE_FORMAT_H
#define AVT_TecPlot_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>
#include <visitstream.h>

class vtkFloatArray;
class vtkPoints;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtTecPlotFileFormat
//
//  Purpose:
//      Reads in TecPlot files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
// ****************************************************************************

class avtTecPlotFileFormat : public avtSTMDFileFormat
{
  public:
                       avtTecPlotFileFormat(const char *);
    virtual           ~avtTecPlotFileFormat() {;};

    virtual const char    *GetType(void)   { return "TecPlot"; };
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
    vtkPoints  *ParseNodesBlock(int numNodes);
    vtkPoints  *ParseNodesPoint(int numNodes);
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

    int Xindex;
    int Yindex;
    int Zindex;
    int spatialDimension;
    int topologicalDimension;

    std::string title;
    int                       numTotalVars;
    std::vector<std::string>  variableNames;
    std::vector<std::string>  allVariableNames;
    std::vector<std::string>  curveNames;
    std::map<std::string,int> curveIndices;
    std::vector<int>          curveDomains;
    std::vector<int>          curveFirstVar;
    std::vector<int>          curveSecondVar;
    std::vector<std::string>  zoneTitles;

    std::vector<vtkDataSet*> meshes;
    std::map<std::string, std::vector<vtkFloatArray*> > vars;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
