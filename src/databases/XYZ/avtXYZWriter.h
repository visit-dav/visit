// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtXYZWriter.h                              //
// ************************************************************************* //

#ifndef AVT_XYZ_WRITER_H
#define AVT_XYZ_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>
#include <fstream>


// ****************************************************************************
//  Class: avtXYZWriter
//
//  Purpose:
//      A module that writes out XYZ files.  This is essentially an
//      atomic data format.  We write every chunk to a single file,
//      which means this is likely to have undefined results in parallel.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
// ****************************************************************************

class
avtXYZWriter : public avtDatabaseWriter
{
  public:
                   avtXYZWriter();
    virtual       ~avtXYZWriter() {;};

  protected:
    std::string    stem;

    virtual void   OpenFile(const std::string &, int nb);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    virtual CombineMode   GetCombineMode(const std::string &plotName) const;
};


#endif
