// ************************************************************************* //
//                            avtOVERFLOWFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_OVERFLOW_FILE_FORMAT_H
#define AVT_OVERFLOW_FILE_FORMAT_H

#include <database_exports.h>

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>
#include <fstream.h>

// ****************************************************************************
//  Class: avtOVERFLOWFileFormat
//
//  Purpose:
//      Reads in OVERFLOW files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 21, 2004
//
// ****************************************************************************

class avtOVERFLOWFileFormat : public avtSTMDFileFormat
{
  public:
                           avtOVERFLOWFileFormat(const char *);
    virtual               ~avtOVERFLOWFileFormat() {;};

    virtual const char    *GetType(void)   { return "OVERFLOW"; };
    virtual void           FreeUpResources(void); 
    virtual bool           HasInvariantMetaData(void) const { return false; };
    virtual bool           HasInvariantSIL(void) const      { return false; };

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    void        InitializeFile();
    void        ReadGridHeader();
    void        ReadSolHeader();
    void        ReadCoords(int domain, float *&x,float *&y,float *&z,int *&ib);
    void        ReadVariable(int domain, int var, float *&vals);

    int         read_int(ifstream &in);
    const char *read_fortran_record(ifstream &in);
    int         parse_int(char *&buff);
    float       parse_float(char *&buff);

  protected:
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *);

    ifstream                    gridin;
    ifstream                    solin;
    streampos                   start_of_coords;
    streampos                   start_of_data;
    bool                        swap_endian;

    std::string                 origfilename;
    int                         ndomains;
    int                        *nx;
    int                        *ny;
    int                        *nz;
    int                         nq;
    int                         nqc;
    int                         nspec;
    std::map<std::string,float> varmap;
};


#endif
