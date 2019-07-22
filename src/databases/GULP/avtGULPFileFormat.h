// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtGULPFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_GULP_FILE_FORMAT_H
#define AVT_GULP_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>


// ****************************************************************************
//  Class: avtGULPFileFormat
//
//  Purpose:
//      Reads in GULP files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for binary files, and for forces and potential energies.
// ****************************************************************************

class avtGULPFileFormat : public avtMTSDFileFormat
{
  public:
                       avtGULPFileFormat(const char *filename);
    virtual           ~avtGULPFileFormat() {;};

    virtual int       GetNTimesteps(void);
    virtual void      GetTimes(std::vector<double> &t);

    virtual const char    *GetType(void)   { return "GULP"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::vector<istream::pos_type>   file_positions;

    ifstream            in;
    std::string         filename;
    bool                md_read;
    bool                binary;

    bool                has_force_and_pe;
    int                 ntimesteps;
    int                 current_timestep;

    int                 natoms;
    int                 dimension;
    std::vector<double> times;

    std::vector<float>  x;
    std::vector<float>  y;
    std::vector<float>  z;
    std::vector<float>  vx;
    std::vector<float>  vy;
    std::vector<float>  vz;
    std::vector<float>  fx;
    std::vector<float>  fy;
    std::vector<float>  fz;
    std::vector<float>  pe;


    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadTimestep(int);
};


#endif
