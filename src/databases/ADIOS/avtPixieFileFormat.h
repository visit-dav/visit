// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Pixie_FILE_FORMAT_H
#define AVT_Pixie_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtPixieFileFormat
//
//  Purpose:
//      Reads in Pixie-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables and add expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

class avtPixieFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(ADIOSFileObject *);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    avtPixieFileFormat(const char *);
    virtual  ~avtPixieFileFormat();

    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    virtual int         GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS-Pixie"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    ADIOSFileObject *file;
    
    bool             initialized;
    void             Initialize();

    virtual void     PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    
    bool             HasCoordinates(const std::string &var, std::string *coords);
                                  
    struct VarInfo
    {
        std::string fileVarName, mesh;
        bool isTimeVarying;
    };
    typedef std::map<std::string, VarInfo> VarInfoMap;

    struct MeshInfo
    {
        uint64_t    dims[3];
        bool        isRectilinear;
        std::string coords[3];
    };
    typedef std::map<std::string, MeshInfo> MeshInfoMap;

    std::vector<int> timecycles;
    VarInfoMap       variables;
    MeshInfoMap      meshes;
    std::string      rawExpression;

    bool             IsVariable(const std::string &vname);
    std::string      GetVarName(const std::string &vname, bool &isTimeVarying);
    std::string      GetVarMesh(const std::string &vname, MeshInfo &mi);
    bool             GetTimeStep(const std::string &vname, int &ts);
};
#endif
