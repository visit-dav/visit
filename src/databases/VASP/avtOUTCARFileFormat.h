// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_OUTCAR_FILE_FORMAT_H
#define AVT_OUTCAR_FILE_FORMAT_H

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>
#include <string>
#include <vector>

namespace avtOUTCARNamespace
{
struct Atom
{
    int elementtype_index;
    float x;
    float y;
    float z; 
    float fx;
    float fy;
    float fz;
    float vx;
    float vy;
    float vz;
};
}

// ****************************************************************************
//  Class: avtOUTCARFileFormat
//
//  Purpose:
//      Reads in OUTCAR files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 23 15:22:37 EST 2007
//    Added support for seeking directly to preset timesteps.
//
//    Jeremy Meredith, Fri Apr 20 15:01:36 EDT 2007
//    Added support for magnetization fields.
//
//    Jeremy Meredith, Tue Mar 10 17:42:20 EDT 2009
//    Added support for POTIM field to get time values.
//
//    Jeremy Meredith, Mon May 10 18:01:50 EDT 2010
//    Changed the way cycles and times are generated.
//
//    Jeremy Meredith, Thu Aug 12 16:26:24 EDT 2010
//    Allowed per-cycle changes in unit cell vectors.
//
//    Jeremy Meredith, Tue Oct 19 12:59:24 EDT 2010
//    Added support for optional velocities.
//
// ****************************************************************************

class avtOUTCARFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        Identify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);

                       avtOUTCARFileFormat(const char *filename);
    virtual           ~avtOUTCARFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "OUTCAR"; };
    virtual void           FreeUpResources(void); 
    virtual void           GetCycles(std::vector<int>&);
    virtual void           GetTimes(std::vector<double>&);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual bool          HasInvariantMetaData(void) const { return false; };

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *,int);


    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadAtomsForTimestep(int);

    ifstream in;
    std::string filename;
    bool metadata_read;

    int ntimesteps;
    int natoms;

    struct UCV
    {
        double v[3][3];
        double *operator[](int i) { return v[i]; }
    };

    std::vector<istream::pos_type>   file_positions;

    bool has_velocities;
    bool has_magnetization;

    std::vector<float>               mags,magp,magd,magtot;

    std::vector<float>               free_energy;
    std::vector< std::vector<avtOUTCARNamespace::Atom> > allatoms;

    std::vector<UCV> unitCell;
    double potim;

    std::vector<std::string> element_names;
    std::vector<int>         element_types;
    std::vector<int>         element_counts;
};


#endif
