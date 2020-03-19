// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtLinesFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_LINES_FILE_FORMAT_H
#define AVT_LINES_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>

#include "visit_gzstream.h"


class     vtkPolyData;


// ****************************************************************************
//  Class: avtLinesFileFormat
//
//  Purpose:
//      A file format reader for lines.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Alister Maguire, Tue Mar 17 08:50:32 PDT 2020
//    Added GetDimensionality.
//
// ****************************************************************************

class avtLinesFileFormat : public avtSTMDFileFormat
{
  public:
                          avtLinesFileFormat(const char *);
    virtual              ~avtLinesFileFormat();
    
    virtual const char   *GetType(void) { return "Lines File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string           filename;
    bool                  readInFile;

    std::vector<vtkPolyData *> lines;
    std::vector<std::string>   lineNames;

    void                  ReadFile(void);
    bool                  GetPoint(istream &, float &, float &, float &,
                                   std::string &);
    int                   GetDimensionality(visit_ifstream &);
};


#endif


