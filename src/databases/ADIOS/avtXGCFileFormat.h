// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_XGC_FILE_FORMAT_H
#define AVT_XGC_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtXGCFileFormat
//
//  Purpose:
//      Reads in XGC-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

class avtXGCFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    static std::string CreateMeshName(const std::string &filename);
    static std::string CreateDiagName(const std::string &filename);
    static std::string CreateSeparatrixName(const std::string &filename);
    
    avtXGCFileFormat(const char *);
    virtual  ~avtXGCFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int timestep, 
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    ADIOSFileObject *file, *meshFile, *diagFile;
    std::string sepFileName;
    bool initialized, haveSepMesh, haveParticles;
    int numNodes, numTris, numPhi;


    void                   Initialize();
    vtkDataSet            *GetParticleMesh(int, int);
    vtkDataArray          *GetTurbulence(int ts, int dom);
    vtkDataArray          *GetSep();
    vtkDataArray          *GetPsi();
    vtkDataSet            *GetSepMesh();
    vtkDataSet            *GetMesh2D(int ts, int dom);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};

#endif
