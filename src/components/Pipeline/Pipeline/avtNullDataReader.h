// ************************************************************************* //
//                           avtNullDataReader.h                             //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_READER_H
#define AVT_NULL_DATA_READER_H
#include <pipeline_exports.h>


#include <avtTerminatingNullDataSource.h>

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
// ****************************************************************************

class PIPELINE_API avtNullDataReader : public avtTerminatingNullDataSource
{
  public:
                                avtNullDataReader();
    virtual                    ~avtNullDataReader() {;};

    int                         Read(char *input);

  protected:
    bool                        haveReadNullData;

};

typedef ref_ptr<avtNullDataReader> avtNullDataReader_p;

#endif

