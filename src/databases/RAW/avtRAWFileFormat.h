// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtRAWFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_RAW_FILE_FORMAT_H
#define AVT_RAW_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>

class vtkPolyData;

// ****************************************************************************
//  Class: avtRAWFileFormat
//
//  Purpose:
//      Reads in RAW files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 28 13:43:19 PST 2007
//
// ****************************************************************************

class avtRAWFileFormat : public avtSTMDFileFormat
{
  public:
                       avtRAWFileFormat(const char *);
    virtual           ~avtRAWFileFormat();

    virtual const char    *GetType(void)   { return "RAW"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void ActivateTimestep();

  protected:
    struct domain_data
    {
        std::string domainName;
        vtkPolyData *mesh;
    };

    bool initialized;
    std::vector<domain_data> meshes;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    vtkPolyData           *NewPD(int nCells);
    void                   ReadFile(const char *name);
};


#endif
