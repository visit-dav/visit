// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtNullDataReader.h                             //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_READER_H
#define AVT_NULL_DATA_READER_H
#include <pipeline_exports.h>


#include <avtOriginatingNullDataSource.h>

#include <ref_ptr.h>


// ****************************************************************************
//  Class: avtNullDataReader
//
//  Purpose:
//      A class which takes as input an avtNullData and can serialize it.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtNullDataReader : public avtOriginatingNullDataSource
{
  public:
                                avtNullDataReader();
    virtual                    ~avtNullDataReader();

    int                         Read(char *input);

  protected:
    bool                        haveReadNullData;

};

typedef ref_ptr<avtNullDataReader> avtNullDataReader_p;

#endif

