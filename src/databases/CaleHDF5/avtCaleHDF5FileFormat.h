// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCaleHDF5FileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_CaleHDF5_FILE_FORMAT_H
#define AVT_CaleHDF5_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include "hdf5.h"

// ****************************************************************************
//  Class: avtCaleHDF5FileFormat
//
//  Purpose:
//      Reads in CaleHDF5 files as a plugin to VisIt.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

class avtCaleHDF5FileFormat : public avtSTSDFileFormat
{
  public:
                       avtCaleHDF5FileFormat(const char *filename);
    virtual           ~avtCaleHDF5FileFormat() {FreeUpResources();};

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual int       GetCycle(void);
    virtual int       GetCycleFromFilename(const char *f) const;
    virtual double    GetTime(void);

    virtual const char    *GetType(void)   { return "CaleHDF5"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    static void Identify(const char *filename);
    static hid_t ReadHDF_Entry(hid_t group, const char* name, void* ptr);

  protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                   GetUsedMeshLimits (void);
    hid_t                  GetHDF5File();
    int  removezoneghost_index(int in,int zk,int zl, int lp);
    hid_t   hdffile ;

    int kminmesh, kmaxmesh, lminmesh, lmaxmesh, nnallsmesh ;
};


#endif
