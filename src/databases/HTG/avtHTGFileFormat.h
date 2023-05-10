// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtHTGFileFormat.h
// ****************************************************************************

#ifndef AVT_HTG_FILE_FORMAT_H
#define AVT_HTG_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtHTGFileFormat
//
//  Purpose:
//      Reads in HTG files as a plugin to VisIt.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Mon May 1 15:28:30 PST 2023
//
// ****************************************************************************

class avtHTGFileFormat : public avtSTMDFileFormat
{
  public:
                       avtHTGFileFormat(const char *, const DBOptionsAttributes *);
    virtual           ~avtHTGFileFormat() {;}

    virtual const char    *GetType(void)   { return "HTG"; }
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
