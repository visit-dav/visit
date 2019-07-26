// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPoint3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_POINT3D_FILE_FORMAT_H
#define AVT_POINT3D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


#include <vector>
#include <string>


class     vtkFloatArray;
class     vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtPoint3DFileFormat
//
//  Purpose:
//      A file format reader for curves.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//   Brad Whitlock, Mon Jun 5 10:43:55 PDT 2006
//   Added config file support.
//
//   Hank Childs, Sat Mar 17 16:29:45 PDT 2007
//   Added FreeUpResources.
//
//   Mark C. Miller, Thu Jul 25 21:12:18 PDT 2019
//   Added spatialDim
// ****************************************************************************

class avtPoint3DFileFormat : public avtSTSDFileFormat
{
  public:
                               avtPoint3DFileFormat(const char *);
    virtual                   ~avtPoint3DFileFormat();
    
    virtual const char        *GetType(void) { return "3D points"; };
    virtual void               FreeUpResources(void);
    
    virtual vtkDataSet        *GetMesh(const char *);
    virtual vtkDataArray      *GetVar(const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { return 0; };

  protected:
    bool                       haveReadData;
    int                        spatialDim;
    vtkFloatArray             *column1;
    vtkFloatArray             *column2;
    vtkFloatArray             *column3;
    vtkFloatArray             *column4;
    vtkUnstructuredGrid       *points;
    std::vector<std::string>   varnames;

    static const char         *MESHNAME;

    void                       ReadData(void);
    bool                       ReadConfigFile(int &);
};


#endif


