// ************************************************************************* //
//                           avtTetradFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TETRAD_FILE_FORMAT
#define AVT_TETRAD_FILE_FORMAT


#include <avtMTSDFileFormat.h>

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
// ****************************************************************************

class avtTetradFileFormat : public avtMTSDFileFormat
{
  public:
                               avtTetradFileFormat(const char *);
    virtual                   ~avtTetradFileFormat();

    virtual void               GetCycles(std::vector<int> &);
    virtual int                GetNTimesteps(void);

    virtual const char        *GetType(void) { return "Tetrad"; };
    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

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


