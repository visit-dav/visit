// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMiliFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MILI_FILE_FORMAT_H
#define AVT_MILI_FILE_FORMAT_H

#include <unordered_map>
#include <vectortypes.h>

#include <Expression.h>

extern "C" {
#include <mili.h>
}

#include <avtMTMDFileFormat.h>
#include <avtMiliMetaData.h>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

class avtMaterial;
class vtkDataArray;
class vtkUnstructuredGrid;
class vtkFloatArray;
class vtkPoints;
class vtkElementLabelArray;

using std::string;

typedef std::unordered_map<string, int> StrToIntMap;


// ****************************************************************************
//  Class: avtMiliFileFormat
//
//  Purpose:
//      A file format reader for Mili.
//
//  Notes:  This filter was largely re-written in Jan 2019 in order to better 
//          handle scalability, extensibility, and the in-coming requests from
//          mili users. Methods that retain significant portions of the original
//          code contain notes about such in their docstrings. The original 
//          filter was first created by Hank Childs on April 11, 2003. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

class avtMiliFileFormat : public avtMTMDFileFormat
{
  public:
                           avtMiliFileFormat(const char *);

    virtual               ~avtMiliFileFormat();
    
    virtual const char    *GetType(void) { return "Mili File Format"; };
    
    virtual void           GetCycles(intVector &);

    virtual void           GetTimes(doubleVector &);

    virtual int            GetNTimesteps(void);

    virtual vtkDataSet    *GetMesh(int, 
                                   int, 
                                   const char *); 

    virtual vtkDataArray  *GetVar(int, 
                                  int, 
                                  const char *);

    virtual vtkDataArray  *GetVectorVar(int, 
                                       int, 
                                       const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    virtual void         *GetAuxiliaryData(const char *, 
                                           int, 
                                           int,
                                           const char *, 
                                           void *,
                                           DestructorFunction &); 

    virtual void          FreeUpResources(void) {};

    virtual bool          CanCacheVariable(const char *);

  protected:

    inline void           OpenDB(int);

    void                  ReadMesh(int);

    int                   ExtractMeshIdFromPath(const string &);

    vtkPoints            *GetNodePositions(int, int, int);

    void                  GetVar(int, 
                                 int, 
                                 int,
                                 MiliVariableMetaData *varMD,
                                 vtkFloatArray *);

    void                  GetVectorVar(int, 
                                       int, 
                                       int,
                                       MiliVariableMetaData *varMD,
                                       vtkFloatArray *);

    void                  GetElementSetVar(int, 
                                           int,
                                           int,
                                           string, 
                                           MiliVariableMetaData *varMD,
                                           vtkFloatArray *);

    void                  ReadMiliVarToBuffer(char *,
                                              const intVector &,
                                              SubrecInfo *,
                                              int,
                                              int,
                                              int,
                                              int,
                                              int,
                                              float *);

    void                  PopulateSubrecordInfo(int, 
                                                int);

    void                  AddMiliVariableToMetaData(avtDatabaseMetaData *,
                                                    int,
                                                    int,
                                                    bool,
                                                    string,
                                                    avtCentering,
                                                    const intVector &,
                                                    const stringVector &);

    //
    // Json extraction methods. 
    //
    void                  LoadMiliInfoJson(const char *);

    int                   CountJsonClassVariables(const rapidjson::Document &,
                                                  StrToIntMap &);

    MiliVariableMetaData *ExtractJsonVariable(const rapidjson::Document &,
                                              const rapidjson::Value &,
                                              string,
                                              string,
                                              string,
                                              int,
                                              bool,
                                              bool,
                                              StrToIntMap &);

    void                  ExtractJsonVectorComponents(const rapidjson::Value &,
                                                      const rapidjson::Value &,
                                                      string,
                                                      int);

    void                  ExtractJsonClasses(rapidjson::Document &,
                                             int,
                                             StrToIntMap &);

    //
    // Label info retrieval.
    //
    void                   RetrieveZoneLabelInfo(const int, 
                                                 char *, 
                                                 const int, 
                                                 const int);
    void                   RetrieveNodeLabelInfo(const int, 
                                                 char *, 
                                                 const int);
    vtkElementLabelArray  *GenerateLabelArray(int,
                                              int,
                                              const stringVector *,
                                             std::vector<MiliClassMetaData *>);

    //
    // Expression helpers. 
    //
    Expression             CreateGenericExpression(const char *, 
                                                   const char *,
                                                   Expression::ExprType);

    Expression             ScalarExpressionFromVec(const char *,
                                                   const char *, 
                                                   int);

    //
    // Protected data. 
    //
    vtkUnstructuredGrid ***datasets;
    avtMaterial         ***materials;

    avtMiliMetaData      **miliMetaData;

    char                  *famroot;
    char                  *fampath;

    int                    nTimesteps;
    int                    nDomains;
    int                    nMeshes;
    int                    dims;

    intVector              cycles;
    doubleVector           times;
    boolVector             meshRead;
    std::vector<Famid>     dbid;
};

#endif
