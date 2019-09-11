// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtXDATFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_XDAT_FILE_FORMAT_H
#define AVT_XDAT_FILE_FORMAT_H

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>

// ****************************************************************************
//  Class: avtXDATFileFormat
//
//  Purpose:
//      Reads in XDAT files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   March  4, 2016
//
//  Modifications:
//
// ****************************************************************************

class avtXDATFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        Identify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);

                       avtXDATFileFormat(const char *filename);
    virtual           ~avtXDATFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "XDAT"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    std::vector<istream::pos_type>   file_positions;

    double unitCell[3][3];

    ifstream in;
    std::string filename;
    bool metadata_read;
    bool full_header;

    bool selective_dynamics;
    bool cartesian;
    double scale;
    double lat[3][3];
    
    int natoms;

    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<int> species;

    std::vector<int> cx;
    std::vector<int> cy;
    std::vector<int> cz;

    std::vector<int> species_counts;
    std::vector<int> element_map;

    void OpenFileAtBeginning();
    void ReadHeader( bool read_full_header = 0 );
    void ReadMetaData();
    void ReadTimestep(int);
};


#endif
