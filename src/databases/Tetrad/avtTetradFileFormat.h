// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTetradFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TETRAD_FILE_FORMAT
#define AVT_TETRAD_FILE_FORMAT


#include <avtMTSDFileFormat.h>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>


// ****************************************************************************
//  Class: avtTetradFileFormat
//
//  Purpose:
//      A file format reader that reads in Tetrad files for INEEL.  Tetrad
//      files are written using HDF5.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
//  Modifications:
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added timeState arg to PopulateDatabaseMetaData
// ****************************************************************************

class avtTetradFileFormat : public avtMTSDFileFormat
{
  public:
                               avtTetradFileFormat(const char *);
    virtual                   ~avtTetradFileFormat();

    virtual void               GetCycles(std::vector<int> &);
    virtual int                GetNTimesteps(void);

    virtual const char        *GetType(void) { return "Tetrad"; };
    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    
    virtual void               GetVars(const char *);
    virtual void               GetTimesteps(const char *);

  protected:
    hid_t                      file_handle;
    vtkDataSet                *mesh_for_all_timesteps;

    std::vector<std::string>   variables;
    std::vector<std::string>   timesteps;

    vtkDataSet                *ConstructMesh(void);
};


#endif


