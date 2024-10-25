// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtWavefrontOBJFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_WAVEFRONT_OBJ_FILE_FORMAT_H
#define AVT_WAVEFRONT_OBJ_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include <set>
#include <string>

// ****************************************************************************
//  Class: avtWavefrontOBJFileFormat
//
//  Purpose:
//      Handles files of the .obj file format.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Mark C. Miller, Fri Oct 19 15:45:44 PDT 2018
//    Add variables to support OBJ groupings. Add GetAux method 
//    to treat single-inclusion groupings also as materials.
// ****************************************************************************

class avtWavefrontOBJFileFormat : public avtSTSDFileFormat
{
  public:
                          avtWavefrontOBJFileFormat(const char *);
    virtual              ~avtWavefrontOBJFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual const char   *GetType(void)  { return "WavefrontOBJ File Format"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

    virtual void          *GetAuxiliaryData(const char *var,
                              const char *type, void *, DestructorFunction &df);

  protected:
    vtkDataSet           *dataset;
    bool                  readInDataset;
    int                   nVars;

    static const char    *MESHNAME;
    static const char    *VARNAME1;
    static const char    *VARNAME2;

    void                  ReadInDataset(void);

    bool                  hasGroups;
    std::set<std::string> uniqueGroupNames;
    std::set<std::string> aggregatedGroupNames;
};


#endif


