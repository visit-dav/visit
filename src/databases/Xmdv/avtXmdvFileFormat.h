// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


// ************************************************************************* //
//                             avtXmdvFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Xmdv_FILE_FORMAT_H
#define AVT_Xmdv_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


class DBOptionsAttributes;


// ****************************************************************************
//  Class: avtXmdvFileFormat
//
//  Purpose:
//      Reads in Xmdv files as a plugin to VisIt.  Xmdv is a tools that
//      uses parallel coordinates to allow users to infer relations between
//      quantities.  The tool has a homegrown format that uses the extension
//      ".okc" to store parallel coordinates data.  This reader is useful
//      in VisIt for two reasons.  (1) It allows us to test "array" data that
//      comes directly from a file (as opposed to an expression).  (2) Since
//      the format is so simple, OKC files are a good way to export data from
//      VisIt into other tools.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Tue Jul 19 17:12:44 PST 2005
//
// ****************************************************************************

class avtXmdvFileFormat : public avtSTSDFileFormat
{
  public:
                       avtXmdvFileFormat(const char *filename, 
                                         const DBOptionsAttributes *);
    virtual           ~avtXmdvFileFormat() {;};

    virtual const char    *GetType(void)   { return "Xmdv"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    bool                      readInData;
    int                       ncells;
    std::vector<float>        values;
    std::vector<std::string>  varnames;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    bool                   ReadXmdvFile(bool);
};


#endif
