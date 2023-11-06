// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCurve3DWriter.h                            //
// ************************************************************************* //

#ifndef AVT_CURVE3D_WRITER_H
#define AVT_CURVE3D_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtCurve3DWriter
//
//  Purpose:
//      A module that writes out Curve3D files.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

class
avtCurve3DWriter : public avtDatabaseWriter
{
  public:
                   avtCurve3DWriter(const DBOptionsAttributes *);
    virtual       ~avtCurve3DWriter() {;}

  protected:
    std::string    stem;
    int            nBlocks;
    std::string    commentStyle;

    virtual void   OpenFile(const std::string &, int nb);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    std::string    SanitizeName(const std::string &name) const;
};


#endif
