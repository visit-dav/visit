// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtRAWWriter.h                               //
// ************************************************************************* //

#ifndef AVT_RAW_WRITER_H
#define AVT_RAW_WRITER_H

#include <avtDatabaseWriter.h>
#include <stdio.h>

// ****************************************************************************
//  Class: avtRAWWriter
//
//  Purpose:
//      A module that writes out RAW files.
//
//  Programmer: Brad Whitlock
//  Creation:   September 28, 2007
//
//  Modifications:
//    Brad Whitlock, Mon Apr  7 15:35:03 PDT 2014
//    Indicate that the format does not deal with materials.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

class
avtRAWWriter : public virtual avtDatabaseWriter
{
  public:
                   avtRAWWriter();
    virtual       ~avtRAWWriter();

  protected:
    FILE *file;
    int   nDomains;

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    virtual bool          CreateTrianglePolyData() const;
    virtual CombineMode   GetCombineMode(const std::string &plotName) const;

    virtual bool   SupportsTargetChunks(void) { return false; };
    virtual bool   SupportsTargetZones(void) { return false; };
    virtual bool   CanHandleMaterials(void) { return false; };
};

#endif
