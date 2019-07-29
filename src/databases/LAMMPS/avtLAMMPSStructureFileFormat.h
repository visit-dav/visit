// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtLAMMPSStructureFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_LAMMPS_STRUCTURE_FILE_FORMAT_H
#define AVT_LAMMPS_STRUCTURE_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtFileFormatInterface.h>

#include <vector>

#define MAX_LAMMPS_STRUCTURE_VARS 6


// ****************************************************************************
//  Class: avtLAMMPSStructureFileFormat
//
//  Purpose:
//      Reads in LAMMPS molecular files.  Supports up to MAX_LAMMPS_VARS extra
//      data fields.  Supports multiple timesteps in one file.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  6, 2009
//
//  Modifications:
//
// ****************************************************************************

class avtLAMMPSStructureFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        FileExtensionIdentify(const std::string&);
    static bool        FileContentsIdentify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);
  public:
                       avtLAMMPSStructureFileFormat(const char *);
    virtual           ~avtLAMMPSStructureFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "EAM"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    ifstream                       in;
    std::vector<istream::pos_type> file_positions;
    std::string                    filename;
    bool                           metaDataRead;
    int                            nTimeSteps;
    int                            nVars;
    int                            nAtoms;

    std::vector< std::vector<int> >          s; //species index
    std::vector< std::vector<float> >        x;
    std::vector< std::vector<float> >        y;
    std::vector< std::vector<float> >        z;
    std::vector< std::vector<float> >        v[MAX_LAMMPS_STRUCTURE_VARS];

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void            OpenFileAtBeginning();

    void ReadTimeStep(int);
    void ReadAllMetaData();
};


#endif
