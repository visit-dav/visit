// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCTRLFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_CTRL_FILE_FORMAT_H
#define AVT_CTRL_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <string>
#include <vector>

namespace avtCTRLNamespace
{
struct Atom
{
    char element[3];
    int atomicnumber;
    float x;
    float y;
    float z; 
};
}

// ****************************************************************************
//  Class: avtCTRLFileFormat
//
//  Purpose:
//      Reads in CTRL files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

class avtCTRLFileFormat : public avtSTSDFileFormat
{
  public:
                       avtCTRLFileFormat(const char *filename);
    virtual           ~avtCTRLFileFormat() {;};

    virtual const char    *GetType(void)   { return "CTRL"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);


    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadAtoms();

    ifstream in;
    std::string filename;
    bool metadata_read;

    std::vector<avtCTRLNamespace::Atom> atoms;
    double alat;
    double unitCell[3][3];
};


#endif
