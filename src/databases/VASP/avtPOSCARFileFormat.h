// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPOSCARFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_POSCAR_FILE_FORMAT_H
#define AVT_POSCAR_FILE_FORMAT_H

#include <avtFileFormatInterface.h>
#include <avtSTSDFileFormat.h>

// ****************************************************************************
//  Class: avtPOSCARFileFormat
//
//  Purpose:
//      Reads in POSCAR files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

class avtPOSCARFileFormat : public avtSTSDFileFormat
{
  public:
    static bool        Identify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);

                       avtPOSCARFileFormat(const char *filename);
    virtual           ~avtPOSCARFileFormat() {;};

    virtual const char    *GetType(void)   { return "POSCAR"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::vector<istream::pos_type>   file_positions;

    double unitCell[3][3];

    ifstream in;
    std::string filename;
    bool file_read;

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
    void ReadFile();
};


#endif
