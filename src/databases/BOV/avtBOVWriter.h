// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtBOVWriter.h                               //
// ************************************************************************* //

#ifndef AVT_BOV_WRITER_H
#define AVT_BOV_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtBOVWriter
//
//  Purpose:
//      A module that writes out BOV files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Jeremy Meredith/Hank Childs, Tue Mar 27 17:03:47 EDT 2007
//    Added numblocks to the OpenFile interface.
//
//    Alister Maguire, Fri Sep  1 15:18:58 PDT 2017
//    Moved constructor and destructor definitions to the C file. 
//    Added gzCompress and SetCompressOutput().
//
//    Alister Maguire, Thu Sep  7 09:02:03 PDT 2017
//    Added DBOptionsAttributes as a constnructor argument. 
//
// ****************************************************************************

class
avtBOVWriter : public virtual avtDatabaseWriter
{
  public:
                   avtBOVWriter(DBOptionsAttributes *);
    virtual       ~avtBOVWriter();

  protected:
    std::string    stem;
    int            nblocks;
    bool           gzCompress;

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    virtual bool   SupportsTargetChunks(void) { return true; };
    virtual bool   SupportsTargetZones(void) { return true; };
    virtual bool   SupportsOutputZonal(void) { return true; };

    // Doing material interface reconstruction does nothing for us. 
    // So we may as well report that we can already do it, so the MIR
    // doesn't execute unnecessarily.
    virtual bool   CanHandleMaterials(void) { return true; };

    virtual void   SetCompressOutput(bool compress) { gzCompress = compress; };
    virtual bool   GetCompressOutput(void) { return gzCompress; };
};


#endif


