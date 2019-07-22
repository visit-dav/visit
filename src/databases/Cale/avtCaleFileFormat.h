// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCaleFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CALE_FILE_FORMAT_H
#define AVT_CALE_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <visit-config.h>

#ifdef HAVE_PDB_PROPER
#include <pdb.h>
#define PDBLIB_ERRORSTRING PD_err
#else
#include <lite_pdb.h>
#ifdef _WIN32
#define PDBLIB_ERRORSTRING "(don't have the PD_err error string)"
#else
#define PDBLIB_ERRORSTRING PD_err
#endif
#endif

// ****************************************************************************
//  Class: avtCaleFileFormat
//
//  Purpose:
//      Reads in cale_visit files as a plugin to VisIt.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
// ****************************************************************************

class avtCaleFileFormat : public avtSTSDFileFormat
{
  public:
                       avtCaleFileFormat(const char *filename);
    virtual           ~avtCaleFileFormat();

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual int       GetCycle(void);
    virtual int       GetCycleFromFilename(const char *f) const;
    virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "Cale"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);
    virtual void          *GetAuxiliaryData(const char *var, 
                                            const char *type,
                                            void *, 
                                            DestructorFunction &df) ;

    static void Identify(const char *filename);

  protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                   GetUsedMeshLimits (void);
    PDBfile                *GetPDBFile();
    int  removezoneghost_index(int in,int zk,int zl, int lp);
    PDBfile *pdbfile ;
    int kminmesh, kmaxmesh, lminmesh, lmaxmesh, nnallsmesh ;
};


#endif
