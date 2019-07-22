// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtparaDIS_tecplotFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_paraDIS_tecplot_FILE_FORMAT_H
#define AVT_paraDIS_tecplot_FILE_FORMAT_H

#define PARADIS_TECPLOT_VERSION_NUMBER "$Revision: 1.3 $"
#define PARADIS_TECPLOT_VERSION_DATE "$Date: 2008/10/08 00:32:37 $"

#include <avtSTSDFileFormat.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include "Point.h"
#include <vector>

// ****************************************************************************
//  Class: avtparaDIS_tecplotFileFormat
//
//  Purpose:
//      Reads in paraDIS_tecplot files as a plugin to VisIt.
//
//  Programmer: rcook -- generated by xml2avt
//  Creation:   Fri Sep 26 12:04:51 PDT 2008
//
// ****************************************************************************

class avtparaDIS_tecplotFileFormat : public avtSTSDFileFormat
{
  public:
                       avtparaDIS_tecplotFileFormat(const char *filename);
    virtual           ~avtparaDIS_tecplotFileFormat() {;};

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual int       GetCycle(void);
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "paraDIS_tecplot"; };
    virtual void           FreeUpResources(void); 
    
    void AddCellToMesh(vtkUnstructuredGrid *linemesh, vector<int> &cell);
    vtkDataSet    * GetTecplotMesh(const char *meshname); 
    virtual vtkDataSet    *GetMesh(const char *);
    
    bool  GetTecplotVar(const char *varname, vtkFloatArray *scalars); 
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);
    
 protected:
    bool ParseTecplotHeader(ifstream &tecplotfile, bool setnumvars=false);
    bool PopulateTecplotMetaData(avtDatabaseMetaData *md); 
    
    // DATA MEMBERS
    std::string mFilename; 
    int mVerbosity; 
    int mProcNum, mNumProcs;
    
    string mFileType; 
    vector<string> mVariableNames; 
    unsigned int mNumElems, mNumPoints, mNumVars; 
    bool mRotateTecplotPoints; 
    rclib::Point<float> mOldOrientation, mNewOrientation; 
    
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
