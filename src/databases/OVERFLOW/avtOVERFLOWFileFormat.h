// ************************************************************************* //
//                            avtOVERFLOWFileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_OVERFLOW_FILE_FORMAT_H
#define AVT_OVERFLOW_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>
#include <visitstream.h>

// ****************************************************************************
//  Class: avtOVERFLOWFileFormat
//
//  Purpose:
//      Reads in OVERFLOW files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 21, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Aug 11 17:46:17 PST 2004
//    I removed the std:: scopes on the ifstreams and streampos members
//    because the std:: namespace is already used for those classes in
//    visitstream.h for most cases. On Windows with the MSVC 6.0 compiler,
//    we use ifstream. etc that are not std:: members so it was not compiling.
//    I also removed a const from read_fortran_record because the output
//    was being freed with the delete [] operator.
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
    char       *read_fortran_record(ifstream &in);
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
