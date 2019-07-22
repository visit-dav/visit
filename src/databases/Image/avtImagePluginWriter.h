// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtImagePluginWriter.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGEPLUGIN_WRITER_H
#define AVT_IMAGEPLUGIN_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtImagePluginWriter
//
//  Purpose:
//      A module that exports databases as image formats.
//
//  Programmer: Mark C. Miller
//  Creation:   November 28, 2007 
//
// ****************************************************************************

class
avtImagePluginWriter : public virtual avtDatabaseWriter
{
  public:
                   avtImagePluginWriter(DBOptionsAttributes *);
    virtual       ~avtImagePluginWriter() {;};

  protected:
    std::string    fileName;
    int            nblocks;
    int            format;
    bool           normalize;
    int            compression;
    int            quality;

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
};

#endif
