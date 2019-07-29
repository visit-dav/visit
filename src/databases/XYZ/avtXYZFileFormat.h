// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtXYZFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_XYZ_FILE_FORMAT_H
#define AVT_XYZ_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>

#define MAX_XYZ_VARS 6

// ****************************************************************************
//  Class: avtXYZFileFormat
//
//  Purpose:
//      Reads in XYZ molecular files.  Supports up to MAX_XYZ_VARS extra
//      data fields.  Supports multiple timesteps in one file.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.  Keep track of file positions
//    for all time steps to accessing even late time steps is instantanous.
//
//    Jeremy Meredith, Wed May 20 11:17:19 EDT 2009
//    Added support for CrystalMakers more "human-friendly" (but
//    less computer friendly) style of XYZ file.
//    nAtoms is now stored per-timestep, not globally.
//
// ****************************************************************************

class avtXYZFileFormat : public avtMTSDFileFormat
{
  public:
                       avtXYZFileFormat(const char *);
    virtual           ~avtXYZFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "XYZ"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    enum XYZStyle { Normal, CrystalMaker };

    ifstream                       in;
    std::vector<istream::pos_type> file_positions;
    std::string                    filename;
    bool                           metaDataRead;
    int                            nTimeSteps;
    int                            nVars;
    std::vector<int>               nAtoms;
    XYZStyle                       fileStyle;
    bool                           corruptElement;

    std::vector< std::vector<int> >   e;
    std::vector< std::vector<float> > x;
    std::vector< std::vector<float> > y;
    std::vector< std::vector<float> > z;
    std::vector< std::vector<float> > v[MAX_XYZ_VARS];

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void            OpenFileAtBeginning();

    void ReadTimeStep(int);
    void ReadAllMetaData();
};


#endif
