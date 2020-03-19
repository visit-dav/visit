// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtMatrixMarketFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MatrixMarket_FILE_FORMAT_H
#define AVT_MatrixMarket_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


// ****************************************************************************
//  Class: avtMatrixMarketFileFormat
//
//  Purpose:
//      Reads in MatrixMarket files as a plugin to VisIt.
//
//  Note: http://math.nist.gov/MatrixMarket/formats.html
//      As of 2009-03-13, this reader supports "array" (dense) and
//      "coordinate" (sparse), real-valued, general matrices.  The
//      format itself has extended support for patterned, complex,
//      and integer matrices which are not yet supported.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

class avtMatrixMarketFileFormat : public avtSTSDFileFormat
{
  public:
                       avtMatrixMarketFileFormat(const char *filename);
    virtual           ~avtMatrixMarketFileFormat() {;};

    virtual const char    *GetType(void)   { return "MatrixMarket"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    std::string   filename;
    bool          dataread;
    vtkDataArray *matrix;
    int           width, height;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                   ReadData();
};


#endif
