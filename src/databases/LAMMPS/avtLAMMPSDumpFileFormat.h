// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtLAMMPSDumpFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_LAMMPS_DUMP_FILE_FORMAT_H
#define AVT_LAMMPS_DUMP_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtFileFormatInterface.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtLAMMPSDumpFileFormat
//
//  Purpose:
//      Reads in LAMMPS molecular files.  Supports up to MAX_LAMMPS_VARS extra
//      data fields.  Supports multiple timesteps in one file.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  6, 2009
//
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes bounds/unit cell, and an optional atom format string.
//
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
// ****************************************************************************

class avtLAMMPSDumpFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        FileExtensionIdentify(const std::string&);
    static bool        FileContentsIdentify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);
  public:
                       avtLAMMPSDumpFileFormat(const char *);
    virtual           ~avtLAMMPSDumpFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "LAMMPS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    ifstream                       in;
    std::vector<int>               cycles;
    std::vector<istream::pos_type> file_positions;
    std::string                    filename;
    bool                           metaDataRead;
    int                            nTimeSteps;
    int                            nVars;
    std::vector<int>               nAtoms;
    double                         xMin, xMax;
    double                         yMin, yMax;
    double                         zMin, zMax;

    int                      currentTimestep;
    bool                     xScaled,yScaled,zScaled;
    int                      xIndex, yIndex, zIndex;
    int                      speciesIndex, idIndex;
    std::vector< std::vector<float> >  vars;
    std::vector<int>                   speciesVar, idVar;
    std::vector< std::string >         varNames;

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void            OpenFileAtBeginning();

    void ReadTimeStep(int);
    void ReadAllMetaData();
};


#endif
