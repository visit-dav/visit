// ************************************************************************* //
//                           avtSpheralFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_SPHERAL_FILE_FORMAT_H
#define AVT_SPHERAL_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <fstream.h>
#include <avtTypes.h>


class vtkDataArray;
class vtkPolyData;


// ****************************************************************************
//  Class: avtSpheralFileFormat
//
//  Purpose:
//      A file format reader for the Spheral++ code.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Apr 15 11:59:39 PDT 2003
//    Made it work on Windows.
//
// ****************************************************************************

struct AllOfOneDomain
{
    std::vector< vtkPolyData * >                  meshes;
    std::vector<std::vector< vtkDataArray * > >   fields;
};


class avtSpheralFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSpheralFileFormat(const char *);
    virtual              ~avtSpheralFileFormat();
    
    virtual const char   *GetType(void) { return "Spheral++ File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          FreeUpResources(void);
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual bool          PerformsMaterialSelection(void) { return true; };
    virtual bool          HasVarsDefinedOnSubMeshes(void) { return true; };

    virtual void          RegisterVariableList(const char *,
                                          const std::vector<CharStrRef> &);

    virtual int           GetCycle(void) { return cycle; };
    virtual double        GetTime(void)  { return dtime; };

  protected:
    std::string           rootfile;
    int                   ndomains;
    int                   cycle;
    bool                  gotCycle;
    double                dtime;
    bool                  gotTime;
    bool                  readInMetaData;

    std::vector<std::string>                nodeLists;
    std::vector<int>                        nodeListSizes;
    std::vector<std::string>                positionField;
    std::vector<int>                        positionDimension;
    std::vector<std::string>                fields;
    std::vector< std::vector<bool> >        fieldDefinedOnNodeList;
    std::vector<avtVarType>                 fieldType;
    std::vector<int>                        fieldDim1;
    std::vector<int>                        fieldDim2;
    std::vector<std::string>                domain_files;
    std::vector<bool>                       read_domain;
    std::vector<bool>                       validNodeLists;

    std::vector<AllOfOneDomain>             cache;

    std::string                             current_file;

    void                  DetermineSubFiles(istream &, int);
    int                   GetLine(istream &, char *, std::vector<int> &);
    void                  ParseHeader(istream &);
    void                  ParseField(char *, int, std::vector<int> &,bool,int);
    void                  ParseNodeList(char *, int, std::vector<int> &);

    void                  ReadDomain(int);
    vtkPolyData          *ReadNodeList(istream &, int);
    vtkDataArray         *ReadField(istream &, int, int &);
    int                   GetNodeListIndexFromName(const char *);
    int                   GetFieldIndexFromName(const char *);
    void                  ReadInMetaData(void);
};


#endif


