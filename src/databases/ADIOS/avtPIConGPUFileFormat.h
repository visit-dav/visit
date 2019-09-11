// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ADIOS_PICONGPU_FILE_FORMAT_H
#define AVT_ADIOS_PICONGPU_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtPIConGPUFileFormat
//
//  Purpose:
//      Reads in ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
//  Modifications:
//
// ****************************************************************************

class avtPIConGPUFileFormat : public avtSTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    avtPIConGPUFileFormat(const char *);
    virtual  ~avtPIConGPUFileFormat();
    virtual int        GetCycle();
    virtual double     GetTime();

    virtual const char    *GetType(void)   { return "ADIOS_PIConGPU"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);
    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    void ActivateTimestep() {}

  protected:
    ADIOSFileObject *fileObj;
    bool             initialized;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *md);
    void                   Initialize();

    vtkDataSet * GetParticleMesh(std::string &nm, int domain);
    vtkDataSet * GetFieldMesh(int domain);
    vtkDataArray * GetParticleVar(bool isIon, std::string &vname, int domain);
    vtkDataArray * GetIonSelect(int domain);
    int          CalcCycle();

    int cycle;
    double time;
};

#endif
